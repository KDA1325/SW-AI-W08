/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
/* tiny.c는 GET 메서드로 정적 콘텐츠와 동적 콘텐츠를 처리하는
 * 단순한 반복형 HTTP/1.0 웹 서버 구현입니다.
 *
 * 2019년 11월 갱신에서는 serve_static()과 clienterror()의
 * sprintf() aliasing 관련 문제를 수정했습니다.
 */
#include "csapp.h"

// 클라이언트 하나의 HTTP 요청과 응답을 처리하는 함수입니다.
void doit(int fd);

// 요청 라인을 읽은 뒤 남은 HTTP 헤더들을 끝까지 읽는 함수입니다.
void read_requesthdrs(rio_t *rp);

// URI를 파일 경로와 CGI 인자로 분해하는 함수입니다.
int parse_uri(char *uri, char *filename, char *cgiargs);

// 정적 파일을 읽어 HTTP 응답으로 보내는 함수입니다.
void serve_static(int fd, char *filename, int filesize);

// 파일 확장자를 보고 MIME 타입을 정하는 함수입니다.
void get_filetype(char *filename, char *filetype);

// CGI 프로그램을 실행해 동적 콘텐츠를 응답하는 함수입니다.
void serve_dynamic(int fd, char *filename, char *cgiargs);

// HTTP 오류 응답을 만들어 보내는 함수입니다.
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

// 프로그램 시작점으로 포트 번호를 받아 Tiny 서버를 실행합니다.
int main(int argc, char **argv)
{
  // listenfd는 서버가 연결을 기다리는 리슨 소켓입니다.
  // connfd는 실제 클라이언트와 통신할 연결 소켓입니다.
  int listenfd, connfd;

  // hostname과 port는 접속한 클라이언트 주소를 문자열로 저장합니다.
  char hostname[MAXLINE], port[MAXLINE];

  // clientlen은 클라이언트 주소 구조체 길이입니다.
  socklen_t clientlen;

  // clientaddr은 클라이언트 주소를 저장하는 버퍼입니다.
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  /* 실행 인자가 올바른지 확인합니다. */
  if (argc != 2)
  {
    // 인자 개수가 틀리면 사용법을 표준 에러로 출력합니다.
    fprintf(stderr, "usage: %s <port>\n", argv[0]);

    // 잘못된 실행이므로 프로그램을 종료합니다.
    exit(1);
  }

  // argv[1] 포트 번호로 서버 리슨 소켓을 엽니다.
  listenfd = Open_listenfd(argv[1]);

  // 서버는 종료하지 않고 계속해서 새 연결을 받습니다.
  while (1)
  {
    // Accept가 주소 정보를 채울 수 있게 구조체 크기를 먼저 넣어둡니다.
    clientlen = sizeof(clientaddr);

    // 대기 중인 연결 요청 하나를 받아 통신용 connfd를 만듭니다.
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // line:netp:tiny:accept
    
    // 클라이언트 주소를 사람이 읽기 쉬운 호스트명과 포트 문자열로 바꿉니다.
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);

    // 어떤 클라이언트가 접속했는지 서버 로그에 출력합니다.
    printf("Accepted connection from (%s, %s)\n", hostname, port);

    // 이 연결 하나에 대한 HTTP 요청과 응답을 처리합니다.
    doit(connfd);  // line:netp:tiny:doit
    
    // 처리가 끝난 연결 소켓을 닫습니다.
    Close(connfd); // line:netp:tiny:close
  }
}

/*
 * doit - handle one HTTP request/response transaction
 */
/* doit는 클라이언트 하나의 HTTP 요청을 읽고
 * 정적 파일 또는 CGI 응답 중 하나를 처리합니다.
 */
void doit(int fd)
{
  // is_static은 정적 콘텐츠 요청인지 여부를 저장합니다.
  int is_static;

  // sbuf는 요청한 파일의 메타데이터를 담습니다.
  struct stat sbuf;
  
  // buf는 요청 라인을 저장합니다.
  // method, uri, version은 요청 라인에서 분리한 문자열입니다.
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  
  // filename은 실제 파일 시스템 경로입니다.
  // cgiargs는 CGI 프로그램에 넘길 쿼리 문자열입니다.
  char filename[MAXLINE], cgiargs[MAXLINE];
  
  // rio는 robust I/O 상태 구조체입니다.
  rio_t rio;

  // 연결된 소켓 fd를 rio 버퍼와 연결합니다.
  Rio_readinitb(&rio, fd);
  
  // 첫 요청 라인을 읽지 못하면 바로 반환합니다.
  if (!Rio_readlineb(&rio, buf, MAXLINE))
    return;
  
    // 요청 라인을 서버 로그에 출력합니다.
  printf("%s", buf);
  
  // 요청 라인에서 메서드, URI, 버전을 추출합니다.
  sscanf(buf, "%s %s %s", method, uri, version);

  // Tiny는 GET 메서드만 지원하므로 다른 메서드는 거절합니다.
  if (strcasecmp(method, "GET"))
  {
    // 지원하지 않는 메서드라는 501 오류 응답을 보냅니다.
    clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
  
    // 오류 응답 후 요청 처리를 종료합니다.
    return;
  }

  // 나머지 요청 헤더를 끝까지 읽어 버립니다.
  read_requesthdrs(&rio);

  // URI를 파일 경로와 CGI 인자로 분해합니다.
  is_static = parse_uri(uri, filename, cgiargs);

  // 요청한 경로의 파일 상태를 확인합니다.
  if (stat(filename, &sbuf) < 0)
  {
    // 파일이 없으면 404 오류 응답을 보냅니다.
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");

    // 더 처리할 수 없으므로 반환합니다.
    return;
  }

  if (is_static)
  {
    // 일반 파일이 아니거나 읽기 권한이 없으면 403을 보냅니다.
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
    {
      // 읽을 수 없는 파일이라는 403 오류 응답을 보냅니다.
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");

      // 오류 응답 후 반환합니다.
      return;
    }

    // 정적 파일 내용을 그대로 클라이언트에게 보냅니다.
    serve_static(fd, filename, sbuf.st_size);
  }
  else
  {
    // 일반 파일이 아니거나 실행 권한이 없으면 403을 보냅니다.
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
    {
      // 실행할 수 없는 CGI 프로그램이라는 403 오류 응답을 보냅니다.
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");

      // 오류 응답 후 반환합니다.
      return;
    }

    // CGI 프로그램을 실행해 동적 응답을 생성합니다.
    serve_dynamic(fd, filename, cgiargs);
  }
}

/*
 * read_requesthdrs - read HTTP request headers
 */
/* 요청 라인을 제외한 나머지 HTTP 헤더들을 읽습니다. */
void read_requesthdrs(rio_t *rp)
{
  // buf는 헤더 한 줄을 저장하는 버퍼입니다.
  char buf[MAXLINE];

  // 첫 번째 헤더 줄을 읽습니다.
  Rio_readlineb(rp, buf, MAXLINE);

  // 읽은 헤더를 서버 로그에 출력합니다.
  printf("%s", buf);

  // 빈 줄이 나올 때까지 나머지 헤더를 계속 읽습니다.
  while (strcmp(buf, "\r\n"))
  {
    // 다음 헤더 줄을 읽습니다.
    Rio_readlineb(rp, buf, MAXLINE);

    // 읽은 헤더를 서버 로그에 출력합니다.
    printf("%s", buf);
  }
}

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 1 if static content, 0 if dynamic content
 */
/* URI를 실제 파일 경로와 CGI 인자로 분해합니다.
 * 반환값 1은 정적 콘텐츠, 0은 동적 콘텐츠를 뜻합니다.
 */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
  // ptr은 URI 안에서 ? 문자의 위치를 가리킵니다.
  char *ptr;

  if (!strstr(uri, "cgi-bin"))
  {
    // cgi-bin이 없으면 정적 콘텐츠로 처리합니다.
    // 정적 요청은 CGI 인자가 없으므로 빈 문자열로 둡니다.
    strcpy(cgiargs, "");

    // 현재 디렉터리 기준 경로를 만들기 위해 .으로 시작합니다.
    strcpy(filename, ".");

    // URI를 뒤에 붙여 실제 파일 경로를 만듭니다.
    strcat(filename, uri);

    // URI가 /로 끝나면 기본 페이지 home.html을 붙입니다.
    if (uri[strlen(uri) - 1] == '/')
      strcat(filename, "home.html");

    // 정적 콘텐츠이므로 1을 반환합니다.
    return 1;
  }

  // cgi-bin이 있으면 동적 콘텐츠로 처리합니다.
  // 쿼리 문자열 구분자인 ? 위치를 찾습니다.
  ptr = index(uri, '?');

  // ?가 있으면 뒤쪽을 CGI 인자로 분리합니다.
  if (ptr)
  {
    // ? 다음 문자열을 cgiargs에 복사합니다.
    strcpy(cgiargs, ptr + 1);

    // ? 위치를 문자열 끝으로 바꿔 순수 경로만 남깁니다.
    *ptr = '\0';
  }
  else
    // 쿼리 문자열이 없으면 빈 문자열을 넣습니다.
    strcpy(cgiargs, "");

  // 현재 디렉터리 기준 경로를 만들기 위해 .으로 시작합니다.
  strcpy(filename, ".");

  // URI 경로를 붙여 CGI 프로그램 경로를 만듭니다.
  strcat(filename, uri);

  // 동적 콘텐츠이므로 0을 반환합니다.
  return 0;
}

/*
 * serve_static - copy a file back to the client
 */
/* 정적 파일을 읽어 클라이언트에게 HTTP 응답으로 보냅니다. */
void serve_static(int fd, char *filename, int filesize)
{
  // srcfd는 정적 파일을 여는 파일 디스크립터입니다.
  int srcfd;

  // srcp는 메모리에 매핑된 파일 내용을 가리킵니다.
  char *srcp;
  
  // filetype은 Content-Type 헤더에 넣을 MIME 타입 문자열입니다.
  char filetype[MAXLINE];
  
  // buf는 응답 헤더를 저장하는 버퍼입니다.
  char buf[MAXBUF];
  
  // p는 buf 안에서 다음 문자열을 써 넣을 위치입니다.
  char *p = buf;
  
  // n은 snprintf가 기록한 문자 수입니다.
  int n;
  
  // remaining은 buf에 남은 공간 크기입니다.
  int remaining = sizeof(buf);

  // 파일 이름을 보고 MIME 타입을 결정합니다.
  get_filetype(filename, filetype);

  // 상태 줄을 기록합니다.
  n = snprintf(p, remaining, "HTTP/1.0 200 OK\r\n");
  
  // 포인터를 다음 기록 위치로 옮깁니다.
  p += n;
  
  // 남은 버퍼 크기를 갱신합니다.
  remaining -= n;

  // Server 헤더를 추가합니다.
  n = snprintf(p, remaining, "Server: Tiny Web Server\r\n");
  
  // 포인터를 다음 기록 위치로 옮깁니다.
  p += n;
  
  // 남은 버퍼 크기를 갱신합니다.
  remaining -= n;

  // Connection 헤더를 추가합니다.
  n = snprintf(p, remaining, "Connection: close\r\n");
  
  // 포인터를 다음 기록 위치로 옮깁니다.
  p += n;
  
  // 남은 버퍼 크기를 갱신합니다.
  remaining -= n;

  // Content-length 헤더를 추가합니다.
  n = snprintf(p, remaining, "Content-length: %d\r\n", filesize);
  
  // 포인터를 다음 기록 위치로 옮깁니다.
  p += n;
  
  // 남은 버퍼 크기를 갱신합니다.
  remaining -= n;

  // Content-type 헤더와 헤더 종료용 빈 줄을 추가합니다.
  snprintf(p, remaining, "Content-type: %s\r\n\r\n", filetype);

  // 완성한 응답 헤더를 클라이언트에게 전송합니다.
  Rio_writen(fd, buf, strlen(buf));
  
  // 디버깅용으로 응답 헤더를 서버 로그에 출력합니다.
  printf("Response headers:\n");
  
  // 실제 전송한 헤더 내용을 그대로 출력합니다.
  printf("%s", buf);

  // 정적 파일을 읽기 전용으로 엽니다.
  srcfd = Open(filename, O_RDONLY, 0);
  
  // 파일 전체를 메모리에 매핑합니다.
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  
  // 파일 디스크립터는 더 이상 필요 없으므로 닫습니다.
  Close(srcfd);
  
  // 매핑된 파일 내용을 클라이언트에게 전송합니다.
  Rio_writen(fd, srcp, filesize);
  
  // 사용이 끝난 메모리 매핑을 해제합니다.
  Munmap(srcp, filesize);
}

/*
 * get_filetype - derive file type from file name
 */
/* 파일 이름의 확장자를 보고 MIME 타입을 정합니다. */
void get_filetype(char *filename, char *filetype)
{
  // .html 파일이면 text/html로 설정합니다.
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  // .gif 파일이면 image/gif로 설정합니다.
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  // .png 파일이면 image/png로 설정합니다.
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  // .jpg 파일이면 image/jpeg로 설정합니다.
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  // 그 외에는 일반 텍스트로 처리합니다.
  else
    strcpy(filetype, "text/plain");
}

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* CGI 프로그램을 실행해 동적 콘텐츠를 클라이언트에게 응답합니다. */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
  // buf는 초기 응답 헤더를 만드는 버퍼입니다.
  // emptylist는 Execve에 넘길 빈 인자 목록입니다.
  char buf[MAXLINE], *emptylist[] = {NULL};

  // 상태 줄을 만듭니다.
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  
  // 상태 줄을 클라이언트에게 보냅니다.
  Rio_writen(fd, buf, strlen(buf));
  
  // Server 헤더를 만듭니다.
  sprintf(buf, "Server: Tiny Web Server\r\n");
  
  // Server 헤더를 클라이언트에게 보냅니다.
  Rio_writen(fd, buf, strlen(buf));

  // 자식 프로세스를 만들어 CGI 프로그램을 실행합니다.
  if (Fork() == 0)
  {
    // CGI 프로그램이 읽을 QUERY_STRING 환경 변수를 설정합니다.
    setenv("QUERY_STRING", cgiargs, 1);
  
    // 자식 프로세스의 표준 출력을 클라이언트 소켓으로 연결합니다.
    Dup2(fd, STDOUT_FILENO);
  
    // CGI 프로그램을 실행합니다.
    Execve(filename, emptylist, environ);
  }
  
  // 부모 프로세스는 자식 CGI 프로세스가 끝날 때까지 기다립니다.
  Wait(NULL);
}

/*
 * clienterror - returns an error message to the client
 */
/* 오류 상황에서 HTML 본문과 HTTP 헤더를 만들어 클라이언트에게 보냅니다. */
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
  // buf는 오류 응답 헤더를 담는 버퍼입니다.
  // body는 오류 HTML 본문을 담는 버퍼입니다.
  char buf[MAXLINE], body[MAXBUF];

  // HTML 제목을 본문에 기록합니다.
  snprintf(body, sizeof(body), "<html><title>Tiny Error</title>");
  
  // body 태그를 본문에 이어 붙입니다.
  snprintf(body + strlen(body), sizeof(body) - strlen(body), "<body bgcolor=\"ffffff\">\r\n");
  
  // 상태 코드와 짧은 오류 메시지를 본문에 추가합니다.
  snprintf(body + strlen(body), sizeof(body) - strlen(body), "%s: %s\r\n", errnum, shortmsg);
  
  // 자세한 오류 설명과 원인을 본문에 추가합니다.
  snprintf(body + strlen(body), sizeof(body) - strlen(body), "<p>%s: %s\r\n", longmsg, cause);
  
  // Tiny 서버 서명을 본문 마지막에 추가합니다.
  snprintf(body + strlen(body), sizeof(body) - strlen(body), "<hr><em>The Tiny Web server</em>\r\n");

  // 상태 줄을 만듭니다.
  snprintf(buf, sizeof(buf), "HTTP/1.0 %s %s\r\n", errnum, shortmsg);

  // 상태 줄을 클라이언트에게 전송합니다.
  Rio_writen(fd, buf, strlen(buf));

  // Content-type 헤더를 만듭니다.
  snprintf(buf, sizeof(buf), "Content-type: text/html\r\n");

  // Content-type 헤더를 전송합니다.
  Rio_writen(fd, buf, strlen(buf));

  // Content-length 헤더와 헤더 종료 빈 줄을 만듭니다.
  snprintf(buf, sizeof(buf), "Content-length: %d\r\n\r\n", (int)strlen(body));

  // Content-length 헤더를 전송합니다.
  Rio_writen(fd, buf, strlen(buf));
  
  // 완성한 오류 본문을 전송합니다.
  Rio_writen(fd, body, strlen(body));
}

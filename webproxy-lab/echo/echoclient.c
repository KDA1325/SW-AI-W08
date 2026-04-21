#include "../csapp.h"

int main(int argc, char **argv)
{
    // 서버와 연결된 클라이언트 소켓 fd
    int clientfd;
    // 서버 호스트, 서버 포트, 한 줄씩 읽고 쓸 버퍼
    char *host, *port, buf[MAXLINE];
    // robust I/O를 위한 rio 버퍼 상태
    rio_t rio;

    // 실행 인자는 host와 port 두 개를 받아야 합니다.
    if (argc != 3) {
        // 인자 개수가 틀리면 사용법을 출력하고 종료합니다.
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    // 첫 번째 인자는 접속할 서버 호스트입니다.
    host = argv[1];
    // 두 번째 인자는 접속할 서버 포트입니다.
    port = argv[2];
    // host, port로 서버에 TCP 연결을 열고 소켓 fd를 받습니다.
    clientfd = Open_clientfd(host, port);

    // 이후 소켓에서 한 줄씩 편하게 읽기 위해 rio 상태를 초기화합니다.
    Rio_readinitb(&rio, clientfd);

    // 표준 입력에서 한 줄씩 읽어서 서버로 보내고 응답을 다시 받습니다.
    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        // 사용자가 입력한 문자열을 서버로 그대로 보냅니다.
        Rio_writen(clientfd, buf, strlen(buf));
        // 서버가 연결을 닫으면 더 이상 읽을 수 없으므로 반복을 끝냅니다.
        if (Rio_readlineb(&rio, buf, MAXLINE) == 0) {
            break;
        }
        // 서버가 돌려준 응답을 클라이언트 터미널에 출력합니다.
        Fputs(buf, stdout);
    }

    // 통신이 끝나면 서버와의 연결 소켓을 닫습니다.
    Close(clientfd);

    // 프로그램을 정상 종료합니다.
    exit(0);
}

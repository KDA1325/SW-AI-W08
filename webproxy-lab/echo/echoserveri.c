#include "../csapp.h"

// 연결된 클라이언트에게 받은 데이터를 한 줄씩 그대로 다시 보내는 함수입니다.
void echo(int connfd);

// 프로그램 시작점으로 포트 번호를 인자로 받아 서버를 실행합니다.
// argc는 인자 개수이고 argv는 각 인자 문자열 배열입니다.
// ex) ./echoserveri 5001
// -> argc: 2
// -> argv[0]: ./echoserveri
// -> argv[1]: 5001
int main(int argc, char **argv)
{
    // listenfd는 서버가 접속을 기다리는 소켓 fd입니다.
    // connfd는 실제로 연결된 클라이언트와 통신할 소켓 fd입니다.
    int listenfd, connfd;

    // 클라이언트 주소 구조체의 길이를 저장합니다.
    socklen_t clientlen;

    // clientaddr은 클라이언트 주소를 저장하는 버퍼입니다.
    // sockaddr_storage를 써서 IPv4와 IPv6를 모두 담을 수 있게 합니다.
    struct sockaddr_storage clientaddr;
    
    // client_hostname은 클라이언트 호스트 이름 문자열입니다.
    // client_port는 클라이언트 포트 번호 문자열입니다.
    char client_hostname[MAXLINE], client_port[MAXLINE];

    // 실행 인자는 프로그램 이름과 포트 번호, 이렇게 총 2개여야 합니다.
    if (argc != 2) {
        // 인자 개수가 틀리면 사용법을 출력하고 종료합니다.
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        // 잘못된 실행이므로 프로그램을 바로 끝냅니다.
        exit(0);
    }

    // argv[1] 포트 번호로 서버가 접속을 기다릴 리슨 소켓을 엽니다.
    listenfd = Open_listenfd(argv[1]);

    // 서버는 종료하지 않고 무한히 반복하며 새 연결을 받습니다.
    while (1) {
        // accept가 주소 정보를 채워 넣을 수 있게 구조체 크기를 먼저 넣어둡니다.
        clientlen = sizeof(struct sockaddr_storage);

        // 대기열의 연결 요청 하나를 받아 실제 통신용 connfd를 만듭니다.
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        // 클라이언트 주소 정보를 보기 쉬운 호스트명과 포트 문자열로 바꿉니다.
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);

        // 어떤 클라이언트가 접속했는지 서버 로그에 출력합니다.
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        // 이 클라이언트와 에코 통신을 처리합니다.
        echo(connfd);

        // 통신이 끝나면 이 클라이언트 전용 소켓을 닫습니다.
        Close(connfd);
    }

    // 실제로는 무한 루프라 여기까지 오지 않지만 종료 코드를 남겨 둡니다.
    exit(0);
}
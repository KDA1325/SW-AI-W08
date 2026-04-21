#include "../csapp.h"

// connfd로 들어오는 한 줄 데이터를 그대로 다시 connfd로 보내는 함수
void echo(int connfd)
{
    // 실제로 읽은 바이트 수
    size_t n;
    // 한 줄 데이터를 잠시 담아둘 버퍼
    char buf[MAXLINE];
    // connfd를 robust I/O로 읽기 위한 rio 상태
    rio_t rio;

    // 연결된 소켓 connfd를 rio 버퍼와 연결합니다.
    Rio_readinitb(&rio, connfd);

    // 클라이언트가 보낸 데이터를 한 줄씩 읽어 EOF가 나올 때까지 반복합니다.
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        // 서버 쪽에서는 몇 바이트를 받았는지 로그로 남깁니다.
        printf("server received %d bytes\n", (int)n);
        // 방금 받은 n바이트를 그대로 클라이언트에게 다시 보냅니다.
        Rio_writen(connfd, buf, n);
    }
}

# Echo Server/Client Top-Down Explanation

## 먼저, 이 문서에서 얻어가야 할 것

자, 먼저 큰 그림을 보겠습니다.

이 문서의 목표는 하나입니다.

> echo 서버와 클라이언트를 코드 줄 단위로 외우기 전에,  
> "무슨 문제를 풀고 있는지"를 먼저 머릿속에 잡는 것

이게 진짜 중요합니다.

처음 공부할 때 제일 답답한 지점은 이겁니다.

- `main(argc, argv)`는 왜 저렇게 생겼지?
- 왜 서버는 `listenfd`와 `connfd`를 나누지?
- 왜 `read`, `write` 말고 `rio` 같은 게 또 나오지?
- 도대체 어디서부터 읽어야 하지?

그래서 이 문서는 순서를 바꿉니다.

코드를 바로 해설하지 않습니다. 먼저 목표를 잡고, 그 목표를 이루기 위해 어떤 문제를 해결해야 하는지부터 봅니다. 그 다음 최소 코드로 내려오고, 마지막에 현재 `echo` 디렉토리 코드가 그 구조를 어떻게 구현했는지 연결하겠습니다.

즉, 순서는 이렇게 갑니다.

1. 목표
2. 해결해야 할 문제
3. 최소 서버/클라이언트 구조
4. 현재 코드와 연결
5. 현재 코드의 현실 체크

## 왜 echo 서버를 먼저 볼까요?

HTTP 서버나 프록시도 결국은 네트워크 프로그램입니다. 겉으로는 복잡해 보여도 뼈대는 거의 같습니다.

```text
listen -> accept -> read -> write -> close
```

echo 서버는 이 흐름을 가장 작고 단순하게 보여주는 예제입니다.

클라이언트는 서버에 연결합니다.  
문자열을 보냅니다.  
서버는 그것을 읽습니다.  
그리고 그대로 다시 돌려줍니다.

정말 이것뿐입니다.

그런데 이 단순한 흐름 안에 이후 `tiny`, `proxy`, 웹 서버, 프록시 서버로 이어지는 핵심 감각이 다 들어 있습니다.

## 서버의 목표를 먼저 한 문장으로 잡아봅시다

서버 목표는 아주 단순합니다.

> TCP로 연결된 클라이언트가 보낸 바이트를 읽어서, 그대로 다시 돌려준다.

그러면 이제 질문이 바뀝니다.

"코드가 뭐지?"가 아니라,
"이 목표를 달성하려면 서버가 어떤 문제를 해결해야 하지?"가 됩니다.

탑다운 방식은 바로 이 순서로 가야 합니다.

## 서버가 해결해야 할 문제는 딱 네 개입니다

서버는 아래 네 문제만 해결하면 됩니다.

1. 몇 번 포트에서 기다릴지 알아야 합니다.
2. 클라이언트가 들어오면 연결을 받아야 합니다.
3. 그 연결에서 데이터를 읽어야 합니다.
4. 읽은 데이터를 그대로 다시 써야 합니다.

이렇게 쪼개서 보면, 서버가 갑자기 훨씬 단순해 보입니다.

## 최소 서버는 어떻게 생길까요?

자, 위 네 문제를 코드로 바로 옮기면 최소 서버는 아래처럼 생깁니다.

```c
int main(int argc, char **argv) {
    int listenfd, connfd;

    listenfd = open_listenfd(argv[1]);

    while (1) {
        connfd = accept(listenfd, NULL, NULL);
        echo(connfd);
        close(connfd);
    }
}
```

그리고 `echo(connfd)`는 이렇게 줄일 수 있습니다.

```c
void echo(int connfd) {
    char buf[8192];
    ssize_t n;

    while ((n = read(connfd, buf, sizeof(buf))) > 0) {
        write(connfd, buf, n);
    }
}
```

이게 진짜 최소 형태입니다.

### 이 코드가 방금 본 네 문제와 어떻게 연결될까요?

- `argv[1]`
  서버가 몇 번 포트에서 기다릴지 받습니다.
- `open_listenfd(argv[1])`
  기다릴 준비를 합니다.
- `accept(...)`
  새 손님 연결을 받습니다.
- `read(...)`
  손님이 보낸 데이터를 읽습니다.
- `write(...)`
  읽은 데이터를 그대로 돌려줍니다.
- `close(connfd)`
  현재 손님과의 연결만 닫습니다.

느낌이 오시죠?

코드를 보기 전에 문제를 먼저 잘게 쪼개면, 함수 이름이 왜 필요한지도 자연스럽게 보입니다.

## `argc`, `argv`는 언제 무엇을 받나요?

여기서 초심자가 가장 많이 막히는 부분이 이것입니다.

그런데 아주 단순합니다.

프로그램 실행할 때 터미널에 적는 단어들이 그대로 `argv`에 들어옵니다.

예를 들어 서버를 이렇게 실행하면:

```bash
./echoserveri 5000
```

아래처럼 들어옵니다.

- `argv[0]`: `"./echoserveri"`
- `argv[1]`: `"5000"`

즉, 서버는 `"5000"`을 받아서 그 포트에서 기다리겠다는 뜻입니다.

그래서 아까 최소 서버 코드의 이 줄이 이해됩니다.

```c
listenfd = open_listenfd(argv[1]);
```

이제 왜 이런 코드가 나왔는지 이해가 되시죠?

## `listenfd`와 `connfd`는 왜 둘 다 필요할까요?

이것도 큰 그림으로 보면 쉽습니다.

- `listenfd`: 대표번호
- `connfd`: 지금 들어온 손님 한 명과 연결된 개별 통화선

가게로 비유하면,

- 대표전화 번호는 계속 살아 있어야 합니다.
- 손님 한 명과 통화가 끝났다고 대표번호를 없애면 안 됩니다.
- 그래서 손님별 연결 소켓이 따로 필요합니다.

이 감각이 잡혀야 서버 코드를 읽을 때 덜 헷갈립니다.

## 왜 `rio` 같은 게 나올까요?

이제 한 단계만 더 올라가 보겠습니다.

아까 본 최소 코드는 `read`, `write`만 써도 됩니다. 이해용으로는 그게 가장 좋습니다.

그런데 교재는 왜 `Rio_readlineb`, `Rio_writen` 같은 걸 쓸까요?

이유는 단순합니다.

TCP는 바이트 스트림입니다. 즉, 우리가 기대하는 만큼 한 번에 예쁘게 읽히지 않을 수 있습니다. 실무에서는 이런 부분 때문에 버퍼링, 부분 읽기/쓰기, 줄 단위 처리 같은 문제가 생깁니다.

그래서 CS:APP은 좀 더 안전한 I/O 래퍼를 제공합니다.

즉, 순서는 이렇게 생각하시면 됩니다.

1. 먼저 최소 코드로 본질을 이해합니다.
2. 그 다음 `rio`로 왜 개선하는지 봅니다.

이게 탑다운 학습입니다.

## 클라이언트 목표도 한 문장으로 잡아봅시다

클라이언트 목표도 단순합니다.

> 서버에 연결해서, 내가 입력한 내용을 보내고, 서버가 돌려준 내용을 출력한다.

그러면 클라이언트가 해결해야 할 문제는 세 개입니다.

1. 어느 호스트와 포트에 연결할지 알아야 합니다.
2. 실제로 연결해야 합니다.
3. 표준입력과 소켓 사이를 중계해야 합니다.

## 최소 클라이언트는 이렇게 생깁니다

```c
int main(int argc, char **argv) {
    int clientfd;
    char buf[8192];
    ssize_t n;

    clientfd = open_clientfd(argv[1], argv[2]);

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        write(clientfd, buf, strlen(buf));
        n = read(clientfd, buf, sizeof(buf) - 1);
        buf[n] = '\0';
        fputs(buf, stdout);
    }

    close(clientfd);
}
```

### 이 코드는 어떤 문제를 해결하나요?

- `argv[1]`, `argv[2]`
  어디로 연결할지 받습니다.
- `open_clientfd(...)`
  서버에 연결합니다.
- `fgets(...)`
  사용자가 입력한 한 줄을 읽습니다.
- `write(...)`
  서버로 보냅니다.
- `read(...)`
  서버 응답을 받습니다.
- `fputs(...)`
  화면에 출력합니다.

## 이제 현재 `echo` 디렉토리 코드와 연결해보겠습니다

자, 이제야 실제 파일을 보는 순서가 됩니다.

이제는 그냥 코드 해설이 아니라,  
"내가 방금 이해한 최소 구조를 이 코드가 어떻게 구현했는가?"  
이 관점으로 보면 됩니다.

현재 `echo` 디렉토리의 역할 분리는 아래와 같습니다.

- 서버 시작점: [echoserveri.c](/workspaces/webproxy_lab_docker/webproxy-lab/echo/echoserveri.c:5)
- 실제 echo 처리: [echo.c](/workspaces/webproxy_lab_docker/webproxy-lab/echo/echo.c:3)
- 클라이언트 테스트 도구: [echoclient.c](/workspaces/webproxy_lab_docker/webproxy-lab/echo/echoclient.c:3)

이 분리는 아주 좋습니다.

- `main`은 전체 흐름을 담당합니다.
- `echo()`는 연결 하나를 처리하는 핵심 동작만 담당합니다.
- 클라이언트는 서버를 검증하는 별도 프로그램 역할을 합니다.

실무에서도 이렇게 역할을 나누면 코드가 읽히고 수정이 쉬워집니다.

## 현재 서버 코드는 최소 서버 구조를 어떻게 구현하나요?

먼저 [echoserveri.c](/workspaces/webproxy_lab_docker/webproxy-lab/echo/echoserveri.c:5)를 보겠습니다.

이 파일은 아까 본 최소 서버 구조를 조금 더 교재 스타일로 감싼 것입니다.

### 1. 포트를 실행 인자로 받습니다

예를 들어:

```bash
./echoserveri 5000
```

그러면 `argv[1]`에 `"5000"`이 들어갑니다.

이건 최소 서버에서 본 "몇 번 포트에서 기다릴지 알아야 한다"를 구현한 것입니다.

### 2. `Open_listenfd(argv[1])`로 서버 소켓을 엽니다

```c
listenfd = Open_listenfd(argv[1]);
```

여기서 `Open_listenfd`는 내부적으로 이런 작업을 감쌉니다.

- `socket`
- `bind`
- `listen`

즉, 최소 서버의 `open_listenfd(...)`와 같은 역할입니다.

### 3. 무한 루프로 클라이언트를 계속 받습니다

```c
connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
echo(connfd);
Close(connfd);
```

이건 최소 서버의 아래 흐름과 정확히 대응됩니다.

```c
connfd = accept(listenfd, NULL, NULL);
echo(connfd);
close(connfd);
```

차이는 무엇일까요?

- 교재 버전은 `Accept`, `Close` 같은 래퍼를 씁니다.
- 클라이언트 주소 정보를 로그로 남기기 위해 `Getnameinfo(...)`도 추가합니다.

하지만 본질은 같습니다.

## 현재 `echo()` 함수는 최소 echo 함수와 어떻게 연결되나요?

[echo.c](/workspaces/webproxy_lab_docker/webproxy-lab/echo/echo.c:3)의 본질은 아까 본 최소 함수와 같습니다.

핵심은 이 한 줄입니다.

> 읽고, 그대로 다시 쓴다.

현재 코드는 단순 `read/write` 대신 `rio`를 씁니다.

### 1. `Rio_readinitb(&rio, connfd)`

소켓을 robust I/O 버퍼와 연결합니다.

### 2. `Rio_readlineb(...)`

한 줄씩 읽습니다.

### 3. `Rio_writen(connfd, buf, n)`

읽은 내용을 그대로 다시 보냅니다.

즉, 최소 버전의:

```c
n = read(...);
write(...);
```

가 교재 스타일에서는:

```c
n = Rio_readlineb(...);
Rio_writen(...);
```

로 올라간 것입니다.

이제 왜 이런 기술이 나왔는지 이해가 되시죠?

## 현재 클라이언트 코드는 최소 클라이언트와 어떻게 연결되나요?

[echoclient.c](/workspaces/webproxy_lab_docker/webproxy-lab/echo/echoclient.c:3)도 똑같습니다.

최소 클라이언트의 흐름:

1. 연결한다
2. 한 줄 읽는다
3. 서버에 보낸다
4. 응답을 읽는다
5. 출력한다

현재 코드도 정확히 그 순서를 따릅니다.

### 1. 서버에 연결합니다

```c
clientfd = Openclientfd(host, port);
```

의도는 맞습니다. 다만 현재 코드에는 오타가 있어서 실제 루트 `csapp.h` 기준으로는 `Open_clientfd`가 맞습니다.

### 2. 소켓용 `rio`를 초기화합니다

```c
Rio_readinitb(&rio, clientfd);
```

### 3. 사용자 입력을 읽어서 서버로 보냅니다

```c
while (Fgets(buf, MAXLINE, stdin) != NULL)
```

### 4. 서버 응답을 읽어 화면에 출력합니다

```c
Rio_readlineb(&rio, buf, MAXLINE);
Fputs(buf, stdout);
```

즉, "내가 친 문장이 서버를 갔다가 그대로 돌아온다"는 경험을 만드는 코드입니다.

## 현재 코드의 현실 체크도 해보겠습니다

자, 여기서부터는 학습 흐름이 아니라 현재 저장소 상태를 점검하는 파트입니다.

이 구분도 중요합니다.

- 위에서는 "개념 구조"를 봤습니다.
- 여기서는 "지금 코드가 실제로 어떤 상태인지"를 봅니다.

### 1. `echoclient.c`에는 오타가 있습니다

- `fprinf` -> `fprintf`
- `Openclientfd` -> `Open_clientfd`

즉, 개념 설명용 뼈대는 맞지만 그대로는 빌드가 안 됩니다.

### 2. `echo` 디렉토리 내부 공통 파일들은 비어 있습니다

현재 아래 파일들은 사실상 비어 있습니다.

- [csapp.h](/workspaces/webproxy_lab_docker/webproxy-lab/echo/csapp.h:1)
- [csapp.c](/workspaces/webproxy_lab_docker/webproxy-lab/echo/csapp.c:1)
- [Makefile](/workspaces/webproxy_lab_docker/webproxy-lab/echo/Makefile:1)

그래서 지금 `echo` 디렉토리는 이렇게 이해하시면 됩니다.

- `echo/` 내부 코드: 학습용 뼈대
- 루트의 `csapp.*`: 실제 공통 엔진

실제 함수 원형과 구현은 루트 쪽 파일에 있습니다.

- [csapp.h](/workspaces/webproxy_lab_docker/webproxy-lab/csapp.h:1)
- [csapp.c](/workspaces/webproxy_lab_docker/webproxy-lab/csapp.c:941)

예를 들어:

- `open_listenfd()`는 내부적으로 주소 후보를 찾고 `socket`, `bind`, `listen`을 처리합니다.
- `open_clientfd()`는 `getaddrinfo` 후 `connect`까지 처리합니다.
- `Rio_readlineb`, `Rio_writen`도 그쪽에 구현되어 있습니다.

느낌이 오시죠? 지금 `echo` 디렉토리는 설계도에 가깝고, 루트 `csapp`은 실제 엔진에 가깝습니다.

## 마지막 정리

정리해보겠습니다.

- 먼저 목표를 잡았습니다.
- 그 목표를 이루기 위해 서버와 클라이언트가 각각 해결해야 할 문제를 나눴습니다.
- 그 다음 최소 구현으로 내려왔습니다.
- 마지막에 현재 `echo` 디렉토리 코드가 그 최소 구조를 어떻게 구현했는지 연결했습니다.

이 순서가 탑다운 학습입니다.

코드를 바로 해설부터 보는 것보다,
"무슨 문제를 푸는가 -> 그래서 어떤 함수가 필요한가 -> 그래서 실제 코드가 왜 이렇게 생겼는가"
이 순서로 가야 덜 막히고 오래 기억됩니다.

이 관점으로 보면 이후 `tiny`, `proxy`도 덜 무겁게 느껴집니다. 결국 echo 서버 뼈대 위에 요청 파싱, 응답 생성, 중계, 동시성 같은 기능이 올라가는 것입니다.

직접 따라 치면서 만들어보시면 훨씬 빨리 체득됩니다. 보기만 하지 마시고, 꼭 `main`부터 한 줄씩 쳐보세요.

/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* adder.c는 두 숫자를 더해서 결과를 HTML 형태로 출력하는
 * 아주 간단한 CGI 프로그램입니다.
 */
/* $begin adder */
#include "csapp.h"

// CGI 프로그램 시작점으로 QUERY_STRING에서 두 숫자를 읽어 합을 계산합니다.
int main(void)
{
  // buf는 QUERY_STRING 전체를 가리키는 포인터입니다.
  // p는 & 문자의 위치를 가리키는 포인터입니다.
  char *buf, *p;

  // arg1과 arg2는 각각 첫 번째, 두 번째 key=value 문자열입니다.
  // content는 최종 HTTP 응답 본문을 저장합니다.
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];

  // n1과 n2는 파싱한 두 정수 값입니다.
  int n1 = 0, n2 = 0;

  /* Extract the two arguments */
  /* QUERY_STRING에서 두 인자를 분리해 정수로 변환합니다. */
  // QUERY_STRING 환경 변수가 있으면 쿼리 문자열을 읽습니다.
  if ((buf = getenv("QUERY_STRING")) != NULL)
  {
    // 두 인자를 구분하는 & 문자 위치를 찾습니다.
    p = strchr(buf, '&');

    // 첫 번째 인자 문자열 끝이 되도록 &를 문자열 종료 문자로 바꿉니다.
    *p = '\0';

    // 첫 번째 key=value 문자열을 arg1에 복사합니다.
    strcpy(arg1, buf);

    // 두 번째 key=value 문자열을 arg2에 복사합니다.
    strcpy(arg2, p + 1);

    // arg1의 = 뒤 숫자를 정수로 변환합니다.
    n1 = atoi(strchr(arg1, '=') + 1);

    // arg2의 = 뒤 숫자를 정수로 변환합니다.
    n2 = atoi(strchr(arg2, '=') + 1);
  }

  /* Make the response body */
  /* HTML 응답 본문을 순서대로 만듭니다. */
  // QUERY_STRING 값을 본문 첫 줄에 기록합니다.
  sprintf(content, "QUERY_STRING=%s\r\n<p>", buf);

  // 안내 문구 첫 부분을 본문 뒤에 붙입니다.
  sprintf(content + strlen(content), "Welcome to add.com: ");

  // 안내 문구 나머지 부분을 본문 뒤에 붙입니다.
  sprintf(content + strlen(content), "THE Internet addition portal.\r\n<p>");

  // 계산 결과 문장을 본문 뒤에 붙입니다.
  sprintf(content + strlen(content), "The answer is: %d + %d = %d\r\n<p>", n1, n2, n1 + n2);

  // 마무리 문구를 본문 뒤에 붙입니다.
  sprintf(content + strlen(content), "Thanks for visiting!\r\n");

  /* Generate the HTTP response */
  /* HTTP 응답 헤더와 본문을 표준 출력으로 보냅니다. */
  // Content-type 헤더를 출력합니다.
  printf("Content-type: text/html\r\n");

  // Content-length 헤더를 출력합니다.
  printf("Content-length: %d\r\n", (int)strlen(content));

  // 헤더와 본문을 구분하는 빈 줄을 출력합니다.
  printf("\r\n");

  // 완성한 본문 내용을 출력합니다.
  printf("%s", content);
  
  // 출력 버퍼를 즉시 비웁니다.
  fflush(stdout);

  // CGI 프로그램을 정상 종료합니다.
  exit(0);
}
/* $end adder */

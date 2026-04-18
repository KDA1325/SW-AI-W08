####################################################################
# CS:APP Proxy Lab
#
# Student Source Files
####################################################################

This directory contains the files you will need for the CS:APP Proxy
Lab.

proxy.c
csapp.h
csapp.c
    These are starter files.  csapp.c and csapp.h are described in
    your textbook. 

    You may make any changes you like to these files.  And you may
    create and handin any additional files you like.

    Please use `port-for-user.pl' or 'free-port.sh' to generate
    unique ports for your proxy or tiny server. 

Makefile
    This is the makefile that builds the proxy program.  Type "make"
    to build your solution, or "make clean" followed by "make" for a
    fresh build. 

    Type "make handin" to create the tarfile that you will be handing
    in. You can modify it any way you like. Your instructor will use your
    Makefile to build your proxy from source.

port-for-user.pl
    Generates a random port for a particular user
    usage: ./port-for-user.pl <userID>

free-port.sh
    Handy script that identifies an unused TCP port that you can use
    for your proxy or tiny. 
    usage: ./free-port.sh

driver.sh
    The autograder for Basic, Concurrency, and Cache.        
    usage: ./driver.sh

nop-server.py
     helper for the autograder.         

tiny
    Tiny Web server from the CS:APP text

--------------------------------------------------------------------
한국어 번역
--------------------------------------------------------------------

이 디렉터리에는 CS:APP Proxy Lab에 필요한 파일들이 들어 있습니다.

proxy.c
csapp.h
csapp.c
    이 파일들은 시작용(starter) 파일입니다. `csapp.c`와 `csapp.h`는
    교재에 설명되어 있습니다.

    이 파일들은 자유롭게 수정해도 됩니다. 또한 필요한 추가 파일을
    만들어 제출해도 됩니다.

    프록시나 tiny 서버에서 사용할 고유 포트를 만들 때는
    `port-for-user.pl` 또는 `free-port.sh`를 사용하세요.

Makefile
    프록시 프로그램을 빌드하는 메이크파일입니다. `make`를 실행하면
    해답을 빌드할 수 있고, 깨끗한 빌드를 하려면 `make clean` 후
    다시 `make`를 실행하면 됩니다.

    `make handin`을 실행하면 제출할 tar 파일이 생성됩니다.
    원하는 대로 수정해도 되지만, 교수자는 여러분의 `Makefile`을
    사용해 소스에서 프록시를 빌드합니다.

port-for-user.pl
    특정 사용자를 위한 임의의 포트를 생성합니다.
    사용법: `./port-for-user.pl <userID>`

free-port.sh
    프록시나 tiny에서 사용할 수 있는 비어 있는 TCP 포트를 찾아주는
    편리한 스크립트입니다.
    사용법: `./free-port.sh`

driver.sh
    Basic, Concurrency, Cache를 위한 자동 채점기입니다.
    사용법: `./driver.sh`

nop-server.py
    자동 채점기를 위한 보조 프로그램입니다.

tiny
    CS:APP 교재에 포함된 Tiny 웹 서버입니다.


CC=gcc
CPP=g++

CPP_FLAGS=-g -fpermissive

all:detect deamon default

detect:
	$(CPP) -o detect ${CPP_FLAGS} detect.cpp -lpthread 
deamon:
	$(CPP) -o deamon ${CPP_FLAGS} deamon.cpp -lpthread 

CPP_SRCS=TcpChatX.cpp dump.cpp

CLIENT_SRCS=client.cpp

SERVER_SRCS=server.cpp

INC_DIRS=-I.

LIB_DIRS=-L /E/msys64/mingw64/x86_64-w64-mingw32/lib
#LIB_DIRS=-L /E/msys64/mingw32/i686-w64-mingw32/lib
ifeq ($(shell uname), Linux)
	OS=Linux
else
	OS=Windows
endif

ifeq ($(OS),Windows)
	LIBS=-lpthread -lwsock32 -lws2_32 -lgdi32
endif

ifeq ($(OS),Linux)
	LIBS=-lpthread -ldl
endif


default: 
	$(CPP) -o cli ${CPP_FLAGS} ${INC_DIRS} ${LIB_DIRS} ${CPP_SRCS} ${CLIENT_SRCS} ${LIBS}
	$(CPP) -o svr ${CPP_FLAGS} ${INC_DIRS} ${LIB_DIRS} ${CPP_SRCS} ${SERVER_SRCS} ${LIBS}

clean:
	rm *.o cli svr detect deamon

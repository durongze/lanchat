all:detect default

detect:
	g++ -o detect detect.cpp -lpthread 

clean:
	rm detect


CPP_SRCS=TcpChatX.cpp

CLIENT_SRCS=client.cpp

SERVER_SRCS=server.cpp

INC_DIRS=-I.

LIB_DIRS=-L /E/msys64/mingw64/x86_64-w64-mingw32/lib
#LIB_DIRS=-L /E/msys64/mingw32/i686-w64-mingw32/lib

LIBS=-lpthread -lwsock32 -lws2_32 -lgdi32

CPP_FLAGS=-fpermissive

default: 
	g++ -o cli ${CPP_FLAGS} ${INC_DIRS} ${LIB_DIRS} ${CPP_SRCS} ${CLIENT_SRCS} ${LIBS}
	g++ -o svrr ${CPP_FLAGS} ${INC_DIRS} ${LIB_DIRS} ${CPP_SRCS} ${SERVER_SRCS} ${LIBS}
 
clean:
	rm *.o
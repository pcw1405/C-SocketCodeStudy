#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>
#pragma comment(lib,"ws2_32.lib")
#define BUFSIZE   1024
#define C2S_REQ_LOGIN  1001 // 로그인 요청 , 수신 코드
#define S2C_RES_LOGIN  2001 
#define CHAT_ROOMS  6
#define NAMESIZE 20

typedef struct ReqLogin
{
	char achID[20];
	char achPWD[20];
} ReqLogin;

int recvLoginPacket(SOCKET hSOCKET)
{
	char message[BUFSIZE];
	// 메시지를 초기화한다
	short code=0,size=0;
	// 패킷의 코드와 크기를 저장할 short형 변수를 각각 0으로 초기화합니다.
	int iret,result;
	//함수의 반환 값(수신된 바이트 수)과 최종 결과를 저장할 변수입니다.
	char buffer[BUFSIZE] = {0, };;
	
	//추가적인 데이터를 처리할 때 사용할 버퍼입니다. 이 코드에서는 사용되지 않지만 초기화되었습니다.
	
	// 최대 2 바이트의 크기를 받을 수 있는 메시지 수신
	iret=recv(hSOCKET,message,2,0);
	

}
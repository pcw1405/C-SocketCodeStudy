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
	
	if(iret<=0)
	{
		if(iret==-1)
			ErrorHanding("패킷 수신 에러!\n");
			//if(iret == -1): iret이 -1이면 패킷 수신 중 오류가 발생했음을 의미합니다.
		
		return -1;
		//return -1;: 함수는 -1을 반환하며 종료됩니다. 이는 패킷 수신 실패를 나타냅니다.
	}
	//if(iret <= 0): recv 함수가 0 이하를 반환하면 수신에 문제가 있는 것으로 판단합니다.
	else{
		memcpy(&code,&message[0],2);
		//수신한 첫 2바이트를 code 변수에 복사합니다. 이 code는 패킷의 유형을 나타냅니다.
		
		memset(&message,0x00,2);
		//memset(&message, 0x00, 2);: message 버퍼의 처음 2바이트를 0으로 초기화합니다.
		
		recv(hSOCKET,message,2,0);
		//다시 클라이언트로부터 2바이트의 데이터를 수신하여 message에 저장합니다. 이 데이터는 이후에 패킷의 크기를 나타냅니다.

		memcpy(&message[2],&size,2);
		//: 수신한 2바이트를 size 변수에 복사합니다. 이 size는 실제 데이터의 크기를 나타냅니다.

		memset(&message,0x00,2);
		//버퍼의 처음 2바이트를 다시 0으로 초기화합니다.


	}

	if(size>0)
		recv(hSOCKET,message,size,0);

	printf("데이터 수신 : code=%d,size=%d\n",code,size,0);
}
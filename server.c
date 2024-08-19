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


void checkLogin(SOCKET hSOCKET)
{
	while(1)
	{
		if(recvLoginPacket(hSOCKET)==0)
			break;
	}


}
//checkLogin(Socket) 함수는 클라이언트로부터
// 로그인 패킷을 계속해서 수신하고, 정상적으로 로그인 요청이 처리되면 루프를 종료하는 역할을 합니다.

int recvLoginPacket(SOCKET hSOCKET)
{
	char message[BUFSIZE];
	// 데이터를 저장할 버퍼를 선언합니다.
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
		//첫 번째 초기화는 code를 추출한 후, 버퍼의 처음 2바이트를 초기화하여 그 부분이 사용되지 않는다는 것을 명확히 합니다

		recv(hSOCKET,message,2,0);
		//다시 클라이언트로부터 2바이트의 데이터를 수신하여 message에 저장합니다. 이 데이터는 이후에 패킷의 크기를 나타냅니다.

		memcpy(&message[2],&size,2);
		//: 수신한 2바이트를 size 변수에 복사합니다. 이 size는 실제 데이터의 크기를 나타냅니다.

		memset(&message,0x00,2);
		//버퍼의 처음 2바이트를 다시 0으로 초기화합니다.
		//두 번째 초기화는 size를 추출한 후, 버퍼를 다시 초기화하여 이전 데이터를 제거하고 새로운 데이터를 받을 준비를 합니다. 

	}	//버퍼는 깨끗한상태로 만든다 

	if(size>0)
		recv(hSOCKET,message,size,0);


	// size가 0보다 크면, 실제 데이터를 수신하여 message 버퍼에 저장합니다.

	printf("데이터 수신 : code=%d,size=%d\n",code,size,0);
  // 수신된 코드와 데이터 크기를 출력합니다.

	if(code==C2S_REQ_LOGIN)
	{
		print("로그인 요청 수신\n");
		result=trxlogin(hSOCKET,message);
	// 패킷 코드가 로그인 요청이면 로그인 처리 함수를 호출합니다.
		return result;
	}
	 // 로그인 처리 결과를 반환합니다.
	else
	{
		printf("패킷 코드 오류\n");
		return -1;
		 // 패킷 코드가 로그인 요청이 아니면 오류 메시지를 출력하고 -1을 반환합니다.
	}
}
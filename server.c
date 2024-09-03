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
//채팅방의 최대 개수를 나타냅니다.
#define NAMESIZE 20


typedef struct ReqLogin
{
	char achID[20];
	char achPWD[20];
} ReqLogin;

typedef struct ResLogin
{
	int iResult;
} ResLogin;

typedef struct ChatClinets
{
	int clntNumber;
	//채팅방에 포함된 클라이언트의 수를 추적하는 데 사용됩니다.
	SOCKET clntSocks[10];
	//채팅방에 속한 클라이언트 소켓들을 저장합니다.
//이 배열에는 채팅방에 접속한 클라이언트들과의 연결이 저장됩니다.

} ChatUsers;



int clnCnt=0;
//현재 접속한 클라이언트의 수를 나타냅니다.
SOCKET clntSocks[10];
//이 배열은 최대 10개의 클라이언트 소켓을 저장할 수 있으며, 각각의 인덱스는 특정 클라이언트와의 연결을 나타냅니다. 
// 즉, 현재 연결된 클라이언트의 소켓들을 저장하는 데 사용됩니다.
ChatUsers chatUsers[CHAT_ROOMS];
//여러 개의 채팅방을 관리하기 위한 구조체 또는 클래스
//ChatUsers 객체는 특정 채팅방에 있는 사용자들을 관리하기 위해 사용

HANDLE hMutex;
//뮤텍스를 사용하면 한 스레드가 자원을 사용하는 동안 다른 스레드가 이 자원에 접근하지 못하게 할 수 있습니다.




DWORD WINAPI ClientConn(void *arg)
{
	SOCKET clnt_sock=(SOCKET)arg;

	//전달된 arg를 SOCKET 타입으로 캐스팅하여 clnt_sock 변수에 저장합니다. 이는 클라이언트와 통신하는 소켓을 나타냅니다.
	int str_len=0,i,room,flag;
	//SOCKET clnt_sock=(SOCKET)arg;
	//flag: 특정 조건을 확인하기 위해 사용됩니다.
	char message[BUFSIZE];
	//message: 클라이언트로부터 수신한 메시지를 저장할 버퍼입니다.

	while((str_len=recv(clnt_sock,message,BUFSIZE,0))!=EOF)
		SendMSG(clnt_sock,message,str_len);
	//recv 함수는 클라이언트 소켓으로부터 데이터를 읽고, 그 데이터를 message 버퍼에 저장한 후 그 길이를 str_len에 반환합니다. 

	//recv 함수는 클라이언트 소켓으로부터 데이터를 읽고, 그 데이터를 message 버퍼에 저장한 후 그 길이를 str_len에 반환합니다. 
	//수신된 데이터가 없을 때까지 (EOF가 반환될 때까지) 이 루프는 계속됩니다.

	WaitForSingleObject(hMutex,INFINITE);
	for(room=0;room<6;++room)
	{
		flag=0;
		for(i=0;i<chatUsers[room].clntNumber;++i)
		{
			if(clnt_sock==chatUsers[room].clntSocks[i])
			{
				flag=1;
				for(;i<chatUsers[room].clntNumber-1;++i)
					chatUsers[room].clntSocks[i] = chatUsers[room].clntSocks[i+1];


					break;
			}
			
		}

		if(flag==1) 
				break;


	}
	chatUsers[room].clntNumber--;

	ReleaseMutex(hMutex);
	closesocket(clnt_sock);

	return 0;
}


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


void SendMSG(SOCKET clnt_sock,char *message,int len)
{
    int i,flag=0,room=0; 
    //flag는 클라이언트 소켓이 어떤 방에 속하는지 확인하기 위한 플래그입니다.
    //room은 방의 인덱스를 나타냅니다.
    
    waitForSingleObject(hMutex,INFINITE);
    //이 명령은 여러 스레드가 동시에 접근할 수 없는 중요한 작업 영역(크리티컬 섹션)을 보호하는 데 사용됩니다. 
    //waitForSingleObject를 통해 뮤텍스를 잠금으로써, 
    //다른 스레드가 같은 자원에 동시에 접근하지 못하게 하고, 데이터 손상이나 충돌을 방지합니다.
    //이 함수는 뮤텍스가 해제될 때까지 무한 대기합니다
    
    for(room=0;room<6;++room)
    {    //이 루프는 각 방의 클라이언트 소켓을 검사하기 위해 사용됩니다.
        for(i=0; i<chatUsers[room].clntNumber;++i)
        { //    //현재 방(room)에 있는 클라이언트 소켓을 순회하는 for 루프를 시작합니다.
            if(clnt_sock==chatUsers[room].clntSocks[i]){
                //현재 클라이언트 소켓(clnt_sock)이 방의 클라이언트 소켓 배열(chatUsers[room].clntSocks[i])에 있는지 검사합니다.
                
                flag=1;
                break;
                //클라이언트 소켓이 일치하면 flag를 1로 설정하고, for 루프를 종료합니다 (break).
            }
        }
        
        if(flag==1)
            break;
            
            //flag가 1이면, 즉 클라이언트 소켓을 찾았으면, 방 인덱스 room의 for 루프도 종료합니다 (break).
    }
    

    
    for(i=0;i,chatUsers[room].clntNumber;++i)
        send(chatUsers[room].clntSocks[i],message,len,0)
    
    //해당 방에 있는 모든 클라이언트 소켓에 메시지를 전송합니다.
    ReleaseMutex(hMutex);
    //뮤텍스를 해제하여 다른 스레드가 자원에 접근할 수 있도록 합니다.
}

int trxLogin(SOCKET hSocket,char *buf){

	ReqLogin login;  // 로그인 요청을 위한 구조체 선언
	ResLogin res;   // 로그인 응답을 위한 구조체 선언
	FILE *fp1, *fp2; // 파일 포인터 선언 (아이디, 비밀번호 파일)
	char achID[20];  // 읽어온 아이디를 저장할 배열
	char achPWD[20];   // 읽어온 비밀번호를 저장할 배열
	char message[BUFSIZE]; // 메시지를 저장할 버퍼
	char buffer[20];   // 파일에서 읽어올 데이터를 임시 저장할 버퍼
	char cnt[20];      // 사용되지 않은 변수
	int iRes,size,iret,code=S2C_RES_LOGIN;  // 응답코드와 초기 변수 설정
	iRes=size=iret=0;  // 변수 초기화

 	// id.txt 파일을 읽기 모드로 엽니다.
	if((fp1=fopen("id.txt","r"))==NULL )
	{
		print("NO File\n"); // 파일이 없을 경우 메시지 출력
	}

	// pwd.txt 파일을 읽기 모드로 엽니다.
	if((fp2=fopen("pwd.txt","r"))==NULL)
	{
		printf("No field\n");   // 파일이 없을 경우 메시지 출력
	}


	// 로그인 요청 구조체에 buf 데이터를 복사합니다.
	memcpy(&login,buf,sizeof(login));


	// 로그인 시도한 아이디와 비밀번호를 출력합니다.
	printf("id: %s,password : %s",login.achID,login.achPWD);
	
	 // id.txt 파일에서 아이디를 찾는 루프
	while(fgets(buffer,sizeof(buffer),fp1))
	{
		// 파일에서 읽은 줄에 로그인 아이디가 포함되어 있는지 확인합니다.
		if(strstr(buffer,login.achID))
		{
			strcpy(achID,login.achID); // 일치하는 아이디를 achID에 저장
			break;  // 찾으면 루프 탈출
		}
	}

 	// pwd.txt 파일에서 비밀번호를 찾는 루프
	while(fgets(buffer,sizeof(buffer),fp2))
	{

		// 파일에서 읽은 줄에 로그인 비밀번호가 포함되어 있는지 확인합니다.
		if(strstr(buffer,login.achPWD))
		{
			strcpy(achPWD,login.achPWD); // 일치하는 비밀번호를 achPWD에 저장
			break;						// 찾으면 루프 탈출
		}

		
	}

	   // 파일을 닫습니다.
	fclose(fp1);
	fclose(fp2);

	 // 입력된 아이디와 비밀번호가 파일에서 찾은 것과 일치하는지 확인합니다.
	if(strcmp(achID,login.achID) !=0 || strcmp(achPWD,login.achPWD)!= 0)
		iRes=-1; // 일치하지 않으면 iRes를 -1로 설정

	res.iResult=iRes; // 결과값을 응답 구조체에 저장

	size=sizeof(res); // 응답 구조체의 크기를 size에 저장

	// 메시지 버퍼를 0으로 초기화합니다.
	memset(message,0x00,BUFSIZE);

	// 응답 코드를 메시지 버퍼의 처음 두 바이트에 복사합니다.
	memcpy(&message[0],&code,2);

	 // 응답 크기를 메시지 버퍼의 2번 인덱스에 복사합니다.
	memcpy(&message[2],&code,2);

	 // 응답 구조체를 메시지 버퍼의 4번 인덱스에 복사합니다.
	memcpy(&message[4],&res,size);

	// 메시지를 소켓을 통해 전송합니다.
	iret=send(hSocket,message,size+4,0);
	//메시지 전송(옵션x)

	if(iret<0)
		ErrorHandling("데이터를 보내지 못했습니다.");

	// 로그인 성공 시 (결과가 0일 때)
	if(res.iResult == 0 )
		chatRoomInfo(hSocket); // 채팅방 정보 함수를 호출합니다.
	
	return res.iResult; // 로그인 결과를 반환합니다.
}
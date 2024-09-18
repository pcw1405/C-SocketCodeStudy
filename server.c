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
    

    
    for(i=0;i<chatUsers[room].clntNumber;++i)
        send(chatUsers[room].clntSocks[i],message,len,0);
    
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
//이 함수는 클라이언트의 로그인 요청을 실제로 처리하는 함수입니다.


void chatRoomInfo(SOCKET hSocket)
{
	char buffer[BUFSIZE];
	//buffer라는 문자 배열을 선언하여 서버와 클라이언트 간에 데이터를 전송하는 데 사용됩니다.
	char *tableLine ="===========================\n";
	// tableLine은 출력 형식에서 채팅방 정보를 구분하는 데 사용되는 문자열입니다.

	sprintf(buffer,"%s| 채팅방1 : %d/2\t | 채팅방2: %d/2\t | 채팅방3 : %d/2\t |\%s",tableLine,chatUsers[0].clntNumber,chatUsers[1].clntNumber,chatUsers[2].clntNumber,tableLine);
	//sprintf 사용: sprintf 함수는 buffer에 문자열을 포맷하여 저장합니다.
	//여기서 tableLine과 채팅방의 사용자 수(chatUsers[x].clntNumber)를 포함하여 출력 형식을 만듭니다.
	send(hSocket,buffer,strlen(buffer),0);
	//send 함수 호출: send 함수는 hSocket을 통해 buffer에 저장된 데이터를 클라이언트에게 전송합니다. 
	//strlen(buffer)는 전송할 데이터의 길이를 계산합니다. 
	//0은 전송 플래그를 나타내며, 기본적인 전송을 의미합니다.
	
	sprintf(buffer,"%s| 채팅방4 : %d/2\t | 채팅방5: %d/2\t | 채팅방6 : %d/2\t |\%s",tableLine,chatUsers[0].clntNumber,chatUsers[4].clntNumber,chatUsers[5].clntNumber,tableLine);
	send(hSocket,buffer,strlen(buffer),0);

}
//이 코드는 클라이언트에게 현재 채팅방의 상태 정보를 보내는 기능을 수행합니다.
//chatRoomInfo 함수는 두 개의 메시지를 생성하여 각 채팅방의 사용자 수를 포함하고, 
//이 정보를 소켓을 통해 클라이언트에 전송합니다
// 메시지는 채팅방의 사용자 수와 최대 수를 포맷하여 문자열로 만들고, 이를 클라이언트에게 전송합니다.



void initMutex()
{
	hMutex=CreateMutex(NULL,FALSE,NULL);
	//CreateMutex 함수는 새로운 뮤텍스 객체를 생성하고, 생성된 뮤텍스의 핸들을 hMutex 변수에 저장합니다.
	//첫 번째 인자 NULL: 보안 속성(security attributes)을 지정하지 않아 기본 보안 설정을 사용합니다.
	//두 번째 인자 FALSE: 뮤텍스가 처음 생성될 때 잠겨있지 않도록 설정합니다. 즉, 뮤텍스를 소유하고 있는 스레드가 없습니다.
	//세 번째 인자 NULL: 뮤텍스의 이름을 지정하지 않기 때문에 이름 없는(익명) 뮤텍스를 생성합니다
	if(hMutex==NULL)
		ErrorHandling("CreateMutex() Error!");
	// CreateMutex 함수가 실패하면 NULL을 반환합니다. 이 조건문은 뮤텍스 생성이 실패했는지 확인하는 역할을 합니다.

}
//뮤텍스를 초기화하기 위해 사용됩니다.
//이 코드는 멀티스레드 환경에서 자원에 대한 동시 접근을 제어하기 위해 뮤텍스를 생성하며,
//  생성 실패 시 적절한 오류 처리를 통해 프로그램의 안정성을 높이는 중요한 역할을 합니다.

//뮤텍스(Mutex)는 멀티스레드 환경에서 여러 스레드가 동시에 공유 자원에 접근하는 것을 막기 위해 사용되는 도구
//쉽게 말해, 여러 사람이 동시에 한 자원을 사용하려고 할 때, 순서를 정해 하나씩 사용하도록 하는 잠금 장치

void windllLoad(WSADATA *wsaData)
{
//WSADATA 구조체는 WSAStartup 함수가 실행될 때 초기화 결과를 반환하는 데 사용됩니다. 
//WSADATA 구조체 포인터를 인자로 받는 함수입니다. 이 구조체는 Windows 소켓 API의
// 초기화 과정에서 소켓의 버전 정보와 같은 초기화 결과를 저장하는 역할을 합니다.


	if(WSAStartup(MAKEWORD(2,2),wsaData)!=0)
		ErrorHandling("WSAStartup");

		//WSAStartup 함수는 Windows에서 소켓 라이브러리를 초기화하는 함수로, 
		//소켓 프로그래밍을 시작하기 전에 반드시 호출해야 합니다.
		//WSAStartup 함수는 Windows에서 소켓 라이브러리를 초기화하는 함수로, 
		//소켓 프로그래밍을 시작하기 전에 반드시 호출해야 합니다.
	//초기화가 성공적으로 완료된 후에는 WSACleanup 함수를 호출하여 
	//소켓 라이브러리를 해제해야 합니다
}
//이 코드는 Windows 소켓 프로그래밍에서 네트워크 통신을 시작하기 위해 WSAStartup 함수를 호출하는 C 언어 함수입니다.
//Winsock(Windows Sockets) 라이브러리는 Windows 운영 체제에서 네트워크 프로그래밍을 위해 소켓을 사용하여 TCP/IP 통신을 할 수 있게 해주는 API입니다.


//이 함수는 TCP 통신을 위해 서버 소켓을 생성
//서버 소켓과 서버의 주소 구조체를 초기화하고,
//, 주어진 포트 번호로 서버 소켓을 설정합니다.
//서버 소켓을 생성한 후, 그 소켓을 바인딩할 IP 주소와 포트 번호를 설정하는 과정입니다.
//socket() 함수로 소켓을 생성하고, 주소 체계(AF_INET), IP 주소(INADDR_ANY), 포트 번호를 설정한 후
// 서버가 클라이언트의 요청을 수락할 준비를 합니다.
void ServInit(SOCKET *serv_sock,SOCKADDR_IN *serv_addr,int port){

	*serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	//이 부분은 소켓을 생성하는 코드입니다 //, SOCK_STREAM은 TCP 통신을 의미합니다.
	//PF_INET은 IPv4 프로토콜을 사용하고, 
	//socket() 함수는 소켓을 생성한 후, 그 값을 포인터 serv_sock에 저장합니다.
	if(*serv_sock==INVALID_SOCKET)
			ErrorHandling("socket() ERROR!");
	//만약 소켓 생성에 실패하면, INVALID_SOCKET 값을 반환합니다.
	//ErrorHandling("socket() ERROR!");을 호출하여 에러 메시지를 출력하고, 
	//적절한 오류 처리를 수행합니다.
	serv_addr->sin_family=AF_INET;
	// sin_family 필드는 주소 체계(address family)를 나타내며, AF_INET은 IPv4 주소 체계를 의미합니다.
	serv_addr->sin_addr.s_addr=hotel(INADDR_ANY);
	//서버의 IP 주소를 설정하는 부분입니다.
	//INADDR_ANY는 "모든 IP 주소"를 의미하며, 
	//htonl()은 호스트 바이트 순서를 네트워크 바이트 순서로 변환하는 함수입니다.
	//즉, 서버가 여러 개의 IP 주소를 가질 경우, 이 설정은 모든 네트워크 인터페이스에서 요청을 받을 수 있도록 합니다.
	serv_addr->sin_port=htons((unsigned short int)port);
	//서버 소켓이 사용할 포트 번호를 설정하는 부분입니다.

	//serv_addr는 SOCKADDR_IN 구조체의 포인터로, 
	//서버의 주소 정보를 담고 있습니다.
}

//오류 메시지를 출력하고 프로그램을 종료하는 역할을 합니다. 
//ErrorHandling 함수의 각 부분을 살펴보면 다음과 같습니다:

void ErrorHandling(char *message){

	//메시지 즉석 출력 가능 
	fputs(message,stderr);
	// 표준 오류 출력(스트림)인 stderr로 출력합니다.
	// stderr는 에러 메시지나 디버그 메시지를 출력할 때 사용하는 표준 출력 스트림입니다. 
	// 이를 통해 오류 메시지가 콘솔에 출력됩니다.
	fputs('\n',stderr);
	//fputc() 함수는 단일 문자를 출력하는 함수입니다.
	exit(-1);
	//exit() 함수는 프로그램을 종료하는 함수입니다.

}
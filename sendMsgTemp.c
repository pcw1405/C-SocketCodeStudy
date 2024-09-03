/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

// 클라이언트 소켓을 통해 메시지를 여러 클라이언트에게 전송하는 기능을 구현하는 함수입니다.
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
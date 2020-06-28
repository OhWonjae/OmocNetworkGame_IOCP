#include "ServerNetwork.h"
#include<memory>
HANDLE g_hIocp;
WSADATA wsaData;
//host socket 정보
SOCKET hsock;
sockaddr_in servaddr;
PlayerRooms* PRooms;
void InitOmocArray(Dol(*omocarray)[YCOUNT][XCOUNT])
{
	for (int i = 0; i < YCOUNT; i++)
	{
		for (int j = 0; j < XCOUNT; j++)
		{
			(*omocarray)[i][j].isPuton = false;
			(*omocarray)[i][j].Color = WHITE;
		}
	}
}
int OmocCount(Dol Dols[YCOUNT][XCOUNT], int x, int y, int color)
{
	int Rcount = 0;
	int RUcount = 0;
	int RDcount = 0;
	int Dcount = 0;
	int Ucount = 0;
	int Lcount = 0;
	int LUcount = 0;
	int LDcount = 0;
	int ix = (x - X_START) / INTERVAL;
	int iy = (y - Y_START) / INTERVAL;
	if (((x - X_START) % INTERVAL) > H_INTERVAL)
	{
		ix += 1;
	}
	if (((y - Y_START) % INTERVAL) > H_INTERVAL)
	{


		iy += 1;
	}
	//x,y 실제 좌표를 ix, iy로 변환시켜주기
	Dols[iy][ix].isPuton = TRUE;
	Dols[iy][ix].Color = color;
	printf("(%d ,%d )\n", ix, iy);

	for (int i = 1; i < 5; i++)
	{
		// 오른쪽으로 만족
		if (ix + i < XCOUNT&&Dols[iy][ix + i].isPuton == TRUE && Dols[iy][ix + i].Color == color)
		{
			Rcount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{

		//오른쪽 위로 만족
		if (ix + i < XCOUNT&&iy + i < YCOUNT&&Dols[iy + i][ix + i].isPuton == TRUE && Dols[iy + i][ix + i].Color == color)
		{
			RUcount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{

		//오른쪽 아래로 만족
		if (ix + i < XCOUNT&&iy - i >= 0 && Dols[iy - i][ix + i].isPuton == TRUE && Dols[iy - i][ix + i].Color == color)
		{
			RDcount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{

		//아래쪽으로 만족
		if (iy + i < YCOUNT&&Dols[iy + i][ix].isPuton == TRUE && Dols[iy + i][ix].Color == color)
		{
			Dcount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{
		//위쪽으로 만족
		if (iy - i >= 0 && Dols[iy - i][ix].isPuton == TRUE && Dols[iy - i][ix].Color == color)
		{
			Ucount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{
		//왼쪽위쪽으로 만족
		if (ix - i >= 0 && iy - i >=0 && Dols[iy - i][ix - i].isPuton == TRUE && Dols[iy - i][ix - i].Color == color)
		{
			LUcount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{
		//왼쪽으로 만족
		if (ix - i >= 0 && Dols[iy][ix - i].isPuton == TRUE && Dols[iy][ix - i].Color == color)
		{
			Lcount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{
		//왼쪽아래쪽으로 만족
		if (ix - i >= 0 && iy + i < YCOUNT&&Dols[iy + i][ix - i].isPuton == TRUE && Dols[iy + i][ix - i].Color == color)
		{
			LDcount++;
		}
		else break;
	}
	
	printf("R%d,L%d,U%d,D%d,LU%d,RD%d,LD%d,RU%d", Rcount, Lcount, Ucount, Dcount, LUcount, RDcount, LDcount, RUcount);
	if (Rcount+Lcount >= 4 || Ucount + Dcount >= 4 || LUcount +RDcount >= 4 || LDcount + RUcount >= 4 )
	{

		printf("omoc5!!\n");
		return 1;
	}
	else return -1;

}

// 동적할당
void Init()
{
	PRooms = new PlayerRooms();

}

void RoomClear(SOCKET sock)
{
	//1. 그방 플레이어들에게 disconnet 메시지 보내기
	Room* room =PRooms->GetPalyerRoom(sock);
	room->BroadCastMessage(PT_Disconnect);
	//2. 방 비워주기
	PRooms->RoomClear(sock);
}


//void Init()
//{
//	delete(inputstream);
//	delete(PRooms);
//	delete(outputstream);
//	delete(InHelloData) ;
//	delete(OutHelloData);
//	delete(InMouseData) ;
//	delete(OutStateData);
//	closesocket(hsock);
//	WSACleanup();
//}

int ServerNetworkSetting()
{
	//wsaStartup
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAError!\n");
		WSACleanup();
		return -1;
	}
	// 소켓생성
	hsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hsock == SOCKET_ERROR)
	{
		printf("socket Create Error!\n");
		shutdown(hsock, SD_BOTH);
		closesocket(hsock);
		WSACleanup();
		return -1;
	}
	//서버주소세팅
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
	//bind
	if (bind(hsock, (sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR)
	{
		printf( "binderror! : %d\n", WSAGetLastError());
		shutdown(hsock, SD_BOTH);
		closesocket(hsock);
		WSACleanup();
		return -1;
	}
	//listen
	if (listen(hsock, 20) == SOCKET_ERROR)
	{
		printf("listen Error!\n");
		shutdown(hsock, SD_BOTH);
		closesocket(hsock);
		WSACleanup();
		return -1;
	}
	return 1;
}
unsigned int __stdcall Thread_func(void* parm)
{
	DWORD dwNumofByte = 0;
	Per_Handle_Data* HandleData;
	Per_IO_Data* IOData;
	while (true)
	{	

		int ret = GetQueuedCompletionStatus(g_hIocp,&dwNumofByte,(PULONG_PTR)&HandleData,(LPOVERLAPPED*)&IOData,INFINITE);
		if (ret==0 && GetLastError()==WAIT_TIMEOUT)
		{
			shutdown(HandleData->fd, SD_BOTH);

			closesocket(HandleData->fd);
			printf("GQCS ERROR!\n");
			delete(HandleData);
			delete(IOData);
			continue;
		}
		else if (dwNumofByte == 0)
		{
			printf("%dClient Out!\n", HandleData->fd);

			shutdown(HandleData->fd, SD_BOTH);
			closesocket(HandleData->fd);			
			delete(HandleData);
			delete(IOData);
			continue;
		}
		if (IOData->OperType==RECV_DATA)
		{
			InputStream inputstream;
			InMOUSEDATA InMouseData;
			InCHATDATA InChatData;
			InDISCONNECT InDisconnect;
			PacketType Ptype;			 
			Room* room = PRooms->GetPalyerRoom(HandleData->fd);
			Player* player = PRooms->GetPlayer(HandleData->fd);
			INT32 playercolor = PRooms->GetPlayerSeatNumber(HandleData->fd);
			memcpy_s(&Ptype, sizeof(INT32), IOData->wsabuf.buf, sizeof(INT32));
			switch (Ptype)
			{
			case PT_GameData:

				// 마우스데이터 받기
				InMouseData.Read(&inputstream, IOData->wsabuf.buf, IOData->wsabuf.len);
				printf("Mouse Packe Recv (%d, %d) From %d Client\n", InMouseData.GetX(), InMouseData.GetY(), HandleData->fd);
				// 게임 데이터 보내기
				//omocarry 룸에 넣기고 확인
				if (OmocCount(room->OmocArray, InMouseData.GetX(), InMouseData.GetY(), playercolor) == 1)
				{
					// 승부 났으면 해당 방의 승리 색깔넣어주기
					room->WinPlayerColor = playercolor;
				}
				
				if (room->BroadCastMessage(Ptype) == -1)
				{
					printf("Send GameData BroadcastMSG Error !\n");
					RoomClear(HandleData->fd);
				}
				break;
			case PT_ChatData:
				//채팅데이터받기
				InChatData.Read(&inputstream, IOData->wsabuf.buf, IOData->wsabuf.len);
				printf("Chatting Packe Recv (%s :  %s) From %d Client\n",player->Name, InChatData.GetChat(), HandleData->fd);
				//해당 룸에 채팅데이터 보내기
				memcpy_s(room->SendPlayerName, sizeof(room->SendPlayerName), player->Name, sizeof(player->Name));
				memcpy_s(room->ChatBuffer,sizeof(room->ChatBuffer), InChatData.GetChat(),strlen(InChatData.GetChat()));
				if (room->BroadCastMessage(Ptype) == -1)
				{
					printf("Send ChatData BroadcastMSG Error !\n");
					RoomClear(HandleData->fd);
					
				}
				break;
			case PT_Disconnect:
				//disconnect데이터 받기
				InDisconnect.Read(&inputstream, IOData->wsabuf.buf, IOData->wsabuf.len);
				printf("Disconnect Packe Recv from %d Client\n", HandleData->fd);
				//룸에서 지워주기 - > Roomclear안에 disconnect 보내는거 있음
				RoomClear(HandleData->fd);
			default:
				break;
			}
		}
		else
		{
			printf("WSA SEND To %d Success!\n", HandleData->fd);
		}
	}
	return 0;
}
//방다 찼는지 확인해주는 스레드 -> 다찬방 있으면 그방 인원들에게 START 패킷 뿌려주기
unsigned int __stdcall FIndFullRoomThread(void* parm)
{
	OutSTARTDATA OutStartData;
	OutputStream outputstream;
	std::vector<Room*> FullRoomVec;
	std::vector<Room*>::iterator it;
	while (true)
	{
		FullRoomVec = PRooms->GetFullRoom();
		int s = FullRoomVec.size();
		for (it=FullRoomVec.begin(); it!=FullRoomVec.end();)
		{
			if ((*it)->BroadCastMessage(PT_Start) == -1)
			{
				break;
			}
			it = FullRoomVec.erase(it);
		}
	}
	return 0;
}
HANDLE OnBtnClickedFindRoomThread()
{
	return CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)FIndFullRoomThread, NULL, NULL, NULL);
}
HANDLE OnBtnClickedThread(HANDLE* handle)
{
	return CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Thread_func, handle, NULL, NULL);
}
int main()
{

	if (ServerNetworkSetting() != -1)
	{
		Init();
		// CPU코어 *2 개의 스레드 생성
		SYSTEM_INFO Systeminfo;
		GetSystemInfo(&Systeminfo);
		INT32 nThreadCount = Systeminfo.dwNumberOfProcessors * 2+1;
		// 쓰레드 할당
		HANDLE* hThread = (HANDLE*)calloc(nThreadCount, sizeof(HANDLE));

		Per_Handle_Data* data = new Per_Handle_Data();
		data->fd = 2;
		//iocp만들기
		g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL,NULL, 0);
		OnBtnClickedFindRoomThread();

		for (int i = 0; i < nThreadCount; i++)
		{
			hThread[i] = OnBtnClickedThread(&g_hIocp);
		}
		while (true)
		{
			sockaddr_in clientaddr;
			int addrlen = sizeof(clientaddr);
			SOCKET ClientSocket = accept(hsock, (sockaddr*)&clientaddr, &addrlen);
			if (ClientSocket== INVALID_SOCKET)
			{
				printf("Client Accept error!\n");
				shutdown(ClientSocket, SD_BOTH);
				closesocket(ClientSocket);
				continue;
			}

			// Hello 받기
			char HrecvBuffer[HRECVBUFFERLEN];
			int sendnum = recv(ClientSocket, HrecvBuffer, sizeof(HrecvBuffer), 0);
			if (sendnum < 0)
			{
				printf("Hello Send Error To Client(%d)\n", ClientSocket);
				shutdown(ClientSocket, SD_BOTH);
				closesocket(ClientSocket);
				continue;
			}
			InputStream Inputstream;
			InHelloDATA InHelloData;
			InHelloData.Read(&Inputstream,HrecvBuffer,sizeof(HrecvBuffer));
			char* ClientName = InHelloData.GetNameData();
			Per_Handle_Data* HandleData = new Per_Handle_Data();
			Per_IO_Data* IOData = new Per_IO_Data();
			HandleData->fd = ClientSocket;
			HandleData->servaddr = &clientaddr;
			IOData->InitData();
			Player* player = new Player(HandleData, ClientName, FALSE);
			PRooms->SetPlayer(player);
			OutputStream outputstream;
			OutHelloDATA OutHelloData;
			OutHelloData.SetData(ClientSocket, PRooms->GetRoomNumber(ClientSocket), PRooms->GetPlayerSeatNumber(ClientSocket));
			OutHelloData.Write(&outputstream);

			//에러에 플레이어 아웃 (플레이어 클래스, 핸들 클래스 다 해제 )
			sendnum = send(ClientSocket, outputstream.GetBuffer(), outputstream.GetLength(),0);
			if (sendnum < 0)
			{
				printf("Hello Send Error To Client(%d)\n", ClientSocket);
				delete(player);
				shutdown(ClientSocket, SD_BOTH);
				closesocket(ClientSocket);
				continue;
			}
			printf("Hello %d's Player[%s] \n\n", ClientSocket, player->Name);
			
			CreateIoCompletionPort((HANDLE)ClientSocket, g_hIocp, (ULONG_PTR)HandleData, 0);
			
			IOData->OperType = RECV_DATA;
			if (WSARecv(HandleData->fd, &IOData->wsabuf, 1, &IOData->wsabuf.len, (LPDWORD)&IOData->flags, (OVERLAPPED*)&IOData->overlapped, NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSA_IO_PENDING)
				{
					printf("WSA Recv Success To Client(%d)\n", ClientSocket);
				}
				else
				{
					printf("WSA Recv Error To Client(%d) %d\n", ClientSocket,WSAGetLastError());
					delete(player);
					shutdown(ClientSocket, SD_BOTH);
					closesocket(ClientSocket);
					continue;
				}
			}


		}

	}
}

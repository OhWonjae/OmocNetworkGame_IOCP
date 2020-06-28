#include "ServerNetwork.h"
#include<memory>
HANDLE g_hIocp;
WSADATA wsaData;
//host socket ����
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
	//x,y ���� ��ǥ�� ix, iy�� ��ȯ�����ֱ�
	Dols[iy][ix].isPuton = TRUE;
	Dols[iy][ix].Color = color;
	printf("(%d ,%d )\n", ix, iy);

	for (int i = 1; i < 5; i++)
	{
		// ���������� ����
		if (ix + i < XCOUNT&&Dols[iy][ix + i].isPuton == TRUE && Dols[iy][ix + i].Color == color)
		{
			Rcount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{

		//������ ���� ����
		if (ix + i < XCOUNT&&iy + i < YCOUNT&&Dols[iy + i][ix + i].isPuton == TRUE && Dols[iy + i][ix + i].Color == color)
		{
			RUcount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{

		//������ �Ʒ��� ����
		if (ix + i < XCOUNT&&iy - i >= 0 && Dols[iy - i][ix + i].isPuton == TRUE && Dols[iy - i][ix + i].Color == color)
		{
			RDcount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{

		//�Ʒ������� ����
		if (iy + i < YCOUNT&&Dols[iy + i][ix].isPuton == TRUE && Dols[iy + i][ix].Color == color)
		{
			Dcount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{
		//�������� ����
		if (iy - i >= 0 && Dols[iy - i][ix].isPuton == TRUE && Dols[iy - i][ix].Color == color)
		{
			Ucount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{
		//������������ ����
		if (ix - i >= 0 && iy - i >=0 && Dols[iy - i][ix - i].isPuton == TRUE && Dols[iy - i][ix - i].Color == color)
		{
			LUcount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{
		//�������� ����
		if (ix - i >= 0 && Dols[iy][ix - i].isPuton == TRUE && Dols[iy][ix - i].Color == color)
		{
			Lcount++;
		}
		else break;
	}
	for (int i = 1; i < 5; i++)
	{
		//���ʾƷ������� ����
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

// �����Ҵ�
void Init()
{
	PRooms = new PlayerRooms();

}

void RoomClear(SOCKET sock)
{
	//1. �׹� �÷��̾�鿡�� disconnet �޽��� ������
	Room* room =PRooms->GetPalyerRoom(sock);
	room->BroadCastMessage(PT_Disconnect);
	//2. �� ����ֱ�
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
	// ���ϻ���
	hsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hsock == SOCKET_ERROR)
	{
		printf("socket Create Error!\n");
		shutdown(hsock, SD_BOTH);
		closesocket(hsock);
		WSACleanup();
		return -1;
	}
	//�����ּҼ���
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

				// ���콺������ �ޱ�
				InMouseData.Read(&inputstream, IOData->wsabuf.buf, IOData->wsabuf.len);
				printf("Mouse Packe Recv (%d, %d) From %d Client\n", InMouseData.GetX(), InMouseData.GetY(), HandleData->fd);
				// ���� ������ ������
				//omocarry �뿡 �ֱ�� Ȯ��
				if (OmocCount(room->OmocArray, InMouseData.GetX(), InMouseData.GetY(), playercolor) == 1)
				{
					// �º� ������ �ش� ���� �¸� ����־��ֱ�
					room->WinPlayerColor = playercolor;
				}
				
				if (room->BroadCastMessage(Ptype) == -1)
				{
					printf("Send GameData BroadcastMSG Error !\n");
					RoomClear(HandleData->fd);
				}
				break;
			case PT_ChatData:
				//ä�õ����͹ޱ�
				InChatData.Read(&inputstream, IOData->wsabuf.buf, IOData->wsabuf.len);
				printf("Chatting Packe Recv (%s :  %s) From %d Client\n",player->Name, InChatData.GetChat(), HandleData->fd);
				//�ش� �뿡 ä�õ����� ������
				memcpy_s(room->SendPlayerName, sizeof(room->SendPlayerName), player->Name, sizeof(player->Name));
				memcpy_s(room->ChatBuffer,sizeof(room->ChatBuffer), InChatData.GetChat(),strlen(InChatData.GetChat()));
				if (room->BroadCastMessage(Ptype) == -1)
				{
					printf("Send ChatData BroadcastMSG Error !\n");
					RoomClear(HandleData->fd);
					
				}
				break;
			case PT_Disconnect:
				//disconnect������ �ޱ�
				InDisconnect.Read(&inputstream, IOData->wsabuf.buf, IOData->wsabuf.len);
				printf("Disconnect Packe Recv from %d Client\n", HandleData->fd);
				//�뿡�� �����ֱ� - > Roomclear�ȿ� disconnect �����°� ����
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
//��� á���� Ȯ�����ִ� ������ -> ������ ������ �׹� �ο��鿡�� START ��Ŷ �ѷ��ֱ�
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
		// CPU�ھ� *2 ���� ������ ����
		SYSTEM_INFO Systeminfo;
		GetSystemInfo(&Systeminfo);
		INT32 nThreadCount = Systeminfo.dwNumberOfProcessors * 2+1;
		// ������ �Ҵ�
		HANDLE* hThread = (HANDLE*)calloc(nThreadCount, sizeof(HANDLE));

		Per_Handle_Data* data = new Per_Handle_Data();
		data->fd = 2;
		//iocp�����
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

			// Hello �ޱ�
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

			//������ �÷��̾� �ƿ� (�÷��̾� Ŭ����, �ڵ� Ŭ���� �� ���� )
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

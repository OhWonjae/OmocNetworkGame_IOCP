
#include "CclientSocket.h"

int Debug = 0;
///////
//RecvPointData(Dol rdols[YCOUNT][XCOUNT]);->�����κ��� �� �迭 �޾ƿ���
//RecvChatData(char*& RecvChatData); -> �����κ��� ä�� ���� �޾ƿ���
//void SetTurn(bool turn); -> ������ ���� ���� turn  ����
///////
//�÷��̾� �̸�
  char PlayerName[20];
  
//�÷��̾� ��ȣ
 INT32 PlayerNumber = -1;
//�÷��̾� ���ȣ
 INT32 PlayerRoomNumber = -1;
//�÷��̾� ����
 INT32 PlayerColor = 0;
//�÷��̾� ��������
 ResultType PlayerResultType = RT_Playing;
//��� �÷��̾� �̸�
 char OtherPlayerName[20];
 //ä�� ���� �÷��̾� �̸�
 char SendPlayerName[20];
// �÷��̾� ������
 BOOL PlayerTurn;
//���� ä�ù���
 char SendChatBuffer[255];
//���� ä�ù���
 char RecvChatBuffer[255];
// ���ӽ�������
 BOOL IsGameStart = FALSE;
// �������
 Dol Dols[YCOUNT][XCOUNT];

int CclientSocket::SocketSetting()
{
	//wsadata ����
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
	{
		PopLog("WSAData errro!");
		return -1;
	}
	//���ϻ���
	hsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hsock==SOCKET_ERROR)
	{
		PopLog("Socket erro!");
		WSACleanup();
		return -1;
	}
	//�����ּҼ���
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &servaddr.sin_addr);
	servaddr.sin_port =htons( PORT);
	//����
	if (connect(hsock, (sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR)
	{
		PopLog("SOCKET CONNECT ERROR!");
		closesocket(hsock);
		WSACleanup();
		return -1;
	}
}

// Socket ���� + ù �������� �Ѱ��ְ� �����κ��� �÷��̾�ѹ��� ���ȣ �ޱ�
CclientSocket::CclientSocket()
{
	OutChatData = new OutCHATDATA();
	InStateData = new InSTATEDATA();
	OutMouseData = new OutMOUSEDATA();
	OutHelloData = new OutHelloDATA();
	InHelloData = new InHelloDATA();
	InStartData = new InSTARTDATA();
	InChatData = new InCHATDATA();
	OutStream = new OutputStream();
	inputstream = new InputStream();
	IndisconnData = new InDISCONNECT();
	OutDisconnect = new OutDISCONNECT();
	if (SocketSetting() != -1)
	{
		char buffer[100];
		wsprintf(buffer,"hsock : %d", hsock);
		PopLog(buffer);
		OutHelloData->SetData(PlayerName);
		OutHelloData->Write(OutStream);  //���Ⱑ ����
		int sendnum = send(hsock, OutStream->GetBuffer(), OutStream->GetLength(), 0);
		if (sendnum == SOCKET_ERROR)
		{
			PopLog("First Name Send Error!");
			return;
		}
		char HrecvBuffer[HRECVBUFFERLEN] = {0};
		int recvnum = recv(hsock, HrecvBuffer, sizeof(HrecvBuffer),0);
		if (recvnum<0)
		{
			PopLog("HelloData Recv Error!");
			return;
		}
		// �÷��̾� ��ȣ�� ���ȣ, �������� Ȯ�� ����
		InHelloData->Read(inputstream, HrecvBuffer, recvnum);
		PlayerRoomNumber = InHelloData->GetRoomNumber();
		PlayerColor = InHelloData->GetPlayerColor();
		wsprintf(buffer,"Connect! Playernumber : %d, RoomNumber : %d, Color : %d", PlayerNumber, PlayerRoomNumber,PlayerColor);
		PopLog(buffer);
	}
}
//���� ���� ����
CclientSocket::~CclientSocket()
{
//	delete(InStateData);
//	delete(OutMouseData);
//	delete(OutHelloData);
//	delete(InHelloData);
//	delete(InStartData);
//	delete(InChatData);
//	delete(OutChatData);
//	delete(OutStream);
//	delete(inputstream);
//	shutdown(hsock, SD_BOTH);
	closesocket(hsock);
	WSACleanup();
}

//Ŭ�󿡼� ������ ����Ʈ���� ������
void CclientSocket::SendPoint(INT32 x, INT32 y)
{
	OutputStream* OutStream = new OutputStream();
	OutMouseData->SetData(x, y);
	OutMouseData->Write(OutStream);
	int sendnum = send(hsock, OutStream->GetBuffer(), OutStream->GetLength(), 0);
	if (sendnum <0)
	{
		PopLog("MousePoint Send Error!");
		return;
	}
	//�÷��̾��� ���ֱ�
	PlayerTurn = FALSE;
	/*char buffer[100];
	wsprintf(buffer, "Client %d send MouseINfo! (%d %d)", PlayerNumber,x,y );
	PopLog(buffer);*/
}
// �������� �� State���� �ް� 1�����迭�� ����迭�� 2�������� ��ȯ���� ����
void CclientSocket::RecvState(char HrecvBuffer[HRECVBUFFERLEN])
{

	InStateData->Read(inputstream,HrecvBuffer,HRECVBUFFERLEN);
	PlayerResultType = InStateData->GetResultType();
	PlayerTurn = InStateData->GetTurn();
	// ����迭 ��ȯ
	Dol* omocArray = InStateData->GetOmocArray();
	int index = 0;
	for (int i = 0; i < YCOUNT; i++)
	{
		for (int j = 0; j < XCOUNT; j++)
		{
			Dols[i][j] = omocArray[index];
			index++;
		}
	}
	
}
// Ŭ�󿡼� ������ ä������ ������
void CclientSocket::SendChatting()
{
	PopLog("Send Chat!");
	OutChatData->SetData(SendChatBuffer);
	OutChatData->Write(OutStream);
		int sendnum = send(hsock, OutStream->GetBuffer(), OutStream->GetLength(), 0);
		if (sendnum == SOCKET_ERROR)
		{
			PopLog("Chatting Send Error!");
			return;
		}
}
void CclientSocket::RecvStart(char HrecvBuffer[HRECVBUFFERLEN])
{
	InStartData->Read(inputstream, HrecvBuffer, HRECVBUFFERLEN);
	//char buffer[100];
	//wsprintf(buffer,"Startpacket Recv! Other name : %s, My turn : %d", OtherPlayerName,PlayerTurn);
	//PopLog(buffer);
}
void CclientSocket::RecvChat(char HrecvBuffer[HRECVBUFFERLEN])
{
	InChatData->Read(inputstream, HrecvBuffer, HRECVBUFFERLEN);
}
void CclientSocket::RecvDisconn(char HrecvBuffer[HRECVBUFFERLEN])
{
	IndisconnData->Read(inputstream, HrecvBuffer, HRECVBUFFERLEN);

}
void CclientSocket::SendDisconnect()
{
	PopLog("Send Disconnect!");
	OutDisconnect->Write(OutStream);
	int sendnum = send(hsock, OutStream->GetBuffer(), OutStream->GetLength(), 0);
	if (sendnum == SOCKET_ERROR)
	{
		PopLog("Disconnect Send Error!");
		return;
	}

}
//�� �Լ��� �޾ƿ� �������� ����� ���� recv������ ����
int  CclientSocket::RecvData()
{

	char HrecvBuffer[HRECVBUFFERLEN];
	memset(HrecvBuffer, 0, sizeof(HrecvBuffer));
	int recvnum = recv(hsock, HrecvBuffer, sizeof(HrecvBuffer), 0);
	if (recvnum < 0)
	{
		PopLog("Recv State From Server Error!");
		return -1;
	}
	INT32 PtypeData;
	memcpy_s(&PtypeData, sizeof(INT32), HrecvBuffer, sizeof(INT32));
	switch (static_cast<PacketType>(PtypeData))
	{
	case PT_Hello:
		// Hello�� ���� �����忡�� ����
		break;
	case PT_Start:
		RecvStart(HrecvBuffer);
		IsGameStart = TRUE;
		break;
	case PT_GameData:
		RecvState(HrecvBuffer);
		break;
	case PT_ChatData:
		RecvChat(HrecvBuffer);
		UpdateDrawOutText();
		break;
	case PT_Disconnect:
		RecvDisconn(HrecvBuffer);
		return -1;
		break;
	default:
		break;
	}
	InvalideRectFunc();
}

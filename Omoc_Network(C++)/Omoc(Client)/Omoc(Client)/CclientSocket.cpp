
#include "CclientSocket.h"

int Debug = 0;
///////
//RecvPointData(Dol rdols[YCOUNT][XCOUNT]);->서버로부터 돌 배열 받아오기
//RecvChatData(char*& RecvChatData); -> 서버로부터 채팅 정보 받아오기
//void SetTurn(bool turn); -> 서버로 부터 받은 turn  셋팅
///////
//플레이어 이름
  char PlayerName[20];
  
//플레이어 번호
 INT32 PlayerNumber = -1;
//플레이어 방번호
 INT32 PlayerRoomNumber = -1;
//플레이어 색깔
 INT32 PlayerColor = 0;
//플레이어 승패정보
 ResultType PlayerResultType = RT_Playing;
//상대 플레이어 이름
 char OtherPlayerName[20];
 //채팅 보낸 플레이어 이름
 char SendPlayerName[20];
// 플레이어 턴정보
 BOOL PlayerTurn;
//보낼 채팅버퍼
 char SendChatBuffer[255];
//받을 채팅버퍼
 char RecvChatBuffer[255];
// 게임시작유무
 BOOL IsGameStart = FALSE;
// 오목버퍼
 Dol Dols[YCOUNT][XCOUNT];

int CclientSocket::SocketSetting()
{
	//wsadata 생성
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
	{
		PopLog("WSAData errro!");
		return -1;
	}
	//소켓생성
	hsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hsock==SOCKET_ERROR)
	{
		PopLog("Socket erro!");
		WSACleanup();
		return -1;
	}
	//서버주소세팅
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &servaddr.sin_addr);
	servaddr.sin_port =htons( PORT);
	//연결
	if (connect(hsock, (sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR)
	{
		PopLog("SOCKET CONNECT ERROR!");
		closesocket(hsock);
		WSACleanup();
		return -1;
	}
}

// Socket 셋팅 + 첫 네임정보 넘겨주고 서버로부터 플레이어넘버와 방번호 받기
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
		OutHelloData->Write(OutStream);  //여기가 문제
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
		// 플레이어 번호와 룸번호, 색깔정보 확인 받음
		InHelloData->Read(inputstream, HrecvBuffer, recvnum);
		PlayerRoomNumber = InHelloData->GetRoomNumber();
		PlayerColor = InHelloData->GetPlayerColor();
		wsprintf(buffer,"Connect! Playernumber : %d, RoomNumber : %d, Color : %d", PlayerNumber, PlayerRoomNumber,PlayerColor);
		PopLog(buffer);
	}
}
//소켓 연결 끊기
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

//클라에서 서버로 포인트정보 보내기
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
	//플레이어턴 없애기
	PlayerTurn = FALSE;
	/*char buffer[100];
	wsprintf(buffer, "Client %d send MouseINfo! (%d %d)", PlayerNumber,x,y );
	PopLog(buffer);*/
}
// 서버에서 온 State정보 받고 1차원배열의 오목배열을 2차원으로 변환시켜 리턴
void CclientSocket::RecvState(char HrecvBuffer[HRECVBUFFERLEN])
{

	InStateData->Read(inputstream,HrecvBuffer,HRECVBUFFERLEN);
	PlayerResultType = InStateData->GetResultType();
	PlayerTurn = InStateData->GetTurn();
	// 오목배열 변환
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
// 클라에서 서버로 채팅정보 보내기
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
//이 함수로 받아온 데이터의 헤더를 보고 recv데이터 구분
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
		// Hello는 메인 스레드에서 수행
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

#pragma once
#include "pch.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include<process.h>
#include<map>
#include<tuple>
#include<vector>
#define SERVERIP "127.0.0.1"
#define PORT 30002
#define SEND_DATA 2
#define RECV_DATA 1
#define MAXLINE 512
#define MaxRoom 100
#define HRECVBUFFERLEN 1200
//오목만들기 서버

#define XCOUNT 19
#define YCOUNT 15
#define X_START 50
#define Y_START 50
#define INTERVAL 40
#define H_INTERVAL 20
// x,y좌표는 0부터 19까지
#define XPOINT(x)(X_START + (x*INTERVAL) )
#define YPOINT(y)(Y_START + (y*INTERVAL) )
#define WHITE 0
#define BLACK 1

typedef struct DOL
{
	BOOL isPuton = FALSE;
	INT32 Color = WHITE;
}Dol;
int OmocCount(Dol Dols[YCOUNT][XCOUNT], int x, int y, int color);
void InitOmocArray(Dol(*omocarray)[YCOUNT][XCOUNT]);
// 패킷타입
enum PacketType
{
	PT_Hello,
	PT_Start,
	PT_GameData,
	PT_ChatData,
	PT_Disconnect
};
//승패 타입
enum ResultType
{
	RT_Playing,
	RT_Win,
	RT_Lose
};
// OutputStream
class OutputStream
{
	//Init으로 항상 초기화 해주고 사용하기
public:
	OutputStream() :
		Buffer(nullptr), mHead(0), mCapacity(0)
	{
		ReallocBuffer(32);
	}
	~OutputStream()
	{
		free(Buffer);
	}
	void Init()
	{
		Buffer = nullptr;
		mHead = 0;
		mCapacity = 0;
		ReallocBuffer(32);
	}
	const char* GetBuffer() const { return Buffer; }
	const UINT32 GetLength() const { return mHead; }
	void Write(UINT32 Data)
	{
		Write(sizeof(Data), &Data);
	}
	void Write(INT32 Data)
	{
		Write(sizeof(Data), &Data);
	}
	void Write(UINT32 Datalen, const void* Data)
	{
		UINT32 resultLen = mHead + Datalen;
		if (resultLen > mCapacity)
			ReallocBuffer(max(mCapacity * 2, resultLen));
		memcpy_s(Buffer + mHead, resultLen, Data, Datalen);
		mHead = resultLen;

		memset(Buffer + mHead, 0, mCapacity - mHead);
	}

private:
	void ReallocBuffer(UINT32 newLength)
	{
		Buffer = static_cast<char*>(realloc(Buffer, newLength));
		mCapacity = newLength;
	}
	char* Buffer;
	UINT32 mHead;
	UINT32 mCapacity;

};
class InputStream
{
public:
	InputStream() :

		mHead(0), mCapacity(0)
	{
	}
	~InputStream() { free(mBuffer); }
	UINT32 GetRemainingDataSize() const { return mCapacity - mHead; }
	//SetSocketBuffer로 소켓의 내용을 mBuffer에 채운후 
	//Read함수를 통해서 outData에 mBuffer의 내용을 할당시키기
	//다른 클래스 받기위해 초기화 할댄 다시 SetSocketBuffer호출 후 읽기반복
	void SetSocketBuffer(char* SocketData, UINT32 AllinByteCount)
	{
		mHead = 0;
		mBuffer = (char*)realloc(mBuffer, AllinByteCount);
		memcpy_s(mBuffer, AllinByteCount, SocketData, AllinByteCount);
		mCapacity = AllinByteCount;
	}
	int Read(UINT32& outData) { return Read(sizeof(outData), &outData); }
	int Read(INT32& outData) { return Read(sizeof(outData), &outData); }
	int Read(UINT32 inByteCount, void* outData)
	{
		if (inByteCount > GetRemainingDataSize())
		{
			printf("InputStream DataOVer Error!\n");
			return -1;
		}
		//outData에다가 mBufffer의 Head를 inByteCount만큼 옮겨가면서 할당
		memcpy_s(outData, inByteCount, mBuffer + mHead, inByteCount);
		mHead += inByteCount;
		return 1;
	}
private:

	char* mBuffer; //소켓으로 받은 내용
	UINT32 mHead;
	UINT32 mCapacity; // mBuffer의 전체 사이즈


};

class OutHelloDATA
	//넘겨줄 Hello데이터 -> SetData로 데이터 세팅 후 Write
{
public:
	OutHelloDATA()
	{
		RoomNumber = -1;
		PlayerNumber = -1;
		Color = -1;
	}
	~OutHelloDATA() {}
	void SetData(INT32 playernum, INT32 roomnum, INT32 color)
	{
		PlayerNumber = playernum;
		RoomNumber = roomnum;
		Color = color;

	}
	void Write(OutputStream* output)
	{
		output->Init();
		output->Write(Ptype);
		output->Write(PlayerNumber);
		output->Write(RoomNumber);
		output->Write(Color);
	}

private:
	PacketType Ptype = PT_Hello;
	INT32 PlayerNumber;
	INT32 RoomNumber;
	INT32 Color;
};

class InHelloDATA
{

public:
	InHelloDATA()
	{
		memset(Name, 0, sizeof(Name));
		NameLen = 0;
	}
	~InHelloDATA() {}
	char* GetNameData()
	{
		return Name;
	}
	void Read(InputStream* input, char* SocketData, UINT32 SocketDataLength)
	{
		NameLen = 0;
		input->SetSocketBuffer(SocketData, SocketDataLength);
		INT32 type = 0;
		input->Read(type);
		if (type != Ptype) { printf("TYpeError!"); }
		input->Read(NameLen);
		input->Read(NameLen, Name);
	}

private:
	PacketType Ptype = PT_Hello;
	INT32 NameLen;
	char Name[20] = { 0 };

};
//넘겨줄 Start데이터
class OutSTARTDATA
{
public:
	OutSTARTDATA()
	{
		
	}
	~OutSTARTDATA() {}
	void SetData(INT32 playernum, char otherplayername[255], INT32 turn)
	{
		PlayerNumber = playernum;
		OtherPlayerNameLen = strlen(otherplayername);
		memcpy_s(OtherPlayerName, sizeof(OtherPlayerName), otherplayername, OtherPlayerNameLen);
		Turn = turn;
	}
	void Write(OutputStream* output)
	{
		output->Init();
		output->Write(Ptype);
		output->Write(PlayerNumber);
		output->Write(OtherPlayerNameLen);
		output->Write(OtherPlayerNameLen,OtherPlayerName);
		output->Write(Turn);
	}

private:
	PacketType Ptype = PT_Start;
	INT32 PlayerNumber;
	INT32 OtherPlayerNameLen;
	char OtherPlayerName[255];
	INT32 Turn;
};
//넘겨받을 마우스좌표 클래스
class InMOUSEDATA
{
public:
	InMOUSEDATA()
	{
	}
	~InMOUSEDATA() {}
	INT32 GetX() { return x; }
	INT32 GetY() { return y; }
	void Read(InputStream* input, char* SocketData, UINT32 SocketDataLength)
	{
		input->SetSocketBuffer(SocketData, SocketDataLength);
		INT32 type = 0;
		input->Read(type);
		if (type != Ptype) { printf("TYpeError!"); }
		input->Read(x);
		input->Read(y);
	}
	
private:
	PacketType Ptype = PT_GameData;
	INT32 x;
	INT32 y;
};
class OutSTATEDATA
	//넘겨줄 게임 상태데이터 -> SetData로 데이터 세팅 후 Write
{
public:
	OutSTATEDATA()
	{
	}
	~OutSTATEDATA() {}
	void SetData(INT32 playernum,ResultType rtype,BOOL turn,Dol (*omocarray)[XCOUNT])
	{
		PlayerNumber = playernum;
		Rtype = rtype;
		Turn = turn;
		for (int i = 0; i < YCOUNT; i++)
		{
			for (int j = 0; j < XCOUNT; j++)
			{
				OmocArray[i][j] = omocarray[i][j];
			}
		}
	}
	void Write(OutputStream* output)
	{
		output->Init();
		output->Write(Ptype);
		output->Write(PlayerNumber);
		output->Write(Rtype);
		output->Write(Turn);
		for (int i = 0; i < YCOUNT; i++)
		{
			for (int j = 0; j < XCOUNT; j++)
			{
				if (OmocArray[i][j].isPuton == FALSE)
				{
					output->Write(0);
				}
				else if(OmocArray[i][j].isPuton==TRUE)
				{
					if (OmocArray[i][j].Color == WHITE)
					{
						output->Write(1);
					}
					else if (OmocArray[i][j].Color == BLACK)
					{
						output->Write(2);
					}
				}
			}
		}

		
	}

private:
	PacketType Ptype = PT_GameData;
	INT32 PlayerNumber;
	ResultType Rtype;
	BOOL Turn;
	Dol OmocArray[YCOUNT][XCOUNT];
};
// 넘겨받을 채팅정보 클래스
class InCHATDATA
{
public:
	InCHATDATA()
	{
	}
	~InCHATDATA() {}
	
	char* GetChat() { return ChatBuffer; }
	void Read(InputStream* input, char* SocketData, UINT32 SocketDataLength)
	{
		input->SetSocketBuffer(SocketData, SocketDataLength);
		INT32 type = 3;
		input->Read(type);
		if (type != Ptype) { printf("TYpeError!"); }
		input->Read(ChatLen);
		input->Read(ChatLen, ChatBuffer);
	}

private:
	PacketType Ptype = PT_ChatData;
	INT32 ChatLen;
	char ChatBuffer[255] = { 0 };

};
class OutCHATDATA
	//넘겨줄 채팅데이터 -> SetData로 데이터 세팅 후 Write
{
public:
	OutCHATDATA()
	{
	}
	~OutCHATDATA() {}
	void SetData(INT32 playernum, char sendplayername[20], char chat[255])
	{
		PlayerNumber = playernum;
		OtherPlayerNameLen = strlen(sendplayername);
		memcpy_s(OtherPlayerName, sizeof(OtherPlayerName), sendplayername, OtherPlayerNameLen);
		ChatLen = strlen(chat);
		memcpy_s(ChatBuffer, sizeof(ChatBuffer), chat, ChatLen);
	}
	void Write(OutputStream* output)
	{
		output->Init();
		output->Write(Ptype);
		output->Write(PlayerNumber);
		output->Write(OtherPlayerNameLen);
		output->Write(OtherPlayerNameLen,OtherPlayerName);
		output->Write(ChatLen);
		output->Write(ChatLen,ChatBuffer);
	}

private:
	PacketType Ptype = PT_ChatData;
	INT32 PlayerNumber;
	INT32 OtherPlayerNameLen;
	char OtherPlayerName[20];
	INT32 ChatLen;
	char ChatBuffer[255];
};
//넘겨줄 Start데이터
class OutDISCONNECTDATA
{
public:
	OutDISCONNECTDATA()
	{

	}
	~OutDISCONNECTDATA() {}
	void SetData(INT32 playernum)
	{
		PlayerNumber = playernum;
	}
	void Write(OutputStream* output)
	{
		output->Init();
		output->Write(Ptype);
		output->Write(PlayerNumber);
	}

private:
	PacketType Ptype = PT_Disconnect;
	INT32 PlayerNumber;
};
// 넘겨줄 채팅정보 클래스
class InDISCONNECT
{
public:
	InDISCONNECT()
	{
		Ptype = PT_Disconnect;
	}
	~InDISCONNECT() {}
	void Read(InputStream* input, char* SocketData, UINT32 SocketDataLength)
	{

		input->SetSocketBuffer(SocketData, SocketDataLength);
		INT32 type = 4;
		input->Read(type);
		if (type != Ptype) { printf("TYpeError!"); }
		printf("Recv DIsconnect!\n");
	}


private:
	PacketType Ptype = PT_Disconnect;

};
class Per_Handle_Data
{
public:
	SOCKET fd;
	sockaddr_in* servaddr;

};
class Per_IO_Data
{
public:
	Per_IO_Data()
	{
		InitData();
	}
	OVERLAPPED overlapped;
	int OperType;
	char buf[MAXLINE];
	WSABUF wsabuf;
	int flags;
	void InitData()
	{
		memset(&overlapped, 0, sizeof(OVERLAPPED));
		OperType = RECV_DATA;
		memset(buf, 0, sizeof(buf));
		wsabuf.buf = buf;
		wsabuf.len = MAXLINE;
		flags = 0;
	}
};

class Player //플레이어 정보
{
public:
	Player(Per_Handle_Data* per_handle_data, char name[20], BOOL turn)
	{
		memcpy_s(&Per_Handle_data, sizeof(Per_Handle_data), per_handle_data, sizeof(Per_Handle_data));
		memcpy_s(Name, sizeof(Name), name, strlen(name));
		Turn = turn;
	}
	~Player()
	{
		//delete(&Per_Handle_data);
		shutdown(Per_Handle_data.fd, SD_BOTH);
		closesocket(Per_Handle_data.fd);
	}
	Per_Handle_Data Per_Handle_data;
	char Name[20];
	BOOL Turn;
};
class Room
{
public:
	Room()
	{
		memset(ChatBuffer, 0, sizeof(ChatBuffer));
		memset(SendPlayerName, 0, sizeof(SendPlayerName));
		isStartRoom = FALSE;
		WinPlayerColor = -1;
		InitOmocArray(&OmocArray);
		PlayerSeat[0] = nullptr;
		PlayerSeat[1] = nullptr;
	}
	BOOL isStartRoom;
	INT32 WinPlayerColor;
	char ChatBuffer[255];
	char SendPlayerName[20];
	Player* PlayerSeat[2];
	Dol OmocArray[YCOUNT][XCOUNT];
	INT32 GetPlayerCount()
	{
		INT32 playercount = 0;
		for (int i = 0; i < 2; i++)
		{
			if (PlayerSeat[i] != nullptr)
				playercount++;
		}
		return playercount;
	}
	void InitRoom()
	{
		isStartRoom = FALSE;
		WinPlayerColor = -1;
		for (int i = 0; i < 2; i++)
		{
			if (PlayerSeat[i] != nullptr)
			{
				delete(PlayerSeat[i]);
				PlayerSeat[i] = nullptr;
			}
			
		}
		InitOmocArray(&OmocArray);
		memset(ChatBuffer, 0, sizeof(ChatBuffer));
		memset(SendPlayerName, 0, sizeof(SendPlayerName));
	}
	int BroadCastMessage(PacketType Ptype)
	{
			
			OutputStream outputstream;
			OutSTATEDATA OutStateData;
			OutSTARTDATA OutStartData;
			OutCHATDATA OutChatData;
			OutDISCONNECTDATA OutDisconnData;
			ResultType Rtype;
			Per_Handle_Data* HandleData;
			for (int i = 0; i < 2; i++)
			{
				if (PlayerSeat[i] == nullptr) { return 0; }
				int otherindex = i == 0 ? 1 : 0;
				switch (Ptype)
				{
				case PT_Start:
					isStartRoom = TRUE;
					PlayerSeat[i]->Turn = i==0?1:0; // 0이 WHITE 1번이 BLACK 이므로 WHITE가 먼저 해야 하므로 WHITE 플레이어의 턴을 1로 해줌(TRUE == 1)
					OutStartData.SetData(PlayerSeat[i]->Per_Handle_data.fd, PlayerSeat[otherindex]->Name, PlayerSeat[i]->Turn);
					OutStartData.Write(&outputstream);
					break;
				case PT_GameData:
					if (WinPlayerColor == -1)
					{
						Rtype = RT_Playing;
						PlayerSeat[i]->Turn = PlayerSeat[i]->Turn == TRUE ? FALSE : TRUE; // 턴 바꿔주기
					}
					else 
					{
						// 0번인덱스가 WHITE이므로
						if (i == WinPlayerColor) { Rtype = RT_Win; }
						else { Rtype = RT_Lose; }
						PlayerSeat[i]->Turn = FALSE; //승부났으면 턴 오프
					}
					
					OutStateData.SetData(PlayerSeat[i]->Per_Handle_data.fd,Rtype, PlayerSeat[i]->Turn,OmocArray);
					OutStateData.Write(&outputstream);
					break;
				case PT_ChatData:
					OutChatData.SetData(PlayerSeat[i]->Per_Handle_data.fd, SendPlayerName, ChatBuffer);
					OutChatData.Write(&outputstream);
					break;
				case PT_Disconnect:
					OutDisconnData.SetData(PlayerSeat[i]->Per_Handle_data.fd);
					OutDisconnData.Write(&outputstream);
					break;
				default:
					break;
				}
				HandleData = &(PlayerSeat[i]->Per_Handle_data);
				Per_IO_Data IOData;
				IOData.InitData();
				IOData.wsabuf.buf = (char*)outputstream.GetBuffer();
				IOData.wsabuf.len = outputstream.GetLength();
				IOData.OperType = SEND_DATA;
				if (WSASend(HandleData->fd, &IOData.wsabuf, 1, &IOData.wsabuf.len, 0, &IOData.overlapped, NULL) == SOCKET_ERROR)
				{
					if (WSAGetLastError() == WSA_IO_PENDING)
					{ 
					}
					else
					{
						printf("%dClient To WsaSend ERror %d\n", HandleData->fd,WSAGetLastError());
						return -1;
					}
				}

				IOData.InitData();
				IOData.OperType = RECV_DATA;
				if (WSARecv(HandleData->fd, &IOData.wsabuf, 1, &IOData.wsabuf.len, (LPDWORD)&IOData.flags, (OVERLAPPED*)&IOData.overlapped, NULL) == SOCKET_ERROR)
				{
					if (WSAGetLastError() == WSA_IO_PENDING)
					{
					}
					else
					{
						printf("%dClient To WsaSend ERror %d\n", HandleData->fd,WSAGetLastError());
						return -1;
					}
				}
			}
			return 1;
	}
};

class PlayerRooms
{
public:
	PlayerRooms()
	{



		for (int i = 0; i < MaxRoom; i++)
		{
			Rooms[i] = new Room();
		}
	}
	INT32 GetRoomNumber(SOCKET sock)
	{
		for (int i = 0; i < MaxRoom; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				if (Rooms[i]->PlayerSeat[j] != nullptr&&Rooms[i]->PlayerSeat[j]->Per_Handle_data.fd == sock)
					return i;
			}
			printf("Can't Find RoomNumber");
			return -1;
		}
	}
	INT32 GetPlayerSeatNumber(SOCKET sock)
	{
		for (int i = 0; i < MaxRoom; i++)
		{
				for (int j = 0; j < 2; j++)
				{
					if (Rooms[i]->PlayerSeat[j]!=nullptr&&Rooms[i]->PlayerSeat[j]->Per_Handle_data.fd == sock)
					{
						return j;
					}

				}					
		}
		printf("Can't Find SeatNumber\n");
		return -1;
	}
	Room* GetPalyerRoom(SOCKET sock)
	{
		for (int i = 0; i < MaxRoom; i++)
		{
			if (Rooms[i]->isStartRoom == TRUE)
			{
				for (int j = 0; j < 2; j++)
				{
					if (Rooms[i]->PlayerSeat[j]!=nullptr &&Rooms[i]->PlayerSeat[j]->Per_Handle_data.fd == sock)
					{
						return Rooms[i];
					}
				}
				
			}
		}
		return NULL;
	}
	std::vector<Room*>  GetFullRoom()
	{
		std::vector<Room*> FullRoomVec;
		for (int i = 0; i < MaxRoom; i++)
		{
			if (Rooms[i]->isStartRoom==FALSE&&Rooms[i]->GetPlayerCount()==2)
			{
				FullRoomVec.push_back(Rooms[i]);
			}
		}
		return FullRoomVec;
	}
	void SetPlayer(Player* player)
	{
		for (int i = 0; i < MaxRoom; i++)
		{
			if (Rooms[i]->isStartRoom == FALSE && Rooms[i]->GetPlayerCount()==1)
			{
				for (int j = 0; j < 2; j++)
				{
					if (Rooms[i]->PlayerSeat[j] == nullptr)
					{
						Rooms[i]->PlayerSeat[j] = player;
						return;
					}
				}
			}
		}
		for (int i = 0; i < MaxRoom; i++)
		{
			if (Rooms[i]->isStartRoom == FALSE && Rooms[i]->GetPlayerCount() == 0)
			{
				Rooms[i]->PlayerSeat[0] = player;		
				return;
			}
		}
		printf("All Rooms Full!\n");
	}
	// Room 플레이어들 지워주고 룸 초기화
	void RoomClear(SOCKET sock)
	{
		for (int i = 0; i < MaxRoom; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				if (Rooms[i]->PlayerSeat[j] != nullptr&&Rooms[i]->PlayerSeat[j]->Per_Handle_data.fd == sock)
				{
					Rooms[i]->InitRoom();
					return;
				}
			}
		}
		printf("Can't ExitPlayer\n");
	}
	Player* GetPlayer(SOCKET sock)
	{
		for (int i = 0; i < MaxRoom; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				if (Rooms[i]->PlayerSeat[j]!=nullptr&&Rooms[i]->PlayerSeat[j]->Per_Handle_data.fd== sock)
				{
					return Rooms[i]->PlayerSeat[j];
				}
			}			
		}
		printf("Can't FInd Player");
		return nullptr;
	}
	Player* GetOtherPlayer(SOCKET playersock)
	{
		for (int i = 0; i < MaxRoom; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				if (Rooms[i]->PlayerSeat[j] != nullptr&&Rooms[i]->PlayerSeat[j]->Per_Handle_data.fd == playersock)
				{
					int otherindex = j == 0 ? 1 : 0;
					return  Rooms[i]->PlayerSeat[otherindex];
				}
			}
		}
		printf("Can't FInd OtherPlayer\n");
		return nullptr;
	}
private:
	Room* Rooms[MaxRoom];
};



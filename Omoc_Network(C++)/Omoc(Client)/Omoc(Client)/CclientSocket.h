#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include<Windows.h>
#include<windef.h>
#include<process.h>

#define SERVERIP "127.0.0.1"
#define PORT 30002
#define XCOUNT 19
#define YCOUNT 15
#define XCOUNT 19
#define YCOUNT 15
#define WHITE 0
#define BLACK 1
#define HRECVBUFFERLEN 1200

typedef struct DOL
{
	BOOL isPuton = FALSE;
	INT32 Color = WHITE;
}Dol;
void PopLog(const char* Clog);
void UpdateDrawOutText();
// CclientSocket���� ����� �Լ�
void InvalideRectFunc();
extern int Debug;
//���� Ÿ��
enum ResultType
{
	RT_Playing,
	RT_Win,
	RT_Lose
};
//�÷��̾� �̸�
extern  char PlayerName[20];

//�÷��̾� ��ȣ
extern INT32 PlayerNumber;
//�÷��̾� ���ȣ
extern INT32 PlayerRoomNumber;
//�÷��̾� ����
extern INT32 PlayerColor;
//�÷��̾� ��������
extern ResultType PlayerResultType;
//��� �÷��̾� �̸�
extern char OtherPlayerName[20];
// �÷��̾� ������
extern BOOL PlayerTurn;

//ä�� ���� �÷��̾� �̸�
extern char SendPlayerName[20];
//���� ä�ù���
extern char SendChatBuffer[255];
//���� ä�ù���
extern char RecvChatBuffer[255];
// ���ӽ�������
extern BOOL IsGameStart;
// �������
extern Dol Dols[YCOUNT][XCOUNT];
// OutputStream
class OutputStream
{
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
	const char* GetBuffer() const {return Buffer;}
	const UINT32 GetLength() const { return mHead; }
	void Write(UINT32 Data)
	{
		Write(sizeof(Data),&Data);
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
	~InputStream() { }
	UINT32 GetRemainingDataSize() const { return mCapacity - mHead; }
	//SetSocketBuffer�� ������ ������ mBuffer�� ä���� 
	//Read�Լ��� ���ؼ� outData�� mBuffer�� ������ �Ҵ��Ű��
	//�ٸ� Ŭ���� �ޱ����� �ʱ�ȭ �Ҵ� �ٽ� SetSocketBufferȣ�� �� �б�ݺ�
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
			PopLog("InputStream DataOVer Error!");
			return -1;
		}
		//outData���ٰ� mBufffer�� Head�� inByteCount��ŭ �Űܰ��鼭 �Ҵ�
		memcpy_s(outData, inByteCount, mBuffer + mHead, inByteCount);
		mHead += inByteCount;
		return 1;
	}
private:

	char* mBuffer; //�������� ���� ����
	UINT32 mHead;
	UINT32 mCapacity; // mBuffer�� ��ü ������


};
// ��ŶŸ��
enum PacketType
{
	PT_Hello,
	PT_Start,
	PT_GameData,
	PT_ChatData,
	PT_Disconnect
};

//�Ѱ��� Hello Ŭ����
class OutHelloDATA
{
public:
	OutHelloDATA()
	{
		Name = nullptr;
	}
	~OutHelloDATA() {}
	void SetData(const char* name)
	{
		Name = name;
	}
	void Write(OutputStream* output)
	{
		if (Name == NULL)
			PopLog("NameData NULL");
		output->Init();
		output->Write(Ptype);
		UINT8 Len = strlen(Name);
		output->Write(Len);
		output->Write(Len, Name);
	}

private:
	PacketType Ptype = PT_Hello;
	const char* Name;
	
};
//�Ѱܹ��� Hello Ŭ����
class InHelloDATA
{
public:
	InHelloDATA()
	{
	}
	~InHelloDATA() {}
	INT32 GetRoomNumber()
	{
		return RoomNumber;
	}
	INT32 GetPlayerColor()
	{
		return color;
	}
	void Read(InputStream* input, char* SocketData, UINT32 SocketDataLength)
	{
		RoomNumber = 0;
		input->SetSocketBuffer(SocketData, SocketDataLength);
		INT32 type = 0;
		input->Read(type);
		if (type != Ptype) { PopLog("TYpeError!"); }
		input->Read(PlayerNumber);
		input->Read(RoomNumber);
		input->Read(color);
	}

private:
	PacketType Ptype = PT_Hello;
	INT32 RoomNumber;
	INT color;
};
//�Ѱܹ��� Start Ŭ����
class InSTARTDATA
{
public:
	InSTARTDATA()
	{
	}
	~InSTARTDATA() {}
	
	void Read(InputStream* input, char* SocketData, UINT32 SocketDataLength)
	{
		input->SetSocketBuffer(SocketData, SocketDataLength);
		INT32 type = 1;
		input->Read(type);
		if (type != Ptype) { PopLog("TYpeError!"); }
		INT32 playernum = 0;
		input->Read(playernum);
		if (PlayerNumber != playernum) { PopLog("InStatePacket Playernumber miss Matching!"); }
		input->Read(NameLen);
		input->Read(NameLen,OtherPlayerName);
		input->Read(PlayerTurn);
	}

private:
	PacketType Ptype = PT_Start;
	INT32 NameLen;
};
//�Ѱ��� ���콺��ǥ Ŭ����
class OutMOUSEDATA
{
public:
	OutMOUSEDATA()
	{
		x = 0;
		y = 0;
	}
	~OutMOUSEDATA(){}
	void SetData(INT32 ix, INT32 iy)
	{
		x = ix;
		y = iy;
	}
	void Write(OutputStream* output)
	{
		output->Init();
		output->Write(Ptype);
		output->Write(x);
		output->Write(y);
	}

private:
	PacketType Ptype = PT_GameData;
	INT32 x;
	INT32 y;
};
//�Ѱܹ��� ���ӻ��� Ŭ����
class InSTATEDATA
{
public:
	InSTATEDATA()
	{
		for (int i = 0; i < XCOUNT*YCOUNT; i++)
		{
			memset(&OmocArray[i], 0, sizeof(Dol));
		}
	}
	~InSTATEDATA() {}
	Dol* GetOmocArray() { return OmocArray; }
	ResultType GetResultType(){return Rtype; }
	bool GetTurn() { return Turn; }
	void Read(InputStream* input, char* SocketData, UINT32 SocketDataLength)
	{
		input->SetSocketBuffer(SocketData, SocketDataLength);
		INT32 ptype = 0;
		input->Read(ptype);
		if (ptype != Ptype) { PopLog("TYpeError!"); }
		INT32 playernum = 0;
		input->Read(playernum);
		if (PlayerNumber != playernum) { PopLog("InStatePacket Playernumber miss Matching!"); }
		INT32 rtype = 0;
		input->Read(rtype);
		Rtype = static_cast<ResultType>(rtype);
		PlayerResultType = Rtype;
		input->Read(Turn);
		//�ȳ����� -> 0 , ���� �� �Ͼ�� ->1 ���� �� ������ ->2
		for (int i = 0; i < XCOUNT*YCOUNT; i++)
		{
			input->Read(IOmocArrray[i]);
			if (IOmocArrray[i] == 0)
			{
				//���ȳ�����
				OmocArray[i].isPuton = FALSE;
				OmocArray[i].Color = WHITE;
			}
			else if(IOmocArrray[i] == 1)
			{
				//�Ͼ� �� ������
				OmocArray[i].isPuton = TRUE;
				OmocArray[i].Color = WHITE;

			}
			else if (IOmocArrray[i] == 2)
			{
				//���� �� ������
				OmocArray[i].isPuton = TRUE;
				OmocArray[i].Color = BLACK;
			}
		}

	}

private:
	PacketType Ptype = PT_GameData;
	ResultType Rtype = RT_Playing;
	BOOL Turn = false;
	UINT32 OmocArrayLen;
	//2�����迭�� 1�����迭�� ����ȭ
	INT32 IOmocArrray[XCOUNT*YCOUNT];
	Dol OmocArray[XCOUNT*YCOUNT];
};
// �Ѱ��� ä������ Ŭ����
class OutCHATDATA
{
public:
	OutCHATDATA()
	{
		Ptype = PT_ChatData;
		ChatBuffer = new char[255];
	}
	~OutCHATDATA() { delete[](ChatBuffer); }
	void SetData(char* chatbuffer)
	{
		memcpy_s(ChatBuffer, 255, chatbuffer, strlen(chatbuffer));
		ChatBuffer[strlen(chatbuffer)] = '\0';
	}
	void Write(OutputStream* output)
	{
		output->Init();
		output->Write(Ptype);
		UINT32 Len = strlen(ChatBuffer);
		output->Write(Len);
		output->Write(Len,ChatBuffer);
	}

private:
	PacketType Ptype = PT_ChatData;
	char* ChatBuffer;

};
//�Ѱܹ��� Chat Ŭ����
class InCHATDATA
{
public:
	InCHATDATA()
	{
	}
	~InCHATDATA() {}
	void Read(InputStream* input, char* SocketData, UINT32 SocketDataLength)
	{
		input->SetSocketBuffer(SocketData, SocketDataLength);
		INT32 type = 3;
		input->Read(type);
		if (type != Ptype) { PopLog("TYpeError!"); }
		INT32 playernum = 0;
		input->Read(playernum);
		if (PlayerNumber != playernum) { PopLog("InChatPacket Playernumber miss Matching!"); }
		
		input->Read(SendPlayerNameLen);
		input->Read(SendPlayerNameLen, SendPlayerName);
		input->Read(ChatLen);
		//RecvChatBuffer �ʱ�ȭ
		memset(RecvChatBuffer, 0, HRECVBUFFERLEN);
		input->Read(ChatLen,RecvChatBuffer);
	}

private:
	PacketType Ptype = PT_ChatData;
	INT32 SendPlayerNameLen;
	UINT32 ChatLen;
};
// �Ѱ��� ä������ Ŭ����
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
		if (type != Ptype) { PopLog("TYpeError!"); }
		INT32 playernum = 0;
		input->Read(playernum);
		if (PlayerNumber != playernum) { PopLog("InDisCOENNCTPacket Playernumber miss Matching!"); }
		PopLog("Recv DIsconnect!\n");
	}


private:
	PacketType Ptype = PT_ChatData;

};
//�Ѱ��� Start������
class OutDISCONNECT
{
public:
	OutDISCONNECT()
	{

	}
	~OutDISCONNECT() {}
	void Write(OutputStream* output)
	{
		output->Init();
		output->Write(Ptype);
	}

private:
	PacketType Ptype = PT_Disconnect;
	
};
class CclientSocket
{
public:
	// �����ڿ��� �г��Ӱ� ���� INIT 
	CclientSocket();
	~CclientSocket();
	void SendChatting();
	void SendPoint(INT32 x, INT32 y);
	void SendDisconnect();
	int RecvData();
private:
	WSADATA wsaData;
	SOCKET hsock;
	sockaddr_in servaddr;
	OutCHATDATA* OutChatData;
	InSTATEDATA* InStateData;
	OutMOUSEDATA* OutMouseData;
	OutHelloDATA* OutHelloData;
	InHelloDATA* InHelloData;
	InSTARTDATA* InStartData;
	InCHATDATA* InChatData;
	OutputStream* OutStream;
	InputStream* inputstream;
	InDISCONNECT* IndisconnData;
	OutDISCONNECT* OutDisconnect;
	int SocketSetting();
	void RecvState(char HrecvBuffer[HRECVBUFFERLEN]);
	void RecvStart(char HrecvBuffer[HRECVBUFFERLEN]);
	void RecvChat(char HrecvBuffer[HRECVBUFFERLEN]);
	void RecvDisconn(char HrecvBuffer[HRECVBUFFERLEN]);


};
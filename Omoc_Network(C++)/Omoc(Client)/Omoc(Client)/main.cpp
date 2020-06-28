
#include "CclientSocket.h"
#define WM_RESTART 10000
//���񸸵�� Ŭ���̾�Ʈ
#define STARTBTN_WIDTH 400
#define STARTBTN_HEIGHT 200
#define HRECVBUFFERLEN 512
#define X_START 50
#define Y_START 50

#define INTERVAL 40
#define H_INTERVAL 20
// x,y��ǥ�� 0���� 19����
#define XPOINT(x)(X_START + (x*INTERVAL) )
#define YPOINT(y)(Y_START + (y*INTERVAL) )
#define X_END (50+((XCOUNT-1)*INTERVAL))
#define Y_END (50+((YCOUNT-1)*INTERVAL))

//ä��â
#define CHATXSTART X_END+50
#define CHATYSTART Y_START
#define ChatWidth 500
#define ChatHeight YCOUNT*INTERVAL
#define NAMEBUFFERLEN 20
#define MAXDRAWTEXTOUTLEN 2500
CHAR lLogbuffer[20];
HWND Hwnd;
//Client ���� 
enum State {Lobby,Matching,InGame  };
static State Cstate = Lobby;
//�г���
char NameChatBuffer[NAMEBUFFERLEN];


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HBRUSH hBrush = CreateSolidBrush(RGB(244, 176, 77));
	//������ ����ü 
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); //����ü ������
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = hBrush;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "C_omoc"; //���� ����̸�
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	//����ü ���
	RegisterClassEx(&wcex);
	//������ ����
	HWND hWnd = CreateWindow("C_omoc", "ClinetOmoc", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT
		, 1000, 800, NULL, NULL, hInstance, NULL);

	//������ ���
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//�޽�������
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
void Reset();
void HandleInit();
void MakeStartButton(HWND hWnd);
//�������� ���� ���� ��� ���
void WInMsg(bool isWin)
{
	Cstate = Lobby;
	InvalidateRect(Hwnd, NULL, TRUE);
		if (isWin)
		{
			MessageBox(Hwnd, "You WIN!", NULL, MB_OK);
		}
		else
			MessageBox(Hwnd, "You Loose!", NULL, MB_OK);	

}

typedef struct pOINTS
{
	int x;
	int y;
}Point;
Point GetCenterPoint(HWND hWnd)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	int Centerx = rect.left + (width/2.0f);
	int Centery = rect.top +(height/2.0f);
	char buffer[100];
	Point point = { Centerx, Centery};
	return point;
}

HWND HGameStartButton;
HWND HEditBox;
HWND HsendButton;
HWND HNameEditBox;
int x;//������ ���� ���콺 x��ǥ
int y;//������ ���� ���콺 y��ǥ
Dol RecvDols[YCOUNT][XCOUNT]; // �������� �޾ƿ� Dols�迭
Point CenterPoint;
RECT ChatRec;
char DrawOutText[MAXDRAWTEXTOUTLEN];
char OutputChatFormat[275];
CclientSocket* CsocketNetwork;

void UpdateDrawOutText()
{
	memset(OutputChatFormat, 0, sizeof(OutputChatFormat));
	wsprintf(OutputChatFormat, "%s : %s\n", SendPlayerName, RecvChatBuffer);
	strncat_s(DrawOutText, sizeof(DrawOutText), OutputChatFormat, strlen(OutputChatFormat));
	//ä��â ������ ä��â Ŭ����
	if (strlen(DrawOutText) >= MAXDRAWTEXTOUTLEN - 300)
	{
		memset(DrawOutText, 0, sizeof(DrawOutText));
	}
}
void OmocArrayInit()
{
	for (int i = 0; i < YCOUNT; i++)
	{
		for (int j = 0; j < XCOUNT; j++)
		{
			Dols[YCOUNT][XCOUNT].Color = 0;
			Dols[YCOUNT][XCOUNT].isPuton = FALSE;
		}
	}
}
// CclientSocket���� ����� �Լ�
void InvalideRectFunc()
{
	InvalidateRect(Hwnd, NULL, TRUE);
}
void PopLog(const char* Clog)
{
	MessageBox(Hwnd, Clog, NULL, MB_OK);
}
void Reset()
{
	char buffer[100];
	wsprintf(buffer, "%d ClientRestart!", PlayerNumber);
	PopLog(buffer);
	// ��������, main�Լ� ���� �ʱ�ȭ
	memset(DrawOutText, 0, sizeof(DrawOutText));
	memset(NameChatBuffer, 0, sizeof(char) * 11);
	DestroyWindow(HGameStartButton);
	DestroyWindow(HEditBox);
	DestroyWindow(HsendButton);
	DestroyWindow(HNameEditBox);
	memset(PlayerName, 0, sizeof(PlayerName));
	memset(OtherPlayerName, 0, sizeof(OtherPlayerName));
	memset(SendChatBuffer, 0, sizeof(SendChatBuffer));
	memset(RecvChatBuffer, 0, sizeof(RecvChatBuffer));
	PlayerNumber = -1;
	PlayerRoomNumber = -1;
	PlayerColor = 0;
	PlayerResultType = RT_Playing;
	PlayerTurn = FALSE;
	IsGameStart = FALSE;
	OmocArrayInit();
	if (CsocketNetwork != nullptr)
	{
		delete(CsocketNetwork);
	}
}
void HandleInit()
{
	memset(DrawOutText, 0, sizeof(DrawOutText));
	DestroyWindow(HGameStartButton);
	DestroyWindow(HEditBox);
	DestroyWindow(HsendButton);
	DestroyWindow(HNameEditBox);
}
void ResetEditBox(HWND hWnd)
{
	memset(NameChatBuffer, 0, sizeof(char) * 11);
}
bool isCorrectName()
{
	int NameLen = strlen(NameChatBuffer);
	if (NameLen <= 0 || NameLen > 10)
		return false;
	return true;
}
void MakeStartButton(HWND hWnd)
{
	Hwnd = hWnd;
	HNameEditBox = CreateWindow("edit", "", WS_CHILD | WS_VISIBLE | WS_BORDER, CenterPoint.x - STARTBTN_WIDTH / 2, CenterPoint.y - STARTBTN_HEIGHT / 2,
		STARTBTN_WIDTH - 75, 50, hWnd, (HMENU)3, NULL, NULL);
	HGameStartButton =  CreateWindow("button", "GameStart", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CenterPoint.x+ STARTBTN_WIDTH/2-50, CenterPoint.y - STARTBTN_HEIGHT / 2,
		 100, 50, hWnd, (HMENU)0, NULL, NULL);
	InvalideRectFunc();
}
void MakeMatchingBox(HDC hdc)
{
	Rectangle(hdc, CenterPoint.x - STARTBTN_WIDTH / 2, CenterPoint.y - STARTBTN_HEIGHT / 2,
		CenterPoint.x + STARTBTN_WIDTH / 2, CenterPoint.y + STARTBTN_HEIGHT / 2);
	SetTextAlign(hdc, TA_CENTER);
	TextOut(hdc, CenterPoint.x, CenterPoint.y, "Matching...", 11);
	
}
void MakeChat(HWND hWnd)
{
	HEditBox = CreateWindow("edit", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE, CHATXSTART, Y_END - 100, ChatWidth - 100, 100, hWnd, (HMENU)1, NULL, NULL);
	HsendButton = CreateWindow("button", "Send", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CHATXSTART + ChatWidth - 100, Y_END - 100, 100, 100, hWnd, (HMENU)2, NULL, NULL);
	InvalidateRect(hWnd, NULL, TRUE);
}

unsigned int __stdcall  RecvThreadFunc(void* arg)
{

	while (true)
	{
		int ret = CsocketNetwork->RecvData();
		if (ret== -1)
		{
			PopLog("RecvError! Socket Close!");

			PostMessage(Hwnd, WM_RESTART, NULL, NULL);
			return 0;
		}
	}
}
char Titlebuffer[100];


const char* TMpName;
HANDLE RecvThread;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	int ix = 0;
	int iy = 0;
	CenterPoint = GetCenterPoint(hWnd);
	char NameLablebuffer[] = "Enter your Name (Within 10 Letters!)  ";
	switch (message)
	{
	case WM_CREATE:
		MakeStartButton(hWnd);	
		break;
	case WM_RESTART:
		Cstate = Lobby;
		Reset();
		MakeStartButton(hWnd);
		break;
	case WM_COMMAND:
     switch (LOWORD(wParam))
     {
		 //���ӽ��� ��ưŬ��
     case 0:
		 
		 if (isCorrectName())
		 {	
			 HandleInit();
			 Cstate = Matching;
			 InvalidateRect(hWnd, NULL, TRUE);
			 //���⼭ �г��Ӻ����� ����Init�ϰ� ��,�� ���� 
			 char buffer[100];
			 wsprintf(buffer, "%d %d", sizeof(PlayerName), strlen(NameChatBuffer));
			 PopLog(buffer);
			 memcpy_s(PlayerName, sizeof(PlayerName), NameChatBuffer, strlen(NameChatBuffer));
			 CsocketNetwork = new CclientSocket();
			 //recv�� ������ �ϳ��� ���� recv �ޱ�
			 unsigned int threadid ;
			 RecvThread = (HANDLE)_beginthreadex(NULL, 0, RecvThreadFunc,NULL, 0, &threadid);
		 }
		 else
		 {
			
			 ResetEditBox(hWnd);
		 }
		 //ä��â editbox 
	 case 1:
		 switch (HIWORD(wParam))
		 {
			case EN_CHANGE:
			 {
				GetWindowText((HWND)lParam, SendChatBuffer, 255);
			 }
		 }
	   break;
	   //ä��â send��ư
	 case 2:
		CsocketNetwork->SendChatting();
		 break;
	   //�г����Է� editbox
	 case 3:
		 switch (HIWORD(wParam))
		 {
		 case EN_CHANGE:
		 {
			 GetWindowText((HWND)lParam, NameChatBuffer, NAMEBUFFERLEN);
		 }
		 }
		 break;
     }
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		switch (Cstate)
		{
		case Lobby:
			
			TextOut(hdc, CenterPoint.x - STARTBTN_WIDTH / 2, CenterPoint.y - STARTBTN_HEIGHT/2-25,NameLablebuffer, sizeof(NameLablebuffer)-1);
			break;
		case Matching:
			MakeMatchingBox(hdc);
			InvalidateRect(hWnd, NULL, TRUE);
			HandleInit();
			Cstate = InGame;
			// ä�ù� ����

			MakeChat(hWnd);
			break;
		case InGame:
			if (IsGameStart)
			{
				//���ȣ, �г���, ���г���, ������ �ؽ�Ʈ ���
				if (PlayerColor == 0) { TMpName = "White"; }
				else { TMpName = "Black"; }
				wsprintf(Titlebuffer, "%s(%d) vs %s ���� [���� �� : %s ,���� �� : %d, ���ȣ : %d]", PlayerName,PlayerNumber, OtherPlayerName, TMpName, PlayerTurn, PlayerRoomNumber);
				TextOutA(hdc, 50, 0, Titlebuffer, strlen(Titlebuffer));
			}
			if (Debug == 1 && IsGameStart == false)
			{

			}
			//���� �ٱ߱�
			for (int i = 0; i < XCOUNT; i++)
			{
				MoveToEx(hdc, XPOINT(i), YPOINT(0), NULL);
				LineTo(hdc, XPOINT(i), YPOINT(YCOUNT));
			}
			for (int i = 0; i < YCOUNT; i++)
			{
				MoveToEx(hdc, XPOINT(0), YPOINT(i), NULL);
				LineTo(hdc, XPOINT(XCOUNT), YPOINT(i));
			}
			// �� ����
			for (int i = 0; i < YCOUNT; i++)
			{
				for (int j = 0; j < XCOUNT; j++)
				{
					if (Dols[i][j].isPuton == TRUE)
					{
						if (Dols[i][j].Color == BLACK)
						{
							SelectObject(hdc, GetStockObject(BLACK_BRUSH));

						}
						else
						{

							SelectObject(hdc, GetStockObject(WHITE_BRUSH));
						}
						Ellipse(hdc, XPOINT(j) - H_INTERVAL, YPOINT(i) - H_INTERVAL, XPOINT(j) + H_INTERVAL, YPOINT(i) + H_INTERVAL);
					}
				}
			}
			SelectObject(hdc, GetStockObject(WHITE_BRUSH));
			Rectangle(hdc, X_END + 50, Y_START, X_END + 50 + ChatWidth, Y_START + ChatHeight - 100);
			ChatRec = { X_END + 55, Y_START+5, X_END + 50 + ChatWidth-5, Y_START + ChatHeight - 100-5 };
			
			DrawText(hdc,DrawOutText , -1, &ChatRec,  DT_WORDBREAK | DT_EDITCONTROL );
			switch (PlayerResultType)
			{
			case RT_Playing:
				break;
			case RT_Win:
				WInMsg(true);
				CsocketNetwork->SendDisconnect();
				//End Packet������ ���� �ٽý���
				break;
			case RT_Lose:
				WInMsg(false);
				CsocketNetwork->SendDisconnect();

				//End Pakcet������ ���� �ٽý���
				break;
			}
			break;
		default:
			break;
		}
	
		EndPaint(hWnd, &ps);
		break;
	case WM_LBUTTONDOWN:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		// ���� �ߺ�����
		ix = (x - X_START) / INTERVAL;
		iy = (y - Y_START) / INTERVAL;
		if ((x - X_START) % INTERVAL > H_INTERVAL) { ix += 1; }
		if ((y - Y_START) % INTERVAL > H_INTERVAL) { iy += 1; }
		// ������ Ŭ��
		if (IsGameStart&& PlayerTurn&&Cstate==InGame&&x>=50-H_INTERVAL &&x<=X_END+H_INTERVAL && y >= 50 - H_INTERVAL && y <= Y_END + H_INTERVAL && Dols[iy][ix].isPuton==FALSE)
		{
			 //���⼭ buffer�� x,y ��ǥ �־ ������
			CsocketNetwork->SendPoint(x, y);
		}		
		break;
	case WM_DESTROY:
		CsocketNetwork->SendDisconnect();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
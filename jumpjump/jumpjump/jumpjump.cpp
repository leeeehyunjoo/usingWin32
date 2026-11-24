// jumpjump.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#include "framework.h"
#include "jumpjump.h"
#include <vector>
#include <time.h>
#include <random>

#define MAX_LOADSTRING 100
#define SET_CHECK_GRAVITY_TIMER 1

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_JUMPJUMP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JUMPJUMP));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JUMPJUMP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_JUMPJUMP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   ///창 생성시 기본 값 설정(900X900)
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 900, 900, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ///중력이 발생할 수 있도록 타이머 설정(0.02초 단위로 메시지 생성)
   SetTimer(hWnd, SET_CHECK_GRAVITY_TIMER, 20, NULL);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//


///중력을 설정합니다
/// gravity->중력 가속도
/// velocity->플레이어의 세로 방향 속도(player.top, player.bottom의 변화)
/// 중력 구현
/// 1.중력만큼 세로 속도를 증가시킨다
/// 2.세로 속도만틈 플레이어 좌표를 아래로 이동시킨다
///

float gravity = 0.5;
float velocityY = 0;

///게임상태 변수 선언 0->실행중 1->게임일시중지 2->게임오버
int gameRunning = 0;

///스크롤상태 변수 선언 true->스크롤중 false->스크롤하지않음
bool isScroll = false;

///배경색을 칠하기 위한 변수 선언
RECT bgPaint;

///실제 게임이 운영될 백그라운드 설정
RECT bg = { 50,50,700,700 };

///플레이어가 움직이면서 백그라운드가 스크롤될 기준점
RECT bgScroll = { 50,250,700,700 };

///플레이어 구조체 설정
RECT player = { 350,600,400,650 };

///발판 구조체 설정하기
//RECT platform = { 250, 500, 450, 520 };

struct Platform
{
    RECT position;
    int type;       ///0=일반발판 1=이동발판 2=깨짐발판
    int movingDir;  ///x좌표의 이동
};

std::vector<Platform> platforms;
//Platform platform = { { 250, 300, 350, 320 },0 };

void AddPlatform(int x, int y, int type)
{
    ///높이 및 너비
    int width = 0;
    int height = 0;

    /// 1. type에 따라 발판 너비와 높이 결정
    if (type == 0) {
        width = 70;
        height = 20;
    }
    else if (type == 1) {
        width = 80;
        height = 20;
    }
    else if (type == 2) {
        width = 85;
        height = 20;
    }

    /// 2. position 구성
    Platform newPlatform;
    newPlatform.position.left = x;
    newPlatform.position.top = y;
    newPlatform.position.right = x + width;
    newPlatform.position.bottom = y + height;

    ///1움직인다 0움직이지 않는다
    if (type == 1)
    {
        newPlatform.movingDir = 1;
    }
    else 
    {
        newPlatform.movingDir = 0;
    }
    
    newPlatform.type = type;


    // 3. platforms에 추가
    platforms.push_back(newPlatform);
    
}

/// 최소 길 보장(y좌표는 일정 간격을 둔다. 플레이어가 이동할 수 있게 하기 위함)
void CreateBasicPath()
{
    AddPlatform(360, 680, 0);
    int y = 600;
    for (int i = 0; i < 6; i++)
    {
        AddPlatform(rand() % 581 + 50, y- 50, 0);
        y -= 110;
    }
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
    {
        ///게임이 실행 중이 아니면 타이머 작업을 중단
        if (gameRunning != 0) {
            return 0;
        }

        if (bgScroll.top >= player.top)
        {
            isScroll = true;
        }


        ///중력 적용 전의 plater bottom 값 저장
        int savedBottom = player.bottom;

        ///SET_CHECK_GRAVITY_TIMER에서 수행할 작업
        velocityY += gravity;

        ///이번 프레임의 예상 위치
        int nextTop = player.top + (int)velocityY;
        int nextBottom = player.bottom + (int)velocityY;

        ///스크롤이 시작되지 않은 상태에서 바닥에 도달했는지 판단
        if (isScroll == false)
        {
            ///플레이어의 예상 bottom이 바닥 경계에 도달했는지 확인
            if (nextBottom >= bg.bottom - 2)
            {
                ///플레이어가 바닥에 정확히 닿도록 위치 보정
                nextBottom = bg.bottom - 2;
                nextTop = nextBottom - 50;

                ///플레이어의 낙하 속도를 초기화
                velocityY = 0;

                ///게임오버 상태로 변경
                gameRunning = 2;
            }
        }

        if (bgScroll.top >= nextTop && velocityY < 0)
        {
            ///플레이어가 기준점 위로 올라가려는 경우
            int scrollAmount = bgScroll.top - nextTop;

            player.top = bgScroll.top;
            player.bottom = bgScroll.top + 50;

            ///발판들을 아래로 이동
            for (int i = 0; i < platforms.size(); i++)
            {
                platforms[i].position.top += scrollAmount;
                platforms[i].position.bottom += scrollAmount;
            }
        }
        else
        {
            ///기존 중력 이동
            player.top = nextTop;
            player.bottom = nextBottom;
        }

        /////스크롤이 시작되지 않은 상태
        //if (player.bottom + (int)velocityY < bg.bottom - 2) {
        //    player.top += (int)velocityY;
        //    player.bottom += (int)velocityY;
        //}
        //else {
        //    player.bottom = bg.bottom - 2;
        //    player.top = player.bottom - 50;
        //    velocityY = 0;
        //    gameRunning = false;
        //}


        ///player가 platform과 충돌하는지 판단합니다
        ///플레이어가 발판에 착지했는지 확인

        if (velocityY > 0) {
            for (int i = 0; i < platforms.size(); i++) {

                ///발판과 충돌 판정
                if (savedBottom <= platforms[i].position.top &&
                    player.bottom >= platforms[i].position.top &&
                    player.left < platforms[i].position.right &&
                    player.right > platforms[i].position.left)
                {
                    ///착지 처리
                    player.bottom = platforms[i].position.top;
                    player.top = player.bottom - 50;
                    velocityY = -13;

                    ///깨짐 발판이면 제거
                    if (platforms[i].type == 2) {
                        platforms.erase(platforms.begin() + i);
                        i--;
                    }
                    break;
                }
            }
        }

        ///type1인 발판을 움직이도록 구현

        for (int i = 0; i < platforms.size(); i++) {
            if (platforms[i].type == 1) {

                ///발판 이동
                platforms[i].position.right += platforms[i].movingDir;
                platforms[i].position.left += platforms[i].movingDir;

                ///오른쪽 경계에 도달하면 왼쪽으로 방향 변경
                if (platforms[i].position.right >= bg.right) {
                    platforms[i].movingDir = -2;
                }

                ///왼쪽 경계에 도달하면 오른쪽으로 방향 변경
                if (platforms[i].position.left <= bg.left) {
                    platforms[i].movingDir = 2;
                }
            }
        }


        InvalidateRect(hWnd, NULL, FALSE);
        
    }
    break;

    case WM_CREATE:
    {
        ///게임 영역 오른쪽 경계=700
        ///type0의 발판 너비는 70이므로 AddPlatform의 x 좌표는 최대 630까지 가능
        //AddPlatform(250, 500, 0);
        //AddPlatform(300, 150, 0);
        //AddPlatform(400, 600, 0);
        //AddPlatform(290, 230, 0);
        //AddPlatform(550, 350, 0);

        //시드 초기화
        srand(time(NULL));

        CreateBasicPath();

        ///type0의 발판을 5~10개 중에 만든다
        for (int i = 0; i < rand() % 10+5; i++) {
            AddPlatform(rand() % 581 + 50, rand() % 631 + 50, 0);
        }

        ///type1의 발판을 5~10개 중에 만든다
        for (int i = 0; i < rand() % 10 + 5; i++) {
            AddPlatform(rand() % 541 + 80, rand() % 631 + 80, 1);
        }

        ///type2의 발판을 5~10개 중에 만든다
        for (int i = 0; i < rand() % 10 + 5; i++) {
            AddPlatform(rand() % 531 + 83, rand() % 631 + 50, 2);
        }
    }
    break;
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_LEFT:
        {
            if (gameRunning != 0)
                return 0;
            ///플레이어는 백그라운드 영역 내에서만 이동이 가능하다
            ///bg의 x좌표보다 작을 수 없다
            if (bg.left < player.left) {
                ///좌측 방향키 누를 시 좌측으로 5만큼 이동
                player.left -= 5;
                player.right -= 5;

                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        break;

        case VK_RIGHT:
        {
            if (gameRunning != 0)
                return 0;

            ///플레이어는 백그라운드 영역 내에서만 이동이 가능하다
            ///bg의 x좌표보다 클 수 없다
            if (bg.right > player.right) {
                ///우측 방향키 누를 시 우측으로 5만큼 이동
                player.left += 5;
                player.right += 5;

                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        break;

        case VK_ESCAPE:
        {
            ///게임이 실행 중일 때만 일시정지
            if (gameRunning == 0)
            {
                ///게임 상태를 일시정지로 변경
                gameRunning = 1;

                ///화면을 다시 그리도록 요청
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        break;

        case VK_SPACE:
        {
            ///게임 일시정지 상태일 경우 이어하기
            if (gameRunning == 1)
            {
                ///게임 실행 상태로 변경
                gameRunning = 0;

                ///화면을 다시 그리도록 요청
                InvalidateRect(hWnd, NULL, FALSE);
            }

            ///게임 오버 상태일 경우
            else if (gameRunning == 2)
            {
                ///게임 재시작은 추후 초기화 함수 구현 시 연결
            }
        }
        break;
        }
    }
    break;
    case WM_COMMAND:
        {

            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;


            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...

            ///배경색 칠하기 위한 브러시 생성
            HBRUSH myBrush=CreateSolidBrush(RGB(173,216,230));
            GetClientRect(hWnd,&bgPaint);   ///창 크기를 가져온다
            FillRect(hdc, &bgPaint, myBrush);   ///myBrush로 가져온 창 크기를 채운다
            DeleteObject(myBrush);  ///배경색 브러시 자원 해제

            ///게임 운영 구역 그리기 위한 펜 생성
            HPEN bgPen = CreatePen(BS_SOLID, 2, RGB(0, 0, 0));
            HPEN osPen = (HPEN)SelectObject(hdc, bgPen);
            Rectangle(hdc, 50, 50, 700, 700);
            SelectObject(hdc, osPen);
            DeleteObject(bgPen);

            ///플레이어 그리기
            HBRUSH playerBrush = CreateSolidBrush(RGB(200, 150, 80));
            FillRect(hdc, &player, playerBrush);
            DeleteObject(playerBrush); ///플레이어 브러시 자원 해제

            ///발판 그리기
            HBRUSH platformBrushType0 = CreateSolidBrush(RGB(120,60,80));
            HBRUSH platformBrushType1 = CreateSolidBrush(RGB(40, 140, 200));
            HBRUSH platformBrushType2 = CreateSolidBrush(RGB(15, 255, 35));
            //FillRect(hdc, &platforms.position, platformBrush);
            for (int i = 0; i < platforms.size(); i++) {
                if(platforms[i].type == 0)
                FillRect(hdc, &platforms[i].position, platformBrushType0);
                
                if(platforms[i].type==1)
                FillRect(hdc, &platforms[i].position, platformBrushType1);

                if(platforms[i].type==2)
                FillRect(hdc, &platforms[i].position, platformBrushType2);
            }
            DeleteObject(platformBrushType0);
            DeleteObject(platformBrushType1);
            DeleteObject(platformBrushType2);


            ///게임 일시정지 알리기
            if (gameRunning == 1)
            {
                TextOut(hdc, 250, 250, L"일시정지", 4);
                TextOut(hdc, 250, 300, L"<space를 눌러 이어하기>", 16);
            }

            ///게임오버 알리기
            if (gameRunning == 2)
            {
                TextOut(hdc, 250, 400, L"게임 오버 ! 다음 기회를 노려보세요 ♬", 22);
            }

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        ///SET_CHECK_GRAVITY_TIMER 제거
        KillTimer(hWnd, SET_CHECK_GRAVITY_TIMER);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

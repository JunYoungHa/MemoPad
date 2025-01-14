﻿// MemoPad.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "MemoPad.h"

#define MAX_LOADSTRING 100
#define MAX_LINE 20
#define MAX_NOUN_SIZE 10

// 전역 변수:
HINSTANCE g_hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
RECT g_WindowRC = { 0, 0, 800, 500 };
static float g_fOffsetX;
static float g_fOffsetY;
WCHAR g_CH_SPELLING_Temp[MAX_NOUN_SIZE];

// 배경 처리
static HBRUSH g_WindowBrush;
static int iHour;


// 폰트 관련 변수
static COLORREF fColor;



// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CH_SPELLING(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK LINESPACING(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
vector<string> OutFromFile(TCHAR filename[], HWND hWnd, bool bTextout);

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
    LoadStringW(hInstance, IDC_MEMOPAD, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MEMOPAD));

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

    return (int)msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    SYSTEMTIME g_SystemTime;
    GetLocalTime(&g_SystemTime);
    iHour = g_SystemTime.wHour;

    if (iHour > 5 && iHour < 19)
    {
        g_WindowBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
        fColor = RGB(0, 0, 0);
    }

    else
    {
        g_WindowBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
        fColor = RGB(255, 255, 255);
    }

    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(nullptr, IDC_IBEAM);
    wcex.hbrBackground = g_WindowBrush;
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MEMOPAD);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));


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
    g_hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    HWND hWnd = CreateWindowW(szWindowClass, L"MemoPad", WS_OVERLAPPEDWINDOW,
        50, 50, g_WindowRC.right - g_WindowRC.left, 
        g_WindowRC.bottom - g_WindowRC.top, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static vector<string> vectext(1);
    static vector<string> vecStorageText(0);

    // iCaretPos -> 움직일 시 카렛 위치
    static int iCount, iLine, iBackEmptyCount, iCaretPos;
    static bool bTextUpdate;
    static bool bInputAble;


    OPENFILENAME OFN, SFN;
    

    TCHAR lpstrFile[100] = _T("");
    static char filepath[100], filename[100];

    static SIZE size;
    // 불러오기 필터
    TCHAR filter[] = _T("텍스트 문서(*.txt)\0*.txt\0모든 파일 \0*.*\0");


    // 폰트 관련 변수 선언
    CHOOSEFONT FONT;
    HFONT hFont, OldFont = 0;
    static LOGFONT LogFont;

    //펜 관련 변수 선언
    static bool g_bDraw = false;	//	마우스가 눌린 상태인지 확인할 수 있는 bool 변수
    static int g_iX;				//	마우스의 x좌표값
    static int g_iY;				//	마우스의 y좌표값
    static int iDrawCount;

    HPEN MyPen = NULL;
    int P_Color = RGB(0, 255, 0);

    POINT pt;


    switch (message)
    {
        // 윈도우를 생성하기 전 메세지 전달
    case WM_CREATE:
        iLine = 0;
        iCount = 0;
        g_fOffsetX = 5;
        g_fOffsetY = 20;
        iBackEmptyCount = 1;
        bTextUpdate = false;
        bInputAble = true;               
        iDrawCount = -1;
        CreateCaret(hWnd, NULL, 3, 15);
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case ID_NEW:
        {
            MessageBox(hWnd, L"새 파일을 열겠습니까?", L"새 파일 선택", MB_OKCANCEL);
            vecStorageText.clear();
            vectext.clear();
            vecStorageText.resize(1);
            vectext.resize(1);
            iLine = 0;
            iCount = 0;
            InvalidateRect(hWnd, NULL, TRUE);
            SetCaretPos(g_fOffsetX, 0);
            break;
        }
        case ID_OPEN:
        {
            memset(&OFN, 0, sizeof(OPENFILENAME));
            OFN.lStructSize = sizeof(OPENFILENAME);
            OFN.hwndOwner = hWnd;
            OFN.lpstrFile = (LPWSTR)filepath;
            OFN.nMaxFileTitle = 100;
            OFN.lpstrFileTitle = (LPWSTR)filename;
            OFN.nMaxFile = 100;
            OFN.lpstrInitialDir = _T(".");
            OFN.lpstrFilter = filter;
            // 열기 대화상자를 보여준다.
            if (GetOpenFileName(&OFN))
            {
                vector<string> vecStrBuffer;
                // 기존 모두 초기화
                vecStorageText.clear();
                vectext.clear();
                iLine = 0;
                iCount = 0;

                TCHAR* CBuffer;

                vecStrBuffer = OutFromFile(OFN.lpstrFile, hWnd, false);

                iLine = vecStrBuffer.size() - 1;

                for (int i = 0; i < vecStrBuffer.size(); i++)
                {
                    for (int j = 0; j < vecStrBuffer.at(i).size(); j++)
                    {
                        iCount++;
                    }
                }

                for (int i = 0; i < vecStrBuffer.size(); i++)
                {
                    if (i == vecStrBuffer.size() - 1)
                    {
                        vectext.push_back(vecStrBuffer.at(i));
                    }

                    else
                        vecStorageText.push_back(vecStrBuffer.at(i));
                }
            }

           InvalidateRect(hWnd, NULL, TRUE);
           CreateCaret(hWnd, NULL, 3, 15);
           bTextUpdate = true;
           UpdateWindow(hWnd);
        }
            break;

        case ID_SAVE:
        {
            memset(&SFN, 0, sizeof(OPENFILENAME));
            SFN.lStructSize = sizeof(OPENFILENAME);
            SFN.hwndOwner = hWnd;
            SFN.lpstrFilter = filter;
            SFN.lpstrFile = lpstrFile;
            SFN.nMaxFile = 256;
            SFN.lpstrInitialDir = _T(".");
            // 저장 대화상자를 보여준다.



            if (GetSaveFileName(&SFN) != 0)
            {
                int iLen = 0;

                // 경로에서 파일 이름 길이 구하기
                for (int i = lstrlen(lpstrFile); i > 0; i--)
                {
                    if (lpstrFile[i - 1] == '\\')
                    {
                        break;
                    }

                    else
                    {
                        iLen++;
                    }
                }

                char FileName[15];
                memset(FileName, 0, sizeof(FileName));


                // 파일 이름을 길이 만큼 새 변수에 저장한다.
                for (int i = 1; i <= iLen; i++)
                {
                    FileName[iLen - i] = lpstrFile[(lstrlen(lpstrFile) - i)];
                }

                // .txt 출력 글자를 붙여준다.
                FileName[iLen] = '.';
                FileName[iLen + 1] = 't';
                FileName[iLen + 2] = 'x';
                FileName[iLen + 3] = 't';


                FILE* fSaveFile;
                fopen_s(&fSaveFile, FileName, "wt");

                if (fSaveFile)
                {
                    TCHAR buffer[500];
                    memset(buffer, 0, sizeof(buffer));

                    int ibufferLastIndex = 0;

                    // 엔터키를 누를 시 저장되지만, 저장 되지 못한 부분인 현재 작업중인 vectext를 넣어준다.
                    vecStorageText.push_back(vectext.at(vectext.size() - 1));

                    // 버퍼에 문자 읽어들이기
                    for (int i = 0; i < vecStorageText.size(); ++i)
                    {
                        for (int j = 0; j < vecStorageText.at(i).size(); ++j)
                        {

                            buffer[j + ibufferLastIndex] = vecStorageText.at(i).at(j);


                            if (j + 1 >= vecStorageText.at(i).size())
                            {
                                ibufferLastIndex = j + ibufferLastIndex + 1;
                            }

                        }

                        buffer[ibufferLastIndex] = CHAR('\n');
                        ibufferLastIndex += 1;

                    }
                    _fputts(buffer, fSaveFile);
                }

                fclose(fSaveFile);
            }

            InvalidateRect(hWnd, NULL, TRUE);
            CreateCaret(hWnd, NULL, 3, 15);
            bTextUpdate = true;
            UpdateWindow(hWnd);
            break;
        }

        case ID_FONT:
            memset(&FONT, 0, sizeof(CHOOSEFONT));
            FONT.lStructSize = sizeof(CHOOSEFONT);
            FONT.hwndOwner = hWnd;
            FONT.lpLogFont = &LogFont;
            FONT.Flags = CF_EFFECTS | CF_SCREENFONTS;
            if (ChooseFont(&FONT) != 0)
            {
                HDC hdc = GetDC(hWnd);
                fColor = FONT.rgbColors;
            }
            InvalidateRgn(hWnd, NULL, TRUE);
            CreateCaret(hWnd, NULL, 3, 15);
            bTextUpdate = true;
            UpdateWindow(hWnd);
            break;
            // 단어를 바꿔준다.
        case ID_CH_SPELLING:
        {
            TCHAR cFilePath[100];
            memset(cFilePath, 0, sizeof(cFilePath));

            if (GetCurrentDirectory(sizeof(cFilePath), cFilePath) > 0)
            {
                vector<string> vecNounBuffer;

                cFilePath[lstrlen(cFilePath)] = '\\';
                cFilePath[lstrlen(cFilePath)] = 'n';
                cFilePath[lstrlen(cFilePath)] = 'o';
                cFilePath[lstrlen(cFilePath)] = 'u';
                cFilePath[lstrlen(cFilePath)] = 'n';
                cFilePath[lstrlen(cFilePath)] = '.';
                cFilePath[lstrlen(cFilePath)] = 't';
                cFilePath[lstrlen(cFilePath)] = 'x';
                cFilePath[lstrlen(cFilePath)] = 't';

                // 명사 파일에서 명사 목록을 불러온다.
                vecNounBuffer = OutFromFile(cFilePath, hWnd, false);

                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_CH_SPELLING), hWnd, CH_SPELLING);

                // 유니코드를 변환
                wstring sText_Temp(g_CH_SPELLING_Temp);
                string sTextWideTemp(sText_Temp.begin(), sText_Temp.end());

                // 부분 변환 부분 찾기
                bool bFind;
                POINT StorReplacePos;
                POINT TextReplacePos;
                string sNoun;

                // 변수 초기화
                bFind = false;
                StorReplacePos.x = 0;
                StorReplacePos.y = 0;
                TextReplacePos.x = 0;
                TextReplacePos.y = 0;

                int iSameCount = 0;

                if (sTextWideTemp.size() > 0 && sTextWideTemp.size() <= 2)
                {
                    MessageBox(hWnd, L"글자 수가 3 이상 단어만 검색할 수 있습니다.", L"주의", MB_OK);
                    InvalidateRect(hWnd, NULL, TRUE);
                    CreateCaret(hWnd, NULL, 3, 15);
                    bTextUpdate = true;
                    break;
                }
                    

                if (sTextWideTemp.size() > 2)
                {
                    // 가장 비슷한 명사를 검색한다.
                    for (int i = 0; i < vecNounBuffer.size(); i++)
                    {
                        for (int j = 0; j < vecNounBuffer.at(i).size(); j++)
                        {
                            if (vecNounBuffer.at(i).at(j) == sTextWideTemp.at(iSameCount))
                            {
                                iSameCount++;

                                // 절반 + 1 만큼 같다면 바꿔준다.
                                if (iSameCount == sTextWideTemp.size() / 2 + 1)
                                {
                                    sNoun = vecNounBuffer.at(i);
                                    bFind = true;
                                    break;
                                }
                            }

                            else
                                iSameCount = 0;
                        }
                    }

                    // 관련 단어를 찾지 못했다면 빠져나온다.
                    if (!bFind)
                    {
                        MessageBox(hWnd, L"관련 단어를 찾을 수 없습니다.", L"찾을 수 없음", MB_OK);
                        InvalidateRect(hWnd, NULL, TRUE);
                        CreateCaret(hWnd, NULL, 3, 15);
                        bTextUpdate = true;
                        break;
                    }

                    iSameCount = 0;

                    // 버퍼에 입력한 문자열이 있는지 검사한다.
                    for (int i = 0; i < vecStorageText.size(); i++)
                    {
                        for (int j = 0; j < vecStorageText.at(i).size(); j++)
                        {
                            if (vecStorageText.at(i).at(j) == sTextWideTemp.at(iSameCount))
                            {
                                iSameCount++;

                                if (iSameCount == sTextWideTemp.size() - 1)
                                {
                                    StorReplacePos.x = j - sTextWideTemp.size() + 2;
                                    StorReplacePos.y = i;
                                    bFind = true;
                                }
                            }

                            else
                                iSameCount = 0;
                        }
                    }
                    if (bFind)
                    {
                        vecStorageText.at(StorReplacePos.y).erase(StorReplacePos.x, sTextWideTemp.size());
                        vecStorageText.at(StorReplacePos.y).insert(StorReplacePos.x, sNoun);
                        InvalidateRect(hWnd, NULL, TRUE);
                        bTextUpdate = true;
                        CreateCaret(hWnd, NULL, 3, 15);
                        break;
                    }

                    // 기존 버퍼에 없으므로 재탐색을 위해 다시 초기화
                    else
                    {
                        iSameCount = 0;

                        // 실시간 입력 버퍼에 해당 입력한 문자열이 있는지 검사한다.
                        for (int i = 0; i < vectext.size(); i++)
                        {
                            for (int j = 0; j < vectext.at(i).size(); j++)
                            {
                                if (vectext.at(i).at(j) == sTextWideTemp.at(iSameCount))
                                {
                                    iSameCount++;

                                    if (iSameCount == sTextWideTemp.size() - 1)
                                    {
                                        TextReplacePos.x = j - sTextWideTemp.size() + 2;
                                        TextReplacePos.y = i;
                                        bFind = true;
                                        break;
                                    }
                                }

                                else
                                    iSameCount = 0;
                            }
                        }

                        if (bFind)
                        {
                            vectext.at(TextReplacePos.y).erase(TextReplacePos.x, sTextWideTemp.size());
                            vectext.at(TextReplacePos.y).insert(TextReplacePos.x, sNoun);
                            InvalidateRect(hWnd, NULL, TRUE);
                            bTextUpdate = true;
                            CreateCaret(hWnd, NULL, 3, 15);
                            break;
                        }


                    }
               
                }
            }
        }
        InvalidateRect(hWnd, NULL, TRUE);
        CreateCaret(hWnd, NULL, 3, 15);
        bTextUpdate = true;
        break;

        case ID_HIGHPEN:
            iDrawCount = -1;
            InvalidateRect(hWnd, NULL, TRUE);
            bTextUpdate = true;
            break;


        case ID_LineSpacing:
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_LINESPACING), hWnd, LINESPACING);
            InvalidateRect(hWnd, NULL, TRUE);
            CreateCaret(hWnd, NULL, 3, 15);
            bTextUpdate = true;
            break;

        case IDM_ABOUT:
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        }
        break;

    case WM_KEYDOWN:
    {
        if (wParam == VK_BACK)
        {
            if (!vectext.at(vectext.size() - 1).empty())
            {
                vectext.at(vectext.size() - 1).pop_back();
                iCount--;

                InvalidateRect(hWnd, NULL, TRUE);

                bTextUpdate = true;
            }

            else
            {
                if (iLine != 0)
                {
                    vectext.clear();
                    vectext.push_back(vecStorageText.at(vecStorageText.size() - 1));
                    vecStorageText.pop_back();
                    --iLine;
                    InvalidateRect(hWnd, NULL, TRUE);
                    bTextUpdate = true;
                }

            }


        }

        if (wParam == VK_RETURN)
        {
            HDC hdc = GetDC(hWnd);

            POINT tPos;
            GetCaretPos(&tPos);

            RECT rc = { tPos.x, tPos.y, tPos.x + 5, tPos.y + g_fOffsetY };

            // Caret 부분이 남지 않도록 지워준다.
            FillRect(hdc, &rc, g_WindowBrush);

            if (!(vecStorageText.empty() && vectext.empty() && iLine > MAX_LINE))
            {
                iLine++;
                iCount++;
                vecStorageText.push_back(vectext.at(vectext.size() - 1));
                vectext.clear();
                vectext.resize(1);
            }
        }


        break;
    }
   
    // 그림 그리기 시작 

    case WM_LBUTTONDOWN:            // 마우스 누름 
    {
        if (iDrawCount == 0)
        {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);

            //	좌표를 얻어와야 한다.
            g_iY = HIWORD(lParam);
            g_iX = LOWORD(lParam);
            //	마우스가 눌려 있다.
            g_bDraw = true;
            
        }

        else
        {
            iDrawCount = 0;
            g_bDraw = false;
        }



        break;

    }
    break;


    case WM_PAINT:
    {
        ShowCaret(hWnd);
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        if (iHour > 5 && iHour < 19)
        {
            SetBkColor(hdc, RGB(255,255,255));
        }

        else
        {
            SetBkColor(hdc, RGB(0, 0, 0));
        }
        

        // 폰트 관련 함수
        hFont = CreateFontIndirect(&LogFont);
        OldFont = (HFONT)SelectObject(hdc, hFont);
        SetTextColor(hdc, fColor);

        for (int count = 0; count < vectext.size(); count++)
        {
            if (bTextUpdate)
            {
                for (int i = 0; i < vecStorageText.size(); i++)
                {
                    GetTextExtentPointA(hdc, vecStorageText.at(i).c_str(), vecStorageText.at(i).size(), &size);
                    SetCaretPos(size.cx + g_fOffsetX, iLine * g_fOffsetY);
                    TextOutA(hdc, g_fOffsetX, g_fOffsetY * i, vecStorageText.at(i).c_str(), vecStorageText.at(i).length());
                }
                bTextUpdate = false;
            }
            GetTextExtentPointA(hdc, vectext.at(vectext.size() - 1).c_str(), vectext.at(vectext.size() - 1).size(), &size);
            SetCaretPos(size.cx + g_fOffsetX, iLine * g_fOffsetY);
            TextOutA(hdc, g_fOffsetX, g_fOffsetY * iLine, vectext.at(count).c_str(), vectext.at(count).length());
        }


        SelectObject(hdc, OldFont);
        DeleteObject(hFont);


        EndPaint(hWnd, &ps);
    }
    break;

    case WM_CHAR:
    {
        if (iLine > MAX_LINE)
        {
            MessageBox(hWnd, L"입력 가능 문자열 초과", L"메모장", MB_OK);
            iLine = MAX_LINE;
            bInputAble = false;
            DestroyCaret();
        }
        else
        {
            if (vectext.size() - 5 <= iCount)
            {
                vectext.reserve(vectext.size() * 2);
            }

            // 엔터키의 숫자는 무시한다.
            if ((!(wParam == VK_RETURN || wParam == VK_BACK)) && bInputAble)
            {
                vectext.at(vectext.size() - 1).push_back(((TCHAR)wParam));
                iCount++;
            }

        }
        InvalidateRect(hWnd, NULL, FALSE);
        break;
    }

    //	그림 그리는 중( 마우스가 눌린 상태에서만 )
    case WM_MOUSEMOVE:		//	마우스 이동
    {
        if (g_bDraw == true)	// 마우스가 눌려있는가?
        {
            // DC를 얻어온다.
            HDC hDC = GetDC(hWnd);

            HPEN MyPen = CreatePen(PS_SOLID, 5, RGB(255, 255, 121));
            HPEN hPrevPen = (HPEN)SelectObject(hDC, MyPen);

            //	선의 시작점을 셋팅한다.( 이전 좌표점이어야 한다. )
            hPrevPen = (HPEN)SelectObject(hDC, MyPen);
            MoveToEx(hDC, g_iX, g_iY, NULL);

            //	현재 마우스 좌표를 얻어온다.
            g_iY = HIWORD(lParam);
            g_iX = LOWORD(lParam);

            //	선을 긋는다.
            LineTo(hDC, g_iX, g_iY);

            //	DC를 release 시킨다.
            ReleaseDC(hWnd, hDC);
        }

        break;
    }

    //	그림 그리기 끝
    case WM_LBUTTONUP:		//	마우스 
    {
        //	마우스가 눌려있다가 떼진 경우이므로, 그리는 동작이 끝났음을 의미
        g_bDraw = false;

        break;
    }

    case WM_DESTROY:
        HideCaret(hWnd);
        DestroyCaret();
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

INT_PTR CALLBACK CH_SPELLING(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (iMsg)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_CHANGE:
            GetDlgItemText(hDlg, IDC_EDIT1, g_CH_SPELLING_Temp, 10);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        case ID_CANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return(INT_PTR)TRUE;
        }
        break;
    case WM_CLOSE:
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK LINESPACING(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    TCHAR buffer[10];
    switch (iMsg)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_OK:
            GetDlgItemText(hDlg, IDC_EDIT1, buffer, 10);
            g_fOffsetY = _wtoi(buffer);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        case ID_CANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    case WM_CLOSE:
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}



// 파일을 읽어온다.
vector<string> OutFromFile(TCHAR filename[], HWND hWnd, bool bTextout)
{
    FILE* fReadFile;
    HDC hdc;
    int iLine;
    TCHAR buffer[500];
    vector<string> vecbufferStorage;
    iLine = 0;
    hdc = GetDC(hWnd);
#ifdef _UNICODE
    _tfopen_s(&fReadFile, filename, _T("r, ccs = UNICODE"));
#else
    _tfopen_s(&fReadFile, filename, _T("r"));
#endif
    while (_fgetts(buffer, 100, fReadFile) != NULL)
    {
        if (buffer[_tcslen(buffer) - 1] == _T('\n'))
            buffer[_tcslen(buffer) - 1] = NULL;
        if (bTextout)
        {
            TextOut(hdc, 0, iLine * 20, buffer, _tcslen(buffer));
            iLine++;
        }
        char cTemp[500];
        WideCharToMultiByte(CP_ACP, 0, buffer, 500, cTemp, 500, NULL, NULL);
        vecbufferStorage.resize(vecbufferStorage.size() + 1);
        vecbufferStorage.at(vecbufferStorage.size() - 1).append(cTemp);
        
    }


    fclose(fReadFile);
    ReleaseDC(hWnd, hdc);

    return vecbufferStorage;
}



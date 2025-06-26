// include the basic windows header files and the Direct3D header files

#include "DirectX.h"

const wchar_t* APPTITLE = L"DirectX 11";


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    HWND hwnd;
    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = APPTITLE;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Error registering class",
            L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    RECT rc = { 0, 0, Width, Height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW | WS_CAPTION, FALSE);

    hwnd = CreateWindowEx(NULL,
        APPTITLE,
        APPTITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hwnd)
    {
        MessageBox(NULL, L"Error creating window",
            L"Error", MB_OK | MB_ICONERROR);
        return false;
    }
    

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // set up and initialize Direct3D
    if (Game_Init(hwnd) == false)
        return false;

    // enter the main loop:
    MSG msg;
    while (gameover == false)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                break;
        }

        // Run game code here
        Game_Run();
    }

    // clean up DirectX 
    Game_End();

    return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_SYSKEYDOWN:
        Keyboard::ProcessMessage(message, wParam, lParam);
        break;

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        Mouse::ProcessMessage(message, wParam, lParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


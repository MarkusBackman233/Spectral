#include "WindowsManager.h"
#include <Windows.h>

#include "RenderManager.h"
#include "src/IMGUI/imgui.h"
#include "InputManager.h"
#include "Editor.h"
#include "iRender.h"

#define WindowClassName L"SpectralWindowClass"


#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif



HINSTANCE WindowsManager::gInstance;
HWND WindowsManager::gWindowHandle;

HRESULT WindowsManager::CreateDesktopWindow()
{

    if (gInstance == NULL)
        gInstance = (HINSTANCE)GetModuleHandle(NULL);

    HICON hIcon = NULL;
    WCHAR szExePath[MAX_PATH];
    GetModuleFileName(NULL, szExePath, MAX_PATH);

    // If the icon is NULL, then use the first one found in the exe
    if (hIcon == NULL)
        hIcon = ExtractIcon(gInstance, szExePath, 0);

    // Register the windows class
    WNDCLASS wndClass;
    wndClass.style = CS_DBLCLKS;
    wndClass.lpfnWndProc = WindowsManager::StaticWindowProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = gInstance;
    wndClass.hIcon = hIcon;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = WindowClassName;

    if (!RegisterClass(&wndClass))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            return HRESULT_FROM_WIN32(dwError);
    }

    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;

    HMENU hMenu = NULL;

    int nDefaultWidth = 1280;
    int nDefaultHeight = 720;
    RECT rc{};
    SetRect(&rc, 0, 0, nDefaultWidth, nDefaultHeight);
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, (hMenu != NULL) ? true : false);

    // Create the window for our viewport.
    gWindowHandle = CreateWindow(
        WindowClassName,
        L"Spectral",
        WS_OVERLAPPEDWINDOW,
        x, y,
        (rc.right - rc.left), (rc.bottom - rc.top),
        0,
        hMenu,
        gInstance,
        0
    );
    DragAcceptFiles(gWindowHandle, TRUE);
    if (gWindowHandle == NULL)
    {
        DWORD dwError = GetLastError();
        return HRESULT_FROM_WIN32(dwError);
    }

    RAWINPUTDEVICE Rid[1];
    Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    Rid[0].dwFlags = RIDEV_INPUTSINK;
    Rid[0].hwndTarget = GetWindowHandle();
    RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

    return S_OK;
}

HWND WindowsManager::GetWindowHandle() const
{
    return gWindowHandle;
}

HINSTANCE WindowsManager::GetWinInstance() const
{
    return gInstance;
}

Math::Vector2 WindowsManager::GetWindowSize()
{
    RECT desktop;
    GetClientRect(gWindowHandle, &desktop);
    
    return Math::Vector2(static_cast<float>(desktop.right - desktop.left), static_cast<float>(desktop.bottom - desktop.top));
}

void WindowsManager::CreateWindowsLoop(std::function<void()> functionBody)
{
    MSG msg{};
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            functionBody();
        }
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowsManager::StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;
    switch (uMsg)
    {
    case WM_CLOSE:
    {
        HMENU hMenu;
        hMenu = GetMenu(hWnd);
        if (hMenu != NULL)
        {
            DestroyMenu(hMenu);
        }
        DestroyWindow(hWnd);
        UnregisterClass(WindowClassName, WindowsManager::gInstance);
        return 0;
    }
    case WM_SYSKEYDOWN:
        InputManager::GetInstance()->RegisterInput(static_cast<InputId>(wParam), InputManager::KeyState::Pressed);
        break;
    case WM_SYSKEYUP:
        InputManager::GetInstance()->RegisterInput(static_cast<InputId>(wParam), InputManager::KeyState::Pressed);
        break;
    case WM_KEYDOWN:
        InputManager::GetInstance()->RegisterInput(static_cast<InputId>(wParam), InputManager::KeyState::Pressed);
        break;
    case WM_KEYUP:
        InputManager::GetInstance()->RegisterInput(static_cast<InputId>(wParam), InputManager::KeyState::Released);
        break;
    case WM_LBUTTONDOWN:
        InputManager::GetInstance()->RegisterInput(InputId::Mouse1, InputManager::KeyState::Pressed);
        break;
    case WM_LBUTTONUP:
        InputManager::GetInstance()->RegisterInput(InputId::Mouse1, InputManager::KeyState::Released);
        break;
    case WM_RBUTTONDOWN:
        InputManager::GetInstance()->RegisterInput(InputId::Mouse2, InputManager::KeyState::Pressed);
        break;
    case WM_RBUTTONUP:
        InputManager::GetInstance()->RegisterInput(InputId::Mouse2, InputManager::KeyState::Released);
        break;
    case WM_MBUTTONDOWN:
        InputManager::GetInstance()->RegisterInput(InputId::Mouse3, InputManager::KeyState::Pressed);
        break;
    case WM_MBUTTONUP:
        InputManager::GetInstance()->RegisterInput(InputId::Mouse3, InputManager::KeyState::Released);
        break;
    case WM_INPUT:
    {
        UINT dwSize = sizeof(RAWINPUT);
        static BYTE lpb[sizeof(RAWINPUT)];

        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

        RAWINPUT* raw = (RAWINPUT*)lpb;

        if (raw->header.dwType == RIM_TYPEMOUSE)
        {
            int xPosRelative = raw->data.mouse.lLastX;
            int yPosRelative = raw->data.mouse.lLastY;
            InputManager::GetInstance()->RegisterMousePosition(Math::Vector2((float)xPosRelative, (float)yPosRelative));

        }
        break;
    }
    break;
    case WM_KILLFOCUS:
    case WM_SYSCOMMAND:
    case SC_MOUSEMENU:
    case WM_INITMENUPOPUP:
    case WM_INITMENU:
        InputManager::GetInstance()->ClearInputs();
        break;
    case WM_DESTROY:
    {

#ifdef _DEBUG
        ID3D11Debug* d3dDebug = nullptr;
        ThrowIfFailed(Render::GetDevice()->QueryInterface(IID_PPV_ARGS(&d3dDebug)));
        ID3D11InfoQueue* d3dInfoQueue = nullptr;
        ThrowIfFailed(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue));

#endif
        RenderManager::GetInstance()->GetDeviceResources()->ReleaseBackBuffer();

#ifdef _DEBUG
        d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
        d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
#endif

        PostQuitMessage(0);
        break;
    }


    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;
        if (width == 0 || height == 0)
        {
            return 0;
        }
        RenderManager::GetInstance()->OnWindowResize();
        break;
    }
#ifdef EDITOR
    case WM_DROPFILES:
    {
        HDROP hDrop = (HDROP)wParam;
        UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

        for (UINT i = 0; i < fileCount; ++i) {
            TCHAR filePath[MAX_PATH];
            DragQueryFile(hDrop, i, filePath, MAX_PATH);
            int bufferSize = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, NULL, 0, NULL, NULL);
            std::string filename(bufferSize, '\0');
            WideCharToMultiByte(CP_UTF8, 0, filePath, -1, filename.data(), bufferSize, NULL, NULL);
            Editor::GetInstance()->HandleDropFile(filename);
        }

        DragFinish(hDrop);
        break;
    }
#endif // EDITOR
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

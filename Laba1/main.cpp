#include "resource.h"
#include <windows.h>
#include <windowsx.h>
#include <cmath>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

HINSTANCE g_hInstance;

struct Sprite {
    int x;
    int y;
    int width;
    int height;
    float angle;
    HICON hIcon;
};

Sprite g_sprite;

const int MOVE_SPEED = 10;
const float ROTATION_SPEED = 15.0f;
const float PI = 3.14159265f;

bool g_isDragging = false;
POINT g_dragOffset;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInstance = hInstance;

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"SpriteWindow";
    wc.hIconSm = wc.hIcon;

    if (!RegisterClassEx(&wc)) {
        return EXIT_FAILURE;
    }

    HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        L"Laba 1",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return EXIT_FAILURE;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

void CreateSprite(int x, int y, int width, int height, float angle, int idIcon) {
    g_sprite.x = x;
    g_sprite.y = y;
    g_sprite.width = width;
    g_sprite.height = height;
    g_sprite.angle = angle;
    g_sprite.hIcon = (HICON)LoadImage(
        g_hInstance,
        MAKEINTRESOURCE(idIcon),
        IMAGE_ICON,
        g_sprite.width,
        g_sprite.height,
        LR_DEFAULTCOLOR
    );
}

void CheckBorder(HWND hwnd) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    if (g_sprite.x < 0)
        g_sprite.x = 0;
    if (g_sprite.x + g_sprite.width > clientRect.right)
        g_sprite.x = clientRect.right - g_sprite.width;

    if (g_sprite.y < 0)
        g_sprite.y = 0;
    if (g_sprite.y + g_sprite.height > clientRect.bottom)
        g_sprite.y = clientRect.bottom - g_sprite.height;
}

void MoveSpriteByWheel(int delta, bool shiftPressed) {
    if (delta > 0) {
        if (shiftPressed) {
            g_sprite.x -= MOVE_SPEED;
            g_sprite.y -= MOVE_SPEED;
        }
        else {
            g_sprite.x += MOVE_SPEED;
            g_sprite.y -= MOVE_SPEED;
        }
    }
    else if (delta < 0) {
        if (shiftPressed) {
            g_sprite.x += MOVE_SPEED;
            g_sprite.y += MOVE_SPEED;
        }
        else {
            g_sprite.x -= MOVE_SPEED;
            g_sprite.y += MOVE_SPEED;
        }
    }
}

void MoveSpriteByKey(HWND hwnd, int x, int y) {
    g_sprite.x += x;
    g_sprite.y += y;
    CheckBorder(hwnd);
}

void DrawSprite(HDC hdc) {
    float rad = g_sprite.angle * PI / 180.0f;

    XFORM xform;
    xform.eM11 = cos(rad);
    xform.eM12 = sin(rad);
    xform.eM21 = -sin(rad);
    xform.eM22 = cos(rad);

    xform.eDx = g_sprite.x + g_sprite.width / 2.0f;
    xform.eDy = g_sprite.y + g_sprite.height / 2.0f;

    SetGraphicsMode(hdc, GM_ADVANCED);
    SetWorldTransform(hdc, &xform);

    DrawIconEx(hdc, -g_sprite.width / 2, -g_sprite.height / 2, g_sprite.hIcon, g_sprite.width, g_sprite.height, 0, NULL, DI_NORMAL);

    ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
    {
        CreateSprite(350, 250, 64, 64, 0.0f, IDI_ICON1);
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &ps.rcPaint, bgBrush);
        DeleteObject(bgBrush);

        DrawSprite(hdc);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_MOUSEWHEEL:
    {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        bool shiftPressed = (GET_KEYSTATE_WPARAM(wParam) & MK_SHIFT) != 0;

        MoveSpriteByWheel(delta, shiftPressed);
        CheckBorder(hwnd);
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }
    case WM_KEYDOWN:
    {
        bool moved = true;
        switch (wParam) {
        case 'W':
            MoveSpriteByKey(hwnd, 0, -MOVE_SPEED);
            break;
        case 'S':
            MoveSpriteByKey(hwnd, 0, MOVE_SPEED);
            break;
        case 'A':
            MoveSpriteByKey(hwnd, -MOVE_SPEED, 0);
            break;
        case 'D':
            MoveSpriteByKey(hwnd, MOVE_SPEED, 0);
            break;
        case VK_UP:
            MoveSpriteByKey(hwnd, 0, -MOVE_SPEED);
            break;
        case VK_DOWN:
            MoveSpriteByKey(hwnd, 0, MOVE_SPEED);
            break;
        case VK_LEFT:
            MoveSpriteByKey(hwnd, -MOVE_SPEED, 0);
            break;
        case VK_RIGHT:
            MoveSpriteByKey(hwnd, MOVE_SPEED, 0);
            break;
        case 'R':
            g_sprite.angle += ROTATION_SPEED;

            if (g_sprite.angle >= 360.0f)
                g_sprite.angle -= 360.0f;

            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case 'Q':
            if (GetKeyState(VK_CONTROL) & 0b1000'0000'0000'0000) {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            break;
        default:
            moved = false;
            break;
        }

        if (moved) {
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
    }
    case WM_LBUTTONDOWN:
    {
        int mouseX = GET_X_LPARAM(lParam);
        int mouseY = GET_Y_LPARAM(lParam);

        RECT spriteRect = { g_sprite.x, g_sprite.y, g_sprite.x + g_sprite.width, g_sprite.y + g_sprite.height };

        if (PtInRect(&spriteRect, { mouseX, mouseY })) {
            g_isDragging = true;
            g_dragOffset.x = mouseX - g_sprite.x;
            g_dragOffset.y = mouseY - g_sprite.y;
            SetCapture(hwnd);
        }
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (g_isDragging) {
            int mouseX = GET_X_LPARAM(lParam);
            int mouseY = GET_Y_LPARAM(lParam);
            g_sprite.x = mouseX - g_dragOffset.x;
            g_sprite.y = mouseY - g_dragOffset.y;
            CheckBorder(hwnd);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        g_isDragging = false;
        ReleaseCapture();
        break;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        PostQuitMessage(EXIT_SUCCESS);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
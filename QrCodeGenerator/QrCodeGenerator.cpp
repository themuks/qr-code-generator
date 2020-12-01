// QrCodeGenerator.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "QrCodeGenerator.h"
#include <iostream>
#include <string>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define CORRECTION_LEVEL "M"
#define VERSION 10
#define MAX_BYTES 5 //1728/8
#define COLS 57
#define ROWS 57

#define MAX_LOADSTRING 100
#define static1 1
#define edit1 2
#define button1 3
#define button2 4

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

BOOLEAN isGenerated = 0;                        // true, если qr-код сгенерирован
HWND mainEdit;                                  // основное поле ввода
TCHAR textToCode[4096];                         // текст из поля для ввода

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
void                drawQrCode(HWND hWnd);
void                drawCode(HDC hdc, int x, int y, int width, int height);
void                errhandler(const char*, HWND);
void                copyToClipboardQrCode();
const TCHAR*        stringToWchar(std::string);
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
void                alert(const TCHAR*);
void                error(const TCHAR*);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_QRCODEGENERATOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_QRCODEGENERATOR));

    MSG msg;

    // Цикл основного сообщения:
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
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_QRCODEGENERATOR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_QRCODEGENERATOR);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
      CW_USEDEFAULT, 0, WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        {
        hInst = ((LPCREATESTRUCT)lParam)->hInstance;
        // создаем лейбл Введите текст
            CreateWindow(TEXT("static"), TEXT("Введите текст:"),
                WS_VISIBLE | WS_CHILD | WS_BORDER,
                15, 25, 170, 20,
                hWnd, (HMENU)static1, hInst, NULL
            );
        // создаем поле для ввода текста
            mainEdit = CreateWindow(TEXT("edit"), TEXT(""),
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_MULTILINE,
                200, 15, 385, 40,
                hWnd, (HMENU)edit1, hInst, NULL
            );
        // создаем кнопку Сгенерировать
            CreateWindow(TEXT("button"), TEXT("Сгенерировать"),
                WS_VISIBLE | WS_CHILD | WS_BORDER,
                600, 15, 170, 40,
                hWnd, (HMENU)button1, hInst, NULL
            );
        // создаем кнопку Сохранить изображение
            CreateWindow(TEXT("button"), TEXT("Скопировать результат"),
                WS_VISIBLE | WS_CHILD | WS_BORDER,
                600, 65, 170, 40,
                hWnd, (HMENU)button2, hInst, NULL
            );
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case ID_GENERATE:
            case button1: {
                int bytes = GetWindowText(mainEdit, textToCode, 4096) * 2; // умножаем на 2 т.к. это wchar
                if (bytes > MAX_BYTES) {
                    //alert(stringToWchar("Введите не более " + std::to_string(MAX_BYTES) + " символов"));
                    alert(L"Введите не более");
                    break;
                }
                // Генерируем QR-код
                isGenerated = 1;
                drawQrCode(hWnd);
                //InvalidateRect(hWnd, NULL, true);
            }
                break;
            case button2: {
                if (isGenerated) {
                    copyToClipboardQrCode();
                }
            }
                break;
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
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
            if (isGenerated) {
                drawCode(hdc, 200, 100, 385, 385);
            }
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
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

void drawQrCode(HWND hWnd) {
    HDC hdc;

    hdc = GetDC(hWnd);
    if (hdc != NULL)
    {
        drawCode(hdc, 200, 100, 385, 385);

        // освобождаем контекст
        ReleaseDC(hWnd, hdc);
    }
    else
    {
        // обработка ошибки получения контекста
        error(L"Невозможно получить контекст окна");
    }
}

void alert(const TCHAR* item)
{
    MessageBoxW(NULL, item, L"Message", MB_OK | MB_ICONINFORMATION);
}

void error(const TCHAR* item)
{
    MessageBoxW(NULL, item, L"Message", MB_OK | MB_ICONERROR);
}

void drawCode(HDC hdc, int x, int y, int width, int height) {
    HPEN hWhitePen, hBlackPen, hOldPen;
    HBRUSH hBlackBrush, hOldBrush;

    hWhitePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    hBlackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    hBlackBrush = CreateSolidBrush(RGB(0, 0, 0));

    // нарисовать черную окружность
    SelectObject(hdc, hBlackPen);
    hOldBrush = (HBRUSH)SelectObject(hdc, hBlackBrush);

    // отрисовка qr-кода
    double dx = (double)width / COLS;
    double dy = (double)height / ROWS;

    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < ROWS; j++) {
            if ((i + j) % 2 == 0) {
                int left = x + (int)(i * dx);
                int top = y + (int)(j * dy);
                int right = x + (int)((i + 1) * dx);
                int bottom = y + (int)((j + 1) * dy);
                Rectangle(hdc, left, top, right, bottom);
            }
        }
    }

    // вернуть старый объекты pen и brush в DC
    SelectObject(hdc, hOldBrush);

    // освободить ресурсы
    DeleteObject(hWhitePen);
    DeleteObject(hBlackPen);
    DeleteObject(hBlackBrush);
}

PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
    BITMAP bmp;
    PBITMAPINFO pbmi;
    WORD    cClrBits;

    // Retrieve the bitmap color format, width, and height.  
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
        errhandler("GetObject", hwnd);

    // Convert the color format to a count of bits.  
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
    if (cClrBits == 1)
        cClrBits = 1;
    else if (cClrBits <= 4)
        cClrBits = 4;
    else if (cClrBits <= 8)
        cClrBits = 8;
    else if (cClrBits <= 16)
        cClrBits = 16;
    else if (cClrBits <= 24)
        cClrBits = 24;
    else cClrBits = 32;

    // Allocate memory for the BITMAPINFO structure. (This structure  
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
    // data structures.)  

    if (cClrBits < 24)
        pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
            sizeof(BITMAPINFOHEADER) +
            sizeof(RGBQUAD) * (1 << cClrBits));

    // There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 

    else
        pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
            sizeof(BITMAPINFOHEADER));

    // Initialize the fields in the BITMAPINFO structure.  

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = bmp.bmWidth;
    pbmi->bmiHeader.biHeight = bmp.bmHeight;
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
    if (cClrBits < 24)
        pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

    // If the bitmap is not compressed, set the BI_RGB flag.  
    pbmi->bmiHeader.biCompression = BI_RGB;

    // Compute the number of bytes in the array of color  
    // indices and store the result in biSizeImage.  
    // The width must be DWORD aligned unless the bitmap is RLE 
    // compressed. 
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
        * pbmi->bmiHeader.biHeight;
    // Set biClrImportant to 0, indicating that all of the  
    // device colors are important.  
    pbmi->bmiHeader.biClrImportant = 0;
    return pbmi;
}

void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi,
    HBITMAP hBMP, HDC hDC)
{
    HANDLE hf;                 // file handle  
    BITMAPFILEHEADER hdr;       // bitmap file-header  
    PBITMAPINFOHEADER pbih;     // bitmap info-header  
    LPBYTE lpBits;              // memory pointer  
    DWORD dwTotal;              // total count of bytes  
    DWORD cb;                   // incremental count of bytes  
    BYTE* hp;                   // byte pointer  
    DWORD dwTmp;

    pbih = (PBITMAPINFOHEADER)pbi;
    lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    if (!lpBits)
        errhandler("GlobalAlloc", hwnd);

    // Retrieve the color table (RGBQUAD array) and the bits  
    // (array of palette indices) from the DIB.  
    if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi,
        DIB_RGB_COLORS))
    {
        errhandler("GetDIBits", hwnd);
    }

    // Create the .BMP file.  
    hf = CreateFile(pszFile,
        GENERIC_READ | GENERIC_WRITE,
        (DWORD)0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);
    if (hf == INVALID_HANDLE_VALUE)
        errhandler("CreateFile", hwnd);
    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
    // Compute the size of the entire file.  
    hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
        pbih->biSize + pbih->biClrUsed
        * sizeof(RGBQUAD) + pbih->biSizeImage);
    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;

    // Compute the offset to the array of color indices.  
    hdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) +
        pbih->biSize + pbih->biClrUsed
        * sizeof(RGBQUAD);

    // Copy the BITMAPFILEHEADER into the .BMP file.  
    if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
        (LPDWORD)&dwTmp, NULL))
    {
        errhandler("WriteFile", hwnd);
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
    if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
        + pbih->biClrUsed * sizeof(RGBQUAD),
        (LPDWORD)&dwTmp, (NULL)))
        errhandler("WriteFile", hwnd);

    // Copy the array of color indices into the .BMP file.  
    dwTotal = cb = pbih->biSizeImage;
    hp = lpBits;
    if (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL))
        errhandler("WriteFile", hwnd);

    // Close the .BMP file.  
    if (!CloseHandle(hf))
        errhandler("CloseHandle", hwnd);

    // Free memory.  
    GlobalFree((HGLOBAL)lpBits);
}

void errhandler(const char* text, HWND hwnd) {
    error((LPWSTR)text);
}

void copyToClipboardQrCode() {
    int length = GetWindowText(mainEdit, textToCode, MAX_BYTES);
    alert(textToCode);
}
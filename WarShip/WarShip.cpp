// WarShip.cpp : Визначає точку входу для застосунку.
//Реалізувати гру «Морський бій на безмежному полі» 
//На поле можливо роз-містити 4 однопалубних, 3 двопалубних, 2 трипалубних та 1 чотирьохпалубний корабель. 
//Програма випадковим чином розміщує кораблі, після чого гравець робить постріли у вигляді пари чисел (номер рядку та номер стовпця). \
    Комп'ю-тер повідомляє: убитий корабель, поранений або повз. Після того, як всі кораблі знищені, комп'ютер видає повідомлення \
    про закінчення гри, повідомляє число ходів та пропонує занести результат у таблицю.

#include "framework.h"
#include "WarShip.h"
#include "fstream"
#include "iostream"
#include <ctime>
#include "string"

#define MAX_LOADSTRING 100
//перенайменування макросів
#define IDD_DIALOG_GETNAMES IDD_DIALOG1
#define IDD_DIALOG_SETTINGS IDD_DIALOG2

#define IDB_SHIP2VERT IDB_BITMAP1
#define IDB_SHIP2HORIZ IDB_BITMAP2

#define IDB_SHIP4VERT IDB_BITMAP5
#define IDB_SHIP4HORIZ IDB_BITMAP6

#define IDB_SHIP6VERT IDB_BITMAP7
#define IDB_SHIP6HORIZ IDB_BITMAP8

#define IDB_FIRE IDB_BITMAP9
#define IDB_VOLNA IDB_BITMAP10

#define IDB_TRASHBOXGRAY IDB_BITMAP3
#define IDB_TRASHBOXRED IDB_BITMAP4

//прапори змінної flags
#define CHECK_PLAY_WITH_BOT 0
#define CHECK_GAME_IS_STARTED 1
#define CHECK_SET_ARENA 2
#define CHECK_ORIENT 3 //0-горизонтальна[nums]
#define CHECK_DELETE_SHIP 5
#define CHECK_SHOW_ARENA 4
#define CHECK_SHOW_HISTORY 6

//ідентифікатори кнопок
#define ButtonStart 1
#define ButtonHistory 2
#define ButtonNextSet 3 //кнопка завершення налаштування поля
#define ButtonChangeOrient 4 //кнопка зміни орієнтації корабля
#define ButtonShowMyArena 5 //кнопка зміни орієнтації корабля
#define ButtonSettings 6 //кнопка налаштувань
#define EditHistory 7 //текстове поле історії
#define ButtonClear 8 //кнопка зтерти історію ігор

// Глобальні змінні:
HBITMAP hBitmap;//вивід зображення
HDC hdcMem;// створення контексту пам'яті для бітового блоку зображення
BITMAP bmp;// визначаємо розміри первісного зображення

TEXTMETRIC tm;//метрики тексту
PAINTSTRUCT ps;
HDC hdc;
HINSTANCE hInst;                                // поточний екземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовку
WCHAR szWindowClass[MAX_LOADSTRING];            // им'я класу головного вікна

//прапори
char flags = 0;
// ||| x //кільк стовпців
int numKol = 5;
// = y //кільк рядків
int numRow = 3;
// ||| x //кільк стовпців попередньої гри
int prevnumKol = 5;
// = y //кільк рядків попередньої гри
int prevnumRow = 3;
//початкова висота ширина програми
int height = 600, width = 500;
//висота/ширина інформаційної області під час гри
int heightHeader = 80, widthHeader = 50;
//позиція курсору при кліку
int posX = 0, posY = 0;
//розмір клітинки поля
int CellSide = 30;
//відсоток кораблів від усього поля
int percentAllShips = 33;
//всього кораблів
int numShips = 0;
//поточний час
time_t tnow = 0;
//час системи на початку гри
time_t TimeStart = 0;
//скільки пройшло часу
time_t TimeEnd = 0;
//текст історії
char* CharTextHistory;

//межі ігрових полів
RECT MainRect1, MainRect2;

//масиви клітинок на полях
RECT** rPole1 ;
RECT** rPole2 ;
//поле "кнопки" смітника
RECT TrashBox;

//структура пункту меню(вибір який корабель ставити)
struct MySetMenu {
    // пункт меню(область)
    RECT* SetMenuRect;
    // чи вибраний пункт меню (для малювання контуру)
    bool* focus;
    //який пункт вибраний (-1) - жоден
    int nowFocused = -1;
    //кількість пунктів
    int numSection = 0;
};
//структура пункту меню(вибір який корабель ставити)
MySetMenu SetShipPosMenu;

//мапи кораблів
struct MyShipMap {
    /*
    2111000211
    0000020000
    0000010000
    0020000020
    */
    //0-порожньо 1/2-корабель
    int** myarena ;
    //розмір корабля розташованого в цій клітинці
    int** ShipSize;
    //порядковий номер корабля що розташований там
    int** ShipNum;
    //мапа власних пострілів(1-постріл/2-влучив)
    int** oponentarena ;
};

//інформація про кораблі
struct MyShips
{
    //розмір корабля
    int size = 0;
    //кількість кораблів(такого розміру)
    int nums = 0;
    //кількість кораблів залишилось
    int zalishoknums = 0;

    //0-горизонтальна[nums]
    bool* orientation = 0;
    //кількість влучань в корабель
    int* damage;
    // корабель потоплений
    bool* dead;
    //позиція "носа" корабля 
    int* posKols, *posRows;
};

//структура гравців
struct MyPlayers
{
    MyPlayers() {
        memset(name, 0, sizeof(name));//обнулення
        wcscpy_s(name, L"Player");//стандартне ім'я
    }
    //ім'я
    wchar_t name[255];
    //найбільший корабель
    int BiggestShip = 1;
    //кількість влучань
    int hits = 0;
    //кількість потоплених кораблів
    int Kills = 0;
    //лічильник пострілів
    int CountShot = 0;
    //хід цього гравця
    bool IsFocus = false;
    //завершив розставляти кораблі/іншу дію
    bool Ready = false;
    //масив кораблей
    MyShips* Ships;
    //мапи
    MyShipMap ShipMap;
};
MyPlayers Player1;
MyPlayers Player2;
//вибраний гравець
MyPlayers* PlayerSetNow = &Player1;
MyPlayers* PlayerOponent = &Player2;


//структура для створення кнопок на головному вікні
struct CONTROLS
{
    HWND hControl;
    int id;// ідентифікатор окна 
};
//кнопка початку гри
CONTROLS BStart;
//кнопка відриття файлу історії ігор
CONTROLS Bhistory;
//кнопка завершення налаштування поля
CONTROLS BnextSet;
//кнопка зміни орієнтації корабля
CONTROLS BChangeOrient;
//кнопка показати свою арену(якщо 2 гравця, аби інший не бачив)
CONTROLS BShowMyArena;
//кнопка налаштувань
CONTROLS BSettings;
//Текстове поле EditHistory
CONTROLS EHistory;
//кнопка зтерти історію ігор
CONTROLS BClear;

//вивід з файлу
std::ifstream out;
//запис у файл
std::ofstream in;

//2 змінні для відкриття блокноту
PROCESS_INFORMATION notepi;
STARTUPINFO notesi;//

//інформація про знадений файл UNICODE
WIN32_FIND_DATA Wfd;
//хендл шуканого файлу
HANDLE hFoundeFile;

//хендл меню
HMENU hMenu;

//ім'я файлу з іcторією ігор
WCHAR LogfileName[256] = L"WarShipOAB219\\log.txt";
//ім'я файлу з налаштуваннями
WCHAR SettingsName[256] = L"WarShipOAB219\\settings.txt";
//ім'я каталогу з іcторією і налаштуваннями
WCHAR LogcatalogName[256] = L"WarShipOAB219";
//змінна для відкриття файлу історії
TCHAR ComandLine[256] = _T("notepad ");

/*/FOR H SCROLBAR*/
SCROLLINFO HScrolInf;
/*/FOR V SCROLBAR*/
SCROLLINFO VScrolInf;

// Обголошення функцій:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    GetNameProc(HWND, UINT, WPARAM, LPARAM);//обробник подій для діалогу отримання імен гравців
INT_PTR CALLBACK    ChangeSettingsProc(HWND, UINT, WPARAM, LPARAM);//обробник подій для діалогу зміни налаштувань

//перевірка наявності директоріі з логами
void checkLogDirectory();
//записати інформацію про налаштування за замовчуванням
void SetFileSettingsText();
//взяти інформацію про налаштування з файлу
void GetFileSettings();
//сховати меню з кнопок
void HideMenu();
//показати меню
void ShowMenu();
//отримати юзернейм користувача
void getMyUserName();

//малювання полів
void PaintArena(HDC);
//малювання поля для розставляння кораблів
void PaintSetPlayerArena(HDC);
//генерація оптимальної кількості кораблів
void GenerateNumOfShips();
//генерація прямокутників арени
void GenerateArenaRects();
//генерація поля юзера
void SetMyArena(HWND, LPARAM);
//генерація поля бота
void SetBotArena();
//гра 
void Shooting(HWND , LPARAM );
//WIN
void Peremoga(HWND);
//обнулити данні гравців
void ResetPlayers();
//хід боту
void BotStep(HWND hWnd);
//визначення кількості інформаційних символів
int WchartSize(wchar_t*);
//отримати текст файлу історії
char* GetHistoryTextlpws();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 
        //обнулення notesi notepi
    ZeroMemory(&notesi, sizeof(notesi));
    notesi.cb = sizeof(notesi);
    ZeroMemory(&notepi, sizeof(notepi));


    // Ініціалізація глобальних рядків
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WARSHIP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // виконати ініціалізацію додатку:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WARSHIP));

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



//  ФУНКЦіЯ: MyRegisterClass()
//  ЦІЛЬ: Регіструє класс вікна.
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WARSHIP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WARSHIP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    

    return RegisterClassExW(&wcex);
}

//   ФУНКЦІЯ: InitInstance(HINSTANCE, int)
//   ЦІЛЬ: зберігає маркер екземпляру й створює головне вікно
//        В цій функції маркер екземпляру зберігається у глобальній змінній, а також
//        створюється і виводится головне вікно програми.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // маркер екземпляру зберігається у глобальній змінній
   //вирахувати розмір вікна програми залежно від розміра екрану
   //width = GetSystemMetrics(SM_CXSCREEN) / 4 * 3;
   //height = GetSystemMetrics(SM_CYSCREEN) / 4 * 3;
   width = GetSystemMetrics(SM_CXSCREEN);
   height = GetSystemMetrics(SM_CYSCREEN);

   HWND hWnd = CreateWindowW(szWindowClass, 
       szTitle, 
       WS_MINIMIZEBOX | WS_SYSMENU ,
       GetSystemMetrics(SM_CXSCREEN)/2 - (width/2), 
       GetSystemMetrics(SM_CYSCREEN)/2 - (height/2),
       width, height,
       nullptr, nullptr, 
       hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


//  ФУНКЦіЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//  ЦІЛЬ: опрацьовує повідомлення в головному вікні.
//  WM_COMMAND  - опрацьовує меню застосунку
//  WM_PAINT    - Промальовка головного вікна
//  WM_DESTROY  -  сообщение вихід і повернутись
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:

        //перевірка встановлення прапора початку гри
        if (((flags & (1 << CHECK_GAME_IS_STARTED)) != 0)) {
            Shooting( hWnd, lParam);
        }
        //перевірка встановлення прапора початку гри
        else if (((flags & (1 << CHECK_SET_ARENA)) != 0))
        {
            SetMyArena(hWnd, lParam);
        }
        
        break;
    case WM_MOUSEWHEEL:
    {
        //маус скрол
        if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
            VScrolInf.nPos -= (VScrolInf.nPos - 10 < VScrolInf.nMin) ? (0) : (20);
        }
        else {
            VScrolInf.nPos += (VScrolInf.nPos + 20 < VScrolInf.nMax) ? (20) : (0);
        }
        SetScrollInfo(hWnd, SB_VERT, &VScrolInf, TRUE);
        InvalidateRect(hWnd, NULL, TRUE);
    }
        break;
    case WM_HSCROLL:
    {
        switch (LOWORD(wParam)) {
        case SB_LINELEFT:
            HScrolInf.nPos -= 5;
            break;
        case SB_LINERIGHT:
            HScrolInf.nPos += 5;
            break;
        case SB_THUMBTRACK:
            HScrolInf.nPos = HIWORD(wParam);
            break;
        };
        SetScrollInfo(hWnd, SB_HORZ, &HScrolInf, TRUE);
        InvalidateRect(hWnd, NULL, TRUE);
}
        break;
    case WM_VSCROLL:
    {
        switch (LOWORD(wParam)) {
        case SB_LINEUP:
            VScrolInf.nPos -= 5;
            break;
        case SB_LINEDOWN:
            VScrolInf.nPos += 5;
            break;
        case SB_THUMBTRACK:
            VScrolInf.nPos = HIWORD(wParam);
            break;
        };
        SetScrollInfo(hWnd, SB_VERT, &VScrolInf, TRUE);
        InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    case WM_CREATE:
    {
        GetFileSettings();
        //робимо пункт вийти з гри недоступним
        EnableMenuItem(GetMenu(hWnd), ID_EXIT_GAME, MF_BYCOMMAND | MF_DISABLED);

        //скрол бар
        HScrolInf.cbSize = sizeof(HScrolInf);
        HScrolInf.nPos = 0;
        HScrolInf.nMax = width;
        HScrolInf.nPage = width;
        HScrolInf.fMask = SIF_ALL;
        HScrolInf.nMin = 0;

        SetScrollInfo(hWnd, SB_HORZ, &HScrolInf, TRUE);

        VScrolInf.cbSize = sizeof(VScrolInf);
        VScrolInf.nPos = 0;
        VScrolInf.nMax = height;
        VScrolInf.nPage = height;
        VScrolInf.fMask = SIF_ALL;
        VScrolInf.nMin = 0;

        SetScrollInfo(hWnd, SB_VERT, &VScrolInf, TRUE);

        //присвоєння унікальних номерів кнопкам
        BStart.id = ButtonStart;
        Bhistory.id = ButtonHistory;
        BnextSet.id = ButtonNextSet;
        BChangeOrient.id = ButtonChangeOrient;
        BShowMyArena.id = ButtonShowMyArena;
        BSettings.id = ButtonSettings;
        BClear.id = ButtonClear;
        EHistory.id = EditHistory;

        //перевірка директорії з логами
        checkLogDirectory();

        //командний рядок для відкриття файлу з історією
        wcscat_s(ComandLine, wcslen(ComandLine) + wcslen(LogfileName) + 1, LogfileName);
        //отримання характеристик шрифту
        hdc = BeginPaint(hWnd,&ps);
        GetTextMetrics(hdc, &tm);
        EndPaint(hWnd, &ps);

        // завантаження зображення з ресурсу
        hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_SHIP2VERT));

        //створення кнопок
        BStart.hControl = CreateWindowW(L"button", L"Розпочати гру", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            width / 2 - 75, height / 3, 150, 80, hWnd, (HMENU)BStart.id, hInst, NULL);

        BSettings.hControl = CreateWindowW(L"button", L"Налаштування", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            width / 2 - 75, height / 3 + 80, 150, 80, hWnd, (HMENU)BSettings.id, hInst, NULL);

        Bhistory.hControl = CreateWindowW(L"button", L"Історія", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            width / 2 - 75, height / 3 + 160, 150, 80, hWnd, (HMENU)Bhistory.id, hInst, NULL);

        BnextSet.hControl = CreateWindowW(L"button", L"Завершити розставляння кораблів", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            3, 3, 260, 40, hWnd, (HMENU)BnextSet.id, hInst, NULL);
        ShowWindow(BnextSet.hControl, SW_HIDE);//сховати кнопку

        BChangeOrient.hControl = CreateWindowW(L"button", L"Поставити корабель: Горизонтально", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            3, 45, 260, 30, hWnd, (HMENU)BChangeOrient.id, hInst, NULL);
        ShowWindow(BChangeOrient.hControl, SW_HIDE);//сховати кнопку  

        BShowMyArena.hControl = CreateWindowW(L"button", L"Показати власну арену", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            width - 255, 5, 250, 40, hWnd, (HMENU)BShowMyArena.id, hInst, NULL);
        ShowWindow(BShowMyArena.hControl, SW_HIDE);//сховати кнопку 

        EHistory.hControl = CreateWindowW(L"edit", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            width / 4, height / 4, width / 2, height / 2, hWnd, (HMENU)EHistory.id, hInst, NULL);
        ShowWindow(EHistory.hControl, SW_HIDE);//сховати поле 

        BClear.hControl = CreateWindowW(L"button", L"Зтерти історію", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            width / 2, 10, 250, 40, hWnd, (HMENU)BClear.id, hInst, NULL);
        ShowWindow(BClear.hControl, SW_HIDE);//сховати поле 


        //смітник 280 , 3, 90, 70
        TrashBox.left = 280;
        TrashBox.top = 3;
        TrashBox.right = 370;
        TrashBox.bottom = 73;
    }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать вибір в меню:
            switch (wmId)
            {
            //BStart - початок гри
            case ButtonStart:
            {

                //якщо гра вже починалась - видалити старі данні
                if (rPole1 != nullptr) {
                    for (int i = 0; i < prevnumKol; i++) {
                        delete[] rPole1[i];
                        delete[] rPole2[i];

                        delete[] Player1.ShipMap.myarena[i];
                        delete[] Player1.ShipMap.oponentarena[i];
                        delete[] Player1.ShipMap.ShipNum[i];
                        delete[] Player1.ShipMap.ShipSize[i];
                        delete[] Player2.ShipMap.myarena[i];
                        delete[] Player2.ShipMap.oponentarena[i];
                        delete[] Player2.ShipMap.ShipNum[i];
                        delete[] Player2.ShipMap.ShipSize[i];
                    }
                    delete[] rPole1;
                    delete[] rPole2;
                    rPole1 = nullptr;

                    delete[] Player1.ShipMap.myarena;
                    delete[] Player1.ShipMap.oponentarena;
                    delete[] Player1.ShipMap.ShipNum;
                    delete[] Player1.ShipMap.ShipSize;
                    delete[] Player2.ShipMap.myarena;
                    delete[] Player2.ShipMap.oponentarena;
                    delete[] Player2.ShipMap.ShipNum;
                    delete[] Player2.ShipMap.ShipSize;

                for (int i = 0; i < Player1.BiggestShip; i++) {

                    delete[] Player1.Ships[i].posKols;
                    delete[] Player1.Ships[i].posRows;
                    delete[] Player1.Ships[i].orientation;
                    delete[] Player1.Ships[i].dead;
                    delete[] Player1.Ships[i].damage;

                    delete[] Player2.Ships[i].posKols;
                    delete[] Player2.Ships[i].posRows;
                    delete[] Player2.Ships[i].orientation;
                    delete[] Player2.Ships[i].dead;
                    delete[] Player2.Ships[i].damage;
                }

                delete[] Player1.Ships;
                delete[] Player2.Ships;
                delete[] SetShipPosMenu.focus;
                delete[] SetShipPosMenu.SetMenuRect;

                
                }
                PlayerSetNow = &Player1;
                PlayerOponent = &Player2;
            wcscpy_s(Player2.name, L"Bot");//далі - якщо гра не з ботом - ім'я 2го гравця зміниться

            //перевірка встановлення прапора гри з ботом
            if ( ((flags & (1 << CHECK_PLAY_WITH_BOT)) == 0) )
                //прийняти імена гравців
                if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_GETNAMES), hWnd, GetNameProc) == 2)
                    break;//якщо натиснуто Скасувати - нічого не робити
            
            //пункт меню налаштування недоступний
            EnableMenuItem(GetMenu(hWnd), ID_SETTINGS, MF_BYCOMMAND | MF_DISABLED);
            //пункт меню вийти з гри доступний
            EnableMenuItem(GetMenu(hWnd), ID_EXIT_GAME, MF_BYCOMMAND | MF_ENABLED);

            HideMenu();//сховати кнопки головного екрану

                ShowWindow(BnextSet.hControl, SW_SHOW);//Показати кнопку завершення етапу розставляння кораблів
                ShowWindow(BChangeOrient.hControl, SW_SHOW);//Показати кнопку зміни орієнтаціі

                GenerateArenaRects();//генерація полів арени

                GenerateNumOfShips();//генерація кораблів
                prevnumKol = numKol;
                prevnumRow = numRow;
                //створення полів для виведення меню вибору корабля
                SetShipPosMenu.SetMenuRect = new RECT[Player1.BiggestShip];
                SetShipPosMenu.focus = new bool[Player1.BiggestShip];
                SetShipPosMenu.numSection = Player1.BiggestShip;
                for (int i = 0; i < Player1.BiggestShip;i++) {
                    SetShipPosMenu.SetMenuRect[i].left = (i*5)+380 + (i * (10 + (20 * tm.tmAveCharWidth)));
                    SetShipPosMenu.SetMenuRect[i].top = 10;
                    SetShipPosMenu.SetMenuRect[i].right = (i*5)+380 + ((i + 1) * (10 + (20 * tm.tmAveCharWidth)));
                    SetShipPosMenu.SetMenuRect[i].bottom = 60;
                    SetShipPosMenu.focus[i] = false;
                }
                //ширина хедера - межа останньої секціі меню
                widthHeader = SetShipPosMenu.SetMenuRect[SetShipPosMenu.numSection - 1].right;
                //прокрутка скролбоксів до меж арени
                //HScrolInf.nMax = (SetShipPosMenu.SetMenuRect[Player1.BiggestShip-1].right > width) ? (SetShipPosMenu.SetMenuRect[Player1.BiggestShip-1].right + 50) : (width + 50);
                
                if (SetShipPosMenu.SetMenuRect[Player1.BiggestShip - 1].right > width) {
                    if (SetShipPosMenu.SetMenuRect[Player1.BiggestShip - 1].right > MainRect1.right) {
                        HScrolInf.nMax = SetShipPosMenu.SetMenuRect[Player1.BiggestShip - 1].right + 50;
                        
                    }
                    else{
                        HScrolInf.nMax = MainRect1.right + 50;
                    }
                }
                else if(MainRect1.right>width)
                {
                    HScrolInf.nMax = MainRect1.right + 50;
                }
                else {
                    HScrolInf.nMax = width + 50;
                }
                SetScrollInfo(hWnd, SB_HORZ, &HScrolInf, TRUE);

                //максимальна позиція скролбару
                VScrolInf.nMax = (MainRect1.bottom > height) ? (MainRect1.bottom + 50 + heightHeader) : (height + 50 + heightHeader);
                SetScrollInfo(hWnd, SB_VERT, &VScrolInf, TRUE);
                //гравці готові розставляти кораблі
                Player1.Ready = false;
                Player2.Ready = false;

                // зміна прапора налаштування арени
                flags ^= (1 << CHECK_SET_ARENA);

                //формування рядка для MessageBoxA
                std::wstring message = PlayerSetNow->name;
                message += L" розставляє кораблі"; // додавання тексту
                //довжина рядка
                int messageLen = WideCharToMultiByte(CP_UTF8, 0, message.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string messageA(messageLen, 0);
                //перетворення тексту в код ansi
                WideCharToMultiByte(CP_ACP, 0, message.c_str(), -1, &messageA[0], messageLen, nullptr, nullptr);
                LPCSTR messageA_lpcstr = messageA.c_str(); // конвертація в LPCSTR
                //
                MessageBoxA(hWnd, messageA_lpcstr, (LPCSTR)"кораблик", 0);
                InvalidateRect(hWnd, NULL, TRUE);
                }
                break;
            case ButtonNextSet:
            {
                //якщо кнопку натиснув перший грок
                if (!Player1.Ready){
                    Player1.Ready = true;
                    for (int i = 0; i < Player1.BiggestShip; i++) 
                        if (Player1.Ships[i].zalishoknums > 0) {
                            Player1.Ready = false;//якщо є залишок кораблів
                            MessageBoxA(hWnd, (LPCSTR)"Розставлені не всі кораблі!", (LPCSTR)"Помилка", 0);
                            break;
                        }
                    if (!Player1.Ready) {
                        //якщо Розставлені не всі кораблі то не іти далі
                        break;
                    }
                    else {
                        PlayerSetNow = &Player2;//передати чергу налаштування до Player2
                         //перевірка встановлення прапора гри з ботом
                         if ( ((flags & (1 << CHECK_PLAY_WITH_BOT)) != 0) ){
                             SetBotArena();//бот розставляє свої кораблики
                             Player2.Ready = true;//бот готовий
                             PlayerSetNow = &Player1;//першим стріляє 1й гравець
                             PlayerOponent = &Player2;
                             //Scrol
                             HScrolInf.nMax = (MainRect2.right > width) ? (MainRect2.right + 50) : (width + 50);
                             SetScrollInfo(hWnd, SB_HORZ, &HScrolInf, TRUE);

                             ShowWindow(BnextSet.hControl, SW_HIDE);//сховати кнопку завершення етапу розставляння кораблів
                             ShowWindow(BChangeOrient.hControl, SW_HIDE);// сховати кнопку зміни орієнтаціі
                             InvalidateRect(hWnd, NULL, TRUE);
                             //game started
                             
                             //час початку гри
                             TimeStart = time(0);
                         }
                         else {
                             InvalidateRect(hWnd, NULL, TRUE);//очистити поле перед тим як 2й гравець почне розставляти кораблі

                             //формування рядка для MessageBoxA
                             std::wstring message = PlayerSetNow->name;
                             message += L" розставляє кораблі\nПерший не може підглядати"; // додавання тексту
                             //довжина рядка
                             int messageLen = WideCharToMultiByte(CP_UTF8, 0, message.c_str(), -1, nullptr, 0, nullptr, nullptr);
                             std::string messageA(messageLen, 0);
                             //перетворення тексту в код anci
                             WideCharToMultiByte(CP_ACP, 0, message.c_str(), -1, &messageA[0], messageLen, nullptr, nullptr);
                             LPCSTR messageA_lpcstr = messageA.c_str(); // конвертація в LPCSTR
                             //
                             MessageBoxA(hWnd, messageA_lpcstr, (LPCSTR)"кораблик", 0);
                             //MessageBoxA(hWnd, (LPCSTR)"Другий гравець розставляє кораблі\nПерший не може підглядати", (LPCSTR)"кораблик", 0);
                             //
                             break;
                         }
                    }
                }

            
            //якщо гра не з ботом - перемикання на налаштування 2го гравця
            //якщо кнопку натиснув другий гравець
            if (!Player2.Ready) {
                Player2.Ready = true;
                for (int i = 0; i < Player2.BiggestShip; i++)
                    if (Player2.Ships[i].zalishoknums > 0) {
                        Player2.Ready = false;
                        MessageBoxA(hWnd, (LPCSTR)"Розставлені не всі кораблі!", (LPCSTR)"Помилка", 0);
                        break;
                    }
                if (!Player2.Ready) {
                    break;
                }
            }
            // зміна прапора початку гри
            flags ^= (1 << CHECK_GAME_IS_STARTED);
            // зміна прапора set arena
            flags ^= (1 << CHECK_SET_ARENA);
            Player1.Ready = false;
            Player2.Ready = false;
            ShowWindow(BShowMyArena.hControl, SW_SHOW);//Показати кнопку ShowMyArena
            PlayerSetNow = &Player1;//першим стріляє 1й гравець
            PlayerOponent = &Player2;
            ShowWindow(BnextSet.hControl, SW_HIDE);//yt Показати кнопку завершення етапу розставляння кораблів
            ShowWindow(BChangeOrient.hControl, SW_HIDE);//ne Показати кнопку зміни орієнтаціі
            InvalidateRect(hWnd, NULL, TRUE);
            //Scrol
            HScrolInf.nMax = (MainRect2.right > width) ? (MainRect2.right + 50) : (width + 50);
            SetScrollInfo(hWnd, SB_HORZ, &HScrolInf, TRUE);
            //час початку гри
            TimeStart = time(0);
            //формування рядка для MessageBoxA
            std::wstring message = PlayerSetNow->name;
            message += L" робить постріл!"; // додавання тексту
            //довжина рядка
            int messageLen = WideCharToMultiByte(CP_UTF8, 0, message.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string messageA(messageLen, 0);
            //перетворення тексту в код anci
            WideCharToMultiByte(CP_ACP, 0, message.c_str(), -1, &messageA[0], messageLen, nullptr, nullptr);
            LPCSTR messageA_lpcstr = messageA.c_str(); // конвертація в LPCSTR
            //
            MessageBoxA(hWnd, messageA_lpcstr, (LPCSTR)"кораблик", 0);
            }
                break;
            case ButtonChangeOrient:
            {
                //перевірка встановлення прапора орієнтації
                if (((flags & (1 << CHECK_ORIENT)) != 0)) {
                    // зміна прапора орієнтації
                    flags ^= (1 << CHECK_ORIENT);
                    SendMessageA(BChangeOrient.hControl, WM_SETTEXT, 0, (LPARAM)"Поставити корабель: Горизонтально");
                }
                else {
                    // зміна прапора орієнтації
                    flags ^= (1 << CHECK_ORIENT);
                    SendMessageA(BChangeOrient.hControl, WM_SETTEXT, 0, (LPARAM)"Поставити корабель: Вертикально");
                }
                InvalidateRect(hWnd, NULL, TRUE);
            }
                break;
            case ButtonShowMyArena:
                // зміна прапора SHOW_ARENA
                flags ^= (1 << CHECK_SHOW_ARENA);
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            case ButtonClear:
            {
                //видалити записи з файлу історії ігор
                CreateFileW((LPCWSTR)LogfileName, 0, 0, 0, CREATE_ALWAYS, 0, NULL);
                CharTextHistory = 0;
                //вивід статік з текстом загадок
                CharTextHistory = GetHistoryTextlpws();
                SetWindowTextA(EHistory.hControl, (LPSTR)CharTextHistory); // присвоїти текст текстовому полю
                
            }
                break;
            //ButtonHistory - відкрити історію боїв
            case ButtonHistory:
            {
                checkLogDirectory();//перевірка наявності файлу 

                //перевірка встановлення прапора показу історії
                if (((flags & (1 << CHECK_SHOW_HISTORY)) != 0)) {
                    // зміна прапора CHECK_SHOW_HISTORY
                    flags ^= (1 << CHECK_SHOW_HISTORY);
                    ShowWindow(EHistory.hControl, SW_HIDE);
                    ShowWindow(BClear.hControl, SW_HIDE);


                    MoveWindow(Bhistory.hControl, width / 2 - 75, height / 3 + 160, 150, 80, false);
                    SetWindowTextA(Bhistory.hControl, (LPSTR)"Історія"); // присвоїти текст кнопці
                    ShowWindow(BStart.hControl, SW_SHOW);
                    ShowWindow(BSettings.hControl, SW_SHOW);
                    InvalidateRect(hWnd, NULL, TRUE);
                }
                else {
                    // зміна прапора CHECK_SHOW_HISTORY
                    flags ^= (1 << CHECK_SHOW_HISTORY);

                    MoveWindow(Bhistory.hControl, 10, 10, 150, 80, false);
                    SetWindowTextA(Bhistory.hControl, (LPSTR)"Назад"); // присвоїти текст кнопці
                    CharTextHistory = 0;
                    //вивід статік з текстом загадок
                    CharTextHistory = GetHistoryTextlpws();
                    SetWindowTextA(EHistory.hControl, (LPSTR)CharTextHistory); // присвоїти текст текстовому полю
                    ShowWindow(EHistory.hControl, SW_SHOW);
                    ShowWindow(BClear.hControl, SW_SHOW);

                    ShowWindow(BStart.hControl, SW_HIDE);
                    ShowWindow(BSettings.hControl, SW_HIDE);
                    InvalidateRect(hWnd, NULL, TRUE);
                }

            }
                break;
            case ButtonSettings:
            case ID_SETTINGS:
            {
                if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETTINGS), hWnd, ChangeSettingsProc) == 2)
                    break;//якщо натиснуто Скасувати - нічого не робити
                //зберегти налаштування
                SetFileSettingsText();
            }
                break;
            case IDM_MYABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case ID_EXIT_GAME:
            {
                 PlayerSetNow = &Player1;
                 PlayerOponent = &Player2;

                //перевірка встановлення прапора початку гри
                if (((flags & (1 << CHECK_GAME_IS_STARTED)) != 0)) {
                    // зміна прапора налаштування поля
                    flags ^= (1 << CHECK_GAME_IS_STARTED);
                }
                //перевірка встановлення прапора налаштування поля
                if (((flags & (1 << CHECK_SET_ARENA)) != 0)) {
                    // зміна прапора налаштування поля
                    flags ^= (1 << CHECK_SET_ARENA);
                }

                //пункт меню налаштування доступний
                EnableMenuItem(GetMenu(hWnd), ID_SETTINGS, MF_BYCOMMAND | MF_ENABLED);
                //пункт меню вийти з гри недоступний
                EnableMenuItem(GetMenu(hWnd), ID_EXIT_GAME, MF_BYCOMMAND |MF_DISABLED);
                //відобразити кнопку завершення розташування кораблів
                ShowWindow(BnextSet.hControl, SW_HIDE);
                ShowWindow(BChangeOrient.hControl, SW_HIDE);
                ShowWindow(BShowMyArena.hControl, SW_HIDE);
                ResetPlayers();
                ShowMenu();
                InvalidateRect(hWnd, NULL, TRUE);
            }
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
            hdc = BeginPaint(hWnd, &ps);
            //перевірка встановлення прапора початку гри
            if ( ((flags & (1 << CHECK_GAME_IS_STARTED)) != 0) ) { 
                PaintArena(hdc);
            }
            //перевірка встановлення прапора налаштування ігрового поля
            else if(((flags & (1 << CHECK_SET_ARENA)) != 0) && Player2.Ready== false) {
                PaintSetPlayerArena(hdc);
            }
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        //зберегти налаштування
        SetFileSettingsText();
        
        //вивільнити пам'ять
        //якщо гра вже починалась - видалити старі данні
        if (rPole1 != nullptr) {
            for (int i = 0; i < prevnumKol; i++) {
                delete[] rPole1[i];
                delete[] rPole2[i];

                delete[] Player1.ShipMap.myarena[i];
                delete[] Player1.ShipMap.oponentarena[i];
                delete[] Player1.ShipMap.ShipNum[i];
                delete[] Player1.ShipMap.ShipSize[i];
                delete[] Player2.ShipMap.myarena[i];
                delete[] Player2.ShipMap.oponentarena[i];
                delete[] Player2.ShipMap.ShipNum[i];
                delete[] Player2.ShipMap.ShipSize[i];
            }
            delete[] rPole1;
            delete[] rPole2;
            rPole1 == nullptr;

            delete[] Player1.ShipMap.myarena;
            delete[] Player1.ShipMap.oponentarena;
            delete[] Player1.ShipMap.ShipNum;
            delete[] Player1.ShipMap.ShipSize;
            delete[] Player2.ShipMap.myarena;
            delete[] Player2.ShipMap.oponentarena;
            delete[] Player2.ShipMap.ShipNum;
            delete[] Player2.ShipMap.ShipSize;

            for (int i = 0; i < Player1.BiggestShip; i++) {

                delete[] Player1.Ships[i].posKols;
                delete[] Player1.Ships[i].posRows;
                delete[] Player1.Ships[i].orientation;
                delete[] Player1.Ships[i].dead;
                delete[] Player1.Ships[i].damage;

                delete[] Player2.Ships[i].posKols;
                delete[] Player2.Ships[i].posRows;
                delete[] Player2.Ships[i].orientation;
                delete[] Player2.Ships[i].dead;
                delete[] Player2.Ships[i].damage;
            }

            delete[] Player1.Ships;
            delete[] Player2.Ships;
            delete[] SetShipPosMenu.focus;
            delete[] SetShipPosMenu.SetMenuRect;
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// обробник повідомлень для вікна "про програму".
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

//обробник подій для діалогу отримання імен гравців
INT_PTR CALLBACK GetNameProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
    //UNREFERENCED_PARAMETER(lParam);
                //рядки для отримання чисел з текстових полів
            wchar_t StrnumKol[255];
            wchar_t StrnumRow[255];
            //дескриптори для роботти з полями вводу
            HWND hEditName1 = 0;
            HWND hEditName2 = 0;
            HWND hEdit3NumKols = 0;
            HWND hEdit4NumRows = 0;
    switch (message)
    {
    case WM_INITDIALOG:
        //Запишему у текстові змінні збережену інформацію
        swprintf_s(StrnumKol, L"%d", numKol);
        swprintf_s(StrnumRow, L"%d", numRow);

        //отримаємо хендли полів вводу
        hEditName1 = GetDlgItem(hDlg, IDC_EDITname1);
        hEditName2 = GetDlgItem(hDlg, IDC_EDITname2);
        hEdit3NumKols = GetDlgItem(hDlg, IDC_EDITkol3);
        hEdit4NumRows = GetDlgItem(hDlg, IDC_EDITrow4);

        //Запишему у текстові поля збережену інформацію
        SendMessageW(hEdit3NumKols, WM_SETTEXT, 0, (LPARAM)StrnumKol);
        SendMessageW(hEdit4NumRows, WM_SETTEXT, 0, (LPARAM)StrnumRow);
        SendMessageW(hEditName1, WM_SETTEXT, 0, (LPARAM)Player1.name);
        SendMessageW(hEditName2, WM_SETTEXT, 0, (LPARAM)Player2.name);
        
        return (INT_PTR)TRUE;
        //break;?
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:

            //обнулення
            wcscpy_s(Player1.name, L"");
            wcscpy_s(Player2.name, L"");
            wcscpy_s(StrnumKol, L"");
            wcscpy_s(StrnumRow, L"");
            
            //отримаємо хендли полів вводу
            hEditName1 = GetDlgItem(hDlg, IDC_EDITname1);
            hEditName2 = GetDlgItem(hDlg, IDC_EDITname2);
            hEdit3NumKols = GetDlgItem(hDlg, IDC_EDITkol3);
            hEdit4NumRows = GetDlgItem(hDlg, IDC_EDITrow4);

            //записуємо текст з поля у глобальну змінну
            SendMessageW(hEditName1, WM_GETTEXT, (WPARAM)255, (LPARAM)Player1.name);
            SendMessageW(hEditName2, WM_GETTEXT, (WPARAM)255, (LPARAM)Player2.name);
            SendMessageW(hEdit3NumKols, WM_GETTEXT, (WPARAM)255, (LPARAM)StrnumKol);
            SendMessageW(hEdit4NumRows, WM_GETTEXT, (WPARAM)255, (LPARAM)StrnumRow);

            //перевірка на заповнення всіх полів
            if (wcscmp(Player1.name, L"") == 0 || wcscmp(Player2.name, L"") == 0
                || wcscmp(StrnumKol, L"") == 0 || wcscmp(StrnumRow, L"") == 0) {
                MessageBoxA(hDlg, (LPCSTR)"Уведено не всі данні", (LPCSTR)"Помилка", 0);
                break;
            }

            numKol = _wtoi(StrnumKol);//wchar_t to int
            numRow = _wtoi(StrnumRow);

            EndDialog(hDlg, LOWORD(wParam));//повертаємо код завершення(код натиснутої кнопки)
            break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));//повертаємо код завершення(код натиснутої кнопки)
            return (INT_PTR)TRUE;
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

//обробник подій для діалогу зміни налаштувань
INT_PTR CALLBACK ChangeSettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);

    //дескриптори для роботти з полями вводу
    HWND hEditKol;
    HWND hEditRow;
    HWND hEditCellSide;
    HWND hEditPercentShip;
    HWND hCheckboxBot;

    //отримаємо хендли полів вводу
    hEditKol = GetDlgItem(hDlg, IDC_EDITkols1);
    hEditRow = GetDlgItem(hDlg, IDC_EDITrows2);
    hEditCellSide = GetDlgItem(hDlg, IDC_EDITcellside3);
    hEditPercentShip = GetDlgItem(hDlg, IDC_EDITpercente);
    hCheckboxBot = GetDlgItem(hDlg, IDC_CHECKbot1);
    
    //рядки для отримання чисел з текстових полів
    wchar_t StrnumKol[255];
    wchar_t StrnumRow[255];
    wchar_t StrCellSide[255];
    wchar_t StrPercentShip[255];
    bool CheckBox = false;

    switch (message)
    {
    case WM_INITDIALOG:
        //Запишему у текстові змінні збережену інформацію
        swprintf_s(StrnumKol, L"%d", numKol);
        swprintf_s(StrnumRow, L"%d", numRow);
        swprintf_s(StrCellSide, L"%d", CellSide);
        swprintf_s(StrPercentShip, L"%d", percentAllShips);

        //Запишему у текстові поля збережену інформацію
        SendMessageW(hEditKol, WM_SETTEXT, 0, (LPARAM)StrnumKol);
        SendMessageW(hEditRow, WM_SETTEXT, 0, (LPARAM)StrnumRow);
        SendMessageW(hEditCellSide, WM_SETTEXT, 0, (LPARAM)StrCellSide);
        SendMessageW(hEditPercentShip, WM_SETTEXT, 0, (LPARAM)StrPercentShip);

        //встановимо збережений стан прапорця
        SendMessageW(hCheckboxBot, BM_SETCHECK, 
            (((flags & (1 << CHECK_PLAY_WITH_BOT)) == 1)) ? (BST_CHECKED) : (BST_UNCHECKED), 0);

        return (INT_PTR)TRUE;

        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            //обнулення
            wcscpy_s(StrnumKol, L"");
            wcscpy_s(StrnumRow, L"");
            wcscpy_s(StrCellSide, L"");
            wcscpy_s(StrPercentShip, L"");

            //записуємо текст з поля у посередні змінні
            SendMessageW(hEditKol, WM_GETTEXT, (WPARAM)255, (LPARAM)StrnumKol);
            SendMessageW(hEditRow, WM_GETTEXT, (WPARAM)255, (LPARAM)StrnumRow);
            SendMessageW(hEditCellSide, WM_GETTEXT, (WPARAM)255, (LPARAM)StrCellSide);
            SendMessageW(hEditPercentShip, WM_GETTEXT, (WPARAM)255, (LPARAM)StrPercentShip);
            CheckBox = (SendMessageW(hCheckboxBot, BM_GETCHECK, (WPARAM)255, 0) == BST_CHECKED);
            
            //перевірка на заповнення всіх полів
            if (wcscmp(StrnumKol,L"") == 0 || wcscmp(StrnumRow, L"") == 0
                || wcscmp(StrCellSide, L"") == 0
                || wcscmp(StrPercentShip, L"") == 0) {

                MessageBoxA(hDlg, (LPCSTR)"Уведено не всі данні",(LPCSTR)"Помилка", 0);
                break;
            }
           
             //перевірка на коректність
            if (_wtoi(StrPercentShip) > 100 || _wtoi(StrPercentShip) < 10) {

                MessageBoxA(hDlg, (LPCSTR)"відсоток кораблів має бути в межах 10-100", (LPCSTR)"Помилка", 0);
                break;
            }
            //перевірка на великий відсоток кораблів(при грі з ботом)
            if (_wtoi(StrPercentShip) > 60) {

                MessageBoxA(hDlg, (LPCSTR)"Обережно, при великому відсотку кораблів\nбот можливо не зможе створити свою арену", (LPCSTR)"Увага", 0);

            }
            //перевірка на розмір клітинок
            if (_wtoi(StrCellSide) < 2) {

                MessageBoxA(hDlg, (LPCSTR)"Замалий розмір клітинок", (LPCSTR)"Увага", 0);
                break;

            }

            //перевірка на необхідність зміни прапора
            if ( (flags & (1 << CHECK_PLAY_WITH_BOT)) == CheckBox) {
            }
            else
            {
                    flags ^= (1 << CHECK_PLAY_WITH_BOT);
            }
            
            //wchar_t to int
            numKol = _wtoi(StrnumKol);
            numRow = _wtoi(StrnumRow);
            CellSide = _wtoi(StrCellSide);
            percentAllShips = _wtoi(StrPercentShip);

            SetFileSettingsText();
            EndDialog(hDlg, LOWORD(wParam));//повертаємо код завершення(код натиснутої кнопки)
            break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));//повертаємо код завершення(код натиснутої кнопки)
            return (INT_PTR)TRUE;
            break;
        }
    }
    return (INT_PTR)FALSE;
}

//перевірка наявності директоріі з логами
void checkLogDirectory() {
    //пошук каталогу
    hFoundeFile = FindFirstFileW((LPCWSTR)LogcatalogName, &Wfd);
    if (hFoundeFile == INVALID_HANDLE_VALUE) { // перевіряємо наявність каталогу
        CreateDirectory((LPCWSTR)LogcatalogName, NULL);//створюємо каталог для файлів
    }

    hFoundeFile = FindFirstFileW(SettingsName, &Wfd);//шукаємо файл з налаштуваннями
    if (hFoundeFile == INVALID_HANDLE_VALUE) {
        CreateFileW((LPCWSTR)SettingsName, 0, 0, 0, CREATE_ALWAYS, 0, NULL);
        SetFileSettingsText();//заповнюємо файл налаштувань за замовчуванням
    }

    hFoundeFile = FindFirstFileW(LogfileName, &Wfd);//шукаємо файл з історією ігор
    if (hFoundeFile == INVALID_HANDLE_VALUE) {
        CreateFileW((LPCWSTR)LogfileName, 0, 0, 0, CREATE_ALWAYS, 0, NULL);
    }
}

//записати інформацію про налаштування за замовчуванням
void SetFileSettingsText() {
    in.open(SettingsName);
    if (in.is_open())
    {
        //запис у файл конфігурації
        in << numKol << "\n";
        in << numRow << "\n";
        in << CellSide << "\n";
        in << percentAllShips << "\n";
        in << ( (flags & (1 << (CHECK_PLAY_WITH_BOT)))?(1):(0) ) << "\n";
        //текстове пояснення відповідно до порядку розташування змінних
        in << "#Кількість стовпців\n\
#Кількість рядків\n\
#Розмір клітинки поля\n\
#Відсоток кораблів від розміру поля\n\
#Грати з комп'ютером(так/ні)" << "\0";
    }
    in.close();
}

//взяти інформацію про налаштування з файлу
void GetFileSettings() {
    bool check = 0;
    out.open(SettingsName);
    if (out.is_open()) {
        //запис данних у змінні
        out >> numKol ;
        out >> numRow ;
        out >> CellSide ;
        out >> percentAllShips ;
        out >> check;
        //перевірка на необхідність зміни прапора гри з ботом
        if ((flags & (1 << CHECK_PLAY_WITH_BOT)) == check) {
        }
        else
        {
            flags ^= (1 << CHECK_PLAY_WITH_BOT);
        }
    }
}

//сховати меню з кнопок
void HideMenu() {
    ShowWindow(Bhistory.hControl, SW_HIDE);
    ShowWindow(BStart.hControl, SW_HIDE);
    ShowWindow(BSettings.hControl, SW_HIDE);
}

//показати кнопки меню
void ShowMenu() {
    ShowWindow(Bhistory.hControl, SW_SHOW);
    ShowWindow(BStart.hControl, SW_SHOW);
    ShowWindow(BSettings.hControl, SW_SHOW);
}

//генерація прямокутників арени
void GenerateArenaRects() {
    /*ІГРОВЕ ПОЛЕ*/
        //визначення положення першого ігрового поля
    MainRect1.left = 10;//x
    MainRect1.top = 10 + heightHeader;//y

    MainRect1.right = 10 + (CellSide + 1) * numKol;//x
    MainRect1.bottom = MainRect1.top + (CellSide + 1) * numRow;//y

    //визначення положення другого ігрового поля
    MainRect2.left = MainRect1.right + (CellSide * 2);//x
    MainRect2.top = 10 + heightHeader;//y

    MainRect2.right = MainRect2.left + (CellSide + 1) * numKol;//x
    MainRect2.bottom = MainRect2.top + (CellSide + 1) * numRow;//y

    //визначення положень клітинок у полях
    rPole1 = new RECT * [numKol];
    rPole2 = new RECT * [numKol];
    for (int i = 0; i < numKol; i++) rPole1[i] = new RECT[numRow];
    for (int i = 0; i < numKol; i++) rPole2[i] = new RECT[numRow];

    for (int i = 0; i < numKol; i++) {
        for (int j = 0; j < numRow; j++) {
            rPole1[i][j].left = i + MainRect1.left + (CellSide * i);//x(+ i для зазору між клітинками)
            rPole1[i][j].top = j + MainRect1.top + (CellSide * j);//y

            rPole1[i][j].right = i + MainRect1.left + (CellSide * (i + 1));//x
            rPole1[i][j].bottom = j + MainRect1.top + (CellSide * (j + 1));//y
        }
    }

    for (int i = 0; i < numKol; i++) {
        for (int j = 0; j < numRow; j++) {
            rPole2[i][j].left = i + MainRect2.left + (CellSide * i);//x(+ i для зазору між клітинками)
            rPole2[i][j].top = j + MainRect2.top + (CellSide * j);//y

            rPole2[i][j].right = i + MainRect2.left + (CellSide * (i + 1));//x
            rPole2[i][j].bottom = j + MainRect2.top + (CellSide * (j + 1));//y
        }
    }
}

//налаштування арени(розставити корблі)
void PaintSetPlayerArena(HDC hdc) {
    
    wchar_t StrInt[255];//рядок для виводу чисел на екран

    HBRUSH hB;//пензлик для малювання прямокутника
    hB = CreateSolidBrush(RGB(0, 0, 0)); //створення чорного пензля  
    
    //відображення чорного фону арени
    Rectangle(hdc, MainRect1.left - HScrolInf.nPos,
        MainRect1.top - VScrolInf.nPos,
        MainRect1.right - HScrolInf.nPos,
        MainRect1.bottom - VScrolInf.nPos);

    hB = CreateSolidBrush(RGB(127, 255, 217));//змінити колір пензля
    SelectObject(hdc, hB);//обрати певний Brush

    //відображення клітинок першого поля
    for (int i = 0; i < numKol; i++) {
        for (int j = 0; j < numRow; j++)
            if (rPole1[i][j].right - HScrolInf.nPos >= 0
                && rPole1[i][j].left - HScrolInf.nPos <= width
                && rPole1[i][j].bottom - VScrolInf.nPos >= 0
                && rPole1[i][j].top - VScrolInf.nPos <= height)
            { 
                Rectangle(hdc, rPole1[i][j].left - HScrolInf.nPos,
                    rPole1[i][j].top - VScrolInf.nPos,
                    rPole1[i][j].right - HScrolInf.nPos,
                    rPole1[i][j].bottom - VScrolInf.nPos);
            }
    }
    // малювати кораблики
    for (int i = 0; i < numKol; i++) {
        for (int j = 0; j < numRow; j++) {
            //якщо "голова" корабля-малювати його
            if (PlayerSetNow->ShipMap.myarena[i][j] == 2)
            {
                if (PlayerSetNow->Ships[PlayerSetNow->ShipMap.ShipSize[i][j] - 1].orientation[PlayerSetNow->ShipMap.ShipNum[i][j]] == false) {
                    // Завантаження зображення з ресурса(три види кораблів для різних розмірів)
                    // hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_SHIP2HORIZ));
                    hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW((PlayerSetNow->ShipMap.ShipSize[i][j] < 3) ? (IDB_SHIP2HORIZ) : ((PlayerSetNow->ShipMap.ShipSize[i][j] < 8) ? (IDB_SHIP4HORIZ) : ( IDB_SHIP6HORIZ))));
                    // створення контексту пам'яті для бітового блока зображення
                    hdcMem = CreateCompatibleDC(hdc);
                    SelectObject(hdcMem, hBitmap);

                    GetObject(hBitmap, sizeof(bmp), &bmp);//визначаємо розміри бітмапу
                    // малюємо розтянуте зображення
                        StretchBlt(hdc,
                            rPole1[i][j].left - HScrolInf.nPos,
                            rPole1[i][j].top - VScrolInf.nPos,
                        rPole1[i+ PlayerSetNow->ShipMap.ShipSize[i][j]-1][j].right - rPole1[i][j].left,
                        CellSide,
                        hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
                    //StretchBlt(hdc,
                        //PlayerSetNow->Ships[PlayerSetNow->ShipMap.ShipSize[i][j] - 1].posX[PlayerSetNow->ShipMap.ShipNum[i][j]] - HScrolInf.nPos,
                        //PlayerSetNow->Ships[PlayerSetNow->ShipMap.ShipSize[i][j] - 1].posY[PlayerSetNow->ShipMap.ShipNum[i][j]] - VScrolInf.nPos,
                        //CellSide * PlayerSetNow->ShipMap.ShipSize[i][j],
                        //CellSide,
                        //hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);інший варіант
                    
                    // Визволення ресурсів
                    DeleteDC(hdcMem);
                    //SetWorldTransform(hdc, NULL);
                    DeleteObject(hBitmap);
                }
                else {
                    // Завантаження зображення з ресурса
                    hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(   (PlayerSetNow->ShipMap.ShipSize[i][j]<3)?(IDB_SHIP2VERT):(  (PlayerSetNow->ShipMap.ShipSize[i][j] < 8)?(IDB_SHIP4VERT):(IDB_SHIP6VERT)  )  ));
                    
                    // створення контексту пам'яті для бітового блока зображення
                    hdcMem = CreateCompatibleDC(hdc);
                    SelectObject(hdcMem, hBitmap);

                    GetObject(hBitmap, sizeof(bmp), &bmp);//визначаємо розміри бітмапу
                    // малюємо розтянуте зображення
                    StretchBlt(hdc,
                        rPole1[i][j].left - HScrolInf.nPos,
                        rPole1[i][j].top - VScrolInf.nPos,
                        CellSide,
                        rPole1[i][j + PlayerSetNow->ShipMap.ShipSize[i][j]-1].bottom - rPole1[i][j].top,
                        hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
                    // Визволення ресурсів
                    DeleteDC(hdcMem);
                    DeleteObject(hBitmap);
                }
            }
        }
    }
    hB = CreateSolidBrush(RGB(240, 102, 65));//змінити колір пензля
    SelectObject(hdc, hB);//обрати певний Brush
    //переміщення кнопок відповідно до положення скролбару
    MoveWindow(BnextSet.hControl, 3 - HScrolInf.nPos, 3, 260, 40, false);
    MoveWindow(BChangeOrient.hControl, 3 - HScrolInf.nPos, 45, 260, 30, false);

    SelectObject(hdc, hB);//обрати певний Brush
    Rectangle(hdc, 0, 0, width, heightHeader);

    //відображення хедеру (кількість розставлених кораблів)
    for (int i = 0; i < Player1.BiggestShip; i++) {
        Rectangle(hdc, SetShipPosMenu.SetMenuRect[i].left - HScrolInf.nPos,
            SetShipPosMenu.SetMenuRect[i].top ,
            SetShipPosMenu.SetMenuRect[i].right - HScrolInf.nPos,
            SetShipPosMenu.SetMenuRect[i].bottom );
        if (SetShipPosMenu.focus[i]) {
            hB = CreateHatchBrush(1, RGB(0, 0, 0));
            RECT focusedrect;
            focusedrect.left = SetShipPosMenu.SetMenuRect[i].left - 1 - HScrolInf.nPos;
            focusedrect.top = SetShipPosMenu.SetMenuRect[i].top - 1;
            focusedrect.right = SetShipPosMenu.SetMenuRect[i].right  + 1 - HScrolInf.nPos;
            focusedrect.bottom = SetShipPosMenu.SetMenuRect[i].bottom  + 1;
            DrawFocusRect(hdc, &focusedrect);//малювання контуру навколо прямокутника(як фокусування на ньому)
        }

        TextOutW(hdc, SetShipPosMenu.SetMenuRect[i].left - HScrolInf.nPos, SetShipPosMenu.SetMenuRect[i].top , L"розмір корабля", 14);
        TextOutW(hdc, SetShipPosMenu.SetMenuRect[i].left - HScrolInf.nPos, SetShipPosMenu.SetMenuRect[i].top  + tm.tmHeight, L"залишилоссь: ", 14);

        swprintf_s(StrInt, L"%d", PlayerSetNow->Ships[i].size);
        TextOutW(hdc, SetShipPosMenu.SetMenuRect[i].left  + 16 * tm.tmAveCharWidth - HScrolInf.nPos, SetShipPosMenu.SetMenuRect[i].top , StrInt, WchartSize(StrInt));
        swprintf_s(StrInt, L"%d", PlayerSetNow->Ships[i].zalishoknums);
        TextOutW(hdc, SetShipPosMenu.SetMenuRect[i].left  + 16 * tm.tmAveCharWidth - HScrolInf.nPos, SetShipPosMenu.SetMenuRect[i].top  + tm.tmHeight, StrInt, WchartSize(StrInt));
    }

    //move DeleteShip Image
    //перевірка встановлення прапора DELETE_SHIP
    if (((flags & (1 << CHECK_DELETE_SHIP)) != 0)) {
        //зображення активного смітника(вибраного)
        hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_TRASHBOXRED));
    }
    else {
        hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_TRASHBOXGRAY));
    }

    // створення контексту пам'яті для бітового блока зображення
    hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, hBitmap);

    GetObject(hBitmap, sizeof(bmp), &bmp);//визначаємо розміри бітмапу
    // малюємо розтянуте зображення
    StretchBlt(hdc,
        TrashBox.left - HScrolInf.nPos,
        TrashBox.top ,
        80,
        70,
        hdcMem,  0,  0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
    // Визволення ресурсів
    DeleteDC(hdcMem);
    DeleteObject(hBitmap);

}

//малювання ігрових полів
void PaintArena(/*HWND hWnd*/HDC hdc) {

    wchar_t StrInt[255];//рядок для виводу чисел на екран

    //info player1
    TextOutW(hdc, 5 - HScrolInf.nPos, 5 - VScrolInf.nPos, L"Гравець1: ", 10); /**/
    TextOutW(hdc, 5 - HScrolInf.nPos, 5 + (tm.tmHeight) + (2) - VScrolInf.nPos, L"Кількість влучань: ", 19);
    TextOutW(hdc, 5 - HScrolInf.nPos, 5 + (tm.tmHeight * 2) + (2 * 2) - VScrolInf.nPos, L"Кількість вбивств: ", 19);

    TextOutW(hdc, 11 * tm.tmAveCharWidth - HScrolInf.nPos, 5 - VScrolInf.nPos, Player1.name, WchartSize(Player1.name));
    swprintf_s(StrInt, L"%d", Player1.hits);
    TextOutW(hdc, 20 * tm.tmAveCharWidth - HScrolInf.nPos, 5 + (tm.tmHeight) + (2) - VScrolInf.nPos, StrInt, WchartSize(StrInt));
    swprintf_s(StrInt, L"%d", Player1.Kills);
    TextOutW(hdc, 20 * tm.tmAveCharWidth - HScrolInf.nPos, 5 + (tm.tmHeight * 2) + (2 * 2) - VScrolInf.nPos, StrInt, WchartSize(StrInt));
    
    //info player2
    TextOutW(hdc, 80 * tm.tmAveCharWidth - HScrolInf.nPos, 5 - VScrolInf.nPos, L"Гравець2: ", 10);
    TextOutW(hdc, 80 * tm.tmAveCharWidth - HScrolInf.nPos, 5 + (tm.tmHeight) + (2) - VScrolInf.nPos, L"Кількість влучань: ", 19);
    TextOutW(hdc, 80 * tm.tmAveCharWidth - HScrolInf.nPos, 5 + (tm.tmHeight * 2) + (2 * 2) - VScrolInf.nPos, L"Кількість вбивств: ", 19);

    TextOutW(hdc, 91 * tm.tmAveCharWidth - HScrolInf.nPos, 5 - VScrolInf.nPos, Player2.name, WchartSize(Player2.name));
    swprintf_s(StrInt, L"%d", Player2.hits);
    TextOutW(hdc, 100 * tm.tmAveCharWidth - HScrolInf.nPos, 5 + (tm.tmHeight) + (2) - VScrolInf.nPos, StrInt, WchartSize(StrInt));
    swprintf_s(StrInt, L"%d", Player2.Kills);
    TextOutW(hdc, 100 * tm.tmAveCharWidth - HScrolInf.nPos, 5 + (tm.tmHeight * 2) + (2 * 2) - VScrolInf.nPos, StrInt, WchartSize(StrInt));

    
    HBRUSH hB;//пензлик для малювання прямокутника

    hB = CreateSolidBrush(RGB(0, 0, 0)); //створення чорного пензля 
    SelectObject(hdc, hB);//обрати певний Brush

    //FillRect(hdc, &MainRect1, hB);//намалювати прямокутник
    Rectangle(hdc, MainRect1.left - HScrolInf.nPos,
        MainRect1.top - VScrolInf.nPos,
        MainRect1.right - HScrolInf.nPos,
        MainRect1.bottom - VScrolInf.nPos);

    Rectangle(hdc, MainRect2.left - HScrolInf.nPos,
        MainRect2.top - VScrolInf.nPos,
        MainRect2.right - HScrolInf.nPos,
        MainRect2.bottom - VScrolInf.nPos);

    hB = CreateSolidBrush(RGB(127, 255, 217));//змінити колір пензля
    SelectObject(hdc, hB);//обрати певний Brush

    //відображення клітинок першого поля (tam постріли)
    for (int i = 0; i < numKol; i++) {
        for (int j = 0; j < numRow; j++)
            if (rPole1[i][j].right - HScrolInf.nPos >= 0 
                && rPole1[i][j].left - HScrolInf.nPos <= width 
                && rPole1[i][j].bottom - VScrolInf.nPos >= 0
                && rPole1[i][j].top - VScrolInf.nPos <= height)
            Rectangle(hdc,  rPole1[i][j].left - HScrolInf.nPos,
                            rPole1[i][j].top - VScrolInf.nPos,
                            rPole1[i][j].right - HScrolInf.nPos,
                            rPole1[i][j].bottom - VScrolInf.nPos);
    }
    //малювати потоплені кораблі опонента
    //малювати власні постріли/влучання (-1 пусто 1 промах(водяні кола) 2 влучив в корабель(вогонь))
    for (int i = 0; i < numKol; i++) {
        for (int j = 0; j < numRow; j++) {
            //якщо "голова" корабля-малювати його
            if (PlayerOponent->ShipMap.myarena[i][j] == 2) {
                if (PlayerOponent->Ships[PlayerOponent->ShipMap.ShipSize[i][j]-1].dead[PlayerOponent->ShipMap.ShipNum[i][j]] == true) {
                    //orient
                    if (PlayerOponent->Ships[PlayerOponent->ShipMap.ShipSize[i][j] - 1].orientation[PlayerOponent->ShipMap.ShipNum[i][j]] == false) {
                        // Завантаження зображення з ресурса
                        // hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_SHIP2HORIZ));
                        hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW((PlayerOponent->ShipMap.ShipSize[i][j] < 3) ? (IDB_SHIP2HORIZ) : ((PlayerOponent->ShipMap.ShipSize[i][j] < 8) ? (IDB_SHIP4HORIZ) : (IDB_SHIP6HORIZ))));
                        // створення контексту пам'яті для бітового блока зображення
                        hdcMem = CreateCompatibleDC(hdc);
                        SelectObject(hdcMem, hBitmap);

                        GetObject(hBitmap, sizeof(bmp), &bmp);//визначаємо розміри бітмапу
                        // малюємо розтянуте зображення
                        StretchBlt(hdc,
                            rPole1[i][j].left - HScrolInf.nPos,
                            rPole1[i][j].top - VScrolInf.nPos,
                            rPole1[i + PlayerOponent->ShipMap.ShipSize[i][j] - 1][j].right - rPole1[i][j].left,
                            CellSide,
                            hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
                        // Визволення ресурсів
                        DeleteDC(hdcMem);
                        //SetWorldTransform(hdc, NULL);
                        DeleteObject(hBitmap);
                    }
                    else {
                        // Завантаження зображення з ресурса
                        hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW((PlayerOponent->ShipMap.ShipSize[i][j] < 3) ? (IDB_SHIP2VERT) : ((PlayerOponent->ShipMap.ShipSize[i][j] < 8) ? (IDB_SHIP4VERT) : (IDB_SHIP6VERT))));

                        // створення контексту пам'яті для бітового блока зображення
                        hdcMem = CreateCompatibleDC(hdc);
                        SelectObject(hdcMem, hBitmap);

                        GetObject(hBitmap, sizeof(bmp), &bmp);//визначаємо розміри бітмапу
                        // малюємо розтянуте зображення
                        StretchBlt(hdc,
                            rPole1[i][j].left - HScrolInf.nPos,
                            rPole1[i][j].top - VScrolInf.nPos,
                            CellSide,
                            rPole1[i][j + PlayerOponent->ShipMap.ShipSize[i][j] - 1].bottom - rPole1[i][j].top,
                            hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

                        DeleteDC(hdcMem);
                        //SetWorldTransform(hdc, NULL);
                        DeleteObject(hBitmap);
                    }
                }
            }
            if (PlayerSetNow->ShipMap.oponentarena[i][j] == 1) {
                //малюємо хвилі
                 // Завантаження зображення з ресурса
                hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_VOLNA));

                // створення контексту пам'яті для бітового блока зображення
                hdcMem = CreateCompatibleDC(hdc);
                SelectObject(hdcMem, hBitmap);

                GetObject(hBitmap, sizeof(bmp), &bmp);//визначаємо розміри бітмапу
                // малюємо розтянуте зображення
                StretchBlt(hdc,
                    rPole1[i][j].left - HScrolInf.nPos,
                    rPole1[i][j].top - VScrolInf.nPos,
                    CellSide,
                    CellSide,
                    hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

                DeleteDC(hdcMem);
                //SetWorldTransform(hdc, NULL);
                DeleteObject(hBitmap);
            }
            else if (PlayerSetNow->ShipMap.oponentarena[i][j] == 2) {
                //малюємо полум'я
                // Завантаження зображення з ресурса
                hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_FIRE));

                // створення контексту пам'яті для бітового блока зображення
                hdcMem = CreateCompatibleDC(hdc);
                SelectObject(hdcMem, hBitmap);

                GetObject(hBitmap, sizeof(bmp), &bmp);//визначаємо розміри бітмапу
                // малюємо розтянуте зображення
                StretchBlt(hdc,
                    rPole1[i][j].left + CellSide/2 - HScrolInf.nPos,
                    rPole1[i][j].top + CellSide /2 - VScrolInf.nPos,
                    CellSide/2,
                    CellSide/2,
                    hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

                DeleteDC(hdcMem);
                //SetWorldTransform(hdc, NULL);
                DeleteObject(hBitmap);
            }

        }
    }

    //відображення клітинок другого поля(власне)
    if (((flags & (1 << CHECK_SHOW_ARENA)) != 0)) {//якщо натиснута кнопка(показати власну арену)
        for (int i = 0; i < numKol; i++) {
            for (int j = 0; j < numRow; j++)
                if (rPole2[i][j].right - HScrolInf.nPos >= 0
                    && rPole2[i][j].left - HScrolInf.nPos <= width
                    && rPole2[i][j].bottom - VScrolInf.nPos >= 0
                    && rPole2[i][j].top - VScrolInf.nPos <= height)
                    Rectangle(hdc, rPole2[i][j].left - HScrolInf.nPos,
                        rPole2[i][j].top - VScrolInf.nPos,
                        rPole2[i][j].right - HScrolInf.nPos,
                        rPole2[i][j].bottom - VScrolInf.nPos);
        }
        // малювати кораблики
        for (int i = 0; i < numKol; i++) {
            for (int j = 0; j < numRow; j++) {
                //якщо "голова" корабля-малювати його
                if (PlayerSetNow->ShipMap.myarena[i][j] == 2)
                {
                    if (PlayerSetNow->Ships[PlayerSetNow->ShipMap.ShipSize[i][j] - 1].orientation[PlayerSetNow->ShipMap.ShipNum[i][j]] == false) {
                        // Завантаження зображення з ресурса
                        // hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_SHIP2HORIZ));
                        hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW((PlayerSetNow->ShipMap.ShipSize[i][j] < 3) ? (IDB_SHIP2HORIZ) : ((PlayerSetNow->ShipMap.ShipSize[i][j] < 8) ? (IDB_SHIP4HORIZ) : (IDB_SHIP6HORIZ))));
                        // створення контексту пам'яті для бітового блока зображення
                        hdcMem = CreateCompatibleDC(hdc);
                        SelectObject(hdcMem, hBitmap);

                        GetObject(hBitmap, sizeof(bmp), &bmp);//визначаємо розміри бітмапу
                        // малюємо розтянуте зображення
                        StretchBlt(hdc,
                            rPole2[i][j].left - HScrolInf.nPos,
                            rPole2[i][j].top - VScrolInf.nPos,
                            rPole2[i + PlayerSetNow->ShipMap.ShipSize[i][j] - 1][j].right - rPole2[i][j].left,
                            CellSide,
                            hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
                        // Визволення ресурсів
                        DeleteDC(hdcMem);
                        //SetWorldTransform(hdc, NULL);
                        DeleteObject(hBitmap);
                    }
                    else {
                        // Завантаження зображення з ресурса
                        hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW((PlayerSetNow->ShipMap.ShipSize[i][j] < 3) ? (IDB_SHIP2VERT) : ((PlayerSetNow->ShipMap.ShipSize[i][j] < 8) ? (IDB_SHIP4VERT) : (IDB_SHIP6VERT))));

                        // створення контексту пам'яті для бітового блока зображення
                        hdcMem = CreateCompatibleDC(hdc);
                        SelectObject(hdcMem, hBitmap);

                        GetObject(hBitmap, sizeof(bmp), &bmp);//визначаємо розміри бітмапу
                        // малюємо розтянуте зображення
                        StretchBlt(hdc,
                            rPole2[i][j].left - HScrolInf.nPos,
                            rPole2[i][j].top - VScrolInf.nPos,
                            CellSide,
                            rPole2[i][j + PlayerSetNow->ShipMap.ShipSize[i][j] - 1].bottom - rPole2[i][j].top,
                            hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

                        DeleteDC(hdcMem);
                        //SetWorldTransform(hdc, NULL);
                        DeleteObject(hBitmap);
                    }

                }

            }
        }
        for (int i = 0; i < numKol; i++) {
            for (int j = 0; j < numRow; j++) {
                //малювати постріли опонента
                if (PlayerOponent->ShipMap.oponentarena[i][j] == 1) {
                    //малюємо хвилі
                     // Завантаження зображення з ресурса
                    hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_VOLNA));

                    // створення контексту пам'яті для бітового блока зображення
                    hdcMem = CreateCompatibleDC(hdc);
                    SelectObject(hdcMem, hBitmap);

                    GetObject(hBitmap, sizeof(bmp), &bmp);//визначаємо розміри бітмапу
                    // малюємо розтянуте зображення
                    StretchBlt(hdc,
                        rPole2[i][j].left - HScrolInf.nPos,
                        rPole2[i][j].top - VScrolInf.nPos,
                        CellSide,
                        CellSide,
                        hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

                    DeleteDC(hdcMem);
                    //SetWorldTransform(hdc, NULL);
                    DeleteObject(hBitmap);
                }
                else if (PlayerOponent->ShipMap.oponentarena[i][j] == 2) {
                    //малюємо полум'я
                    // Завантаження зображення з ресурса
                    hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_FIRE));

                    // створення контексту пам'яті для бітового блока зображення
                    hdcMem = CreateCompatibleDC(hdc);
                    SelectObject(hdcMem, hBitmap);

                    GetObject(hBitmap, sizeof(bmp), &bmp);//визначаємо розміри бітмапу
                    // малюємо розтянуте зображення
                    StretchBlt(hdc,
                        rPole2[i][j].left + CellSide / 2 - HScrolInf.nPos,
                        rPole2[i][j].top + CellSide / 2 - VScrolInf.nPos,
                        CellSide / 2,
                        CellSide / 2,
                        hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

                    DeleteDC(hdcMem);
                    //SetWorldTransform(hdc, NULL);
                    DeleteObject(hBitmap);
                }
            }
        }
    }

        
    //переміщення кнопки відображення власного поля
    //MoveWindow(BShowMyArena.hControl, width - 255 - HScrolInf.nPos, 5 - VScrolInf.nPos, 250, 40, false);
    //MoveWindow(BShowMyArena.hControl, MainRect2.left - HScrolInf.nPos, MainRect2.top - 40 - VScrolInf.nPos, 250, 40, false);
    MoveWindow(BShowMyArena.hControl, MainRect2.left - HScrolInf.nPos, SetShipPosMenu.SetMenuRect[0].bottom - VScrolInf.nPos, 250, MainRect2.top - SetShipPosMenu.SetMenuRect[0].bottom, false);
}

//генерація оптимальної кількості кораблів
void GenerateNumOfShips() {

    int PoleSize = numKol * numRow;//всього клітинок на полі

    //int PoleSize = numKol * numRow;//всього клітинок на полі
    int CellsForShips = floor(PoleSize * (percentAllShips / 100.0));//кількість клітинок для кораблів
    int N = 1;//найбільший тип корабля
    for (; 2 * N < CellsForShips * 0.2 && N < numKol/2 && N < numRow/2; N++) {}

    //генерація мап пострілів
    Player1.ShipMap.myarena = new int* [numKol];
    Player1.ShipMap.oponentarena = new int* [numKol];
    Player1.ShipMap.ShipNum = new int* [numKol];
    Player1.ShipMap.ShipSize = new int* [numKol];

    Player2.ShipMap.myarena = new int* [numKol];
    Player2.ShipMap.oponentarena = new int* [numKol];
    Player2.ShipMap.ShipNum = new int* [numKol];
    Player2.ShipMap.ShipSize = new int* [numKol];

    //виділення пам'яті для мап
    for (int i = 0; i < numKol; i++) {
        Player1.ShipMap.myarena[i] = new int[numRow];
        Player1.ShipMap.oponentarena[i] = new int[numRow];
        Player1.ShipMap.ShipNum[i] = new int[numRow];
        Player1.ShipMap.ShipSize[i] = new int[numRow];
        
        Player2.ShipMap.myarena[i] = new int[numRow];
        Player2.ShipMap.oponentarena[i] = new int[numRow];
        Player2.ShipMap.ShipNum[i] = new int[numRow];
        Player2.ShipMap.ShipSize[i] = new int[numRow];
    }
    //обнулення мап
    for (int i = 0; i < numKol; i++) {
        for (int j = 0; j < numRow; j++) {
            Player1.ShipMap.myarena[i][j] = 0;
            Player1.ShipMap.oponentarena[i][j] = 0;
            Player1.ShipMap.ShipNum[i][j] = 0;
            Player1.ShipMap.ShipSize[i][j] = 0;

            Player2.ShipMap.myarena[i][j] = 0;
            Player2.ShipMap.oponentarena[i][j] = 0;
            Player2.ShipMap.ShipNum[i][j] = 0;
            Player2.ShipMap.ShipSize[i][j] = 0;
        }
    }

    //запис найбільшого рангу корабля
    Player1.BiggestShip = N;
    Player2.BiggestShip = N;

    Player1.Ships = new MyShips[N];
    Player2.Ships = new MyShips[N];
    //ініціалізація розмірів кораблів
    for (int i = 0; i < N; i++) {
        Player1.Ships[i].size = i + 1;
        Player2.Ships[i].size = i + 1;
    }

    //delete у кінці функції
    int* CellsshipN = new int[N];//кількість клітинок для кожного типу корабля
    int* CellToshipN = new int[N];//перенесення остачі клітинок від кораблів вищого рангу до кораблів нижчого
    double* percentshipN = new double[N];//відсотки кораблів від дозволеної кількості клітинок

    CellsForShips = floor(PoleSize * (percentAllShips / 100.0));//кількість клітинок для кораблів

    //генерація процентажу для кожного типу корабля
    if (N == 1) {
        //якщо однопалубний корабель найбільший - йому всі 100%
        percentshipN[0] = 100;
    }
    else
    {
        double sum = 0;//скільки використали від ста відсотків
        //percentshipN[N - 1] = 40;//найбільший корабель займає відсотків
        percentshipN[N - 1] = 2 * 100/N;//найбільший корабель займає відсотків
        sum += percentshipN[N - 1];//сумма використаних відсотків клітин()
        for (int i = N - 2; i >= 0; i--) {
            if (i == 0) {
                //якщо розрахунок для останнього(однопалубного) корабля-віддаємо весь залишок відсотків
                percentshipN[i] = 100 - sum;
                //if (percentshipN[i] == 0)percentshipN[i] = 1;//якщо буде 0 - зробити хочаб 1--
            }
            else {
                percentshipN[i] = rand() % (int)(100 - sum - i);//
                //якщо відсоток кораблів менший за необхідний для одного корабля
                //виділяємо стільки, аби вистачило на 1 корабель
                if(percentshipN[i] < double((double)(Player1.Ships[i].size)/CellsForShips))
                    percentshipN[i] = double((double)(Player1.Ships[i].size) / CellsForShips);

                sum += percentshipN[i];//перерахунок використаних відсотків
            }
        }
    }

    //кількість кожного типу корабля
    CellToshipN[N - 1] = 0;//найвищий ранг рахуємо першим, йому остачі ніхто не лишав
    for (int i = N - 1; i >= 0; i--) {
        CellsshipN[i] = CellToshipN[i] + floor(CellsForShips * (double)(percentshipN[i] / 100.0));//клеток для кор
        Player1.Ships[i].nums = floor((double)(CellsshipN[i] / Player1.Ships[i].size));//кількість кораблів
        if (Player1.Ships[i].nums <= 0)
            Player1.Ships[i].nums = 1;//якщо раптом залишиться 0 - зробити 1
        Player1.Ships[i].zalishoknums = Player1.Ships[i].nums;
        Player1.Ships[i].posKols = new int[Player1.Ships[i].nums];
        Player1.Ships[i].posRows = new int[Player1.Ships[i].nums];
        Player1.Ships[i].orientation = new bool[Player1.Ships[i].nums];
        Player1.Ships[i].dead = new bool[Player1.Ships[i].nums];
        Player1.Ships[i].damage = new int[Player1.Ships[i].nums];

        Player2.Ships[i].nums = Player1.Ships[i].nums;
        Player2.Ships[i].zalishoknums = Player2.Ships[i].nums;
        Player2.Ships[i].posKols = new int[Player2.Ships[i].nums];
        Player2.Ships[i].posRows = new int[Player2.Ships[i].nums];
        Player2.Ships[i].orientation = new bool[Player2.Ships[i].nums];
        Player2.Ships[i].dead = new bool[Player2.Ships[i].nums];
        Player2.Ships[i].damage = new int[Player2.Ships[i].nums];
        //якщо це не 1палубний корабель, залишок клітинок перераховуємо на ранг нижчому кораблю
        if (i != 0)CellToshipN[i - 1] = CellsshipN[i] - (Player1.Ships[i].nums * Player1.Ships[i].size);
    }
    //обнулення
    for (int i = 0; i < Player1.BiggestShip; i++) {
        for (int j = 0; j < Player1.Ships[i].nums; j++) {
            Player1.Ships[i].posKols[j] = -1;
            Player1.Ships[i].posRows[j] = -1;
            Player1.Ships[i].dead[j] = 0;
            Player1.Ships[i].damage[j] = 0;

            Player2.Ships[i].posKols[j] = -1;
            Player2.Ships[i].posRows[j] = -1;
            Player2.Ships[i].dead[j] = 0;
            Player2.Ships[i].damage[j] = 0;
        }
    }

    //запис загальної кількоті кораблів
    numShips = 0;
    for (int i = 0; i < N;i++) {
        numShips += Player1.Ships[i].nums;
    }

    delete[] CellsshipN;
    delete[] CellToshipN;
    delete[] percentshipN;
}

//отримати юзернейм користувача
void getMyUserName() {
    char username[256];
    DWORD len = 255;
    GetUserNameW((LPWSTR)username, &len);
}

//визначення кількості інформаційних символів
int WchartSize(wchar_t* mystr) {
    int i = 0;
    for (; mystr[i] != '\0'; i++) {}
    return i;
}

//генерація поля бота
void SetBotArena() {
    int check = 0;//перевірка правильного розташування
    int i = 0, j = 0;//координати клітинки
    int SelectMenu = 0;//SetShipPosMenu.nowFocused (розмір вибраного корабля)
    int Orient = 0;
    srand(time(0));//генерація не псевдовипадкова

    SelectMenu = PlayerSetNow->BiggestShip - 1;//спочатку розставимо найбільші кораблі
    //поки наявні однопалубні кораблі
    while (PlayerSetNow->Ships[0].zalishoknums > 0)//>= ?
    {
        while (PlayerSetNow->Ships[(SelectMenu>=0)?(SelectMenu):(0)].zalishoknums > 0)
        {
            check = 0;
            Orient = rand() % 2;//0..1

            i = rand() % numKol;//0..numKol-1
            j = rand() % numRow;
            
            //перевірка накладання корабля на інший
            //перевірка встановлення прапора ORIENT
            if (Orient==1) {
                //j     //vert
                for (int k = j;
                    (k - j) < PlayerSetNow->Ships[SelectMenu].size; k++)
                {
                    //перевірка накладання корабля на інший
                    if (k >= numRow || PlayerSetNow->ShipMap.myarena[i][k] != 0) {
                        check = 1;
                    }
                }
                if (check)
                    break;
            }
            else {
                for (int k = i;
                    (k - i) < PlayerSetNow->Ships[SelectMenu].size; k++)
                {
                    //перевірка накладання корабля на інший
                    if (k >= numKol || PlayerSetNow->ShipMap.myarena[k][j] != 0) {
                        //MessageBoxA(hWnd, (LPCSTR)"Невірне розташування корабля", (LPCSTR)"Помилка", 0);
                        check = 1;
                    }
                }
                if (check)
                    break;
            }

            PlayerSetNow->Ships[SelectMenu].zalishoknums--;

            //позиції корабля в клітинках posKols
            PlayerSetNow->Ships[SelectMenu].posKols[PlayerSetNow->Ships[SelectMenu].zalishoknums] = i;
            PlayerSetNow->Ships[SelectMenu].posRows[PlayerSetNow->Ships[SelectMenu].zalishoknums] = j;

            //перевірка встановлення прапора ORIENT
            if (Orient == 1) {
                //j     //vert
                for (int k = j;
                    (k - j) < PlayerSetNow->Ships[SelectMenu].size; k++)
                {
                    PlayerSetNow->ShipMap.myarena[i][k] = (k == j) ? (2) : (1);//"голова" корабля == 2
                    PlayerSetNow->ShipMap.ShipNum[i][k] = PlayerSetNow->Ships[SelectMenu].zalishoknums;
                    PlayerSetNow->ShipMap.ShipSize[i][k] = PlayerSetNow->Ships[SelectMenu].size;
                }
                PlayerSetNow->Ships[SelectMenu].orientation[PlayerSetNow->Ships[SelectMenu].zalishoknums] = true;
            }
            else {
                //i
                for (int k = i;
                    (k - i) < PlayerSetNow->Ships[SelectMenu].size; k++)
                {
                    PlayerSetNow->ShipMap.myarena[k][j] = (k == i) ? (2) : (1);//"голова" корабля == 2
                    PlayerSetNow->ShipMap.ShipNum[k][j] = PlayerSetNow->Ships[SelectMenu].zalishoknums;
                    PlayerSetNow->ShipMap.ShipSize[k][j] = PlayerSetNow->Ships[SelectMenu].size;
                }
                PlayerSetNow->Ships[SelectMenu].orientation[PlayerSetNow->Ships[SelectMenu].zalishoknums] = false;
            }

        //перевіряємо чи завершили розставляти кораблі вибраного типу (розміру)
        if (PlayerSetNow->Ships[SelectMenu].zalishoknums == 0 && SelectMenu>=0)
            SelectMenu--;

        }
    }
    //запис мап у файли
    /*
    in.open("botarena.txt");
    if (in.is_open())
    {
        //запис у файл конфігурації
        for (int j = 0; j < numRow; j++) {
            in << "\n";
            for (int i = 0; i < numKol; i++)
                in << PlayerSetNow->ShipMap.myarena[i][j] << " ";
        }
    }
    in.close();

    in.open("botShipSize.txt");
    if (in.is_open())
    {
        //запис у файл конфігурації
        for (int j = 0; j < numRow; j++) {
            in << "\n";
            for (int i = 0; i < numKol; i++)
                in << PlayerSetNow->ShipMap.ShipSize[i][j] << " ";
        }
    }
    in.close();

    in.open("botShipNum.txt");
    if (in.is_open())
    {
        //запис у файл конфігурації
        for (int j = 0; j < numRow; j++) {
            in << "\n";
            for (int i = 0; i < numKol; i++)
                in << PlayerSetNow->ShipMap.ShipNum[i][j] << " ";
        }
    }
    in.close();*/
}

//генерація поля юзера
void SetMyArena(HWND hWnd,LPARAM lParam) {
    //координати кліку
    int posX = LOWORD(lParam);
    int posY = HIWORD(lParam);
    
        //якщо клік був у межах "хедера" 
        if (posY < heightHeader && posX < widthHeader) {
            //якщо клікнули по смітнику
            if (posX < TrashBox.right - HScrolInf.nPos && posX > TrashBox.left - HScrolInf.nPos) {
                //змінюємо фокусування
                //SetShipPosMenu.focus[(SetShipPosMenu.nowFocused!=-1)?(SetShipPosMenu.nowFocused):(0)] = false;
                if (SetShipPosMenu.nowFocused != -1)
                    SetShipPosMenu.focus[SetShipPosMenu.nowFocused] = false;
                SetShipPosMenu.nowFocused = -1;

                // зміна прапора вибору смітника
                flags ^= (1 << CHECK_DELETE_SHIP);

                InvalidateRect(hWnd, NULL, TRUE);
                return;
            }
            for (int i = 0; i < SetShipPosMenu.numSection; i++) {
                //якщо клікнули саме по цій секції
                if (posX  > SetShipPosMenu.SetMenuRect[i].left - HScrolInf.nPos && posX  < SetShipPosMenu.SetMenuRect[i].right - HScrolInf.nPos) {
                    //змінюємо фокусування
                    //SetShipPosMenu.focus[(SetShipPosMenu.nowFocused != -1) ? (SetShipPosMenu.nowFocused) : (0)] = false;
                    if(SetShipPosMenu.nowFocused != -1)
                        SetShipPosMenu.focus[SetShipPosMenu.nowFocused] = false;
                    SetShipPosMenu.nowFocused = i;
                    SetShipPosMenu.focus[i] = true;
                    break;
                }

            }
        }
        //якщо клік був у межах клітин ігрового поля 
        else if (posY > MainRect1.top - VScrolInf.nPos && posX < MainRect1.right - HScrolInf.nPos
                && posY < MainRect1.bottom - VScrolInf.nPos && posX > MainRect1.left - HScrolInf.nPos)
        {
            int i = 0, j = 0;
            //якщо клікнули саме по цій клітинці
            for( j = 0; posY > rPole1[0][j].bottom - VScrolInf.nPos && j < numRow-1;j++){}
            for( i = 0; posX > rPole1[i][j].right - HScrolInf.nPos && i < numKol-1; i++){}
            /*перевірка кліку
            rPole1[i][j].left++;rPole1[i][j].top++;
            rPole1[i][j].right--;rPole1[i][j].bottom--;*/

            //перевірка встановлення прапора DELETE_SHIP
                if (((flags & (1 << CHECK_DELETE_SHIP)) != 0)) {
                    //hBitmap = LoadBitmapW(hInst, MAKEINTRESOURCEW(IDB_TRASHBOXRED));
                    if (PlayerSetNow->ShipMap.myarena[i][j] != 0) {
                        ////дізнатись розмір корабля/номер
                        //змістити данні інших кораблів цього типу(розміру) 
                        //видалити з трьох мап/ редагувати мапи
                        int sizebuf = PlayerSetNow->ShipMap.ShipSize[i][j]-1;
                        int numbuf = PlayerSetNow->ShipMap.ShipNum[i][j];
                        int posKolsbuf = PlayerSetNow->Ships[sizebuf].posKols[numbuf];
                        int posRowsbuf = PlayerSetNow->Ships[sizebuf].posRows[numbuf];

                        //додаємо видалений корабель до залишку кораблів такого розміру
                        PlayerSetNow->Ships[sizebuf].zalishoknums++;
                        //видаляємо інформацію про цей корабель з мап (мапа наявності/розміру/номеру)
                        //з структури
                        if (PlayerSetNow->Ships[sizebuf].orientation[numbuf]) {
                            //j     //vert
                            i = PlayerSetNow->Ships[sizebuf].posKols[numbuf];
                            j = PlayerSetNow->Ships[sizebuf].posRows[numbuf];
                            for (int k = j;
                                (k - j) <= sizebuf; k++)
                            {
                                PlayerSetNow->ShipMap.myarena[i][k] = 0;
                                PlayerSetNow->ShipMap.ShipNum[i][k] = 0;
                                PlayerSetNow->ShipMap.ShipSize[i][k] = 0;
                            }
                        }
                        else {
                            //i
                            i = PlayerSetNow->Ships[sizebuf].posKols[numbuf];
                            j = PlayerSetNow->Ships[sizebuf].posRows[numbuf];
                            for (int k = i;
                                (k - i) <= sizebuf; k++)
                            {
                                PlayerSetNow->ShipMap.myarena[k][j] = 0;
                                PlayerSetNow->ShipMap.ShipNum[k][j] = 0;
                                PlayerSetNow->ShipMap.ShipSize[k][j] = 0;
                            }
                        }

                        //пройтись по всім кораблям які менші за номером за видаляємий
                        for (int t = numbuf; t > 0; t--) {
                            //якщо наступний за номером корабель має позицію -1 - ще не встановлений
                            if (PlayerSetNow->Ships[sizebuf].posKols[t - 1] != -1) {

                                PlayerSetNow->Ships[sizebuf].orientation[t] = PlayerSetNow->Ships[sizebuf].orientation[t - 1];

                                PlayerSetNow->Ships[sizebuf].posKols[t] = PlayerSetNow->Ships[sizebuf].posKols[t - 1];
                                PlayerSetNow->Ships[sizebuf].posRows[t] = PlayerSetNow->Ships[sizebuf].posRows[t - 1];

                                PlayerSetNow->Ships[sizebuf].posKols[t - 1] = -1;

                                //редагування мапи номерів кораблів
                                if (PlayerSetNow->Ships[sizebuf].orientation[t]) {
                                    //j     //vert
                                    i = PlayerSetNow->Ships[sizebuf].posKols[t];
                                    j = PlayerSetNow->Ships[sizebuf].posRows[t];
                                    for (int k = j;
                                        (k - j) <= sizebuf; k++)
                                    {
                                        PlayerSetNow->ShipMap.ShipNum[i][k]++;
                                    }
                                    PlayerSetNow->Ships[sizebuf].orientation[t] = true;
                                }
                                else {
                                    //i
                                    i = PlayerSetNow->Ships[sizebuf].posKols[t];
                                    j = PlayerSetNow->Ships[sizebuf].posRows[t];
                                    for (int k = i;
                                        (k - i) <= sizebuf; k++)
                                    {
                                        PlayerSetNow->ShipMap.ShipNum[k][j]++;
                                    }
                                    PlayerSetNow->Ships[sizebuf].orientation[t] = false;
                                }
                            }
                        }
                    }

           }else //перевірка на вибраний пункт меню корабля
                if (SetShipPosMenu.nowFocused != -1) 
                    //перевірка наявності вибраного типу кораблів
                if(PlayerSetNow->Ships[SetShipPosMenu.nowFocused].zalishoknums > 0){
                    //перевірка накладання корабля на інший
                    //перевірка встановлення прапора ORIENT
                    if ((flags & (1 << CHECK_ORIENT)) != 0) {
                        //j     //vert
                        for (int k = j;
                            (k - j) < PlayerSetNow->Ships[SetShipPosMenu.nowFocused].size; k++) 
                        {
                            //перевірка накладання корабля на інший
                            if (k >= numRow || PlayerSetNow->ShipMap.myarena[i][k] != 0) {
                                MessageBoxA(hWnd, (LPCSTR)"Невірне розташування корабля", (LPCSTR)"Помилка", 0);
                                return;
                            }
                        }
                    }
                    else {
                        for (int k = i;
                            (k - i) < PlayerSetNow->Ships[SetShipPosMenu.nowFocused].size; k++)
                        {
                            //перевірка накладання корабля на інший
                            if (k >= numKol || PlayerSetNow->ShipMap.myarena[k][j] != 0) {
                                MessageBoxA(hWnd, (LPCSTR)"Невірне розташування корабля", (LPCSTR)"Помилка", 0);
                                return;
                            }
                        }
                    }

                    //позиціі корабля в пікселях
                    PlayerSetNow->Ships[SetShipPosMenu.nowFocused].zalishoknums--;

                    //позиції корабля в клітинках posKols
                    PlayerSetNow->Ships[SetShipPosMenu.nowFocused].posKols[PlayerSetNow->Ships[SetShipPosMenu.nowFocused].zalishoknums] = i;
                    PlayerSetNow->Ships[SetShipPosMenu.nowFocused].posRows[PlayerSetNow->Ships[SetShipPosMenu.nowFocused].zalishoknums] = j;

                    //перевірка встановлення прапора ORIENT
                    if((flags & (1 << CHECK_ORIENT)) != 0) {
                        //j     //vert
                        for (int k = j;
                            (k-j) < PlayerSetNow->Ships[SetShipPosMenu.nowFocused].size; k++) 
                        {
                            PlayerSetNow->ShipMap.myarena[i][k] = (k == j)?(2):(1);//"голова" корабля == 2
                            PlayerSetNow->ShipMap.ShipNum[i][k] = PlayerSetNow->Ships[SetShipPosMenu.nowFocused].zalishoknums;
                            PlayerSetNow->ShipMap.ShipSize[i][k] = PlayerSetNow->Ships[SetShipPosMenu.nowFocused].size;
                            }
                        PlayerSetNow->Ships[SetShipPosMenu.nowFocused].orientation[PlayerSetNow->Ships[SetShipPosMenu.nowFocused].zalishoknums] = true;
                    }
                    else {
                        //i
                        for (int k = i;
                            (k-i) < PlayerSetNow->Ships[SetShipPosMenu.nowFocused].size; k++) 
                        {
                            PlayerSetNow->ShipMap.myarena[k][j] = (k == i) ? (2) : (1);//"голова" корабля == 2
                            PlayerSetNow->ShipMap.ShipNum[k][j] = PlayerSetNow->Ships[SetShipPosMenu.nowFocused].zalishoknums;
                            PlayerSetNow->ShipMap.ShipSize[k][j] = PlayerSetNow->Ships[SetShipPosMenu.nowFocused].size;
                        }
                        PlayerSetNow->Ships[SetShipPosMenu.nowFocused].orientation[PlayerSetNow->Ships[SetShipPosMenu.nowFocused].zalishoknums] = false;
                    }
                }
             }
             //запис мап у файли
        
        /*
        if (!Player1.Ready) {
            in.open("myarena.txt");
        }
        else {
            in.open("botarena.txt"); 
        }
        if (in.is_open())
        {
            //запис у файл конфігурації
            for(int j = 0; j < numRow ; j++)  {
                in << "\n";
                for (int i = 0; i < numKol ; i++)
                    in << PlayerSetNow->ShipMap.myarena[i][j] << " ";
            }
        }
        in.close();

        if (!Player1.Ready) {
            in.open("ShipSize.txt");
        }
        else {
            in.open("botShipSize.txt");
        }
        if (in.is_open())
        {
            //запис у файл конфігурації
            for (int j = 0; j < numRow; j++) {
                in << "\n";
                for (int i = 0; i < numKol; i++)
                    in << PlayerSetNow->ShipMap.ShipSize[i][j] << " ";
            }
        }
        in.close();

        if (!Player1.Ready) {
            in.open("ShipNum.txt");
        }
        else {
            in.open("botShipNum.txt");
        }
        if (in.is_open())
        {
            //запис у файл конфігурації
            for (int j = 0; j < numRow; j++) {
                in << "\n";
                for (int i = 0; i < numKol; i++)
                    in << PlayerSetNow->ShipMap.ShipNum[i][j] << " ";
            }
        }
        in.close();
        */
    InvalidateRect(hWnd, NULL, TRUE);
}

//обробка кліків-пострілів
void Shooting(HWND hWnd, LPARAM lParam) {
    int posX = LOWORD(lParam);
    int posY = HIWORD(lParam);

    // перевірка встановлення прапора гри з ботом
    if (((flags & (1 << CHECK_PLAY_WITH_BOT)) != 0)) {}
    else
    {
        //встановлення SHOW_ARENA = 0
        if (((flags & (1 << CHECK_SHOW_ARENA)) != 0))
            flags ^= (1 << CHECK_SHOW_ARENA);
    }
    
    
    //якщо клік був у межах поля арени супротивника
    if (posY > MainRect1.top - VScrolInf.nPos && posX < MainRect1.right - HScrolInf.nPos
        && posY < MainRect1.bottom - VScrolInf.nPos && posX > MainRect1.left - HScrolInf.nPos)
    {
        int i = 0, j = 0;
        //якщо клікнули саме по цій клітинці
        for (j = 0; posY > rPole1[0][j].bottom - VScrolInf.nPos && j < numRow - 1; j++) {}
        for (i = 0; posX > rPole1[i][j].right - HScrolInf.nPos && i < numKol - 1; i++) {}


        //якщо сюди ще не стріляли
        if(PlayerSetNow->ShipMap.oponentarena[i][j] == 0){
        //лічильник пострілів
        PlayerSetNow->CountShot++;
        //якщо влучили
        if (PlayerOponent->ShipMap.myarena[i][j] != 0) {
            //позначити влучення на мапі
            PlayerSetNow->ShipMap.oponentarena[i][j] = 2;

            //лічильник влучань
            PlayerSetNow->hits++;
            PlayerOponent->Ships[PlayerOponent->ShipMap.ShipSize[i][j]-1].damage[PlayerOponent->ShipMap.ShipNum[i][j]]++;
            if (PlayerOponent->Ships[PlayerOponent->ShipMap.ShipSize[i][j]-1].damage[PlayerOponent->ShipMap.ShipNum[i][j]] == PlayerOponent->ShipMap.ShipSize[i][j]) {
                PlayerOponent->Ships[PlayerOponent->ShipMap.ShipSize[i][j]-1].dead[PlayerOponent->ShipMap.ShipNum[i][j]] = true;
                //лічильник потоплень
                PlayerSetNow->Kills++;
                //якщо перемога
                if (PlayerSetNow->Kills == numShips) {
                    PlayerSetNow->Ready = 1;//переміг
                    PlayerOponent->Ready = 0;
                    //запис результатів:
                    //\n\n 
                    //гра
                    // початок - час, кінець - час2
                    // гравець1: ім'я (переміг/програв)
                    // n пострілів,н влучань, н потоплень
                    // -//-
                    //
                    InvalidateRect(hWnd, NULL, TRUE);

                    //формування рядка для MessageBoxA
                    std::wstring message = PlayerSetNow->name;
                    message += L" Переміг!"; // додавання тексту
                    //довжина рядка
                    int messageLen = WideCharToMultiByte(CP_UTF8, 0, message.c_str(), -1, nullptr, 0, nullptr, nullptr);
                    std::string messageA(messageLen, 0);
                    //перетворення тексту в код anci
                    WideCharToMultiByte(CP_ACP, 0, message.c_str(), -1, &messageA[0], messageLen, nullptr, nullptr);
                    LPCSTR messageA_lpcstr = messageA.c_str(); // конвертація в LPCSTR
                    //
                    MessageBoxA(hWnd, messageA_lpcstr, (LPCSTR)"кораблик", 0);
                    Peremoga(hWnd);
                    return;
                }

            }
            InvalidateRect(hWnd, NULL, TRUE);
            // перевірка встановлення прапора гри з ботом
                if (((flags & (1 << CHECK_PLAY_WITH_BOT)) != 1))
                    MessageBoxA(hWnd, (LPCSTR)"Влучили! Так тримати!", (LPCSTR)"кораблик", 0);
        }
        else {//якщо не влучили
            //позначити постріл на мапі
            PlayerSetNow->ShipMap.oponentarena[i][j] = 1;

            // перевірка встановлення прапора гри з ботом
            if (((flags & (1 << CHECK_PLAY_WITH_BOT)) != 0)) {
                BotStep(hWnd);
            }
            else {
                //перехід ходу до іншого гравця
                MyPlayers* temp = PlayerSetNow;
                PlayerSetNow = PlayerOponent;
                PlayerOponent = temp;

                InvalidateRect(hWnd, NULL, TRUE);

                //формування рядка для MessageBoxA
                std::wstring message = PlayerSetNow->name;
                message += L" робить постріл!"; // додавання тексту
                //довжина рядка
                int messageLen = WideCharToMultiByte(CP_UTF8, 0, message.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string messageA(messageLen, 0);
                //перетворення тексту в код anci
                WideCharToMultiByte(CP_ACP, 0, message.c_str(), -1, &messageA[0], messageLen, nullptr, nullptr);
                LPCSTR messageA_lpcstr = messageA.c_str(); // конвертація в LPCSTR
                //
                MessageBoxA(hWnd, messageA_lpcstr, (LPCSTR)"кораблик", 0);
            }
        }
        }
    }

}
//set history
void Peremoga(HWND hWnd) {
    //запис результатів:
    //\n\n 
    //гра
    // початок - час, кінець - час2
    // гравець1: ім'я (переміг/програв)
    // n пострілів,н влучань, н потоплень
    // -//-
    //
    //LogfileName

    in.open(LogfileName, std::ios::in | std::ios::ate);
    char bufer[256];
    if (in.is_open()) {
        in << "\n";//новий рядок
        in << "\n";//новий рядок
        in << "гра";
        in << "\n";//новий рядок

        //форматування строки виводу часу під формат години-хвилини-секунди гг:хх:сс
        sprintf_s(bufer, ("%02d:%02d:%02d"),
            (TimeStart / 3600) % 24,
            (TimeStart / 60) % 60,
            (TimeStart % 60));
        in << "початок " << bufer;
        in << ", ";
        TimeEnd = time(0);
        //форматування строки виводу часу під формат години-хвилини-секунди гг:хх:сс
        sprintf_s(bufer, ("%02d:%02d:%02d"),
            (TimeEnd / 3600) % 24,
            (TimeEnd / 60) % 60,
            (TimeEnd % 60));
        in << "кінець " << bufer;
        in << ".";
        in << "\n";//новий рядок

        in << "Поле ";
        in << numKol;
        in << " на ";
        in << numRow;
        in << "\n";//новий рядок

        in << "Гравець1 ";
        //увести ім'я 1
        //формування рядка
            std::wstring message = PlayerSetNow->name;
            message += L" "; // додавання тексту
            //довжина рядка
            int messageLen = WideCharToMultiByte(CP_UTF8, 0, message.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string messageA(messageLen, 0);
            //перетворення тексту в код anci
            WideCharToMultiByte(CP_ACP, 0, message.c_str(), -1, &messageA[0], messageLen, nullptr, nullptr);
            LPCSTR messageA_lpcstr = messageA.c_str(); // конвертація в LPCSTR

        //for (int i = 0; i < WchartSize(Player1.name); i++)
            //in << char(Player1.name[i]);
            in << messageA_lpcstr;
        
            in << ((PlayerSetNow->Ready == 1) ? (" переміг") : (" програв"));
        in << "\n";//новий рядок
        in << PlayerSetNow->CountShot << " пострілів, " << PlayerSetNow->hits << " влучань, " << PlayerSetNow->Kills << " потоплень";
        in << "\n";//новий рядок

        in << "Гравець2 ";
        //формування рядка
         message = PlayerOponent->name;
        message += L" "; // додавання тексту
        //довжина рядка
         messageLen = WideCharToMultiByte(CP_UTF8, 0, message.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string messageA2(messageLen, 0);
        //перетворення тексту в код anci
        WideCharToMultiByte(CP_ACP, 0, message.c_str(), -1, &messageA2[0], messageLen, nullptr, nullptr);
         messageA_lpcstr = messageA2.c_str(); // конвертація в LPCSTR

        //for (int i = 0; i < WchartSize(Player1.name); i++)
            //in << char(Player1.name[i]);
        in << messageA_lpcstr;

        in << ((PlayerOponent->Ready == 1) ? (" переміг") : (" програв"));
        in << "\n";//новий рядок
        in << PlayerOponent->CountShot << " пострілів, " << PlayerOponent->hits << " влучань, " << PlayerOponent->Kills << " потоплень";
        in << "\n";//новий рядок
    }
    in.close();
    ShowWindow(BShowMyArena.hControl, SW_HIDE);
    SendMessageA(hWnd, WM_COMMAND, LOWORD(ID_EXIT_GAME), 0);//кнопка назад
    // ID_EXIT_GAME в кінці
    ResetPlayers();
}
//зкинути значення змінних гравців
void ResetPlayers() {
    Player1.CountShot = 0;
    Player1.hits = 0;
    Player1.IsFocus = 0;
    Player1.Kills = 0;
    Player1.Ready = 0;


    Player2.CountShot = 0;
    Player2.hits = 0;
    Player2.IsFocus = 0;
    Player2.Kills = 0;
    Player2.Ready = 0;
}
//крок боту
void BotStep(HWND hWnd) {
        int i = 0, j = 0;
        //якщо клікнули саме по цій клітинці
        i = rand() % numKol;
        j = rand() % numRow;
        //поки не вибереться тока по якій ще не стріляли
        while (PlayerOponent->ShipMap.oponentarena[i][j] != 0)
        {
            i = rand() % numKol;
            j = rand() % numRow;
        }


        //лічильник пострілів
        PlayerOponent->CountShot++;
        //якщо влучили
        if (PlayerSetNow->ShipMap.myarena[i][j] != 0) {
            //позначити влучення на мапі
            PlayerOponent->ShipMap.oponentarena[i][j] = 2;

            //лічильник влучань
            PlayerOponent->hits++;
            PlayerSetNow->Ships[PlayerSetNow->ShipMap.ShipSize[i][j] - 1].damage[PlayerSetNow->ShipMap.ShipNum[i][j]]++;
            if (PlayerSetNow->Ships[PlayerSetNow->ShipMap.ShipSize[i][j] - 1].damage[PlayerSetNow->ShipMap.ShipNum[i][j]] == PlayerSetNow->ShipMap.ShipSize[i][j]) {
                PlayerSetNow->Ships[PlayerSetNow->ShipMap.ShipSize[i][j] - 1].dead[PlayerSetNow->ShipMap.ShipNum[i][j]] = true;
                //лічильник потоплень
                PlayerOponent->Kills++;
                //якщо перемога
                if (PlayerOponent->Kills == numShips) {
                    PlayerOponent->Ready = 1;//переміг
                    PlayerSetNow->Ready = 0;
                    //запис результатів:
                    //\n\n 
                    //гра
                    // початок - час, кінець - час2
                    // гравець1: ім'я (переміг/програв)
                    // n пострілів,н влучань, н потоплень
                    // -//-
                    //
                    InvalidateRect(hWnd, NULL, TRUE);
                    //формування рядка для MessageBoxA
                    std::wstring message = PlayerOponent->name;
                    message += L" Переміг!"; // додавання тексту
                    //довжина рядка
                    int messageLen = WideCharToMultiByte(CP_UTF8, 0, message.c_str(), -1, nullptr, 0, nullptr, nullptr);
                    std::string messageA(messageLen, 0);
                    //перетворення тексту в код anci
                    WideCharToMultiByte(CP_ACP, 0, message.c_str(), -1, &messageA[0], messageLen, nullptr, nullptr);
                    LPCSTR messageA_lpcstr = messageA.c_str(); // конвертація в LPCSTR
                    //
                    MessageBoxA(hWnd, messageA_lpcstr, (LPCSTR)"кораблик", 0);
                    Peremoga(hWnd);
                    return;
                }

            }
            InvalidateRect(hWnd, NULL, TRUE);
            /*
            //формування рядка для MessageBoxA
            std::wstring message = PlayerOponent->name;
            message += L" Влучив!"; // додавання тексту
            //довжина рядка
            int messageLen = WideCharToMultiByte(CP_UTF8, 0, message.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string messageA(messageLen, 0);
            //перетворення тексту в код anci
            WideCharToMultiByte(CP_ACP, 0, message.c_str(), -1, &messageA[0], messageLen, nullptr, nullptr);
            LPCSTR messageA_lpcstr = messageA.c_str(); // конвертація в LPCSTR
            //
            MessageBoxA(hWnd, messageA_lpcstr, (LPCSTR)"кораблик", 0);
            */
            BotStep(hWnd);
        }
        else {//якщо не влучили
            //позначити постріл на мапі
            PlayerOponent->ShipMap.oponentarena[i][j] = 1;
            InvalidateRect(hWnd, NULL, TRUE);
            /*
            //формування рядка для MessageBoxA
            std::wstring message = PlayerOponent->name;
            message += L" зробив постріл!"; // додавання тексту
            //довжина рядка
            int messageLen = WideCharToMultiByte(CP_UTF8, 0, message.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string messageA(messageLen, 0);
            //перетворення тексту в код anci
            WideCharToMultiByte(CP_ACP, 0, message.c_str(), -1, &messageA[0], messageLen, nullptr, nullptr);
            LPCSTR messageA_lpcstr = messageA.c_str(); // конвертація в LPCSTR
            //
            MessageBoxA(hWnd, messageA_lpcstr, (LPCSTR)"кораблик", 0);
            */
                return;

        }

        return;
}
//отримати текст файлу історії
char* GetHistoryTextlpws() {
    //знайти файл
    HANDLE fileHandle = CreateFileW((LPCWSTR)LogfileName,
        GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    //розмір тексту
    DWORD fileSize = GetFileSize(fileHandle, NULL) + 2;
    //текст
    wchar_t* buffer = (wchar_t*)calloc(fileSize + 1, sizeof(wchar_t));
    //скільки прочитано символів
    DWORD bytesRead = (DWORD)0;

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        return nullptr;
    }
    //читання тексту
    if (!ReadFile(fileHandle, (LPVOID)buffer, fileSize, &bytesRead, NULL))
    {
        return nullptr;
    }
    CloseHandle(fileHandle);
    return (char*)buffer; // возвращает готовый текст из файла в стринг
}



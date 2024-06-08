#include <Windows.h>

// имя класса окна и размеры окна
#define MyWindowClassName "MyWindowClass";
#define MyWindowClassWindowX 300
#define MyWindowClassWindowY 120

// название окна
#define AppName "Day of week and L2 cashe size"

// размер строки результата и её положение в окне
#define AResultTextBufferSize 128
#define AResultTextBufferX 10
#define AResultTextBufferY 10


// структура в которую записываем результы вызовов библиотечной функции
typedef struct _MyFuncResult
{
	DWORD aL2CasheSize;
	WORD aDayOfWeek;
} MyFuncResult, *PMyFuncResult;

MyFuncResult AResult;
CHAR AResultTextBuffer[AResultTextBufferSize] = "Day of week: ";


// функция, запускаема в отдельном потоке
DWORD WINAPI ThreadFunc (PVOID pParameter)
{
	int aLibraryFuncResult;

	// объявляем функцию и создаем указатель на неё
	typedef int (*ImportFunction) (PMyFuncResult);
	ImportFunction GetDayOfWeekAndL2CasheSize;

	// загружаем динамическую библиотеку
	HINSTANCE hLibInstance = LoadLibrary(TEXT("RGRDLL.dll"));
	if (hLibInstance != NULL) {
		// получаем указатель на нуфную функцию из библиотеки
		GetDayOfWeekAndL2CasheSize = (ImportFunction)GetProcAddress(hLibInstance, "GetDayOfWeekAndL2CasheSize");

		// вызываем её
		aLibraryFuncResult = GetDayOfWeekAndL2CasheSize(&AResult);

		// выгружаем библиотеку
		FreeLibrary(hLibInstance);
	}
	else {
		strcpy_s(AResultTextBuffer, AResultTextBufferSize, "Failed to load RGRDLL.dll");
		return 1;
	}

	// объявляем переменные, необходимые для корректной записи в строку результата
	size_t sBufferCurSize;
	CHAR aBytesPostfix[4] = "B";

	// записываем день недели в строку результата
	switch (AResult.aDayOfWeek)
	{
	case 0:
		strcat_s(AResultTextBuffer, AResultTextBufferSize, "Sunday (0)");
		break;

	case 1:
		strcat_s(AResultTextBuffer, AResultTextBufferSize, "Monday (1)");
		break;

	case 2:
		strcat_s(AResultTextBuffer, AResultTextBufferSize, "Tuesday (2)");
		break;

	case 3:
		strcat_s(AResultTextBuffer, AResultTextBufferSize, "Wednesday (3)");
		break;

	case 4:
		strcat_s(AResultTextBuffer, AResultTextBufferSize, "Thursday (4)");
		break;

	case 5:
		strcat_s(AResultTextBuffer, AResultTextBufferSize, "Friday (5)");
		break;

	case 6:
		strcat_s(AResultTextBuffer, AResultTextBufferSize, "Saturday (6)");
		break;

	default:
		strcat_s(AResultTextBuffer, AResultTextBufferSize, "Unknown (");
		sBufferCurSize = strlen(AResultTextBuffer);
		_itoa_s(
			AResult.aDayOfWeek,
			AResultTextBuffer + sBufferCurSize,
			AResultTextBufferSize - sBufferCurSize,
			10
		);
		strcat_s(AResultTextBuffer, AResultTextBufferSize, ")");
		break;
	}

	// записываем размер кеша L2 в строку результата
	strcat_s(AResultTextBuffer, AResultTextBufferSize, "\nL2 Cashe size: ");
	sBufferCurSize = strlen(AResultTextBuffer);

	if (aLibraryFuncResult != 0) {
		strcat_s(AResultTextBuffer, AResultTextBufferSize, "Unknown");
		return 1;
	}

	if ((AResult.aL2CasheSize & 0xFFFFF) == 0) {
		AResult.aL2CasheSize >>= 20;
		strcpy_s(aBytesPostfix, 4, "MB");
	}
	else if ((AResult.aL2CasheSize & 0x3FF) == 0) {
		AResult.aL2CasheSize >>= 10;
		strcpy_s(aBytesPostfix, 4, "KB");
	}
	_itoa_s(
		AResult.aL2CasheSize,
		AResultTextBuffer + sBufferCurSize,
		AResultTextBufferSize - sBufferCurSize,
		10
	);
	strcat_s(AResultTextBuffer, AResultTextBufferSize, aBytesPostfix);

	return 0;
}



// функция обработки сообщении окна
LRESULT CALLBACK WindowFunc
(
	HWND   hWindowHandle,
	UINT   aMessage,
	WPARAM pParameters,
	LPARAM lpParameters
)
{
	// структура, которая содержит дескриптор контекста устройтва и прямоугольник для рисования
	PAINTSTRUCT aPaintStruct;
	
	switch (aMessage)
	{
	// до создания окна
	case WM_CREATE: {
		// создаем поток, в котором выполняется функция для подготовки строки результата
		HANDLE hThreadHandle = CreateThread(NULL, 0, ThreadFunc, NULL, 0, NULL);
		if (hThreadHandle == NULL)
			return 1;
		
		// ожидаем окончания работы функции и закрываем поток
		WaitForSingleObject(hThreadHandle, INFINITE);
		CloseHandle(hThreadHandle);

		break;
	}
	// перед отображением
	case WM_PAINT:
		// подготавливаем окно для рисования и заполняем структуру aPaintStruct
		BeginPaint(hWindowHandle, &aPaintStruct);
		// изменяем область отрисовкм
		SetRect(
			&(aPaintStruct.rcPaint),
			AResultTextBufferX,
			AResultTextBufferY,
			MyWindowClassWindowX - AResultTextBufferX,
			MyWindowClassWindowY - AResultTextBufferY
		);
		// отрисовываем строку реультата в окне в измененной области
		DrawText(
			aPaintStruct.hdc,
			AResultTextBuffer,
			-1,
			&(aPaintStruct.rcPaint),
			DT_LEFT | DT_EXTERNALLEADING | DT_WORDBREAK);

		// заканчиваем рисование в окне
		EndPaint(hWindowHandle, &aPaintStruct);

		break;

	// при мастабировании
	case WM_GETMINMAXINFO: {
		PMINMAXINFO pMinMaxInfo = (PMINMAXINFO)lpParameters;
		pMinMaxInfo->ptMinTrackSize.x = MyWindowClassWindowX;
		pMinMaxInfo->ptMinTrackSize.y = MyWindowClassWindowY;

		break;
	}
	// при закрытии окна
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	// все отсальные сообщения обрабатывать стандартно
	default:
		return DefWindowProc(
			hWindowHandle,
			aMessage,
			pParameters,
			lpParameters
		);
	}

	return 0;
}



// точка входа
int WINAPI WinMain
(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	// инициализируем класс окна
	WNDCLASS aWindowClass;
	aWindowClass.lpszClassName = (PCSTR)MyWindowClassName;				// имя класса
	aWindowClass.hInstance = hInstance;									// дескриптор процесса
	aWindowClass.lpfnWndProc = WindowFunc;								// функция обработки сообщений
	aWindowClass.style = CS_HREDRAW | CS_VREDRAW;						// перерисовка при маштабировании
	aWindowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);				// стандартная иконка
	aWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);					// стандартный курсор
	aWindowClass.lpszMenuName = NULL;									// отсутствие меню
	aWindowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	// кисть для фона
	aWindowClass.cbClsExtra = 0;										// дополнительные параметры
	aWindowClass.cbWndExtra = 0;

	// регистрируем класс окна
	if (!RegisterClass(&aWindowClass)) {
		return MessageBox(NULL, "Failed to register WindowClass", "Error", MB_ICONERROR);
	}

	// создаем окно зарегистрированного класса
	HWND hWindowHandle = CreateWindow(
		aWindowClass.lpszClassName,
		AppName,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		CW_USEDEFAULT, CW_USEDEFAULT, MyWindowClassWindowX, MyWindowClassWindowY,
		HWND_DESKTOP,
		NULL,
		hInstance,
		NULL
	);
	if (hWindowHandle == NULL) {
		return MessageBox(NULL, "Failed to create Window", "Error", MB_ICONERROR);
	}

	// отображаем окно
	ShowWindow(hWindowHandle, nShowCmd);
	//UpdateWindow(hWindowHandle);

	MSG aMessage;
	// запускаем цикл обработки сообщений
	while (GetMessage(&aMessage, NULL, 0, 0)) {
		TranslateMessage(&aMessage);
		DispatchMessage(&aMessage);
	}

	return (int)aMessage.wParam;
}

#include <Windows.h>
#include <iostream>


#define T(s) (TEXT(#s))
typedef unsigned long long QWORD;


LPCTSTR Title = T(MyWindowTitle);

LRESULT CALLBACK WndProc(HWND hWnd, UINT imsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
WNDPROC OldEditProc;

HINSTANCE g_hInst;
HWND hMainWnd;


void GetDefinition(LPTSTR Word, int len);

ATOM RegisterCustomClass(HINSTANCE hInst)
{
	WNDCLASS WC;
	WC.cbClsExtra = 0;
	WC.cbWndExtra = 0;
	WC.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_WINDOW - 1);
	WC.hCursor = LoadCursor(NULL, IDC_ARROW);
	WC.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WC.hInstance = hInst;
	WC.lpfnWndProc = WndProc;
	WC.lpszClassName = Title;
	WC.lpszMenuName = NULL;
	WC.style = CS_VREDRAW | CS_HREDRAW;

	return RegisterClass(&WC);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
	g_hInst = hInst;
	RegisterCustomClass(hInst);
	hMainWnd = CreateWindow(Title, Title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInst, NULL);
	ShowWindow(hMainWnd, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
HWND hEdit;
HWND hReport;
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CREATE:
		hEdit = CreateWindow(T(edit), NULL, WS_CHILDWINDOW | WS_BORDER | ES_UPPERCASE | ES_MULTILINE|WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, g_hInst, NULL);
		hReport = CreateWindow(T(edit), NULL, WS_CHILDWINDOW | WS_BORDER | ES_UPPERCASE | ES_MULTILINE | WS_VISIBLE,0,0,0,0, hWnd, NULL, g_hInst, NULL);
		OldEditProc = (WNDPROC)SetWindowLong(hEdit, GWL_WNDPROC, (LONG)&EditProc);
		return 0;
	case WM_SIZE:
		SetWindowPos(hEdit, 0, 0, 0,LOWORD(lParam),HIWORD(lParam)*2/3, SWP_NOMOVE);
		SetWindowPos(hReport, 0, 0, HIWORD(lParam) * 2 / 3, LOWORD(lParam), HIWORD(lParam) * 1 / 3,NULL);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

LRESULT CALLBACK EditProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static bool Tracking = false;

	switch (iMsg)
	{
	case WM_CREATE:
	{
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		if (!Tracking)
		{
			Tracking = true;
			TRACKMOUSEEVENT TM;
			TM.cbSize = sizeof(TM);
			TM.dwFlags = TME_HOVER | TME_LEAVE;
			TM.dwHoverTime = HOVER_DEFAULT;
			TM.hwndTrack = hWnd;
			bool b = (bool)TrackMouseEvent(&TM);
		}
		return 0;
	}
	case WM_MOUSELEAVE:
	{
		Tracking = false;
		return 0;
	}
	case WM_MOUSEHOVER: //word detection
	{
		DWORD Index = SendMessage(hWnd, EM_CHARFROMPOS, wParam, lParam); //line = HIWORD, charindex = LOWORD
		int length = SendMessage(hWnd, EM_LINELENGTH, LOWORD(Index), 0);
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		TCHAR* line = new TCHAR[length+1]();
		int linenum = HIWORD(Index);
		WORD* firstwordoftheline = (WORD*)line;
		*firstwordoftheline = length + 1;
		int len = SendMessage(hWnd, EM_GETLINE, HIWORD(Index), (LPARAM)line);


		DWORD charStart = SendMessage(hWnd, EM_LINEINDEX, HIWORD(Index), 0);

		int w_start, w_end;

		if (line[LOWORD(Index) - charStart] != ' ')
		{
			for (w_start = LOWORD(Index) - charStart; w_start >= 0; --w_start)
			{
				if (line[w_start] == ' ')
				{
					break;
				}
			}
			++w_start;
			for (w_end = LOWORD(Index) - charStart; w_end < length; ++w_end)
			{
				if (line[w_end] == ' ')
				{
					line[w_end] = '\0';
					break;
				}
			}

			while (w_start < w_end)
			{
				if (!IsCharAlpha(line[w_start]))
					++w_start;
				else if (!IsCharAlpha(line[w_end]))
					--w_end;
				else break;
			}
			++w_end;
			GetDefinition(line + w_start, w_end - w_start);

		}
		delete[] line;
		Tracking = false;
		return 0;
	}

	}
	return OldEditProc(hWnd, iMsg, wParam, lParam);
}

#ifdef _UNICODE
#define tstrcpy(x1,x2) lstrcpy(x1,x2)
#else
#define tstrcpy(x1,x2) strcpy(x1,x2)
#endif

int custom_strcmp(char* s1, char* s2) //s1 ends on '\0', s2 ends on ' '
{
	while (*s1 != '\0' && *s2 != ' ')
	{
		if (*s1 > *s2) return 1;
		if (*s1 < *s2) return -1;
		++s1, ++s2;
	}
	if (*s1 == '\0' && *s2 != ' ')
		return -1;
	if (*s1 != '\0' && *s2 == ' ')
		return 1;
	return 0;
}
void GetDefinition(LPTSTR Word, int len)
{
	if (len>0)
	{
		TCHAR* FileName = new TCHAR[10];
		wsprintf(FileName, T(Dic\\%c.txt\0), Word[0]);

		char* c_str = new char[len + 1];
		for (int i = 0; i < len; ++i)
			c_str[i] = Word[i];
		c_str[len] = 0;

		HANDLE hDic = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hDic == INVALID_HANDLE_VALUE)
			return;
		HANDLE hFMap = CreateFileMapping(hDic, NULL, PAGE_READONLY, NULL, NULL, NULL);
		if (hFMap == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hDic);
			return;
		}
		char* pMap = (char*)MapViewOfFile(hFMap, FILE_MAP_READ, 0, 0, NULL);

		LARGE_INTEGER a;
		GetFileSizeEx(hDic, &a);

		int begin = 0;
		int end = a.QuadPart;
		int mid;

		bool failed = false;

		while (begin < end)
		{
			mid = (begin + end) / 2;
			while (pMap[mid] != '\n') --mid;
			while (!isalpha(pMap[mid]))
				++mid; //now mid is at start of some word.
			if (custom_strcmp(c_str, pMap + mid) < 0)
			{
				if (end == mid)
				{
					failed = true;
					break;
				}
				end = mid;
			}
			else if (custom_strcmp(c_str, pMap + mid) > 0)
			{
				if (begin == mid)
				{
					failed = true;
					break;
				}
				begin = mid;
			}
			else break;
		}
		end = mid;
		while (pMap[end] != '\n') ++end;

		if (!failed)
		{
			TCHAR* DefText = new TCHAR[end - mid + 1]();
			for (int i = mid; i < end; ++i)
				DefText[i - mid] = pMap[i];

			SetWindowText(hReport, DefText);
			delete[] DefText;
		}
		else
		{
			SetWindowText(hReport, T(Failed to load definition.));
		}

		UnmapViewOfFile(pMap);
		CloseHandle(hFMap);
		CloseHandle(hDic);
	}
	
}

void ShowDefinition()
{

}
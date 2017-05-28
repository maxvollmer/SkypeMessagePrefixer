
#include "stdafx.h"

BOOL CALLBACK FindSkypeWindow(_In_ HWND hwnd, _In_ LPARAM lParam);
BOOL CALLBACK FindSkypeMessageInput(_In_ HWND hwnd, _In_ LPARAM lParam);
void PrefixSkypeMessageInput(HWND hwnd);
void PrefixSkypeMessage(std::wstring &message);

const char SKYPE_WINDOW_TITLE_PREFIX[] = "Skype™? - ";
const char SKYPE_WINDOW_CLASSNAME[] = "tSkMainForm";
const char SKYPE_CHATINPUT_WINDOW_CLASSNAME[] = "TChatRichEdit";
const wchar_t SKYPE_MESSAGE_PREFIX[] = L"!! ";

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	while (true)
	{
		EnumChildWindows(NULL, FindSkypeWindow, NULL);
		Sleep(50);
	}
}

BOOL CALLBACK FindSkypeWindow(_In_ HWND hwnd, _In_ LPARAM lParam)
{
	DWORD dwResult = 0;
	char windowTitle[256];
	windowTitle[0] = 0;
	SendMessageTimeoutA(hwnd, WM_GETTEXT, 256, (LPARAM)windowTitle, SMTO_ABORTIFHUNG, 100, &dwResult);

	char windowClassname[256];
	windowClassname[0] = 0;
	GetClassNameA(hwnd, windowClassname, 256);

	bool foundSkypeWindow = std::string(windowTitle).find(SKYPE_WINDOW_TITLE_PREFIX) == 0 && strcmp(SKYPE_WINDOW_CLASSNAME, windowClassname) == 0;

	if (foundSkypeWindow)
	{
		EnumChildWindows(hwnd, FindSkypeMessageInput, NULL);
	}

	return foundSkypeWindow ? FALSE : TRUE;
}

BOOL CALLBACK FindSkypeMessageInput(_In_ HWND hwnd, _In_ LPARAM lParam)
{
	char windowClassname[256];
	windowClassname[0] = 0;
	GetClassNameA(hwnd, windowClassname, 256);

	if (strcmp(SKYPE_CHATINPUT_WINDOW_CLASSNAME, windowClassname) == 0)
	{
		PrefixSkypeMessageInput(hwnd);
	}

	EnumChildWindows(hwnd, FindSkypeMessageInput, NULL);
	return TRUE;
}

void PrefixSkypeMessageInput(HWND hwnd)
{
	DWORD dwResult = 0;
	int windowTextLength = GetWindowTextLengthW(hwnd) + 256;
	wchar_t* windowText = new wchar_t[windowTextLength];
	windowText[0] = 0;
	SendMessageTimeoutW(hwnd, WM_GETTEXT, windowTextLength, (LPARAM)windowText, SMTO_ABORTIFHUNG, 100, &dwResult);

	std::wstring windowTextString = windowText;
	std::wstring fixedWindowTextString = windowTextString;
	PrefixSkypeMessage(fixedWindowTextString);

	if (fixedWindowTextString.compare(windowTextString) != 0)
	{
		SendMessageTimeoutW(hwnd, WM_SETTEXT, NULL, (LPARAM)fixedWindowTextString.c_str(), SMTO_ABORTIFHUNG, 100, &dwResult);
		SendMessageTimeoutW(hwnd, EM_SETSEL, (WPARAM)fixedWindowTextString.length(), (LPARAM)fixedWindowTextString.length(), SMTO_ABORTIFHUNG, 100, &dwResult);
	}

	delete[] windowText;
}

void PrefixSkypeMessage(std::wstring &message)
{
	// if message is a link, don't add prefix
	if (message.find(L"http://") == 0 || message.find(L"https://") == 0 || message.find(L"www.") == 0)
	{
		return;
	}

	// if message is a prefixed link, remove prefix
	if (message.find(L"!! http://") == 0 || message.find(L"!! https://") == 0 || message.find(L"!! www.") == 0)
	{
		message = message.substr(3).c_str();
		return;
	}

	// if message is empty or already prefixed, don't add prefix
	if (trim(std::wstring(message)).length() > 0 && message.find(SKYPE_MESSAGE_PREFIX) != 0)
	{
		// do some additional checks, so we don't get stuff like "!! ! !!" when the user deletes one of the prefix characters
		message = trim(message);
		if (message.find(L"!") == 0)
		{
			message = message.substr(1);
			if (message.find(L"!") == 0)
			{
				message = message.substr(1);
			}
			if (message.find(L" ") == 0)
			{
				message = message.substr(1);
			}
		}

		// finally add prefix
		message = SKYPE_MESSAGE_PREFIX + message;
	}
}

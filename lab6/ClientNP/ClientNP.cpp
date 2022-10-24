#include <iostream>
#include <clocale>
#include "WSAErrors.h"
#include "Windows.h"
#define NAME L"\\\\.\\pipe\\Tube"
#define STOP "STOP"
#define EXIT "EXIT"

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "rus");
    HANDLE hPipe;
    DWORD lp;
    char buff[50];
    try
    {
        if ((hPipe = CreateFile(NAME,GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_EXISTING, NULL, NULL)) == INVALID_HANDLE_VALUE)
            throw set_error_msg("CreateFile: ", GetLastError());
        int messages_count = 0;
        std::cout << "Enter a number of messages: " << std::endl;
        std::cin >> messages_count;
        for (int i = 0; i < messages_count; i++)
        {
            string msg = "Message #" + to_string(i + 1);
            if (!WriteFile(hPipe, msg.c_str(), msg.length() + 1, &lp, NULL))
                throw set_error_msg("WriteFile: ", GetLastError());
            if (!ReadFile(hPipe, buff, sizeof(buff), &lp, NULL))
                throw set_error_msg("ReadFile: ", GetLastError());
            std::cout << buff << std::endl;
        }
        std::cout << "Enter 'EXIT' to stop server" << std::endl;
        std::cin >> buff;
        if (!strcmp(buff, EXIT))
        {
            if (!WriteFile(hPipe, EXIT, strlen(EXIT) + 1, &lp, NULL))
                throw set_error_msg("WriteFile: ", GetLastError());
        }
        else
        {
            if (!WriteFile(hPipe, STOP, strlen(STOP) + 1, &lp, NULL))
                throw set_error_msg("WriteFile: ", GetLastError());
        }
        if (!CloseHandle(hPipe))
            throw set_error_msg("CloseHandle: ", GetLastError());
        system("pause");
    }
    catch (string error)
    {
        cout << error << endl;
    }
}

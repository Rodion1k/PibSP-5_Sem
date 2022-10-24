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
    DWORD mode = PIPE_READMODE_MESSAGE;
    char buff[50];
    try
    {
        if ((hPipe = CreateFile(NAME,GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_EXISTING, NULL, NULL)) == INVALID_HANDLE_VALUE)
            throw set_error_msg("CreateFile: ", GetLastError());
        if (!SetNamedPipeHandleState(hPipe, &mode, NULL, NULL))
            throw set_error_msg("SetNamedPipeHandleState: ", GetLastError());
        int messages_count = 0;
        std::cout << "Enter a number of messages: " << std::endl;
        std::cin >> messages_count;
        for (int i = 0; i < messages_count; i++)
        {
            string msg = "Message #" + to_string(i + 1);
            char obuff[50];
            strcpy_s(obuff, msg.c_str());
            if (!TransactNamedPipe(hPipe, obuff, sizeof(obuff), buff, sizeof(buff), &lp, NULL))
                throw set_error_msg("TransactNamedPipe: ", GetLastError());
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

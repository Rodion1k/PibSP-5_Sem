#include <iostream>
#include <clocale>
#include "WSAErrors.h"
#define NAME L"\\\\.\\pipe\\Tube"

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "rus");
    HANDLE hPipe;
    DWORD lp;
    bool is_work = true;
    char buf[50];
    try
    {
        if ((hPipe = CreateNamedPipe(NAME,PIPE_ACCESS_DUPLEX,PIPE_TYPE_MESSAGE | PIPE_WAIT, 1,NULL,NULL, INFINITE,NULL)) 
            == INVALID_HANDLE_VALUE)
            throw set_error_msg("CreateNamedPipe: ", GetLastError());
        std::cout << "Server waiting..." << std::endl;
        while (true)
        {
            if (!ConnectNamedPipe(hPipe,NULL))
                throw set_error_msg("ConnectNamedPipe: ", GetLastError());
            while (true)
            {
                if (ReadFile(hPipe, buf, sizeof(buf), &lp,NULL))
                {
                    if (strcmp(buf, "STOP") == 0)
                        break;
                    if (strcmp(buf, "EXIT") == 0)
                    {
                        is_work = false;
                        break;
                    }
                    std::cout << buf << std::endl;
                    if (WriteFile(hPipe, buf, sizeof(buf), &lp, NULL))
                    {
                        if (strstr(buf, "ClientNPct"))
                        {
                            break;
                        }
                    }
                    else
                    {
                        throw set_error_msg("WriteFile: ", GetLastError());
                    }
                }
                else
                {
                    throw set_error_msg("ReadFile: ", GetLastError());
                }
            }
            if (!DisconnectNamedPipe(hPipe))
            {
                throw set_error_msg("DisconnectNamedPipe: ", GetLastError());
            }
            std::cout << "Client disconnected" << std::endl;
            if (!is_work)
                break;
        }
        if (!CloseHandle(hPipe))
        {
            throw set_error_msg("CloseHandle: ", GetLastError());
        }
        std::cout << "Server stopped" << std::endl;
        system("pause");
    }
    catch (string error)
    {
        std::cout << error << std::endl;
    }
}

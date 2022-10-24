#include "WSAErrors.h"
#include <iostream>
#include <ctime>
#define NAME L"\\\\.\\mailslot\\Box"

int main(int argc, char* argv[])
{
    HANDLE handle_mail;
    DWORD write_length;
    int count;
    char write_buff[] = "Hello from client";
    try
    {
        if ((handle_mail = CreateFile(NAME,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL)) ==
            INVALID_HANDLE_VALUE)
            throw set_error_msg("CreateFile", GetLastError());

        std::cout << "Client: Mailslot opened" << std::endl
            << "enter count of messages for sending to server: ";
        cin >> count;
        time_t start = clock();
        for (int i = 0; i < count; i++)
        {
            if (!WriteFile(handle_mail, write_buff, sizeof(write_buff), &write_length, NULL))
                throw set_error_msg("WriteFile", GetLastError());
            std::cout << "Client: Message sent: " << i << std::endl;
        }
        std::cout << "Client: All messages sent" << std::endl;
        strcpy_s(write_buff, "STOP");
        if (!WriteFile(handle_mail, write_buff, sizeof(write_buff), &write_length, NULL))
            throw set_error_msg("WriteFile", GetLastError());
        time_t end = clock();
        std::cout << "time for sending " << count << " messages = " << (end - start) << std::endl;
        if (!CloseHandle(handle_mail))
            throw set_error_msg("CloseHandle", GetLastError());
    }
    catch (string error)
    {
        std::cout << "error: " << error << std::endl;
    }
    system("pause");
}

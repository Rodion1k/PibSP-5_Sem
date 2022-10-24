#include "WSAErrors.h"
#include<iostream>
#include "Windows.h"
#define NAME L"\\\\.\\mailslot\\Box"

int main(int argc, char* argv[])
{
    HANDLE handle_mail;
    DWORD message_length;
    char mess_buff[100];

    try
    {
        if ((handle_mail = CreateMailslot(NAME,NULL,MAILSLOT_WAIT_FOREVER,NULL)) == INVALID_HANDLE_VALUE)
            throw set_error_msg("CreateMailslot: ", GetLastError());
        std::cout << "server waiting..." << std::endl;
        while (true)
        {
            if (!ReadFile(handle_mail, mess_buff, sizeof(mess_buff), &message_length,NULL))
                throw set_error_msg("ReadFile: ", GetLastError());
            if (strcmp(mess_buff, "STOP") == 0)
                break;
            std::cout << "message: " << mess_buff << std::endl;
        }
        if (!CloseHandle(handle_mail))
            throw set_error_msg("CloseHandle:", GetLastError());
    }
    catch (string error)
    {
        std::cout << error << std::endl;
    }
    system("pause");
}

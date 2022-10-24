#include <iostream>
#include <clocale>
#include "WSAErrors.h"
#include "Windows.h"
#define NAME L"\\\\.\\pipe\\Tube"


int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "rus");
    DWORD lp;
    char buff[50], buff2[50];
    try
    {
        int messages_count = 0;
        std::cout << "Enter a number of messages: " << std::endl;
        std::cin >> messages_count;
        for (int i = 0; i < messages_count; i++)
        {
            string msg = "Message ClientNPct #" + to_string(i + 1);
            strcpy_s(buff2, msg.c_str());
            if (!CallNamedPipe(NAME, buff2, sizeof(buff2), buff, sizeof(buff), &lp, NMPWAIT_WAIT_FOREVER))
            {
                throw set_error_msg("CallNamedPipe: ", GetLastError());
            }
            std::cout << buff << std::endl;
        }

        system("pause");
    }
    catch (string error)
    {
        cout << error << endl;
    }
}

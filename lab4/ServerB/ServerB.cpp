#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WS2tcpip.h>
#include "WSAErrors.h"
#pragma comment(lib, "Ws2_32.lib")

SOCKET serv_sock;

bool get_request_from_client(char* name, short port, sockaddr* from, int* flen)
{
    char buff[50];
    memset(from, 0, sizeof(flen));

    if ((recvfrom(serv_sock, buff, sizeof(buff), NULL, from, flen)) == SOCKET_ERROR)
    {
        if (WSAGetLastError() == WSAETIMEDOUT)
            return false;
        throw set_error_msg("recv:", WSAGetLastError());
    }
    std::cout << buff << std::endl;
    if (strcmp(buff, name) == 0)
    {
        return true;
    }
    return false;
}

bool check_server(char* name, short port, sockaddr* from, int* flen)
{
    char serverName[50];
    try
    {
        if ((serv_sock = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET)
            throw set_error_msg("socket:", WSAGetLastError());
        int optval = 1;
        if (setsockopt(serv_sock, SOL_SOCKET, SO_BROADCAST,
                       (char*)&optval, sizeof(int)) == SOCKET_ERROR)
            throw set_error_msg("opt: ", WSAGetLastError());

        SOCKADDR_IN all;
        all.sin_family = AF_INET;
        all.sin_port = htons(2000);
        all.sin_addr.s_addr = INADDR_BROADCAST;
        int sendLen = 0;

        if ((sendLen = sendto(serv_sock, name, sizeof(name), NULL,
                              (sockaddr*)&all, sizeof(all))) == SOCKET_ERROR)
            throw set_error_msg("sendto: ", WSAGetLastError());

        if (recvfrom(serv_sock, serverName, sizeof(serverName), NULL, from, flen) == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAETIMEDOUT)return false;
            throw set_error_msg("recv: ", WSAGetLastError());
        }

        if (strcmp(serverName, name) == 0)
        {
            cout << " Found same server!!!" << endl;
            SOCKADDR_IN* addr = (SOCKADDR_IN*)from;
            cout << "Servers's port: " << addr->sin_port << "\n";
            cout << "Server's IP: " << inet_ntoa(addr->sin_addr) << "\n";
            closesocket(serv_sock);
            return true;
        }
        closesocket(serv_sock);
        return false;
    }
    catch (string errorMsgText)
    {
        cout << endl << "Error: " << errorMsgText;
        return false;
    }
}

bool put_answer_to_client(const char* name, sockaddr* to, int* lto)
{
    if ((sendto(serv_sock, name, strlen(name) + 1, NULL, to, *lto)) == SOCKET_ERROR)
    {
        throw set_error_msg("send:", WSAGetLastError());
    }

    return true;
}

int main(int argc, char* argv[])
{
    WSADATA wsa_data;
    try
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
            throw set_error_msg("WSAStartup: ", WSAGetLastError());
        SOCKADDR_IN serv;
        serv.sin_family = AF_INET;
        serv.sin_port = htons(2000);
        serv.sin_addr.s_addr = INADDR_ANY;

        SOCKADDR_IN clnt;
        int lc = sizeof(clnt);
        char name[] = "Hello";

        check_server(name, 2000, (sockaddr*)&clnt, &lc);

        if ((serv_sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
        {
            WSACleanup();
            throw set_error_msg("socket: ", WSAGetLastError());
        }

        if (bind(serv_sock, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
        {
            throw set_error_msg("bind: ", WSAGetLastError());
        }


        while (true)
        {
            if (get_request_from_client(name, 2000, (sockaddr*)&clnt, &lc))
                std::cout << std::endl << put_answer_to_client(name, (sockaddr*)&clnt, &lc);
            else
            {
                std::cout << std::endl << "Wrong request" << std::endl;
                // std::cout << std::endl << put_answer_to_client("**Eror name**", (sockaddr*)&clnt, &lc);
            }
            std::cout << "ADDRES client :          " << inet_ntoa(clnt.sin_addr) << " : " << htons(clnt.sin_port) <<
                endl;
        }
    }
    catch (string message)
    {
        cout << message << endl;
        closesocket(serv_sock);
        WSACleanup();
    }
}

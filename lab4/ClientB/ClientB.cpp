#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <tchar.h>
#include "Winsock2.h"
#include "WSAErrors.h"

#pragma comment(lib, "WS2_32.lib")

SOCKET client;

bool get_server(char* call, short port, sockaddr* from, int* flen)
{
    memset(from, 0, *flen);
    if ((client = socket(AF_INET,SOCK_DGRAM,NULL)) == INVALID_SOCKET)
        throw set_error_msg("socket() failed", WSAGetLastError());
    int optval = 1;
    if (setsockopt(client,SOL_SOCKET,SO_BROADCAST, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
        throw set_error_msg("setsockopt() failed", WSAGetLastError());
    SOCKADDR_IN all;
    all.sin_family = AF_INET;
    all.sin_port = htons(port);
    all.sin_addr.s_addr = INADDR_BROADCAST;

    if (sendto(client, call, strlen(call) + 1, NULL, (sockaddr*)&all, sizeof(all)) == SOCKET_ERROR)
        throw set_error_msg("sendto() failed", WSAGetLastError());

    char server_name[50];
    if (recvfrom(client, server_name, sizeof(server_name),NULL, from, flen) == SOCKET_ERROR)
    {
        if (WSAGetLastError() == WSAETIMEDOUT)
            return false;
        throw set_error_msg("recvfrom() failed", WSAGetLastError());
    }
    SOCKADDR_IN* addr = (SOCKADDR_IN*)&from;
    std::cout << std::endl << "server port: " << addr->sin_port;
    std::cout << std::endl << "IP-server: " << inet_ntoa(addr->sin_addr);
    if (strcmp(server_name, call) == 0)
    {
        std::cout << std::endl << "server with this name exists.\n";
        return true;
    }
    std::cout << std::endl << "server with this name does not  exists.\n";
    return false;
}

int main(int argc, char* argv[])
{
    WSADATA wsa_data;
    try
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
            throw set_error_msg("WSAStartup() failed", WSAGetLastError());
        char call[] = "Hello";
        SOCKADDR_IN client_addr;
        int lc = sizeof(client_addr);
        get_server(call, 2000, (sockaddr*)&client_addr, &lc);
        if (closesocket(client) == SOCKET_ERROR)
            throw set_error_msg("closesocket() failed", WSAGetLastError());
        if (WSACleanup() == SOCKET_ERROR)
            throw set_error_msg("WSACleanup() failed", WSAGetLastError());
    }
    catch (std::string error_msg)
    {
        std::cout << std::endl << error_msg;
    }
    system("pause");
}

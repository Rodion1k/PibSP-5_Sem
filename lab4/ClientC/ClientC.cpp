#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <tchar.h>
#include "Winsock2.h"
#include "WSAErrors.h"

#pragma comment(lib, "WS2_32.lib")

SOCKET client;

bool get_server_by_name(char* name, char* call, sockaddr* from, int* flen)
{
    memset(from, 0, *flen);
    hostent* host = gethostbyname(name);
    if (host == NULL)
        throw set_error_msg("gethostbyname() failed: ", WSAGetLastError());

    char* ip_addr = inet_ntoa(*(in_addr*)(host->h_addr));
    cout << "\nHost name: " << host->h_name;
    cout << "\nIP server: " << ip_addr;

    SOCKADDR_IN server_in;
    server_in.sin_family = AF_INET;
    server_in.sin_port = htons(2000);
    server_in.sin_addr.s_addr = inet_addr(ip_addr);

    char message[10];

    if (sendto(client, call, strlen(call) + 1, 0, (sockaddr*)&server_in, *flen) == SOCKET_ERROR)
        throw set_error_msg("sendto() failed: ", WSAGetLastError());

    if ((recvfrom(client, message, sizeof(message), NULL, from, flen)) == SOCKET_ERROR)
        throw set_error_msg("recvfrom:", WSAGetLastError());

    std::cout << "\nmessage from server: " << message << std::endl;
    return true;
}

int main(int argc, char* argv[])
{
    WSADATA wsa_data;
    try
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
            throw set_error_msg("WSAStartup() failed", WSAGetLastError());
        if ((client = socket(AF_INET,SOCK_DGRAM,NULL)) == INVALID_SOCKET)
            throw set_error_msg("socket() failed", WSAGetLastError());
        char call[] = "Hello";
        char name[] = "Rodion";
        SOCKADDR_IN client_addr;
        int lc = sizeof(client_addr);

        get_server_by_name(name, call, (sockaddr*)&client_addr, &lc);
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

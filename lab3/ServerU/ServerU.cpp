#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#include "WSAErrors.h"
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    int count = 0;
    WSADATA wsa_data;
    SOCKET serv_sock;
    SOCKADDR_IN serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(2000);
    serv.sin_addr.s_addr = INADDR_ANY;
    try
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
            throw set_error_msg("WSAStartup: ", WSAGetLastError());
        if ((serv_sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
        {
            WSACleanup();
            throw set_error_msg("socket: ", WSAGetLastError());
        }
        if (bind(serv_sock, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
        {
            closesocket(serv_sock);
            WSACleanup();
            throw set_error_msg("bind: ", WSAGetLastError());
        }
        cout << "Server started" << endl;
        SOCKADDR_IN client;
        int client_size = sizeof(client);
        ZeroMemory(&client, sizeof(client));
        std::vector<char> serv_buff(255), client_buff(255);
        while (true)
        {
            Sleep(10);
            if (recvfrom(serv_sock, serv_buff.data(), 255, 0, (sockaddr*)&client, &client_size) == SOCKET_ERROR)
            {
                closesocket(serv_sock);
                WSACleanup();
                throw set_error_msg("recvfrom: ", WSAGetLastError());
            }
            if (strcmp(serv_buff.data(), "") == 0)
            {
                break;
            }
            cout << "Client: " << serv_buff.data() << endl;
            client_buff = serv_buff;
            
            if (sendto(serv_sock, client_buff.data(), 255, 0, (sockaddr*)&client, client_size) == SOCKET_ERROR)
            {
                closesocket(serv_sock);
                WSACleanup();
                throw set_error_msg("sendto: ", WSAGetLastError());
            }
        }
        cout << "server off" << endl;
        closesocket(serv_sock);
        WSACleanup();
    }
    catch (string message)
    {
        cout << message << endl;
    }
}

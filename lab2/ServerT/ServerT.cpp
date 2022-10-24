#define _WINSOCK_DEPRECATED_NO_WARNINGS
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
    bool isRunning = true;
    try
    {
        int erStat = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (erStat != 0)
            throw set_error_msg("WSAStartup: ", WSAGetLastError());

        if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        {
            WSACleanup();
            throw set_error_msg("socket: ", WSAGetLastError());
        }
        if (bind(serv_sock, (SOCKADDR*)&serv, sizeof(serv)) == SOCKET_ERROR)
        {
            closesocket(serv_sock);
            WSACleanup();
            throw set_error_msg("bind: ", WSAGetLastError());
        }
        if (listen(serv_sock, SOMAXCONN) == SOCKET_ERROR)
        {
            closesocket(serv_sock);
            WSACleanup();
            throw set_error_msg("listen: ", WSAGetLastError());
        }
        cout << "Server started" << endl;
        SOCKADDR_IN client;
        SOCKET client_sock;
        ZeroMemory(&client, sizeof(client));
        int client_size = sizeof(client);
        std::vector<char> serv_buff(255), client_buff(255);
        int packet_size;
        while (true)
        {
            if ((client_sock = accept(serv_sock, (sockaddr*)&client, &client_size)) == INVALID_SOCKET)
            {
                closesocket(serv_sock);
                WSACleanup();
                throw set_error_msg("accept: ", WSAGetLastError());
            }
            cout << "         Client connect: " << endl;
            cout << "ADDRES client :          " << inet_ntoa(client.sin_addr) << " : " << htons(client.sin_port) << endl
                << endl << endl << endl << endl;
            send(client_sock, to_string(++count).c_str(), 255, 0);

            while (true)
            {
                packet_size = recv(client_sock, serv_buff.data(), 255, 0);
                if (packet_size == SOCKET_ERROR)
                {
                    closesocket(client_sock);
                    closesocket(serv_sock);
                    WSACleanup();
                    throw set_error_msg("receive message from client error: ", WSAGetLastError());
                }
                if (strcmp(serv_buff.data(), "") == 0)
                {
                    isRunning = false;
                    break;
                }
                if (strcmp(serv_buff.data(), "z") == 0)
                {
                    break;
                }
                cout << "Client: " << serv_buff.data() << endl;
                client_buff = serv_buff;
                packet_size = send(client_sock, serv_buff.data(), 255, 0);
                if (packet_size == SOCKET_ERROR)
                {
                    closesocket(client_sock);
                    closesocket(serv_sock);
                    WSACleanup();
                    throw set_error_msg("send message to client error: ", WSAGetLastError());
                }
            }
            if (!isRunning)
                break;
            cout << "client off" << endl;
            closesocket(client_sock);
          
        }
        closesocket(serv_sock);
        closesocket(client_sock);
        WSACleanup();
        cout << "server off" << endl;
    }
    catch (string error_msgText)
    {
        std::cout << "Error: " << error_msgText << std::endl;
    }
}

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#include <ctime>
#include "WSAErrors.h"
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[])
{
    int id;
    int count;
    WSADATA wsa_data;
    SOCKET client_sock;
    SOCKADDR_IN server;
    int server_size = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(2000);
    clock_t start, end;
    setlocale(LC_ALL, "Russian");
    try
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
            throw set_error_msg("WSAStartup: ", WSAGetLastError());

        if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
        {
            WSACleanup();
            throw set_error_msg("socket: ", WSAGetLastError());
        }
        
        in_addr ip_to_num{};
        if (inet_pton(AF_INET, "127.0.0.1", &ip_to_num) <= 0)
            throw set_error_msg("inet_pton: ", WSAGetLastError());
        server.sin_addr = ip_to_num;
        sendto(client_sock, "Hello", 6, 0, (sockaddr*)&server, server_size);
        std::vector<char> serv_buff(255), client_buff(255);
        recv(client_sock, serv_buff.data(), 255, 0);
        id = atoi(serv_buff.data());
        cout << "Your id is " << id << endl;
        serv_buff.clear();
        while (true)
        {
            std::cout << "Your (client) message: ";

            fgets(client_buff.data(), 255, stdin);
            if (client_buff[0] == 'q')
            {
                if (sendto(client_sock, client_buff.data(), 255, 0, (sockaddr*)&server, server_size) == SOCKET_ERROR)
                {
                    closesocket(client_sock);
                    WSACleanup();
                    throw set_error_msg("sendto: ", WSAGetLastError());
                }
                break;
            }
            count = atoi(client_buff.data());
            memset(client_buff.data(), 0, 255);
            start = clock();
            for (int i = 0; i < count; i++)
            {
                if (sendto(client_sock,
                           ("hello from client(id=" + to_string(id) + ")" + "count=" + to_string(i)).c_str(), 255,
                           0, (sockaddr*)&server, server_size) == SOCKET_ERROR)
                    throw set_error_msg("send message error: ", WSAGetLastError());

                if (recvfrom(client_sock, serv_buff.data(), 255, 0, (sockaddr*)&server, &server_size) == SOCKET_ERROR)
                    throw set_error_msg("recv message error: ", WSAGetLastError());

                cout << "Server: " << serv_buff.data() << endl;
                memset(serv_buff.data(), 0, 255);
            }
            end = clock();
            cout << "Time for send and recv: " << ((double)(end - start) / CLK_TCK) << " c" << endl;
        }
    }
    catch (string message)
    {
        cout << message << endl;
    }
}

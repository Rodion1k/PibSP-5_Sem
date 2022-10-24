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
    server.sin_family = AF_INET;
    server.sin_port = htons(2000);
    clock_t start, end;

    setlocale(LC_ALL, "Russian");
    try
    {
        int erStat = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (erStat != 0)
            throw set_error_msg("WSAStartup: ", WSAGetLastError());

        if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        {
            WSACleanup();
            throw set_error_msg("socket: ", WSAGetLastError());
        }
        //192.168.0.104
        in_addr ip_to_num{};
        erStat = inet_pton(AF_INET, "127.0.0.1", &ip_to_num);
        server.sin_addr = ip_to_num;

        if (erStat <= 0)
            throw set_error_msg("ip: ", WSAGetLastError());

        if ((erStat = connect(client_sock, (sockaddr*)(&server), sizeof(server))) != 0)
        {
            closesocket(client_sock);
            WSACleanup();
            throw set_error_msg("connect: ", WSAGetLastError());
        }

        std::cout << "Connection established SUCCESSFULLY. Ready to send a message to Server" << std::endl;
        std::vector<char> serv_buff(255), client_buff(255),std(255);
        id = 0;
        recv(client_sock, serv_buff.data(), 255, 0); // для теста с севой 
        id = atoi(serv_buff.data());
        cout << "Your id is " << id << endl;
        serv_buff.clear();
        while (true)
        {
            std::cout << "Your (client) message: ";

            fgets(client_buff.data(), 255, stdin);
            if (client_buff[0] == 'q')
            {
                send(client_sock, "", 1, 0);
                break;
            }
            if(client_buff[0]=='z')
            {
                send(client_sock, "z", 2, 0);
                break;
            }
           
            count = atoi(client_buff.data());
            memset(client_buff.data(), 0, 255);
            start = clock();
            for (int i = 0; i < count; i++)
            {
                if (send(client_sock,
                         ("hello from client(id=" + to_string(id) + ")" + "count=" + to_string(i)).c_str(), 255,
                         0) == SOCKET_ERROR)
                    throw set_error_msg("send message error: ", WSAGetLastError());
                if (recv(client_sock, serv_buff.data(), 255, 0) == SOCKET_ERROR)
                    throw set_error_msg("recv message error: ", WSAGetLastError());
                cout << "Server: " << serv_buff.data() << endl;
                memset(serv_buff.data(), 0, 255);
            }
            end = clock();
            cout << "Time for send and recv: " << ((double)(end - start) / CLK_TCK) << " c" << endl;
        }
    }
    catch (std::string error_msgText)
    {
        std::cout << "Error: " << error_msgText << std::endl;
        system("pause");
    }
}

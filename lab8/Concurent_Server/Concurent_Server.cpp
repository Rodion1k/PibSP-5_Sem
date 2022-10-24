#include <iostream>

#include "WSAErrors.h"
#include <stdio.h>
#include <tchar.h>
#include <winsock.h>
#include <string>
#include <list>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")
#define AS_SQ 10

using namespace std;

SOCKET server_socket;
int server_port;
char named_pipe_name;


//server  info
volatile long connectionCount = 0;
volatile long sayNoCount = 0;
volatile long successConnections = 0;
volatile long currentActiveConnections = 0;


enum TalkersCommand
{
    START,
    STOP,
    EXIT,
    STATISTICS,
    WAIT,
    SHUTDOWN,
    GETCOMMAND,
    LOAD_LIB,
    UNLOAD_LIB,
};

CRITICAL_SECTION scListContact;

HANDLE h_accept_server, h_dispatch_server;

DWORD WINAPI accept_server(LPVOID lpParam);
DWORD WINAPI dispatch_server(LPVOID lpParam);


void command_cycle(TalkersCommand& cmd);
bool accept_cycle(int sq);
void open_socket();
void close_socket();


struct Contact
{
    enum TE
    {
        EMPTY,
        ACCEPT,
        CONTACT,
    } type;

    enum ST
    {
        WORK,
        ABORT,
        TIMEOUT,
        FINISH,
    } sthread;

    SOCKET socket;
    SOCKADDR_IN sockaddr_in;
    int len_sockaddr_in;
    HANDLE hthread;
    HANDLE htimer;
    char message[50];
    char serv_name[15];

    Contact(TE t = EMPTY, const char* name_serv = "")
    {
        memset(&sockaddr_in, 0, sizeof(SOCKADDR_IN));
        len_sockaddr_in = sizeof(SOCKADDR_IN);
        type = t;
        strcpy_s(serv_name, name_serv);
        message[0] = 0x00;
    }

    void SetST(ST sth, const char* m = "")
    {
        sthread = sth;
        strcpy_s(message, m);
    }
};

typedef list<Contact> ListContact;

ListContact contacts;


void init_server(int argc, char* argv[])
{
    switch (argc)
    {
    case 2:
        server_port = atoi(argv[1]);
        break;
    case 3:
        server_port = atoi(argv[1]);
    // что-то
        break;
    case 4:
        server_port = atoi(argv[1]);
    // что-то
        break;
    default:
        server_port = 2000;
        break;
    }
}

int main(int argc, char* argv[])
{
    init_server(argc, argv);
    cout << "server_port = " << server_port << endl;
    InitializeCriticalSection(&scListContact);
    volatile TalkersCommand cmd = START;
    h_accept_server = CreateThread(NULL,NULL, accept_server, (LPVOID)&cmd,NULL,NULL);
    h_dispatch_server = CreateThread(NULL,NULL, dispatch_server, (LPVOID)&cmd,NULL,NULL);
    WaitForSingleObject(h_dispatch_server, INFINITE);
    CloseHandle(h_dispatch_server);
    WaitForSingleObject(h_accept_server, INFINITE);
    CloseHandle(h_accept_server);
}


DWORD WINAPI accept_server(LPVOID lpParam)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
        throw set_error_msg("WSAStartup failed with error: ", WSAGetLastError());
    command_cycle(*static_cast<TalkersCommand*>(lpParam));
    if (WSACleanup() == SOCKET_ERROR)
        throw set_error_msg("WSACleanup failed with error: ", WSAGetLastError());
    return 0;
}

DWORD WINAPI dispatch_server(LPVOID lpParam)
{
    TalkersCommand cmd = *static_cast<TalkersCommand*>(lpParam);
    while (cmd != EXIT)
    {
        for (auto i = contacts.begin(); i != contacts.end(); i++)
        {
            if (i->type == Contact::ACCEPT)
            {
                char message[10];
                recv(i->socket, message, sizeof(message), 0);
                strcpy_s(i->message, message);
            }
            i->type = Contact::CONTACT;
            i->hthread = h_accept_server;
            //i->sthread=
        }
    }
    return 0;
}

void command_cycle(TalkersCommand& cmd)
{
    int squirt = 0;
    while (cmd != EXIT)
    {
        switch (cmd)
        {
        case START:
            {
                cmd = GETCOMMAND;
                cout << "START command" << endl;
                squirt = AS_SQ;
                open_socket();
                break;
            }
        case STOP:
            {
                cmd = GETCOMMAND;
                squirt = 0;
                cout << "STOP command" << endl;
                close_socket();
                break;
            }
        default:
            break;
        }
        if (accept_cycle(squirt))
        {
            cmd = GETCOMMAND;
        }
    }
}

bool accept_cycle(int sq)
{
    bool rc = false;
    Contact contact(Contact::ACCEPT, "EchoServer");

    while (sq-- > 0 && !rc)
    {
        if ((contact.socket = accept(server_socket, (sockaddr*)&contact.sockaddr_in, &contact.len_sockaddr_in) ==
            INVALID_SOCKET))
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                throw set_error_msg("accept() failed with error", WSAGetLastError());
        }
        else
        {
            rc = true;
            EnterCriticalSection(&scListContact);
            contacts.push_front(contact);
            LeaveCriticalSection(&scListContact);
            cout << "Client connected" << endl;
        }
    }
    return rc;
}

void open_socket()
{
    SOCKADDR_IN serv;
    u_long nonblk = 1;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
        throw set_error_msg("socket() failed with error", WSAGetLastError());

    serv.sin_family = AF_INET;
    serv.sin_port = htons(server_port);
    serv.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
        throw set_error_msg("bind() failed with error", WSAGetLastError());
    if (listen(server_socket,SOMAXCONN) == SOCKET_ERROR)
        throw set_error_msg("listen() failed with error", WSAGetLastError());
    if (ioctlsocket(server_socket, FIONBIO, &nonblk) == SOCKET_ERROR)
        throw set_error_msg("ioctlsocket() failed with error", WSAGetLastError());
}

void close_socket()
{
    if (closesocket(server_socket) == SOCKET_ERROR)
        throw set_error_msg("closesocket() failed with error", WSAGetLastError());
}

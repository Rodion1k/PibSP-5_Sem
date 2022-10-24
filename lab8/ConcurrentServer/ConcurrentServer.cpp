#include <list>
#include "WinSock2.h"
#include <string>
#include "WSAErrors.h"
#include <list>
#include <time.h>
#include <iostream>
#include <string>
#pragma comment(lib, "WS2_32.lib")

#define IP_SERVER "127.0.0.1"
#define AS_SQ 10
using namespace std;
int server_port;
SOCKET server_socket;

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

volatile TalkersCommand previous_command = GETCOMMAND;
HANDLE h_accept_server, hConsolePipe, hGarbageCleaner, hDispatchServer, hResponseServer;
HANDLE hClientConnectedEvent = CreateEvent(NULL, FALSE, FALSE, L"ClientConnected");
DWORD WINAPI accept_server(LPVOID pPrm);
DWORD WINAPI ConsolePipe(LPVOID pPrm);
DWORD WINAPI GarbageCleaner(LPVOID pPrm);
DWORD WINAPI DispatchServer(LPVOID pPrm);
DWORD WINAPI ResponseServer(LPVOID pPrm);

CRITICAL_SECTION scListContact;

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

typedef std::list<Contact> ListContact;

ListContact contacts;

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

bool AcceptCycle(int sq)
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

void CommandsCycle(TalkersCommand& cmd)
{
    int sq = 0;
    while (cmd != EXIT)
    {
        switch (cmd)
        {
        case START:
            {
                cmd = GETCOMMAND;
                if (previous_command != START)
                {
                    sq = AS_SQ;
                    cout << "Start command" << endl;
                    open_socket();
                    previous_command = START;
                }
                else cout << "start already done" << endl;
                break;
            }
        case STOP:
            {
                sq = 0;
                cout << "Stop command" << endl;
                close_socket();
                break;
            }
        case EXIT:
            {
                break;
            }
        case STATISTICS:
            {
                break;
            }
        case WAIT:
            {
                break;
            }
        case SHUTDOWN:
            {
                break;
            }
        case GETCOMMAND:
            {
                break;
            }

        default:
            cout << "Unknown command" << endl;
            break;
        }
        if (cmd != STOP)
        {
            if (AcceptCycle(sq))
            {
                cmd = GETCOMMAND;
                //SetEvent(hClientConnectedEvent);
            }
            //else SleepEX(0,TRUE);
        }
    }
}

void CALLBACK ASWTimer(LPVOID prm)
{
    
}


DWORD accept_server(LPVOID pPrm)
{
    DWORD rc = 0;
    WSADATA wsaData;
    try
    {
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            throw set_error_msg("WSAStartup failed with error: ", WSAGetLastError());
        CommandsCycle(*((TalkersCommand*)pPrm));
        if (WSACleanup() == SOCKET_ERROR)
            throw set_error_msg("WSACleanup failed with error: ", WSAGetLastError());
    }
    catch (string err)
    {
        cout << err << endl;
    }
    cout << "shutdown acceptServer" << endl;
    ExitThread(rc);
}

DWORD DispatchServer(LPVOID pPrm)
{
    DWORD rc = 0;
    TalkersCommand command = *(TalkersCommand*)pPrm;

    while (true)
    {
        for (auto i = contacts.begin(); i != contacts.end();)
        {
            if (i->type == Contact::ACCEPT)
            {
                i->type = Contact::CONTACT;
                i->hthread = h_accept_server;
                //i->sthread
                i->htimer = CreateWaitableTimer(0, false, 0);
                LARGE_INTEGER timeout;
                int seconds = 60;
                timeout.QuadPart = -(10000000 * seconds);
                SetWaitableTimer(i->htimer, &timeout, 0, PTIMERAPCROUTINE(ASWTimer), (LPVOID)&(*i), false);
                SleepEx(0, true);
            }
        }
    }
}


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
    if (h_accept_server == NULL)
        throw set_error_msg("CreateThread failed with error: ", GetLastError());

    hDispatchServer = CreateThread(NULL,NULL, DispatchServer, (LPVOID)&cmd,NULL,NULL);
    if (hDispatchServer == NULL)
        throw set_error_msg("CreateThread failed with error: ", GetLastError());

    WaitForSingleObject(hDispatchServer, INFINITE);
    CloseHandle(hDispatchServer);
    WaitForSingleObject(h_accept_server, INFINITE);
    CloseHandle(h_accept_server);
}

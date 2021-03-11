#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_PORT "7777"
bool readyToReceive(SOCKET* sock, int interval = 1)
{
    fd_set fd_read;
    FD_ZERO(&fd_read);
    FD_SET(*sock, &fd_read);
    fd_set fd_write;
    FD_ZERO(&fd_write);
    FD_SET(*sock, &fd_write);


    timeval tv;
    tv.tv_sec = interval;
    tv.tv_usec = 0;

    return (select(*sock + 1, &fd_read, &fd_write, 0, &tv) == 1);
};

void send_message(SOCKET* sock) {
    while(true) {
        char str[255];
        scanf("%s", str);
        int iResult = send(*sock, str, (int) strlen(str), 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(*sock);
        }
    }
}

void receive_message(SOCKET* sock){
    while(true) {
        char recvbuf[255]={0};
        int recvbuflen = 255;
        int iResult = recv(*sock, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
            printf("%s", recvbuf);
        else if ( iResult == 0 )
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    }
}

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    addrinfo *result = NULL;
    addrinfo hints;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    SOCKET ConnectSocket = socket(result->ai_family, result->ai_socktype,
                                  result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Connect to server.
    iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    if(readyToReceive(&ConnectSocket)) {
        std::thread read(receive_message, &ConnectSocket);
        std::thread write(send_message, &ConnectSocket);
        read.join();
        write.join();

    }

    return 0;
}

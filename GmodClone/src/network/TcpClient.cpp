#ifdef _WIN32
#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#include <string>

#include "tcpclient.hpp"

#ifdef _WIN32
// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
#endif

bool TcpClient::hasInit = false;

TcpClient::TcpClient(SOCKET socket)
{
    if (!TcpClient::hasInit)
    {
        #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(0x0101, &wsaData);
        #endif
        TcpClient::hasInit = true;
    }

    sd = socket;
}

TcpClient::TcpClient(char* ip, int port, int timeoutSec)
{
    if (!TcpClient::hasInit)
    {
        #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(0x0101, &wsaData);
        #endif
        TcpClient::hasInit = true;
    }

    attemptConnection(ip, port, timeoutSec);
}

TcpClient::TcpClient(const char* ip, int port, int timeoutSec)
{
    if (!TcpClient::hasInit)
    {
        #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(0x0101, &wsaData);
        #endif
        TcpClient::hasInit = true;
    }

    attemptConnection((char*)ip, port, timeoutSec);
}

TcpClient::~TcpClient()
{
    closeConnection();
}

void TcpClient::attemptConnection(char* ip, int port, int timeoutSec)
{
    if (sd != INVALID_SOCKET)
    {
        return;
    }

    // Convert the server name to ip address
    struct addrinfo* result = nullptr;
    struct addrinfo  hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char dnsIp[32] = {0};

    int dwRetval = getaddrinfo(ip, std::to_string(port).c_str(), &hints, &result);
    if (dwRetval != 0)
    {
        printf("error when get addr info\n");
    }
    else
    {
        struct addrinfo* ptr = nullptr;
        for (ptr=result; ptr != nullptr; ptr=ptr->ai_next)
        {
            struct sockaddr_in* addr = (struct sockaddr_in *)ptr->ai_addr;

            inet_ntop(AF_INET, &(addr->sin_addr), dnsIp, 32);
        }
    }

    // Construct server address struct.
    struct sockaddr_in sad; // structure to hold server's address

    memset((char*)&sad, 0, sizeof(sad)); // clear sockaddr structure
    sad.sin_family = AF_INET; // set family to Internet

    #ifdef _WIN32
    InetPton(AF_INET, (PCSTR)(dnsIp), &sad.sin_addr.s_addr);
    #else
    inet_pton(AF_INET, dnsIp, &sad.sin_addr.s_addr);
    #endif

    sad.sin_port = htons((u_short)port);

    // Map TCP transport protocol name to protocol number
    struct protoent* ptrp = getprotobyname("tcp");
    if (ptrp == nullptr)
    {
        return;
    }

    // Create a socket
    sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd == INVALID_SOCKET)
    {
        return;
    }

    // Eliminate "Address already in use" error message.
    const char flag = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(char)) == -1)
    {

    }

    // If the timeout is < 0, block until connection succeeds or error
    if (timeoutSec < 0)
    {
        if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0)
        {
            closeConnection();
            return;
        }

        return;
    }

    // Set the socket in non-blocking

    #ifdef _WIN32
    unsigned long iMode = 1;
    int iResult = ioctlsocket(sd, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
    {
        closeConnection();
        return;
    }
    #else
    unsigned long iMode = 1;
    int iResult = ioctl(sd, FIONBIO, &iMode);
    if (iResult < 0)
    {
        closeConnection();
        return;
    }
    #endif

    connect(sd, (struct sockaddr*)&sad, sizeof(sad));

    // Put socket back into blocking mode

    #ifdef _WIN32
    iMode = 0;
    iResult = ioctlsocket(sd, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
    {
        printf("Failed to put socket back into blocking mode!\n");
    }
    #else
    iMode = 0;
    iResult = ioctl(sd, FIONBIO, &iMode);
    if (iResult < 0)
    {
        printf("Failed to put socket back into blocking mode!\n");
    }
    #endif

    fd_set setWrite;
    FD_ZERO(&setWrite);
    FD_SET(sd, &setWrite);

    timeval timeout;
    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = 0;

    // Wait until socket can be written to, or until timeout
    select(0, nullptr, &setWrite, nullptr, &timeout);
    if (FD_ISSET(sd, &setWrite))
    {

    }
    else
    {
        closeConnection();
    }
}

bool TcpClient::isOpen()
{
    return (sd != INVALID_SOCKET);
}

int TcpClient::write(const void* bytes, int numBytesToSend, int timeoutSec)
{
    return write((void*)bytes, numBytesToSend, timeoutSec);
}

int TcpClient::write(void* bytes, int numBytesToSend, int timeoutSec)
{
    if (sd == INVALID_SOCKET ||
        numBytesToSend <= 0  ||
        bytes == nullptr)
    {
        return -1;
    }

    int currentBytesSent = 0;
    while (currentBytesSent < numBytesToSend)
    {
        if (timeoutSec > 0)
        {
            fd_set setWrite;
            struct timeval timeout;
            FD_ZERO(&setWrite);
            FD_SET(sd, &setWrite);

            timeout.tv_sec = timeoutSec;
            timeout.tv_usec = 0;
            int selectval = select((int)sd + 1, NULL, &setWrite, NULL, &timeout);

            if (selectval == -1)
            {
                closeConnection();
                return currentBytesSent;
            }
            else if (selectval == 0) // Timeout happened
            {
                closeConnection();
                return currentBytesSent;
            }
        }

        // Send response
        int flags = 0; //| MSG_DONTWAIT; //not waiting (MSG_DONTWAIT) causes the return to happen early and not send all the bytes much more often.

        int numBytesSent = send(sd, &((char*)bytes)[currentBytesSent], numBytesToSend - currentBytesSent, flags);

        if (numBytesSent == -1)
        {
            closeConnection();
            return currentBytesSent;
        }
        else if (numBytesSent == 0)
        {
            closeConnection();
            return currentBytesSent;
        }

        currentBytesSent += numBytesSent;
    }

    return currentBytesSent;
}

int TcpClient::read(void* buffer, int numBytesToRead, int timeoutSec)
{
    if (sd == INVALID_SOCKET ||
        numBytesToRead <= 0  ||
        buffer == nullptr)
    {
        return -3;
    }

    int currentBytesRead = 0;
    while (currentBytesRead < numBytesToRead)
    {
        if (timeoutSec > 0)
        {
            fd_set setRead;
            struct timeval timeout;
            FD_ZERO(&setRead);
            FD_SET(sd, &setRead);
            timeout.tv_sec = timeoutSec;
            timeout.tv_usec = 0;
            int selectval = select((int)sd + 1, &setRead, NULL, NULL, &timeout);
            if (selectval == -1)
            {
                closeConnection();
                return -1;
            }
            else if (selectval == 0) // Timeout happened
            {
                return 0;
            }
        }

        int flags = 0;

        int numBytesRead = recv(sd, &((char*)buffer)[currentBytesRead], numBytesToRead - currentBytesRead, flags);

        if (numBytesRead == -1)
        {
            closeConnection();
            return -1;
        }

        if (numBytesRead == 0)
        {
            closeConnection();
            return -2;
        }

        currentBytesRead += numBytesRead;
    }

    return currentBytesRead;
}

#ifdef _WIN32
void TcpClient::closeConnection()
{
    if (sd != INVALID_SOCKET)
    {
        closesocket(sd);
        sd = INVALID_SOCKET;
    }
}
#else
void TcpClient::closeConnection()
{
    if (sd != INVALID_SOCKET)
    {
        close(sd);
        sd = INVALID_SOCKET;
    }
}
#endif

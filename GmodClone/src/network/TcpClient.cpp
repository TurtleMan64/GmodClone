#ifdef _WIN32
#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>
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
        printf("Error when calling getaddrinfo.\n");
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
        printf("Could not eliminate the 'address already in use' error message\n");
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

    #ifdef _WIN32
    // Set the socket in non-blocking
    unsigned long iMode = 1;
    int iResult = ioctlsocket(sd, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
    {
        closeConnection();
        return;
    }

    if (connect(sd, (struct sockaddr*)&sad, sizeof(sad)) < 0)
    {
        // This is expected, connect should be in progress.
    }
    #else
    if (connectWithTimeout(sd, (struct sockaddr*)&sad, sizeof(sad), timeoutSec*1000) < 0)
    {
        closeConnection();
        return;
    }
    #endif

    // Put socket back into blocking mode

    #ifdef _WIN32
    iMode = 0;
    iResult = ioctlsocket(sd, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
    {
        printf("Failed to put socket back into blocking mode!\n");
    }

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
    #endif
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
        printf("Closing the connection\n");
        close(sd);
        sd = INVALID_SOCKET;
    }
}
#endif

#ifdef _WIN32
int TcpClient::connectWithTimeout(SOCKET, const struct sockaddr*, socklen_t, unsigned int)
{
    return -1;
}
#else
//https://stackoverflow.com/questions/2597608/c-socket-connection-timeout
int TcpClient::connectWithTimeout(SOCKET sockfd, const struct sockaddr* addr, socklen_t addrlen, unsigned int timeout_ms)
{
    int rc = 0;

    // Set O_NONBLOCK
    int sockfd_flags_before;
    if ((sockfd_flags_before=fcntl(sockfd,F_GETFL,0)<0))
    {
        return -1;
    }

    if (fcntl(sockfd,F_SETFL,sockfd_flags_before | O_NONBLOCK)<0)
    {
        return -1;
    }

    // Start connecting (asynchronously)
    do
    {
        if (connect(sockfd, addr, addrlen) < 0)
        {
            // Did connect return an error? If so, we'll fail.
            if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS))
            {
                rc = -1;
            }
            // Otherwise, we'll wait for it to complete.
            else
            {
                // Set a deadline timestamp 'timeout' ms from now (needed b/c poll can be interrupted)
                struct timespec now;
                if (clock_gettime(CLOCK_MONOTONIC, &now)<0)
                {
                    rc=-1; break;
                }

                struct timespec deadline = { .tv_sec = now.tv_sec,
                                             .tv_nsec = now.tv_nsec + timeout_ms*1000000l};
                // Wait for the connection to complete.
                do
                {
                    // Calculate how long until the deadline
                    if (clock_gettime(CLOCK_MONOTONIC, &now)<0)
                    {
                        rc=-1; break;
                    }

                    int ms_until_deadline = (int)(  (deadline.tv_sec  - now.tv_sec)*1000l
                                                  + (deadline.tv_nsec - now.tv_nsec)/1000000l);
                    if (ms_until_deadline<0)
                    {
                        rc=0; break;
                    }

                    // Wait for connect to complete (or for the timeout deadline)
                    struct pollfd pfds[] = { { .fd = sockfd, .events = POLLOUT, .revents = 0 } };
                    rc = poll(pfds, 1, ms_until_deadline);
                    // If poll 'succeeded', make sure it *really* succeeded
                    if (rc>0)
                    {
                        int error = 0; socklen_t len = sizeof(error);
                        int retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
                        if (retval==0)
                        {
                            errno = error;
                        }

                        if (error!=0)
                        {
                            rc=-1;
                        }
                    }
                }
                // If poll was interrupted, try again.
                while (rc==-1 && errno==EINTR);

                // Did poll timeout? If so, fail.
                if (rc==0)
                {
                    errno = ETIMEDOUT;
                    rc=-1;
                }
            }
        }
    }
    while(0);

    // Restore original O_NONBLOCK state
    if (fcntl(sockfd,F_SETFL,sockfd_flags_before)<0)
    {
        return -1;
    }

    // Success
    return rc;
}
#endif

#pragma once

#ifdef WIN32
#include <winsock2.h>
typedef int socklen_t;
static int ioctl(SOCKET s, long cmd, u_long* argp)
{
	return ioctlsocket(s, cmd, argp);
}
static int close(SOCKET s)
{
	return closesocket(s);
}
#elif ARM9
typedef int socklen_t;
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
static int close(int s)
{
	return closesocket(s);
}
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#endif

struct UdpSocketInternal
{
#ifdef WIN32
	SOCKET socket;
#else
	int socket;
#endif
};

#define INTERNAL ((UdpSocketInternal*)internal)

#ifdef _WINDOWS
#define NET_ERROR(msg) {char err[1024]; _snprintf(err, 1024, "%s failed error %i (%s:%i)\n", msg, ::WSAGetLastError(), __FILE__, __LINE__); MessageBox(NULL, err, NULL, MB_OK | MB_ICONERROR);}
#elif WIN32
#define NET_ERROR(msg) printf("%s failed error %i (%s:%i)\n", msg, ::WSAGetLastError(), __FILE__, __LINE__)
#else
#define NET_ERROR(msg) printf("%s failed error %i (%s:%i)\n", msg, errno, __FILE__, __LINE__)
#endif



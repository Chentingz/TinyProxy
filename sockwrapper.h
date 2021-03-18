/*
*	winsock的包裹函数
*
*/
#include <winsock2.h>


SOCKET Socket(int af, int type, int protocol);

int Bind(SOCKET s, const struct sockaddr FAR *name, int namelen);

int Listen(SOCKET s, int backlog);

int Connect(SOCKET s, const struct sockaddr FAR *name, int namelen);

SOCKET Accept(SOCKET s, struct sockaddr FAR *addr, int FAR *addrlen);

int Recv(SOCKET s, char FAR *buf, int len, int flags);

int Send(SOCKET s, char FAR *buf, int len, int flags);
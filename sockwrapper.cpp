/*
*	winsockµÄ°ü¹üº¯Êý
*
*/



#include <winsock2.h>
#include <stdio.h>

SOCKET Socket(int af, int type, int protocol)
{
	SOCKET sock = socket(af, type, protocol);
	
	if(sock == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		exit(1);
	}
	
	return sock;
}

int Bind(SOCKET s, const struct sockaddr FAR *name, int namelen)
{
	int iResult = bind(s, name, namelen);
	
	if(iResult == SOCKET_ERROR)
	{
		printf("Error at bind(): %d\n", WSAGetLastError());
		closesocket(s);
		WSACleanup();
		exit(1);
	}
	return iResult;
}

int Listen(SOCKET s, int backlog)
{
	int iResult = listen(s, backlog);
	if(iResult != 0)
	{
		printf("Error at listen(): %d\n", WSAGetLastError());
		closesocket(s);
		WSACleanup();
		exit(1);
	}
	
	return iResult;
}

int Connect(SOCKET s, const struct sockaddr FAR *name, int namelen)
{
	int iResult = connect(s, name, namelen);
	if(iResult == SOCKET_ERROR)
	{
		printf("Error at connect(): %d\n", WSAGetLastError());
		closesocket(s);
	//	WSACleanup();
	//	exit(1);
	}
	
	return iResult;
}

SOCKET Accept(SOCKET s, struct sockaddr FAR *addr, int FAR *addrlen)
{
	SOCKET sock = accept(s, addr, addrlen);
	if(sock == INVALID_SOCKET)
	{
		printf("Error at accept(): %ld\n", WSAGetLastError());
		closesocket(s);
	//	WSACleanup();
	//	exit(1);
	}
	
	return sock;
}

int Recv(SOCKET s, char FAR *buf, int len, int flags)
{
	int iBytes = recv(s, buf, len, flags);
	
	if(iBytes ==  SOCKET_ERROR)
	{
		printf("Error at recv(): %ld\n", WSAGetLastError());
		closesocket(s);
	//	WSACleanup();
	//	exit(1);
		return iBytes;
	}
	else if(iBytes == 0)
	{
		printf("Connection closed.\n");
		closesocket(s);
		return iBytes;

	}
	else
	{
		return iBytes;
	}
	
}

int Send(SOCKET s, char FAR *buf, int len, int flags)
{
	int iBytes = send(s, buf, len, flags);
	
	if(iBytes ==  SOCKET_ERROR)
	{
		printf("Error at send(): %ld\n", WSAGetLastError());
		closesocket(s);
	//	WSACleanup();
	//	exit(1);
		return iBytes;
	}
	else if(iBytes == 0)
	{
		printf("Connection closed.\n");
		closesocket(s);
		return iBytes;
	}
	else
	{
		return iBytes;
	}
}

// 02TCP����˿ͻ��˱��ģ��.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <locale.h>
#include <winsock2.h>

DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter);
int main()
{
	//setlocale(LC_ALL, "");
	//0����ʼ�� Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup error,code: %d", iResult);
		return 0;
	}
	//1,�����׽���
	SOCKET hsocket=socket(AF_INET, SOCK_STREAM, 0);
	if (hsocket == INVALID_SOCKET)
	{
		printf("socket error,code: %d", WSAGetLastError());
		return 0;
	}
	sockaddr_in si;
	si.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	si.sin_family = AF_INET;
	si.sin_port = htons(10086);

	//2,���׽���
	bind(hsocket, (SOCKADDR*)&si, sizeof(si));

	//3,��ʼ����
	
	listen(hsocket, 5);
	printf("%d �������������ض˿�%d...\n", htonl(INADDR_ANY), htons(si.sin_port));

	while (true)
	{

	
		//4.�ȴ�����
		printf("�ȴ��ͻ�������...\n");
		
		sockaddr_in clientSockaddr = {0};
		int len = sizeof(clientSockaddr);
		SOCKET hsocketAccept = accept(hsocket, (SOCKADDR*)&clientSockaddr, &len);
		if (hsocketAccept == INVALID_SOCKET)
		{
			printf("accept error,error code : %d \n",WSAGetLastError());
			return 0;
		}

		HANDLE cthread=CreateThread(NULL, 0, ThreadProc, (LPVOID)hsocketAccept, 0, NULL);
		CloseHandle(cthread);
		
	}
	WSACleanup();
    return 1;
}


DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter)
{
	while (true)
	{
		char recvBuf[1024] = { 0 };
		int recvCount = recv((SOCKET)lpParameter, recvBuf, 1024, 0);
		if (recvCount == SOCKET_ERROR)
		{
			printf("recv error,error code : %d \n", WSAGetLastError());
			return 0;
		}
		printf("Bytes recvCount��%d \n", recvCount);
		printf("%s\n", recvBuf);

		if (send((SOCKET)lpParameter, recvBuf, strlen(recvBuf) + 1, 0) == SOCKET_ERROR)
		{
			printf("send error,error code : %d \n", WSAGetLastError());
			return 0;
		}
		if (strcmp(recvBuf, "close")==0)
		{
			break;
		}
	}
	shutdown((SOCKET)lpParameter, SD_BOTH);
	closesocket((SOCKET)lpParameter);
	return 0;
}
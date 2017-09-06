// 02TCP服务端客户端编程模型-client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <winsock2.h>
#pragma comment(lib,"Ws2_32.lib")

int main()
{
	while (true)
	{
	WSADATA wsdata;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsdata);
	if(iResult !=0)
	{
		printf("WSAStartup error,code: %d", iResult);
		return 0;
	}
	SOCKET  hSocketCoont= socket(AF_INET, SOCK_STREAM, 0);
	if (hSocketCoont==INVALID_SOCKET)
	{
		printf("hSocketCoont error,code: %d", WSAGetLastError());
		return 0;
	}
	sockaddr_in si;
	si.sin_addr.S_un.S_addr = inet_addr("172.16.42.133");
	si.sin_family = AF_INET;
	si.sin_port = htons(10086);

	
	int iconSta=connect(hSocketCoont, (sockaddr *)&si, sizeof(sockaddr_in));
	if (iconSta == SOCKET_ERROR)
	{
		printf("connect  error,code: %d", WSAGetLastError());
		//return 0;
	}

	char senBuff[1024] = {0};

	
		printf("输入你要发送的内容  \n");
		scanf_s("%s", senBuff, sizeof(senBuff));
		int sendCout = send(hSocketCoont, senBuff, (int)strlen(senBuff) + 1, 0);
		if (sendCout == SOCKET_ERROR)
		{
			printf("send error,error code : %d \n", WSAGetLastError());
			//return 0;
		}
		//shutdown(hSocketCoont, SD_SEND);
		printf("Bytes Send：%d \n", sendCout);

		char recvBuff[1024] = { 0 };
		int recvCount = recv(hSocketCoont, recvBuff, 1024, 0);
		if (recvCount == SOCKET_ERROR)
		{
			printf("recv error,error code : %d \n", WSAGetLastError());
			//return 0;
		}
		printf("Bytes recvCount：%d \n", recvCount);
		//shutdown(hSocketCoont, SD_RECEIVE);
		printf("FORM SERVICE:\n %s\n", recvBuff);
	
	closesocket(hSocketCoont);
	WSACleanup();


	}
    return 1;

}


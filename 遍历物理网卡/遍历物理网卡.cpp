// 遍历物理网卡.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


#define  HAVE_REMOTE
#include <pcap.h>

#include <WinSock2.h>
#pragma comment(lib,"wpcap.lib")
#pragma  comment(lib,"ws2_32.lib")

char g_mackDump[65535];//64k

struct MACPack
{
	u_char destination[6];
	u_char Source[6];
	u_char Type[2];

};
struct IpPack
{

};

typedef struct stTCP
{
	WORD wSourcePort;
	WORD wDestPort;
	DWORD dwSequenNum;
	DWORD dwAckNum;
	WORD wOffsetReserveFlag;
	WORD wWindow;
	WORD wCheckSum;
	WORD wUrgentPinger;
	DWORD dwOptionPedding;
}stTcp;

void TCPunPack(const struct pcap_pkthdr *myPcap, const u_char *myChar)
{
	printf("============TCP包解析===============\n");
	stTcp* myTcp = (stTcp*)(myChar + 34);
	printf("SourcePort:%d\n", ntohs(myTcp->wSourcePort));
	printf("DestPort:%d\n", ntohs(myTcp->wDestPort));
	printf("序列号:%u\n", ntohl(myTcp->dwSequenNum));
	printf("确认号:%u\n", ntohl(myTcp->dwAckNum));
	printf("====================================\n\n\n\n");
}

typedef struct stUDP
{
	WORD wSourcePort;
	WORD wDestPort;
	WORD wLength;
	WORD wCheckSum;
}stUdp;

void UDPunPack(const struct pcap_pkthdr *myPcap, const u_char *myChar)
{
	printf("============UDP包解析===============\n");
	stUdp* myUdp = (stUdp*)(myChar + 34);
	printf("SourcePort:%d\n", ntohs(myUdp->wSourcePort));
	printf("DestPort:%d\n", ntohs(myUdp->wDestPort));
	printf("长度:%u\n", ntohs(myUdp->wLength));
	printf("检验和:%u\n", ntohs(myUdp->wCheckSum));
	printf("====================================\n\n\n\n");
}
void ipUnPack(const struct pcap_pkthdr *header, const u_char *pkt_data)
{

	printf("============IP包解析===============\n");
	memset(g_mackDump, 0, sizeof(g_mackDump));
	DWORD i;
	DWORD dwIndex = 0;
	//采用直接复制的形式，方便后面进行判断
	memcpy(g_mackDump, pkt_data, header->caplen);
	//6+6+2 = 14  故解析的下标从14开始
	char temp;
	temp = g_mackDump[14];
	char cVersion = (temp & 0xF0) >> 0x4;
	char cLength = temp & 0x0F;
	//4位版本
	printf("Version:%1x[4bit]\n", cVersion);
	//4位首部长度
	printf("HeadLen:%d[4bit]\n", ((int)cLength) * 4);
	//8位服务类型
	printf("TOS:0x%02x[8bit]\n", g_mackDump[15]);
	//16位总长度
	short sTotalLength = ntohs(*((short*)(g_mackDump + 16)));
	printf("PacketLen:%d[16bit]\n", sTotalLength);
	//16位标识
	printf("PacketID:%x%x[16bit]\n", g_mackDump[18], g_mackDump[19]);
	//3位标志
	char flags = (g_mackDump[20] & 0xE0) >> 5;
	printf("Fragment Flag:0x%1x[3bit]\n", flags);
	//13位片偏移
	short nOffset = *((short*)(g_mackDump + 20)) & 0x1F;
	printf("Fragment offset:%dbit\n", nOffset);
	//8位生存时间
	printf("TTL:%d[8bit]\n", g_mackDump[22]);

	//8位协议
	printf("Protocol:%d[8bit]\n", g_mackDump[23]);
	switch (g_mackDump[23])
	{
	case 1:
		printf("-----ICMP\n");
		break;
	case 2:
		printf("-----IGMP\n");
		break;
	case 6:
		printf("-----TCP\n");
		break;
	case 17:
		printf("-----UDP\n");
		break;
	}



	//16位首部校验和
	short checkSum = ntohs(*((short*)(g_mackDump + 24)));
	printf("CheckSum:0x%x[16bit]\n", checkSum);
	//32位源ip地址
	struct in_addr addr = { 0 };
	addr.S_un.S_addr = *((long*)(g_mackDump + 26));
	printf("SourceIP:%s\n", inet_ntoa(addr));
	//32位目的ip地址
	memset(&addr, 0, sizeof(addr));
	addr.S_un.S_addr = *((long*)(g_mackDump + 30));
	printf("DestIP:%s\n", inet_ntoa(addr));

	printf("===============================\n");
	switch (g_mackDump[23])
	{
	case 6:
		TCPunPack(header, pkt_data);
		break;
	case 17:
		UDPunPack(header, pkt_data);
		break;
	}
}

void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	/*
	*pkt_data==是二进制数
	2进制转16进制，因为计算机是以16位禁止数显示
	时刻注意内存安全问题

	1,首先取第一个字节,一个字节是8个2进制位，故而一个字节由2个十六进制数组成
	2,取字节的低4位进行转换，然后取高四位转换。然后存到一个新的字节里面。由两两字节显示以前的一个字节里面的数字
	3,打印输出
	*/
	memset(g_mackDump, 0, sizeof(g_mackDump));
	DWORD indexTem = 0;
	DWORD i = 0;
	for (; i < header->caplen; i++)
	{
		u_char LowBit = *(pkt_data+i) & 0x0f; //和1111与得到本身的值
		u_char highBit = (*(pkt_data + i) & 0xf0) >> 4;

		if (highBit >= 0 && highBit <= 9)
		{
			g_mackDump[indexTem++] = highBit + 0x30;
		}
		else
		{
			g_mackDump[indexTem++] = highBit - 10 + 0x41;
		}
		if (LowBit >= 0 && LowBit <= 9)
		{
			g_mackDump[indexTem++] = LowBit + 0x30;
		}
		else
		{
			g_mackDump[indexTem++] = LowBit - 10 + 0x41;
		}
	}

	printf("目标地址（Destination）：");
	for ( i = 0; i < 12; i+=2)
	{
		printf("%c%c", g_mackDump[i], g_mackDump[i + 1]);
		if (i != 10)
		{
			printf(":");
		}
		else
		{
			printf("\n");
		}
	}

	printf("源地址（Source）：");
	for (i=12; i < 24; i += 2)
	{
		printf("%c%c", g_mackDump[i], g_mackDump[i + 1]);
		if (i != 22)
		{
			printf(":");
		}
		else
		{
			printf("\n");
		}
	}
	printf("类型（type）：");
	char sc[5] = { 0 };
	memcpy(sc, &(g_mackDump[24]), 4);
	int* sh = (int*)sc;
	switch (*sh)
	{
	case 0x30303830://
	{
		printf("ipv4 \n");
		ipUnPack(header, pkt_data);
		break;
	}
	case 0x36303830://
	{
		printf("ARP \n");
		getchar();
		break;
	}
	default:
		break;
	}
	printf("Len：%d \n",header->caplen);

	printf("\n\n================================\n\n");
	//getchar();

}

int main()
{
	pcap_if_t * pi;
	char errorBuff[1024] = { 0 };
	int p =pcap_findalldevs(&pi, errorBuff);
	if (p == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errorBuff);
		return 0;
	}
	printf("正在获取本地网卡列表 \n");

	pcap_if_t* newpft = pi;
	do 
	{
		printf("Name：%s;ds:%s \n", pi->name, pi->description);


	} while (pi=pi->next);
	int s = 1;
	printf("请输入监听的网卡序号 \n");
	//scanf("%d", &s);
	for (int i=0;i<s;i++)
	{
		newpft = newpft->next;
	}
	char errorOpenBuff[1024] = { 0 };
	pcap_t *adhandle =pcap_open(newpft->name, 65535, 0, 0, NULL, errorOpenBuff);
	if (adhandle == NULL)
	{
		fprintf(stderr, "%s, %s is not supported by WinPcap.error code:\n", errorOpenBuff,newpft->name);
		/* 释放设备列表 */
		pcap_freealldevs(pi);
		return -1;

	}
	pcap_freealldevs(pi);

	pcap_loop(adhandle, 0,packet_handler,NULL);
    return 0;
}

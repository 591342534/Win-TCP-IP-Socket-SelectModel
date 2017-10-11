// ������������.cpp : �������̨Ӧ�ó������ڵ㡣
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
	printf("============TCP������===============\n");
	stTcp* myTcp = (stTcp*)(myChar + 34);
	printf("SourcePort:%d\n", ntohs(myTcp->wSourcePort));
	printf("DestPort:%d\n", ntohs(myTcp->wDestPort));
	printf("���к�:%u\n", ntohl(myTcp->dwSequenNum));
	printf("ȷ�Ϻ�:%u\n", ntohl(myTcp->dwAckNum));
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
	printf("============UDP������===============\n");
	stUdp* myUdp = (stUdp*)(myChar + 34);
	printf("SourcePort:%d\n", ntohs(myUdp->wSourcePort));
	printf("DestPort:%d\n", ntohs(myUdp->wDestPort));
	printf("����:%u\n", ntohs(myUdp->wLength));
	printf("�����:%u\n", ntohs(myUdp->wCheckSum));
	printf("====================================\n\n\n\n");
}
void ipUnPack(const struct pcap_pkthdr *header, const u_char *pkt_data)
{

	printf("============IP������===============\n");
	memset(g_mackDump, 0, sizeof(g_mackDump));
	DWORD i;
	DWORD dwIndex = 0;
	//����ֱ�Ӹ��Ƶ���ʽ�������������ж�
	memcpy(g_mackDump, pkt_data, header->caplen);
	//6+6+2 = 14  �ʽ������±��14��ʼ
	char temp;
	temp = g_mackDump[14];
	char cVersion = (temp & 0xF0) >> 0x4;
	char cLength = temp & 0x0F;
	//4λ�汾
	printf("Version:%1x[4bit]\n", cVersion);
	//4λ�ײ�����
	printf("HeadLen:%d[4bit]\n", ((int)cLength) * 4);
	//8λ��������
	printf("TOS:0x%02x[8bit]\n", g_mackDump[15]);
	//16λ�ܳ���
	short sTotalLength = ntohs(*((short*)(g_mackDump + 16)));
	printf("PacketLen:%d[16bit]\n", sTotalLength);
	//16λ��ʶ
	printf("PacketID:%x%x[16bit]\n", g_mackDump[18], g_mackDump[19]);
	//3λ��־
	char flags = (g_mackDump[20] & 0xE0) >> 5;
	printf("Fragment Flag:0x%1x[3bit]\n", flags);
	//13λƬƫ��
	short nOffset = *((short*)(g_mackDump + 20)) & 0x1F;
	printf("Fragment offset:%dbit\n", nOffset);
	//8λ����ʱ��
	printf("TTL:%d[8bit]\n", g_mackDump[22]);

	//8λЭ��
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



	//16λ�ײ�У���
	short checkSum = ntohs(*((short*)(g_mackDump + 24)));
	printf("CheckSum:0x%x[16bit]\n", checkSum);
	//32λԴip��ַ
	struct in_addr addr = { 0 };
	addr.S_un.S_addr = *((long*)(g_mackDump + 26));
	printf("SourceIP:%s\n", inet_ntoa(addr));
	//32λĿ��ip��ַ
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
	*pkt_data==�Ƕ�������
	2����ת16���ƣ���Ϊ���������16λ��ֹ����ʾ
	ʱ��ע���ڴ氲ȫ����

	1,����ȡ��һ���ֽ�,һ���ֽ���8��2����λ���ʶ�һ���ֽ���2��ʮ�����������
	2,ȡ�ֽڵĵ�4λ����ת����Ȼ��ȡ����λת����Ȼ��浽һ���µ��ֽ����档�������ֽ���ʾ��ǰ��һ���ֽ����������
	3,��ӡ���
	*/
	memset(g_mackDump, 0, sizeof(g_mackDump));
	DWORD indexTem = 0;
	DWORD i = 0;
	for (; i < header->caplen; i++)
	{
		u_char LowBit = *(pkt_data+i) & 0x0f; //��1111��õ������ֵ
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

	printf("Ŀ���ַ��Destination����");
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

	printf("Դ��ַ��Source����");
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
	printf("���ͣ�type����");
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
	printf("Len��%d \n",header->caplen);

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
	printf("���ڻ�ȡ���������б� \n");

	pcap_if_t* newpft = pi;
	do 
	{
		printf("Name��%s;ds:%s \n", pi->name, pi->description);


	} while (pi=pi->next);
	int s = 1;
	printf("������������������ \n");
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
		/* �ͷ��豸�б� */
		pcap_freealldevs(pi);
		return -1;

	}
	pcap_freealldevs(pi);

	pcap_loop(adhandle, 0,packet_handler,NULL);
    return 0;
}

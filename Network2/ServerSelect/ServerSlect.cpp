//http://www.cnblogs.com/lidabo/p/3804411.html
//windows �� linux �׽����е� select ����ǳ��

// server.cpp :   
// �����м������׽��ֹ�����У��������������������������㣬��ȻҲ���Բ����������  

#include "winsock.h"  
#include "stdio.h"
#include <ctime>
#pragma comment (lib,"wsock32.lib")  
#define MAX 64

//Socket�б�
struct socket_list {
	SOCKET MainSock;
	int num;
	SOCKET sock_array[MAX];
};
//��ʼ���б�
void init_list(socket_list *list)
{
	int i;
	list->MainSock = 0;
	list->num = 0;
	for (i = 0; i < MAX; i++) {
		list->sock_array[i] = 0;
	}
}
//����һ��Socket���б�
void insert_list(SOCKET s, socket_list *list)
{
	int i;
	for (i = 0; i < MAX; i++) {
		if (list->sock_array[i] == 0) {
			list->sock_array[i] = s;
			list->num += 1;
			break;
		}
	}
}
//���б�ɾ��һ��Socket
void delete_list(SOCKET s, socket_list *list)
{
	int i;
	for (i = 0; i < MAX; i++) {
		if (list->sock_array[i] == s) {
			list->sock_array[i] = 0;
			list->num -= 1;
			break;
		}
	}
}

void make_fdlist(socket_list *list, fd_set *fd_list)
{
	int i;
	//FD_SET(s,*set)���򼯺��м���һ���׽ӿ�������
	//��������׽ӿ������� s û�ڼ����У������������Ѿ����õĸ���С��������ʱ���ͰѸ����������뵽�����У�����Ԫ�ظ����� 1����
	//�����ǽ� s ��ֱֵ�ӷ��������С�
	FD_SET(list->MainSock, fd_list);
	for (i = 0; i < MAX; i++) {
		if (list->sock_array[i] > 0) {
			FD_SET(list->sock_array[i], fd_list);
		}
	}
}

/*
1. ���� FD_ZERO ����ʼ���׽���״̬��
2. ���� FD_SET ������Ȥ���׽������������뼯���У�ÿ��ѭ����Ҫ���¼��룬��Ϊ select ���º󣬻ὫһЩû�������������׽����Ƴ����У���
3. ���õȴ�ʱ��󣬵��� select ���� -- �����׽��ֵ�״̬��
4. ���� FD_ISSET�����ж��׽����Ƿ�����Ӧ״̬��Ȼ������Ӧ���������磬����׽��ֿɶ����͵��� recv ����ȥ�������ݡ�
�ؼ��������׽��ֶ��к�״̬�ı�ʾ�봦��*/

int main(int argc, char* argv[])
{
	int port = 1234;
	if (argc > 1)
	{
		port = atoi(argv[1]);
		if (port < 1024 || port>65534)
		{
			printf("Port error. Use %s port\n", argv[0]);
		}
	}
	//s��Server,sock�ǿͻ���
	SOCKET s, sock;
	struct sockaddr_in ser_addr, remote_addr;
	int len;
	char buf[128];
	WSAData wsa;
	int retval;
	struct socket_list sock_list;
	fd_set readfds, writefds, exceptfds;
	timeval timeout;        //select �����ȴ�ʱ�䣬��ֹһֱ�ȴ�  
	int i;
	unsigned long arg;

	WSAStartup(0x101, &wsa);
	s = socket(AF_INET, SOCK_STREAM, 0);
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(port);
	bind(s, reinterpret_cast<sockaddr*>(&ser_addr), sizeof(ser_addr));

	listen(s, 5);
	timeout.tv_sec = 5;     // ����׽��ּ������� 1s ��û�����ݣ�select �ͻ᷵�أ���ʱ select ���� 0  
	timeout.tv_usec = 0;
	init_list(&sock_list);

	//FD_ZERO(*set)���ǰѼ�����գ���ʼ��Ϊ 0��ȷ�е�˵���ǰѼ����е�Ԫ�ظ�����ʼ��Ϊ 0�������޸�����������). 
	//ʹ�ü���ǰ�������� FD_ZERO ��ʼ�������򼯺���ջ����Ϊ�Զ���������ʱ��fd_set ����Ľ������ֵ�����²���Ԥ������⡣
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	sock_list.MainSock = s;
	arg = 1;
	ioctlsocket(sock_list.MainSock, FIONBIO, &arg);

	while (1) {
		make_fdlist(&sock_list, &readfds);
		//make_fdlist(&sock_list,&writefds);  
		//make_fdlist(&sock_list,&exceptfds);  

		retval = select(0, &readfds, &writefds, &exceptfds, &timeout);// �������ʱ�䣬�Ͳ��������������һ�� 0 ֵ��  
		if (retval == SOCKET_ERROR) {
			retval = WSAGetLastError();
			printf("error code=%d", retval);
			break;
		}
		if (retval == 0) {
			//printf("select() is time-out! There is no data or new-connect coming!\n");
			continue;
		}

		char sendBuf[128];
		//FD_ISSET(s,*set)������������Ƿ��ڼ����У�����ڼ����з��ط� 0 ֵ�����򷵻� 0.
		//���ĺ궨�岢û�и�������ʵ�֣���ʵ�ֵ�˼·�ܼ򵥣������������ϣ��ж��׽��� s �Ƿ��������С�
		if (FD_ISSET(sock_list.MainSock, &readfds)) {
			len = sizeof(remote_addr);
			sock = accept(sock_list.MainSock, reinterpret_cast<sockaddr*>(&remote_addr), &len);
			if (sock == SOCKET_ERROR)
				continue;
			printf("accept a connection\n");
			insert_list(sock, &sock_list);

			memset(sendBuf, 0, sizeof(sendBuf));
			sprintf_s(sendBuf, "Welcome.  type 1 to get time,0 to exit.");
			send(sock, sendBuf, strlen(sendBuf) + 1, 0);

		}
		for (i = 0; i < MAX; i++) {
			if (sock_list.sock_array[i] == 0)
				continue;
			sock = sock_list.sock_array[i];

			if (FD_ISSET(sock, &readfds)) {
				//���ܿͻ������ݣ��������ݳ���
				retval = recv(sock, buf, 128, 0);
				if (retval == 0) {
					closesocket(sock);
					printf("close a socket\n");
					delete_list(sock, &sock_list);
					continue;
				}
				if (retval == -1) {
					retval = WSAGetLastError();
					if (retval == WSAEWOULDBLOCK)
						continue;
					closesocket(sock);
					printf("close a socket\n");
					delete_list(sock, &sock_list);   // ���ӶϿ��󣬴Ӷ������Ƴ����׽���  
					continue;
				}
				buf[retval] = 0;
				printf("->%s\n", buf);

				memset(sendBuf, 0, sizeof(sendBuf));
				if (strcmp(buf, "date") == 0)
				{
					time_t t = time(nullptr);
					strftime(sendBuf, sizeof(sendBuf), "%Y/%m/%d %X %A ����� %j �� %z", localtime(&t));
					send(sock, sendBuf, strlen(sendBuf) + 1, 0);
				}
				else if (strcmp(buf, "q") == 0)
				{
					sprintf_s(sendBuf, "BYE");
					send(sock, sendBuf, strlen(sendBuf) + 1, 0);
					closesocket(sock);
					printf("close a socket\n");
					delete_list(sock, &sock_list);
				}
				else
				{
					sprintf_s(sendBuf, "Type 1 to get time,0 to exit.");
					send(sock, sendBuf, strlen(sendBuf) + 1, 0);
				}
			}
			//if(FD_ISSET(sock,&writefds)){  
			//}  
			//if(FD_ISSET(sock,&exceptfds)){  

		}
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);
	}
	closesocket(sock_list.MainSock);
	WSACleanup();
	return 0;
}
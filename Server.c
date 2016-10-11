#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>


//允许保存未处理链接请求数目
#define LISTENQ 8
//线程的数量
#define NUM_THREADS 10
//输入文本最大长度
#define MAXLINE 4096
#define MsgLEN 1024
//服务器端口号
#define SERV_PORT 6667



/*线程
 * connfd:与一个客户端相连接的套接字
 * ip:
 
*/
typedef struct {
	int connfd;
	struct in_addr ip;
	
}Thread_data;


//变量
pthread_t threads[NUM_THREADS];
Thread_data thread_data_array[NUM_THREADS];
int online = 0;                             //在线玩家个数
char recvbuf[MAXLINE];

//函数

void initArray();                                //初始化进程数组
void* requestHandle(void* threadid);           //线程处理函数
void recvHandle(char* m, struct in_addr ip);           //处理接收到的信息
int readn(int fd, char *bp, size_t len);       //都取socket传来的数据

int main(int argc, char** argv){
	
//	pid_t childpid;
	int rc;
	int listenfd, connfd;
	int i=0,count=0;
	socklen_t clilen;

	initArray();   //初始化进程数组

	struct sockaddr_in cliaddr, servaddr;
	
	//为服务器创建一个套接字
	listenfd = socket(AF_INET , SOCK_STREAM , 0);
	
	//填充套接字地址
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
		
	const int on = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	//为套接字分配一个本地套接字地址
	bind(listenfd , (struct sockaddr *)&servaddr , sizeof(servaddr));
	
	//将服务器套接字转换为被动套接字，接受客户端的连接请求
	listen(listenfd , LISTENQ);

	printf("%s\n",  "Server running...waiting for connections.");
	
	while(1){
		i = 0;
		clilen = sizeof(cliaddr);
		//接受客户端的连接
		connfd = accept(listenfd , (struct sockaddr *)&cliaddr , &clilen);
		printf("%s\n", "Received request...");
		while(i<NUM_THREADS){
			if(thread_data_array[i].connfd == -1){
				break;
			}
			i++;
		}
		thread_data_array[i].ip = cliaddr.sin_addr;  //ip
		thread_data_array[i].connfd = connfd;   //socket
		rc = pthread_create(&threads[i], NULL, requestHandle, (void*)&thread_data_array[i]);
		if(rc){
			printf("ERROR; return code from pthread_create() is %d\n",i);
			exit(-1);
		}
	}
	return 0;
}

void initArray(){  //初始化进程数组
	int i=0;
	for(i=0; i<NUM_THREADS; i++){
		thread_data_array[i].connfd = -1;
		//thread_data_array[i].ip = NULL;
	}
}

void* requestHandle(void* threadPoint){   //线程处理函数
	int n = 0;
	printf("thread cearted for dealing with client requests.\n");
	memset(recvbuf , 0 , MAXLINE);
	int connfd = ((Thread_data*)threadPoint)->connfd;
	while( n = recv(connfd , recvbuf , MAXLINE,0) > 0){
		//printf("%s%d\n", "string received from thread ", ((Thread_data*)threadPoint)->thread_id);
		recvHandle(recvbuf, ((Thread_data*)threadPoint)->ip);
		memset(recvbuf , 0 , MAXLINE);
	}
	//printf("thread %d exit.\n",((Thread_data*)threadPoint)->thread_id);
	if(n < 0){
		printf("%s\n", "Read error.");
	}
	Thread_data* tp = (Thread_data*)threadPoint;
	tp->connfd = -1;
	//tp->ip = NULL;
	
	pthread_exit(NULL);
}


void recvHandle(char* m, struct in_addr ip){  //处理接收到的信息
	int i = 0;
	char sendBuf[MAXLINE];
	
	for(i=0; i<NUM_THREADS; i++){
		if(thread_data_array[i].connfd != -1){
			//put "ip:message" to the sendBuf
			memset(sendBuf, 0 , MAXLINE);
			sprintf(sendBuf,"%d", inet_ntoa(ip));
			sendBuf[strlen(sendBuf)] = ':';
			strcat(sendBuf, m );
			send(thread_data_array[i].connfd, sendBuf, MAXLINE, 0);
		}
	}

}


int readn(int fd, char *bp, size_t len)   //都取socket传来的数据
{
	int cnt;
	int rc;

	cnt = len;
	while ( cnt > 0 )
	{
		rc = recv( fd, bp, cnt, 0 );
		if ( rc < 0 )				/* read error? */
		{
			if ( errno == EINTR )	/* interrupted? */
				continue;			/* restart the read */
			return -1;				/* return error */
		}
		if ( rc == 0 )				/* EOF? */
			return len - cnt;		/* return short count */
		bp += rc;
		cnt -= rc;
	}
	return len;
}

#include "head.h"

int socket_connect();
int main()
{
	int sockfd = socket_connect();

	while(1)
	{
		pid_t pid;
		struct sockaddr_in clientaddr;
		socklen_t len = sizeof(clientaddr);
		int connfd = accept(sockfd,(struct sockaddr *)&clientaddr,&len);
		if(connfd == -1)
		{
			perror("accept failed\n");
			continue;
		}
		else
		{
			cout<<"客户端连接成功"<<endl;

			pid = fork();
			if(pid < 0)
			{
				perror("fork failed\n");
			}
			else if(pid == 0)
			{
				SDisk disk;
				disk.set_socket(connfd);
				disk.user_login();
                disk.disk_control();

			}
			else
			{
				close(connfd);
			}

		}
	}

	return 0;
}


int SDisk:: disk_control()
{
   // Control control;
    char m_resp[BUFF_SIZE];
    char control[32];

    while(1) {

        memset(m_resp, '0', BUFF_SIZE);
        if(recv(sockfd, m_resp, BUFF_SIZE, 0) < 0) {
            perror("accept control signal error!");
        }

        memset(control, '0', sizeof(control));
        sscanf(m_resp, "%s", control);
        cout << "control: " << control << endl;

        if(strcmp(control, "PUSH") == 0) {
            disk_upload();
        } else if(strcmp(control, "PULL") == 0) {
            disk_download();
        }
        /*
        switch(control) {
            case PUSH:
                disk_upload();
                break;
            case PULL:
                cout << "ggggggggggggggg" << endl;
                disk_download();
                break;
        }
        */

    }

}

int socket_connect()
{
	struct sockaddr_in addr;
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, SERV_IP, &addr.sin_addr);
	//addr.sin_addr.s_addr = htonl(SERV_IP);

	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
		return -1;

	if((bind(sockfd,(struct sockaddr *)&addr,sizeof(addr))) == -1)
		return -1;

	if(listen(sockfd,20))
		return -1;

	return sockfd;
}

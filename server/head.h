#ifndef HEAD_H_
#define HEAD_H_


#include <iostream>
#include <exception>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERV_IP "127.0.0.1"
#define SERV_PORT  10000
#define BUFF_SIZE 256
#define BUFF_DATA_SIZE 1024

//typedef enum Control{PUSH, PULL, LISTU, LISTS, DELETE, SHARE}Control;

struct User{
    char name[32];
    char passwd[32];
};

struct File{
     char name[32];
     int typeP;
     char md5[32];
};





using namespace std;

class SDisk
{
	public:
		SDisk();

		~SDisk();

		int user_register();//用户注册

		int user_login();//user login online_disk

        int disk_control();

        int user_cd();    //登录用户主目录

		int disk_list();//显示我的文件

        int disk_listShare();   //显示分享文件

		int disk_upload();//上传文件

		int disk_download();//下载文件

		int disk_quit();//退出登录



		void set_socket(int socket);

	private:

		int sockfd;//socket id
        struct User user;

    public:

        char m_cmd[BUFF_SIZE];
        char m_resp[BUFF_SIZE];
        char send_buff[BUFF_DATA_SIZE];
        char recv_buff[BUFF_DATA_SIZE];


};





















#endif

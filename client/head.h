#ifndef _HEAD_H
#define _HEAD_H

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 10000

#define BUFF_SIZE  256
#define BUFF_DATA_SIZE   1024

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/md5.h>

using namespace std;

typedef enum Control{PUSH, PULL, LISTU, LISTS, DELETE, SHARE}Control;

struct User{
    char name[32];
    char passwd[32];
};

struct File{
    char name[32];
    int typeP;
    char md5[32];
};

class CDisk
{
    public:

        CDisk(int sockfd);
        ~CDisk();

        int user_login();          //user login online_disk
        int disk_control();
        int disk_list();            //显示私有文件
        int disk_listShare();       //显示共享文件
        int disk_share();

        int disk_upload(struct File f);         //上传文件
        int disk_download(struct File f);       //下载文件
        int disk_quit();           //退出登录

    private:

        int sockfd;
        struct User user;

        char m_cmd[BUFF_SIZE];
        char m_resp[BUFF_SIZE];
        char send_buff[BUFF_DATA_SIZE];
        char recv_buff[BUFF_DATA_SIZE];


};


#endif

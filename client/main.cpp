#include "head.h"


int connect();
//int control(CDisk *disk);

int main(int argc, char const* argv[])
{
    int sockfd;
    sockfd = connect();
    CDisk disk(sockfd);
    disk.user_login();
    disk.disk_control();

    return 0;
}


int connect()
{
    int sockfd = 0;
    struct sockaddr_in addr_ser;
    socklen_t addrlen  = sizeof(struct sockaddr);
    int res;

    cout << "Welcome to the Disk system" << endl;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror("Create sockfd error!");
        exit(1);
    }

    bzero(&addr_ser, sizeof(addr_ser));
    addr_ser.sin_family = AF_INET;
    addr_ser.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &addr_ser.sin_addr);

    res = connect(sockfd, (struct sockaddr *)&addr_ser, addrlen);
    if(res == -1) {
        perror("Connect error!");
        exit(-1);
    }

    return sockfd;
}

#if 1

int CDisk:: disk_control()
{
    char order_buff[BUFF_SIZE];
    char arg1[32];
    char arg2[32];
    char arg3[32];
    char arg4[32];

    while(1) {
        cout << "请输入操作指令:" << endl;
        memset(order_buff, '0', sizeof(order_buff));
        memset(arg1, '0', sizeof(arg1));
        memset(arg2, '0', sizeof(arg2));
        memset(arg3, '0', sizeof(arg3));
        memset(arg4, '0', sizeof(arg4));

        read(0, order_buff, BUFF_SIZE);
        //fgets(order_buff, BUFF_SIZE, stdin);
        sscanf(order_buff, "%s %s %s %s", arg1, arg2, arg3, arg4);

        if(strcmp(arg1, "disk") == 0) {
            if(strcmp(arg2, "push") == 0) {
                struct File file;
                memset((void *)&file, 0, sizeof(file));
                strcpy(file.name, arg3);
                if(strcmp(arg4, "true") == 0) {
                    file.typeP = 1;
                    disk_upload(file);
                } else if(strcmp(arg4, "false") == 0) {
                    file.typeP = 0;
                    disk_upload(file);
                } else {
                     perror("文件属性格式错误!");
                     continue;
                }
            } else if(strcmp(arg2, "pull") == 0) {
                 struct File file;
                 memset((void *)&file, 0, sizeof(file));
                 strcpy(file.name, arg3);
                 disk_download(file);
            }else {
                 cout << "该操作不支持！" << endl;
            }
        } else {
            cout << "该命令不支持!" << endl;
        }
    }
}
#endif

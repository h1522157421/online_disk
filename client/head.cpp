#include "head.h"


int md5sum(char *filename, unsigned char *md5);
long int get_file_size(char *filename);
int get_file_block(char *filename);
//int set_file_block(char *filename, long int block);

CDisk::CDisk(int my_sockfd)
{
    sockfd = my_sockfd;
}

CDisk:: ~CDisk()
{

}

int CDisk:: user_login()          //user login online_disk
{
    char res[32];
    char again[32];

    cout << "Please input user_name" << endl;
    scanf("%s", user.name);
    cout << "Please input password" << endl;
    scanf("%s", user.passwd);

    memset(m_cmd, 0, BUFF_SIZE);
    sprintf(m_cmd, "%s %s\n", user.name, user.passwd);

    if((send(sockfd, m_cmd, BUFF_SIZE, 0)) < 0) {
        perror("Send user_info error!");
        exit(-1);
    }

    if((recv(sockfd, m_resp, BUFF_SIZE, 0)) < 0) {
        perror("recv Login error!");
        exit(-1);
    }

    sscanf(m_resp, "%s", &res);
    if(strcmp(res,"-1") == 0) {
        cout<<"Login error!"<<endl;
        exit(-1);
    }
    if(strcmp(res, "1") == 0) {
     cout << "Login success!" << endl;
    }
    return 0;
}



int CDisk:: disk_list()            //显示我的文件
{
    memset(m_cmd, '0', BUFF_SIZE);
    strcpy(m_cmd, "ls");
    //disk_sendcmd();

}


int CDisk:: disk_listShare()            //显示分享文件
{

}



int CDisk:: disk_upload(struct File f)         //上传文件
{
    struct File file = f;

    char md5[64];
    unsigned char md5tmp[64];
    int i = 0;
    FILE *fp = NULL;
    char ch[32];

    memset(md5, '0', sizeof(md5));
    memset(md5tmp, '0', sizeof(md5tmp));

    md5sum(file.name, md5tmp);
    for(i = 0; i < 16; ++i) {
       sprintf(&md5[i*2], "%02x", md5tmp[i]);
    }
    //cout << md5 << endl;

    strcpy(file.md5, md5);

    memset(m_cmd, '0', BUFF_SIZE);
    memcpy(m_cmd, "PUSH", BUFF_SIZE);
    if(send(sockfd, m_cmd, BUFF_SIZE, 0) < 0) {
         perror("Send push_file signal error!");
         exit(-1);
    }

    if(send(sockfd, (char *)&file, sizeof(struct File), 0) < 0) {
        perror("文件指令发送失败!");
        exit(-1);
    }

    memset(m_resp, '0', BUFF_SIZE);
    if(recv(sockfd, m_resp, BUFF_SIZE, 0) < 0) {
        perror("error");
    }

    sscanf(m_resp, "%s", ch);
    if(ch[0] == 'y') {
         cout << "该文件已在服务器中存在, 秒传!" << endl;
         return 0;
    }

    long int file_size = get_file_size(file.name);
    cout << file_size << endl;

    memset(m_cmd, '0', BUFF_SIZE);
    sprintf(m_cmd, "%d", file_size);
    if(send(sockfd, m_cmd, BUFF_SIZE, 0) < 0) {
         perror("Send push_file signal error!");
         exit(-1);
    }


#if 1
    if((fp = fopen(file.name, "rb")) == NULL) {
         perror("文件打开失败！");
         exit(-1);
    }


    memset(send_buff, '0', BUFF_DATA_SIZE);
    cout << "正在传输文件！" << endl;
    int len = 0;

    while(file_size > 0) {
        if((len = fread(send_buff, 1, BUFF_DATA_SIZE, fp)) < 0) {
            perror("读文件内容失败！");
            exit(-1);
        }
        if(send(sockfd, send_buff, len, 0) < 0) {
            perror("发送数据失败！");
            exit(-1);
        }
        memset(send_buff, '0',  BUFF_DATA_SIZE);

        file_size -= len;

    }

    if(file_size == 0) {
         cout << "传输完成！" << endl;
    } else {
         cout << "传输失败！" << endl;

    }
    fclose(fp);
#endif
}

int CDisk:: disk_download(struct File f)
{
    struct File file = f;
    char ch[32];
    int t1 = 0;
    long int seek = 0;
    FILE *fp = NULL;
    long int file_len = 0;
    long int block = 0;
    int read_len = 0;
    int write_len = 0;
    char md5[64];
    unsigned char md5tmp[64];
    int i = 0;

    //cout << "file_name: " << file.name << endl;

    memset(m_cmd, '0', BUFF_SIZE);
    memcpy(m_cmd, "PULL", BUFF_SIZE);
    if(send(sockfd, m_cmd, BUFF_SIZE, 0) < 0) {
         perror("Send pull_file signal error!");
         exit(-1);
    }

    if(send(sockfd, (char *)&file, sizeof(struct File), 0) < 0) {
        perror("文件信息（文件名）发送失败!");
        exit(-1);
    }

    memset(m_resp, '0', BUFF_SIZE);
    if(recv(sockfd, m_resp, BUFF_SIZE, 0) < 0) {        //接收文件是否存在
        perror("error");
    }

    memset(ch, '0', sizeof(ch));
    sscanf(m_resp, "%s", ch);
    if(ch[0] == 'n') {
        cout << "您所要下载的文件不存在！ 请检查文件名或LIST命令查看所有文件！" << endl;
        return 0;
    }

    cout << "您所要下载的文件存在！" << endl;

    memset(m_resp, '0', BUFF_SIZE);
    if(recv(sockfd, m_resp, BUFF_SIZE, 0) < 0) {
        perror("文件信息接收失败!");
        exit(-1);
    }

    memset(&file, '0', sizeof(file));
    sscanf(m_resp, "%s %d %s", file.name, &file.typeP, file.md5);
    cout << "file_info: " << file.name << " " <<  file.typeP << " " << file.md5 << endl;

    char fname[32];
    memset(fname, '0', sizeof(fname));
    strcpy(fname, file.name);
    strcat(fname, "_tmp");

    t1 = get_file_block(fname);       //查看是否可续传
    seek = t1 * BUFF_DATA_SIZE;
    cout << "get_seek: " << seek << endl;


    memset(m_cmd, '0', BUFF_SIZE);
    sprintf(m_cmd, "%ld", t1);
    if(send(sockfd, m_cmd, BUFF_SIZE, 0) < 0) {
         perror("Send start block error!");
         exit(-1);
    }
    memset(m_resp, '0', BUFF_SIZE);
    if(recv(sockfd, m_resp, BUFF_SIZE, 0) < 0) {
        perror("接收待读文件长度失败!");
        exit(-1);
    }
    sscanf(m_resp, "%ld", &file_len);
    cout << "file_len: " << file_len << endl;

    if((fp = fopen(file.name, "wba+")) == NULL) {
        perror("待写文件打开失败！");
        exit(-1);
    }
    fseek(fp, seek, SEEK_SET);

#if 1
    FILE *fp1 = fopen(fname, "wta+");
    if(fp1 == NULL ) {
         perror("打开临时文件失败！");
         exit(-1);
    }

#endif

    memset(recv_buff, '0', BUFF_DATA_SIZE);
    while(file_len > 0) {
        if((read_len = recv(sockfd, recv_buff, BUFF_DATA_SIZE, 0)) < 0) {
            perror("接收文件内容出错！");
            exit(-1);
        }
        ++block;
        if(block == 1) {
             cout << "正在接收文件！" << endl;
        }

        write_len = fwrite(recv_buff, sizeof(char), read_len, fp);
        if(write_len > read_len) {
             perror("写入文件出错！");
             exit(-1);
        }


        //cout << "block: " <<  block << endl;

        int block_t = block;
        char ch[8] = {0};
        int i = 7;
        memset(ch, '0', sizeof(ch));
        while(block_t) {
             ch[i] = block_t % 10 + 48;
             block_t /= 10;
             --i;
        }
        //printf("ch: %s\n", ch);
        //cout << "ch[]: " << ch << endl;

        //fprintf(fp1, "\n%ld", block);
        fwrite(ch, sizeof(char), 8, fp1);
        //set_file_block(fname, block);
        file_len -= read_len;
        memset(recv_buff, '0', BUFF_DATA_SIZE);
    }
    fclose(fp);
    //fclose(fp1);

    if(block <= 0) {
        perror("文件传输失败！");
        return 0 ;
    }



    memset(md5, '0', sizeof(md5));
    memset(md5tmp, '0', sizeof(md5tmp));

    md5sum(file.name, md5tmp);
    for(i = 0; i < 16; ++i) {
       sprintf(&md5[i*2], "%02x", md5tmp[i]);
    }
    cout << "下载的文件md5值： " <<  md5 << endl;
    if(strcmp(file.md5, md5) == 0) {
        cout << "文件成功下载！" << endl;
        remove(fname);
    } else {
        cout << "文件下载有误！" << endl;
    }



    return 0;
}

#if 0
int set_file_block(char *filename, long int block)
{
    //cout << "block: " <<  block << endl;
    FILE *fp = NULL;

    fp = fopen(filename, "wt");
    if(fp == NULL) {
         perror("打开临时文件失败！");
    } else {
         fprintf(fp, "%ld", block);
         //fwrite(&block, sizeof(long int), 1, fp);
         fclose(fp);
    }
}
#endif

int get_file_block(char *filename)
{
    FILE *fp = NULL;
    long int block = 0;
    char ch[8];

    memset(ch, '0', sizeof(ch));

    fp = fopen(filename, "rt");
    if(fp == NULL) {
         cout << "没有临时文件！" << endl;
    } else {
         fseek(fp, -8, SEEK_END);
         fread(ch, sizeof(ch), 1, fp);
         //cout << "ch: " << ch << endl;
         block = atoi(ch);
         fclose(fp);
    }

    return block;
}




long int get_file_size(char *filename)
{
    FILE *fp = fopen(filename, "r");
    fseek(fp, 0, SEEK_END);
    long int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fclose(fp);
    return size;
}

int md5sum(char *filename, unsigned char *md5)
{
    MD5_CTX ctx;
    char buffer[1024];
    unsigned char outmd[16];
    int len = 0;
    int i = 0;
    FILE *fp = NULL;

    memset(outmd, 0, sizeof(outmd));
    memset(buffer, 0, sizeof(buffer));
    fp = fopen(filename, "rb");
    if(fp == NULL) {
        perror("Push file open error!");
    }
    MD5_Init(&ctx);
    while((len = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        MD5_Update(&ctx, buffer, len);
        memset(buffer, 0, sizeof(buffer));
    }

    MD5_Final(outmd, &ctx);
    for(i =0; i < 16; ++i) {
        md5[i] = outmd[i];
    }
    fclose(fp);
    return 0;
}


int disk_download();       //下载文件
int disk_quit();           //退出登录

/*
int CDisk:: disk_sendcmd()        //发送指令
{
    try {
        if((send(sockfd, m_cmd, strlen(m_cmd), 0)) < 0) {
            perror("Send error!");
            exit;

        }
    }catch(exception e) {
        cout << e.what() << endl;
    }
}
int CDisk:: disk_recvinfo()
{
    memset(m_resp, 0, BUFF_SIZE);
    try {
        recv(sockfd, m_resp, BUFF_SIZE, 0);
    } catch(exception e) {
         cout << e.what() << endl;
    }

}
*/



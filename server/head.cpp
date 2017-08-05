#include "head.h"

using namespace std;

int mysql_check_ufile(struct User user, struct File *file);
int mysql_check_md5(struct File *file, char *file_name);
int mysql_check_file_exist(struct File *file);
int mysql_fill(struct User user, struct File file, int flag);
long int get_file_size(char *filename);

SDisk::SDisk()
{

}

SDisk::~SDisk()
{

}

int SDisk::user_login()
{
    char name[32];
    char passwd[32];
#if 1
    memset(m_resp, '0', BUFF_SIZE);
    if((recv(sockfd, m_resp, BUFF_SIZE, 0)) < 0) {
        perror("Recv user_info error!");
        exit(-1);
    }
	sscanf(m_resp,"%s%s", user.name, user.passwd);
    strcpy(name, user.name);
    strcpy(passwd, user.passwd);

	const char* db = "online_disk",*server = "127.0.0.1",
	      		*user = "root",*password = "111111";

	MYSQL connection;
	mysql_init(&connection);
	char sql[256] = {0};

    cout << name <<  " " << passwd << endl;
    //strcpy(sql, "select * from user where username='name' and passwd='passwd'");
	strcpy(sql,"select * from user where username=\"");
	strcat(sql,name);
	strcat(sql,"\" and passwd=\"");
	strcat(sql,passwd);
	strcat(sql,"\";");
	if(mysql_real_connect(&connection,server,user,password,db,0,NULL,0))
	{
		int res = mysql_query(&connection,sql);
		if(res)
		{
			cout<<"Select Error."<<endl;
			return -1;
		}
		else
		{
			MYSQL_RES *res_ptr = mysql_store_result(&connection);
			if(res_ptr)
			{
				int column = mysql_num_fields(res_ptr);//获取列数
				int row = mysql_num_rows(res_ptr)+1;//获取行数，+1表示还有第一行字段名
				if(row <= 1)
				{
					memset(m_resp,'0',BUFF_SIZE);
					strcpy(m_resp,"-1");

				}
				else
				{
                    memset(m_resp, '0', BUFF_SIZE);
                    strcpy(m_resp, "1");
                    /*
					MYSQL_ROW result_row = mysql_fetch_row(res_ptr);
						cout<<"获取到UID是: "<<result_row[0]<<endl;
					int uid = atoi(result_row[0]);
					memset(m_cmd,0,BUFFER_SIZE);
					strcpy(m_cmd,"获取的UID是:");
					strcat(m_cmd,result_row[0]);
					send(mysocket,m_cmd,strlen(m_cmd),0);
                    */
				}

                if((send(sockfd, m_resp, BUFF_SIZE, 0)) < 0) {
                     perror("Sent User_resp error!");
                     exit(-1);
                }

			}
		}

	}
	return 0;
#endif
}

void SDisk::set_socket(int sock)
{
	sockfd = sock;
}

int SDisk:: disk_upload()
{
   // cout << "upload: user:" << user.name << endl;
    FILE *fp = NULL;
    struct File file;
    int file_size = 0;
    int size = 0;
    int read_len = 0;
    int write_len = 0;
    char file_name[32];

    memset(&file, '0', sizeof(file));
    if((recv(sockfd, &file, sizeof(file), 0)) < 0) {
        perror("Recv file info error!");
        exit(-1);
    }

    char f_name[32] = {'0'};
    strcpy(f_name, user.name);
    strcat(f_name, "_");
    strcat(f_name, file.name);

    memset(&file.name, '0', sizeof(file.name));
    strcpy(file.name, f_name);
   // cout << file.name << " " <<  file.typeP << " " << file.md5 << endl;

    memset(file_name, '0', sizeof(file_name));
    int t = mysql_check_md5(&file, file_name);
    //cout << "t: " << t;

    if(t != 0) {
        cout << "该文件已存在，使用秒传功能！" << endl;

        memset(m_cmd, '0', BUFF_SIZE);
        strcpy(m_cmd, "yes");
        if((send(sockfd, m_cmd, BUFF_SIZE, 0)) < 0) {
            perror("回应文件是采用秒传失败！");
            return 0;
        }

        memset(file.name, '0', sizeof(file.name));
        strcpy(file.name, file_name);
        int t1 = mysql_check_ufile(user, &file);
       // cout << "t1" << t1 << endl;
        if(t1 == 0) {
            mysql_fill(user, file, 0);
        } else {
             cout << "该文件已由该用户上传过！" << endl;
        }
        return 0;
    }

    memset(m_cmd, '0', BUFF_SIZE);
    strcpy(m_cmd, "no");
    if((send(sockfd, m_cmd, BUFF_SIZE, 0)) < 0) {
        perror("回应文件不是采用秒传失败！");
    }


    memset(m_resp, '0', BUFF_SIZE);
    if((recv(sockfd, m_resp, BUFF_SIZE, 0)) < 0) {
        perror("Recv file len error!");
        exit(-1);
    }

    sscanf(m_resp, "%d", &file_size);
    //cout << file_size << endl;

    if((chdir("/root/2016-2/test")) < 0) {
         perror("Change home_dir error!");
         exit(-1);
    }

    //cout << "nnnnnnnnnnnnnnnnnnn" << endl;
    if((fp = fopen(file.name, "wb")) == NULL) {
         perror("文件打开失败！");
         exit(-1);
    }
    //cout << "mmmmmmmmmmmmmmmmmmmm" << endl;


    memset(recv_buff, '0', BUFF_DATA_SIZE);
    while(file_size > 0) {
        if((read_len = recv(sockfd, recv_buff, BUFF_DATA_SIZE, 0)) < 0) {
            perror("接收文件内容出错!");
            exit(-1);
        }
        ++size;
        if(size == 1) {
            cout << "正在接收来自" << user.name << "的文件" << endl;
        }

        write_len = fwrite(recv_buff, sizeof(char), read_len, fp);
        if(write_len > read_len) {
             perror("写入文件出错！");
             exit(-1);
        }

        file_size -= read_len;
        memset(recv_buff, '0', BUFF_DATA_SIZE);
    }

    if(size > 0) {
         //cout << user.name << "的文件" << file.name << "传送结束!" << endl;
         mysql_fill(user, file, 1);
    } else {
         perror("文件传输失败！");
    }

    fclose(fp);
    exit(0);

}


int SDisk:: disk_download()
{
    //cout << "llllllllllllllllllllllll" << endl;
    struct File file;
    int t = 0;
    long int file_len = 0;
    long int seek = 0;
    FILE *fp = NULL;
    int len = 0;


    memset(&file, '0', sizeof(file));
    if((recv(sockfd, &file, sizeof(file), 0)) < 0) {
        perror("Recv file info error!");
        exit(-1);
    }

    cout << "down: file.name" << file.name << endl;

    t = mysql_check_file_exist(&file);

    cout << "t:  " << t << endl;
    if(t == 0) {
        memset(m_cmd, '0', BUFF_SIZE);
        strcpy(m_cmd, "no");
        if((send(sockfd, m_cmd, BUFF_SIZE, 0)) < 0) {
            perror("回应文件不存在失败！");
            return 0;
        }
        return 0;
    }

    memset(m_cmd, '0', BUFF_SIZE);
    strcpy(m_cmd, "yes");
    if((send(sockfd, m_cmd, BUFF_SIZE, 0)) < 0) {
        perror("回应文件存在失败！");
        return 0;
    }

    cout << "file_info:" << file.name << " " << file.typeP << " " << file.md5 << endl;

    memset(m_cmd, '0', BUFF_SIZE);
    sprintf(m_cmd, "%s %d %s", file.name, file.typeP, file.md5);
    if((send(sockfd, m_cmd, BUFF_SIZE, 0)) < 0) {
        perror("send file info error!");
        exit(-1);
    }

    memset(m_resp, '0', BUFF_SIZE);
    if((recv(sockfd, m_resp, BUFF_SIZE, 0)) < 0) {
        perror("Recv start block error!");
        exit(-1);
    }
    sscanf(m_resp, "%ld", &seek);
    seek *= BUFF_DATA_SIZE;

    cout << "seek: " << seek << endl;

    if((chdir("/root/2016-2/test")) < 0) {
         perror("Change home_dir error!");
         exit(-1);
    }

    file_len = get_file_size(file.name);
    file_len -= seek;

    cout << "file_len: " << file_len << endl;

    memset(m_cmd, '0', BUFF_SIZE);
    sprintf(m_cmd, "%ld", file_len);
    if((send(sockfd, m_cmd, BUFF_SIZE, 0)) < 0) {
        perror("待传输文件长度发送失败！");
    }



    if((fp = fopen(file.name, "rb")) == NULL) {
         perror("待写文件打开失败！");
         exit(-1);
    }

    fseek(fp, seek, SEEK_SET);

    memset(send_buff, '0', BUFF_DATA_SIZE);
    cout << "正在传输文件！" << endl;

    while(file_len > 0) {
        if((len = fread(send_buff, 1, BUFF_DATA_SIZE, fp)) < 0) {
            perror("读文件内容失败！");
            exit(-1);
        }

        if(send(sockfd, send_buff, len, 0) < 0) {
             perror("发送文件内容失败！");
             exit(-1);
        }

        memset(send_buff, '0', BUFF_DATA_SIZE);
        file_len -= len;
    }

    if(file_len == 0) {
         cout << "文件传输完成！" << endl;
    } else {
        cout << "文件传输失败！" << endl;
    }
    fclose(fp);
}






int mysql_check_file_exist(struct File *file)
{
    cout << "check file_name: " << file->name << endl;
     MYSQL conn;
     MYSQL_RES *res_ptr;
     MYSQL_ROW result_row;
     int res = 0;
     int row = 0;
     int column = 0;
     int value = 0;
     char sql[256];
     //char file_name[32];
     memset(sql, '0', sizeof(sql));
     strcpy(sql, "select * from file where name='");
     strcat(sql, file->name);
     strcat(sql, "';");
     //cout << "查询的sql: " << sql << endl;

     mysql_init(&conn);
     if(mysql_real_connect(&conn, "127.0.0.1", "root", "111111", "online_disk", 0, NULL, CLIENT_FOUND_ROWS)) {
         res = mysql_query(&conn, sql);

         if(res) {
              perror("Select sql error!");
              exit(-1);
         } else {
             res_ptr = mysql_store_result(&conn);
             if(res_ptr) {
                  column = mysql_num_fields(res_ptr);
                  row = mysql_num_rows(res_ptr) + 1;
                  if(row <= 1) {
                      value = 0;
                  } else {
                      //memset(file_name, '0', sizeof(file_name));
                      result_row = mysql_fetch_row(res_ptr);
                      strcpy(file->md5, result_row[2]);
                      file->typeP = atoi(result_row[1]);
                      value = 1;
                  }
             } else {
                  cout << "没有查询到匹配的数据！" << endl;
             }
         }
     } else {
         perror("Connect failed!");
         exit(-1);
     }

     mysql_close(&conn);

     //cout << "check md5: " << value;
     return value;

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

int mysql_fill(struct User user, struct File file, int flag)
{
    //cout << "fill :  user_name: " << user.name << "  file_name: " <<  file.name << endl;
    MYSQL conn;
    int res = 0;
    char sql[256];
    char tmp[32];

    mysql_init(&conn);
    if(mysql_real_connect(&conn, "127.0.0.1", "root", "111111", "online_disk", 0, NULL, CLIENT_FOUND_ROWS)) {
        if(flag == 1) {
            memset(sql, '0', sizeof(sql));
            strcpy(sql, "insert into file values('");
            strcat(sql, file.name);
            strcat(sql, "',");
            sprintf(tmp, "%d", file.typeP);
            strcat(sql, tmp);
            strcat(sql, ",'");
            strcat(sql, file.md5);
            strcat(sql, "');");
            //cout << "insert file: " << sql << endl;

            res = mysql_query(&conn, sql);
            if(res) {
                 cout << "Insert file error!" << endl;
            } else {
                cout << "Insert file success!" << endl;
            }
        }

        memset(sql, '0', sizeof(sql));
        strcpy(sql, "insert into access values('");
        strcat(sql, user.name);
        strcat(sql, "','");
        strcat(sql, file.name);
        strcat(sql, "');");
       // cout << "insert access: " << sql << endl;

        res = mysql_query(&conn, sql);
        if(res) {
            cout << "Insert access error!" << endl;
        } else {
            cout << "Insert access access!" << endl;
        }
    } else {
         perror("COnnect Failed!");
         exit(-1);
    }

    return 0;
}

int mysql_check_ufile(struct User user, struct File *file)
{

    cout << "u_file :  user_name: " << user.name << "  file_name: "  << file->name << endl;
    //cout << "check md5: " << file->md5 << endl;
     MYSQL conn;
     MYSQL_RES *res_ptr;
     MYSQL_ROW result_row;
     int res = 0;
     int row = 0;
     int column = 0;
     int value = 0;
     char sql[256];
     //memset(sql, '0', sizeof(sql));
     strcpy(sql, "select * from access where uname='");
     strcat(sql, user.name);
     strcat(sql, "' and fname='");
     strcat(sql, file->name);
     strcat(sql, "';");
     //cout << "查询的access sql: " << sql << endl;

     mysql_init(&conn);
     if(mysql_real_connect(&conn, "127.0.0.1", "root", "111111", "online_disk", 0, NULL, CLIENT_FOUND_ROWS)) {
         res = mysql_query(&conn, sql);

         if(res) {
              perror("Select sql error!");
              exit(-1);
         } else {
             res_ptr = mysql_store_result(&conn);
             if(res_ptr) {
                  column = mysql_num_fields(res_ptr);
                  row = mysql_num_rows(res_ptr) + 1;
                  if(row <= 1) {
                      value = 0;
                  } else {
                      // result_row = mysql_fetch_row(res_ptr);
                      //value = atoi(result_row[0]);
                      value = 1;
                  }
             } else {
                  cout << "没有查询到匹配的数据！" << endl;
             }
         }
     } else {
         perror("Connect failed!");
         exit(-1);
     }

     mysql_close(&conn);

     //cout << "check md5: " << value;
     return value;
}

int mysql_check_md5(struct File *file, char *file_name)
{
    //cout << "check md5: " << file->md5 << endl;
     MYSQL conn;
     MYSQL_RES *res_ptr;
     MYSQL_ROW result_row;
     int res = 0;
     int row = 0;
     int column = 0;
     int value = 0;
     char sql[256];
     //char file_name[32];
     //memset(sql, '0', sizeof(sql));
     strcpy(sql, "select * from file where md5='");
     strcat(sql, file->md5);
     strcat(sql, "';");
     //cout << "查询的sql: " << sql << endl;

     mysql_init(&conn);
     if(mysql_real_connect(&conn, "127.0.0.1", "root", "111111", "online_disk", 0, NULL, CLIENT_FOUND_ROWS)) {
         res = mysql_query(&conn, sql);

         if(res) {
              perror("Select sql error!");
              exit(-1);
         } else {
             res_ptr = mysql_store_result(&conn);
             if(res_ptr) {
                  column = mysql_num_fields(res_ptr);
                  row = mysql_num_rows(res_ptr) + 1;
                  if(row <= 1) {
                      value = 0;
                  } else {
                      //memset(file_name, '0', sizeof(file_name));
                      result_row = mysql_fetch_row(res_ptr);
                      strcpy(file_name, result_row[0]);
                      //file_name = atoi(result_row[0]);
                      value = 1;
                  }
             } else {
                  cout << "没有查询到匹配的数据！" << endl;
             }
         }
     } else {
         perror("Connect failed!");
         exit(-1);
     }

     mysql_close(&conn);

     //cout << "check md5: " << value;
     return value;
}


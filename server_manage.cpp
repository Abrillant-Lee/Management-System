#include <iostream>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "server_mysql.h"

using namespace std;

int flag = 1;
int cnt = 0;
int client[128] = {0};
student stu;

void *Read_Data(void *arg)
{
    int ret;                       // 用于存储read函数的返回值，即实际读取的字节数
    int i;                         // 一般用于循环计数或临时变量
    int cli = *(int *)arg;         // 从指针arg中取出值，赋给cli。这里假设arg是一个指向int的指针，cli可能是一个客户端的socket描述符
    cout << "cli " << cli << endl; // 输出cli的值，用于调试或显示客户端的socket描述符
    char buf[32] = {0};            // 定义一个字符数组buf，大小为32，所有元素初始化为0。用于存储从客户端读取的数据
    while (1)
    {
        if (flag == 1)
        {
            ret = read(cli, buf, sizeof(buf)); // 读取数据
            if (ret > 0)
            {
                switch (buf[0])
                {
                case '1':
                    Mysql_Find(cli);
                    cout << "显示学生信息" << endl; /* 显示学生信息 */
                    break;
                case '2':
                    flag = 0;
                    stu.Mysql_Add(cli);
                    cout << "添加学生信息" << endl; /* 添加学生信息 */
                    break;
                case '3':
                    flag = 0;
                    stu.Mysql_Dete(cli);
                    cout << "删除学生信息" << endl; /* 删除学生信息 */
                    break;
                case '4':
                    flag = 0;
                    stu.Mysql_Updata(cli);
                    cout << "修改学生信息" << endl; /* 修改学生信息 */
                    break;
                case '5':
                    flag = 0;
                    stu.Mysql_FindOne(cli);
                    cout << "查找学生信息" << endl; /* 查找学生信息 */
                    break;
                case '6':
                    cout << "删除所有学生信息" << endl;
                    break;
                case '0':
                    close(cli);
                    return NULL;
                    break;
                default:
                    cout << "输入有误，请重新输入" << endl;
                    break;
                }
            }
        }
        else
        {
            continue;
        }
    }
}

int main(int argc, char *argv[])
{
    int sockfd;
    int port;
    int ret;
    int cfd;
    string msg;
    pthread_t readthread;
    char str[32] = {0};
    char buf[32] = {0};
    socklen_t len;
    struct sockaddr_in ser, cli;
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // 选择为IPV4家族进行通信
    if (sockfd == -1)
    {
        cerr << "socket is fail: " << endl;
        exit(0);
    }
    port = atoi(argv[1]);                     // 把字符串转为整数
    ser.sin_family = AF_INET;                 // IPV4地址家族
    ser.sin_port = htons(port);               // 端口号
    ser.sin_addr.s_addr = inet_addr(argv[2]); // ip地址
    ret = bind(sockfd, (struct sockaddr *)&ser, sizeof(struct sockaddr_in));
    if (ret == -1)
    {
        cerr << "bind is fail: " << endl;
        exit(0);
    }
    ret = listen(sockfd, 5); // 监听等待的客户端
    if (ret == -1)
    {
        cerr << "listen is fail: " << endl;
    }
    len = sizeof(cli);
    Mysql_Connect();
    while (1)
    {
        cfd = accept(sockfd, (struct sockaddr *)&cli, &len);
        if (cfd == -1)
        {
            cerr << "accept is fail: " << endl;
            exit(0);
        }
        cout << "有新的客户端连接: " << cfd << endl;
        client[cnt] = cfd;

        /*    信息提示          */
        msg = "+-----------------------------------------------+\n";
        msg += "|              学生学籍信息管理系统             |\n";
        msg += "+-----------------------------------------------+\n";
        msg += "|   1、显示学生信息    |  4、删除学生信息       |\n";
        msg += "|   2、添加学生信息    |  5、查找学生信息       |\n";
        msg += "|   3、删除学生信息    |  6、                   |\n";
        msg += "+-----------------------------------------------+\n";
        msg += "|                  0、退出                      |\n";
        msg += "+-----------------------------------------------+\n";
        write(client[cnt], msg.c_str(), msg.length());

        flag = 1;

        pthread_create(&readthread, NULL, Read_Data, &client[cnt]);
        cnt++;
    }

    return 0;
}

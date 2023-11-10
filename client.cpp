#include <iostream>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
using namespace std;

/**
 * @brief Cli_Read()函数，用于读取从服务器发送的数据
 * @param arg 是一个指向套接字描述符的指针
 * @return 无返回值
 */
void *Cli_Read(void *arg)
{
    int ret;                  // 用于保存 read 函数的返回值
    int sockfd = *(int *)arg; // 从参数中获取套接字描述符
    char buf[1024] = {0};     // 定义缓冲区，用于保存读取的数据

    // 主循环，不断读取数据
    while (1)
    {
        // 从套接字中读取数据，保存到缓冲区
        ret = read(sockfd, buf, sizeof(buf));

        // 如果读取到了数据
        if (ret > 0)
        {
            cout << buf << flush; // 输出读取到的数据
            usleep(1000);         // 等待一段时间
            memset(buf, 0, 1024); // 清空缓冲区，准备下一次读取
        }
    }
}

/**
 * @brief 主函数，用于创建客户端并与服务器进行通信
 * @param argc 命令行参数的数量
 * @param argv 命令行参数的数组，argv[1] 是服务器的端口号，argv[2] 是服务器的 IP 地址
 * @return 程序执行状态，0 表示正常退出
 */
int main(int argc, char *argv[])
{
    int sockfd;             // 套接字描述符
    int port;               // 服务器端口号
    int ret;                // 用于保存函数返回值
    pthread_t thread01;     // 读取数据的线程
    string re;              // 用于保存读取的字符串
    char str[64];           // 用于保存输入的字符串
    char buf[32] = {0};     // 缓冲区
    struct sockaddr_in cli; // 服务器地址结构体

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // 如果创建套接字失败，输出错误信息并退出程序
    if (sockfd == -1)
    {
        cerr << "socket is fail: " << endl;
        exit(0);
    }

    port = atoi(argv[1]); // 从命令行参数中获取服务器端口号

    // 设置服务器地址结构体
    cli.sin_family = AF_INET;
    cli.sin_port = htons(port);
    cli.sin_addr.s_addr = inet_addr(argv[2]);

    // 连接到服务器
    ret = connect(sockfd, (struct sockaddr *)&cli, sizeof(struct sockaddr_in));
    // 如果连接失败，输出错误信息并退出程序
    if (ret == -1)
    {
        cerr << "connect is fail: " << endl;
        exit(0);
    }

    // 创建读取数据的线程
    pthread_create(&thread01, NULL, Cli_Read, &sockfd);

    // 主循环，读取用户输入并发送到服务器
    while (1)
    {
        cin >> str;                      // 读取用户输入
        write(sockfd, str, strlen(str)); // 发送用户输入到服务器
        memset(str, '\0', 64);           // 清空输入字符串，准备下一次输入
    }
    return 0;
}
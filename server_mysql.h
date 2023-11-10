#ifndef _SQL_H_
#define _SQL_H_

#include <iostream>
#include <mysql.h>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
using namespace std;

bool Mysql_Connect(void);
bool Mysql_Find(int cfd);

extern int flag;

class student
{
private:
    // 添加数据
    char name[64];
    char stuname[64];
    int age;
    char gender[64];
    int chinese;
    int math;
    int english;
    // 删除数据
    char DeleData[64];
    // 修改数据
    char ChangeFlag;
    char ChangeData[64];
    // 查找某一个
    char findflag;
    char finddata[64];

public:
    bool Mysql_Add(int cfd);
    bool Mysql_Dete(int cfd);
    bool Mysql_Updata(int cfd);
    bool Mysql_FindOne(int cfd);
};

#endif
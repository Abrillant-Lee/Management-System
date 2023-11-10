#include "server_mysql.h"

MYSQL mysql;                                                        // MySQL连接对象
const char *server = "127.0.0.1";                                   // MySQL服务器的地址
const char *user = "root";                                          // 用于连接MySQL服务器的用户名
const char *password = "root";                                      // 用于连接MySQL服务器的密码
const char *database = "student";                                   // 要连接的MySQL数据库的名称
const char *INIT_ERROR_MSG = "init is error";                       // 初始化MySQL对象失败时的错误消息
const char *CONNECT_ERROR_MSG = "mysql_real_connect(): ";           // 连接到MySQL服务器失败时的错误消息
const char *CONNECT_SUCCESS_MSG = "Connected MySQL successful!";    // 连接到MySQL服务器成功时的消息
const char *CREATE_TABLE_ERROR_MSG = "mysql_real_query(): ";        // 执行SQL查询失败时的错误消息
const char *CREATE_TABLE_SUCCESS_MSG = "create tables is success!"; // 创建表成功时的消息

// 创建用户表的SQL语句
string createUsersTableSQL()
{
    // 返回创建表的SQL语句
    return "CREATE TABLE IF NOT EXISTS users("
           "name varchar(100), "          // 用户名字段，最大长度100
           "studentnumber varchar(100), " // 学生编号字段，最大长度100
           "age int(50), "                // 年龄字段，整数类型
           "gender varchar(50), "         // 性别字段，最大长度50
           "身高 int(20), "               // 身高字段，整数类型
           "体重 int(20), "               // 体重字段，整数类型
           "鞋码 int(20)"                 // 脚码字段，整数类型
                                          //    "手机号 varchar(100), "        // 手机号字段，最大长度100
                                          //    "邮箱 varchar(100) "           // 邮箱字段，最大长度100
           ");";                          // 结束创建表的SQL语句
}

// 连接到MySQL数据库的函数
bool Mysql_Connect(void)
{
    MYSQL *mysql_init_result;               // 初始化MySQL对象的结果
    int query_result;                       // 执行SQL查询的结果
    string sql;                             // SQL查询字符串
    mysql_init_result = mysql_init(&mysql); // 初始化MySQL对象
    // 如果初始化失败，打印错误消息并返回false
    if (mysql_init_result == NULL)
    {
        cout << INIT_ERROR_MSG << mysql_error(&mysql) << endl;
        return false;
    }
    // 尝试连接到MySQL服务器
    if (mysql_real_connect(&mysql, server, user, password, database, 3306, NULL, 0) == NULL)
    {
        // 如果连接失败，打印错误消息并返回false
        cout << CONNECT_ERROR_MSG << mysql_error(&mysql) << endl;
        return false;
    }
    cout << CONNECT_SUCCESS_MSG << endl;                                // 如果连接成功，打印成功消息
    sql = createUsersTableSQL();                                        // 创建用户表的SQL查询
    query_result = mysql_real_query(&mysql, sql.c_str(), sql.length()); // 执行SQL查询
    // 如果查询失败，打印错误消息并返回false
    if (query_result != 0)
    {
        cout << CREATE_TABLE_ERROR_MSG << mysql_error(&mysql) << endl;
        return false;
    }
    cout << CREATE_TABLE_SUCCESS_MSG << endl;
    // 如果所有操作都成功，返回true
    return true;
}

// 执行MySQL查询并获取结果
MYSQL_RES *execute_query_and_get_result(MYSQL *mysql, const string &query)
{
    // 执行查询
    int rec = mysql_real_query(mysql, query.c_str(), query.length());
    if (rec != 0)
    {
        // 如果查询失败，抛出异常
        throw runtime_error(mysql_error(mysql));
    }
    // 获取查询结果
    MYSQL_RES *res = mysql_store_result(mysql);
    if (res == NULL)
    {
        // 如果获取结果失败，抛出异常
        throw runtime_error(mysql_error(mysql));
    }
    return res;
}

// 查找所有
bool Mysql_Find(int cfd)
{
    MYSQL_RES *res;          // 定义MySQL结果集指针
    int i = 0;               // 定义循环变量
    MYSQL_FIELD *fields;     // 定义MySQL字段指针
    string sql;              // 定义SQL查询语句字符串
    my_ulonglong rows;       // 定义行数变量
    MYSQL_ROW row;           // 定义MySQL行指针
    unsigned int num_fields; // 定义字段数量变量
    // SQL查询语句
    sql = "SELECT * FROM users";
    try
    {
        // 执行查询并获取结果
        res = execute_query_and_get_result(&mysql, sql);
    }
    catch (const runtime_error &e)
    {
        // 如果查询失败，输出错误信息并返回false
        cout << "mysql_real_query(): " << e.what() << endl;
        return false;
    }

    // 获取字段数量
    num_fields = mysql_num_fields(res);
    cout << num_fields << endl;
    string fie;
    // 获取字段信息
    fields = mysql_fetch_fields(res);
    for (i = 0; i < num_fields; i++)
    {
        // 将字段名添加到字符串中
        fie += fields[i].name;
        fie += "\t";
    }
    fie += "\n";
    // 将字段名发送给客户端
    write(cfd, fie.c_str(), fie.length());
    // 获取行数
    rows = mysql_num_rows(res);
    cout << rows << endl;
    usleep(1000);
    // 查询返回结果集中的数
    string str;
    while ((row = mysql_fetch_row(res)))
    {
        for (i = 0; i < num_fields; i++)
        {
            // 将每个字段的值添加到字符串中
            str += row[i];
            str += "\t";
        }
        str += "\n";
        // 将结果发送给客户端
        write(cfd, str.c_str(), str.length());
        str.clear();
    }
    return true;
}

// 添加数据
bool student::Mysql_Add(int cfd)
{
    int ret = 0;         // 用于存储read函数的返回值
    string msg;          // 用于存储提示信息
    int rec;             // 用于存储mysql_real_query的返回值
    ostringstream oss;   // 用于构造SQL查询语句
    char data[64] = {0}; // 用于临时存储从客户端读取的数据

    cout << "flag000" << flag << endl;

    msg = "请输入学生信息:\n";             // 提示信息
    write(cfd, msg.c_str(), msg.length()); // 将提示信息发送给客户端
    msg = "姓名:";
    write(cfd, msg.c_str(), msg.length());
    ret = read(cfd, name, sizeof(name)); // 从客户端读取姓名
    cout << strlen(name) << name << cfd << endl;

    msg = "学号:";
    write(cfd, msg.c_str(), msg.length());
    ret = read(cfd, stuname, sizeof(stuname)); // 从客户端读取学号
    cout << sizeof(stuname) << stuname << cfd << endl;

    msg = "年龄:";
    write(cfd, msg.c_str(), msg.length());
    read(cfd, data, sizeof(data)); // 从客户端读取年龄
    age = atoi(data);              // 将年龄转换为整数
    cout << sizeof(int) << age << cfd << endl;

    msg = "性别:";
    write(cfd, msg.c_str(), msg.length());
    read(cfd, gender, sizeof(gender)); // 从客户端读取性别
    cout << sizeof(gender) << gender << cfd << endl;

    msg = "身高:";
    write(cfd, msg.c_str(), msg.length());
    read(cfd, data, sizeof(data)); // 从客户端读取身高
    chinese = atoi(data);          // 将身高转换为整数
    cout << sizeof(chinese) << "  " << chinese << cfd << endl;

    msg = "体重:";
    write(cfd, msg.c_str(), msg.length());
    read(cfd, data, sizeof(data)); // 从客户端读取体重
    math = atoi(data);             // 将体重转换为整数
    cout << sizeof(math) << "  " << math << cfd << endl;

    msg = "脚码:";
    write(cfd, msg.c_str(), msg.length());
    read(cfd, data, sizeof(data)); // 从客户端读取脚码
    english = atoi(data);          // 将脚码转换为整数

    char str[1024]; // 用于存储SQL查询语句

    // 构造SQL查询语句
    sprintf(str, "insert into users(name,studentnumber,age,gender,身高,体重,脚码) values( \
        '%s','%s',%d,'%s',%d,%d,%d);",
            name, stuname, age, gender, chinese, math, english);

    rec = mysql_real_query(&mysql, str, strlen(str)); // 执行SQL查询
    if (rec != 0)                                     // 如果mysql_real_query返回非零值，表示查询失败
    {
        cout << "mysql_real_query(): " << mysql_error(&mysql) << endl; // 输出错误信息
        return -1;                                                     // 返回-1表示添加失败
    }
    cout << "添加数据成功" << endl; // 输出成功信息

    flag = 1; // 设置标志位

    return true; // 返回true表示添加成功
}

// 删除数据
bool student::Mysql_Dete(int cfd)
{
    char str[128] = {0};                   // 用于存储SQL查询语句
    int rec;                               // 用于存储mysql_real_query的返回值
    string sql;                            // 用于存储提示信息
    sql = "请输入你想要删除学生的学号: ";  // 提示信息
    write(cfd, sql.c_str(), sql.length()); // 将提示信息发送给客户端

    read(cfd, DeleData, sizeof(DeleData));                                   // 从客户端读取学生学号
    sprintf(str, "delete from users where studentnumber = '%s';", DeleData); // 构造SQL查询语句

    rec = mysql_real_query(&mysql, str, strlen(str)); // 执行SQL查询
    if (rec != 0)                                     // 如果mysql_real_query返回非零值，表示查询失败
    {
        cout << "mysql_real_query(): " << mysql_error(&mysql) << endl; // 输出错误信息
        return false;                                                  // 返回false表示删除失败
    }
    cout << "delete msg is success" << endl; // 输出成功信息
    flag = 1;                                // 设置标志位

    return true; // 返回true表示删除成功
}

// 修改
bool student::Mysql_Updata(int cfd)
{
    string sql;
    char buffer[32] = {0};
    char str[256] = {0};
    int rec;
    MYSQL_RES *res;
    my_ulonglong rows;
    string msg;
again:
    sql = "请输入你想要修改学生的学号: ";
    // sql = "请输入你要修改的数据: \n[1]姓名 [2]学号 [3]年龄 [4]性别 \n[5]语文成绩 [6]体重成绩 [7]脚码\n:";
    write(cfd, sql.c_str(), sql.length());

    read(cfd, ChangeData, sizeof(ChangeData));
    if (memcmp(ChangeData, "exit", 4) == 0)
    {
        msg = "+-----------------------------------------------+\n";
        msg += "|              学生学籍信息管理系统             |\n";
        msg += "+-----------------------------------------------+\n";
        msg += "|   1、显示学生信息    |  4、删除学生信息       |\n";
        msg += "|   2、添加学生信息    |  5、查找学生信息       |\n";
        msg += "|   3、删除学生信息    |  6、                   |\n";
        msg += "+-----------------------------------------------+\n";
        msg += "|                  0、退出                      |\n";
        msg += "+-----------------------------------------------+\n";
        write(cfd, msg.c_str(), msg.length());
        flag = 1;
        return false;
    }
    sprintf(str, "select * from users where studentnumber = '%s';", ChangeData);
    rec = mysql_real_query(&mysql, str, strlen(str));
    if (rec != 0)
    {
        cout << "mysql_real_query(): " << mysql_error(&mysql) << endl;
        return false;
    }

    res = mysql_store_result(&mysql);
    if (NULL == res)
    {
        cout << "mysql_store_result(): " << mysql_error(&mysql) << endl;
        return false;
    }

    rows = mysql_num_rows(res);
    cout << rows << endl;
    if (rows == 0)
    {
        sql = "未查到此学号的学生 请重新输入\n";
        write(cfd, sql.c_str(), sql.length());
        flag = 1;
        goto again;
    }

    sql = "请输入你要修改的数据: \n[1]姓名 [2]学号 [3]年龄 [4]性别 \n[5]身高 [6]体重 [7]脚码\n:";
    write(cfd, sql.c_str(), sql.length());

    // read(cfd, &ChangeFlag, sizeof(&ChangeFlag));
    read(cfd, &ChangeFlag, sizeof(ChangeFlag));
    switch (ChangeFlag)
    {
    case '1':
        sql = "请输入你想要修改的姓名: ";
        write(cfd, sql.c_str(), sql.length());
        memset(buffer, 0, 128);
        read(cfd, buffer, sizeof(buffer));
        sprintf(str, "update users set name='%s' where studentnumber='%s';", buffer, ChangeData);
        rec = mysql_real_query(&mysql, str, strlen(str));
        if (rec != 0)
        {
            cout << "mysql_real_query(): " << mysql_error(&mysql) << endl;
            return false;
        }
        write(cfd, "修改姓名成功\n", sizeof("修改姓名成功"));
        break;
    case '2':
        sql = "请输入你想要修改的学号: ";
        write(cfd, sql.c_str(), sql.length());
        memset(buffer, 0, 128);
        read(cfd, buffer, sizeof(buffer));
        sprintf(str, "update users set studentnumber='%s' where studentnumber='%s';", buffer, ChangeData);
        rec = mysql_real_query(&mysql, str, strlen(str));
        if (rec != 0)
        {
            cout << "mysql_real_query(): " << mysql_error(&mysql) << endl;
            return false;
        }
        write(cfd, "修改学号成功\n", sizeof("修改学号成功"));
        break;
    case '3':
        sql = "请输入你想要修改的年龄: ";
        write(cfd, sql.c_str(), sql.length());
        memset(buffer, 0, 128);
        read(cfd, buffer, sizeof(buffer));
        sprintf(str, "update users set age='%s' where studentnumber='%s';", buffer, ChangeData);
        rec = mysql_real_query(&mysql, str, strlen(str));
        if (rec != 0)
        {
            cout << "mysql_real_query(): " << mysql_error(&mysql) << endl;
            return false;
        }
        write(cfd, "修改年龄成功\n", sizeof("修改年龄成功"));
        break;
    case '4':
        sql = "请输入你想要修改的性别: ";
        write(cfd, sql.c_str(), sql.length());
        memset(buffer, 0, 128);
        read(cfd, buffer, sizeof(buffer));
        sprintf(str, "update users set gender='%s' where studentnumber='%s';", buffer, ChangeData);
        rec = mysql_real_query(&mysql, str, strlen(str));
        if (rec != 0)
        {
            cout << "mysql_real_query(): " << mysql_error(&mysql) << endl;
            return false;
        }
        write(cfd, "修改性别成功\n", sizeof("修改性别成功"));
        break;
    case '5':
        sql = "请输入你想要修改的身高: ";
        write(cfd, sql.c_str(), sql.length());
        memset(buffer, 0, 128);
        read(cfd, buffer, sizeof(buffer));
        sprintf(str, "update users set 身高='%s' where studentnumber='%s';", buffer, ChangeData);
        rec = mysql_real_query(&mysql, str, strlen(str));
        if (rec != 0)
        {
            cout << "mysql_real_query(): " << mysql_error(&mysql) << endl;
            return false;
        }
        write(cfd, "修改身高成功\n", sizeof("修改身高成功"));
        break;
    case '6':
        sql = "请输入你想要修改的体重: ";
        write(cfd, sql.c_str(), sql.length());
        memset(buffer, 0, 128);
        read(cfd, buffer, sizeof(buffer));
        sprintf(str, "update users set 体重='%s' where studentnumber='%s';", buffer, ChangeData);
        rec = mysql_real_query(&mysql, str, strlen(str));
        if (rec != 0)
        {
            cout << "mysql_real_query(): " << mysql_error(&mysql) << endl;
            return false;
        }
        write(cfd, "修改体重成功\n", sizeof("修改体重成功"));
        break;
    case '7':
        sql = "请输入你想要修改的脚码: ";
        write(cfd, sql.c_str(), sql.length());
        memset(buffer, 0, 128);
        read(cfd, buffer, sizeof(buffer));
        sprintf(str, "update users set 英语='%s' where studentnumber='%s';", buffer, ChangeData);
        rec = mysql_real_query(&mysql, str, strlen(str));
        if (rec != 0)
        {
            cout << "mysql_real_query(): " << mysql_error(&mysql) << endl;
            return false;
        }
        write(cfd, "修改脚码成功\n", sizeof("修改脚码成功"));
        break;
    default:
        sql = "输入有误请重新输入:";
        write(cfd, sql.c_str(), sql.length());
        goto again;
        break;
    }

    flag = 1;
    return true;
}

// 查找某一个
bool student::Mysql_FindOne(int cfd)
{
    int i = 0;
    MYSQL_RES *res;
    MYSQL_FIELD *fields;
    string sql;
    my_ulonglong rows;
    MYSQL_ROW row;
    unsigned int num_fields;
    int rec;
    char str[128] = {0};
    string msg;

lable:
    sql = "请输入你要通过学号查找还是姓名查找：[1]姓名查找 [2]学号查找 [0]退出";
    write(cfd, sql.c_str(), sql.length());

    // read(cfd, &findflag, sizeof(&findflag));
    read(cfd, &findflag, sizeof(findflag));
    if (findflag == '1')
    {
        sql = "请输入你想要查找的姓名: ";
        write(cfd, sql.c_str(), sql.length());
        memset(finddata, 0, 64);
        read(cfd, finddata, sizeof(finddata));

        sprintf(str, "SELECT * FROM users where name = '%s';", finddata);
        rec = mysql_real_query(&mysql, str, sizeof(str));
        if (rec != 0)
        {
            cout << "mysql_real_query(): " << mysql_error(&mysql) << endl;
            return false;
        }
    }
    else if (findflag == '2')
    {
        sql = "请输入你想要查找的学号: ";
        write(cfd, sql.c_str(), sql.length());
        memset(finddata, 0, 64);
        read(cfd, finddata, sizeof(finddata));
        memset(str, 0, 128);
        sprintf(str, "SELECT * FROM users where studentnumber = '%s';", finddata);
        rec = mysql_real_query(&mysql, str, sizeof(str));
        if (rec != 0)
        {
            cout << "mysql_real_query(): " << mysql_error(&mysql) << endl;
            return false;
        }
    }
    else if (findflag == '0')
    {
        msg = "+-----------------------------------------------+\n";
        msg += "|              学生学籍信息管理系统             |\n";
        msg += "+-----------------------------------------------+\n";
        msg += "|   1、显示学生信息    |  4、修改学生信息       |\n";
        msg += "|   2、添加学生信息    |  5、查找学生信息       |\n";
        msg += "|   3、删除学生信息    |  6、                   |\n";
        msg += "+-----------------------------------------------+\n";
        msg += "|                  0、退出                      |\n";
        msg += "+-----------------------------------------------+\n";
        write(cfd, msg.c_str(), msg.length());
        flag = 1;
        return false;
    }
    res = mysql_store_result(&mysql);
    if (NULL == res)
    {
        printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
        return false;
    }

    num_fields = mysql_num_fields(res);
    cout << num_fields << endl;
    string fie;
    fields = mysql_fetch_fields(res);
    for (i = 0; i < num_fields; i++)
    {
        strcat(fields[i].name, "\t");
        fie += fields[i].name;
    }
    fie += "\n";
    write(cfd, fie.c_str(), fie.length());

    rows = mysql_num_rows(res);
    cout << rows << endl;
    if (rows == 0)
    {
        sql = "未查到此学生 请重新输入\n";
        write(cfd, sql.c_str(), sql.length());
        flag = 1;
        goto lable;
    }
    usleep(1000);

    // 查询返回结果集中的数
    string result, buf;
    while ((row = mysql_fetch_row(res)))
    {
        for (i = 0; i < num_fields; i++)
        {
            result += row[i];
            result += "\t";
        }
        result += "\n";
        write(cfd, result.c_str(), result.length());
        result.clear();
    }

    flag = 1;
    return true;
}

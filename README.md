# 项目简介
- C++学生信息管理系统，首先客户端需要登录到服务器，登录到服务器之后可以对学生的信息进行增删改查（学号，姓名，成绩，专业……）

# 如何使用该项目
编译指令：

- 客户端：```g++ ./client.cpp -o client```<br>

- 服务器：```g++ ./server_manage.cpp ./server_mysql.cpp -o server -lpthread -I /usr/include/mysql -L/usr/lib/mysql -lmysqlclient```

# 一、环境配置

## Linux系统

- Linux服务器（华为弹性云服务器、阿里云......)

- VMware虚拟机安装linux系统（网上搜索教程安装配置）

- WSL（windows下的linux子系统）

## Mysql环境配置
安装Mysql:`sudo apt install mysql-server`<br>



### cpp环境
























//为啥突然想搞业务层，因为业务会变复杂。并且我发现原本服务器层很多私有方法/属性都被线程池层访问了
//所以加一个业务层 来访问数据

/*
* 这里把注册和登录的业务梳理清楚
* 如果是一个刚刚连接上的客户端，那么他可以选择注册或登录(选哪个由他自己决定)
* 如果是注册，他发过来的账号，用户名(当前只有用户名嘞) 去数据库检查一下，如果没有这个名字就注册成功，并添加进数据库。
* 提醒客户端注册成功并发送新报文来登录
*   有就失败并发送失败报文(于是乎别忘了还有写失败报文(封装成接口))  
*  
* 如果是登录，他发过来账号和姓名也要数据库校验一下，成功就发送广播通知xxx登录成功(已实现)，失败就发送给他错误报文
* 数据库表细节确定表结构和报文结构再说
*/
#pragma once
#include"../../../myhead.h"

#include"myProto.h"

class Server;

class Service
{
    private:
    //数据库

    Server&server; //需要网络层的接口
    //为什么用引用，因为拷贝不了锁属性

    std::unordered_map<std::string,int>cliname_fd;  //维护用户名对应的通信套接字(同时也表明登录状态)
    //啊，因为这个哈希表是可读可写的，是临界资源，所以必须加锁
    std::mutex mutex_cli;

    std::unordered_map<int,std::string>fd_cliname;  //反向--通过fd找到套接字信息，便于直接查键
    //具体应用：客户端下线，server那里只知道fd，我要在service写接口删掉他的信息，首先通过fd查这个哈希表
    //找到名字后再删上面的表和这张表(fd已经失效了) 而注册信息持久化保存在数据库上的
    std::mutex mutex_fd;

    void handleLogin(int sockfd,MSG&msg_struct);
    void handleChat(int sockfd,MSG&msg_struct);
    void handleQuit(int sockfd,MSG&msg_struct);  //quit是服务器主动发送退出消息，server那里是掉线，由server通知
    void handleRegister(int sockfd,MSG&msg_struct); //登录才加入哈希表，注册只是注册用户信息
    void broadcast(MSG&msg_struct,int flag);
    public:
    Service(Server&s); //跟数据库相关
    ~Service();
   

    //补充方法-->
    /*
    Q1:最大的问题还是客户端套接字地址是私有的，访问不了，是让服务器层加接口还是把socket融合到一起
    服务器加接口，服务器架构必须遵循分层解耦。
    网络层（服务器层）只负责底层字节流的传输、连接管理（fd 的增删）和 epoll事件驱动；业务层只负责处理“消息的含义”

    Q2:怎么加，直接返回哈希表吗?
    no,首先最明显的问题就是业务层可以修改哈希表了,这不行
    再来，遍历的时候可能fd被删除，fd突然失效了。 如果加锁，但是调用了服务器层的方法，会导致死锁
    最后职责清晰：业务层只需要知道发给谁，发什么，怎么发，怎么发具体的细节不需要考虑



    最后:接口的意义在于：

让错误的代码根本写不出来

业务层不可能误删 map

业务层不可能遍历时踩到迭代器失效

业务层不需要知道锁的存在

业务层不需要知道 fd 是怎么管理的
    */
    

    // void broadcast(); //广播：给所有的客户端发送消息 (默认)

     //消息转发(不需要构造新报文，只需要找到目标用户名发给它就行了)
     void MessageForward(MSG&msg_struct);

     //提供修改容器的接口
    void delLoginuser(int fd);
};








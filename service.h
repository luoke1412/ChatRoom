//为啥突然想搞业务层，因为业务会变复杂。并且我发现原本服务器层很多私有方法/属性都被线程池层访问了
//所以加一个业务层 来访问数据
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

    std::unordered_map<std::string,int>cliname_fd;  //维护用户名对应的通信套接字
    //啊，因为这个哈希表是可读可写的，是临界资源，所以必须加锁
    std::mutex mutex;

    public:
    Service(Server&s); //跟数据库相关
    ~Service();
    void handleLogin(int sockfd,MSG&msg_struct);
    void handleChat(int sockfd,MSG&msg_struct);
    void handleQuit(int sockfd,MSG&msg_struct);

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
};








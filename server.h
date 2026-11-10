//处理主逻辑：初始化并启动服务器，epoll架构处理客户端请求
//通信的细节都属于网络层
#pragma once
#include"../../../myhead.h"
//#include"threadpool.h"
#define SER_IP "172.17.165.43"
#define SER_PORT 9005
//1024

class threadPool;

struct MSG;

class Server
{
    private:
    //与通信相关
    int sfd;
    sockaddr_in sin;
    socklen_t slen;
    
    //定义一个标志表示是否启动
    bool running;  

    //epoll相关
    int epfd;  //epoll树的句柄
    epoll_event evs[1024]; //定义存储触发事件信息的容器(一次可能有多个事件触发)

    //sockaddr_in cli_sockaddr[1024];  //存储客户端套接字地址，下标表示通信用的套接字，默认值是？
    //映射 fd<--->sockaddr，不应该用数组，动态数组也不合适，原因有三：
    //1. fd随时都有可能删除，是不连续的，vector作为连续内存，可能浪费空间
    //2. 删除复杂度是O(n)
    //3. fd 会复用，vector 无法区分“有效/无效
    std::unordered_map<long long,sockaddr_in>cli_sockaddr;
    //很明显，可读可写，是临界资源，需要保护
    std::mutex mutex;

    void startServer(); //初始化服务器


    //处理依赖倒置的接口(不暴露业务层细节)
    std::function<void(int)> disconnect;
public:
    void setDisconnect(std::function<void(int)>func);

    Server();
    ~Server();
    void run(threadPool&pool); //启动服务器(epoll架构)

    //给业务层提供的接口, 广播是网络层的天然职责
   // void broadcast(MSG&msg_struct,int flag); //广播：给所有的客户端发送消息 (默认)
//已移动到业务层，理由：这里的广播是广播给所有连接上的客户端，而不是已经登录的客户端
//强行保留网络层需要关心谁谁谁登录没，不符合单一职责。   而且在聊天室广播和网络层面的广播不是一回事


    // 添加到业务层
    // 其实最好是要给报文消息加一个目的套接字字段，于是就可以单独转发给某个人
    // void MessageForward(MSG&msg_struct);


    //因为涉及到处理粘包和半包，封装一个发送和接收的接口(因为不关注发给谁，发什么内容，只管发送本身，可以留)
    void send_msg(int sockfd,const char*buf);

    //void recv_msg()  用不上
};







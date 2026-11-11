//线程池模块：负责工作线程的创建与销毁，任务队列的管理（入队/出队），以及基于互斥锁和条件变量的线程同步
#pragma once
#include"../../../myhead.h"
//#include"service.h"
#include<atomic>
#include"myProto.h"

class Service;   // 前向声明


//线程池
class threadPool
{
private:

    //std::vector<std::function<void()>>threadset; //线程容器

    //把 lambda等可调用对象直接传给了 std::thread的构造函数，在 vector 里原地构造了一个线程对象
    std::vector<std::thread>threadset;  //存储线程本身，而不是线程体。这样主线程才能删除线程池
    std::queue<std::function<void()>>Task;  //任务队列
    std::atomic_bool run;  //标识线程池是否启动，默认不启动

    //与同步互斥相关  
    std::mutex mutex;  //要保护临界区时加锁
    std::condition_variable cv;  //在条件满足的时候加锁
    //这里总结哪些是临界资源(写写/读写) :Task,run


    /*私有方法：对外暴露一个接口*/
    void creat_Thread(int num);  //创建n个线程  

    // //具体方法--->交给业务层处理
    // void handleLogin(int sockfd);
    // void handleChat(int sockfd);
    // void handleQuit(int sockfd);

    // //补充方法
    // void broadcast(); //广播：给所有的客户端发送消息 (默认)

    // //其实最好是要给报文消息加一个目的套接字字段，于是就可以单独转发给某个人
    // void MessageForward();


    //业务处理接口(有锁属性存在，不能拷贝，只能引用！)
    Service& service;

public:
    threadPool(int num,Service&s); //构造
    ~threadPool();  //析构
    // void handleTask(char*buf,int newfd,sockaddr_in* cli_sockaddr);//从队列里取任务来执行
    //不用了,addtask逻辑已经写好了

    void addTask(int sockfd,std::string msg_string);  //封装任务，添加任务，并且要唤醒一个线程来干活

    //void startPool(int num); //启动线程池。 在构造的时候启动就行了
};
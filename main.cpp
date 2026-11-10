//g++ main.cpp myProto.cpp server.cpp service.cpp threadpool.cpp -pthread
#include"threadpool.h"
#include"server.h"
#include"myProto.h"
#include"service.h"
using namespace std;
//遇到的时候再改：除了接收以及socket通信要用，其他情况一律用string
//1.总结哪些是临界资源
//2.加数据库，自己构造网络报文
//3.画出架构图
int main()
{
    try
    {//看属性和构造函数，找依赖关系就行了

       //数据层初始化

       //初始化网络层
       Server server;
        //初始化业务
        Service service(server);
        server.setDisconnect([&service](int fd){ //初始化回调函数
            service.delLoginuser(fd);
        }); 

       //初始化线程池
       threadPool pool(100,service);
       
       //一切的入口
       server.run(pool);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    



/*被server瓜分了*/
    // //1.创建套接字
    // int sfd=socket(AF_INET,SOCK_STREAM,0);
    // if(sfd==-1) 
    // {
    //     perror("socket error");
    //     return -1;
    // }
    // //2.绑定地址
    // sockaddr_in sin;
    // sin.sin_family=AF_INET;
    // sin.sin_port=htons(SER_PORT);
    // sin.sin_addr.s_addr=inet_addr(SER_IP);
    // if(bind(sfd,(const sockaddr*)&sin,sizeof(sin))==-1)
    // {
    //     perror("bind error");
    //     return -1;
    // } 

    // //3.开启监听
    // if(listen(sfd,128)==-1)
    // {
    //     perror("listen error");
    //     return -1;
    // }

    // /**************线程池部分*************/
    // threadPool pool(5);

    // /*************epoll部分**********/
    // //epoll树的句柄,创建一棵epoll树
    // int epfd=epoll_create(1);
    // if(epfd==-1)
    // {
    //     perror("epoll_create error");
    //     return -1;
    // }

    // //创建epoll实例，把其加入到epoll树中
    // struct epoll_event ev;
    // ev.events=EPOLLIN;  //检查读事件
    // ev.data.fd=sfd;

    // epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&ev);

    // while(1)
    // {
    //     //定义存储事件信息的容器
    //     //epoll_event evs;  可能有多个事件触发，应该用结构体的
       
    //     epoll_event evs[128];

    //     //定义存储对方套接字的容器，不然找不到
    //     sockaddr_in cli_sockaddr[128];
    //     //阻塞等待事件发生,且返回事件数量
    //     int num=epoll_wait(epfd,evs,128,-1);
    //     if(num==-1)
    //     {
    //         perror("epoll_wait error");
    //         break;
    //     }
    //     //说明有事件发生了，遍历容器
    //     for(int i=0;i<num;i++)
    //     {
    //         int evfd=evs[i].data.fd;
    //         if(evfd==sfd) //说明是连接请求 
    //         {
    //              sockaddr_in sin;
    //              socklen_t len=sizeof(sin);
    //             int newfd=accept(sfd,(sockaddr*)&sin,&len); //暂时不接收客户端信息
    //             if(newfd==-1)
    //             {
    //                 perror("accept error");
    //                 break;
    //             }
    //             //存储套接字信息
    //             cli_sockaddr[newfd]=sin;

    //             //把新的套接字注册到epoll树上
    //              struct epoll_event ev;
    //             ev.events=EPOLLIN;  //检查读事件
    //             ev.data.fd=newfd;

    //             epoll_ctl(epfd,EPOLL_CTL_ADD,newfd,&ev);
    //         }

    //         //其他都是负责通信的事件(也就是真实的业务场景)
    //         /*
    //         else 
    //         {
    //             char buf[128]; //存储接收的容器
    //             int num=recv(evfd,buf,sizeof(buf),0);
    //             if(num==0) //应该是断开连接了
    //             {
    //                 //先把epoll树注册的给删掉
    //                 epoll_ctl(epfd,EPOLL_CTL_DEL,evfd,&ev);
    //                 printf("[%d]节点销毁",evfd);
    //                 close(evfd);
    //             }
    //             else if(num==-1)
    //             {
    //                 perror("epoll_ctl error");
    //                 break;
    //             }

    //             buf[num]=0;
    //             printf("收到数据：%s\n",buf);

    //             //发送数据过去，发现不知道对方套接字信息......
    //             //accept时是获取到了的，提前存储
    //             if(send(evfd,buf,sizeof(buf),0)==-1)
    //             {
    //                 perror("send error");
    //                 break;
    //             }

    //             printf("已发送\n");
    //             //省了
    //             // char ip[128]=cli_sockaddr[]
    //             // printf("已发送给[%d,%d]",  
    //         }
    //             */

    //         //我当时觉得没用到线程池，是因为没有定义任务，如果客户端是有要处理的
    //         //任务的，让主线程来处理肯定不妥，会拖慢整个处理事件的进度
    //         //所以这里的做法是把消息+套接字等等信息打包成任务让线程池处理
            
    //         else{   //处理任务
    //            //1.主线程先接收
    //            char buf[128];
    //             if(num==0) //应该是断开连接了
    //             {
    //                 //先把epoll树注册的给删掉
    //                 epoll_ctl(epfd,EPOLL_CTL_DEL,evfd,&ev);
    //                 printf("[%d]节点销毁",evfd);
    //                 close(evfd);
    //             }
    //             else if(num==-1)
    //             {
    //                 perror("epoll_ctl error");
    //                 break;
    //             }

    //             buf[num]=0;
    //             //2.主线程先接收消息，然后把消息，evs,套接字一块传进去
    //             pool.handleTask(buf,epfd,cli_sockaddr);
    //         }
    //     }
    // }
    
    // close(epfd);
    // close(sfd);
    //
}

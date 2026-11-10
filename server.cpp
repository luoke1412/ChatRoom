#include"server.h"
#include"threadpool.h"
void Server::startServer()//初始化服务器
{
   //1.创建套接字
    sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd==-1) 
    {
        perror("socket error");
        return;
    }
    //2.绑定地址
    //sockaddr_in sin;
    sin.sin_family=AF_INET;
    sin.sin_port=htons(SER_PORT);
    sin.sin_addr.s_addr=inet_addr(SER_IP);
    slen=sizeof(sin);
    if(bind(sfd,(const sockaddr*)&sin,slen)==-1)
    {
        perror("bind error");
        return;
    } 

    //设置地址重用
    int reuse=1;
    if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))==-1)
    {
        perror("setsockopt erorr");
        return;
    }

    //3.开启监听
    if(listen(sfd,128)==-1)
    {
        perror("listen error");
        return ;
    }
}
Server::Server()
{
    startServer();
}
Server::~Server()
{
    //关闭之前其实可以给所有客户端发消息先发消息
    close(sfd);
    close(epfd);
}
void Server::run(threadPool&pool) //启动服务器(epoll架构)
{
    running=1;
    
    /*************epoll部分**********/
    //epoll树的句柄,创建一棵epoll树
    epfd=epoll_create(1);
    if(epfd==-1)
    {
        perror("epoll_create error");
        return ;
    }

    //创建epoll实例，把其加入到epoll树中
    struct epoll_event ev;
    ev.events=EPOLLIN;  //检查读事件
    ev.data.fd=sfd;

    epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&ev);


    while(running) //但是感觉没啥用
    {
        //定义存储事件信息的容器
        //epoll_event evs;  可能有多个事件触发，应该用结构体的
       
        //epoll_event evs[128];

        //定义存储对方套接字的容器，不然找不到
        //sockaddr_in cli_sockaddr[128];
        //阻塞等待事件发生,且返回事件数量
        int num=epoll_wait(epfd,evs,128,-1);
        if(num==-1)
        {
            perror("epoll_wait error");
            break;
        }
        //说明有事件发生了，遍历容器
        for(int i=0;i<num;i++)
        {
            int evfd=evs[i].data.fd;
            if(evfd==sfd) //说明是连接请求 
            {
                 sockaddr_in cin;
                 socklen_t len=sizeof(cin);
                int newfd=accept(sfd,(sockaddr*)&cin,&len); 
                if(newfd==-1)
                {
                    perror("accept error");
                    break;
                }
                {
                    std::lock_guard<std::mutex>lock(mutex);

                    //存储套接字信息
                    cli_sockaddr[newfd]=cin;
                }
                
                

                //把新的套接字注册到epoll树上
                 struct epoll_event ev;
                ev.events=EPOLLIN; 
                ev.data.fd=newfd;

                epoll_ctl(epfd,EPOLL_CTL_ADD,newfd,&ev);
            }

            //其他都是负责通信的事件(也就是真实的业务场景)
            /*
            else 
            {
                char buf[128]; //存储接收的容器
                int num=recv(evfd,buf,sizeof(buf),0);
                if(num==0) //应该是断开连接了
                {
                    //先把epoll树注册的给删掉
                    epoll_ctl(epfd,EPOLL_CTL_DEL,evfd,&ev);
                    printf("[%d]节点销毁",evfd);
                    close(evfd);
                }
                else if(num==-1)
                {
                    perror("epoll_ctl error");
                    break;
                }

                buf[num]=0;
                printf("收到数据：%s\n",buf);

                //发送数据过去，发现不知道对方套接字信息......
                //accept时是获取到了的，提前存储
                if(send(evfd,buf,sizeof(buf),0)==-1)
                {
                    perror("send error");
                    break;
                }

                printf("已发送\n");
                //省了
                // char ip[128]=cli_sockaddr[]
                // printf("已发送给[%d,%d]",  
            }
                */

            //我当时觉得没用到线程池，是因为没有定义任务，如果客户端是有要处理的
            //任务的，让主线程来处理肯定不妥，会拖慢整个处理事件的进度
            //所以这里的做法是把消息+套接字等等信息打包成任务让线程池处理
            
            else{   //处理任务
               //1.主线程先接收消息
                char buf[MAX_MSG_SIZE];
                int ret=0;
                do
                {
                    ret+=recv(evfd,buf+ret,sizeof(buf)-ret,0);
                    if(ret==0) //这是在接收缓冲区收到了EOF，说明断开了
                    {
                        //先把epoll树注册的给删掉
                        epoll_ctl(epfd,EPOLL_CTL_DEL,evfd,&ev);
                        printf("[%d]节点销毁",evfd);

                        {
                            std::lock_guard<std::mutex>lock(mutex);
                            cli_sockaddr.erase(evfd); // 删除客户端套接字地址信息
                        }
                        //删除业务层维护的登录注册表
                        
                        close(evfd);
                        break;
                    }
                    else if(ret==-1)
                    {
                        perror("epoll_ctl error");
                        break;
                    }
                } while (ret!=MAX_MSG_SIZE);
                 
                
                //2.然后添加到任务队列里面(注意把文件描述符传进去)
                if(ret!=0&&ret!=-1)pool.addTask(evfd,buf);

            }
        }
    }
}


void Server::broadcast(MSG&msg_struct,int flag) //广播：给所有的客户端发送消息
{
    //构造新的消息报文
    struct MSG newmsg;
    msg_struct.type=flag;
    strcpy(newmsg.myname,msg_struct.myname);
    strcpy(newmsg.toname,msg_struct.toname);
    newmsg.settimeStamp();
    switch(flag)
    {
        case LOGIN:
            sprintf(newmsg.text,"%s登录成功!\n",newmsg.myname);
            break;
        
        case QUIT:
            sprintf(newmsg.text,"%s退出成功!\n",newmsg.myname);
            break;
        
        case CHAT:
            strcpy(newmsg.text,msg_struct.text); //单纯的发送消息
            break;
    }

    //先序列化再发
    std::string newmsg_string=newmsg.serialize(); 
    
    {
        std::lock_guard<std::mutex>lock(mutex);
        
        for(auto&c:cli_sockaddr)  //c就相当于 *it
        {
            //套接字句柄
            int sockfd=c.first;
            //注意：只发给已经登录进去的用户，保存登录信息的容器在业务层

            //发送消息
            send_msg(sockfd,newmsg_string.c_str());
           
        }
    }
    
}


 void Server::send_msg(int sockfd,const char*buf)
{
    int ret=0;
    do
    {
        ret+=send(sockfd,buf+ret,MAX_MSG_SIZE-ret,0);
        if(ret==-1)
        {
            perror("send error");
            break;
        }
    }while(ret!=MAX_MSG_SIZE);
}

// TODO [阶段0→1] 字节流/变长协议: 切非阻塞+ET时, 每变长字段加 4 字节长度头,
//   替换现在的定长 MAX_MSG_SIZE 塞满方式(当前 4 字节总长度头已删)
// TODO [阶段0→2] recv 收包: 非阻塞 fd 下 ret==-1 需区分 EAGAIN(继续收) 与真错误(断连),
//   且非阻塞无阻塞点, do-while 收满循环要改成"收一段就回 epoll_wait"
//   (当前阻塞 fd 下 ret==0=EOF、收满才交任务, 是对的)

#include"service.h"
#include"server.h"  //要用到发送消息和广播的接口
//处理登录
void Service::handleLogin(int sockfd,MSG&msg_struct)
{   
      std::string myname(msg_struct.myname);
      //1.查询数据库
      //select
      bool res=1;
      if(res) //如果成功
      {
          //2维护哈希表
        {
          std::lock_guard<std::mutex>lock(mutex_cli);
          cliname_fd[myname]=sockfd;
        }
      
          //3.广播消息
        server.broadcast(msg_struct,LOGIN);    
      }

      else{ //失败处理，构造失败报文
          MSG errbuf;
          errbuf.type=ERROR;
          strcpy(errbuf.text,"未查询到账号信息，请重新登录");
          server.send_msg(sockfd,errbuf.serialize().c_str());
      }
      
}
void Service::handleChat(int sockfd,MSG&msg_struct)
{
      std::string myname(msg_struct.myname);
      std::string msg_string=msg_struct.serialize();
    //1.更新数据库

    //2.查询哈希表并转发
    {
        std::lock_guard<std::mutex>lock(mutex_cli);
        if(cliname_fd.find(myname)!=cliname_fd.end())
        {
            int fd=cliname_fd[myname];
            //发送
            server.send_msg(fd,msg_string.c_str());
            //perror
        }
    }
    //发送也放在锁里面，因为fd本身也是临界资源
}
void Service::handleQuit(int sockfd,MSG&msg_struct)
{
    std::string myname(msg_struct.myname);
      //1.查询和更新数据库
      
      //2更新状态
      
      //3.广播消息
    server.broadcast(msg_struct,QUIT); 
}


 void Service::handleRegister(int sockfd,MSG&msg_struct)
 {
     std::string myname(msg_struct.myname);
      //1.查询数据库
      //select
      bool res=1;
      if(res) //如果成功
      {
          //2维护哈希表
        {
          std::lock_guard<std::mutex>lock(mutex_cli);
          cliname_fd[myname]=sockfd;
        }
      
          //3.广播消息
        server.broadcast(msg_struct,LOGIN);    
      }

      else{ //失败处理，构造失败报文
          MSG errbuf;
          errbuf.type=ERROR;
          strcpy(errbuf.text,"未查询到账号信息，请重新登录");
          server.send_msg(sockfd,errbuf.serialize().c_str());
      }
 }


void Service::MessageForward(MSG&msg_struct)
{
    //现在的问题是怎么找，怎么通过用户名找到对应的客户端套接字地址或者通信套接字

    //思路一是数据库：但是数据库只存MSG里面的信息，而且更适合持久化数据。fd是动态的
    //那么只能再加一个通信套接字 <--映射--> 用户名。 在解析出用户名的时候(业务层)添加

   std::string toname(msg_struct.toname,strlen(msg_struct.toname));

   if(cliname_fd.find(toname)!=cliname_fd.end()) 
   {
        int sockfd=cliname_fd[toname];
        //序列化后发送
        std::string msg_string=msg_struct.serialize();

        server.send_msg(sockfd,msg_string.c_str());
   }
}

Service::Service(Server&s):server(s){}


Service::~Service()
{

}

void Service::updateLoginuser(int fd)
{
    if(fd_cliname.find(fd)==fd_cliname.end()) return;

    if(cliname_fd.find(fd_cliname[fd])!=cliname_fd.end())
    cliname_fd.erase(fd_cliname[fd]); //相当快
    fd_cliname.erase(fd);
}
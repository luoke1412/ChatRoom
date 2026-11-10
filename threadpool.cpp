#include"threadpool.h"
#include"service.h"  //破除循环引用 
    void threadPool::addTask(int sockfd,char*msg_c)  //添加任务，并且要唤醒一个
    {
        
        {
            //临界资源先保护
            std::lock_guard<std::mutex>lock(mutex);

            //构造任务(同样lambda)
            //子线程必须要拿到任务体，还有通信用的套接字
            Task.push([msg_c,this,sockfd](){  //无参！
            //解析消息，先转化为string
            std::string msg_string(msg_c,MAX_MSG_SIZE);

            struct MSG msg_struct;
            msg_struct.reserialize(msg_string);

            //至此，解析成功
            //查看消息类型
            switch(msg_struct.type)
            {

                case REGISTER:
                    service.handleRegister(sockfd,msg_struct);
                    break;

                case LOGIN:
                    service.handleLogin(sockfd,msg_struct);
                    break;

                case CHAT:
                    service.handleChat(sockfd,msg_struct);
                    break;

                case QUIT:
                    service.handleQuit(sockfd,msg_struct);
                    break;
                
                default:
                    std::cout<<"非法消息类型!\n";
                    break;

            }
            //不用循环，一次消息就是一个任务，子线程执行完了就进入下一个任务
            //这是由单reactor每次请求都会由主线程接收引起的
        });
        }
        
        //唤醒其中一个线程
        cv.notify_one();

    }
    //更推荐原地构造传入lambda
    void threadPool::creat_Thread(int num)  //创建n个线程  
    {
        for(int i=0;i<num;i++)
        {
            // //原地构造存储线程任务体， 用lambda书写线程体内容(函数名不重要)
            // threadset.emplace_back([this](){ //捕获当前类
            //     //正文：如果有空闲任务就抢锁，抢到了就工作，没抢到就睡觉，等机会去~
            //     while(1)
            //     {
            //         {
                        
            //         }

            //         //handleTask
            //     }
            // });
            
            std::function<void()> func=[this]{ //捕获当前类
                 //正文：如果有任务就抢锁，抢到了就工作，没抢到就睡觉，等机会去~
                 //no,不管如何先抢锁，如果没抢到任务或者本来没任务，先释放锁，再去睡觉
                 //所以应该用条件变量+互斥锁
                 while(true)
                 {
                     std::unique_lock<std::mutex>lock(mutex);

                     cv.wait(lock,[this](){
                        return Task.size();  //如果为0就阻塞，并且内部封装了while循环
                     });

                     //说明抢到锁了
                     //特殊情况：被主线程唤醒了并且当前没任务，直接退出
                     if(run==false&&Task.empty()) return;

                     //正常执行任务
                     std::function<void()> func2=Task.front();

                     lock.unlock(); //解锁，因为已经不会访问临界资源了
                     
                     //执行任务，直接调用就好了
                     func2();
                 }
            };  

            threadset.emplace_back(func);   //真正创建子线程(或者用lambda表达式传入)
        }
    }
    //具体方法
    // void handleTask(char*buf,int newfd,sockaddr_in* cli_sockaddr) 
    // {


    // }
    // void threadPool::handleLogin(int sockfd)
    // {

    // }
    // void threadPool::handleChat(int sockfd)
    // {

    // }
    // void threadPool::handleQuit(int sockfd)
    // {

    // }

    threadPool::threadPool(int num,Service&s):service(s) //构造
    {
        //创建线程池
        creat_Thread(num);
        //启动线程池
        run=true;
    }
    threadPool::~threadPool()  //析构
    {
        // {
        //     //抢锁
        //     std::lock_guard<std::mutex>lock(mutex);
        //     run=1;
        // }
        // //自动解锁

        //改成原子属性了
        run=false;
        //唤醒所有的线程，然后阻塞回收线程资源
        cv.notify_all();

        for(int i=0;i<threadset.size();i++)
        {
            std::cout<<"线程"<<threadset[i].get_id()<<"准备退出\n";
            threadset[i].join();
        }

    }

    // void threadPool::startPool() //启动线程池
    // {
    //     run=true;
    //     printf("pool run!\n");
    // }
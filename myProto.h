//我要把它封装成真正的应用层协议报文，不是用结构体
//那么就要解决：怎么分包 / 粘包？怎么跨机器兼容？怎么知道收到了完整的一包？
#pragma once
#include"../../../myhead.h"
#define LOGIN 1
#define CHAT 2
#define QUIT 3
#define REGISTER 4 //注册功能
#define ERROR 5  //错误报文


#define NAME 128 //定义最大名称：如果客户端输入超过了，会提示重新输入
#define TEXT 128 //定义最大消息输入量：如果客户端输入超过了，会提示重新输入
#define TIME 128
#define MAX_MSG_SIZE NAME+TEXT+2*sizeof(int)+TIME

//网络聊天室--->任务消息的三种类型，对应三种不同的处理 

//封装一个消息类型
struct MSG
{
    //int body_len; //正文实际长度：
    int type;  //消息类型
    //设置成定长，方便解析
    char myname[NAME];  //如果是聊天室，没有ID那怎么行
    char toname[NAME];  //目标用户的ID，为"ALL"则为广播(用ID来指代客户端，上层不需要知道对方的IP)
    char text[TEXT];  //消息正文
    char timestamp[TIME]; // 发送时间戳 类似格式：[xxx年-xxx月-xxx日 xxx时:xxx分:xxx秒]

     //这里再补充两个方法，用于提高传输效率
    //序列化和反序列化：
    //定义： 序列化是将对象转换为可存储或传输的字节流或文本格式（如 JSON、二进制），
    //反序列化则是将这些数据恢复为原始对象的过程

    //序列化：将结构体转化为字符串
    std::string serialize()const; 

     //反序列化：将string字符串恢复为结构体：接收之后用
    void reserialize(std::string msg_string);

    void settimeStamp();  //将时间转化为字符串
};
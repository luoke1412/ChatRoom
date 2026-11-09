#include"myProto.h"
//我序列化定死一个包长度是MAX_MSG_SIZE。 循环读取不需要长度字段就可以避免半包和粘包，
//如果是变长(哪怕我在序列化把temp.append(myname,NAME)改成 temp.append(myname,strlen(myname));
//那也是变长，反序列化的时候我不知道该读多少长度)
//序列化
std::string MSG::serialize() const
{
    std::string temp="";
    //string& append(const char* s, size_t n);  const char* s:要读数据的起始地址  n:读的字节大小
    //temp.append(reinterpret_cast<const char*>(&body_len),sizeof(type));
    //<const char*>(body_len)   假设body_len=123不加地址相当于直接去0x123读内容...
    //取了body_len的首地址，然后读字节的方式改成了一步一个字节
   
    //单测发现，加之前要把小端变大端
    //std::cout<<"变之前："<<type;
    int type_=htonl(type);
    temp.append(reinterpret_cast<const char*>(&type_),sizeof(type));
    temp.append(myname,NAME);
    temp.append(toname,NAME);  //定长，虽然有所浪费，但是一定不会出错
    temp.append(text,TEXT);
    temp.append(timestamp,TIME);
    return temp;
}

//反序列化
void MSG::reserialize(std::string msg_string)
{
    //一步一步解析
    int off=0;
    // memcpy(&body_len,msg_string.c_str()+off,sizeof(int));
    // off+=sizeof(int);
    int type_;
    memcpy(&type_,msg_string.c_str()+off,sizeof(int));
    //大端变小端
    
    type=ntohl(type_);
    //std::cout<<"  变之后："<<type;
    off+=sizeof(int);
    memcpy(myname,msg_string.c_str()+off,NAME);
    off+=NAME;
     memcpy(toname,msg_string.c_str()+off,NAME);
    off+=NAME;
     memcpy(text,msg_string.c_str()+off,TEXT);
    off+=TEXT;
     memcpy(timestamp,msg_string.c_str()+off,TIME);
}
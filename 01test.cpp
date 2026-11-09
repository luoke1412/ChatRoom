#include<iostream>
#include<stdio.h>
#include"myProto.h"
using namespace std;
int main(int argc, const char *argv[])
{
    MSG msg{2000,"abc","bbb","hello,world","[2025 12 16 11:22:37]"};
    string temp=msg.serialize();
    //cout<<temp<<endl;
    MSG msg2;
    msg2.reserialize(temp);
    cout<<msg2.type<<msg.myname<<msg.toname<<msg.text<<msg.timestamp;
    return 0;  
}
#include<iostream>
#include"net/TcpServer.hpp"

using namespace MindbniM;
int main()
{
    LOG_ROOT_ADD_STDOUT_APPEND_DEFAULT();
    TcpServer ser(8848);
    ser.SetMessageCallBack([](TcpConnect::ptr con)
    {
        std::string message=con->ReAlltoBody();
        std::cout<<message<<std::endl;
        sleep(3);
    });
    ser.start();
    return 0;
}

#include<iostream>
#include"server/Reactor.hpp"

using namespace MindbniM;
int main()
{
    Reactor server;
    Connect::ptr lis=std::make_shared<Listen>(8848);
    server.AddConnect(lis);
    server.start();
    return 0;
}
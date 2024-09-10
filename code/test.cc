//#include<log/log.hpp>
//#include<unistd.h>
#include<iostream>
#include"buff/buffer.hpp"

using namespace MindbniM;
int main()
{
    Buffer buff;
    std::string b="123456";
    buff.push_back(b.begin(),b.end());
    std::cout<<buff<<std::endl;;
    return 0;
}
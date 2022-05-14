#include <iostream>
#include <string>
#include <chrono>

void fun1(std::string str)
{
    std::string s(std::move(str));
}

void fun2(const std::string& str)
{
    std::string s(str);
}


int main()
{
    const int N = 10000000;
    
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i)
    {
        fun1("qwertyuiopasdfghjklzxcvbnmkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk");
    }
    auto delta1 = std::chrono::steady_clock::now() - start;
    std::cout << "delta1 = " << delta1.count() << std::endl;

    start = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i)
    {
        fun2("qwertyuiopasdfghjklzxcvbnmkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk");
    }
    auto delta2 = std::chrono::steady_clock::now() - start;
    std::cout << "delta2 = " << delta2.count() << std::endl;

    return 0;
}
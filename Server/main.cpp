#include "pch.h"
#include "IOExecutor.h"
#include "PacketBase.hpp"
#include "MCWorld.h"

int main()
{
    if (false == Mgr(IOExecutor)->InitServer("8888"))
    {
        std::cout << "서버 초기화 실패" << std::endl;
        return 1;
    }
  
    Mgr(MCWorld)->Init();

    std::thread io_thread{ []() {Mgr(IOExecutor)->IORoutine(); } };

    std::string exit_word;
    for (;;)
    {
        std::cin >> exit_word;
        if ("EXIT" == exit_word)
        {
            Mgr(IOExecutor)->EndServer();
            break;
        }
    }
    io_thread.join();
}
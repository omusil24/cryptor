#include "Core.h"

#include <thread>

 struct Test {
    void func(int x, const string& a) {}
    void runt()
    {
         std::thread t(&Test::func, this, 42, "");
          t.join();
    }
};

int main(int argc, char** argv)
{    
    Core c;
    return c.run(argc, argv);
}

#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

class Detect{
    public:
        Detect(){}
        ~Detect(){}
        int Init()
        {
            std::thread p(&Detect::Process, this);
            p.detach();
            return 0;
        }

        int Process()
        {
            while(1) { 
                std::unique_lock<std::mutex> lk(m);
                cv.wait(lk,[this]{return prepared;});
                if (isEmpty) {
                    std::cout << typeid(this).name() << "\033[31m::" << __FUNCTION__ << " empty\033[0m" << std::endl;
                    exit(1);
                } else {
                    sleep(1);
                    std::cout << typeid(this).name() << "\033[32m::" << __FUNCTION__ << " full\033[0m" << std::endl;
                }
                // isEmpty = 0;
                prepared = false;
                lk.unlock();
                cv.notify_one();
            }
            return 0;
        }
        
        int Input()
        {
            while(!prepared) { 
                std::unique_lock<std::mutex> lk(m);
                cv.wait(lk,[this]{return !prepared;});
                isEmpty = ((rand() % 77) == 0);
                if (isEmpty) {
                    std::cout << typeid(this).name() << "\033[31m::" << __FUNCTION__ << " empty\033[0m" << std::endl;
                } else {
                    std::cout << typeid(this).name() << "\033[32m::" << __FUNCTION__ << " full\033[0m" << std::endl;
                }
                prepared = true;
                lk.unlock();
                cv.notify_one();
            }
            return 0;
        }
        
    private:
        std::mutex m;
        std::condition_variable cv;
        bool prepared;
        bool isEmpty;
};

int main(int argc, char** argv)
{
    Detect d;
    d.Init();
    while(1) {
        d.Input();
        //std::cout << "\033[33m:::::::::::::::::::::::::::::::::::\033[0m" << std::endl;
    }
    return 0;
}

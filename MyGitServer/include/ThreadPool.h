#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
using namespace std;

class ThreadPool {
    vector<thread>workers;//线程池
    queue<function<void()>> tasks;//任务队列，存放待执行的任务（具体函数）
    mutex task_mutex;//任务队列锁
    condition_variable cv;//条件变量，用于通知线程池中有任务需要执行
    bool stop;//线程池是否停止
public:
    ThreadPool(int num_threads);
    ~ThreadPool();

    //利用泛型函数写法，添加任务（具体函数）到任务队列中，此处只能在函数定义的时候写出具体实现，不能分离，否则无法找到定义
    template<class F,class... Args>
    auto enqueue(F&& f, Args&&... args)->std::future<typename std::result_of<F(Args...)>::type>{
        using return_type=typename std::result_of<F(Args...)>::type;

        auto task=make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f),std::forward<Args>(args)...)
        );

        future<return_type>result=task->get_future();

        {
            unique_lock<mutex>lock(task_mutex);
            if(stop){
                throw runtime_error("ThreadPool is stopped");
            }

            tasks.emplace([task](){(*task)();});
        }
        cv.notify_one();
        return result;
    }
    
    void stop_workers();
};





#endif // _THREADPOOL_H_
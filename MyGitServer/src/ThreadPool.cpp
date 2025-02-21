#include "ThreadPool.h"

ThreadPool::ThreadPool(int num_threads):stop(false){
    for(int i=0;i<num_threads;++i){
        workers.emplace_back(thread([this](){
            while(true){
                function<void()> task;
                {
                    unique_lock<mutex>lock(task_mutex);
                    this->cv.wait(lock,[this]{
                        return this->stop||!this->tasks.empty();
                    });
                    if(this->stop&&this->tasks.empty())return;
                    task=move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        }));
    }
}

ThreadPool::~ThreadPool(){
    {
        unique_lock<mutex>lock(task_mutex);
        stop=true;
    }
    cv.notify_all();
    for(auto&worker:workers){
        worker.join();
    }
}


void ThreadPool::stop_workers(){
    stop=true;
    cv.notify_all();
}

// template<class F, class... Args>
// auto ThreadPool::enqueue(F&& f, Args&&... args)->future<typename std::result_of<F(Args...)>::type>{
//     using return_type=typename std::result_of<F(Args...)>::type;

//     auto task=make_shared<std::packaged_task<return_type()>>(
//         std::bind(std::forward<F>(f),std::forward<Args>(args)...)
//     );

//     future<return_type>result=task.get_future();

//     {
//         unique_lock<mutex>lock(task_mutex);
//         if(stop){
//             throw runtime_error("ThreadPool is stopped");
//         }

//         tasks.emplace([task](){(*task)();});
//     }
//     cv.notify_one();
//     return result;
// }
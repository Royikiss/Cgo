/*************************************************************************
	> File Name: Cgo-ThreadPool.h
	> Author:Royi 
	> Mail:royi990001@gmail.com 
	> Created Time: Tue 30 Jul 2024 11:45:54 PM CST
	> Describe: 
************************************************************************/
#ifndef _THREAD_POLL_H__
#define _THREAD_POLL_H__

#define __NAMESPACE_Cgo_BEGIN__ namespace Cgo {
#define __NAMESPACE_Cgo_END__  }  

#include <queue>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

__NAMESPACE_Cgo_BEGIN__

/*************************************************************************
* > Class Name: task
* > Father class: none
* > Describe: Standardize binding of different functions and the different
            parameters (including types and numbers) required by the 
            function as tasks to be executed in a thread pool.
 ************************************************************************/
class task {
    using self = task;
    using func_t = std::function<void()>;

public:

    /*************************************************************************
    * > class task 
    * > name : constructor
    ************************************************************************/
    template <typename FUNC_T, typename ...ARGS>
    task(FUNC_T func, ARGS... args) {
        this->_func = std::bind(func, std::forward<ARGS> (args)...);
    }

    /*************************************************************************
    * > class ~task 
    * > name : destructor
     ************************************************************************/
    ~task() = default;

    /*************************************************************************
    * > class task 
    * > name : run 
    * > Describe: : run the task
     ************************************************************************/
    void run() {
        this->_func();
        return ;
    }

private:
    func_t _func;
};

/*************************************************************************
* > Class Name: thread_pool
* > Father class: none
* > Describe: Standardize the thread pool, which can be used to execute
 ************************************************************************/
class thread_pool {

    using task_t = Cgo::task;
    using mutex_t = std::mutex;
    using cond_t = std::condition_variable;
    using running_t = std::unordered_map<std::thread::id, bool>;
    using task_queue_t = std::queue<Cgo::task *>;
    using thread_ptrs_t = std::vector<std::thread *>;

private:
    
    mutex_t m_mutex;                                        // mutex for queue
    cond_t m_cond;                                          // condition variable for queue
    bool state;                                             // thread_pool state
    int _thread_num;                                        // number of threads
    thread_ptrs_t _threads;                                 // pointers of threads
    running_t _running;                                     // thread running state
    task_queue_t _tasks;                                    // task queue

    /*************************************************************************
    * > Function Name: stop_running
    * > From class: thread_pool
    * > Describe: poisom method
     ************************************************************************/
    void _stop_running() {
        auto id = std::this_thread::get_id();
        this->_running[id] = false;
        return ;
    }

    /*************************************************************************
    * > Function Name: get_task
    * > From class: thread_pool
    * > Describe: get task from task queue
     ************************************************************************/
    task_t *get_task() {
        std::unique_lock<std::mutex> locker(m_mutex); while (_tasks.empty()) {
            m_cond.wait(locker);
        }
        Cgo::task *t = _tasks.front();
        _tasks.pop();
        return t;
    }

public:

    /*************************************************************************
    * > class thread_pool 
    * > name : constructor 
    * > Describe: : init information about thread_pool
     ************************************************************************/
    thread_pool(int thread_num = 1) : _thread_num(thread_num), _threads(thread_num), state(false)   {
        this->start();
        return ;
    }

    /*************************************************************************
    * > class thread_pool 
    * > name : destructor 
    * > Describe: : stop all task and release all resource
     ************************************************************************/
    ~thread_pool() {
        this->stop();
        while (!_tasks.empty()) {
            delete _tasks.front();
            _tasks.pop();
        }
        return ;
    }


    /*************************************************************************
    * > class thread_pool 
    * > name : start 
    * > Describe: : make all thread start to work
     ************************************************************************/
    void start() {
        if (state == true) return ;
        for (int i = 0; i < _thread_num; ++i) {
            _threads[i] = new std::thread(&thread_pool::worker, this);
        }
        state = true;
        return ;
    }

    /*************************************************************************
    * > Function Name: worker
    * > From class: thread_pool
    * > Describe: make all thread start to work
     ************************************************************************/
    void worker() {
        auto id = std::this_thread::get_id();
        _running[id] = true;
        while (_running[id]) {
            Cgo::task *t = get_task();
            t->run();
            delete t;
        }
        return ;
    }


    /*************************************************************************
    * > Function Name: add_task
    * > From class: thread_pool
    * > Describe: add task to thread_pool
     ************************************************************************/
    template <typename FUNC_T, typename ...ARGS>
    void add_task(FUNC_T func, ARGS... args) {
        std::unique_lock<std::mutex> locker(m_mutex);
        _tasks.push(new task(func, std::forward<ARGS> (args)...));
        locker.unlock();
        m_cond.notify_one();
        return ;
    }

    /*************************************************************************
    * > Function Name: get_thread_num
    * > From class: thread_pool
    * > Describe: get thread number
     ************************************************************************/
    int get_thread_num() {
        return this->_thread_num;
    }

    /*************************************************************************
    * > Function Name: stop
    * > From class: thread_pool
    * > Describe: stop all task and release all resource
     ************************************************************************/
    void stop() {
        if (state == false) return ;
        for (int i = 0; i < _thread_num; ++i) {
            this->add_task(&thread_pool::_stop_running, this);
        }
        for (int i = 0; i < _thread_num; ++i) {
            _threads[i]->join();
        }
        for (int i = 0; i < _thread_num; ++i) {
            delete _threads[i];
            _threads[i] = nullptr;
        }
        state = false;
        return ;
    }
};

__NAMESPACE_Cgo_END__

// DATE: 2024-08-03
// FILENAME: thread_pool.h
// AUTHOR: royi
// END:

#endif


//
//  ThreadWorker.cpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/3.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#include "ThreadWorker.hpp"
#include <iostream>

using namespace ylq;
ThreadWorker::ThreadWorker():m_stop(false),m_worker([&](){
    while(!m_stop)
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock,[&](){
                return m_stop|haveWork();
            });
            if(m_stop&&(!haveWork()))
                break;
        }
        work();
    }
}){}

void ThreadWorker::work()
{
    //add your code
    std::cout<<"TheadWorker::work()"<<std::endl;
}
bool ThreadWorker::haveWork()
{
    //default there no work
    return false;
}

void ThreadWorker::beginWork()
{
    m_cv.notify_one();
}



ThreadWorker::~ThreadWorker()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop=true;
    }
    m_cv.notify_all();
    if(m_worker.joinable())
        m_worker.join();
}

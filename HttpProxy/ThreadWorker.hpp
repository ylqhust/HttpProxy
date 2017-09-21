//
//  ThreadWorker.hpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/3.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#ifndef ThreadWorker_hpp
#define ThreadWorker_hpp


#include <mutex>
#include <thread>
#include <condition_variable>

namespace ylq {
    class ThreadWorker
    {
    public:
        ThreadWorker();
        virtual ~ThreadWorker();
    protected:
        void beginWork();
        virtual void work();
        virtual bool haveWork();
        std::mutex m_mutex;
        bool m_stop;
    private:
        std::condition_variable m_cv;
        std::thread m_worker;
    };
}
#endif /* ThreadWorker_hpp */

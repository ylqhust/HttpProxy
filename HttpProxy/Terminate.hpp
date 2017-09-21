//
//  Terminate.hpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/4.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#ifndef Terminate_hpp
#define Terminate_hpp
#include "ThreadWorker.hpp"

namespace ylq {
    class Terminate : public ThreadWorker
    {
    public:
        static Terminate &getInstance(void (*f)());
        void start();
    protected:
        void work();
        bool haveWork();
    private:
        Terminate(void (*f)());
        void (*terminate)();
        bool dontwork;
    };
}

#endif /* Terminate_hpp */

//
//  Terminate.cpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/4.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#include "Terminate.hpp"
#include "Utils.h"
#include <sys/select.h>
#include <stdlib.h>


using namespace ylq;

Terminate::Terminate(void (*f)()):terminate(f),dontwork(false){}

void Terminate::work()
{
    Utils<1>::log(1, "Terminate begin work.\n");
    fd_set readfds;
    timeval timeout;
    int ret;
    char buff[100];
    
    while(true)
    {
        __DARWIN_FD_ZERO(&readfds);
        __DARWIN_FD_SET(0, &readfds);
        timeout.tv_sec=4;
        timeout.tv_usec=0;
        ret=select(1, &readfds, nullptr, nullptr, &timeout);
        if(ret==0)
        {
            Utils<1>::log(1, "No input in command line.\n");
            continue;
        }
        else if(ret==-1)
        {
            Utils<1>::log(1, "some error in command line happend.\n");
            exit(-1);
        }
        scanf("%s",buff);
        Utils<1>::log(1, "read from command line %s\n",buff);
        if(strcmp(buff, "exit")==0)
            break;
    }
    Utils<1>::log(1, "will terminate the process.\n");
    if(terminate!=nullptr)
        terminate();
    dontwork=true;
}


bool Terminate::haveWork()
{
    return !dontwork;
}

Terminate &Terminate::getInstance(void (*f)())
{
    static Terminate instance(f);
    return instance;
}

void Terminate::start()
{
    beginWork();
}

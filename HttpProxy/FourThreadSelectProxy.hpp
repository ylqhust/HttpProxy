//
//  FourThreadSelectProxy.hpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/2.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#ifndef FourThreadSelectProxy_hpp
#define FourThreadSelectProxy_hpp

//4个线程，每个线程中使用select解决连接问题，这是比Proxy.hpp的一种更好的解决方案
//Proxy.hpp中，每一个Proxy只能处理一个连接，即使使用了线程池，
//当线程池中的线程用完了后，这将导致后面到来的连接等待，如果这时候前面的连接速度很慢，将导致很低的吞吐量
//因此必须改进，每一个Proxy不能够只处理一个连接，必须要能够处理多个连接。

#include <condition_variable>
#include <thread>
#include <string>
#include <mutex>
#include <deque>
#include "Proxy.hpp"
#include "ThreadWorker.hpp"
#include <atomic>


namespace ylq {
    
    using CLIENT_FD=int;
    using SERVER_FD=int;
    
    class Connector : public Proxy,public ThreadWorker
    {
    public:
        void addClienFD(CLIENT_FD t_client_fd);
    protected:
        void work();
        bool haveWork();
    private:
        std::deque<CLIENT_FD> m_deque;
    };
    
    
    class FTSP_Worker : public Proxy,public ThreadWorker
    {
    public:
        void addCS_FD(CLIENT_FD t_client_fd,SERVER_FD t_server_fd);
    protected:
        void work();
        bool haveWork();
    private:
        std::vector<std::pair<CLIENT_FD, SERVER_FD> > m_fds;
    };
    class FourThreadSelectProxy
    {
    public:
        static FourThreadSelectProxy &getInstance();
        void addProcessRequest(CLIENT_FD t_client_fd,SERVER_FD t_server_fd);
        void addConnectRequest(CLIENT_FD t_client_fd);
    private:
        FTSP_Worker _4works[4];//4个连接处理者
        Connector _16connector[16];//16个连接建立者
        std::atomic<unsigned int> m_nextWorkIndicator;
        std::atomic<unsigned int> m_nextConnectorIndicator;
    };
}

#endif /* FourThreadSelectProxy_hpp */

//
//  Proxy.hpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/1.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#ifndef Proxy_hpp
#define Proxy_hpp

#include "ThreadPool.h"
#include <string>

namespace ylq {
    class Hook;
    class Proxy : public Runnable
    {
    protected:
        int     comWithClient_fd;
        int     comWithServer_fd;
        char    clientBuff[4096];
        char    serverBuff[4096];
        int     cblen;
        int     sblen;
        std::string     host;
        std::string     ip;
        std::string     port;
        
        std::shared_ptr<Hook> m_hook;
        void preprocess();
        void logClientBuff();
        void getHeaderFromClient()throw (std::string);
        void dnsPhase()throw (std::string);
        void connectToServer()throw (std::string);
        void separateHttpsAndHttp()throw (std::string);
        void beginRelay()throw (std::string);
        int sendToClient()throw (std::string);
        int sendToServer()throw (std::string);
        int recvFromClient()throw (std::string);
        int recvFromServer()throw (std::string);
        void manualCloseSocket();
        Proxy();
    public:
        Proxy(int t_client_fd,
              std::shared_ptr<Hook> t_hook=nullptr);
        virtual void operator()();
        virtual ~Proxy();
    };
}


#endif /* Proxy_hpp */

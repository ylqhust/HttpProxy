//
//  HookMT.hpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/4.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#ifndef HookMT_hpp
#define HookMT_hpp


#include "Hook.hpp"
#include <string>

namespace ylq {
    class HookMT : public Hook
    {
        
        std::string getHostIp()throw (std::string);
        void modify(char *buff,int &len);
    public:
        void hookDataSendToServer(const std::string &t_host,const std::string &t_ip,const std::string &t_port,char *buff,int &len);
        void hookDataSendToClient(const std::string &t_host,const std::string &t_ip,const std::string &t_port,char *buff,int &len);
    };
}

#endif /* HookMT_hpp */

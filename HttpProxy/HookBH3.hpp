//
//  HookBH3.hpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/2.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#ifndef HookBH3_hpp
#define HookBH3_hpp

#include "Hook.hpp"
#include <string>

namespace ylq {
    class HookBH3 : public Hook
    {
        
        void modify(char *buff,int &len);
    public:
        std::string getHostIp()throw (std::string);
        void hookDataSendToServer(const std::string &t_host,const std::string &t_ip,const std::string &t_port,char *buff,int &len);
        void hookDataSendToClient(const std::string &t_host,const std::string &t_ip,const std::string &t_port,char *buff,int &len);
    };
}

#endif /* HookBH3_hpp */

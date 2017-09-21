//
//  Hook.hpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/2.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#ifndef Hook_hpp
#define Hook_hpp

#include <string>

namespace ylq {
    class Hook
    {
    public:
        virtual void hookDataSendToServer(const std::string &t_host,const std::string &t_ip,const std::string &t_port,char *buff,int &len);
        virtual void hookDataSendToClient(const std::string &t_host,const std::string &t_ip,const std::string &t_port,char *buff,int &len);
    };
}

#endif /* Hook_hpp */

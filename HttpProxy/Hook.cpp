//
//  Hook.cpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/2.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#include "Hook.hpp"

using namespace ylq;
void Hook::hookDataSendToServer(const std::string &t_host, const std::string &t_ip, const std::string &t_port, char *buff, int &len)
{
    //default do nothing
}

void Hook::hookDataSendToClient(const std::string &t_host, const std::string &t_ip, const std::string &t_port, char *buff, int &len)
{
    //default do nothing
}

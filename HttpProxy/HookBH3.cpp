//
//  HookBH3.cpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/2.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#include "HookBH3.hpp"
#include "Utils.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <regex>
#include <string>
#include <sstream>

using namespace ylq;


/**
 requestURL:
 http://139.224.7.27/query_gameserver?version=1.4.0_gf_ios&uid=35363091&t=1493606194
 
 response:
 
 {
    "account_url":"https://account.mihoyo.com",
    "asset_boundle_url":"https://bundle.bh3.com/asset_bundle/ios01/1.0",
    "ext":
        {
            "is_xxxx":"0",
            "show_version_text":"0"
        },
    "gameserver":{"ip":"139.224.43.40","port":15100},
    "gateway":{"ip":"139.224.43.40","port":15100},
    "msg":"",
    "oaserver_url":"http://139.224.7.27:1080",
    "retcode":0
 }
 
 **/
void HookBH3::hookDataSendToServer(const std::string &t_host, const std::string &t_ip, const std::string &t_port, char *buff, int &len)
{
    Hook::hookDataSendToServer(t_host, t_ip, t_port, buff, len);
    //do nothing
}

void HookBH3::hookDataSendToClient(const std::string &t_host, const std::string &t_ip, const std::string &t_port, char *buff, int &len)
{
    Hook::hookDataSendToClient(t_host, t_ip, t_port, buff, len);
    if(t_ip!=std::string("139.224.7.27"))
        return;
    buff[len]=0;
    Utils<1>::log(1,"find response:\n%s\n",buff);
    modify(buff,len);
}

void HookBH3::modify(char *buff,int &len)
{
    int port=15100;
    std::regex pattern("\"ip\":\"[0-9.]+\",\"port\":[0-9]+");
    std::string str(buff,buff+len);
    std::ostringstream newGameServer;
    newGameServer<<"\"ip\":"<<"\""<<getHostIp()<<"\",\"port\":"<<port;
    std::string newData = std::regex_replace(str, pattern,newGameServer.str());
    std::copy(newData.begin(), newData.end(), buff);
    len=(int)newData.size();
}

std::string HookBH3::getHostIp()throw (std::string)
{
    int sock;
    struct sockaddr_in sin;
    ifreq ifr;
    const char *ethName1="en0";
    const char *ethName2="en1";
    sock=socket(AF_INET, SOCK_STREAM, 0);
    if(sock==-1)
        throw std::string("HookBH3->getHostIp()->socket() error\n");
    
    strncpy(ifr.ifr_name, ethName1, IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1]=0;
    if(ioctl(sock, SIOCGIFADDR,&ifr)==0)
    {
        memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
        return inet_ntoa(sin.sin_addr);
    }
    
    
    strncpy(ifr.ifr_name, ethName2, IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1]=0;
    if(ioctl(sock, SIOCGIFADDR,&ifr)==0)
    {
        memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
        return inet_ntoa(sin.sin_addr);
    }
    throw std::string("HookBH3->getHostIp() no name to local eth\n");
}

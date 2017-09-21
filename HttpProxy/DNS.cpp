//
//  DNS.cpp
//  HttpProxy
//
//  Created by yangliqun on 2017/4/30.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#include "DNS.hpp"
#include <regex>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Utils.h"
#include <thread>
#include <iostream>
#include <sys/select.h>


using namespace ylq;

std::string DNS::googleDnsServer("8.8.8.8");
std::string DNS::tencentDnsServer("115.159.157.26");
std::string DNS::workDnsServer=tencentDnsServer;//default
std::string DNS::IP_FAILED("fuck");

void DNS::getHostAndPort(const std::string &t_httpHeader, std::string &t_host, std::string &t_port)throw (std::string) try
{
    std::regex pattern("Host: ?([^:\r\n]*)");
    std::regex portPattern("[A-Z]{3,} .*:([0-9]{1,5})");
    std::smatch s;
    std::regex_search(t_httpHeader,s,pattern);
    if(s.empty())
        throw "";
    t_host = *(++s.begin());
    std::smatch ss;
    std::regex_search(t_httpHeader, ss, portPattern);
    if(ss.empty())
        t_port="80";
    else
        t_port=*(++ss.begin());
}
catch(...)
{
    throw std::string("DNS::getHostAndPort exception ## ")+t_httpHeader;
}

void DNS::useDnsServer(const std::string &dnsServer)
{
    DNS::workDnsServer=dnsServer;
}
std::string DNS::getIp(const std::string &t_host)
{
    std::regex pattern("[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+");
    if(std::regex_match(t_host.begin(), t_host.end(), pattern))
        return t_host;
    std::vector<char> buff1 = dns_packet::construct(t_host);
    std::vector<char> buff2 = dns_packet::construct(t_host);
    std::vector<char> buff3 = dns_packet::construct(t_host);
    
    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd==-1)
    {
        return DNS::IP_FAILED;
    }
    sockaddr_in dnsServer;
    socklen_t dslen=sizeof(dnsServer);
    dnsServer.sin_family=AF_INET;
    dnsServer.sin_port=htons(53);
    dnsServer.sin_addr.s_addr=inet_addr(workDnsServer.c_str());
    
    
    unsigned char recvbuff[1024];
    sockaddr_in ser;
    socklen_t serlen=sizeof(ser);
    
    fd_set recvset;
    int count=0;
    
    while(count<3) // try 3 times
    {
        sendto(sd, (void *)&buff1[0], buff1.size(), 0, (sockaddr *)&dnsServer, dslen);
        __DARWIN_FD_ZERO(&recvset);
        __DARWIN_FD_SET(sd, &recvset);
        timeval timeout;
        timeout.tv_sec=4;
        timeout.tv_usec=0;
        int ret = select(sd+1, &recvset, nullptr, nullptr, &timeout);
        if(ret==-1)
        {
            Utils<0>::log(0,"DNS select error:");
            return DNS::IP_FAILED;
        }
        else if(ret==0)
        {
            count++;
            continue;
        }
        break;
    }
    if(count==3)
    {
        Utils<0>::log(0,"## DNS::getIp() timeout. %s\n",t_host.c_str());
        return DNS::IP_FAILED;
    }
    
    int len = recvfrom(sd, recvbuff, sizeof(recvbuff), 0, (sockaddr *)&ser, &serlen);
    if(len<=0)
        throw "this should not happend";
    //just select the last 32-bits
    char ip[16];
    sprintf(ip,"%u.%u.%u.%u",recvbuff[len-4],recvbuff[len-3],recvbuff[len-2],recvbuff[len-1]);
    close(sd);
    return ip;
}


dns_packet::dns_packet(const std::string &t_host):dp2(t_host)
{
    bzero(&dp1, sizeof(dns_packet_part1));
    dp1.transactionId=dns_packet::getAId();
    dp1.recursion=1;
    dp1.questions=htons(1);
}
dns_packet_part2::dns_packet_part2(const std::string &t_host)
{
    nameAndtypeAndClass.reserve(1+t_host.size()+1+sizeof(uint16_t)+sizeof(uint16_t));
    
    nameAndtypeAndClass.push_back(0x00);
    auto pos=nameAndtypeAndClass.begin();
    int count=0;
    for(char c:t_host)
    {
        if(c!='.')
        {
            nameAndtypeAndClass.push_back(c);
            count++;
        }
        else
        {
            *pos=count;
            count=0;
            pos=nameAndtypeAndClass.end();
            nameAndtypeAndClass.push_back(0x00);
        }
    }
    *pos=count;
    nameAndtypeAndClass.push_back(0x00);//end of the domain
    nameAndtypeAndClass.push_back(0x00);//type
    nameAndtypeAndClass.push_back(0x01);//type
    nameAndtypeAndClass.push_back(0x00);//class
    nameAndtypeAndClass.push_back(0x01);//class
}



std::vector<char> dns_packet::construct(const std::string &t_host)
{
    dns_packet dp(t_host);
    return dp;
}

uint16_t dns_packet::getAId()
{
    static uint16_t begin=1000;
    return begin++;
}

size_t dns_packet_part2::size()const
{
    return nameAndtypeAndClass.size();
}
dns_packet::operator std::vector<char> ()
{
    std::vector<char> buff(sizeof(dns_packet_part1)+dp2.size(),0);
    std::memcpy((void *)&buff[0], (const void *)&dp1, sizeof(dns_packet_part1));
    std::copy(dp2.nameAndtypeAndClass.begin(), dp2.nameAndtypeAndClass.end(), buff.begin()+sizeof(dns_packet_part1));
    return buff;
}

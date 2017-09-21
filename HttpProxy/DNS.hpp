//
//  DNS.hpp
//  HttpProxy
//
//  Created by yangliqun on 2017/4/30.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#ifndef DNS_hpp
#define DNS_hpp
#include <string>
#include <vector>

namespace ylq {
    
    struct dns_packet_part1
    {
        uint16_t transactionId;
        uint16_t recursion:1,dontcare:15;
        uint16_t questions;
        uint16_t answerRRs;
        uint16_t authRRs;
        uint16_t additRRS;
    };
    struct dns_packet_part2
    {
        std::vector<char> nameAndtypeAndClass;
        dns_packet_part2(const std::string &t_host);
        size_t size()const;
    };
    struct dns_packet
    {
        struct dns_packet_part1 dp1;
        struct dns_packet_part2 dp2;
        dns_packet(const std::string &t_host);
        static std::vector<char> construct(const std::string &t_host);
        static uint16_t getAId();
        operator std::vector<char> ();
    };
    
    
    
    class DNS
    {
    public:
        static std::string getIp(const std::string &t_host);
        static std::string IP_FAILED;
        static void useDnsServer(const std::string &dnsServer);
        static void getHostAndPort(const std::string &t_httpHeader,std::string &t_host,std::string &t_port)throw (std::string);
        static std::string googleDnsServer;
        static std::string tencentDnsServer;
    private:
        static std::string workDnsServer;
    };
}


#endif /* DNS_hpp */

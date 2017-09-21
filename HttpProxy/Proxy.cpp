//
//  Proxy.cpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/1.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#include "Proxy.hpp"
#include "close_gurad.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include "Utils.h"
#include "DNS.hpp"
#include <iostream>
#include <fcntl.h>
#include <regex>
#include <exception>
#include <sys/select.h>
#include "Hook.hpp"


using namespace ylq;
#define Utils Utils<1>

Proxy::Proxy(int t_client_fd,std::shared_ptr<Hook> t_hook):comWithClient_fd(t_client_fd),m_hook(t_hook)
{
    comWithServer_fd=-1;
}
Proxy::Proxy()
{
    comWithClient_fd=-1;
    comWithServer_fd=-1;
}
void Proxy::manualCloseSocket()try
{
    if(comWithServer_fd!=-1)
    {
        close(comWithServer_fd);
        comWithServer_fd=-1;
    }
    if(comWithClient_fd!=-1)
    {
        close(comWithClient_fd);
        comWithClient_fd=-1;
    }
}
catch(...)
{
    Utils::log(1,"I dont know what happend.\n");
}

Proxy::~Proxy()
{
    Utils::log(1, "one Proxy terminate.\n");
    manualCloseSocket();
}
void Proxy::logClientBuff()
{
    clientBuff[cblen]=0;
    Utils::log(0,"recv from client %d:\n%s\n",cblen,clientBuff);
}
int Proxy::sendToClient()throw (std::string)
{
    if(m_hook.get()!=nullptr)
        m_hook->hookDataSendToClient(host, ip, port, serverBuff, sblen);
    int tmp=(int)send(comWithClient_fd, serverBuff, sblen, 0);
    if(tmp>0)
        return tmp;
    else if(tmp==0)
        throw std::string("client close the socket checked in relayer\n");
    else
        throw std::string("sendToClient() some error happend\n");
    
}
int Proxy::sendToServer()throw (std::string)
{
    if(m_hook.get()!=nullptr)
        m_hook->hookDataSendToServer(host, ip, port, clientBuff, cblen);
    int tmp=(int)send(comWithServer_fd,clientBuff,cblen,0);
    if(tmp>0)
        return tmp;//send normal
    else if(tmp==0)
        throw std::string("server close the socket checked in relayer\n");//server close the socket
    else
        throw std::string("sendToServer() some error happend\n");
}
int Proxy::recvFromClient()throw (std::string)
{
    cblen=(int)recv(comWithClient_fd, clientBuff, sizeof(clientBuff)-1, 0);
    if(cblen>0)
        return cblen;//recv from client normal
    else if(cblen==0)//mean client close the socket
        throw std::string("client close the socket\n");
    else
        throw std::string("recvFromClient() some error happend\n");
}
int Proxy::recvFromServer()throw (std::string)
{
    sblen=(int)recv(comWithServer_fd,serverBuff,sizeof(serverBuff)-1,0);
    if(sblen>0)
        return sblen;//recv from server normal
    else if(sblen==0)
        throw std::string("server close the socket\n");
    else
        throw std::string("recvFromServer() some error happend\n");
}

void Proxy::getHeaderFromClient()throw (std::string)
{
    timeval sptimeout;
    sptimeout.tv_sec=3;
    sptimeout.tv_usec=0;
    fd_set sprefds;
    __DARWIN_FD_ZERO(&sprefds);
    __DARWIN_FD_SET(comWithClient_fd, &sprefds);
    int spret=select(comWithClient_fd+1, &sprefds, nullptr, nullptr, &sptimeout);
    if(spret==0)//timeout
        throw std::string("first recv from client timeout\n");
    if(spret==-1)//some error
        throw std::string("first recv from client some error\n");
    cblen=(int)recv(comWithClient_fd, clientBuff, sizeof(clientBuff), 0);
    if(cblen<=0)
        throw std::string("recv from client error\n");
    logClientBuff();
}
void Proxy::dnsPhase()throw (std::string)
{
    DNS::getHostAndPort({clientBuff,clientBuff+cblen}, host, port);
    ip = DNS::getIp(host);
    if(ip==DNS::IP_FAILED)
        throw std::string("Get IP Failed:")+host+":"+port+"\n";
}
void Proxy::connectToServer()throw (std::string)
{
    comWithServer_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(comWithServer_fd==-1)
        throw std::string("comWithServer_fd socket() error\n");
    sockaddr_in reallyServer;
    socklen_t rslen=sizeof(reallyServer);
    reallyServer.sin_family=AF_INET;
    reallyServer.sin_port=htons(std::atoi(port.c_str()));
    reallyServer.sin_addr.s_addr=inet_addr(ip.c_str());
    if(connect(comWithServer_fd, (sockaddr *)&reallyServer, rslen)==-1)
        throw std::string("comWithServer_fd connect() error\n");
}
void Proxy::separateHttpsAndHttp()throw (std::string)
{
    if(std::strncmp(clientBuff,"CONNECT", 7)==0)
    {
        //CONNECT,HTTPS
        const char *ce=
"HTTP/1.0 200 Connection established\r\n\
Proxy-Agent: NetCache NetApp/6.0.7\r\n\r\n";
        std::copy(ce, ce+strlen(ce), serverBuff);
        sblen=(int)strlen(ce);
        sendToClient();
    }
    else
    {
        //http
        preprocess();
        sendToServer();
    }
}

void Proxy::beginRelay()throw (std::string)
{
    fd_set readfds;
    int ret;
    timeval timeout;
    while(true)
    {
        __DARWIN_FD_ZERO(&readfds);
        __DARWIN_FD_SET(comWithServer_fd, &readfds);
        __DARWIN_FD_SET(comWithClient_fd, &readfds);
        timeout.tv_sec=10;
        timeout.tv_usec=0;
        ret=select(std::max(comWithServer_fd, comWithClient_fd)+1, &readfds, nullptr, nullptr,&timeout);
        if(ret==-1)//some error
            throw std::string("Proxy select error\n");
        if(ret==0)//timeout
            throw std::string("network is not well host:")+host+" Port:"+port+"\n";
        if(__DARWIN_FD_ISSET(comWithClient_fd, &readfds))
        {
            //read from client,and send to server
            recvFromClient();
            sendToServer();
        }
        if(__DARWIN_FD_ISSET(comWithServer_fd, &readfds))
        {
            //read from server,and send to client
            recvFromServer();
            sendToClient();
        }
    }
    
}
void Proxy::operator()()try
{
    getHeaderFromClient();//从客户端获取http或者https请求头
    dnsPhase();//dns阶段，通过请求头提取出其中的ip地址或者主机名称，如果是主机名称，那么还要经过dns阶段，获取ip地址
    connectToServer();//获取了ip地址后，连接remote server
    separateHttpsAndHttp();//对https和http的不同处理
    beginRelay();//连接建立，开始中继服务
    
}
catch(std::string msg)
{
    Utils::log(0,"%s",msg.c_str());
    manualCloseSocket();
}
catch(...)
{
    Utils::log(0,"some unknow error happend\n");
    manualCloseSocket();
}



void Proxy::preprocess()try
{
    //remove the "http://host" from the http header
    std::regex pattern("[A-Z]{3,} (http:\/\/[^\/]*)");
    std::string str(clientBuff,clientBuff+cblen);
    std::smatch sm;
    std::regex_search(str,sm,pattern);
    if(sm.empty())
        return;
    std::string match=*(++sm.begin());
    char *pos = std::search(clientBuff, clientBuff+cblen, match.begin(), match.end());
    (*std::copy(pos+match.size(), clientBuff+cblen, pos))=0;
    cblen=cblen-(int)match.size();
}
catch(...)
{
    Utils::log(0,"preprocess error %s\n",clientBuff);
}

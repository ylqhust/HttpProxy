//
//  FourThreadSelectProxy.cpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/2.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#include "FourThreadSelectProxy.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/select.h>
#include "DNS.hpp"
#include "Utils.h"
#include <set>

using namespace ylq;

void Connector::addClienFD(CLIENT_FD t_client_fd)
{
    {
        std::unique_lock<std::mutex> m(m_mutex);
        if(m_stop)
            return;
        m_deque.push_back(t_client_fd);
    }
    beginWork();
}


bool Connector::haveWork()
{
    return !m_deque.empty();
}

void Connector::work()try
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        comWithClient_fd=m_deque.front();
        m_deque.pop_front();
    }
    getHeaderFromClient();//从客户端获取http或者https请求头
    dnsPhase();//dns阶段，通过请求头提取出其中的ip地址或者主机名称，如果是主机名称，那么还要经过dns阶段，获取ip地址
    connectToServer();//获取了ip地址后，连接remote server
    separateHttpsAndHttp();//对https和http的不同处理
    FourThreadSelectProxy::getInstance().addProcessRequest(comWithClient_fd,comWithServer_fd);
}
catch(const std::string &msg)
{
    Utils<1>::log(1,"Connector error: %s\n",msg.c_str());
    manualCloseSocket();
}
catch(...)
{
    Utils<1>::log(1,"Connector unknow error.\n");
    manualCloseSocket();
}


void FTSP_Worker::addCS_FD(CLIENT_FD t_client_fd, SERVER_FD t_server_fd)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_stop)
            return;
        m_fds.push_back({t_client_fd,t_server_fd});
    }
    beginWork();
}

bool FTSP_Worker::haveWork()
{
    return !m_fds.empty();
}

void FTSP_Worker::work()try
{
    fd_set readfds;
    int ret;
    timeval timeout;
    CLIENT_FD maxcfd;
    SERVER_FD maxsfd;
    
    while(true)
    {
        Utils<1>::log(1,"The FTSP_Worker %x have fd count %d.\n",std::this_thread::get_id(),m_fds.size());
        maxcfd=-1;
        maxsfd=-1;
        __DARWIN_FD_ZERO(&readfds);
        for(auto pair_cs:m_fds)
        {
            maxcfd=std::max(maxcfd, pair_cs.first);
            maxsfd=std::max(maxsfd, pair_cs.second);
            __DARWIN_FD_SET(pair_cs.first, &readfds);
            __DARWIN_FD_SET(pair_cs.second, &readfds);
        }
        timeout.tv_sec=4;
        timeout.tv_usec=0;
        ret=select(std::max(maxcfd, maxsfd)+1, &readfds,nullptr,nullptr, &timeout);
        
        if(ret==-1||ret==0)//error or timeout
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            for(auto pair_cs:m_fds)
            {
                close(pair_cs.first);
                close(pair_cs.second);
            }
            m_fds.clear();
            comWithClient_fd=-1;
            comWithServer_fd=-1;
            std::string msg;
            msg+=(ret==0)?"timeout.\n":"select error.\n";
            throw msg;
        }
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            for(auto pos=m_fds.begin();pos!=m_fds.end();++pos)
            {
                auto pair_cs=*pos;
                if(__DARWIN_FD_ISSET(pair_cs.first, &readfds))
                {
                    comWithClient_fd=pair_cs.first;
                    comWithServer_fd=pair_cs.second;
                    try{
                        recvFromClient();
                        sendToServer();
                    }
                    catch(const std::string &msg)
                    {
                        Utils<1>::log(1, "FTSP_Worker %d %s",std::this_thread::get_id(),msg.c_str());
                        m_fds.erase(pos);
                        manualCloseSocket();
                        --pos;
                    }
                }
                else if(__DARWIN_FD_ISSET(pair_cs.second,&readfds))
                {
                    comWithClient_fd=pair_cs.first;
                    comWithServer_fd=pair_cs.second;
                    try{
                        recvFromServer();
                        sendToClient();
                    }
                    catch(const std::string &msg)
                    {
                        Utils<1>::log(1, "FTSP_Worker %d %s",std::this_thread::get_id(),msg.c_str());
                        m_fds.erase(pos);
                        manualCloseSocket();
                        --pos;
                    }
                }
            }
        }
       
    }
}
catch(const std::string &msg)
{
    Utils<1>::log(1,"Connector error: %d %s\n",std::this_thread::get_id(),msg.c_str());
    manualCloseSocket();
}
catch(...)
{
    Utils<1>::log(1,"Connector unknow error.\n");
    manualCloseSocket();
}

FourThreadSelectProxy &FourThreadSelectProxy::getInstance()
{
    static FourThreadSelectProxy instance;
    return instance;
}

void FourThreadSelectProxy::addProcessRequest(CLIENT_FD t_client_fd, SERVER_FD t_server_fd)
{
    _4works[(m_nextWorkIndicator++)%4].addCS_FD(t_client_fd, t_server_fd);
}
void FourThreadSelectProxy::addConnectRequest(CLIENT_FD t_client_fd)
{
    _16connector[(m_nextConnectorIndicator++)%16].addClienFD(t_client_fd);
}

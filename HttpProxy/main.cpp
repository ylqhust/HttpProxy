#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <errno.h>
#include <string>
#include "close_gurad.hpp"
#include "ThreadPool.h"
#include "Proxy.hpp"
#include "HookBH3.hpp"
#include "HookMT.hpp"
#include "FourThreadSelectProxy.hpp"
#include "Utils.h"
#include "Terminate.hpp"
#include "DNS.hpp"

#include "test.h"
#define DEFAULT_PORT 8894


using namespace ylq;
int    socket_fd, connect_fd;
void terminate()
{
    Utils<1>::log(1, "terminate be called.\n");
    close(socket_fd);
}
int main()
{
    Terminate::getInstance(terminate);
    DNS::useDnsServer(DNS::googleDnsServer);
    struct sockaddr_in     servaddr;
    
    if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        perror("socket() error:");
        exit(0);
    }
    close_guard<int> socfd(socket_fd);
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(DEFAULT_PORT);
    
    if( bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        perror("bind socket error:");
        exit(0);
    }
    
    if( listen(socket_fd, 100) == -1){
        perror("listen socket error:");
        exit(0);
    }
    
    sockaddr_in client;
    socklen_t clen=sizeof(client);
    std::shared_ptr<HookBH3> hook(new HookBH3);
    std::shared_ptr<HookMT> hook2(new HookMT);
    std::cout<<"Ip:"<<hook->getHostIp()<<" Port:"<<DEFAULT_PORT<<std::endl;
    
    printf("=======waiting=======\n");
    while(true){
        if( (connect_fd = accept(socket_fd, (struct sockaddr*)&client, &clen)) == -1){
            perror("accept socket error:");
            break;
        }
        Utils<1>::log(1, "accept a client\n");
      //  FourThreadSelectProxy::getInstance().addConnectRequest(connect_fd);
        ThreadPool::getInstance().addTask(std::shared_ptr<Proxy>(new Proxy(connect_fd,hook)));
    }
    return 0;
}


//
//  test.h
//  HttpProxy
//
//  Created by yangliqun on 2017/5/1.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#ifndef test_h
#define test_h

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "HookBH3.hpp"
#include "HookMT.hpp"
using namespace ylq;

void test()
{
    int sock_fd = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in server;
    bzero(&server, sizeof(sockaddr_in));
    server.sin_family=AF_INET;
    server.sin_port=htons(80);
    server.sin_addr.s_addr=inet_addr("139.224.7.27");
    
    if(connect(sock_fd, (sockaddr *)&server, sizeof(server))==-1)
    {
        printf("connection error\n");
        return;
    }
    
    const char *query=
"GET /query_gameserver?version=1.4.0_gf_ios&uid=35363091&t=1493629449 HTTP/1.1\r\n\
Host: 139.224.7.27\r\n\
Connection: close\r\n\
Accept: */*\r\n\
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.81 Safari/537.36\r\n\
Accept-Language: zh-cn\r\n\
Accept-Encoding: gzip, deflate\r\n\
X-Unity-Version: 5.3.6p5\r\n\
\r\n";
    send(sock_fd, query, strlen(query), 0);
    char buff[1024];
    ssize_t len=recv(sock_fd, buff, sizeof(buff), 0);
    buff[len]=0;
    printf("len=%l\n%s\n",len,buff);
    close(sock_fd);
    return;
}

void test2()
{
    char buff[]=R"({"account_url":"https://account.mihoyo.com","asset_boundle_url":"https://bundle.bh3.com/asset_bundle/ios01/1.0","ext":{"is_xxxx":"0","show_version_text":"0"},"gameserver":{"ip":"139.224.43.40","port":15100},"gateway":{"ip":"139.224.43.40","port":15100},"msg":"","oaserver_url":"http://139.224.7.27:1080","retcode":0})";
    
    HookBH3 h;
    int len=sizeof(buff);
    h.hookDataSendToClient("", "139.224.7.27", "80", buff,len);
    
    printf("after modify :\n%s \n%d\n",buff,len);
}

void test3()
{
    char buff[]=R"(<GameServerId>5353</GameServerId>
    <GameServerName>19区 烁珖海床</GameServerName>
    <GameServerIp>59.151.103.135</GameServerIp>
    <Port>36666</Port>
    <GameServerUrl></GameServerUrl>
    <Status>1</Status>
    <Order>1</Order>
    <Weight>1</Weight>)";
    
    HookMT h;
    int len=sizeof(buff);
    printf("%s \n %d\n",buff,len);
    h.hookDataSendToClient("ac.locojoy.com", "59.151.104.134", "80", buff,len);
    printf("%s \n %d\n",buff,len);
    
}
#endif /* test_h */

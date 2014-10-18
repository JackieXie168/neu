/*

      ___           ___           ___
     /\__\         /\  \         /\__\
    /::|  |       /::\  \       /:/  /
   /:|:|  |      /:/\:\  \     /:/  /
  /:/|:|  |__   /::\~\:\  \   /:/  /  ___
 /:/ |:| /\__\ /:/\:\ \:\__\ /:/__/  /\__\
 \/__|:|/:/  / \:\~\:\ \/__/ \:\  \ /:/  /
     |:/:/  /   \:\ \:\__\    \:\  /:/  /
     |::/  /     \:\ \/__/     \:\/:/  /
     /:/  /       \:\__\        \::/  /
     \/__/         \/__/         \/__/


The Neu Framework, Copyright (c) 2013-2014, Andrometa LLC
All rights reserved.

neu@andrometa.net
http://neu.andrometa.net

Neu can be used freely for commercial purposes. If you find Neu
useful, please consider helping to support our work and the evolution
of Neu by making a PayPal donation to: neu@andrometa.net

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
 
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
 
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
*/

#ifndef NEU_N_LISTENER_H
#define NEU_N_LISTENER_H

#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

#include <neu/NSocket.h>

namespace neu{
  
  class NListener{
  public:
    NListener()
      : port_(-1),
    listenBackLog_(1000){}

    ~NListener(){
      close();
    }
    
    bool listen(int port){
      close();
      
      fd_ = socket(PF_INET, SOCK_STREAM, 0);
      
      if(fd_ < 0){
        return false;
      }

      int so_reuseaddr = 1;

      setsockopt(fd_,
                 SOL_SOCKET,
                 SO_REUSEADDR,
                 &so_reuseaddr,
                 sizeof(so_reuseaddr));
      
      sockaddr_in addr;
      
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = INADDR_ANY;
      addr.sin_port = htons(port);
      
      int status = ::bind(fd_, (const sockaddr*)&addr, sizeof(addr));
      if(status < 0){
        return false;
      }
      
      if(::listen(fd_, listenBackLog_) < 0){
        return false;
      }
      
      port_ = port;
      
      return true;
    }

    NSocket* accept(){
     if(port_ < 0){
        return 0;
      }
      
      int opts = fcntl(fd_, F_GETFL);
      opts &= ~O_NONBLOCK;
      
      fcntl(fd_, F_SETFL, opts);
      
      sockaddr addr;
      socklen_t len = sizeof(sockaddr);
            
      int fd = ::accept(fd_, &addr, &len);
      
      if(fd < 0){
        return 0;
      }
      
      int reuseOn = 1;
      setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, 
                 &reuseOn, sizeof(reuseOn));
      
      char host[2048];
      
      getnameinfo(&addr, sizeof(addr), host, 2048, 0, 0, 0);

      sockaddr_in addrIn;
      socklen_t inLen = sizeof(addrIn);
      getpeername(fd, (sockaddr*)&addrIn, &inLen);

      NSocket* socket = new NSocket(fd);
      socket->setHost_(host);
      socket->setIPAddress_(inet_ntoa(addrIn.sin_addr));

      return socket;
    }

    NSocket* accept(double timeout){
      if(port_ == -1){
        return 0;
      }
      
      sockaddr addr;
      socklen_t len = sizeof(sockaddr);
      
      int opts = fcntl(fd_, F_GETFL);
      opts |= O_NONBLOCK;
      
      fcntl(fd_, F_SETFL, opts);

      int fd;

      while((fd = ::accept(fd_, &addr, &len)) < 0){
        fd_set rfs;
        FD_ZERO(&rfs);
        FD_SET(fd_, &rfs);

        double sec = floor(timeout);
        double fsec = timeout - sec;
        
        timeval tv;
        tv.tv_sec = sec;
        tv.tv_usec = fsec*1e6;

        int ret = select(fd_ + 1, &rfs, 0, 0, &tv);
        
        if(ret < 1){
          return 0;
        }
      }

      opts = fcntl(fd, F_GETFL);
      opts &= ~O_NONBLOCK;
      fcntl(fd, F_SETFL, opts);
      
      int reuseOn = 1;
      setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, 
                 &reuseOn, sizeof(reuseOn));
      
      char host[2048];
      
      getnameinfo(&addr, sizeof(addr), host, 2048, 0, 0, 0);

      sockaddr_in addrIn;
      socklen_t inLen = sizeof(addrIn);
      getpeername(fd, (sockaddr*)&addrIn, &inLen);

      NSocket* socket = new NSocket(fd);
      socket->setHost_(host);
      socket->setIPAddress_(inet_ntoa(addrIn.sin_addr));

      return socket;
    } 
    
    void close(){
      if(port_ < 0){
        return;
      }

      ::close(fd_);
      port_ = -1;
    }

    NListener& operator=(const NListener&) = delete;

    NListener(const NListener&) = delete;
    
  private:  
    int port_;
    int fd_;
    size_t listenBackLog_;
  };
  
} // end namespace neu

#endif // NEU_N_LISTENER_H

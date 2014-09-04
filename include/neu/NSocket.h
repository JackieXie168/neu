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

Neu can be used freely for commercial purposes. We hope you will find
Neu powerful, useful, and fun! If so, please consider making a
donation via PayPal to: neu@andrometa.net

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

#ifndef NEU_N_SOCKET_H
#define NEU_N_SOCKET_H

#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <neu/NResourceManager.h>
#include <neu/global.h>

namespace neu{

  class NSocket{
  public:
    NSocket()
    : fd_(-1),
    resetReceiveTimeout_(false),
    resetSendTimeout_(false){
      _resourceManager->add(this);
    }
    
    NSocket(int fd)
    : fd_(fd),
    resetReceiveTimeout_(false),
    resetSendTimeout_(false){
      _resourceManager->add(this);
    }
    
    ~NSocket(){
      close();
      _resourceManager->remove(this);
    }
    
    bool connect(const nstr& host, int port){
      hostent* h;
      h = gethostbyname(host.c_str());
      if(!h){
        return false;
      }
      
      fd_ = socket(AF_INET, SOCK_STREAM, 0);
      
      if(fd_ < 0){
        return false;
      }
      
      int reuseOn = 1;
      setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuseOn, sizeof(reuseOn));
      
      sockaddr_in addr;
      bzero((char*)&addr, sizeof(addr));
      addr.sin_family = AF_INET;
      bcopy((char*)h->h_addr,
            (char*)&addr.sin_addr.s_addr,
            h->h_length);
      addr.sin_port = htons(port);
      
      if(::connect(fd_, (sockaddr*)&addr, sizeof(addr)) < 0){
        return false;
      }
      
      host_ = host;
      
      return true;
    }
    
    void close(){
      if(fd_ >= 0){
        ::close(fd_);
        fd_ = -1;
      }
    }
    
    int send(char* buf, size_t len){
      if(resetSendTimeout_){
        timeval tv;
        tv.tv_sec = 315360000;
        tv.tv_usec = 0;
        
        setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO,
                   (timeval*)&tv, sizeof(timeval));
        
        resetSendTimeout_ = false;
      }
      
      return ::send(fd_, buf, len, 0);
    }
    
    int send(char* buf, size_t len, double timeout){
      double sec = floor(timeout);
      double fsec = timeout - sec;
      
      timeval tv;
      tv.tv_sec = sec;
      tv.tv_usec = fsec*1e6;
      
      setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO,
                 (timeval*)&tv, sizeof(timeval));
      
      resetSendTimeout_ = true;
      
      return ::send(fd_, buf, len, 0);
    }
    
    int receive(char* buf, size_t len){
      if(resetReceiveTimeout_){
        timeval tv;
        tv.tv_sec = 315360000;
        tv.tv_usec = 0;
        
        setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO,
                   (timeval*)&tv, sizeof(timeval));
        resetReceiveTimeout_ = false;
      }
      
      return ::recv(fd_, buf, len, 0);
    }
    
    int receive(char* buf, size_t len, double timeout){
      double sec = floor(timeout);
      double fsec = timeout - sec;
      
      timeval tv;
      tv.tv_sec = sec;
      tv.tv_usec = fsec*1e6;
      
      setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO,
                 (timeval*)&tv, sizeof(timeval));
      
      resetReceiveTimeout_ = true;
      
      return ::recv(fd_, buf, len, 0);
    }
    
    void setHost_(const nstr& host){
      host_ = host;
    }
    
    const nstr& host(){
      return host_;
    }
    
    void setIPAddress_(const nstr& ipAddress){
      ipAddress_ = ipAddress;
    }
    
    const nstr& ipAddress(){
      return ipAddress_;
    }
    
    static bool isLocalHost(const nstr& host){
      return host == "127.0.0.1" || host == "localhost" || host == "127.0.1.1";
    }

    NSocket& operator=(const NSocket&) = delete;

    NSocket(const NSocket&) = delete;
    
  private:
    int fd_;
    nstr host_;
    nstr ipAddress_;
    bool resetReceiveTimeout_;
    bool resetSendTimeout_;
  };
  
} // end namespace neu

#endif // NEU_N_SOCKET_H

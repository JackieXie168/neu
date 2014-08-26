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
 
 
Neu, Copyright (c) 2013-2014, Andrometa LLC
All rights reserved.

neu@andrometa.net
http://neu.andrometa.net

Neu can be used freely for commercial purposes. We hope you will find
Neu powerful, useful to make money or otherwise, and fun! If so,
please consider making a donation via PayPal to: neu@andrometa.net

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

#include <neu/NCommunicator.h>

#include <atomic>

#include <neu/NProc.h>
#include <neu/NBasicMutex.h>
#include <neu/NVSemaphore.h>
#include <neu/NError.h>
#include <neu/NSocket.h>
#include <neu/NGuard.h>

using namespace std;
using namespace neu;

namespace{
  
  class ReceiveProc : public NProc{
  public:
    ReceiveProc(NProcTask* task, NCommunicator_* c);
    
    void run(nvar& r);
    
  private:
    NProcTask* task_;
    NCommunicator_* c_;
    NSocket* s_;
  };
  
  class SendProc : public NProc{
  public:
    SendProc(NProcTask* task, NCommunicator_* c);
    
    void run(nvar& r);
    
  private:
    NProcTask* task_;
    NCommunicator_* c_;
    NSocket* s_;
  };
  
} // end namespace

namespace neu{
  
  class NCommunicator_{
  public:
    NCommunicator_(NCommunicator* o, NProcTask* task)
    : o_(o),
    task_(task),
    socket_(0),
    sendProc_(0),
    receiveProc_(0),
    encoder_(0){
      
    }
    
    ~NCommunicator_(){
      if(socket_){
        if(task_->terminate(sendProc_)){
          delete sendProc_;
        }
        
        if(task_->terminate(receiveProc_)){
          delete receiveProc_;
        }
        
        delete socket_;
      }
    }
    
    void setSocket(NSocket* socket){
      socket_ = socket;
      init();
    }
    
    void init(){
      connected_ = true;
      
      sendProc_ = new SendProc(task_, this);
      receiveProc_ = new ReceiveProc(task_, this);

      task_->queue(sendProc_);
      task_->queue(receiveProc_);
    }
    
    bool connect(const nstr& host, int port){
      if(socket_){
        NERROR("socket exists");
      }
      
      socket_ = new NSocket;
      if(!socket_->connect(host, port)){
        delete socket_;
        socket_ = 0;
        return false;
      }
      
      init();
      
      return true;
    }
    
    NProcTask* task(){
      return task_;
    }
    
    void close(){
      if(!connected_){
        return;
      }
      
      connected_ = false;
      socket_->close();
      
      o_->onClose(true);
    }
    
    void close_(){
      if(!connected_){
        return;
      }
      
      connected_ = false;
      socket_->close();
      
      o_->onClose(false);
    }
    
    bool isConnected() const{
      return connected_;
    }
    
    void send(nvar& msg){
      sendMutex_.lock();
      sendQueue_.emplace_back(move(msg));
      sendMutex_.unlock();
      sendSem_.release();
    }
    
    bool receive(nvar& msg, double timeout){
      if(!receiveSem_.acquire(timeout)){
        return false;
      }
      
      receiveMutex_.lock();
      msg = move(receiveQueue_.front());
      receiveQueue_.pop_front();
      receiveMutex_.unlock();
      return true;
    }
    
    bool receive(nvar& msg){
      receiveSem_.acquire();
      
      receiveMutex_.lock();
      msg = move(receiveQueue_.front());
      receiveQueue_.pop_front();
      receiveMutex_.unlock();
      return true;
    }
    
    bool get(nvar& msg, double timeout){
      if(!sendSem_.acquire(timeout)){
        return false;
      }

      sendMutex_.lock();
      msg = move(sendQueue_.front());
      sendQueue_.pop_front();
      sendMutex_.unlock();
      return true;
    }
    
    void put(nvar& msg){
      receiveMutex_.lock();
      receiveQueue_.emplace_back(move(msg));
      receiveMutex_.unlock();
      receiveSem_.release();
    }
    
    NSocket* socket(){
      return socket_;
    }
    
    virtual char* header(uint32_t& size){
      return encoder_ ? encoder_->header(size) : 0;
    }
    
    virtual char* encrypt(char* buf, uint32_t& size){
      return encoder_ ? encoder_->encrypt(buf, size) : buf;
    }
    
    virtual char* decrypt(char* buf, uint32_t& size){
      return encoder_ ? encoder_->decrypt(buf, size) : buf;
    }
    
    nvar& session(){
      return session_;
    }
    
    void setEncoder(NCommunicator::Encoder* encoder){
      encoder_ = encoder;
    }
    
  private:
    NCommunicator* o_;
    NCommunicator::Encoder* encoder_;
    NProcTask* task_;
    NSocket* socket_;
    nqueue sendQueue_;
    NBasicMutex sendMutex_;
    NVSemaphore sendSem_;
    SendProc* sendProc_;
    nqueue receiveQueue_;
    NBasicMutex receiveMutex_;
    NVSemaphore receiveSem_;
    ReceiveProc* receiveProc_;
    atomic_bool connected_;
    nvar session_;
  };
  
} // end namespace neu

ReceiveProc::ReceiveProc(NProcTask* task, NCommunicator_* c)
: task_(task),
c_(c),
s_(c_->socket()){
  
}

void ReceiveProc::run(nvar& r){
  if(!c_->isConnected()){
    return;
  }
  
  int n;
  uint32_t size;
  
  char* h = c_->header(size);
  if(h){
    char* hbuf = (char*)malloc(size);
    n = s_->receive(hbuf, size, _timeout);
    if(n == -1){
      signal(task_, this);
      return;
    }
    
    if(n != 4){
      free(hbuf);
      c_->close_();
      return;
    }

    for(size_t i = 0; i < size; ++i){
      if(hbuf[i] != h[i]){
        free(hbuf);
        c_->close_();
        return;
      }
    }
    free(hbuf);
    n = s_->receive((char*)&size, 4);
  }
  else{
    n = s_->receive((char*)&size, 4, _timeout);
    if(n == -1){
      signal(task_, this);
      return;
    }
  }

  if(n != 4){
    c_->close_();
    return;
  }
  
  char* buf = (char*)malloc(size);
  
  uint32_t nTotal = 0;
  
  while(nTotal < size){
    n = s_->receive(buf + nTotal, size - nTotal);
    
    if(n <= 0){
      free(buf);
      c_->close_();
      return;
    }
    
    nTotal += n;
  }
  
  buf = c_->decrypt(buf, size);
  
  nvar msg;
  msg.unpack(buf, size);
  free(buf);
  
  c_->put(msg);
  
  signal(task_, this);
}

SendProc::SendProc(NProcTask* task, NCommunicator_* c)
: task_(task),
c_(c),
s_(c_->socket()){
  
}

void SendProc::run(nvar& r){
  if(!c_->isConnected()){
    return;
  }
  
  nvar msg;
  if(!c_->get(msg, _timeout)){
    signal(task_, this);
    return;
  }
  
  int n;
  uint32_t size;

  char* h = c_->header(size);
  if(h){
    n = s_->send(h, size);
    if(n != size){
      c_->close();
      return;
    }
  }
  
  char* buf = msg.pack(size);
  n = s_->send((char*)&size, 4);

  if(n != 4){
    free(buf);
    c_->close();
    return;
  }
  
  buf = c_->encrypt(buf, size);

  n = s_->send(buf, size);
  
  free(buf);
  
  if(n != size){
    c_->close();
    return;
  }
  
  signal(task_, this);
}

NCommunicator::NCommunicator(NProcTask* task){
  x_ = new NCommunicator_(this, task);
}

NCommunicator::~NCommunicator(){
  delete x_;
}

bool NCommunicator::connect(const nstr& host, int port){
  return x_->connect(host, port);
}

void NCommunicator::setSocket(NSocket* socket){
  x_->setSocket(socket);
}

NProcTask* NCommunicator::task(){
  return x_->task();
}

void NCommunicator::close(){
  x_->close();
}

bool NCommunicator::isConnected() const{
  return x_->isConnected();
}

void NCommunicator::send(nvar& msg){
  x_->send(msg);
}

bool NCommunicator::receive(nvar& msg){
  return x_->receive(msg);
}

bool NCommunicator::receive(nvar& msg, double timeout){
  return x_->receive(msg, timeout);
}

nvar& NCommunicator::session(){
  return x_->session();
}

void NCommunicator::setEncoder(Encoder* encoder){
  x_->setEncoder(encoder);
}

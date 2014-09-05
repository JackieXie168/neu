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

#include <neu/NBroker.h>

#include <neu/NProc.h>
#include <neu/NServer.h>
#include <neu/NBasicMutex.h>
#include <neu/NRWMutex.h>
#include <neu/NObject.h>
#include <neu/NClass.h>
#include <neu/global.h>
#include <neu/NReadGuard.h>
#include <neu/NServerProc.h>

using namespace std;
using namespace neu;

namespace{
  
  static const int OP_ERROR = 100;
  static const int OP_OBTAIN = 101;
  static const int OP_OBTAINED = 102;
  static const int OP_CALL = 103;
  static const int OP_CALLED = 104;
  
  class DistributedObject;
  
  class ServerProc : public NServerProc{
  public:
    ServerProc(NProcTask* task, NBroker_* broker);
    
    void onClose(bool manual);
    
    void run(nvar& r);
    
  private:
    NProcTask* task_;
    NBroker_* broker_;
    DistributedObject* obj_;
  };
  
  class Server : public NServer{
  public:
    Server(NProcTask* task, NBroker_* broker)
    : NServer(task),
    task_(task),
    broker_(broker){
      
    }
    
    NCommunicator* create(){
      return new ServerProc(task(), broker_);
    }
    
    bool authenticate(NCommunicator* comm, const nvar& auth);

  private:
    NProcTask* task_;
    NBroker_* broker_;
  };
  
  class Client : public NCommunicator{
  public:
    Client(NProcTask* task, NBroker_* broker);
    
    void setObj(NObject* obj){
      obj_ = obj;
    }
    
    NObject* obj(){
      return obj_;
    }
    
    void onClose(bool manual);
    
  private:
    NBroker_* broker_;
    NObject* obj_;
  };
  
  class DistributedObject{
  public:
    DistributedObject(NProcTask* task)
    : task_(task){}
    
    nstr className;
    NObject* obj;
    
    ~DistributedObject(){
      revoke();
    }
    
    void addServerProc(ServerProc* proc){
      mutex_.lock();
      serverProcs_.add(proc);
      mutex_.unlock();
    }
    
    void removeServerProc(ServerProc* proc){
      mutex_.lock();
      serverProcs_.erase(proc);
      mutex_.unlock();
    }
    
    void revoke(){
      mutex_.lock();
      for(ServerProc* serverProc : serverProcs_){
        serverProc->close();
        if(task_->terminate(serverProc)){
          delete serverProc;
        }
      }
      mutex_.unlock();
    }
    
  private:
    typedef NHashSet<ServerProc*> ServerProcSet_;

    NProcTask* task_;
    ServerProcSet_ serverProcs_;
    NBasicMutex mutex_;
  };
  
} // end namespace

namespace neu{
  
  class NBroker_{
  public:
    
    NBroker_(NBroker* o, NProcTask* task)
    : o_(o),
    task_(task),
    server_(0),
    encoder_(0){
      
    }
    
    ~NBroker_(){
      if(server_){
        delete server_;
      }
    }
    
    NProcTask* task(){
      return task_;
    }
    
    bool listen(int port){
      server_ = new Server(task_, this);
      if(server_->listen(port)){
        return true;
      }
      
      delete server_;
      server_ = 0;
      return false;
    }
    
    void distribute(NObject* object,
                    const nstr& className,
                    const nstr& objectName){
      DistributedObject* o = new DistributedObject(task_);
      o->className = className;
      o->obj = object;
      
      serverMutex_.writeLock();
      distributedObjectMap_[objectName] = o;
      serverMutex_.unlock();
    }
    
    void revoke(const nstr& objectName){
      serverMutex_.readLock();
      auto itr = distributedObjectMap_.find(objectName);
      assert(itr != distributedObjectMap_.end());
      distributedObjectMap_.erase(itr);
      serverMutex_.unlock();
      
      DistributedObject* o = itr->second;
      o->revoke();
      delete o;
    }
    
    NObject* obtain(const nstr& host,
                    int port,
                    const nstr& objectName,
                    const nvar& auth){
      
      Client* client = new Client(task_, this);
      client->setEncoder(encoder_);
      
      if(!client->connect(host, port)){
        delete client;
        return 0;
      }

      nvar a = auth;
      client->send(a);
      nvar authResp;
      if(!client->receive(authResp) || !authResp){
        delete client;
        return 0;
      }
      
      nvar req;
      req("op") = OP_OBTAIN;
      req("o") = objectName;
      client->send(req);

      nvar resp;
      if(!client->receive(resp) || resp["op"] != OP_OBTAINED){
        delete client;
        return 0;
      }
      
      const nstr& className = resp["c"];
      
      NObject* obj = NClass::createRemote(className, o_);
      
      if(!obj){
        delete client;
        NERROR("failed to create remote object of class: " + className);
      }
      
      client->setObj(obj);
      clientMutex_.writeLock();
      objectClientMap_.insert({obj, client});
      clientMutex_.unlock();
      
      return obj;
    }
    
    void release(NObject* object){
      clientMutex_.writeLock();
      
      auto itr = objectClientMap_.find(object);
      if(itr == objectClientMap_.end()){
        clientMutex_.unlock();
        return;
      }
      
      Client* client = itr->second;
      objectClientMap_.erase(itr);
      clientMutex_.unlock();
      delete client;
    }
    
    void setLogStream(std::ostream& ostr){
      logStream_ = &ostr;
    }
    
    NBroker* outer(){
      return o_;
    }
    
    void setEncoder(NCommunicator::Encoder* encoder){
      encoder_ = encoder;
    }
    
    NCommunicator::Encoder* encoder(){
      return encoder_;
    }
    
    nvar process_(NObject* obj, const nvar& n){
      NReadGuard guard(clientMutex_);
      
      auto itr = objectClientMap_.find(obj);
      assert(itr != objectClientMap_.end());

      Client* client = itr->second;
      
      nvar req;
      req("op") = OP_CALL;
      req("f") = n;

      client->send(req);
      
      nvar resp;
      if(!client->receive(resp) || resp["op"] != OP_CALLED){
        NERROR("failed to remote process: " + n);
      }
      
      nvar& f = resp["f"];
      size_t size = n.size();

      for(size_t i = 0; i < size; ++i){
        if(n[i].fullType() == nvar::Pointer){
          *n[i] = move(f[i]);
        }
      }
      
      return resp["r"];
    }
    
    DistributedObject* obtain_(ServerProc* proc, const nstr& objName){
      NReadGuard guard(serverMutex_);
      
      auto itr = distributedObjectMap_.find(objName);
      if(itr == distributedObjectMap_.end()){
        return 0;
      }
      
      DistributedObject* o = itr->second;
      o->addServerProc(proc);
      return o;
    }
    
    void removeClient(NObject* obj){
      clientMutex_.writeLock();
      auto itr = objectClientMap_.find(obj);
      objectClientMap_.erase(itr);
      clientMutex_.unlock();
      delete obj;
      // ndm - need to delete the client
    }
    
  private:
    typedef NHashMap<NObject*, Client*> ObjectClientMap_;
    typedef NHashMap<nstr, DistributedObject*> DistributedObjectMap_;
    
    NBroker* o_;
    NProcTask* task_;
    Server* server_;
    NCommunicator::Encoder* encoder_;
    ObjectClientMap_ objectClientMap_;
    DistributedObjectMap_ distributedObjectMap_;
    ostream* logStream_;
    NRWMutex clientMutex_;
    NRWMutex serverMutex_;
  };
  
} // end namespace neu

void ServerProc::run(nvar& r){
  nvar req;
  if(!receive(req, _timeout)){
    signal(task_, this);
    return;
  }
  
  int op = req["op"];

  switch(op){
    case OP_OBTAIN:{
      obj_ = broker_->obtain_(this, req["o"]);
      
      if(!obj_){
        close();
        return;
      }
      
      nvar resp;
      resp("op") = OP_OBTAINED;
      resp("c") = obj_->className;
      send(resp);
      break;
    }
    case OP_CALL:{
      if(!obj_){
        close();
        return;
      }
      
      nvar& f = req["f"];
      
      nvar resp;
      try{
        resp("r") = obj_->obj->process(f);
      }
      catch(NError& e){
        resp("op") = OP_ERROR;
        send(resp);
        break;
      }
      
      resp("op") = OP_CALLED;
      resp("f") = move(f);
      
      send(resp);
      break;
    }
  }
  
  signal(task_, this);
}

ServerProc::ServerProc(NProcTask* task, NBroker_* broker)
: NServerProc(task),
task_(task),
broker_(broker),
obj_(0){
  setEncoder(broker_->encoder());
}

void ServerProc::onClose(bool manual){
  obj_->removeServerProc(this);
  terminate();
}

bool Server::authenticate(NCommunicator* comm, const nvar& auth){
  if(!broker_->outer()->authenticate(auth)){
    return false;
  }

  ServerProc* proc = static_cast<ServerProc*>(comm);
  task_->queue(proc);
  
  return true;
}

Client::Client(NProcTask* task, NBroker_* broker)
: NCommunicator(task),
broker_(broker){
  
}

void Client::onClose(bool manual){
  broker_->removeClient(obj_);
}

NBroker::NBroker(NProcTask* task){
  x_ = new NBroker_(this, task);
}

NBroker::~NBroker(){
  delete x_;
}

NProcTask* NBroker::task(){
  return x_->task();
}

bool NBroker::listen(int port){
  return x_->listen(port);
}

void NBroker::distribute(NObject* object,
                         const nstr& className,
                         const nstr& objectName){
  x_->distribute(object, className, objectName);
}

void NBroker::revoke(const nstr& objectName){
  x_->revoke(objectName);
}

NObject* NBroker::obtain(const nstr& host,
                         int port,
                         const nstr& objectName,
                         const nvar& auth){
  return x_->obtain(host, port, objectName, auth);
}

void NBroker::release(NObject* object){
  x_->release(object);
}

void NBroker::setLogStream(std::ostream& ostr){
  x_->setLogStream(ostr);
}

void NBroker::setEncoder(NCommunicator::Encoder* encoder){
  x_->setEncoder(encoder);
}

nvar NBroker::process_(NObject* obj, const nvar& n){
  return x_->process_(obj, n);
}

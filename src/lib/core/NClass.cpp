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


The Neu Framework, Copyright (c) 2013-2015, Andrometa LLC
All rights reserved.

neu@andrometa.net
http://neu.andrometa.net

Neu can be used freely for commercial purposes. If you find Neu
useful, please consider helping to support our work and the evolution
of Neu by making a donation via: http://donate.andrometa.net

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

#include <neu/NClass.h>

#include <neu/NRWMutex.h>
#include <neu/NReadGuard.h>
#include <neu/NWriteGuard.h>

using namespace std;
using namespace neu;

namespace{
  
  class Global{
  public:
    Global(){}
    
    void registerClass(NClass* c){
      const nstr& name = c->name();
      const nstr& fullName = c->fullName();
      
      classMap_[name] = c;
      classMap_[fullName] = c;
    }

    void getClasses(nvec& classes){
      NMap<NClass*, bool> m;

      for(auto& itr : classMap_){
        NClass* c = itr.second;
      
        if(m.has(c)){
          continue;
        }
        
        classes.push_back(c->fullName());
        
        m[c] = true;
      }
    }
  
    NClass* getClass(const nstr& name){
      auto itr = classMap_.find(name);
      if(itr == classMap_.end()){
        return 0;
      }

      return itr->second;
    }
    
    NObjectBase* create(const nvar& f){
      auto itr = classMap_.find(f);
      
      if(itr == classMap_.end()){
        return 0;
      }
      
      return itr->second->construct(f);
    }
    
    NObjectBase* recreate(const nvar& v){
      auto itr = classMap_.find(v["type"]);
      
      if(itr == classMap_.end()){
        return 0;
      }
      
      return itr->second->reconstruct(v);
    }
    
    NObject* createRemote(const nstr& className, NBroker* broker){
      auto itr = classMap_.find(className);
      
      if(itr == classMap_.end()){
        return 0;
      }
      
      return itr->second->constructRemote(broker);
    }
    
  private:
    typedef NHashMap<nstr, NClass*> ClassMap_;
    
    ClassMap_ classMap_;
  };

  Global* _global = 0;
  NRWMutex _mutex;
  
} // end namespace

namespace neu{
  
  class NClass_{
  public:
    NClass_(NClass* o, const nstr& fullName)
    : o_(o),
    fullName_(fullName){
     
      nvec ns;
      fullName.split(ns, "::");
      name_ = ns.back();
      
      fullName_ = nstr::join(ns, "_");
    }
    
    const nstr& name(){
      return name_;
    }
    
    const nstr& fullName(){
      return fullName_;
    }
    
    const nvar& metadata(){
      return metadata_;
    }
    
    void setMetadata(const nvar& v){
      metadata_ = v;
    }
    
  private:
    NClass* o_;
    nstr name_;
    nstr fullName_;
    nvar metadata_;
  };
  
} // end namespace neu

NClass::NClass(const nstr& fullName){
  x_ = new NClass_(this, fullName);

  NWriteGuard guard(_mutex);
  
  if(!_global){
    _global = new Global;
  }
  
  _global->registerClass(this);
}

NClass::~NClass(){
  delete x_;
}

const nstr& NClass::name() const{
  return x_->name();
}

const nstr& NClass::fullName() const{
  return x_->fullName();
}

NObjectBase* NClass::create(const nvar& f){
  NReadGuard guard(_mutex);
  
  return _global->create(f);
}

NObjectBase* NClass::recreate(const nvar& v){
  NReadGuard guard(_mutex);
  
  return _global->recreate(v);
}

NObject* NClass::createRemote(const nstr& className, NBroker* broker){
  NReadGuard guard(_mutex);
  
  return _global->createRemote(className, broker);
}

nvec NClass::getClasses(){
  nvec ret;
  
  NReadGuard guard(_mutex);
  _global->getClasses(ret);
  return ret;
}

NClass* NClass::getClass(const nstr& name){
  NReadGuard guard(_mutex);
  return _global->getClass(name);
}

const nvar& NClass::metadata() const{
  return x_->metadata();
}

void NClass::setMetadata(const nvar& v){
  x_->setMetadata(v);
}

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

#include <neu/NMObject.h>

#include <atomic>

#include <neu/NFuncMap.h>
#include <neu/NClass.h>
#include <neu/NScope.h>
#include <neu/NPool.h>
#include <neu/NCommand.h>
#include <neu/NMGenerator.h>
#include <neu/NMParser.h>
#include <neu/NRegex.h>
#include <neu/NBasicMutex.h>
#include <neu/NSys.h>
#include <neu/global.h>
#include <neu/NProgram.h>

using namespace std;
using namespace neu;

namespace{
  
  class CommandPool : public NPool<NCommand>{
  public:
    CommandPool()
    : NPool(2){}
    
    NCommand* init(){
      NCommand* cmd =
      new NCommand(_mathKernelPath,
                   NCommand::Input|NCommand::Output|NCommand::Error);
      
      cmd->setCloseSignal(3);
      return cmd;
    }
    
  };
  
  CommandPool _commandPool;
  
  class FuncMap : public NFuncMap{
  public:
    FuncMap();
    
    NMObject_* obj(void* o);
  };
  
  NFunc _runFunc;
  
  FuncMap _funcMap;
  
  class Class : public NClass{
  public:
    Class() : NClass("neu::NMObject"){}
    
    NObjectBase* construct(const nvar& f){
      switch(f.size()){
        case 0:
          return new NMObject;
        case 1:
          return new NMObject(static_cast<NScope*>(f[0].obj()));
        default:
          return 0;
      }
    }
  };
  
  Class _class;
  
  NRegex _outputRegex("[^]*Out\\[\\d+\\]//FullForm= ([^]+)"
                      "In\\[\\d+\\]:=[^]*");
  
  bool _initialized = false;
  NBasicMutex _mutex;
  
} // end namespace

namespace neu{
  
  class NMObject_{
  public:
    
    NMObject_(NMObject* o)
    : o_(o),
    generator_(o_){
      init();
    }
    
    NMObject_(NMObject* o, NScope* sharedScope)
    : o_(o),
    generator_(o_){
      init();
    }

    void init(){
      _mutex.lock();
      if(!_initialized){

        NProgram::require();
        
        if(_mathKernelPath.empty()){
          NERROR("unspecified _mathKernelPath");
        }
        
        if(!NSys::exists(_mathKernelPath)){
          NERROR("invalid _mathKernelPath: " + _mathKernelPath);
        }

        _initialized = true;
      }
      _mutex.unlock();
    }
    
    NFunc handle(const nvar& v, uint32_t flags){
      NMGenerator::Type t = type(v);
      
      if(t == NMGenerator::Requested ||
         (t & NMGenerator::Supported && flags & NObject::Delegated)){
        v.setFunc(_runFunc);
        return _runFunc;
      }
      
      return o_->NObject::handle(v, flags);
    }
    
    NMGenerator::Type type(const nvar& v){
      switch(v.type()){
        case nvar::Function:{
          NMGenerator::Type t = NMGenerator::type(v);
          
          if(!(t & NMGenerator::Supported) || t == NMGenerator::Requested){
            return t;
          }
          
          size_t size = v.size();
          for(size_t i = 0; i < size; ++i){
            NMGenerator::Type ti = type(v[i]);
            
            if(!(ti & NMGenerator::Supported)){
              return ti;
            }
            else if(ti == NMGenerator::Requested){
              t = ti;
            }
          }
          
          return t;
        }
        case nvar::Symbol:
          return o_->Get(v) == none ?
          NMGenerator::Requested : NMGenerator::Supported;
      }
      
      return NMGenerator::Supported;
    }
    
    nvar run_(const char* fs, const nvec& v){
      nvar f = nvar(fs, nvar::Func);
      f.append(v);
      
      stringstream sstr;
      generator_.generate(sstr, f);

      NCommand* cmd = _commandPool.acquire();
      
      cmd->write(sstr.str());
      
      nvec m;
      
      if(!cmd->matchOutput(_outputRegex, m, 10.0)){
        delete cmd;
        _commandPool.release(0);
        
        NERROR("failed to run: " + v);
      }
      
      _commandPool.release(cmd);
      
      //cout << "out: " << m[1] << endl;
      
      return parser_.parse(m[1]);
    }
    
    static NMObject_* obj(void* o){
      return static_cast<NMObject*>(o)->x_;
    }
    
  private:
    NMObject* o_;
    NMParser parser_;
    NMGenerator generator_;
  };
  
} // end namespace neu

FuncMap::FuncMap(){
  _runFunc = [](void* o, const char* f, nvec& v) -> nvar{
    return NMObject_::obj(o)->run_(f, v);
  };
}

NMObject::NMObject(){
  x_ = new NMObject_(this);
  setStrict(false);
  setExact(true);
}

NMObject::NMObject(NScope* sharedScope)
: NObject(sharedScope){
  x_ = new NMObject_(this, sharedScope);
  setStrict(false);
  setExact(true);
}

NMObject::~NMObject(){
  delete x_;
}

NFunc NMObject::handle(const nvar& v, uint32_t flags){
  return x_->handle(v, flags);
}

void NMObject::setMathKernelPath(const nstr& path){
  _mathKernelPath = path;
}

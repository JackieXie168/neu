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

#include <neu/NHSObject.h>

#include <sstream>
#include <fstream>

#include <neu/NFuncMap.h>
#include <neu/NClass.h>
#include <neu/NScope.h>
#include <neu/NCommand.h>
#include <neu/NHSGenerator.h>
#include <neu/NHSParser.h>
#include <neu/NBasicMutex.h>
#include <neu/NProgram.h>
#include <neu/NSys.h>
#include <neu/NRegex.h>

using namespace std;
using namespace neu;

namespace{
  
  nstr _haskellPath;
  NBasicMutex _mutex;
  bool _initialized;
  
  class FuncMap : public NFuncMap{
  public:
    FuncMap();
    
    NHSObject_* obj(void* o);
  };
  
  NFunc _runFunc;
  
  FuncMap _funcMap;
  
  class Class : public NClass{
  public:
    Class() : NClass("neu::NHSObject"){}
    
    NObjectBase* construct(const nvar& f){
      switch(f.size()){
        case 0:
          return new NHSObject;
        case 1:
          return new NHSObject(static_cast<NScope*>(f[0].obj()));
        default:
          return 0;
      }
    }
  };
  
  Class _class;
  
  NRegex _awaitRegex("[^]*(.+?\\.hs:\\d+:\\d+:([^]*)\\bneu_haskell> )"
                     "|([^]*)\\bneu_haskell> ");
  
  NRegex _outputRegex("[^]*?(<interactive>:\\d+:\\d+:[^]*)\\bneu_haskell> "
                      "|([^]*)\\bneu_haskell> ");
  
} // end namespace

namespace neu{
  
  class NHSObject_{
  public:
    
    NHSObject_(NHSObject* o)
    : o_(o),
    generator_(o_){
      init();
    }
    
    NHSObject_(NHSObject* o, NScope* sharedScope)
    : o_(o),
    generator_(o_){
      init();
    }
    
    ~NHSObject_(){
      delete cmd_;
    }
    
    void init(){
      _mutex.lock();
      if(!_initialized){
        
        NProgram::use();
        
        if(_haskellPath.empty()){
          NERROR("unspecified Haskell path");
        }
        
        if(!NSys::exists(_haskellPath)){
          NERROR("invalid Haskell path: " + _haskellPath);
        }
        
        _initialized = true;
      }
      _mutex.unlock();
      
      nstr c = _haskellPath + " -ignore-dot-ghci";
      
      cmd_ =
      new NCommand(c, NCommand::Input|NCommand::Output|NCommand::Error);
      
      cmd_->write(":set prompt \"neu_haskell> \"\n");
      
      awaitPrompt();
    }
    
    NFunc handle(const nvar& v, uint32_t flags){
      NHSGenerator::Type t = type(v);
      
      if(t == NHSGenerator::Requested ||
         (t & NHSGenerator::Supported && flags & NObject::Delegated)){
        v.setFunc(_runFunc);
        return _runFunc;
      }
      
      return o_->NObject::handle(v, flags);
    }
    
    NHSGenerator::Type type(const nvar& v){
      switch(v.type()){
        case nvar::Function:{
          NHSGenerator::Type t = NHSGenerator::type(v);
          
          if(!(t & NHSGenerator::Supported) || t == NHSGenerator::Requested){
            return t;
          }
          
          size_t size = v.size();
          for(size_t i = 0; i < size; ++i){
            NHSGenerator::Type ti = type(v[i]);
            
            if(!(ti & NHSGenerator::Supported)){
              return ti;
            }
            else if(ti == NHSGenerator::Requested){
              t = ti;
            }
          }
          
          return t;
        }
        case nvar::Symbol:
          return o_->Get(v).isNone() ?
          NHSGenerator::Requested : NHSGenerator::Supported;
      }
      
      return NHSGenerator::Supported;
    }
    
    nvar handleSymbol(const nstr& s){
      cmd_->write(s);
      cmd_->write("\n");
      
      nstr out;
      nstr err = getOutput(out);
      if(!err.empty()){
        NERROR("error: " + err);
      }
      
      nvar r;
      if(out.empty()){
        r = none;
      }
      else{
        r = parser_.parse(out);
        if(r.isNone()){
          NERROR("failed to process [3]");
        }
      }
      
      return r;
    }
    
    nvar run_(const nstr& fs, const nvec& v){
      nvar f = nvar(fs, nvar::Func);
      f.append(v);
      
      stringstream sstr;
      generator_.generate(sstr, f);
      
      //cout << "hs code: " << sstr.str() << endl;
      
      cmd_->write(sstr.str());
      cmd_->write("\n");
      
      nstr out;
      nstr err = getOutput(out);
      if(!err.empty()){
        NERROR("error: " + err);
      }
      
      nvar r;
      if(out.empty()){
        r = none;
      }
      else{
        r = parser_.parse(out);
        if(r.isNone()){
          NERROR("failed to process [1]");
        }
      }
      
      return r;
    }
    
    void load(const nvar& n){
      stringstream ostr;
      
      generator_.generate(ostr, n);
      
      nstr code = ostr.str();
      code += "\n\n";
      
      //cout << "------------------" << endl;
      //cout << code << endl;
      //cout << "------------------" << endl;
      
      nstr tmpPath = NSys::tempFilePath("hs");
      
      FILE* file = fopen(tmpPath.c_str(), "w+");
      
      fwrite(code.c_str(), 1, code.length(), file);
      fclose(file);
      
      cmd_->write(":load " + tmpPath + "\n");
      
      nstr err = awaitPrompt();
      
      if(!err.empty()){
        ifstream istr(tmpPath.c_str());
        
        err += "\n\n";
        
        size_t i = 1;
        string line;
        while(getline(istr, line)){
          err += nstr::toStr(i) + ": ";
          err += line + "\n";
          ++i;
        }
        
        remove(tmpPath.c_str());
        
        NERROR("load error: " + err);
      }
      
      remove(tmpPath.c_str());
    }
    
    nstr awaitPrompt(){
      nvec m;
      
      cmd_->matchOutput(_awaitRegex, m);
      
      if(m[3].str().empty()){
        nstr& err = m[2];
        err.strip();
        return err;
      }
      
      return "";
    }
    
    nstr getOutput(nstr& str, bool expr=true){
      nvec m;
      
      cmd_->matchOutput(_outputRegex, m);
      
      if(m[2].str().empty()){
        nstr& err = m[1];
        err.strip();
        return err;
      }
      
      str = m[2];
      str.strip();
      
      size_t pos = str.find("\n");
      
      if(pos != nstr::npos){
        if(expr && !str.empty()){
          str.insert(0, "<<<<<");
        }
        str += ">>>>>";
      }
      else{
        if(expr && !str.empty()){
          str.insert(0, "EXPR_ ");
        }
      }
      
      str += "\n";
      
      return "";
    }
    
    static NHSObject_* obj(void* o){
      return static_cast<NHSObject*>(o)->x_;
    }
    
  private:
    NHSObject* o_;
    NHSParser parser_;
    NHSGenerator generator_;
    NCommand* cmd_;
  };
  
} // end namespace neu

FuncMap::FuncMap(){
  _runFunc = [](void* o, const nstr& f, nvec& v) -> nvar{
    return NHSObject_::obj(o)->run_(f, v);
  };
}

NHSObject::NHSObject(){
  x_ = new NHSObject_(this);
  setHandleSymbol(true);
}

NHSObject::NHSObject(NScope* sharedScope)
: NObject(sharedScope){
  x_ = new NHSObject_(this, sharedScope);
  setHandleSymbol(true);
}

NHSObject::~NHSObject(){
  delete x_;
}

NFunc NHSObject::handle(const nvar& v, uint32_t flags){
  return x_->handle(v, flags);
}

nvar NHSObject::handleSymbol(const nstr& s){
  return x_->handleSymbol(s);
}

void NHSObject::setHaskellPath(const nstr& path){
  _haskellPath = path;
}

void NHSObject::load(const nvar& n){
  x_->load(n);
}

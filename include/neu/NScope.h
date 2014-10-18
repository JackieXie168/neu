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

#ifndef NEU_N_SCOPE_H
#define NEU_N_SCOPE_H

#include <neu/NObjectBase.h>
#include <neu/nvar.h>
#include <neu/NRWMutex.h>

namespace neu{
  
  class NScope : public NObjectBase{
  public:
    static const uint32_t classId;
    
    NScope(bool limiting=false, bool shared=false)
    : limiting_(limiting),
    shared_(shared ? new Shared_ : 0){}
    
    ~NScope(){}
    
    virtual bool instanceOf(uint32_t classId) const{
      return classId == NScope::classId;
    }
    
    bool isLimiting() const{
      return limiting_;
    }
    
    void clear(){
      symbolMap_.clear();
      functionMap_.clear();
    }
    
    void setSymbolFast(const nstr& s, const nvar& v){
      symbolMap_[s] = v;
    }
    
    void setSymbol(const nstr& s, const nvar& v){
      if(shared_){
        shared_->symbolMutex_.writeLock();
        symbolMap_[s] = v;
        shared_->symbolMutex_.unlock();
      }
      else{
        symbolMap_[s] = v;
      }
    }
        
    bool getSymbol(const nstr& s, nvar& v){
      if(shared_){
        shared_->symbolMutex_.readLock();
        
        auto itr = symbolMap_.find(s);
        if(itr == symbolMap_.end()){
          shared_->symbolMutex_.unlock();
          return false;
        }
        
        v = itr->second;
        shared_->symbolMutex_.unlock();
        return true;
      }
      
      auto itr = symbolMap_.find(s);
      if(itr == symbolMap_.end()){
        return false;
      }
      
      v = itr->second;
      return true;
    }
    
    void setFunction(const nvar& s, const nvar& b){
      if(shared_){
        shared_->functionMutex_.writeLock();
        functionMap_.insert({{s.str(), s.size()}, {s, b}});
        shared_->functionMutex_.unlock();
        return;
      }
      
      functionMap_.insert({{s.str(), s.size()}, {s, b}});
    }
    
    bool getFunction(const nstr& f, size_t arity, nvar& s, nvar& b){
      if(shared_){
        shared_->functionMutex_.readLock();
        auto itr = functionMap_.find({f, arity});
        if(itr == functionMap_.end()){
          shared_->functionMutex_.unlock();
          return false;
        }
        
        shared_->functionMutex_.unlock();
        s = itr->second.first;
        b = itr->second.second;
        return true;
      }
      
      auto itr = functionMap_.find({f, arity});
      if(itr == functionMap_.end()){
        return false;
      }
      
      s = itr->second.first;
      b = itr->second.second;
      return true;
    }
    
    void dump(){
      for(auto& itr : symbolMap_){
        std::cout << itr.first << ": " << itr.second << std::endl;
      }
      
      for(auto& itr : functionMap_){
        std::cout << itr.second.first << ": " << itr.second.second << std::endl;
      }
    }
    
  private:
    struct SymHash_{
      size_t operator()(const nstr& k) const{
        return std::hash<std::string>()(k.str());
      }
    };

    typedef NHashMap<nstr, nvar, SymHash_> SymbolMap_;
    
    typedef std::pair<nstr, int16_t> FuncKey_;
    
    struct FuncHash_{
      size_t operator()(const FuncKey_& k) const{
        return std::hash<std::string>()(k.first.str()) ^
        std::hash<int16_t>()(k.second);
      }
    };

    typedef NHashMap<FuncKey_, std::pair<nvar, nvar>, FuncHash_> FunctionMap_;
    
    SymbolMap_ symbolMap_;
    FunctionMap_ functionMap_;

    bool limiting_ : 1;
    
    struct Shared_{
      mutable NRWMutex symbolMutex_;
      mutable NRWMutex functionMutex_;
    };
    
    mutable Shared_* shared_;
  };
  
} // end namespace neu

#endif // NEU_N_SCOPE_H

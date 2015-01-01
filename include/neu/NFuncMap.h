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

#ifndef NEU_N_FUNC_MAP_H
#define NEU_N_FUNC_MAP_H

#include <neu/nvar.h>

namespace neu{
  
  class NFuncMap{
  public:
    NFuncMap(){}
    
    ~NFuncMap(){}
    
    NFunc add(const nstr& func, NFunc fp){
      functorMap_.insert({{func, -1}, fp});
      return fp;
    }
    
    NFunc add(const nstr& func, size_t arity, NFunc fp){
      functorMap_.insert({{func, arity}, fp});
      return fp;
    }
    
    NFunc map(const nvar& f) const{
      assert(f.fullType() == nvar::Function);
      
      auto itr = functorMap_.find({f.str(), f.size()});
      if(itr == functorMap_.end()){
        itr = functorMap_.find({f.str(), -1});
        if(itr == functorMap_.end()){
          return 0;
        }
      }
      
      f.setFunc(itr->second);
      return itr->second;
    }
    
  private:
    typedef std::pair<nstr, int16_t> FuncKey_;
    
    struct Hash_{
      size_t operator()(const FuncKey_& k) const{
        return std::hash<std::string>()(k.first.str()) ^
        std::hash<int16_t>()(k.second);
      }
    };
    
    typedef NHashMap<FuncKey_, NFunc, Hash_> FunctorMap_;
    
    FunctorMap_ functorMap_;
  };
  
} // end namespace neu

#endif // NEU_N_FUNC_MAP_H

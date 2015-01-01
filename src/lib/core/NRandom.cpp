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

#include <neu/NRandom.h>

#include <neu/NFuncMap.h>
#include <neu/NClass.h>

using namespace std;
using namespace neu;

namespace{
  
  class FuncMap : public NFuncMap{
  public:
    FuncMap(){
      
      add("timeSeed", 0,
          [](void* o, const nstr&, nvec& v) -> nvar{
            return obj(o)->timeSeed();
          });
      
      add("uniform", 0,
          [](void* o, const nstr&, nvec& v) -> nvar{
            return obj(o)->uniform();
          });
      
      add("uniform", 2,
          [](void* o, const nstr&, nvec& v) -> nvar{
            return obj(o)->uniform(v[0], v[1]);
          });
      
      add("equilikely", 2,
          [](void* o, const nstr&, nvec& v) -> nvar{
            return obj(o)->equilikely(v[0], v[1]);
          });
      
      add("exponential", 1,
          [](void* o, const nstr&, nvec& v) -> nvar{
            return obj(o)->exponential(v[0]);
          });
      
      add("normal", 2,
          [](void* o, const nstr&, nvec& v) -> nvar{
            return obj(o)->normal(v[0], v[1]);
          });
      
      add("bernoulli", 1,
          [](void* o, const nstr&, nvec& v) -> nvar{
            return obj(o)->bernoulli(v[0]);
          });
      
      add("binomial", 2,
          [](void* o, const nstr&, nvec& v) -> nvar{
            return obj(o)->binomial(v[0], v[1]);
          });
      
      add("poisson", 1,
          [](void* o, const nstr&, nvec& v) -> nvar{
            return obj(o)->poisson(v[0]);
          });
      
      add("expSelect", 2,
          [](void* o, const nstr&, nvec& v) -> nvar{
            return obj(o)->expSelect(v[0], v[1]);
          });
    }
    
    static NRandom* obj(void* o){
      return static_cast<NRandom*>(o);
    }
  };
  
  FuncMap _funcMap;
  
  class Class : public NClass{
  public:
    Class() : NClass("neu::NRandom"){}
    
    NObjectBase* construct(const nvar& f){
      switch(f.size()){
        case 0:
          return new NRandom;
        case 1:
          return new NRandom(f[0]);
        default:
          return 0;
      }
    }
  };
  
  Class _class;
  
} // end namespace

NFunc NRandom::handle(const nvar& v, uint32_t flags){
  return _funcMap.map(v) ? : 0;
}

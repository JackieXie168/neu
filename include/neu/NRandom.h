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

#ifndef NEU_N_RANDOM_H
#define NEU_N_RANDOM_H

#include <random>
#include <cmath>

#include <neu/NObject.h>
#include <neu/NSys.h>

namespace neu{
  
  class NRandom : public NObject{
  public:
    NRandom(uint64_t seed=0)
    : uniform_(0, 1){
      rng_.seed(seed);
      
    }
    
    ~NRandom(){
      
    }
    
    void setSeed(int64_t seed){
      rng_.seed(seed);
    }
    
    uint64_t timeSeed(){
      double t = NSys::now();
      
      double sec = std::floor(t);
      double fsec = t - sec;
      
      uint64_t seed = sec * 10000000 + fsec*1000;
      
      rng_.seed(seed);
      
      return seed;
    }
    
    double uniform(){
      return uniform_(rng_);
    }
    
    // b > a
    double uniform(double a, double b){
      return a + (b - a) * uniform();
    }
    
    // b > a
    int64_t equilikely(int64_t a, int64_t b){
      return a + int64_t((b - a + 1) * uniform());
    }
    
    // x > 0
    double exponential(const double x){
      return -1.0/x * std::log(1.0 - uniform());
    }
    
    // s > 0
    double normal(double m, double s){
      std::normal_distribution<double> d(m, s);
      
      return d(rng_);
    }
    
    // 0 <= p <= 1
    bool bernoulli(double p){
      return uniform() < p ? true : false;
    }
    
    // n > 0; 0 <= p <= 1
    int64_t binomial(int64_t n, double p){
      int64_t y;
      
      for(size_t i = 0; i < n; ++i){
        y += bernoulli(p) ? 1 : 0;
      }
      
      return y;
    }
    
    // m > 0
    int64_t poisson(double m){
      int64_t y = 0;
      double t = std::exp(m);
      
      do{
        ++y;
        t *= uniform();
      }while(t >= 1.0);
      
      return y - 1;
    }
    
    // b > 0; s > 0
    uint64_t expSelect(uint64_t b, double s){
      uint64_t ret;
      do{
        ret = uint64_t(exponential(s * 10 / b));
      }while(ret > b);
      
      return ret; 
    }
    
    void randomSequence(int64_t a, int64_t b, nvar& seq){
      if(b < a){
        NERROR("b < a");
      }
      
      seq.intoVector();
      nvec& v = seq;
      v.reserve(b - a);
      
      for(int64_t i = a; i <= b; ++i){
        size_t j = equilikely(0, v.size());
        
        auto itr = v.begin();
        advance(itr, j);
        v.insert(itr, i);
      }
    }

    nvar randomSequence(int64_t a, int64_t b){
      nvar seq;
      randomSequence(a, b, seq);
      return seq;
    }
    
    NFunc handle(const nvar& v, uint32_t flags=0);
    
  private:
    std::mt19937_64 rng_;
    std::uniform_real_distribution<double> uniform_;
  };
  
} // end namespace neu

#endif // NEU_N_RANDOM_H

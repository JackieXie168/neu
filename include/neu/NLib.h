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

#ifndef NEU_N_LIB_H
#define NEU_N_LIB_H

#include <neu/NObject.h>
#include <neu/NProgram.h>
#include <neu/NSys.h>

namespace neu{
  
  class NLib : public NObject{
  public:
    NLib();
    
    ~NLib();
    
    static double pi(){
      return M_PI;
    }
    
    static double sqrt(double v){
      return std::sqrt(v);
    }
    
    static double pow(double v, double exponent){
      return std::pow(v, exponent);
    }
    
    static double abs(double v){
      return std::fabs(v);
    }
    
    static double log10(double v){
      return std::log10(v);
    }
    
    static double cos(double v){
      return std::cos(v);
    }
    
    static double acos(double v){
      return std::acos(v);
    }
    
    static double cosh(double v){
      return std::cosh(v);
    }
    
    static double sin(double v){
      return std::sin(v);
    }
    
    static double asin(double v){
      return std::asin(v);
    }
    
    static double sinh(double v){
      return std::sinh(v);
    }
    
    static double tan(double v){
      return std::tan(v);
    }
    
    static double atan(double v){
      return std::atan(v);
    }
    
    static double atan2(double y, double x){
      return std::atan2(y, x);
    }
    
    static double tanh(double v){
      return std::tanh(v);
    }
    
    static double inf(){
      return nvar::inf();
    }
    
    static double negInf(){
      return nvar::negInf();
    }
    
    static double floor(double v){
      return std::floor(v);
    }
    
    static double ceil(double v){
      return std::ceil(v);
    }
    
    static double round(double v){
      return std::round(v);
    }
    
    static double distance(const nvar& u, const nvar& v){
      size_t size = u.size();
      
      if(size != v.size()){
        NERROR("size mismatch");
      }
      
      double x = 0;
      for(size_t i = 0; i < size; ++i){
        double d = u[i] - v[i];
        x += d*d;
      }
      
      return std::sqrt(x);
    }
    
    static nvar vectorRange(const nvar& b){
      return vectorRange(0, b, 1);
    }
    
    static nvar vectorRange(const nvar& a, const nvar& b){
      return vectorRange(a, b, 1);
    }
    
    static nvar vectorRange(const nvar& a, const nvar& b, const nvar& inc){
      nvar ret;
      nvar pos = a;
      
      while(pos <= b){
        ret << pos;
        pos += inc;
      }
      
      return ret;
    }
    
    static nvar listRange(const nvar& b){
      return listRange(0, b, 1);
    }
    
    static nvar listRange(const nvar& a, const nvar& b){
      return listRange(a, b, 1);
    }
    
    static nvar listRange(const nvar& a, const nvar& b, const nvar& inc){
      nvar ret;
      ret.intoList();
      nvar pos = a;
      
      while(pos <= b){
        ret << pos;
        pos += inc;
      }
      
      return ret;
    }
    
    static nvar clamp(const nvar& v, const nvar& min, const nvar& max){
      if(v < min){
        return min;
      }
      else if(v > max){
        return max;
      }
      
      return v;
    }
    
    static void sleep(double dt){
      NSys::sleep(dt);
    }
    
    static double now(){
      return NSys::now();
    }
    
    static nvar args(){
      return NProgram::args();
    }
    
    NFunc handle(const nvar& v, uint32_t flags=0);
    
  private:
    class NLib_* x_;
  };
  
} // end namespace neu

#endif // NEU_N_LIB_H

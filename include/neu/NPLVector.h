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

#ifndef NEU_NPL_VECTOR_H
#define NEU_NPL_VECTOR_H

#include <neu/nvar.h>

#include <cmath>

namespace neu{

  template<typename T, size_t N>
  class NPLVector{
  public:
    NPLVector(){
      
    }

    NPLVector(std::initializer_list<T> il){
      std::copy(il.begin(), il.end(), vc_);
    }
   
    NPLVector(const nvec& v){
      if(v.size() != N){
        NERROR("size mismatch");
      }
      
      for(size_t i = 0; i < N; ++i){
        vc_[i] = v[i];
      }
    }

    NPLVector(const nvar& v){
      if(v.size() != N){
        NERROR("size mismatch");
      }
      
      for(size_t i = 0; i < N; ++i){
        vc_[i] = v[i];
      }
    }
    
    NPLVector<T,N>& operator=(std::initializer_list<T> il){
      std::copy(il.begin(), il.end(), vc_);

      return *this;
    }

    NPLVector<T,N>& operator=(const nvec& v){
      if(v.size() != N){
        NERROR("size mismatch");
      }
      
      for(size_t i = 0; i < size(); ++i){
        vc_[i] = v[i];
      }
      
      return *this;
    }

    NPLVector<T,N>& operator=(const nvar& v){
      if(v.size() != N){
        NERROR("size mismatch");
      }
      
      for(size_t i = 0; i < N; ++i){
        vc_[i] = v[i];
      }
      
      return *this;
    }
    
    NPLVector<T,N>& operator=(T x){
      for(size_t i = 0; i < N; ++i){
        vc_[i] = x;
      }
      
      return *this;
    }
    
    static size_t size(){
      return N;
    }

    T& operator[](size_t i){
      return vc_[i];
    }

    const T& operator[](size_t i) const{
      return vc_[i];
    }
    
    NPLVector<T,N>& operator+=(const NPLVector<T,N>& v){
      for(size_t i = 0; i < N; ++i){
        vc_[i] += v.vc_[i];
      }
      
      return *this;
    }

    NPLVector<T,N>& operator+=(T x){
      for(size_t i = 0; i < N; ++i){
        vc_[i] += x;
      }
      
      return *this;
    }
    
    NPLVector<T,N>& operator-=(const NPLVector<T,N>& v){
      for(size_t i = 0; i < N; ++i){
        vc_[i] -= v.vc_[i];
      }
      
      return *this;
    }

    NPLVector<T,N>& operator-=(T x){
      for(size_t i = 0; i < N; ++i){
        vc_[i] -= x;
      }
      
      return *this;
    }
    
    NPLVector<T,N>& operator*=(const NPLVector<T,N>& v){
      for(size_t i = 0; i < N; ++i){
        vc_[i] *= v.vc_[i];
      }
      
      return *this;
    }
    
    NPLVector<T,N>& operator*=(T x){
      for(size_t i = 0; i < N; ++i){
        vc_[i] *= x;
      }
      
      return *this;
    }
    
    NPLVector<T,N>& operator/=(const NPLVector<T,N>& v){
      for(size_t i = 0; i < N; ++i){
        vc_[i] /= v.vc_[i];
      }
      
      return *this;
    }
    
    NPLVector<T,N>& operator/=(T x){
      for(size_t i = 0; i < N; ++i){
        vc_[i] /= x;
      }
      
      return *this;
    }

    void normalize(){
      T d = 0;
      
      for(size_t i = 0; i < N; ++i){
        d += vc_[i] * vc_[i];
      }
      
      d = sqrt(d);
      
      for(size_t i = 0; i < N; ++i){
        vc_[i] /= d;
      }
    }
    
    T magnitude() const{
      T d = 0;

      for(size_t i = 0; i < N; ++i){
        d += vc_[i] * vc_[i];
      }

      return sqrt(d);
    }
    
  private:
    T vc_[N] __attribute__ ((aligned (16)));
  };
  
  template<typename T, size_t N>
  std::ostream& operator<<(std::ostream& ostr, const NPLVector<T,N>& v){
    ostr << "[";
    for(size_t i = 0; i < N; ++i){
      if(i > 0){
        ostr << ","; 
      }
      ostr << v[i];
    }
    ostr << "]";
    return ostr;
  }
  
  namespace npl{
    
    typedef NPLVector<uint8_t, 2> uchar2;
    typedef NPLVector<uint8_t, 3> uchar3;
    typedef NPLVector<uint8_t, 4> uchar4;
    typedef NPLVector<uint8_t, 8> uchar8;
    typedef NPLVector<uint8_t, 9> uchar9;
    typedef NPLVector<uint8_t, 16> uchar16;
    
    typedef NPLVector<int8_t, 2> char2;
    typedef NPLVector<int8_t, 3> char3;
    typedef NPLVector<int8_t, 4> char4;
    typedef NPLVector<int8_t, 8> char8;
    typedef NPLVector<int8_t, 9> char9;
    typedef NPLVector<int8_t, 16> char16;
    
    typedef NPLVector<uint16_t, 2> ushort2;
    typedef NPLVector<uint16_t, 3> ushort3;
    typedef NPLVector<uint16_t, 4> ushort4;
    typedef NPLVector<uint16_t, 8> ushort8;
    typedef NPLVector<uint16_t, 9> ushort9;
    typedef NPLVector<uint16_t, 16> ushort16;
    
    typedef NPLVector<int16_t, 2> short2;
    typedef NPLVector<int16_t, 3> short3;
    typedef NPLVector<int16_t, 4> short4;
    typedef NPLVector<int16_t, 8> short8;
    typedef NPLVector<int16_t, 9> short9;
    typedef NPLVector<int16_t, 16> short16;
    
    typedef NPLVector<uint32_t, 2> uint2;
    typedef NPLVector<uint32_t, 3> uint3;
    typedef NPLVector<uint32_t, 4> uint4;
    typedef NPLVector<uint32_t, 8> uint8;
    typedef NPLVector<uint32_t, 9> uint9;
    typedef NPLVector<uint32_t, 16> uint16;
    
    typedef NPLVector<int32_t, 2> int2;
    typedef NPLVector<int32_t, 3> int3;
    typedef NPLVector<int32_t, 4> int4;
    typedef NPLVector<int32_t, 8> int8;
    typedef NPLVector<int32_t, 9> int9;
    typedef NPLVector<int32_t, 16> int16;
    
    typedef NPLVector<uint64_t, 2> ulong2;
    typedef NPLVector<uint64_t, 3> ulong3;
    typedef NPLVector<uint64_t, 4> ulong4;
    typedef NPLVector<uint64_t, 8> ulong8;
    typedef NPLVector<uint64_t, 9> ulong9;
    typedef NPLVector<uint64_t, 16> ulong16;
    
    typedef NPLVector<int64_t, 2> long2;
    typedef NPLVector<int64_t, 3> long3;
    typedef NPLVector<int64_t, 4> long4;
    typedef NPLVector<int64_t, 8> long8;
    typedef NPLVector<int64_t, 9> long9;
    typedef NPLVector<int64_t, 16> long16;
    
    typedef NPLVector<float, 2> float2;
    typedef NPLVector<float, 3> float3;
    typedef NPLVector<float, 4> float4;
    typedef NPLVector<float, 8> float8;
    typedef NPLVector<float, 9> float9;
    typedef NPLVector<float, 16> float16;
    
    typedef NPLVector<double, 2> double2;
    typedef NPLVector<double, 3> double3;
    typedef NPLVector<double, 4> double4;
    typedef NPLVector<double, 8> double8;
    typedef NPLVector<double, 9> double9;
    typedef NPLVector<double, 16> double16;
    
    template<typename T, size_t N>
    NPLVector<T, N> crossProduct(const NPLVector<T, N>& u,
                                 const NPLVector<T, N>& v){
      NPLVector<T, N> r;
      
      r[0] = u[1]*v[2] - u[2]*v[1];
      r[1] = u[2]*v[0] - u[0]*v[2];
      r[2] = u[0]*v[1] - u[1]*v[0];
      
      return r;
    }
    
    template<typename T, size_t N>
    T dotProduct(const NPLVector<T, N>& u,
                 const NPLVector<T, N>& v){
      T r = 0;
      
      for(size_t i = 0; i < N; ++i){
        r += u[i]*v[i];
      }
      
      return r;
    }
    
    template<typename T, size_t N>
    T distance(const NPLVector<T, N>& u,
               const NPLVector<T, N>& v){
      T d = 0;
      for(size_t i = 0; i < N; ++i){
        T di = u[i] - v[i];
        di *= di;
        d += di;
      }
      return sqrt(d);
    }
    
  } // end namespace npl

} // end namespace neu

#endif // NEU_NPL_VECTOR_H

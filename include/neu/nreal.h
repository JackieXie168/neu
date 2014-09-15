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

#ifndef NEU_N_REAL_H
#define NEU_N_REAL_H

#include <iostream>
#include <ostream>

#include <neu/nstr.h>
#include <neu/nrat.h>

namespace neu{
  
  class nreal{
  public:
    
    nreal();
    
    explicit nreal(const char* s);
    
    nreal(double x);
    
    nreal(int64_t x);
    
    nreal(int x);
    
    nreal(const nreal& r);
    
    nreal(const nrat& r);
    
    ~nreal();
    
    nreal& operator=(const nreal& r);
    
    nreal& operator+=(const nreal& r);
    
    nreal operator+(const nreal& x) const{
      nreal ret(*this);
      return ret += x;
    }
    
    nreal operator++(int){
      nreal ret(*this);
      *this += 1;
      return ret;
    }
    
    nreal& operator++(){
      return *this += 1;
    }
    
    nreal& operator-=(const nreal& r);
    
    nreal operator-(const nreal& x) const{
      nreal ret(*this);
      return ret -= x;
    }
    
    nreal operator--(int){
      nreal ret(*this);
      *this -= 1;
      return ret;
    }
    
    nreal& operator--(){
      return *this -= 1;
    }
    
    nreal operator-() const;
    
    nreal& operator*=(const nreal& r);
    
    nreal operator*(const nreal& x) const{
      nreal ret(*this);
      return ret *= x;
    }
    
    nreal& operator/=(const nreal& r);
    
    nreal operator/(const nreal& x) const{
      nreal ret(*this);
      return ret /= x;
    }
    
    nreal& operator%=(const nreal& x);
    
    nreal operator%(const nreal& x) const{
      nreal ret(*this);
      return ret %= x;
    }
    
    bool operator<(const nreal& x) const;
    
    bool operator>(const nreal& x) const;
    
    bool operator<=(const nreal& x) const;
    
    bool operator>=(const nreal& x) const;
    
    bool operator==(const nreal& x) const;
    
    bool operator!=(const nreal& x) const;
    
    void setPrecision(size_t bits);
    
    static void setDefaultPrecision(size_t bits);
    
    static size_t defaultPrecision();
    
    double toDouble() const;
    
    int64_t toLong() const;
    
    nrat toRat() const;
    
    static nreal fromStr(const nstr& str);
    
    nstr toStr(bool exp=true, int precision=-1) const;
    
    static nreal cos(const nreal& x);
    
    static nreal sin(const nreal& x);
    
    static nreal tan(const nreal& x);
    
    static nreal acos(const nreal& x);
    
    static nreal asin(const nreal& x);
    
    static nreal atan(const nreal& x);
    
    static nreal atan2(const nreal& y, const nreal& x);
    
    static nreal cosh(const nreal& x);
    
    static nreal sinh(const nreal& x);
    
    static nreal tanh(const nreal& x);
    
    static nreal exp(const nreal& x);
    
    static nreal log(const nreal& x);
    
    static nreal log10(const nreal& x);
    
    static nreal pow(const nreal& base, const nreal& exponent);
    
    static nreal sqrt(const nreal& x);
    
    static nreal ceil(const nreal& x);
    
    static nreal floor(const nreal& x);
    
    static nreal abs(const nreal& x);
    
    static nreal pi();
    
    static nreal euler();
    
    static nreal catalan();
    
  private:
    class nreal_* x_;
  };
  
  inline bool operator<(double t, const nreal& x){
    return nreal(t) < x;
  }
  
  inline bool operator<(int t, const nreal& x){
    return nreal(t) < x;
  }
  
  inline bool operator<(int64_t t, const nreal& x){
    return nreal(t) < x;
  }
  
  inline bool operator<(const nrat& t, const nreal& x){
    return nreal(t) < x;
  }
  
  inline bool operator<=(double t, const nreal& x){
    return nreal(t) <= x;
  }
  
  inline bool operator<=(int t, const nreal& x){
    return nreal(t) <= x;
  }
  
  inline bool operator<=(int64_t t, const nreal& x){
    return nreal(t) <= x;
  }
  
  inline bool operator<=(const nrat& t, const nreal& x){
    return nreal(t) <= x;
  }
  
  inline bool operator>(double t, const nreal& x){
    return nreal(t) > x;
  }
  
  inline bool operator>(int t, const nreal& x){
    return nreal(t) > x;
  }
  
  inline bool operator>(int64_t t, const nreal& x){
    return nreal(t) > x;
  }
  
  inline bool operator>(const nrat& t, const nreal& x){
    return nreal(t) > x;
  }
  
  inline bool operator>=(double t, const nreal& x){
    return nreal(t) >= x;
  }
  
  inline bool operator>=(int t, const nreal& x){
    return nreal(t) >= x;
  }
  
  inline bool operator>=(int64_t t, const nreal& x){
    return nreal(t) >= x;
  }
  
  inline bool operator>=(const nrat& t, const nreal& x){
    return nreal(t) >= x;
  }
  
  inline bool operator==(double t, const nreal& x){
    return nreal(t) == x;
  }
  
  inline bool operator==(int t, const nreal& x){
    return nreal(t) == x;
  }
  
  inline bool operator==(int64_t t, const nreal& x){
    return nreal(t) == x;
  }
  
  inline bool operator==(const nrat& t, const nreal& x){
    return nreal(t) == x;
  }
  
  inline bool operator!=(double t, const nreal& x){
    return nreal(t) != x;
  }
  
  inline bool operator!=(int t, const nreal& x){
    return nreal(t) != x;
  }
  
  inline bool operator!=(int64_t t, const nreal& x){
    return nreal(t) != x;
  }
  
  inline bool operator!=(const nrat& t, const nreal& x){
    return nreal(t) != x;
  }
  
  inline nreal operator+(double t, const nreal& x){
    return nreal(t) + x;
  }
  
  inline nreal operator+(int t, const nreal& x){
    return nreal(t) + x;
  }
  
  inline nreal operator+(int64_t t, const nreal& x){
    return nreal(t) + x;
  }
  
  inline nreal operator+(const nrat& t, const nreal& x){
    return nreal(t) + x;
  }
  
  inline nreal operator-(double t, const nreal& x){
    return nreal(t) - x;
  }
  
  inline nreal operator-(int t, const nreal& x){
    return nreal(t) - x;
  }
  
  inline nreal operator-(int64_t t, const nreal& x){
    return nreal(t) - x;
  }
  
  inline nreal operator-(const nrat& t, const nreal& x){
    return nreal(t) - x;
  }
  
  inline nreal operator*(double t, const nreal& x){
    return nreal(t) * x;
  }
  
  inline nreal operator*(int t, const nreal& x){
    return nreal(t) * x;
  }
  
  inline nreal operator*(int64_t t, const nreal& x){
    return nreal(t) * x;
  }
  
  inline nreal operator*(const nrat& t, const nreal& x){
    return nreal(t) * x;
  }
  
  inline nreal operator/(double t, const nreal& x){
    return nreal(t) / x;
  }
  
  inline nreal operator/(int t, const nreal& x){
    return nreal(t) / x;
  }
  
  inline nreal operator/(int64_t t, const nreal& x){
    return nreal(t) / x;
  }
  
  inline nreal operator/(const nrat& t, const nreal& x){
    return nreal(t) / x;
  }
  
  inline nreal operator%(double t, const nreal& x){
    return nreal(t) % x;
  }
  
  inline nreal operator%(int t, const nreal& x){
    return nreal(t) % x;
  }
  
  inline nreal operator%(int64_t t, const nreal& x){
    return nreal(t) % x;
  }
  
  inline nreal operator%(const nrat& t, const nreal& x){
    return nreal(t) % x;
  }
  
  inline std::ostream& operator<<(std::ostream& ostr, const nreal& r){
    return ostr << r.toStr();
  }
  
} // end namespace neu

#endif // NEU_N_REAL_H

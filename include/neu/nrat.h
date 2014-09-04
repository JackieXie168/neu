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

#ifndef NEU_N_RAT_H
#define NEU_N_RAT_H

#include <cstddef>
#include <ostream>
#include <sstream>
#include <cmath>

#include <boost/rational.hpp>

#include <neu/nstr.h>

namespace neu{
  
  extern bool _improper;
  
  class nrat{
  public:
    nrat(){
      
    }
    
    nrat(const nrat& r)
    : r_(r.r_){
      
    }
    
    nrat(int numerator)
    : r_(numerator){
    }
    
    nrat(int64_t numerator)
    : r_(numerator){
      
    }
    
    nrat(int64_t numerator, int64_t denominator)
    : r_(numerator, denominator){
      
    }
    
    ~nrat(){
      
    }
    
    static nrat fromDouble(double f){
      int64_t sign = f < 0 ? -1 : 1;
      f = fabs(f);
      double df = 1;
      int64_t n = 1;
      int64_t d = 1;
      
      while(df != f){
        if(df < f){
          n += 1;
        }
        else{
          d += 1;
          n = f * d;
        }
        df = double(n)/d;
        if(d > 100000000){
          break;
        }
      }
      
      return nrat(sign*n, d);
    }
    
    int64_t numerator() const{
      return r_.numerator();
    }
    
    int64_t denominator() const{
      return r_.denominator();
    }
    
    nrat& operator=(const nrat& r){
      r_ = r.r_;
      return *this;
    }
    
    nrat& operator+=(const nrat& r){
      r_ += r.r_;
      return *this;
    }
    
    template<typename T>
    nrat operator+(const T& t) const{
      nrat ret(*this);
      return ret += nrat(t);
    }
    
    double operator+(double d) const{
      return this->toDouble() + d;
    }
    
    nrat operator+(const nrat& r) const{
      nrat ret(*this);
      return ret += r;
    }
    
    nrat operator++(int){
      nrat ret(*this);
      *this += 1;
      return ret;
    }
    
    nrat& operator++(){
      return *this += 1;
    }
    
    nrat& operator-=(const nrat& r){
      r_ -= r.r_;
      return *this;
    }
    
    template<typename T>
    nrat operator-(const T& t) const{
      nrat ret(*this);
      return ret -= nrat(t);
    }
    
    double operator-(double d) const{
      return this->toDouble() - d;
    }
    
    nrat operator-(const nrat& r) const{
      nrat ret(*this);
      return ret -= r;
    }
    
    nrat operator-() const{
      return nrat(-r_.numerator(), r_.denominator());
    }
    
    nrat operator--(int){
      nrat ret(*this);
      *this -= 1;
      return ret;
    }
    
    nrat& operator--(){
      return *this -= 1;
    }
    
    nrat& operator*=(const nrat& r){
      r_ *= r.r_;
      return *this;
    }
    
    template<typename T>
    nrat operator*(const T& t) const{
      nrat ret(*this);
      return ret *= nrat(t);
    }
    
    double operator*(double d) const{
      return this->toDouble() * d;
    }
    
    nrat operator*(const nrat& r) const{
      nrat ret(*this);
      return ret *= r;
    }
    
    nrat& operator/=(const nrat& r){
      r_ /= r.r_;
      return *this;
    }
    
    template<typename T>
    nrat operator/(const T& t) const{
      nrat ret(*this);
      return ret /= nrat(t);
    }
    
    double operator/(double d) const{
      return this->toDouble() / d;
    }
    
    nrat operator/(const nrat& r) const{
      nrat ret(*this);
      return ret /= r;
    }
    
    bool operator<(const nrat& r) const{
      return r_ < r.r_;
    }
    
    template<typename T>
    bool operator<(const T& t) const{
      return *this < nrat(t);
    }
    
    bool operator<(double d) const{
      return this->toDouble() < d;
    }
    
    bool operator>(const nrat& r) const{
      return r_ > r.r_;
    }
    
    template<typename T>
    bool operator>(const T& t) const{
      return *this > nrat(t);
    }
    
    bool operator>(double d) const{
      return this->toDouble() > d;
    }
    
    bool operator==(const nrat& r) const{
      return r_ == r.r_;
    }
    
    template<typename T>
    bool operator==(const T& t) const{
      return *this == nrat(t);
    }
    
    bool operator==(double d) const{
      return this->toDouble() == d;
    }
    
    bool operator<=(const nrat& r) const{
      return r_ <= r.r_;
    }
    
    template<typename T>
    bool operator<=(const T& t) const{
      return *this <= nrat(t);
    }
    
    bool operator<=(double d) const{
      return this->toDouble() <= d;
    }
    
    bool operator>=(const nrat& r) const{
      return r_ >= r.r_;
    }
    
    template<typename T>
    bool operator>=(const T& t) const{
      return *this >= nrat(t);
    }
    
    bool operator>=(double d) const{
      return this->toDouble() >= d;
    }
    
    bool operator!=(const nrat& r) const{
      return r_ != r.r_;
    }
    
    template<typename T>
    bool operator!=(const T& t) const{
      return *this != nrat(t);
    }
    
    bool operator!=(double d) const{
      return this->toDouble() != d;
    }
    
    int64_t toLong() const{
      return r_.numerator() / r_.denominator();
    }
    
    double toDouble() const{
      return r_.numerator() / static_cast<double>(r_.denominator());
    }
    
    bool toBool() const{
      return r_.numerator() != 0;
    }
    
    nstr toStr(int prec) const{
      std::stringstream ostr;
      
      int64_t n = numerator();
      int64_t d = denominator();
      
      if(_improper || abs(n) < d){
        int p = 7; // NObject::precedence("Div")
        if(p > prec){
          ostr << "(";
        }
        ostr << n << "/" << d;
        if(p > prec){
          ostr << ")";
        }
      }
      else{
        int64_t i = n/d;
        int p = 8; // NObject::precedence("Add")
        if(i < 0){
          ostr << "-(";
        }
        else if(p > prec){
          ostr << "(";
        }
        ostr << abs(i) << "+" << abs(n) % d << "/" << d;
        if(i < 0 || p > prec){
          ostr << ")";
        }
      }
      
      return ostr.str();
    }
    
  private:
    typedef boost::rational<int64_t> Rational_;
    
    Rational_ r_;
  };
  
  std::ostream& operator<<(std::ostream& ostr, const nrat& r);
  
  inline bool operator<(double d, const nrat& r){
    return d < r.toDouble();
  }
  
  inline bool operator<(int t, const nrat& r){
    return nrat(t) < r;
  }
  
  inline bool operator<(int64_t t, const nrat& r){
    return nrat(t) < r;
  }
  
  inline bool operator>(double d, const nrat& r){
    return d > r.toDouble();
  }
  
  inline bool operator>(int t, const nrat& r){
    return nrat(t) > r;
  }
  
  inline bool operator>(int64_t t, const nrat& r){
    return nrat(t) > r;
  }
  
  inline bool operator<=(double d, const nrat& r){
    return d <= r.toDouble();
  }
  
  inline bool operator<=(int t, const nrat& r){
    return nrat(t) <= r;
  }
  
  inline bool operator<=(int64_t t, const nrat& r){
    return nrat(t) <= r;
  }
  
  inline bool operator>=(double d, const nrat& r){
    return d >= r.toDouble();
  }
  
  inline bool operator>=(int t, const nrat& r){
    return nrat(t) >= r;
  }
  
  inline bool operator>=(int64_t t, const nrat& r){
    return nrat(t) >= r;
  }
  
  inline bool operator==(double d, const nrat& r){
    return d == r.toDouble();
  }
  
  inline bool operator==(int t, const nrat& r){
    return nrat(t) == r;
  }
  
  inline bool operator==(int64_t t, const nrat& r){
    return nrat(t) == r;
  }
  
  inline bool operator!=(double d, const nrat& r){
    return d == r.toDouble();
  }
  
  inline bool operator!=(int t, const nrat& r){
    return nrat(t) != r;
  }
  
  inline bool operator!=(int64_t t, const nrat& r){
    return nrat(t) != r;
  }
  
  inline double operator+(double d, const nrat& r){
    return d + r.toDouble();
  }
  
  inline nrat operator+(int t, const nrat& r){
    return nrat(t) + r;
  }
  
  inline nrat operator+(int64_t t, const nrat& r){
    return nrat(t) + r;
  }
  
  inline double operator-(double d, const nrat& r){
    return d - r.toDouble();
  }
  
  inline nrat operator-(int t, const nrat& r){
    return nrat(t) - r;
  }
  
  inline nrat operator-(int64_t t, const nrat& r){
    return nrat(t) - r;
  }
  
  inline double operator*(double d, const nrat& r){
    return d * r.toDouble();
  }
  
  inline nrat operator*(int t, const nrat& r){
    return nrat(t) * r;
  }
  
  inline nrat operator*(int64_t t, const nrat& r){
    return nrat(t) * r;
  }
  
  inline double operator/(double d, const nrat& r){
    return d / r.toDouble();
  }
  
  inline nrat operator/(int t, const nrat& r){
    return nrat(t) / r;
  }
  
  inline nrat operator/(int64_t t, const nrat& r){
    return nrat(t) / r;
  }
  
  inline std::ostream& operator<<(std::ostream& ostr, const nrat& r){
    return ostr << r.toStr(9999);
  }
  
} // end namespace neu

#endif // NEU_N_RAT_H

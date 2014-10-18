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

#include <neu/nreal.h>

#ifndef NEU_NO_PRECISE

#include <sstream>
#include <cstdlib>
#include <cstring>

#include <gmp.h>
#include <mpfr.h>

#include <neu/nvar.h>
#include <neu/global.h>

using namespace std;
using namespace neu;

namespace neu{
  
  class nreal_{
  public:
    
    nreal_(nreal* o)
    : o_(o){
      mpfr_init(r_);
    }
    
    nreal_(nreal* o, double x)
    : o_(o){
      
      mpfr_init_set_d(r_, x, GMP_RNDD);
    }
    
    nreal_(nreal* o, int64_t x)
    : o_(o){
      mpfr_init_set_ld(r_, x, GMP_RNDN);
    }
    
    nreal_(nreal* o, int x)
    : o_(o){
      mpfr_init_set_si(r_, x, GMP_RNDN);
    }
    
    ~nreal_(){
      mpfr_clear(r_);
    }
    
    bool fromStr(const nstr& str){
      if(mpfr_set_str(r_, str.c_str(), 10, GMP_RNDN) == 0){
        return true;
      }
      else{
        return false;
      }
    }
    
    nstr toStr(bool exp, int precision){
      char buf[8192];
      stringstream fstr;
      
      if(precision < 0){
        mpfr_sprintf(buf, "%.RNe", r_);
        
        size_t p = 0;
        size_t pl = 0;
        char c = 0;
        size_t len = strlen(buf);
        
        for(size_t i = 0; i < len; ++i){
          if(nstr::isDigit(buf[i])){
            ++p;
            if(buf[i] == '0' || buf[i] == '9'){
              if(buf[i] == c){
                ++pl;
              }
              else if(buf[i + 1] != 'e'){
                pl = 0;
              }
              
              c = buf[i];
            }
            else if(buf[i + 1] != 'e'){
              c = 0;
              pl = 0;
            }
          }
          else if(buf[i] == 'e'){
            break;
          }
        }
        
        if(exp){
          fstr << "%." << p - pl << "RNg";
        }
        else{
          fstr << "%." << p - pl << "RNf";
        }
        
        mpfr_sprintf(buf, fstr.str().c_str(), r_);
      }
      else{
        if(exp){
          fstr << "%." << precision << "RNg";
        }
        else{
          fstr << "%." << precision << "RNf";
        }
        mpfr_sprintf(buf, fstr.str().c_str(), r_);
      }
      return buf;
    }
    
    void setPrecision(size_t bits){
      if(bits < MPFR_PREC_MIN || bits > MPFR_PREC_MAX){
        NERROR("invalid precision" + nvar(bits));
      }
      
      mpfr_prec_round(r_, bits, GMP_RNDN);
    }
    
    double toDouble() const{
      return mpfr_get_d(r_, GMP_RNDN);
    }
    
    int64_t toLong() const{
#ifdef __APPLE__
      return mpfr_get_sj(r_, GMP_RNDN);
#else
      return mpfr_get_si(r_, GMP_RNDN);
#endif
    }
    
    nrat toRat() const{
      char buf[32];
      mpfr_sprintf(buf, "%.16RNf", r_);
      
      nstr s = buf;
      size_t i = s.find(".");
      int64_t denominator = 1;
      bool neg = false;
      if(s.findReplace("-", "", false) != 0){
        neg = true;
        --i;
      }
      if(i != nstr::npos){
        for(size_t j = 0; j < s.length() - i - 1; ++j){
          denominator *= 10;
        }
        s.replace(i, 1, "");
      }
      int64_t numerator = atoll(s.c_str());
      
      if(neg){
        return nrat(-numerator, denominator);
      }
      else{
        return nrat(numerator, denominator);
      }
    }
    
    mpfr_t& real(){
      return r_;
    }
    
    void set(const nreal_& r){
      mpfr_set(r_, r.r_, GMP_RNDN);
    }
    
    void addA(const nreal_& r){
      mpfr_add(r_, r_, r.r_, GMP_RNDN);
    }
    
    void subA(const nreal_& r){
      mpfr_sub(r_, r_, r.r_, GMP_RNDN);
    }
    
    void mulA(const nreal_& r){
      mpfr_mul(r_, r_, r.r_, GMP_RNDN);
    }
    
    void divA(const nreal_& r){
      mpfr_div(r_, r_, r.r_, GMP_RNDN);
    }
    
    void modA(const nreal_& r){
      mpfr_fmod(r_, r_, r.r_, GMP_RNDN);
    }
    
    bool lt(const nreal_& r){
      return mpfr_less_p(r_, r.r_);
    }
    
    bool le(const nreal_& r){
      return mpfr_lessequal_p(r_, r.r_);
    }
    
    bool gt(const nreal_& r){
      return mpfr_greater_p(r_, r.r_);
    }
    
    bool ge(const nreal_& r){
      return mpfr_greaterequal_p(r_, r.r_);
    }
    
    bool eq(const nreal_& r){
      return mpfr_equal_p(r_, r.r_);
    }
    
    bool ne(const nreal_& r){
      return !eq(r);
    }
    
  private:
    nreal* o_;
    mpfr_t r_;
  };
  
} // end namespace neu

nreal::nreal(){
  x_ = new nreal_(this);
}

nreal::nreal(const char* s){
  x_ = new nreal_(this);
  if(!x_->fromStr(s)){
    NERROR("construction from string failed: " + s);
  }
}

nreal::nreal(double x){
  x_ = new nreal_(this, x);
}

nreal::nreal(int64_t x){
  x_ = new nreal_(this, x);
}

nreal::nreal(int x){
  x_ = new nreal_(this, x);
}

nreal::nreal(const nrat& r){
  x_ = new nreal_(this, r.numerator());
  nreal d = r.denominator();
  x_->divA(*d.x_);
}

nreal::nreal(const nreal& r){
  x_ = new nreal_(this);
  x_->set(*r.x_);
}

nreal::~nreal(){
  delete x_;
}

nreal& nreal::operator=(const nreal& r){
  x_->set(*r.x_);
  return *this;
}

nreal& nreal::operator+=(const nreal& r){
  x_->addA(*r.x_);
  return *this;
}

nreal& nreal::operator-=(const nreal& r){
  x_->subA(*r.x_);
  return *this;
}

nreal nreal::operator-() const{
  nreal ret;
  mpfr_neg(ret.x_->real(), x_->real(), GMP_RNDN);
  return ret;
}

nreal& nreal::operator*=(const nreal& r){
  x_->mulA(*r.x_);
  return *this;
}

nreal& nreal::operator/=(const nreal& r){
  x_->divA(*r.x_);
  return *this;
}

nreal& nreal::operator%=(const nreal& r){
  x_->modA(*r.x_);
  return *this;
}

bool nreal::operator<(const nreal& x) const{
  return x_->lt(*x.x_);
}

bool nreal::operator>(const nreal& x) const{
  return x_->gt(*x.x_);
}

bool nreal::operator<=(const nreal& x) const{
  return x_->le(*x.x_);
}

bool nreal::operator>=(const nreal& x) const{
  return x_->ge(*x.x_);
}

bool nreal::operator==(const nreal& x) const{
  return x_->eq(*x.x_);
}

bool nreal::operator!=(const nreal& x) const{
  return x_->ne(*x.x_);
}

void nreal::setPrecision(size_t bits){
  x_->setPrecision(bits);
}

void nreal::setDefaultPrecision(size_t bits){
  if(bits < MPFR_PREC_MIN || bits > MPFR_PREC_MAX){
    NERROR("invalid precision" + nvar(bits));
  }
  
  mpfr_set_default_prec(bits);
}

size_t nreal::defaultPrecision(){
  return mpfr_get_default_prec();
}

nreal nreal::fromStr(const nstr& str){
  nreal r;
  if(!r.x_->fromStr(str)){
    NERROR("construction from string failed: " + str);
  }
  return r;
}

nstr nreal::toStr(bool exp, int precision) const{
  return x_->toStr(exp, precision);
}

double nreal::toDouble() const{
  return x_->toDouble();
}

int64_t nreal::toLong() const{
  return x_->toLong();
}

nrat nreal::toRat() const{
  return x_->toRat();
}

nreal nreal::sqrt(const nreal& x){
  nreal ret;
  mpfr_sqrt(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::pow(const nreal& base, const nreal& exponent){
  nreal ret;
  mpfr_pow(ret.x_->real(), base.x_->real(), exponent.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::cos(const nreal& x){
  nreal ret;
  mpfr_cos(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::sin(const nreal& x){
  nreal ret;
  mpfr_sin(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::tan(const nreal& x){
  nreal ret;
  mpfr_tan(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::acos(const nreal& x){
  nreal ret;
  mpfr_acos(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::asin(const nreal& x){
  nreal ret;
  mpfr_asin(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::atan(const nreal& x){
  nreal ret;
  mpfr_atan(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::cosh(const nreal& x){
  nreal ret;
  mpfr_cosh(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::sinh(const nreal& x){
  nreal ret;
  mpfr_sinh(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::tanh(const nreal& x){
  nreal ret;
  mpfr_tanh(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::atan2(const nreal& y, const nreal& x){
  nreal ret;
  mpfr_atan2(ret.x_->real(), y.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::exp(const nreal& x){
  nreal ret;
  mpfr_exp(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::log(const nreal& x){
  nreal ret;
  mpfr_log(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::log10(const nreal& x){
  nreal ret;
  mpfr_log10(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::ceil(const nreal& x){
  nreal ret;
  mpfr_ceil(ret.x_->real(), x.x_->real());
  return ret;
}

nreal nreal::floor(const nreal& x){
  nreal ret;
  mpfr_floor(ret.x_->real(), x.x_->real());
  return ret;
}

nreal nreal::abs(const nreal& x){
  nreal ret;
  mpfr_abs(ret.x_->real(), x.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::pi(){
  nreal ret;
  mpfr_const_pi(ret.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::euler(){
  nreal ret;
  mpfr_const_euler(ret.x_->real(), GMP_RNDN);
  return ret;
}

nreal nreal::catalan(){
  nreal ret;
  mpfr_const_catalan(ret.x_->real(), GMP_RNDN);
  return ret;
}

#else // double-precision version

#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <neu/NError.h>

using namespace std;
using namespace neu;

namespace neu{
  
  class nreal_{
  public:
    
    nreal_(nreal* o)
    : o_(o){}
    
    nreal_(nreal* o, double x)
    : o_(o),
    r_(x){}
    
    nreal_(nreal* o, int64_t x)
    : o_(o),
    r_(x){}
    
    nreal_(nreal* o, int x)
    : o_(o),
    r_(x){}
    
    bool fromStr(const nstr& str){
      r_ = atof(str.c_str());
      return true;
    }
    
    nstr toStr(bool exp, int precision) const{
      if(precision < 0){
        precision = 16;
      }
      
      stringstream ostr;
      ostr.precision(precision);
      if(!exp){
        ostr.setf(ios::fixed);
      }
      ostr << r_;
      return ostr.str();
    }
    
    void setPrecision(size_t bits){}
    
    size_t defaultPrecision(){
      return 64;
    }
    
    double toDouble() const{
      return r_;
    }
    
    int64_t toLong() const{
      return r_;
    }
    
    nrat toRat() const{
      nstr s = toStr(false, 16);
      size_t i = s.find(".");
      int64_t denominator = 1;
      bool neg = false;
      if(s.findReplace("-", "", false) != 0){
        neg = true;
        --i;
      }
      if(i != nstr::npos){
        for(size_t j = 0; j < s.length() - i - 1; ++j){
          denominator *= 10;
        }
        s.replace(i, 1, "");
      }
      int64_t numerator = atoll(s.c_str());
      
      if(neg){
        return nrat(-numerator, denominator);
      }
      else{
        return nrat(numerator, denominator);
      }
    }
    
    void set(const nreal_& r){
      r_ = r.r_;
    }
    
    void addA(const nreal_& r){
      r_ += r.r_;
    }
    
    void subA(const nreal_& r){
      r_ -= r.r_;
    }
    
    void mulA(const nreal_& r){
      r_ *= r.r_;
    }
    
    void divA(const nreal_& r){
      r_ /= r.r_;
    }
    
    void modA(const nreal_& r){
      r_ = fmod(r_, r.r_);
    }
    
    bool lt(const nreal_& r){
      return r_ < r.r_;
    }
    
    bool le(const nreal_& r){
      return r_ <= r.r_;
    }
    
    bool gt(const nreal_& r){
      return r_ > r.r_;
    }
    
    bool ge(const nreal_& r){
      return r_ >= r.r_;
    }
    
    bool eq(const nreal_& r){
      return r_ == r.r_;
    }
    
    bool ne(const nreal_& r){
      return r_ != r.r_;
    }
    
  private:
    nreal* o_;
    double r_;
  };
  
} // end namespace neu

nreal::nreal(){
  x_ = new nreal_(this);
}

nreal::nreal(const char* s){
  x_ = new nreal_(this);
  if(!x_->fromStr(s)){
    NERROR("construction from string failed: " + nstr(s));
  }
}

nreal::nreal(double x){
  x_ = new nreal_(this, x);
}

nreal::nreal(int64_t x){
  x_ = new nreal_(this, x);
}

nreal::nreal(int x){
  x_ = new nreal_(this, x);
}

nreal::nreal(const nrat& r){
  x_ = new nreal_(this, r.numerator());
  nreal d = r.denominator();
  x_->divA(*d.x_);
}

nreal::nreal(const nreal& r){
  x_ = new nreal_(this);
  x_->set(*r.x_);
}

nreal::~nreal(){
  delete x_;
}

nreal& nreal::operator=(const nreal& r){
  x_->set(*r.x_);
  return *this;
}

nreal& nreal::operator+=(const nreal& r){
  x_->addA(*r.x_);
  return *this;
}

nreal& nreal::operator-=(const nreal& r){
  x_->subA(*r.x_);
  return *this;
}

nreal nreal::operator-() const{
  return -x_->toDouble();
}

nreal& nreal::operator*=(const nreal& r){
  x_->mulA(*r.x_);
  return *this;
}

nreal& nreal::operator/=(const nreal& r){
  x_->divA(*r.x_);
  return *this;
}

nreal& nreal::operator%=(const nreal& r){
  x_->modA(*r.x_);
  return *this;
}

bool nreal::operator<(const nreal& x) const{
  return x_->lt(*x.x_);
}

bool nreal::operator>(const nreal& x) const{
  return x_->gt(*x.x_);
}

bool nreal::operator<=(const nreal& x) const{
  return x_->le(*x.x_);
}

bool nreal::operator>=(const nreal& x) const{
  return x_->ge(*x.x_);
}

bool nreal::operator==(const nreal& x) const{
  return x_->eq(*x.x_);
}

bool nreal::operator!=(const nreal& x) const{
  return x_->ne(*x.x_);
}

void nreal::setPrecision(size_t bits){
  x_->setPrecision(bits);
}

void nreal::setDefaultPrecision(size_t bits){}

size_t nreal::defaultPrecision(){
  return 64;
}

nreal nreal::fromStr(const nstr& str){
  nreal r;
  if(!r.x_->fromStr(str)){
    NERROR("construction from string failed: " + str);
  }
  return r;
}

nstr nreal::toStr(bool exp, int precision) const{
  return x_->toStr(exp, precision);
}

double nreal::toDouble() const{
  return x_->toDouble();
}

int64_t nreal::toLong() const{
  return x_->toLong();
}

nrat nreal::toRat() const{
  return x_->toRat();
}

nreal nreal::sqrt(const nreal& x){
  return std::sqrt(x.toDouble());
}

nreal nreal::pow(const nreal& base, const nreal& exponent){
  return std::pow(base.toDouble(), exponent.toDouble());
}

nreal nreal::cos(const nreal& x){
  return std::sqrt(x.toDouble());
}

nreal nreal::sin(const nreal& x){
  return std::sin(x.toDouble());
}

nreal nreal::tan(const nreal& x){
  return std::tan(x.toDouble());
}

nreal nreal::acos(const nreal& x){
  return std::acos(x.toDouble());
}

nreal nreal::asin(const nreal& x){
  return std::asin(x.toDouble());
}

nreal nreal::atan(const nreal& x){
  return std::atan(x.toDouble());
}

nreal nreal::cosh(const nreal& x){
  return std::cosh(x.toDouble());
}

nreal nreal::sinh(const nreal& x){
  return std::sinh(x.toDouble());
}

nreal nreal::tanh(const nreal& x){
  return std::tanh(x.toDouble());
}

nreal nreal::atan2(const nreal& y, const nreal& x){
  return std::atan2(y.toDouble(), x.toDouble());
}

nreal nreal::exp(const nreal& x){
  return std::exp(x.toDouble());
}

nreal nreal::log(const nreal& x){
  return std::log(x.toDouble());
}

nreal nreal::log10(const nreal& x){
  return std::log10(x.toDouble());
}

nreal nreal::ceil(const nreal& x){
  return std::ceil(x.toDouble());
}

nreal nreal::floor(const nreal& x){
  return std::floor(x.toDouble());
}

nreal nreal::abs(const nreal& x){
  return std::abs(x.toDouble());
}

nreal nreal::pi(){
  return M_PI;
}

nreal nreal::euler(){
  return 0.5772156649015329;
}

nreal nreal::catalan(){
  return 0.9159655941772190;
}

#endif

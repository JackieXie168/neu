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

#ifndef N_CONCEPT_H
#define N_CONCEPT_H

#include <neu/NObject.h>

namespace neu{

class NConcept : public NObject{
public:
  enum NConceptType{
    NConceptPrototype
  };

  struct PrototypeFlag{};
  
  static PrototypeFlag* Prototype;
  
  NConcept(const NConcept& c);

  NConcept(PrototypeFlag*, const nvar& metadata);

  NConcept();

  virtual ~NConcept();

  virtual NConcept* create() const = 0;

  virtual NConcept* copy() const = 0;

  virtual NFunc handle(const nvar& n, uint32_t flags);

  virtual nstr name() const = 0;

  void setParamName(const nstr& paramName);

  const nstr& paramName() const;

  virtual nvar val() const = 0;

  nvar objectRep() const{
    return val();
  }

  void setObj(NConcept* c){
    set(c->val());
  }

  virtual void set(const nvar& v) = 0;

  virtual void validate(const nvar& v) = 0;

  virtual double match(NConcept* concept, bool full){
    return match(concept->attributes(), full);
  }

  virtual double match(const nvar& v, bool full);
  
  void setIn(bool flag);

  bool getIn() const;

  void setOut(bool flag);
  
  bool getOut() const;

  void setConst(bool flag);
  
  bool getConst() const;

  void setStatic(bool flag);
  
  bool getStatic() const;

  void setVec(bool flag);

  void undefVec();

  bool getVec() const;

  void setSet(bool flag);

  bool getSet() const;

  void undefSet();

  void setLength(size_t length);

  bool getLength() const;

  void setWeight(double weight);

  double getWeight() const;

  void undefLength();

  void setPoly(bool flag);

  bool getPoly() const;

  size_t getAge() const;

  size_t getInputUses() const;

  size_t getOutputUses() const;

  void setEnabled(bool flag);

  bool getEnabled() const;

  bool getGiven() const;

  void setTakeThis(bool flag);

  bool getTakeThis() const;

  void setCloneOf(const nstr& cloneOf);

  const nstr& getCloneOf() const;

  void setTakeCloneOf(const nstr& cloneOf);

  const nstr& getTakeCloneOf() const;

  void setDescription(const nvar& description);

  const nstr& getDescription() const;

  void setRemove(const nvar& value);

  const nvar& getRemove() const;

  const nvar& attributes() const;

  nvar operator+(const nvar& v) const{
    return val() + v;
  }
  
  template<typename T>
  nvar operator+(const T& t) const{
    return val() + nvar(t);
  }

  nvar operator-(const nvar& v) const{
    return val() - v;
  }

  template<typename T>
  nvar operator-(const T& t) const{
    return val() - nvar(t);
  }

  nvar operator*(const nvar& v) const{
    return val() * v;
  }

  template<typename T>
  nvar operator*(const T& t) const{
    return val() * nvar(t);
  }

  nvar operator/(const nvar& v) const{
    return val() / v;
  }

  template<typename T>
  nvar operator/(const T& t) const{
    return val() / nvar(t);
  }

  nvar operator%(const nvar& v) const{
    return val() % v;
  }

  template<typename T>
  nvar operator%(const T& t) const{
    return val() % nvar(t);
  }

  bool operator<(const nvar& v) const{
    return val() < v;
  }

  template<typename T>
  bool operator<(const T& t) const{
    return val() < nvar(t);
  }

  bool operator>(const nvar& v) const{
    return val() > v;
  }

  template<typename T>
  bool operator>(const T& t) const{
    return val() > nvar(t);
  }

  bool operator<=(const nvar& v) const{
    return val() <= v;
  }

  template<typename T>
  bool operator<=(const T& t) const{
    return val() <= nvar(t);
  }

  bool operator>=(const nvar& v) const{
    return val() >= v;
  }

  template<typename T>
  bool operator>=(const T& t) const{
    return val() >= nvar(t);
  }

  bool operator==(const nvar& v) const{
    return val() == v;
  }

  template<typename T>
  bool operator==(const T& t) const{
    return val() == nvar(t);
  }

  bool operator!=(const nvar& v) const{
    return val() != v;
  }

  template<typename T>
  bool operator!=(const T& t) const{
    return val() != nvar(t);
  }

  double toDouble() const{
    return val();
  }

  double toLong() const{
    return val();
  }

  operator nvar() const{
    return val();
  }

  operator double() const{
    return val();
  }

  operator uint8_t() const{
    return val();
  }
    
  operator int8_t() const{
    return val();
  }
    
  operator uint16_t() const{
    return val();
  }
    
  operator int16_t() const{
    return val();
  }
    
  operator uint32_t() const{
    return val();
  }
    
  operator int32_t() const{
    return val();
  }
    
  operator uint64_t() const{
    return val();
  }
    
  operator int64_t() const{
    return val();
  }
    
#if defined (__i386__) || defined (__APPLE__)      
    
  operator unsigned long int() const{
    return val();
  }
    
  operator long int() const{
    return val();
  }
    
#endif

  void setAttribute_(const nstr& key, const nvar& value);

  void eraseAttribute_(const nstr& key);

  void setStringAttribute_(const nstr& key, const nvar& value);

  void setStringMapAttribute_(const nstr& key, const nvar& value);

  const nvar& getAttribute_(const nstr& key) const;

  bool hasAttribute_(const nstr& key) const;

  static bool isNumeric(const nvar& v,
                        bool allowNegative=true);

  static bool isInteger(const nvar& v,
                        bool allowNegative=true);

private:
  class NConcept_* x_;
};

template<typename T>
inline nvar operator+(const T& t, const NConcept& v2){
  return nvar(t) + v2.val();
}

template<typename T>
inline nvar operator-(const T& t, const NConcept& v2){
  return nvar(t) - v2.val();
}

template<typename T>
inline nvar operator*(const T& t, const NConcept& v2){
  return nvar(t) * v2.val();
}

template<typename T>
inline nvar operator/(const T& t, const NConcept& v2){
  return nvar(t) / v2.val();
}

template<typename T>
inline nvar operator%(const T& t, const NConcept& v2){
  return nvar(t) % v2.val();
}

template<typename T>
inline bool operator<(const T& t, const NConcept& v2){
  return nvar(t) < v2.val();
}

template<typename T>
inline bool operator>(const T& t, const NConcept& v2){
  return nvar(t) > v2.val();
}

template<typename T>
inline bool operator<=(const T& t, const NConcept& v2){
  return nvar(t) <= v2.val();
}

template<typename T>
inline bool operator>=(const T& t, const NConcept& v2){
  return nvar(t) >= v2.val();
}

template<typename T>
inline bool operator==(const T& t, const NConcept& v2){
  return nvar(t) == v2.val();
}

template<typename T>
inline bool operator!=(const T& t, const NConcept& v2){
  return nvar(t) != v2.val();
}

inline std::ostream& operator<<(std::ostream& ostr, const NConcept& v){
  ostr << v.val();
  return ostr;
}

} // end namespace neu

#endif // NEU_N_CONCEPT_H


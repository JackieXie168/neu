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

#include <neu/NConcept.h>

#include <iostream>
#include <cmath>

#include <neu/NCOntology.h>

using namespace std;
using namespace neu;

namespace neu{
  
  NConcept::PrototypeFlag* NConcept::Prototype = 0;
  
  class NConcept_{
  public:
    NConcept_(NConcept* o)
    : o_(o){
      setIn(true);
      setOut(false);
      setConst(false);
      undefLength();
      setPoly(false);
      setEnabled(true);
    }
    
    NConcept_(NConcept* o, const NConcept_& c)
    : o_(o),
    attrs_(c.attrs_){}
    
    ~NConcept_(){}
    
    void setParamName(const nstr& paramName){
      paramName_ = paramName;
    }
    
    const nstr& paramName() const{
      return paramName_;
    }
    
    void setIn(bool flag){
      setAttribute_("in", flag);
    }
    
    bool getIn() const{
      return getAttribute_("in");
    }
    
    void setOut(bool flag){
      setAttribute_("out", flag);
    }
    
    bool getOut() const{
      return getAttribute_("out");
    }
    
    void setConst(bool flag){
      setAttribute_("const", flag);
    }
    
    bool getConst() const{
      return getAttribute_("const");
    }
    
    void setStatic(bool flag){
      setAttribute_("static", flag);
    }
    
    bool getStatic() const{
      return getAttribute_("static");
    }
    
    void setVec(bool flag){
      if(flag){
        setAttribute_("vec", true);
      }
      else{
        eraseAttribute_("vec");
      }
    }
    
    void undefVec(){
      setAttribute_("vec", undef);
    }
    
    bool getVec() const{
      return hasAttribute_("vec");
    }
    
    void setSet(bool flag){
      if(flag){
        setAttribute_("set", true);
        setVec(true);
      }
      else{
        eraseAttribute_("set");
      }
    }
    
    void undefSet(){
      setAttribute_("set", undef);
    }
    
    bool getSet() const{
      return hasAttribute_("set");
    }
    
    void setLength(size_t length){
      setAttribute_("length", length);
    }
    
    void undefLength(){
      setAttribute_("length", undef);
    }
    
    bool getLength() const{
      return getAttribute_("length");
    }
    
    void setWeight(double weight){
      setAttribute_("weight", weight);
    }
    
    double getWeight() const{
      return getAttribute_("weight");
    }
    
    void setPoly(bool flag){
      setAttribute_("poly", flag);
    }
    
    bool getPoly() const{
      return getAttribute_("poly");
    }
    
    size_t getAge() const{
      return getAttribute_("age");
    }
    
    size_t getInputUses() const{
      return getAttribute_("inputUses");
    }
    
    size_t getOutputUses() const{
      return getAttribute_("outputUses");
    }
    
    bool getGiven() const{
      return hasAttribute_("given");
    }
    
    void setEnabled(bool flag){
      setAttribute_("enabled", flag);
    }
    
    bool getEnabled() const{
      return getAttribute_("enabled");
    }
    
    void setTakeThis(bool flag){
      if(flag){
        setAttribute_("takeThis", true);
      }
      else{
        eraseAttribute_("takeThis");
      }
    }
    
    bool getTakeThis() const{
      return hasAttribute_("takeThis");
    }
    
    void setDescription(const nvar& description){
      setStringAttribute_("description", description);
    }
    
    const nstr& getDescription() const{
      return getAttribute_("description");
    }
    
    void setRemove(const nvar& value){
      setStringMapAttribute_("remove", value);
    }
    
    const nvar& getRemove() const{
      return getAttribute_("remove");
    }
    
    void setCloneOf(const nstr& cloneOf){
      setAttribute_("cloneOf", cloneOf);
    }
    
    const nstr& getCloneOf() const{
      return getAttribute_("cloneOf");
    }
    
    void setTakeCloneOf(const nstr& cloneOf){
      setAttribute_("takeCloneOf", cloneOf);
    }
    
    const nstr& getTakeCloneOf() const{
      return getAttribute_("takeCloneOf");
    }
    
    void error_(const nstr& key, const nvar& value){
      throw NError("NConcept: invalid attribute for key '" + key +
                   "' value: " + value.toStr());
    }
    
    void setAttribute_(const nstr& key, const nvar& value){
      attrs_(key) = value;
    }
    
    void eraseAttribute_(const nstr& key){
      attrs_.erase(key);
    }
    
    void setStringAttribute_(const nstr& key, const nvar& value){
      nstr str;
      
      if(value.isString()){
        str = value;
      }
      else{
        if(!value.hasVector()){
          error_(key, value);
        }
        
        for(size_t i = 0; i < value.size(); ++i){
          if(!value[i].isString()){
            error_(key, value);
          }
        }
        
        str = nstr::join(value.vec(), "; ");
      }
      
      attrs_(key) = str;
    }
    
    void setStringMapAttribute_(const nstr& key, const nvar& value){
      nvec values;
      
      if(value.isString()){
        values.push_back(value);
      }
      else{
        if(!value.hasVector()){
          error_(key, value);
        }
        
        for(size_t i = 0; i < value.size(); ++i){
          if(!value[i].isString()){
            error_(key, value);
          }
          
          values.push_back(value[i]);
        }
      }
      
      nvar& r = attrs_(key);
      for(const nstr& k : values){
        r(k) = true;
      }
    }
    
    const nvar& getAttribute_(const nstr& key) const{
      if(!attrs_.has(key)){
        throw NError("NConcept::getAttribute: undefined attribute: " + key);
      }
      
      return attrs_[key];
    }
    
    bool hasAttribute_(const nstr& key) const{
      return attrs_.has(key);
    }
    
    const nvar& attributes() const{
      return attrs_;
    }
    
    double match(const nvar& v, bool full){
      //cout << "attrs_ is: " << attrs_ << endl;
      //cout << "v is: " << v << endl;
      
      if(attrs_["in"] != v["in"]){
        //cout << "f1" << endl;
        return -1;
      }
      
      if(attrs_["out"] && !v["out"]){
        //cout << "a is: " << attrs_.out << endl;
        //cout << "b is: " << v.out << endl;
        
        //cout << "f2" << endl;
        return -1;
      }
      
      if(attrs_.has("vec")){
        if(!v.has("vec") && attrs_["vec"] != undef){
          return -1;
        }
      }
      else if(v.has("vec")){
        return -1;
      }
      
      if(attrs_.has("set")){
        if(!v.has("set") && attrs_["set"] != undef){
          return -1;
        }
      }
      else if(v.has("set")){
        return -1;
      }
      
      if(full){
        if(attrs_.has("takeCloneOf")){
          if(!v.has("cloneOf")){
            //cout << "f5" << endl;
            return -1;
          }
          
          if(attrs_["takeCloneOf"] != v["cloneOf"]){
            //cout << "f6" << endl;
            return -1;
          }
        }
      }
      else{
        return 1;
      }
      
      // no age bias for right now
      return 1;
      
      double age = v["age"];
      
      if(age > 100){
        return -1;
      }
      
      double m = pow((100.0 - age)/50.0, 2.0);
      
      //cout << "m is: " << m << endl;
      
      return m;
    }
    
  private:
    NConcept* o_;
    nstr paramName_;
    nvar attrs_;
  };
  
} // end namespace neu

NConcept::NConcept(const NConcept& c){
  x_ = new NConcept_(this, *c.x_);
}

NConcept::NConcept(PrototypeFlag*, const nvar& metadata){
  x_ = new NConcept_(this);
  
  NCOntology* ontology = NCOntology::get();
  ontology->addConcept(this, metadata);
}

bool NConcept::isNumeric(const nvar& v, bool allowNegative){
  if(v.hasVector()){
    for(const nvar& vi : v){
      if(vi == undef){
        return false;
      }
      
      if(!vi.isNumeric()){
        return false;
      }
      
      if(!allowNegative && vi < 0){
        return false;
      }
    }
  }
  else if(v.hasSet()){
    const nset& s = v;
    for(const nvar& vi : v){
      if(vi == undef){
        return false;
      }
      
      if(!vi.isNumeric()){
        return false;
      }
      
      if(!allowNegative && vi < 0){
        return false;
      }
    }
    
    return true;
  }
  
  if(v == undef){
    return false;
  }
  
  if(v.isNumeric()){
    if(allowNegative){
      return true;
    }
    return v >= 0; 
  }
  
  return false;
}

bool NConcept::isInteger(const nvar& v, bool allowNegative){
  if(v.hasVector()){
    for(const nvar& vi : v){
      if(vi == undef){
        return false;
      }
      
      if(!vi.isInteger()){
        return false;
      }
      
      if(!allowNegative && vi < 0){
        return false;
      }
    }
    
    return true;
  }
  
  if(v == undef){
    return false;
  }
  
  if(v.isInteger()){
    if(allowNegative){
      return true;
    }
    return v >= 0; 
  }
  
  return false;
}

#ifndef META_GUARD
#include "NConcept_meta.h"
#endif

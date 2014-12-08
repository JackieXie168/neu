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

#include <neu/NCOntology.h>
#include <neu/NCCodeGen.h>
#include <neu/NConcept.h>

#include <iostream>
#include <cmath>
#include <fstream>

#include <neu/NError.h>
#include <neu/NMLParser.h>
#include <neu/NMultimap.h>
#include <neu/NProc.h>
#include <neu/NMutex.h>
#include <neu/NVSemaphore.h>
#include <neu/NRandom.h>
#include <neu/NGuard.h>
#include <neu/NSys.h>
#include <neu/NCommand.h>
#include <neu/NMLGenerator.h>
#include <neu/NScope.h>
#include <neu/NEncoder.h>

using namespace std;
using namespace neu;

namespace{

class Method;

typedef NMultimap<nstr, pair<Method*, NConcept*>> ParamMethodMap;
typedef NMultimap<Method*, pair<NConcept*, nstr>> MethodParamMap;

typedef NMap<nstr, nstr> RenameMap;
typedef NMap<nstr, bool> NameMap;
  
NCOntology* _outerOntology = 0;
NCOntology_* _ontology = 0;

static const uint32_t END_METHOD_ID = 9999999; 

static const size_t SERVER_THREADS = 8;

static const size_t MULTI_CHOOSE_ATTEMPTS = 50;

static uint64_t _uId = 0;
NMutex _uIdMutex;

uint64_t _getUId(){
  _uIdMutex.lock();
  uint64_t uId = _uId++;
  _uIdMutex.unlock();
  return uId;
}

typedef NMap<double, nvar> ParamVarMap;
typedef NVector<ParamVarMap> ParamVarVec;

nstr getTemp(nvar& state, const nstr& tp){
  return tp.lowercase() + "_" + nstr::toStr(state["temp"]++);
}

void remapCode(RenameMap& m, size_t& temp, nvar& n){
  if(n.isSymbol()){
    auto itr = m.find(n.str());
    if(itr != m.end()){
      n.str() = itr->second;
    }
  }
  else if(n.isFunction("Var")){
    if(n.size() > 1){
      remapCode(m, temp, n[1]);
    }

    nstr& s = n[0].str();

    size_t pos = s.rfind('_');

    nstr t = s.substr(0, pos) + "_" + nstr::toStr(temp++);
    m[s] = t;
    s = t;
  }
  else if(n.isFunction()){
    for(auto& ni : n){
      remapCode(m, temp, ni);
    }
  }
}

void remapState(RenameMap& m, nvar& state){
  nvar& vars = state["vars"];
  nmap& tm = vars;
  for(auto& itr : tm){
    nvec ek;
    nmap& vm = itr.second;
    for(auto& itr2 : vm){
      auto mitr = m.find(itr2.first);
      if(mitr != m.end()){
        vm[mitr->second] = itr2.second;
        ek.push_back(mitr->first);
      }
    }
    for(const nstr& k : ek){
      vm.erase(k);
    }
  }
}

 void remapInOuts(RenameMap& rm, nvar& si){
   if(si.has("ins")){
     nvar vn;
     nmap& m = si["ins"];
     for(auto& itr : m){
       auto mitr = rm.find(itr.first);
       if(mitr != rm.end()){
         vn(mitr->second) = true;
       }
       else{
         vn(itr.first) = true;
       }
     }
     si["ins"] = vn;
   }

   if(si.has("outs")){
     nvar vn;
     nmap& m = si["outs"];
     for(auto& itr : m){
       auto mitr = rm.find(itr.first);
       if(mitr != rm.end()){
         vn(mitr->second) = true;
       }
       else{
         vn(itr.first) = true;
       }
     }
     si["outs"] = vn;
   }
}
 
class Method : public NObject{
public:
  Method(NConcept* concept, const nstr& name, uint32_t id)
    : concept_(concept),
      name_(name),
      id_(id),
      this_(0),
      thisOut_(0),
      return_(0){
    setEnabled(true);
  }
  
  NFunc handle(const nvar& n, uint32_t flags);

  const nstr& name() const{
    return name_;
  }

  NConcept* concept(){
    return concept_;
  }

  uint32_t id() const{
    return id_;
  }

  void setData(const nvar& data){
    data_ = data;
  }

  void setReturn(NConcept* ret){
    return_ = ret;
  }

  NConcept* getReturn(){
    return return_;
  }

  void setThis(NConcept* c){
    this_ = c;
  }

  NConcept* getThis(){
    return this_;
  }

  void removeParamAttrs(nvar& v){
    v.erase("takeThis");
  }

  void getInitialState(nvar& state){
    state("solutionId") = _getUId();
    state("solutionTag") = nstr::getB62Id(state["solutionId"]);
    state("temp") = 0;
    state("seq").pushBack(0);

    nvar& seq = state["seq"].back();
    seq("static") = false;
    nvar& outs = seq("outs");
    nvar& vars = state("vars");

    nvar code = nfunc("Block");

    for(auto& itr : paramMap_){
      const nstr& pn = itr.first;

      NConcept* param = itr.second;
      
      const nstr& type = param->name();
      nvar& vt = vars(type);

      if(param->getConst()){
        nvar& vv = vt(pn);
        vv = param->attributes();
        removeParamAttrs(vv);
        vv("in") = true;
        vv("out") = !vv["const"];
        vv("age") = 0;
        vv("given") = true;
        vv("inputUses") = 0;
        vv("outputUses") = 0;

        if(!vv.has("weight")){
          vv("weight") = 1.0;
        }

        outs(pn) = 2;          
      }
      else{
        nstr temp = getTemp(state, type);
          
        code << (nfunc("Var") << nsym(temp) <<
                 (nfunc("Call") << nsym(pn) << nfunc("copy"))
                 << nvar()({"shared", true, "type", type}));
    
        nvar& vv = vt(temp);
        vv = param->attributes();
        removeParamAttrs(vv);
        vv("in") = true;
        vv("out") = true;
        vv("age") = 0;
        vv("inputUses") = 0;
        vv("outputUses") = 0;
        vv("given") = true;
        vv("cloneOf") = pn;

        if(!vv.has("weight")){
          vv("weight") = 1.0;
        }

        outs(temp) = 2;
      }
    }

    seq("code") = code;
  }

  void getFinalState(const nvec& vs, nvar& state){
    state["seq"].pushBack(id_);
    nvar& seq = state["seq"].back();
    seq("static") = false;

    nvar& ins = seq("ins");

    nvar code = nfunc("Block");

    for(size_t i = 0; i < vs.size(); ++i){
      const nvar& vi = vs[i];

      nvar f = nfunc("Call") << nsym(paramVec_[i].first) <<
      (nfunc("setObj") << nsym(vi));
      
      code << f;
      
      ins(vi) = true;
    }
    
    seq("code") = code;
  }

  void run(const nvec& vs, nvar& state){
    state["seq"].pushBack(id_);
    nvar& seq = state["seq"].back();

    nvar& outs = seq("outs");
    nvar& ins = seq("ins");

    nvar& vars = state["vars"];

    nmap& tm = vars;
    for(auto& itr : tm){
      nmap& vm = itr.second;
      for(auto& itr2 : vm){
        nvar& v = itr2.second;
        if(!v["given"]){
          ++v["age"];
        }
      }
    }

    NObject* obj;
    if(post_ != none){
      obj = new NObject;
    }
    else{
      obj = 0;
    }

    bool isStatic = true;
    for(const nvar& vi : vs){
      if(!vi["s"]){
        isStatic = false;
        break;
      }
    }

    nstr returnTemp;
    nstr returnType;
    
    const nvar& v0 = vs[0];

    if(return_){
      if(return_->getPoly()){
        returnType = v0["t"];
      }
      else{
        returnType = return_->name();
      }

      returnTemp = getTemp(state, returnType);

      nvar& vt = vars(returnType);
      nvar& vv = vt(returnTemp);

      if(obj){
        obj->DefSym("ret", nvar(&vv, nvar::Ptr));
      }

      vv = return_->attributes();

      removeParamAttrs(vv);
      vv("in") = true;
      vv("out") = !vv["const"];
      vv("age") = 0;
      vv("given") = false;
      vv("inputUses") = 1;
      vv("outputUses") = 1;
      vv("static") = isStatic;

      if(!vv.has("weight")){
        vv("weight") = 1.0;
      }

      outs(returnTemp) = 2;
    }

    const nstr& type = v0["t"];
    nvar& vt = vars(type);
    nvar& vv = vt[v0];

    if(obj){
      obj->DefSym("self", nvar(&vv, nvar::Ptr));
    }

    vv("in") = true;
    ++vv["inputUses"];
    ins(v0) = true;
    
    if(!this_->getConst()){
      if(thisOut_){
        nvar vn = thisOut_->attributes();
        removeParamAttrs(vn);
        vv.outerMerge(vn);
      }

      vv("out") = true;
      ++vv["outputUses"];
      vv("static") = isStatic;
      outs(v0) = 1;
    }

    nvar call = nfunc(name_);

    nvar code = nfunc("Block");

    for(size_t i = 1; i < vs.size(); ++i){
      const nvar& vi = vs[i];

      NConcept* pi = paramVec_[i - 1].second;

      NConcept* po;

      auto itr = paramOutMap_.find(paramVec_[i - 1].first);
      if(itr != paramOutMap_.end()){
        po = itr->second;
      }
      else{
        po = 0;
      }

      nstr type;
      if(pi->getPoly()){
        type = vi["t"];
      }
      else{
        type = pi->name();
      }

      nvar& vt = vars(vi["t"]);
      nvar& vv = vt[vi];

      if(obj){
        obj->DefSym(paramVec_[i - 1].first, nvar(&vv, nvar::Ptr));
      }

      if(pi->getOut()){
        if(pi->getIn()){
          if(po){
            nvar vn = po->attributes();
            removeParamAttrs(vn);
            vv.outerMerge(vn);
          }

          vv("in") = true;
          vv("out") = true;
          ++vv["inputUses"];
          ++vv["outputUses"];
          vv("static") = isStatic;
          ins(vi) = true;
          outs(vi) = 1;
        }
        else{
          nstr temp = getTemp(state, type);
          nvar& vn = vt(temp);
          if(po){
            vn = po->attributes();
          }
          else{
            vn = pi->attributes();
          }
          removeParamAttrs(vn);
          vv("in") = true;
          vv("out") = true;
          vv("age") = 0;
          vv("given") = false;
          vv("inputUses") = 0;
          vv("outputUses") = 1;
          vv("static") = isStatic;

          if(!vv.has("weight")){
            vv("weight") = 1.0;
          }

          outs(vi) = 2;

          code << (nfunc("Var") << nsym(temp) << undef <<
                   nvar()({"shared", true, "type", type}));
        }
      }
      else{
        ++vv["inputUses"];
        ins(vi) = true;
      }
      
      call << nsym(vi);
    }

    nvar mc = nfunc("Call") << nsym(v0) << call;

    nvar f;
    if(returnTemp.empty()){
      f = mc;
    }
    else{
      f = nfunc("Var") << nsym(returnTemp) << mc <<
      nvar()({"shared", true, "type", returnType});
    }

    code << f;
    seq("code") = code;
    seq("static") = isStatic;

    if(obj){
      obj->run(post_);
      delete obj;
    }
  }

  bool getParamVars(const nvar& state,
                    ParamVarVec& ps,
                    ostream* matchLog,
                    double unusedBias);

  bool addParam(const nstr& name, NConcept* param){
    if(paramMap_.has(name)){
      return false;
    }
    
    paramMap_.insert({name, param});
    paramVec_.push_back({name, param});

    return true;
  }

  void init(){
    nvec keys;
    data_.keys(keys);

    for(const nstr& k : keys){
      size_t pos = k.find("_out");

      if(pos != nstr::npos && pos == k.length() - 4){
        nstr nk = k.substr(0, k.length() - 4);

        NConcept* c;

        if(nk == "self"){
          if(this_->getConst()){
            NERROR("On concept '" + concept_->name() +
                   "' on method '" + name_ +
                   "' self_out given for const method");
          }

          if(!thisOut_){
            thisOut_ = this_->copy();
          }
          c = thisOut_;
        }
        else{
          auto itr = paramOutMap_.find(nk);

          if(itr == paramOutMap_.end()){
            auto itr2 = paramMap_.find(nk);

            if(itr2 == paramMap_.end()){
              NERROR("On concept '" + concept_->name() +
                     "' on method '" + name_ +
                     "' invalid attribute '" + k);
            }
            
            if(itr2->second->getConst()){
              NERROR("On concept '" + concept_->name() +
                     "' on method '" + name_ +
                     "' '" + k + "' given for const parameter");
            }
            
            c = itr2->second->copy();
            paramOutMap_.insert(make_pair(nk, c));  
          }
          else{
            c = itr->second;
          }
        }

        nvec keys2;
        data_[k].keys(keys2);

        for(const nstr& k2 : keys2){
          const nvar& value = data_[k][k2];

          nvar n;

          if(value == undef){
            n = nfunc("Call") << nfunc("undef" + k2.uppercase());
          }
          else{
            n = nfunc("Call") << (nfunc("set" + k2.uppercase()) << value);
          }

          try{
            c->run(n);
          }
          catch(NError& e){
            NERROR("On concept '" + concept_->name() +
                   "' on method '" + name_ +
                   "' invalid attribute '" + k2 +
                   "' for parameter '" + k +
                   "' value: " + value.toStr());
          }
        }

        continue;
      }

      auto itr = paramMap_.find(k);
      if(itr != paramMap_.end()){
        nvec keys2;
        data_[k].keys(keys2);

        for(const nstr& k2 : keys2){
          const nvar& value = data_[k][k2];

          nvar n;

          if(value == undef){
            n = nfunc("Call") << (nfunc("undef" + k2.uppercase()));
          }
          else{
            n = nfunc("Call") << (nfunc("set" + k2.uppercase()) << value);
          }
          
          try{
            itr->second->run(n);
          }
          catch(NError& e){
            NERROR("On concept '" + concept_->name() +
                   "' on method '" + name_ +
                   "' invalid attribute '" + k2 +
                   "' for parameter '" + k +
                   "' value: " + value.toStr());
          }
        }
      }
      else if(k == "self" || k == "ret"){
        nvec keys2;
        data_[k].keys(keys2);

        for(const nstr& k2 : keys2){
          const nvar& value = data_[k][k2];

          nvar n;

          if(value == undef){
            n = nfunc("Call") << (nfunc("undef" + k2.uppercase()));
          }
          else{
            n = nfunc("Call") << (nfunc("set" + k2.uppercase()) << value);
          }

          try{
            if(k == "self"){
              this_->run(n);
            }
            else{
              return_->run(n);
            }
          }
          catch(NError& e){
            NERROR("On concept '" + concept_->name() +
                   "' on method '" + name_ +
                   "' invalid attribute '" + k2 +
                   "' for parameter '" + k +
                   "' value: " + value.toStr());
          }
        }
      }
      else{
        const nvar& value = data_[k];

        nvar n = nfunc("Call") << (nfunc("set" + k.uppercase()) << value);
        
        try{
          NObject::run(n);
        }
        catch(NError& e){
          NERROR("On concept '" + concept_->name() +
                 "' on method '" + name_ +
                 "' invalid attribute '" + k +
                 "' value: " + value.toStr());
        }
      }
    }

    for(auto& itr : paramMap_){
      NConcept* param = itr.second;
      setParamDefaults(param);
    }

    if(post_ != none){
      NObject o;
      o.DefSym("self", true);

      if(return_){
        o.DefSym("ret", true);
      }
      
      for(auto& itr : paramMap_){
        o.DefSym(itr.first, true);
      }
      
      validatePost(o, post_);
    }
  }

  void setParamDefaults(NConcept* param){
    if(!param->hasAttribute_("takeThis")){
      param->setTakeThis(false);
    }
  }

  void error_(const nstr& key, const nvar& value){
    NERROR("On concept '" + concept_->name() +
           "' on method '" + name_ +
           "' invalid attribute for key '" + key +
           "' value: " + value.toStr());
  }
  
  void setAttribute_(const nstr& key, const nvar& value){
    attributes_(key) = value;
  }

  const nvar& getAttribute_(const nstr& key) const{
    if(attributes_.has(key)){
      return attributes_[key];
    }

    return undef;
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

    attributes_(key) = str;
  }

  void setDescription(const nvar& value){
    setStringAttribute_("description", value);
  }

  const nstr& getDescription() const{
    return getAttribute_("description");
  }

  void setEnabled(bool flag){
    setAttribute_("enabled", flag);
  }

  bool getEnabled() const{
    return getAttribute_("enabled");
  }

  void setPost(const nvar& v){
    post_ = v;
  }

  void mapMethod(MethodParamMap& inMap,
                 ParamMethodMap& mapOut);

  void validatePost(NObject& o, const nvar& n){
    if(n.isFunction()){
      if(n.isFunction("Scoped", 1)){
        NScope s;
        o.PushScope(&s);
        validatePost(o, n[0]);
        o.PopScope();
      }
      else if(n.isFunction("VarSet", 2)){
        o.DefSym(n[0], true);
        validatePost(o, n[1]);
      }
      else{
        for(auto& ni : n){
          validatePost(o, ni);
        }
      }
    }
    else if(n.isSymbol()){
      nvar g = o.Get(n);
      if(g.isNone()){
        NERROR("On concept '" + concept_->name() +
               "' on method '" + name_ +
               "' on postcondition 'post' "
               "undefined symbol: '" + n.str() + "'");
      }
    }
  }

  void writeMethodSignature(ostream& ostr){
    ostr << "--------------------------" << endl;
   
    if(return_){
      if(return_->getConst()){
        ostr << "const ";
      }

      ostr << return_->name() << "* ";
    }
    else{
      ostr << "void ";
    }

    ostr << this_->name() << "::" << name_ << "(";

    bool first = true;

    for(auto& itr : paramVec_){
      if(first){
        first = false;
      }
      else{
        ostr << ", ";
      }

      NConcept* pc = itr.second;

      if(pc->getConst()){
        ostr << "const ";
      }

      ostr << pc->name() << "* ";
      
      ostr << itr.first;
    }

    ostr << ")";

    if(this_->getConst()){
      ostr << " const";
    }

    ostr << ";" << endl;
  }

  void logMethodAndVariables(ostream& ostr, const nvar& state){
    if(!this_){
      return;
    }

    writeMethodSignature(ostr);

    ostr << endl;

    if(return_){
      ostr << "ret: " << return_->attributes() << endl << endl;
    }
    
    ostr << "self: " << this_->attributes() << endl << endl;

    for(auto& itr : paramVec_){
      NConcept* pc = itr.second;

      ostr << itr.first << ": " << pc->attributes() << endl << endl;
    }

    ostr << endl;

    const nvar& vars = state["vars"];

    nvec keys;
    vars.keys(keys);

    for(const nstr& k : keys){
      ostr << k << ":" << endl;

      const nvar& vk = vars[k];

      nvec keys2;
      vk.keys(keys2);

      for(const nstr& k2 : keys2){
        ostr << "    " << k2 << ": " << vk[k2] << endl << endl;
      }

      ostr << endl;
    }
  }
  
private:
  typedef NMap<nstr, NConcept*> ParamMap_;
  typedef NVector<pair<nstr, NConcept*>> ParamVec_;

  NConcept* concept_;
  nstr name_;
  size_t id_;
  nvar data_;
  ParamMap_ paramMap_;
  ParamMap_ paramOutMap_;
  ParamVec_ paramVec_;
  NConcept* return_;
  NConcept* this_;
  NConcept* thisOut_;
  nvar attributes_;
  nvar post_;
};

class ConceptDef{
public:
  ConceptDef(const nstr& name,
             NConcept* concept,
             const nstr& extends,
             const nvar& metadata)
    : name_(name),
      concept_(concept),
      metadata_(metadata){

    if(extends != "NConcept"){
      directExtends_ = extends;
      addExtends(extends);
    }

    //cout << "################ metadata for: " << name << endl;
    //cout << metadata << endl;
  }

  ~ConceptDef(){
    for(auto& itr : methodMap_){
      delete itr.second;
    }
  }

  const nvar& metadata() const{
    return metadata_;
  }

  void addMethod(Method* method){
    methodMap_.insert(make_pair(method->name(), method));
  }

  void addExtends(const nstr& concept){
    extendsMap_.insert(make_pair(concept, true));
  }

  void addExtendedBy(const nstr& concept){
    extendedByMap_.insert(make_pair(concept, true));
  }

  bool extends(const nstr& concept){
    return extendsMap_.has(concept);
  }

  bool extendedBy(const nstr& concept){
    return extendedByMap_.has(concept);
  }

  nvec getExtends() const{
    nvec ret;
    for(auto& itr : extendsMap_){
      ret.push_back(itr.first);
    }

    return ret;
  }

  void getExtendedByMap(nvar& m) const{
    for(auto& itr : extendedByMap_){
      m(itr.first) = true;
    }
  }

  const nstr& directExtends(){
    return directExtends_;
  }

  const nstr& name() const{
    return name_;
  }

  void setData(const nvar& data){
    data_ = data;
  }

  void init(){
    nvec keys;
    data_.keys(keys);

    for(const nstr& k : keys){
      nvar n = nfunc("Call") << (nfunc("set" + k.uppercase()) << data_[k]);

      try{
        concept_->run(n);
      }
      catch(NError& e){
        NERROR("Error handling field '" + k + "' on concept: " + name_);
      }
    }

    if(!concept_->hasAttribute_("remove")){
      concept_->setAttribute_("remove", undef);
    }

    for(auto& itr: methodMap_){
      itr.second->init();
    }
  }

  NConcept* concept(){
    return concept_;
  }

  void mapMethods(MethodParamMap& inMap,
                  ParamMethodMap& outMap){
    if(!concept_->getEnabled()){
      cout << "disabled: " << name_ << endl;
      return;
    }

    for(auto& itr : methodMap_){
      Method* m = itr.second;

      if(m->getEnabled()){
        m->mapMethod(inMap, outMap);
      }
      else{
        cout << "disabled: " << name_ << "::" << m->name() << "()" << endl;
      }
    }
  }

  Method* getMethod(const nstr& method){
    auto itr = methodMap_.find(method);

    if(itr != methodMap_.end()){
      return itr->second;
    }

    return 0;
  }

private:
  typedef NMap<nstr, Method*> MethodMap_;
  typedef NMap<nstr, bool> ExtendsMap_;
  typedef NMap<nstr, bool> ExtendedByMap_;
  
  nstr name_;
  nvar metadata_;
  nvar data_;
  MethodMap_ methodMap_;
  ExtendsMap_ extendsMap_;
  ExtendedByMap_ extendedByMap_;
  nstr directExtends_;
  NConcept* concept_;
};

class Solution{
public:
  Solution(const nvar& state, double priority)
    : state_(state),
      priority_(priority),
      fitness_(0),
      numTemps_(0),
      size_(0),
      startSize_(0),
      reduction_(0){

    nvec& seq = state_["seq"];

    //cout << "seq length: " << seq.size() << endl;

    //cout << "seq is: " << seq << endl;

    NameMap outMap;
    nlist newSeq;

    bool usesStatic = false;

    for(size_t i = seq.size(); i > 0; --i){
      const nvar& si = seq[i - 1];

      bool isStatic = si["static"];

      if(isStatic){
        usesStatic = true;
      }

      if(!isStatic && !si.has("ins")){
        newSeq.push_front(si);
        continue;
      }

      bool matched = false;

      if(si.has("outs")){
        const nvar& outs = si["outs"];
        const nmap& m = outs;

        for(auto& itr : m){
          if(outMap.has(itr.first)){
            matched = true;
            break;
          }
        }
      }
      else{
        matched = true;
      }

      if(matched){
        if(si.has("ins")){
          const nvar& ins = si["ins"];
          const nmap& m = ins;

          for(auto& itr : m){
            outMap[itr.first] = true;
          }
        }

        newSeq.push_front(si);
      }
    }

    nmap& tm = state_["vars"];
    auto itr = tm.begin();
    while(itr != tm.end()){
      nmap& vm = itr->second;

      auto itr2 = vm.begin();
      while(itr2 != vm.end()){
        if(!outMap.has(itr2->first)){
          vm.erase(itr2++);
        }
        else{
          ++itr2;
        }
      }

      if(vm.empty()){
        tm.erase(itr++);
      }
      else{
        ++itr;
      }
    }

    solution_ = nfunc("Block");
    
    startSize_ = seq.size();

    seq.clear();
    state_("sequence") = nvec();
    state_("usesStatic") = usesStatic;
    
    RenameMap rm;
    size_t temp = 0;

    for(nvar& si : newSeq){
      nvar n = si["code"];
      remapCode(rm, temp, n);
      si("code") = n;
      remapInOuts(rm, si);
      seq.push_back(si);
      solution_ << n;
      state_["sequence"].pushBack(si.toLong());
    }

    numTemps_ = temp;
    size_ = seq.size();

    //cout << "seq is: " << seq << endl;
    //cout << "size is: " << size_ << endl << endl;

    reduction_ = double(startSize_)/size_; 

    remapState(rm, state_);

    checksum_ = NEncoder::md5(solution_.toStr());

    //cout << "final seq is: " << seq << endl;

    //cout << "final seq length: " << seq.size() << endl << endl;

    //cout << "solution is: " << solution_ << endl;
    //cout << "num temps: " << temp << endl;
  }

  ~Solution(){

  }

  double priority() const{
    return priority_;
  }

  double fitness() const{
    return fitness_;
  }

  void setFitness(double fitness){
    fitness_ = fitness;
  }

  nvar solution(){
    return solution_;
  }
  
  nvar& state(){
    return state_;
  }
  
  const nstr& checksum() const{
    return checksum_;
  }

  size_t numTemps() const{
    return numTemps_;
  }

  size_t size() const{
    return size_;
  }

  void setSize(size_t size){
    size_ = size;
  }

  size_t startSize() const{
    return startSize_;
  }

  double reduction() const{
    return reduction_;
  }

private:
  nvar solution_;
  nvar state_;
  double priority_;
  double fitness_;
  nstr checksum_;
  size_t numTemps_;
  size_t size_;
  size_t startSize_;
  double reduction_;
};

class Solutions{
public:
  Solutions(size_t maxSize)
  : maxSize_(maxSize),
    totalFitness_(0),
    totalTemps_(0),
    totalSize_(0),
    totalStartSize_(0),
    totalReduction_(0){

  }

  ~Solutions(){
    for(auto& itr : readyMap_){
      delete itr.second;
    }

    for(auto& itr : solutionMap_){
      delete itr.second;
    }
  }

  void clear(){
    NGuard guard(solutionMutex_);

    auto itr = solutionMap_.begin();
    while(itr != solutionMap_.end()){
      delete itr->second;
      solutionMap_.erase(itr++);
    }

    checksumMap_.clear();

    totalFitness_ = 0;
    totalTemps_ = 0;
    totalSize_ = 0;
    totalStartSize_ = 0;
    totalReduction_ = 0;
  }

  void submit(Solution* solution){
    readyMutex_.lock();
    readyMap_.insert(make_pair(solution->priority(), solution));

    readyMutex_.unlock();
    readySem_.release();
  }

  Solution* next(){
    readySem_.acquire();
    readyMutex_.lock();

    auto itr = readyMap_.begin();
    Solution* solution = itr->second;

    readyMap_.erase(itr);
    readyMutex_.unlock();

    return solution;
  }

  int enter(Solution* solution, bool& full){
    NGuard guard(solutionMutex_);

    full = solutionMap_.size() >= maxSize_;

    if(checksumMap_.has(solution->checksum())){
      delete solution;
      return -2;
    }

    double fitness = solution->fitness();

    if(full){
      auto itr = solutionMap_.end();
      --itr;
      if(fitness > itr->first){
        Solution* s = itr->second;
        totalFitness_ -= s->fitness();
        totalTemps_ -= s->numTemps();
        totalSize_ -= s->size();
        totalStartSize_ -= s->startSize();
        totalReduction_ -= s->reduction();
        checksumMap_.erase(s->checksum());
        delete s;
        solutionMap_.erase(itr);
      }
      else{
        delete solution;
        return -1;
      }
    }
    
    totalFitness_ += fitness;
    totalTemps_ += solution->numTemps();
    totalSize_ += solution->size();
    totalStartSize_ += solution->startSize();
    totalReduction_ += solution->reduction();
    auto itr = solutionMap_.insert(make_pair(fitness, solution));

    checksumMap_.insert(make_pair(solution->checksum(), true));

    return distance(solutionMap_.begin(), itr);
  }
  
  size_t size() const{
    NGuard guard(solutionMutex_);

    return solutionMap_.size();
  }

  Solution* get(size_t i){
    NGuard guard(solutionMutex_);

    if(i > solutionMap_.size()){
      return 0;
    }

    auto itr = solutionMap_.begin();
    advance(itr, i);

    return itr->second;
  }
  
  nvec getStats() const{
    NGuard guard(solutionMutex_);

    size_t size = solutionMap_.size();

    if(size == 0){
      return {0, 0, 0, 0};
    }

    nvec r;

    auto itr = solutionMap_.begin();
    Solution* s = itr->second;
    
    r.push_back(size);

    r.push_back(s->fitness());
    
    itr = solutionMap_.end();
    --itr;

    r.push_back(s->fitness());

    r.push_back(totalFitness_ / size);

    r.push_back(double(totalTemps_)/size);

    r.push_back(double(totalSize_)/size);

    r.push_back(double(totalStartSize_)/size);

    r.push_back(totalReduction_/size);

    return r;
  }

private:
  typedef NMultimap<double, Solution*, greater<double>> SolutionMap_;
  typedef NMap<nstr, bool> ChecksumMap_;

  SolutionMap_ readyMap_;
  SolutionMap_ solutionMap_;
  ChecksumMap_ checksumMap_;
  NMutex readyMutex_;
  mutable NMutex solutionMutex_;
  NVSemaphore readySem_;
  size_t maxSize_;
  double totalFitness_;
  size_t totalTemps_;
  size_t totalSize_;
  size_t totalStartSize_;
  double totalReduction_;
};

class Proc : public NProc{
public:
  Proc(NProcTask* task, NCCodeGen_* codeGen, Method* method)
  : task_(task),
  codeGen_(codeGen),
  method_(method){
    
    if(method->name() == "<<start>>"){
      type_ = 0;
    }
    else if(method->name() == "<<end>>"){
      type_ = 2;
    }
    else{
      type_ = 1;
    }
    
    random_.timeSeed();
  }
  
  ~Proc(){
    
  }
  
  Method* method(){
    return method_;
  }
  
  bool handle(nvar& v, nvar& r){
    r = move(v);
    return true;
  }
  
  void run(nvar& r);
  
  int chooseOut(nvar& state, bool initial);

  uint32_t id() const{
    return method_->id();
  }

  void lock(){
    mutex_.lock();
  }

  void unlock(){
    mutex_.unlock();
  }

  bool multiChooseVars(const ParamVarVec& ps, nvec& pv, ostream* matchLog);

  bool chooseVars(ParamVarVec& ps, nvec& pv, ostream* matchLog);

  void mapProc(Proc* proc, int id){
    idMap_[id] = proc;
  }
  
  void signal(int id, nvar& v){
    NProc::signal(task_, idMap_[id], v);
  }
  
private:
  typedef NMap<int, Proc*> IdMap_;
  
  NProcTask* task_;
  Method* method_;
  NRandom random_;
  NMutex randomMutex_;
  NCCodeGen_* codeGen_;
  NMutex mutex_;
  size_t type_;
  IdMap_ idMap_;
};

} // end namespace

namespace neu{

class NCOntology_{
public:
  NCOntology_(NCOntology* o)
  : o_(o),
    initialized_(false){
    
  }

  ~NCOntology_(){
    for(auto& itr : conceptMap_){
      delete itr.second;
    }
  }

  void mapMethods(MethodParamMap& inMap,
                  ParamMethodMap& outMap){
    for(auto& itr : conceptMap_){
      itr.second->mapMethods(inMap, outMap);
    }
  }
  
  void init(){
    if(initialized_){
      return;
    }

    NMLParser parser;
    stringstream estr;
    parser.setErrorStream(estr);

    typedef NMultimap<nstr, nstr> ExtendedByMap;
    ExtendedByMap extendedByMap;

    uint32_t methodId = 1;

    for(auto& itr : conceptMap_){
      ConceptDef* concept = itr.second;

      nvec extends = concept->getExtends();
      nlist front(extends.begin(), extends.end());
      NameMap visited;
      while(!front.empty()){
        nstr c = front.popFront();

        if(c == "NConcept"){
          continue;
        }

        auto itr = conceptMap_.find(c);
        if(itr == conceptMap_.end()){
          NERROR("Invalid extends field '" + c + "' on: " + concept->name());
        }

        nvec ei = itr->second->getExtends();

        for(size_t i = 0; i < ei.size(); ++i){
          const nstr& ci = ei[i];

          if(ci == "NConcept"){
            continue;
          }

          if(!conceptMap_.has(ci)){
            NERROR("invalid extends field '" + ci + "' on: " + ci);
          }

          if(!visited.has(ci)){
            concept->addExtends(ci);
            front.push_back(ci);
            visited[ci] = true;
          }
        }
      }

      extends = concept->getExtends();
      for(const nstr& ei : extends){
        extendedByMap.insert(make_pair(ei, concept->name()));
      }

      const nvar& metadata = concept->metadata();

      if(!metadata.has("comment")){
        NERROR("Missing data field on: " + concept->name());
      }

      const nvar& comment = metadata["comment"];

      if(!comment.isString()){
        NERROR("[1] invalid data field on: " + concept->name());
      }

      nstr code = comment;
      code += ";\n";

      nvar n = parser.parse(code);
      
      if(n.isNone()){
        NERROR("Parse error on comment field in: " + concept->name());
      }

      if(n.isSymbolic()){
        NERROR("[2] invalid data field on: " + concept->name());
      }

      concept->setData(n);

      if(!metadata.has("methods")){
        NERROR("Missing methods on: " + concept->name());
      }

      const nvar& methods = metadata["methods"];
      nvec keys;
      methods.keys(keys);

      for(const nvar& k : keys){
        const nvar& mi = methods[k];
        
        if(!mi.has("comment")){
          continue;
        }
        
        const nvar& comment = mi["comment"];
        
        if(!comment.isString()){
          NERROR("[1] invalid comment field on concept: '" +
                 concept->name() + "' on method: " + k);
        }
        
        nstr code = comment;
        code += ";\n";
        
        nvar n = parser.parse(code);
        
        if(n.isNone()){
          NERROR("Parse error on data field on concept: '" +
                 concept->name() + "' on method: " + k);
        }

        if(n.isSymbolic()){
          NERROR("[2] invalid data field on concept: '" +
                 concept->name() + "' on method: " + k);
        }

        Method* method = new Method(concept->concept(), k[0], methodId++);
        method->setData(n);

        nvar pt;
        try{
          pt = method->New(nfunc(concept->name()));
        }
        catch(NError& e){
          NERROR("Unable to create concept 'this' "
                 "on concept: '" + concept->name() +
                 "' on method: " + k);
        }

        NConcept* ct = dynamic_cast<NConcept*>(pt.obj());

        if(!ct){
          NERROR("Unable to create concept 'this' "
                 "on concept: '" + concept->name() +
                 "' on method: " + k);
        }

        if(!mi.has("const")){
          NERROR("Missing const field "
                 "on concept: '" + concept->name() +
                 "' on method: " + k);
        }

        ct->setParamName("self");
        ct->setConst(mi["const"]);
        ct->setOut(!mi["const"]);

        method->setThis(ct);

        if(mi.has("return")){
          const nvar& r = mi["return"];

          if(!r.has("type")){
            NERROR("Invalid return type on concept: '" +
                   concept->name() + "' on method: " + k);
          }

          nstr type = r["type"];

          bool poly;
          if(type == "NConcept"){
            type = concept->name();
            poly = true;
          }
          else{
            poly = false;
          }

          nvar p;
          try{
            p = method->New(nfunc(type));
          }
          catch(NError& e){
            NERROR("Unable to create concept '" +
                   r["type"].str() + "' on concept: '" + concept->name() +
                   "' on method: " + k);
          }
          
          NConcept* rc = dynamic_cast<NConcept*>(p.obj());

          if(!rc){
            NERROR("Ontology: unable to create concept '" +
                   r["type"].str() + "' on concept: '" + concept->name() +
                   "' on method: " + k);
          }

          if(!r.has("const")){
            NERROR("Missing const field on return "
                   "on concept: '" + concept->name() +
                   "' on method: " + k);
          }

          rc->setParamName("ret");
          rc->setIn(false);
          rc->setOut(true);
          rc->setConst(r["const"]);
          rc->setPoly(poly);

          method->setReturn(rc);
        }

        for(size_t i = 0; i < mi.size(); ++i){
          const nvar& pi = mi[i];

          if(!pi.isSymbol()){
            NERROR("[1] invalid parameter " + nstr::toStr(i) +
                   "' on concept: '" + concept->name() +
                   "' on method: " + k);
          }

          if(NMLParser::isReservedName(pi)){
            NERROR("[2] invalid parameter " + nstr::toStr(i) +
                   "' on concept: '" + concept->name() +
                   "' on method: " + k);
          }

          if(!pi.has("type")){
            NERROR("[3] invalid parameter '" + pi.str() +
                   "' on concept: '" + concept->name() +
                   "' on method: " + k);
          }

          nvar p;

          nstr type = pi["type"];

          bool poly;
          if(type == "NConcept"){
            type = concept->name();
            poly = true;
          }
          else{
            poly = false;
          }

          try{
            p = method->New(nfunc(type));
          }
          catch(NError& e){
            NERROR("Unable to create concept '" +
                   pi["type"].str() + "' on concept: '" + concept->name() +
                   "' on method: " + k);
          }
          
          NConcept* ci = dynamic_cast<NConcept*>(p.obj());

          if(!ci){
            NERROR("Unable to create concept '" +
                   pi["type"].str() + "' on concept: '" + concept->name() +
                   "' on method: " + k);
          }

          if(!pi.has("const")){
            NERROR("Missing const field on parameter '" +
                   pi.str() + " on concept: '" + concept->name() +
                   "' on method: " + k);
          }
          
          ci->setParamName(pi);
          ci->setConst(pi["const"]);
          ci->setOut(!pi["const"]);
          ci->setPoly(poly);

          if(!method->addParam(pi.str(), ci)){
            NERROR("Duplicate parameter " + nstr::toStr(i) +
                   " on concept: '" + concept->name() +
                   "' on method: " + k);
          }
        }

        concept->addMethod(method);
      }
    }

    typedef NMultimap<nstr, nstr> ExtendedMap_;

    for(auto& itr : conceptMap_){
      itr.second->init();
      auto p = extendedByMap.equal_range(itr.second->name());
      auto eitr = p.first;
      while(eitr != p.second){
        itr.second->addExtendedBy(eitr->second);
        ++eitr;
      }
    }

    initialized_ = true;

    totalMethods_ = methodId - 1;
  }

  void addConcept(NConcept* concept, const nvar& metadata){
    //cout << "metadata is: " << metadata << endl;
    
    nvec keys;
    metadata.keys(keys);

    if(keys.size() != 1){
      NERROR("[1] invalid metadata");
    }

    const nstr& name = keys[0];

    if(conceptMap_.has(name)){
      NERROR("Found duplicate concept: " + name);
    }

    if(!metadata[name].has("extends")){
      NERROR("Missing extends field on: " + name);
    }

    const nvar& extends = metadata[name]["extends"];
    if(!extends.isSymbol()){
      NERROR("Invalid extends field on: " + name);
    }

    conceptMap_.insert(make_pair(name, new ConceptDef(name, 
                                                      concept,
                                                      extends,
                                                      metadata[name])));
  }

  ConceptDef* getConceptDef(const nstr& concept){
    auto itr = conceptMap_.find(concept);
    if(itr != conceptMap_.end()){
      return itr->second;
    }
    return 0;
  }

  size_t totalMethods() const{
    return totalMethods_;
  }

private:
  typedef NMap<nstr, ConceptDef*> ConceptMap_;

  NCOntology* o_;
  ConceptMap_ conceptMap_;
  bool initialized_ : 1;
  size_t totalMethods_;
};

class NCCodeGen_{
public:
  NCCodeGen_(NCCodeGen* o, size_t population)
  : o_(o),
    population_(population),
    mergeRate_(0.5),
    restartRate_(1.0),
    selectionPressure_(0.5),
    unusedBias_(5.0),
    minComplexity_(0),
    maxComplexity_(999999),
    complexity_(10),
    numProcs_(0),
    numEdges_(0),
    backtrack_(true),
    fill_(false),
    resetInterval_(0),
    iteration_(0),
    round_(0),
    runStartTime_(NSys::now()),
    roundStartTime_(NSys::now()),
    initialized_(false),
    solution_(0),
    matchLog_(0),
    errorLog_(0),
    disableAll_(false){

    resetStats();

    task_ = new NProcTask(1);
    solutions_ = new Solutions(population);

    _ontology->init();

    random_.timeSeed();
  }

  ~NCCodeGen_(){
    if(task_){
      delete task_;
      delete solutions_;
    }

    shutdown();
  }

  void shutdown(){
    if(matchLog_){
      matchLog_->close();
      delete matchLog_;
      matchLog_ = 0;
    }

    if(errorLog_){
      errorLog_->close();
      delete errorLog_;
      errorLog_ = 0;
    }
  }

  void resetStats(){
    totalMisses_ = 0;
    totalSolutions_ = 0;
    totalAccepted_ = 0;
    totalDenied_ = 0;
    totalErrors_ = 0;
    totalDuplicates_ = 0;
  }

  nvar reset(){
    roundStartTime_ = NSys::now();
    ++round_;
    iteration_ = 0;
    solutions_->clear();
    resetStats();
    return true;
  }

  nvar setResetInterval(size_t interval){
    resetInterval_ = interval;

    return true;
  }

  nvar iteration() const{
    return iteration_;
  }

  nvar round() const{
    return round_;
  }

  nvar elapsedRoundTime() const{
    double dt = NSys::now() - roundStartTime_;
    return dt;
  }

  nvar elapsedRunTime() const{
    double dt = NSys::now() - runStartTime_;
    return dt;
  }

  nvar population() const{
    return population_;
  }

  nvar numSolutions() const{
    return solutions_->size();
  }

  nvar getCGG(nvar& cgg) const{
    for(auto& itr : graphMap_){
      const nstr& from = itr.first;
      const nstr& to = itr.second;

      if(cgg.has(from)){
        cgg[from].pushBack(to);
      }
      else{
        nvar& ci = cgg(from);
        ci("_fused") = true;
        ci.pushBack(to);
      }
    }

    return true;
  }

  void init_(){
    if(initialized_){
      return;
    }

    Method* startMethod = new Method(0, "<<start>>", 0);
    for(auto& itr : inputMap_){
      NConcept* input = itr.second;

      NConcept* output = input->copy();
      output->setIn(false);
      output->setOut(true);

      startMethod->setParamDefaults(output);
      startMethod->addParam(itr.first, output);
    }

    Method* endMethod = new Method(0, "<<end>>", END_METHOD_ID);
    for(auto& itr : outputMap_){
      NConcept* output = itr.second;

      NConcept* input = output->copy();
      input->setIn(true);
      input->setOut(false);
      
      endMethod->setParamDefaults(input);
      endMethod->addParam(itr.first, input);
    }

    MethodParamMap inMap;
    ParamMethodMap outMap;

    startMethod->mapMethod(inMap, outMap);
    endMethod->mapMethod(inMap, outMap);
    _ontology->mapMethods(inMap, outMap);

    /*
    for(auto& itr : inMap){
      cout << "input: " << itr.second.second << ": " << itr.first->name() << endl;
    }

    for(auto& itr : outMap){
      cout << "output: " << itr.first << ": " << itr.second.first->name() << endl;
    }
    */

    start_ = 0;
    connect(endMethod, inMap, outMap);

    if(!start_){
      NERROR("[1] failed to generate");
    }

    if(numEdges_ < 2){
      NERROR("[1] the CGG contains no edges");
    }
    
    initialized_ = true;

    cout << endl;

    cout << "total methods: " << _ontology->totalMethods() << endl;

    cout << "total nodes: " << numProcs_ << endl;

    size_t realProcs = numProcs_ - 2;
    size_t edgeUpperBound = realProcs * realProcs + realProcs * 2;

    cout << "edge upper bound: " << edgeUpperBound << endl;
    cout << "edges used: " << numEdges_ << endl;
    cout << "unused edges: " << edgeUpperBound - numEdges_ << endl;

    cout << "edge usage: " << double(numEdges_) / edgeUpperBound << endl; 

    cout << "---------------------------------" << endl;

    //exit(0);
  }

  void setMergeRate(double rate){
    mergeRate_ = rate;
  }

  double mergeRate() const{
    return mergeRate_;
  }

  void setRestartRate(double rate){
    restartRate_ = rate;
  }

  double restartRate() const{
    return restartRate_;
  }

  void setSelectionPressure(double rate){
    selectionPressure_ = rate;
  }

  double selectionPressure() const{
    return selectionPressure_;
  }

  void setUnusedBias(double bias){
    unusedBias_ = bias;
  }

  double unusedBias() const{
    return unusedBias_;
  }

  void setMinComplexity(size_t c){
    minComplexity_ = c;
  }

  size_t minComplexity() const{
    return minComplexity_;
  }

  void setMaxComplexity(size_t c){
    maxComplexity_ = c;
  }

  size_t maxComplexity() const{
    return maxComplexity_;
  }

  void setComplexity(size_t c){
    complexity_ = c;
  }

  size_t complexity() const{
    return complexity_;
  }

  void setBacktrack(bool flag){
    backtrack_ = flag;
  }
  
  bool backtrack(){
    return backtrack_;
  }

  void setFill(bool flag){
    fill_ = flag;
  }

  void enableMatchLog(){
    matchLog_ = new ofstream("match.log");
    if(matchLog_->fail()){
      NERROR("Failed to create match.log");
    }
  }

  void enableErrorLog(){
    errorLog_ = new ofstream("error.log");
    if(errorLog_->fail()){
      NERROR("Failed to create error.log");
    }
  }

  ostream* matchLog(){
    return matchLog_;
  }

  ostream* errorLog(){
    return errorLog_;
  }

  Proc* connect(Method* method,
                const MethodParamMap& inMap,
                const ParamMethodMap& outMap){
    Proc* proc = new Proc(task_, this, method);
    ++numProcs_;

    methodMap_[method->id()] = method;
    procMap_[method] = proc;

    if(method->name() == "<<start>>"){
      start_ = proc;
    }
    
    auto p = inMap.equal_range(method);

    typedef NMap<Method*, bool> MethodMap;
    MethodMap methodMap;

    auto itr = p.first;
    while(itr != p.second){
      NConcept* input = itr->second.first;

      auto p2 = outMap.equal_range(itr->second.second);
      auto itr2 = p2.first;
      while(itr2 != p2.second){
        Method* m = itr2->second.first;

        if(methodMap.has(m)){
          ++itr2;
          continue;
        }

        bool enabled = !disableAll_;

        NConcept* mc = m->concept();

        if(mc){
          const nstr& cn = mc->name();
          const nstr& mn = m->name();
          
          auto citr = enableConceptMap_.find(cn);
          if(citr != enableConceptMap_.end()){
            enabled = citr->second;
          }
          
          auto mitr = enableMethodMap_.find(make_pair(cn, mn));
          if(mitr != enableMethodMap_.end()){
            enabled = mitr->second;
          }
          
          if(!enabled){
            ++itr2;
            continue;
          }
        }

        NConcept* output = itr2->second.second;

        nvar attrs = output->attributes();
        attrs("in") = true;

        if(input->match(attrs, false) < 0){
          ++itr2;
          continue;
        }

        Proc* proc2;
        auto pitr = procMap_.find(m);
        if(pitr == procMap_.end()){
          proc2 = connect(m, inMap, outMap);
        }
        else{
          proc2 = pitr->second;
        }

        if(proc2 != start_ || proc->method()->name() != "<<end>>"){
          proc2->mapProc(proc, proc->id());
          ++numEdges_;

          NConcept* fromConcept = proc2->method()->concept();

          nstr from;

          if(fromConcept){
            from = fromConcept->name() + "::";
          }

          from += proc2->method()->name();

          NConcept* toConcept = proc->method()->concept();

          nstr to;

          if(toConcept){
            to = toConcept->name() + "::";
          }

          to += proc->method()->name();          

          cout << from << " => " << to << endl;

          graphMap_.insert(make_pair(from, to));
        }

        methodMap[m] = true;

        ++itr2;
      }

      ++itr;
    }
    
    return proc;
  }

  void addInput(const nstr& name, NConcept* input){
    if(inputMap_.has(name)){
      NERROR("Duplicate input: " + name);
    }

    input->setIn(true);

    if(!input->hasAttribute_("static")){
      input->setStatic(false);
    }
    
    inputMap_.insert(make_pair(name, input));      

    if(!input->getConst()){
      NConcept* output = input->copy();
      output->setIn(false);
      output->setOut(true);
      output->setTakeCloneOf(name);
      
      outputMap_.insert(make_pair(name, output));
    }
  }

  void addOutput(const nstr& name, NConcept* output){
    if(outputMap_.has(name)){
      NERROR("Duplicate output: " + name);
    }

    output->setIn(false);
    output->setOut(true);

    outputMap_.insert(make_pair(name, output));
  }

  void generate(){
    init_();
    task_->queue(start_);
  }

  void next(){
    solution_ = solutions_->next();
  }

  void miss(){
    ++totalMisses_;
  }

  void queue(bool miss=false){
    if(miss){
      ++totalMisses_;
      ++totalSolutions_;
    }
    
    task_->queue(start_);
  }

  void queue(nvar& state, size_t methodId){
    Proc* proc = procMap_[methodMap_[methodId]];
    task_->queue(proc, state);
  }

  bool run(){
    if(!solution_){
      NERROR("No solution has been selected");
    }

    nvar f = solution_->solution();

    obj_.Reset();

    for(auto& itr : inputMap_){
      obj_.DefSym(itr.first, itr.second);
    }

    for(auto& itr : outputMap_){
      if(!inputMap_.has(itr.first)){
        obj_.DefSym(itr.first, itr.second);
      }
    }

    try{
      obj_.run(f);
    }
    catch(NError& e){
      //cout << "e is: " << e << endl;
      
      if(errorLog_){
        ostream& ostr = *errorLog_;

        nstr nml = NMLGenerator::toStr(f);

        ostr << "-----------------------" << endl;
        ostr << nml << endl << endl;
        ostr << e << endl;
      }

      ++totalErrors_;
      delete solution_;
      solution_ = 0;
      queue();
      return false;
    }

    return true;
  }

  nvar getSolution(){
    if(!solution_){
      NERROR("No solution has been selected");
    }

    return solution_->solution();
  }

  nvar getFinalSolution(){
    if(!solution_){
      NERROR("No solution has been selected");
    }
    
    normalizeSolution(solution_);

    Solution finalSolution(solution_->state(), 0);

    //cout << "final state is: " << solution_->state() << endl;

    return finalSolution.solution();
  }

  nvar getSolution(size_t rank, nvar& solution){
    Solution* s = solutions_->get(rank);

    if(!s){
      return false;
    }

    solution("solution") = s->solution();
    solution("fitness") = s->fitness();

    return true;
  }

  void remapState(RenameMap& m, nvar& state){
    nvar& vars = state["vars"];
    nmap& tm = vars;
    for(auto& itr : tm){
      nvec ek;
      nmap& vm = itr.second;
      for(auto& itr2 : vm){
        auto mitr = m.find(itr2.first);
        if(mitr != m.end()){
          vm[mitr->second] = itr2.second;
          ek.push_back(mitr->first);
        }
      }
      for(const nstr& k : ek){
        vm.erase(k);
      }
    }
  }

  void normalizeSolution(Solution* solution){
    nvar& state = solution->state();

    if(!state["usesStatic"]){
      return;
    }

    nvec& seq = state["seq"];

    //cout << "in seq: " << seq << endl;

    NameMap m;

    nvec newSeq;

    for(nvar& si : seq){
      if(si["static"]){
        bool found = false;
        nvar code;
        const nmap& om = si["outs"];

        for(auto& itr : om){
          if(m.has(itr.first)){
            found = true;
            break;
          }

          if(si.has("ins")){
            NConcept* c = 
              static_cast<NConcept*>(obj_.Get(nsym(itr.first)).obj());

            if(code.isNone()){
              code = nfunc("Block");
            }

            const nstr& name = c->name();
            
            code << (nfunc("Var") << nsym(itr.first) <<
                     (nfunc("New") << nfunc(name)) <<
                     nvar()({"shared", true, "type", name}));
            
            code << nfunc("Call") << nsym(itr.first) << nfunc("set") <<
            c->val();
          }

          m[itr.first] = true;
        }

        if(!found){
          si.erase("ins");
          if(code != none){
            si("code") = code;
          }
          newSeq.push_back(move(si));
        }
      }
      else{
        newSeq.push_back(move(si));
      }
    }

    seq = move(newSeq);

    solution->setSize(seq.size());

    //cout << "out seq: " << seq << endl;
    //cout << "num temps: " << m.size() << endl;
  }

  bool finish(double fitness){
    solution_->setFitness(fitness);

    ++iteration_;

    if(resetInterval_ > 0 && iteration_ % resetInterval_ == 0){
      reset();
    }

    bool full;
    int pos = solutions_->enter(solution_, full);

    if(pos == -2){
      ++totalDuplicates_;
    }

    bool success = pos >= 0;

    if(full || !fill_){
      if(success){
        ++totalAccepted_;
      }
      else{
        ++totalDenied_;
      }
    }

    if(success){
      normalizeSolution(solution_);
    }

    size_t size = solutions_->size();

    if((full || !fill_) && size > 1 && random_.uniform() < mergeRate_){
      size_t i = random_.expSelect(size - 1, selectionPressure_);
      size_t j;

      do{
        j = random_.expSelect(size - 1, selectionPressure_);
      }while(j == i);

      Solution* s1 = solutions_->get(i);
      Solution* s2 = solutions_->get(j);

      nvar state = s1->state();
      nvar state2 = s2->state();
      
      state("solutionId") = _getUId();
      state("solutionTag") = nstr::getB62Id(state["solutionId"]);
      state("temp") = 0;
      state.erase("sequence");

      RenameMap rm;
      RenameMap rm2;
      nvec& seq = state["seq"];
      seq.pop_back();

      size_t temp = 0;

      bool first = true;
      bool firstEmpty;

      for(nvar& si : seq){
        nvar n = si["code"];

        if(first){
          firstEmpty = n.isFunction("Block", 0);
          first = false;
        }

        remapCode(rm, temp, n);
        si("code") = n;
        remapInOuts(rm, si);
      }

      nvec& seq2 = state2["seq"];
      seq2.pop_back();

      //cout << "---------------------------" << endl;
      //cout << "seq1 is: " << seq << endl;
      //cout << "------" << endl;
      //cout << "seq2 is: " << seq2 << endl;
      //cout << "===========================" << endl;

      first = true;

      for(nvar& si : seq2){
        nvar n = si["code"];

        if(first){
          first = false;
          if(firstEmpty && n.isFunction("Block", 0)){
            continue;
          }
        }

        remapCode(rm2, temp, n);
        si("code") = n;
        remapInOuts(rm2, si);
        seq.push_back(si);
      }

      remapState(rm, state);
      remapState(rm2, state2);

      // ndm - how to handle this?
      //state["vars"].fuse(state2.vars, nvar::FuseGreater);

      nvec choices = state["lastChoices"];
      choices.append(state2["lastChoices"]);

      size_t id = choices[random_.equilikely(0, choices.size() - 1)];

      Proc* proc = procMap_[methodMap_[id]];
      task_->queue(proc, state);
    }
    else{
      queue();
    }
    
    return success;
  }

  void reject(){
    if(solution_){
      delete solution_;
      solution_ = 0;
      queue();
    }
  }

  void submit(Solution* solution){
    ++totalSolutions_;
    solutions_->submit(solution);
  }

  nvec getStats() const{
    nvec s = solutions_->getStats();
    s.push_back(totalSolutions_);
    s.push_back(totalMisses_);
    s.push_back(totalAccepted_);
    s.push_back(totalDenied_);
    s.push_back(totalErrors_);
    s.push_back(totalDuplicates_);
    return s;
  }

  void disableAll(){
    disableAll_ = true;
  }
  
  void enable(const nstr& concept, const nstr& method, bool flag){
    ConceptDef* c = _ontology->getConceptDef(concept);

    if(!c){
      NERROR("Invalid concept: " + concept);
    }

    Method* m = c->getMethod(method);

    if(!m){
      NERROR("Invalid method: " + concept + "::" + method);
    }
    
    enableMethodMap_.insert(make_pair(make_pair(concept, method), flag));
  }

  void enable(const nstr& concept, bool flag){
    ConceptDef* c = _ontology->getConceptDef(concept);

    if(!c){
      NERROR("Invalid concept: " + concept);
    }

    enableConceptMap_.insert(make_pair(concept, flag));
  }

private:
  typedef NMultimap<nstr, nstr> Name2Map_;
  typedef NMap<size_t, Method*> MethodMap_;
  typedef NMap<Method*, Proc*> ProcMap_;
  typedef NMap<nstr, NConcept*> ParamMap_;
  typedef NMap<nstr, bool> EnableConceptMap_;
  typedef NMap<pair<nstr,nstr>, bool> EnableMethodMap_;

  NCCodeGen* o_;
  size_t population_;
  double mergeRate_;
  double restartRate_;
  double selectionPressure_;
  double unusedBias_;
  size_t minComplexity_;
  size_t maxComplexity_;
  size_t complexity_;
  bool backtrack_;
  bool fill_;
  size_t resetInterval_;
  size_t iteration_;
  size_t round_;
  double runStartTime_;
  double roundStartTime_;
  Solutions* solutions_;
  MethodMap_ methodMap_;
  ProcMap_ procMap_;
  bool initialized_;
  ParamMap_ inputMap_;
  ParamMap_ outputMap_;
  Proc* start_;
  Solution* solution_;
  NProcTask* task_;
  NObject obj_;
  NRandom random_;
  Name2Map_ graphMap_;
  ofstream* matchLog_;
  ofstream* errorLog_;
  int port_;
  size_t numProcs_;
  size_t numEdges_;
  size_t totalMisses_;
  size_t totalSolutions_;
  size_t totalAccepted_;
  size_t totalDenied_;
  size_t totalErrors_;
  size_t totalDuplicates_;
  EnableConceptMap_ enableConceptMap_;
  EnableMethodMap_ enableMethodMap_;
  bool disableAll_;
};

} // end namespace neu

int Proc::chooseOut(nvar& state, bool initial){
  typedef NMap<double, int> PMap;
  PMap m;
  
  double s = 0;

  nvec outs;
  for(auto& itr : idMap_){
    outs.push_back(itr.first);
  }
  
  assert(!outs.empty());
  
  if(codeGen_->backtrack() && state.has("lastChoices")){
    const nvec& lastChoices = state["lastChoices"];
    for(int i : lastChoices){
      s += 1;
      m.insert(make_pair(s, -i));
    }
  }

  if(!initial){
    const nvec& firstChoices = state["firstChoices"];
    for(int i : firstChoices){
      s += codeGen_->restartRate();
      m.insert(make_pair(s, -i));
    }    
  }

  bool foundEnd = false;

  for(int i : outs){
    if(i == END_METHOD_ID){
      foundEnd = true;
    }
    else{
      s += 1; // ndm - currently all edges have weight 1
      m.insert(make_pair(s, i));
    }
  }

  if(foundEnd){
    size_t length = state["seq"].size();
    size_t maxComplexity = codeGen_->maxComplexity();

    if(length >= maxComplexity){
      return END_METHOD_ID;
    }

    size_t minComplexity = codeGen_->minComplexity();

    if(length >= minComplexity){
      size_t complexity = codeGen_->complexity();
      
      double p = 1.0/complexity;

      s += s * p;
      m.insert(make_pair(s, END_METHOD_ID));
    }
  }

  randomMutex_.lock();
  double r = random_.uniform(0, s);
  randomMutex_.unlock();

  auto itr = m.lower_bound(r);
  
  int j = itr->second;

  state("lastChoices") = nvec();
  if(j > 0){
    nvec& lastChoices = state["lastChoices"];
    for(int i : outs){
      if(i != j && i != END_METHOD_ID){
        lastChoices.push_back(i);
      }
    }
  }
  else{
    nvec newLastChoices;
    nvec& lastChoices = state["lastChoices"];
    for(int i : lastChoices){
      if(i != method_->id()){
        newLastChoices.push_back(i);
      }
    }
    lastChoices = move(newLastChoices);
  }

  if(initial){
    state("firstChoices") = state["lastChoices"];
  }

  return j;
}

bool Proc::multiChooseVars(const ParamVarVec& ps, nvec& pv, ostream* matchLog){
  for(size_t i = 0; i < MULTI_CHOOSE_ATTEMPTS; ++i){
    ParamVarVec ips(ps);
    if(chooseVars(ips, pv, matchLog)){
      return true;
    }
    pv.clear();

    if(matchLog){
      ostream& ostr = *matchLog;
      
      ostr << "======" << endl;
    }
  }

  if(matchLog){
    ostream& ostr = *matchLog;
    
    ostr << "FAILED AFTER " << MULTI_CHOOSE_ATTEMPTS << " attempts" << endl;
  }
  
  return false;
}

bool Proc::chooseVars(ParamVarVec& ps, nvec& pv, ostream* matchLog){
  NConcept* tc = method_->getThis();

  for(size_t i = 0; i < ps.size(); ++i){
    ParamVarMap& pm = ps[i];

    if(pm.empty()){
      pv.push_back(undef);
    }
    else{
      bool match = false;
      
      auto itr = pm.end();
      --itr;
      
      double s = itr->first;
      
      while(!pm.empty()){
        randomMutex_.lock();
        double r = random_.uniform(0, s);
        randomMutex_.unlock();

        itr = pm.lower_bound(r);

        assert(itr != pm.end());

        const nvar& pj = itr->second;

        if(i == 0 || !tc){
          pv.push_back(pj);
          match = true;

          if(matchLog){
            ostream& ostr = *matchLog;
            ostr << "MAP: mapped param: " << i << " to: " << pj.str() << endl;
          }

          break;
        }
        else if((!pj["p"] || pj["t"] == pv[0]["t"]) &&
           (pj["h"] || pj.str() != pv[0])){
          pv.push_back(pj);
          match = true;

          if(matchLog){
            ostream& ostr = *matchLog;
            ostr << "MAP: mapped param: " << i << " to: " << pj.str() << endl;
          }

          break;
        }

        pm.erase(itr);

        s = 0;
        ParamVarMap pm2;
        for(auto& itr2 : pm){
          nvar& p2 = itr2.second;
          s += p2["m"].toDouble();
          pm2.insert(make_pair(s, move(p2))); 
        }

        pm = move(pm2);
      }

      if(!match){
        if(matchLog){
          ostream& ostr = *matchLog;
          
          ostr << "MISS: failed to map parameter: " << i << endl;
        }
        return false;
      }
    }
  }

  if(matchLog){
    ostream& ostr = *matchLog;
    
    ostr << "HIT!!!!!" << endl;
  }
  
  return true;
}

void Proc::run(nvar& r){
  NGuard guard(mutex_);

  if(type_ == 0){
    nvar state;
    method_->getInitialState(state);
    //cout << "initial state: " << state << endl;

    int j = chooseOut(state, true);

    signal(j, state);
  }
  else if(type_ == 1){
    //cout << "-------------- running: " << method_->name() << endl;

    nvar& state = r;

    ostream* matchLog = codeGen_->matchLog();
    if(matchLog){
      method_->logMethodAndVariables(*matchLog, state);
    }

    ParamVarVec ps;
    nvec vs;
    if(!method_->getParamVars(state, ps, matchLog, codeGen_->unusedBias()) ||
       !multiChooseVars(ps, vs, matchLog)){

      nvec& choices = state["lastChoices"];

      if(choices.empty()){
        if(matchLog){
          *matchLog << "STARTING OVER" << endl;
        }

        const nvec& firstChoices = state["firstChoices"];
        size_t j = random_.equilikely(0, firstChoices.size() - 1);
        size_t id = firstChoices[j];
        codeGen_->miss();
        codeGen_->queue(state, id);
        return;

        /*
        codeGen_->queue(true);
        return 0;
        */
      }

      size_t j = random_.equilikely(0, choices.size() - 1);
      size_t id = choices[j];
      choices.erase(j);

      codeGen_->queue(state, id);

      if(matchLog){
        *matchLog << "BACKTRACKING" << endl;
      }

      return;
    }
    
    method_->run(vs, state);

    int j = chooseOut(state, false);

    if(j < 0){
      codeGen_->queue(state, -j);
      return;
    }

    //cout << "-------- out state: " << state << endl << endl << endl;

    signal(j, state);
  }
  else if(type_ == 2){
    nvar& state = r;

    ostream* matchLog = codeGen_->matchLog();
    if(matchLog){
      method_->logMethodAndVariables(*matchLog, state);
    }

    ParamVarVec ps;
    nvec vs;
    if(!method_->getParamVars(state, ps, matchLog, codeGen_->unusedBias()) ||
       !multiChooseVars(ps, vs, matchLog)){
      nvec& choices = state["lastChoices"];

      if(choices.empty()){
        if(matchLog){
          *matchLog << "END STARTING OVER" << endl;
        }

        const nvec& firstChoices = state["firstChoices"];
        size_t j = random_.equilikely(0, firstChoices.size() - 1);
        size_t id = firstChoices[j];
        codeGen_->miss();
        codeGen_->queue(state, id);
        return;
        
        /*
        codeGen_->queue(true);
        return 0;
        */
      }

      size_t j = random_.equilikely(0, choices.size() - 1);
      size_t id = choices[j];
      choices.erase(j);

      codeGen_->queue(state, id);

      if(matchLog){
        *matchLog << "END BACKTRACKING" << endl;
      }

      return;
    }

    method_->getFinalState(vs, state);

    //cout << "------------ out final state: " << state << endl;

    Solution* solution = new Solution(state, 1);
    codeGen_->submit(solution);

    //codeGen_->queue();
  }
}

bool Method::getParamVars(const nvar& state,
                          ParamVarVec& ps,
                          ostream* matchLog,
                          double unusedBias){
  const nvar& vars = state["vars"];
  
  if(this_){
    ps.push_back(ParamVarMap());
    ParamVarMap& pm = ps.back();

    ConceptDef* c = _ontology->getConceptDef(concept_->name());
    assert(c);

    nvar em;
    c->getExtendedByMap(em);
    em(concept_->name()) = true;
    
    double s = 0;

    const nmap& me = em;

    for(auto& itr : me){
      const nstr& ei = itr.first;

      if(vars.has(ei)){
        const nvar& sp = vars[ei];
        const nmap& m = sp.map();
      
        for(auto& mitr : m){
          const nvar& attrs = mitr.second;

          double m = this_->match(attrs, true);
          
          if(m > 0){
            m *= 1.0/(attrs["inputUses"].toDouble() + 1.0/unusedBias);

            nvar pi = mitr.first;
            pi("m") = m * attrs["weight"];
            pi("s") = attrs["static"];
            pi("p") = this_->getPoly();
            pi("t") = ei;
            s += m;
            pm.insert(make_pair(s, pi));
          }
        }      
      }
    }

    if(pm.empty()){
      if(matchLog){
        ostream& ostr = *matchLog;

        ostr << "MISS: no matches for param: 0" << endl;
      }
      return false;
    }
    else{
      if(matchLog){
        ostream& ostr = *matchLog;
        
        ostr << "self: ";
        bool first = true;
        for(auto& mitr : pm){
          if(first){
            first = false;
          }
          else{
            ostr << ", ";
          }
          ostr << mitr.first << ":" << mitr.second.str();
        }
        ostr << endl;
      }
    }
  }

  for(auto& itr : paramVec_){
    ps.push_back(ParamVarMap());
    ParamVarMap& pm = ps.back();

    NConcept* param = itr.second;

    if(!param->getIn()){
      continue;
    }

    ConceptDef* c = _ontology->getConceptDef(param->name());
    assert(c);
    nvar em;
    c->getExtendedByMap(em);
    em(param->name()) = true;

    double s = 0;

    const nmap& me = em;

    for(auto itr : me){
      const nstr& ei = itr.first;
      if(vars.has(ei)){
        const nvar& sp = vars[ei];
        const nmap& m = sp.map();
        
        for(auto& mitr : m){
          const nvar& attrs = mitr.second;

          if(id_ == END_METHOD_ID && attrs["outputUses"] == 0){
            continue;
          }

          double m = param->match(attrs, true);
          
          if(m > 0){
            m *= 1.0/(attrs["inputUses"].toDouble() + 1.0/unusedBias);

            nvar pi = mitr.first;
            pi("m") = m * attrs["weight"];
            pi("s") = attrs["static"];
            pi("p") = param->getPoly();
            pi("t") = ei;
            pi("h") = param->getTakeThis();
            s += m;
            pm.insert(make_pair(s, pi));
          }
        }
      }
    }
    
    if(pm.empty()){
      if(matchLog){
        ostream& ostr = *matchLog;
        
        ostr << "MISS: no matches for param: " << itr.first << endl;
      }
      return false;
    }
    else{
      if(matchLog){
        ostream& ostr = *matchLog;
        
        ostr << itr.first << ": ";
        bool first = true;
        for(auto& mitr : pm){
          if(first){
            first = false;
          }
          else{
            ostr << ", ";
          }
          ostr << mitr.first << ":" << mitr.second.str();
        }
        ostr << endl;
      }
    }
  }

  return true;
}

void Method::mapMethod(MethodParamMap& inMap,
                       ParamMethodMap& outMap){
  nvec ev;
  if(concept_){
    ConceptDef* c = _ontology->getConceptDef(concept_->name());
    assert(c);
    nvar em;
    c->getExtendedByMap(em);
    em(concept_->name()) = true;

    nvec keys;
    em.keys(keys);
    for(const nstr& k : keys){
      ConceptDef* ce = _ontology->getConceptDef(k);
      assert(ce);
      const nvar& rm = ce->concept()->getRemove();
      if(rm.has(name_)){
        em.erase(k);
      }
    }
    
    em.keys(ev);
  }
  
  if(this_){
    for(const nstr& ei : ev){
      inMap.insert(make_pair(this, make_pair(this_, ei)));

      if(this_->getOut()){

        NConcept* po;
        if(thisOut_){
          po = thisOut_;
        }
        else{
          po = this_;
        }

        outMap.insert(make_pair(ei, make_pair(this, po)));
      }
    }
  }
    
  if(return_){
    if(return_->getPoly()){
      for(const nstr& ei : ev){
        outMap.insert(make_pair(ei, make_pair(this, return_)));
      }
    }
    else{
      outMap.insert(make_pair(return_->name(), make_pair(this, return_)));      
    }
  }

  for(auto& itr : paramMap_){
    NConcept* p = itr.second;
      
    if(p->getIn()){
      if(p->getPoly()){
        for(const nstr& ei : ev){
          inMap.insert(make_pair(this, make_pair(p, ei)));
        }
      }
      else{
        inMap.insert(make_pair(this, make_pair(p, p->name())));
      }
    }

    if(p->getOut()){
      NConcept* po;

      auto itr2 = paramOutMap_.find(itr.first);
      if(itr2 == paramOutMap_.end()){
        po = p;
      }
      else{
        po = itr2->second;
      }

      if(p->getPoly()){
        for(const nstr& ei : ev){
          outMap.insert(make_pair(ei, make_pair(this, po)));
        }
      }
      else{
        outMap.insert(make_pair(p->name(), make_pair(this, po)));
      }
    }
  }
}

NCOntology* NCOntology::get(){
  if(!_outerOntology){
    _outerOntology = new NCOntology;
    _ontology = _outerOntology->x_;
  }

  return _outerOntology;
}

NCOntology::NCOntology(){
  x_ = new NCOntology_(this);
}

#ifndef META_GUARD
#include "Method_meta.h"
#include "NCOntology_meta.h"
#include "NCCodeGen_meta.h"
#endif

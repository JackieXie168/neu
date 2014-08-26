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
 
 
Neu, Copyright (c) 2013-2014, Andrometa LLC
All rights reserved.

neu@andrometa.net
http://neu.andrometa.net

Neu can be used freely for commercial purposes. We hope you will find
Neu powerful, useful to make money or otherwise, and fun! If so,
please consider making a donation via PayPal to: neu@andrometa.net

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

#include <neu/NObject.h>

#include <neu/NFuncMap.h>
#include <neu/NScope.h>
#include <neu/NClass.h>
#include <neu/NThread.h>
#include <neu/NRWMutex.h>
#include <neu/NBroker.h>

using namespace std;
using namespace neu;

namespace{
  
  class FuncMap : public NFuncMap{
  public:
    FuncMap();
  };
  
  FuncMap _funcMap;
  
  class Global{
  public:
    Global()
    : globalScope_(new NScope){
      
      precedenceMap_("VarSet") = 17;
      precedenceMap_("Set") = 17;
      precedenceMap_("Var") = 17;
      precedenceMap_("Def") = 17;
      precedenceMap_("DefSym") = 17;
      precedenceMap_("AddBy") = 17;
      precedenceMap_("SubBy") = 17;
      precedenceMap_("MulBy") = 17;
      precedenceMap_("DivBy") = 17;
      precedenceMap_("AltBy") = 17;
      precedenceMap_("ModBy") = 17;
      precedenceMap_("Or") = 15;
      precedenceMap_("And") = 15;
      precedenceMap_("EQ") = 11;
      precedenceMap_("NE") = 11;
      precedenceMap_("LT") = 10;
      precedenceMap_("GT") = 10;
      precedenceMap_("LE") = 10;
      precedenceMap_("GE") = 10;
      precedenceMap_("Push") = 9;
      precedenceMap_("Sub") = 8;
      precedenceMap_("Add") = 8;
      precedenceMap_("Mod") = 7;
      precedenceMap_("Div") = 7;
      precedenceMap_("Mul") = 7;
      precedenceMap_("Not") = 5;
      precedenceMap_("Neg") = 5;
      precedenceMap_("Pow") = 4;
      precedenceMap_("PostDec") = 3;
      precedenceMap_("PostInc") = 3;
      precedenceMap_("Dec") = 2;
      precedenceMap_("Inc") = 2;
      
    }
    
    NScope* globalScope(){
      return globalScope_;
    }
    
    const int precedence(const nstr& op){
      return precedenceMap_.get(op, -1);
    }
    
  private:
    NScope* globalScope_;
    nvar precedenceMap_;
  };
  
  Global _global;
  
  class Class : public NClass{
  public:
    Class() : NClass("neu::NObject"){
      
    }
    
    NObjectBase* construct(const nvar& f){
      switch(f.size()){
        case 0:
          return new NObject;
        case 1:
          return new NObject(static_cast<NScope*>(f[0].obj()));
        default:
          return 0;
      }
    }
  };
  
  Class _class;
  
  NFunc _ret0Func;
  NFunc _ret1Func;
  NFunc _breakFunc;
  NFunc _continueFunc;
  
} // end namespace

const uint32_t NObject::classId = NObjectBase::getClassId();
const uint32_t NScope::classId = NObjectBase::getClassId();

namespace neu{
  
  class NObject_{
  public:
    typedef NVector<NScope*> ScopeStack;
    
    class ThreadContext{
    public:
      
      void pushScope(NScope* scope){
        scopeStack.push_back(scope);
      }
      
      NScope* getScope(size_t i){
        return scopeStack[i];
      }
      
      NScope* topScope(){
        return scopeStack.back();
      }
      
      void popScope(){
        scopeStack.pop_back();
      }
      
      void dumpScopes(){
        for(size_t i = 0; i < scopeStack.size(); ++i){
          cout << "---------------------" << endl;
          scopeStack[i]->dump();
        }
        cout << "======================" << endl;
      }
      
      ScopeStack scopeStack;
    };
    
    class ThreadData{
    public:
      
      ~ThreadData(){
        for(auto& itr : contextMap_){
          delete itr.second;
        }
      }
      
      ThreadContext*
      getContext(NObject_* obj, const NThread::id& threadId){
        
        contextMutex_.readLock();
        auto itr = contextMap_.find(threadId);
        if(itr != contextMap_.end()){
          ThreadContext* context = itr->second;
          contextMutex_.unlock();
          return context;
        }
        contextMutex_.unlock();
        
        ThreadContext* context = new ThreadContext;
        
        obj->initContext_(context);
        
        contextMutex_.writeLock();
        contextMap_.insert({threadId, context});
        contextMutex_.unlock();
        
        return context;
      }
      
    private:
      typedef NMap<NThread::id, ThreadContext*> ContextMap_;
      
      ContextMap_ contextMap_;
      NRWMutex contextMutex_;
    };
    
    NObject_(NObject* o)
    : o_(o),
    exact_(false),
    strict_(true),
    sharedScope_(false),
    threadData_(0),
    broker_(0){
      
      NScope* gs = _global.globalScope();
      mainContext_.pushScope(gs);
      
      NScope* s = new NScope;
      s->setSymbolFast("Global", gs);
      s->setSymbolFast("Shared", gs);
      s->setSymbolFast("This", o_);
      mainContext_.pushScope(s);
    }
    
    NObject_(NObject* o, NScope* sharedScope)
    : o_(o),
    exact_(false),
    strict_(true),
    sharedScope_(true),
    threadData_(0),
    broker_(0){
      
      NScope* gs = _global.globalScope();
      mainContext_.pushScope(gs);
      mainContext_.pushScope(sharedScope);
      
      NScope* s = new NScope;
      s->setSymbolFast("Global", gs);
      s->setSymbolFast("Shared", sharedScope);
      s->setSymbolFast("This", o_);
      mainContext_.pushScope(s);
    }
    
    NObject_(NObject* o, NBroker* broker)
    : o_(o),
    exact_(false),
    strict_(true),
    sharedScope_(false),
    threadData_(0),
    broker_(broker){
      
      NScope* gs = _global.globalScope();
      mainContext_.pushScope(gs);
      
      NScope* s = new NScope;
      s->setSymbolFast("Global", gs);
      s->setSymbolFast("Shared", gs);
      s->setSymbolFast("This", o_);
      mainContext_.pushScope(s);
    }
    
    ~NObject_(){
      if(sharedScope_){
        delete mainContext_.getScope(2);
      }
      else{
        delete mainContext_.getScope(1);
      }
      
      if(threadData_){
        delete threadData_;
      }
      
      if(broker_){
        broker_->release(o_);
      }
    }
    
    static NObject_* obj(void* o){
      return static_cast<NObject*>(o)->x_;
    }
    
    void setStrict(bool flag){
      strict_ = flag;
    }
    
    void setExact(bool flag){
      exact_ = flag;
    }
    
    bool isRemote(){
      return broker_;
    }
    
    void enableThreading(){
      if(threadData_){
        return;
      }
      
      threadData_ = new ThreadData;
    }
    
    void initContext_(ThreadContext* context){
      context->pushScope(mainContext_.getScope(0));
      context->pushScope(mainContext_.getScope(1));
      
      if(sharedScope_){
        context->pushScope(mainContext_.getScope(2));
      }
    }
    
    ThreadContext* getContext(){
      if(threadData_){
        NThread::id threadId = NThread::thisThreadId();
        
        if(threadId == NThread::mainThreadId){
          return &mainContext_;
        }
        
        return threadData_->getContext(this, threadId);
      }
      
      return &mainContext_;
    }
    
    void getSymbol(ThreadContext* context, const nstr& s, nvar& v){
      for(int i = context->scopeStack.size() - 1; i >= 0; --i){
        NScope* scope = context->getScope(i);
        
        if(scope->getSymbol(s, v)){
          return;
        }
        
        if(scope->isLimiting()){
          i = sharedScope_ ? 3 : 2;
        }
      }
      
      if(strict_){
        NERROR("symbol not in scope: " + s);
      }
      
      v = nsym(s);
    }
    
    void getSymbolNone(ThreadContext* context, const nstr& s, nvar& v){
      for(int i = context->scopeStack.size() - 1; i >= 0; --i){
        NScope* scope = context->getScope(i);
        
        if(scope->getSymbol(s, v)){
          return;
        }
        
        if(scope->isLimiting()){
          i = sharedScope_ ? 3 : 2;
        }
      }
      
      v = none;
    }
    
    bool getFunction(ThreadContext* context,
                     const nstr& f,
                     size_t arity,
                     nvar& s,
                     nvar& b){
      for(int i = context->scopeStack.size() - 1; i >= 0; --i){
        NScope* scope = context->getScope(i);
        
        if(scope->getFunction(f, arity, s, b)){
          return true;
        }
        
        if(scope->isLimiting()){
          i = sharedScope_ ? 2 : 1;
        }
      }
      
      return false;
    }
    
    nvar process(const nvar& v, uint32_t flags=0){
      // ******* it can be helpful to uncomment this line for debugging
      //cout << "processing: " << v << endl;
      
      nvar& vd = *v;
      
      switch(vd.type()){
        case nvar::Function:{
          NFunc fp = vd.func();
          
          if(fp){
            return (*fp)(o_, v);
          }
          
          fp = o_->handle(vd, flags);
          
          if(fp){
            return (*fp)(o_, v);
          }
          
          ThreadContext* context = getContext();
          
          nvar s;
          nvar b;
          if(getFunction(context, vd.str(), vd.size(), s, b)){
            NScope scope(true);
            context->pushScope(&scope);
            
            size_t size = vd.size();
            for(size_t i = 0; i < size; ++i){
              scope.setSymbolFast(s[i], vd[i].toPtr());
            }
            
            try{
              nvar r = process(b);
              context->popScope();

              if(r.isFunction()){
                NFunc f = (*r).func();
                if(f == _ret0Func){
                  return none;
                }
                else if(f == _ret1Func){
                  return r[0];
                }
              }
              
              return r;
            }
            catch(NError& e){
              context->popScope();
              throw e;
            }
          }
          
          return Throw(v, "failed to process function");
        }
        case nvar::Symbol:{
          nvar p;
          getSymbol(getContext(), vd, p);
          return p;
        }
        default:
          return v;
      }
      
      return v;
    }

    nvar Throw(const nvar& v1, const nvar& v2){
      nstr msg = v1.toStr() + ": ";
      
      nstr loc = v1.getLocation();
      if(!loc.empty()){
        msg += loc + ": ";
      }
      
      msg += process(v2).str();
      
      NERROR(msg);
      
      return none;
    }

    nvar Import(const nvar& v){
      const nstr& className = v;
      
      NObjectBase* ob = NClass::create(nvar(className, nvar::Func));
      if(!ob){
        return Throw(v, "Import[0] failed[1]");
      }
      
      NObject* o = dynamic_cast<NObject*>(ob);
      if(!o){
        return Throw(v, "Import[0] failed[2]");
      }

      nvar ov(o, nvar::SharedObject);
      
      NClass* c = NClass::getClass(className);
      assert(c);
      
      ThreadContext* context = getContext();
      NScope* scope = context->topScope();
      
      const nmap& m = c->metadata()[className]["methods"];
      
      for(auto& itr : m){
        const nvar& k = itr.first;

        nvar f = nfunc(k[0]);
        size_t size = k[1];

        for(size_t i = 0; i < size; ++i){
          f << nsym("p" + nvar(i));
        }
        
        scope->setFunction(f, nfunc("Ret") << (nfunc("Call") << ov << f));
      }
      
      return none;
    }
    
    nvar Reset(){
      if(sharedScope_){
        mainContext_.getScope(2)->clear();
      }
      else{
        mainContext_.getScope(1)->clear();
      }
      
      return none;
    }
    
    nvar Add(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 + p2;
    }
    
    nvar Sub(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 - p2;
    }
    
    nvar Mul(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 * p2;
    }
    
    nvar Div(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 / p2;
    }
    
    nvar Mod(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 % p2;
    }
    
    nvar Neg(const nvar& v){
      nvar p = process(v);
      
      return -p;
    }
    
    nvar AddBy(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 += p2;
    }
    
    nvar SubBy(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 -= p2;
    }
    
    nvar MulBy(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 *= p2;
    }
    
    nvar DivBy(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 /= p2;
    }
    
    nvar ModBy(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 %= p2;
    }
    
    nvar Inc(const nvar& v){
      nvar p = process(v);
      
      return ++p;
    }
    
    nvar PostInc(const nvar& v){
      nvar p = process(v);
      
      return p++;
    }
    
    nvar Dec(const nvar& v){
      nvar p = process(v);
      
      return --p;
    }
    
    nvar PostDec(const nvar& v){
      nvar p = process(v);
      
      return p--;
    }
    
    nvar LT(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 < p2;
    }
    
    nvar LE(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 <= p2;
    }
    
    nvar GT(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 > p2;
    }
    
    nvar GE(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 >= p2;
    }
    
    nvar EQ(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 == p2;
    }
    
    nvar NE(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 != p2;
    }
    
    nvar And(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 && p2;
    }
    
    nvar Or(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 || p2;
    }
    
    nvar Not(const nvar& v){
      nvar p = process(v);
      
      return !p;
    }

    nvar Pow(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return nvar::pow(p1, p2, exact_ ? o_ : 0);
    }
    
    nvar Sqrt(const nvar& v){
      nvar p = process(v);
      
      return nvar::sqrt(p, exact_ ? o_ : 0);
    }
    
    nvar Exp(const nvar& v){
      nvar p = process(v);
      
      return nvar::exp(p, exact_ ? o_ : 0);
    }
    
    nvar Abs(const nvar& v){
      nvar p = process(v);
      
      return nvar::abs(p);
    }
    
    nvar Floor(const nvar& v){
      nvar p = process(v);
      
      return nvar::floor(p);
    }
    
    nvar Ceil(const nvar& v){
      nvar p = process(v);
      
      return nvar::ceil(p);
    }
    
    nvar Log10(const nvar& v){
      nvar p = process(v);
      
      return nvar::log10(p, exact_ ? o_ : 0);
    }
    
    nvar Log(const nvar& v){
      nvar p = process(v);
      
      return nvar::log(p, exact_ ? o_ : 0);
    }
    
    nvar Cos(const nvar& v){
      nvar p = process(v);
      
      return nvar::cos(p, exact_ ? o_ : 0);
    }
    
    nvar Acos(const nvar& v){
      nvar p = process(v);
      
      return nvar::acos(p, exact_ ? o_ : 0);
    }
    
    nvar Cosh(const nvar& v){
      nvar p = process(v);
      
      return nvar::cosh(p, exact_ ? o_ : 0);
    }
    
    nvar Sin(const nvar& v){
      nvar p = process(v);
      
      return nvar::sin(p, exact_ ? o_ : 0);
    }
    
    nvar Asin(const nvar& v){
      nvar p = process(v);
      
      return nvar::asin(p, exact_ ? o_ : 0);
    }
    
    nvar Sinh(const nvar& v){
      nvar p = process(v);
      
      return nvar::sinh(p, exact_ ? o_ : 0);
    }
    
    nvar Tan(const nvar& v){
      nvar p = process(v);
      
      return nvar::tan(p, exact_ ? o_ : 0);
    }
    
    nvar Atan(const nvar& v){
      nvar p = process(v);
      
      return nvar::atan(p, exact_ ? o_ : 0);
    }
    
    nvar Tanh(const nvar& v){
      nvar p = process(v);
      
      return nvar::tanh(p, exact_ ? o_ : 0);
    }
    
    nvar Var(const nvar& v){
      ThreadContext* context = getContext();
      NScope* currentScope = context->topScope();
      
      nvar r = new nvar;
      
      currentScope->setSymbol(v, r);
      
      return r;
    }
    
    nvar Var(const nvar& v1, const nvar& v2){
      ThreadContext* context = getContext();
      NScope* currentScope = context->topScope();
      
      nvar r = new nvar(process(v2));
      
      currentScope->setSymbol(v1, r);
      
      return r;
    }
    
    nvar Var(const nvar& v1, const nvar& v2, const nvar& v3){
      ThreadContext* context = getContext();
      NScope* currentScope = context->topScope();
      
      nvar p1 = process(v2);
      
      nvar r;
      if(v3.get("shared", false)){
        r = nvar(p1.obj(), nvar::SharedObject);
      }
      else if(v3.get("local", false)){
        r = nvar(p1.obj(), nvar::LocalObject);
      }
      else{
        r = new nvar(move(p1));
      }
      
      currentScope->setSymbol(v1, r);
      
      return r;
    }
    
    nvar Set(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1.set(p2);
    }
    
    nvar VarSet(const nvar& v1, const nvar& v2){
      nvar p2 = process(v2);
      
      ThreadContext* context = getContext();
      
      nvar s;
      getSymbolNone(context, v1, s);
      
      if(s.some()){
        s.set(p2);
      }
      else{
        s = new nvar(*p2);
        NScope* currentScope = context->topScope();
        
        currentScope->setSymbol(v1, s);
      }
      
      return s;
    }
    
    nvar Get(const nvar& v){
      ThreadContext* context = getContext();
      
      nvar r;
      getSymbolNone(context, v, r);
      
      return r;
    }

    nvar Get(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1.get(p2).toPtr();
    }
    
    nvar Idx(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1[p2].toPtr();
    }
    
    nvar Dot(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      
      return p1[v2].toPtr();
    }
    
    nvar Put(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1(p2).toPtr();
    }
    
    nvar DotPut(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      
      return p1(v2).toPtr();
    }
    
    nvar Cs(const nvar& v){
      return v;
    }
    
    nvar Call(const nvar& v){
      const nvar& p = *v;
      
      nvar f(v.str(), nvar::Func);
      
      size_t size = p.size();
      for(size_t i = 0; i < size; ++i){
        f << process(p[i]);
      }
      
      return process(f);
    }
    
    nvar Call(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      
      const nvar& p2 = *v2;
      
      NObject* o = static_cast<NObject*>(p1.obj());
      
      nvar f(v2.str(), nvar::Func);
      
      size_t size = p2.size();
      for(size_t i = 0; i < size; ++i){
        f << process(p2[i]);
      }
      
      return o->process(f);
    }
    
    nvar Def(const nvar& v1, const nvar& v2){
      ThreadContext* context = getContext();
      
      NScope* scope = context->topScope();
      scope->setFunction(v1, v2);

      return none;
    }
    
    nvar Def(const nvar& v1, const nvar& v2, const nvar& v3){
      nvar p1 = process(v1);
      
      NScope* scope;
      
      try{
        scope = toScope(p1);
      }
      catch(NError& e){
        return Throw(v1, "Def[0] is not a scope");
      }
      
      scope->setFunction(v2, v3);

      return none;
    }
    
    nvar DefSym(const nvar& v1, const nvar& v2){
      ThreadContext* context = getContext();
      
      NScope* scope = context->topScope();
      scope->setSymbol(v1, v2);
      
      return none;
    }
    
    nvar DefSym(const nvar& v1, const nvar& v2, const nvar& v3){
      nvar p1 = process(v1);
      
      NScope* scope;
      
      try{
        scope = toScope(p1);
      }
      catch(NError& e){
        return Throw(v1, "DefSym[0] is not a scope");
      }
      
      scope->setSymbol(v2, v3);
      
      return none;
    }
    
    nvar In(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      NObject* o = static_cast<NObject*>(p1.obj());
 
      return o->process(v2);
    }
    
    nvar New(const nvar& v){
      NObjectBase* o = NClass::create(v);
      
      if(o){
        return o;
      }
      
      ThreadContext* context = getContext();

      nstr n = "__class_" + v.str();
      nvar s;
      
      getSymbolNone(context, n, s);
      if(s.some()){
        const nmap& m = s["ctors"];

        auto itr = m.find(v.size());
        if(itr == m.end()){
          return Throw(v, "New[0] invalid ctor");
        }

        const nvar& ctor = itr->second;
        const nvar& f = ctor[1];
        
        NScope scope(true);
        context->pushScope(&scope);

        for(size_t i = 0; i < f.size(); ++i){
          const nvar& si = f[i];
          const nvar& pi = v[i];
          
          scope.setSymbolFast(si, pi);
        }

        NObject* o;
        try{
          o = New(ctor[0]).ptr<NObject>();
        }
        catch(NError& e){
          context->popScope();
          return Throw(v, "New[0] failed to create base object");
        }

        context->popScope();
        
        o->process(s["stmts"]);
        
        o->PushScope(&scope);
        try{
          o->process(ctor[2]);
        }
        catch(NError& e){
          delete o;
          return Throw(v, "New[0] failed to construct object");
        }
        o->PopScope();

        return o;
      }
      
      return Throw(v, "New[0] failed to create: " + v);
    }
    
    nvar New(const nvar& v1, const nvar& v2){
      NObjectBase* o = NClass::create(v1);
      
      if(o){
        if(v2.get("shared", false)){
          return nvar(o, nvar::SharedObject);
        }
        else if(v2.get("local", false)){
          return nvar(o, nvar::LocalObject);
        }
        else{
          return o;
        }
      }
      
      return Throw(v1, "New[0] failed to create: " + v1);
    }

    nvar Ret(const nvar& v){
      nvar p = process(v);
      
      nvar r = nfunc("Ret") << move(p);
      (*r).setFunc(_ret1Func);
      
      return r;
    }
    
    nvar Block_n(const nvar& v){
      size_t size = v.size();
      nvar r = none;

      for(size_t i = 0; i < size; ++i){
        r = process(v[i]);
        if(r.isFunction()){
          NFunc f = (*r).func();
          if(f == _ret0Func ||
             f == _ret1Func ||
             f == _breakFunc ||
             f == _continueFunc){
            return r;
          }
        }
      }
      
      return r;
    }
    
    nvar ScopedBlock_n(const nvar& v){
      ThreadContext* context = getContext();
      
      NScope scope;
      context->pushScope(&scope);
      
      size_t size = v.size();
      nvar r = none;

      for(size_t i = 0; i < size; ++i){
        try{
          r = process(v[i]);
        }
        catch(NError& e){
          context->popScope();
          throw e;
        }
        
        if(r.isFunction()){
          NFunc f = (*r).func();
          if(f == _ret0Func ||
             f == _ret1Func ||
             f == _breakFunc ||
             f == _continueFunc){
            context->popScope();
            return r;
          }
        }
      }
      
      context->popScope();
      
      return r;
    }
    
    nvar Print_n(const nvar& v){
      nstr s = process(v[0]);
      
      size_t size = v.size();
      size_t pos = 0;

      for(size_t i = 1; i < size; ++i){
        size_t p = s.find("%v", pos);
      
        if(p == nstr::npos){
          return Throw(v, "Print[" + nvar(i) + "] missing token");
        }

        nstr ri = process(v[i]).toStr();
        
        s.replace(p, 2, ri);

        pos = p + ri.length();
      }
      
      cout << s << endl;
      
      return none;
    }
    
    nvar PushScope(const nvar& v){
      nvar p = process(v);
      
      NScope* scope = toScope(p);
      if(!scope){
        return Throw(v, "PushScope[0] is not a scope");
      }
      
      ThreadContext* context = getContext();
      context->pushScope(scope);

      return none;
    }
    
    nvar PopScope(){
      ThreadContext* context = getContext();
      context->popScope();
      
      return none;
    }
    
    nvar PushBack(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.pushBack(p2);
      
      return p1.toPtr();
    }

    nvar TouchVector(const nvar& v1){
      nvar p1 = process(v1);
      
      p1.intoVector();
      
      return none;
    }

    nvar TouchList(const nvar& v1){
      nvar p1 = process(v1);
      
      p1.intoList();
      
      return none;
    }
    
    nvar TouchQueue(const nvar& v1){
      nvar p1 = process(v1);
      
      p1.intoQueue();
      
      return none;
    }
    
    nvar TouchSet(const nvar& v1){
      nvar p1 = process(v1);
      
      p1.intoSet();
      
      return none;
    }
    
    nvar TouchHashSet(const nvar& v1){
      nvar p1 = process(v1);
      
      p1.intoHashSet();
      
      return none;
    }
    
    nvar TouchMap(const nvar& v1){
      nvar p1 = process(v1);
      
      p1.intoMap();
      
      return none;
    }
    
    nvar TouchHashMap(const nvar& v1){
      nvar p1 = process(v1);
      
      p1.intoHashMap();
      
      return none;
    }
    
    nvar TouchMultimap(const nvar& v1){
      nvar p1 = process(v1);
      
      p1.intoMultimap();
      
      return none;
    }
    
    nvar Keys(const nvar& v1){
      nvar p1 = process(v1);
      
      return p1.keys();
    }
    
    nvar PushFront(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.pushFront(p2);
      
      return none;
    }
    
    nvar PopBack(const nvar& v1){
      nvar p1 = process(v1);
      
      return p1.popBack();
    }
    
    nvar HasKey(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1.hasKey(p2);
    }
    
    nvar Insert(const nvar& v1, const nvar& v2, const nvar& v3){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      nvar p3 = process(v3);
      
      p1.insert(v2, v3);
      
      return none;
    }
    
    nvar Clear(const nvar& v1){
      nvar p1 = process(v1);
      
      p1.clear();
      
      return none;
    }
    
    nvar Empty(const nvar& v1){
      nvar p1 = process(v1);
      
      return p1.empty();
    }
    
    nvar Back(const nvar& v1){
      nvar p1 = process(v1);
      
      return p1.back();
    }
    
    nvar Erase(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.erase(p2);
      
      return none;
    }
    
    nvar Merge(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.merge(p2);
      
      return none;
    }
    
    nvar OuterMerge(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.outerMerge(p2);
      
      return none;
    }
    
    nvar Class(const nvar& v1){
      nstr n = "__class_" + v1["name"].str();
      
      ThreadContext* context = getContext();
      NScope* scope = context->topScope();
      scope->setSymbol(n, v1);
      
      return none;
    }
    
    nvar For(const nvar& v1, const nvar& v2, const nvar& v3, const nvar& v4){
      process(v1);
      
      for(;;){
        nvar c = process(v2);
        if(!c){
          return none;
        }

        nvar r = process(v4);

        bool should = true;
        if(r.isFunction()){
          NFunc f = (*r).func();
          if(f == _ret0Func || f == _ret1Func){
            return r;
          }
          else if(f == _breakFunc){
            return none;
          }
          else if(f == _continueFunc){
            should = false;
          }
        }
        
        if(should){
          process(v3);
        }
      }
      
      return none;
    }
    
    nvar While(const nvar& v1, const nvar& v2){
      for(;;){
        nvar c = process(v1);
        if(!c){
          return none;
        }
        
        nvar r = process(v2);
        
        bool should = true;
        if(r.isFunction()){
          NFunc f = (*r).func();
          if(f == _ret0Func || f == _ret1Func){
            return r;
          }
          else if(f == _breakFunc){
            return none;
          }
        }
      }
    }
    
    nvar If(const nvar& v1, const nvar& v2){
      if(process(v1)){
        return process(v2);
      }
      
      return none;
    }
    
    nvar If(const nvar& v1, const nvar& v2, const nvar& v3){
      if(process(v1)){
        return process(v2);
      }
      else{
        return process(v3);
      }
    }
    
    nvar Switch(const nvar& v1, const nvar& v2, const nvar& v3){
      nvar p1 = process(v1);
      const nmap& m = v3;
      
      auto itr = m.find(p1);
      if(itr == m.end()){
        return process(v2);
      }
      
      return process(itr->second);
    }
    
    nvar IsFalse(const nvar& v){
      nvar p = process(v);
      
      return p.isFalse();
    }
    
    nvar IsTrue(const nvar& v){
      nvar p = process(v);
      
      return p.isTrue();
    }
    
    nvar IsDefined(const nvar& v){
      nvar p = process(v);
      
      return p.isDefined();
    }
    
    nvar IsString(const nvar& v){
      nvar p = process(v);
      
      return p.isString();
    }
    
    nvar IsSymbol(const nvar& v){
      nvar p = process(v);
      
      return p.isSymbol();
    }
    
    nvar IsFunction(const nvar& v){
      nvar p = process(v);
      
      return p.isFunction();
    }
    
    nvar IsFunction(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1.isFunction(p2);
    }
    
    nvar IsFunction(const nvar& v1, const nvar& v2, const nvar& v3){
      nvar p1 = process(v1);
      nstr f = process(v2);
      size_t arity = process(v3);
      
      return p1.isFunction(f, arity);
    }
    
    nvar IsSymbolic(const nvar& v){
      nvar p = process(v);
      
      return p.isSymbolic();
    }
    
    nvar IsNumeric(const nvar& v){
      nvar p = process(v);
      
      return p.isNumeric();
    }
    
    nvar IsReference(const nvar& v){
      nvar p = process(v);
      
      return p.isReference();
    }
    
    nvar IsPointer(const nvar& v){
      nvar p = process(v);
      
      return p.isPointer();
    }
    
    nvar IsInteger(const nvar& v){
      nvar p = process(v);
      
      return p.isInteger();
    }
    
    nvar IsRational(const nvar& v){
      nvar p = process(v);
      
      return p.isRational();
    }
    
    nvar IsReal(const nvar& v){
      nvar p = process(v);
      
      return p.isReal();
    }
    
    nvar GetStr(const nvar& v){
      nvar p = process(v);
      
      return p.str();
    }
    
    nvar GetVec(const nvar& v){
      nvar p = process(v);
      
      return p.vec();
    }
    
    nvar GetList(const nvar& v){
      nvar p = process(v);
      
      return p.list();
    }
    
    nvar GetAnySequence(const nvar& v){
      nvar p = process(v);
      
      return p.anySequence();
    }
    
    nvar GetMap(const nvar& v){
      nvar p = process(v);
      
      return p.map();
    }
    
    nvar GetMultimap(const nvar& v){
      nvar p = process(v);
      
      return p.multimap();
    }
    
    nvar GetAnyMap(const nvar& v){
      nvar p = process(v);
      
      return p.anyMap();
    }
    
    nvar Append(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.append(p2);
      
      return none;
    }
    
    nvar Normalize(const nvar& v){
      nvar p = process(v);
      
      return p.normalize();
    }
    
    nvar Head(const nvar& v){
      nvar p = process(v);
      
      return p.head();
    }
    
    nvar SetHead(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.setHead(p2);
      
      return none;
    }
    
    nvar ClearHead(const nvar& v){
      nvar p = process(v);
      p.clearHead();
      
      return none;
    }
    
    nvar NumKeys(const nvar& v){
      nvar p = process(v);
      
      return p.numKeys();
    }
    
    nvar Size(const nvar& v){
      nvar p = process(v);
      
      return p.size();
    }
    
    nvar MapEmpty(const nvar& v){
      nvar p = process(v);
      
      return p.mapEmpty();
    }
    
    nvar AllEmpty(const nvar& v){
      nvar p = process(v);
      
      return p.allEmpty();
    }
    
    nvar HasVector(const nvar& v){
      nvar p = process(v);
      
      return p.hasVector();
    }
    
    nvar HasList(const nvar& v){
      nvar p = process(v);
      
      return p.hasList();
    }
    
    nvar HasMap(const nvar& v){
      nvar p = process(v);
      
      return p.hasMap();
    }
    
    nvar HasMultimap(const nvar& v){
      nvar p = process(v);
      
      return p.hasMultimap();
    }
    
    nvar PopFront(const nvar& v){
      nvar p = process(v);
      
      return p.popFront();
    }
    
    nvar AllKeys(const nvar& v){
      nvar p = process(v);
      
      return p.allKeys();
    }
    
    nvar Open(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.open(p2);
      
      return none;
    }
    
    nvar Save(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.save(p2);
      
      return none;
    }
    
    nvar Unite(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.unite(p2);
      
      return p1;
    }
    
    nvar Intersect(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.intersect(p2);
      
      return p1;
    }
    
    nvar Complement(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.complement(p2);
      
      return p1;
    }
    
    nvar Inf(){
      return nvar::inf();
    }
    
    nvar NegInf(){
      return nvar::negInf();
    }
    
    nvar Nan(){
      return nvar::nan();
    }
    
    nvar Min(){
      return nvar::min();
    }
    
    nvar Max(){
      return nvar::max();
    }
    
    nvar Epsilon(){
      return nvar::epsilon();
    }
    
    nvar Max(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return nvar::max(p1, p2);
    }
    
    nvar Min(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return nvar::min(p1, p2);
    }
    
    nvar Func(const nvar& v){
      nvar p = process(v);
      
      return nfunc(p);
    }
    
    nvar Sym(const nvar& v){
      nvar p = process(v);
      
      return nsym(p);
    }
    
    nvar VarRef(const nvar& v){
      nvar p = process(v);
      
      return nref(p);
    }
    
    nvar VarPtr(const nvar& v){
      nvar p = process(v);
      
      return nptr(p);
    }
    
    nvar NML(const nvar& v){
      nvar p = process(v);
      
      return nml(p);
    }
    
    void foo(nvar& x){
      cout << "called foo" << endl;
      
      x = 29;
    }
    
    NScope* currentScope(){
      ThreadContext* context = getContext();
      
      return context->topScope();
    }
    
    NScope* objectScope(){
      ThreadContext* context = getContext();
      
      return sharedScope_ ? context->scopeStack[2] : context->scopeStack[1];
    }
    
    NScope* toScope(const nvar& v){
      NObjectBase* o = v.obj();
      if(o->instanceOf(NScope::classId)){
        return static_cast<NScope*>(o);
      }
      
      return static_cast<NObject*>(o)->objectScope();
    }
    
    nvar remoteProcess(const nvar& n){
      assert(broker_);
      
      return broker_->process_(o_, n);
    }
    
    void dumpScopes(){
      ThreadContext* context = getContext();
      context->dumpScopes();
    }
    
  private:
    NObject* o_;
    
    ThreadContext mainContext_;
    ThreadData* threadData_;
    NBroker* broker_;
    
    bool exact_ : 1;
    bool strict_ : 1;
    bool sharedScope_ : 1;
  };
  
} // end namespace neu

FuncMap::FuncMap(){
  add("Import", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Import(v[0]);
      });
  
  add("Reset", 0,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Reset();
      });
  
  add("Add", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Add(v[0], v[1]);
      });
  
  add("Sub", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Sub(v[0], v[1]);
      });
  
  add("Mul", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Mul(v[0], v[1]);
      });
  
  add("Div", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Div(v[0], v[1]);
      });
  
  add("Mod", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Mod(v[0], v[1]);
      });
  
  add("Neg", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Neg(v[0]);
      });
  
  add("AddBy", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->AddBy(v[0], v[1]);
      });
  
  add("SubBy", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->SubBy(v[0], v[1]);
      });
  
  add("MulBy", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->MulBy(v[0], v[1]);
      });
  
  add("DivBy", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->DivBy(v[0], v[1]);
      });
  
  add("ModBy", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->ModBy(v[0], v[1]);
      });
  
  add("Inc", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Inc(v[0]);
      });
  
  add("PostInc", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->PostInc(v[0]);
      });
  
  add("Dec", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Dec(v[0]);
      });
  
  add("PostDec", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->PostDec(v[0]);
      });
  
  add("LT", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->LT(v[0], v[1]);
      });
  
  add("LE", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->LE(v[0], v[1]);
      });
  
  add("GT", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->GT(v[0], v[1]);
      });
  
  add("GE", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->GE(v[0], v[1]);
      });
  
  add("EQ", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->EQ(v[0], v[1]);
      });
  
  add("NE", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->NE(v[0], v[1]);
      });
  
  add("And", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->And(v[0], v[1]);
      });
  
  add("Or", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Or(v[0], v[1]);
      });
  
  add("Not", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Not(v[0]);
      });

  add("Pow", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Pow(v[0], v[1]);
      });
  
  add("Sqrt", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Sqrt(v[0]);
      });
  
  add("Exp", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Exp(v[0]);
      });
  
  add("Abs", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Abs(v[0]);
      });
  
  add("Floor", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Floor(v[0]);
      });
  
  add("Ceil", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Ceil(v[0]);
      });
  
  add("Log10", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Log10(v[0]);
      });
  
  add("Log", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Log(v[0]);
      });
  
  add("Cos", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Cos(v[0]);
      });
  
  add("Acos", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Acos(v[0]);
      });
  
  add("Cosh", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Cosh(v[0]);
      });
  
  add("Sin", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Sin(v[0]);
      });
  
  add("Asin", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Asin(v[0]);
      });
  
  add("Sinh", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Sinh(v[0]);
      });
  
  add("Tan", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Tan(v[0]);
      });
  
  add("Atan", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Atan(v[0]);
      });
  
  add("Tanh", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Tanh(v[0]);
      });
  
  add("Var", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Var(v[0]);
      });
  
  add("Var", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Var(v[0], v[1]);
      });
  
  add("Var", 3,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Var(v[0], v[1], v[2]);
      });
  
  add("Set", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Set(v[0], v[1]);
      });
  
  add("VarSet", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->VarSet(v[0], v[1]);
      });
  
  add("Get", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Get(v[0]);
      });

  add("Get", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Get(v[0], v[1]);
      });
  
  add("Idx", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Idx(v[0], v[1]);
      });
  
  add("Dot", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Dot(v[0], v[1]);
      });
  
  add("Put", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Put(v[0], v[1]);
      });
  
  add("DotPut", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->DotPut(v[0], v[1]);
      });
  
  add("Cs", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Cs(v[0]);
      });
  
  add("In", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->In(v[0], v[1]);
      });
  
  add("Call", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Call(v[0]);
      });
  
  add("Call", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Call(v[0], v[1]);
      });
  
  add("Def", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Def(v[0], v[1]);
      });
  
  add("Def", 3,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Def(v[0], v[1], v[2]);
      });
  
  add("DefSym", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->DefSym(v[0], v[1]);
      });
  
  add("DefSym", 3,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->DefSym(v[0], v[1], v[2]);
      });
  
  add("New", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->New(v[0]);
      });
  
  add("New", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->New(v[0], v[1]);
      });
  
  add("Block",
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Block_n(v);
      });
  
  add("ScopedBlock",
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->ScopedBlock_n(v);
      });
  
  add("Print",
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Print_n(v);
      });
  
  _ret0Func =
  add("Ret", 0,
      [](void* o, const nvar& v) -> nvar{
        return v;
      });
  
  _ret1Func =
  add("Ret", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Ret(v[0]);
      });
  
  _breakFunc =
  add("Break", 0,
      [](void* o, const nvar& v) -> nvar{
        return v;
      });
  
  _continueFunc =
  add("Continue", 0,
      [](void* o, const nvar& v) -> nvar{
        return v;
      });
  
  add("PushScope", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->PushScope(v[0]);
      });
  
  add("PopScope", 0,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->PopScope();
      });
  
  add("PushBack", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->PushBack(v[0], v[1]);
      });
  
  add("TouchVector", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->TouchVector(v[0]);
      });
  
  add("TouchList", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->TouchList(v[0]);
      });
  
  add("TouchQueue", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->TouchQueue(v[0]);
      });
  
  add("TouchSet", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->TouchSet(v[0]);
      });
  
  add("TouchHashSet", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->TouchHashSet(v[0]);
      });
  
  add("TouchMap", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->TouchMap(v[0]);
      });

  add("TouchHashMap", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->TouchHashMap(v[0]);
      });
  
  add("TouchMultimap", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->TouchMultimap(v[0]);
      });
  
  add("Keys", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Keys(v[0]);
      });
  
  add("PushFront", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->PushFront(v[0], v[1]);
      });
  
  add("PopBack", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->PopBack(v[0]);
      });
  
  add("HasKey", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->HasKey(v[0], v[1]);
      });
  
  add("Insert", 3,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Insert(v[0], v[1], v[2]);
      });
  
  add("Clear", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Clear(v[0]);
      });
  
  add("Empty", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Empty(v[0]);
      });
  
  add("Back", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Back(v[0]);
      });
  
  add("Erase", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Erase(v[0], v[1]);
      });
  
  add("Merge", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Merge(v[0], v[1]);
      });
  
  add("OuterMerge", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->OuterMerge(v[0], v[1]);
      });

  add("If", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->If(v[0], v[1]);
      });
  
  add("If", 3,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->If(v[0], v[1], v[2]);
      });
  
  add("For", 4,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->For(v[0], v[1], v[2], v[3]);
      });
  
  add("While", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->While(v[0], v[1]);
      });
  
  add("Switch", 3,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Switch(v[0], v[1], v[2]);
      });

  add("Class", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Class(v[0]);
      });
  
  add("IsFalse", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsFalse(v[0]);
      });
  
  add("IsTrue", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsTrue(v[0]);
      });
  
  add("IsDefined", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsDefined(v[0]);
      });
  
  add("IsString", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsString(v[0]);
      });
  
  add("IsSymbol", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsSymbol(v[0]);
      });
  
  add("IsFunction", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsFunction(v[0]);
      });
  
  add("IsFunction", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsFunction(v[0], v[1]);
      });
  
  add("IsFunction", 3,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsFunction(v[0], v[1], v[2]);
      });

  add("IsSymbolic", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsSymbolic(v[0]);
      });
  
  add("IsNumeric", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsNumeric(v[0]);
      });
  
  add("IsReference", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsReference(v[0]);
      });
  
  add("IsPointer", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsPointer(v[0]);
      });
  
  add("IsInteger", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsInteger(v[0]);
      });
  
  add("IsRational", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsRational(v[0]);
      });
  
  add("IsReal", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->IsReal(v[0]);
      });
  
  add("GetStr", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->GetStr(v[0]);
      });
  
  add("GetVec", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->GetVec(v[0]);
      });
  
  add("GetList", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->GetList(v[0]);
      });
  
  add("GetAnySequence", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->GetAnySequence(v[0]);
      });
  
  add("GetMap", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->GetMap(v[0]);
      });
  
  add("GetMultimap", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->GetMultimap(v[0]);
      });
  
  add("GetAnyMap", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->GetAnyMap(v[0]);
      });
  
  add("Append", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Append(v[0], v[1]);
      });
  
  add("Normalize", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Normalize(v[0]);
      });
  
  add("Head", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Head(v[0]);
      });
  
  add("SetHead", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->SetHead(v[0], v[1]);
      });
  
  add("ClearHead", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->ClearHead(v[0]);
      });
  
  add("NumKeys", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->NumKeys(v[0]);
      });
  
  add("Size", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Size(v[0]);
      });
  
  add("MapEmpty", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->MapEmpty(v[0]);
      });
  
  add("AllEmpty", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->AllEmpty(v[0]);
      });
  
  add("HasVector", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->HasVector(v[0]);
      });
  
  add("HasList", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->HasList(v[0]);
      });
  
  add("HasMap", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->HasMap(v[0]);
      });
  
  add("HasMultimap", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->HasMultimap(v[0]);
      });
  
  add("PopFront", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->PopFront(v[0]);
      });
  
  add("AllKeys", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->AllKeys(v[0]);
      });
  
  add("Open", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Open(v[0], v[1]);
      });
  
  add("Save", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Save(v[0], v[1]);
      });
  
  add("Unite", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Unite(v[0], v[1]);
      });
  
  add("Intersect", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Intersect(v[0], v[1]);
      });
  
  add("Complement", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Complement(v[0], v[1]);
      });
  
  add("Inf", 0,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Inf();
      });
  
  add("NegInf", 0,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->NegInf();
      });
  
  add("Nan", 0,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Nan();
      });
  
  add("Min", 0,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Min();
      });
  
  add("Max", 0,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Max();
      });
  
  add("Epsilon", 0,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Epsilon();
      });
  
  add("Max", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Max(v[0], v[1]);
      });
  
  add("Min", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Min(v[0], v[1]);
      });
  
  add("Func", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Func(v[0]);
      });
  
  add("Sym", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->Sym(v[0]);
      });
  
  add("VarRef", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->VarRef(v[0]);
      });
  
  add("VarPtr", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->VarPtr(v[0]);
      });
  
  add("NML", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::obj(o)->NML(v[0]);
      });
  
  add("foo", 1,
      [](void* o, const nvar& v) -> nvar{
        NObject_::obj(o)->foo(*v[0]);
        return none;
      });
  
  add("dumpScopes", 0,
      [](void* o, const nvar& v) -> nvar{
        NObject_::obj(o)->dumpScopes();
        return none;
      });
}

NObject::NObject(){
  x_ = new NObject_(this);
}

NObject::NObject(NScope* sharedScope){
  x_ = new NObject_(this, sharedScope);
}

NObject::NObject(NBroker* broker){
  x_ = new NObject_(this, broker);
}

NObject::~NObject(){
  delete x_;
}

void NObject::enableThreading(){
  x_->enableThreading();
}

nvar NObject::process(const nvar& v, uint32_t flags){
  return x_->process(v, flags);
}

nvar NObject::remoteProcess(const nvar& v){
  return x_->remoteProcess(v);
}

NFunc NObject::handle(const nvar& v, uint32_t flags){
  return _funcMap.map(v);
}

int NObject::precedence(const nvar& f){
  return _global.precedence(f);
}

void NObject::setStrict(bool flag){
  x_->setStrict(flag);
}

void NObject::setExact(bool flag){
  x_->setExact(flag);
}

bool NObject::isRemote(){
  return x_->isRemote();
}

nvar NObject::Reset(){
  return x_->Reset();
}

nvar NObject::Throw(const nvar& v1, const nvar& v2){
  return x_->Throw(v1, v2);
}

nvar NObject::Add(const nvar& v1, const nvar& v2){
  return x_->Add(v1, v2);
}

nvar NObject::Sub(const nvar& v1, const nvar& v2){
  return x_->Sub(v1, v2);
}

nvar NObject::Mul(const nvar& v1, const nvar& v2){
  return x_->Mul(v1, v2);
}

nvar NObject::Div(const nvar& v1, const nvar& v2){
  return x_->Div(v1, v2);
}

nvar NObject::Mod(const nvar& v1, const nvar& v2){
  return x_->Mod(v1, v2);
}

nvar NObject::Neg(const nvar& v){
  return x_->Neg(v);
}

nvar NObject::AddBy(const nvar& v1, const nvar& v2){
  return x_->AddBy(v1, v2);
}

nvar NObject::SubBy(const nvar& v1, const nvar& v2){
  return x_->SubBy(v1, v2);
}

nvar NObject::MulBy(const nvar& v1, const nvar& v2){
  return x_->MulBy(v1, v2);
}

nvar NObject::DivBy(const nvar& v1, const nvar& v2){
  return x_->DivBy(v1, v2);
}

nvar NObject::ModBy(const nvar& v1, const nvar& v2){
  return x_->ModBy(v1, v2);
}

nvar NObject::Inc(const nvar& v){
  return x_->Inc(v);
}

nvar NObject::PostInc(const nvar& v){
  return x_->PostInc(v);
}

nvar NObject::Dec(const nvar& v){
  return x_->Dec(v);
}

nvar NObject::PostDec(const nvar& v){
  return x_->PostDec(v);
}

nvar NObject::LT(const nvar& v1, const nvar& v2){
  return x_->LT(v1, v2);
}

nvar NObject::LE(const nvar& v1, const nvar& v2){
  return x_->LE(v1, v2);
}

nvar NObject::GT(const nvar& v1, const nvar& v2){
  return x_->GT(v1, v2);
}

nvar NObject::GE(const nvar& v1, const nvar& v2){
  return x_->GE(v1, v2);
}

nvar NObject::EQ(const nvar& v1, const nvar& v2){
  return x_->EQ(v1, v2);
}

nvar NObject::NE(const nvar& v1, const nvar& v2){
  return x_->NE(v1, v2);
}

nvar NObject::And(const nvar& v1, const nvar& v2){
  return x_->And(v1, v2);
}

nvar NObject::Or(const nvar& v1, const nvar& v2){
  return x_->Or(v1, v2);
}

nvar NObject::Not(const nvar& v){
  return x_->Not(v);
}

nvar NObject::Pow(const nvar& v1, const nvar& v2){
  return x_->Pow(v1, v2);
}

nvar NObject::Sqrt(const nvar& v){
  return x_->Sqrt(v);
}

nvar NObject::Exp(const nvar& v){
  return x_->Exp(v);
}

nvar NObject::Abs(const nvar& v){
  return x_->Abs(v);
}

nvar NObject::Floor(const nvar& v){
  return x_->Floor(v);
}

nvar NObject::Ceil(const nvar& v){
  return x_->Ceil(v);
}

nvar NObject::Log10(const nvar& v){
  return x_->Log10(v);
}

nvar NObject::Log(const nvar& v){
  return x_->Log(v);
}

nvar NObject::Cos(const nvar& v){
  return x_->Cos(v);
}

nvar NObject::Acos(const nvar& v){
  return x_->Acos(v);
}

nvar NObject::Cosh(const nvar& v){
  return x_->Cosh(v);
}

nvar NObject::Sin(const nvar& v){
  return x_->Sin(v);
}

nvar NObject::Asin(const nvar& v){
  return x_->Asin(v);
}

nvar NObject::Sinh(const nvar& v){
  return x_->Sinh(v);
}

nvar NObject::Tan(const nvar& v){
  return x_->Tan(v);
}

nvar NObject::Atan(const nvar& v){
  return x_->Atan(v);
}

nvar NObject::Tanh(const nvar& v){
  return x_->Tanh(v);
}

nvar NObject::Var(const nvar& v){
  return x_->Var(v);
}

nvar NObject::Var(const nvar& v1, const nvar& v2){
  return x_->Var(v1, v2);
}

nvar NObject::Var(const nvar& v1, const nvar& v2, const nvar& v3){
  return x_->Var(v1, v2, v3);
}

nvar NObject::Set(const nvar& v1, const nvar& v2){
  return x_->Set(v1, v2);
}

nvar NObject::VarSet(const nvar& v1, const nvar& v2){
  return x_->VarSet(v1, v2);
}

nvar NObject::Get(const nvar& v){
  return x_->Get(v);
}

nvar NObject::Idx(const nvar& v1, const nvar& v2){
  return x_->Idx(v1, v2);
}

nvar NObject::Get(const nvar& v1, const nvar& v2){
  return x_->Get(v1, v2);
}

nvar NObject::Dot(const nvar& v1, const nvar& v2){
  return x_->Dot(v1, v2);
}

nvar NObject::Put(const nvar& v1, const nvar& v2){
  return x_->Put(v1, v2);
}

nvar NObject::DotPut(const nvar& v1, const nvar& v2){
  return x_->DotPut(v1, v2);
}

nvar NObject::Cs(const nvar& v){
  return x_->Cs(v);
}

nvar NObject::In(const nvar& v1, const nvar& v2){
  return x_->In(v1, v2);
}

nvar NObject::Call(const nvar& v){
  return x_->Call(v);
}

nvar NObject::Call(const nvar& v1, const nvar& v2){
  return x_->Call(v1, v2);
}

nvar NObject::Def(const nvar& v1, const nvar& v2){
  return x_->Def(v1, v2);
}

nvar NObject::Def(const nvar& v1, const nvar& v2, const nvar& v3){
  return x_->Def(v1, v2, v3);
}

nvar NObject::DefSym(const nvar& v1, const nvar& v2){
  return x_->DefSym(v1, v2);
}

nvar NObject::DefSym(const nvar& v1, const nvar& v2, const nvar& v3){
  return x_->DefSym(v1, v2, v3);
}

nvar NObject::New(const nvar& v){
  return x_->New(v);
}

nvar NObject::New(const nvar& v1, const nvar& v2){
  return x_->New(v1, v2);
}

nvar NObject::PushScope(const nvar& v){
  return x_->PushScope(v);
}

nvar NObject::PopScope(){
  return x_->PopScope();
}

nvar NObject::PushBack(const nvar& v1, const nvar& v2){
  return x_->PushBack(v1, v2);
}

nvar NObject::TouchVector(const nvar& v1){
  return x_->TouchVector(v1);
}

nvar NObject::TouchList(const nvar& v1){
  return x_->TouchList(v1);
}

nvar NObject::TouchQueue(const nvar& v1){
  return x_->TouchQueue(v1);
}

nvar NObject::TouchSet(const nvar& v1){
  return x_->TouchSet(v1);
}

nvar NObject::TouchHashSet(const nvar& v1){
  return x_->TouchHashSet(v1);
}

nvar NObject::TouchMap(const nvar& v1){
  return x_->TouchMap(v1);
}

nvar NObject::TouchHashMap(const nvar& v1){
  return x_->TouchHashMap(v1);
}

nvar NObject::TouchMultimap(const nvar& v1){
  return x_->TouchMultimap(v1);
}

nvar NObject::Keys(const nvar& v1){
  return x_->Keys(v1);
}

nvar NObject::PushFront(const nvar& v1, const nvar& v2){
  return x_->PushFront(v1, v2);
}

nvar NObject::PopBack(const nvar& v1){
  return x_->PopBack(v1);
}

nvar NObject::HasKey(const nvar& v1, const nvar& v2){
  return x_->HasKey(v1, v2);
}

nvar NObject::Insert(const nvar& v1, const nvar& v2, const nvar& v3){
  return x_->Insert(v1, v2, v3);
}

nvar NObject::Clear(const nvar& v1){
  return x_->Clear(v1);
}

nvar NObject::Empty(const nvar& v1){
  return x_->Empty(v1);
}

nvar NObject::Back(const nvar& v1){
  return x_->Back(v1);
}

nvar NObject::Erase(const nvar& v1, const nvar& v2){
  return x_->Erase(v1, v2);
}

nvar NObject::Merge(const nvar& v1, const nvar& v2){
  return x_->Merge(v1, v2);
}

nvar NObject::OuterMerge(const nvar& v1, const nvar& v2){
  return x_->OuterMerge(v1, v2);
}

nvar NObject::For(const nvar& v1,
                  const nvar& v2,
                  const nvar& v3,
                  const nvar& v4){
  return x_->For(v1, v2, v3, v4);
}

nvar NObject::While(const nvar& v1, const nvar& v2){
  return x_->While(v1, v2);
}

nvar NObject::If(const nvar& v1, const nvar& v2){
  return x_->If(v1, v2);
}

nvar NObject::If(const nvar& v1, const nvar& v2, const nvar& v3){
  return x_->If(v1, v2, v3);
}

nvar NObject::Switch(const nvar& v1, const nvar& v2, const nvar& v3){
  return x_->Switch(v1, v2, v3);
}

nvar NObject::IsFalse(const nvar& v){
  return x_->IsFalse(v);
}

nvar NObject::IsTrue(const nvar& v){
  return x_->IsTrue(v);
}

nvar NObject::IsDefined(const nvar& v){
  return x_->IsDefined(v);
}

nvar NObject::IsString(const nvar& v){
  return x_->IsString(v);
}

nvar NObject::IsSymbol(const nvar& v){
  return x_->IsSymbol(v);
}

nvar NObject::IsFunction(const nvar& v){
  return x_->IsFunction(v);
}

nvar NObject::IsFunction(const nvar& v1, const nvar& v2){
  return x_->IsFunction(v1, v2);
}

nvar NObject::IsFunction(const nvar& v1, const nvar& v2, const nvar& v3){
  return x_->IsFunction(v1, v2, v3);
}

nvar NObject::IsSymbolic(const nvar& v){
  return x_->IsSymbolic(v);
}

nvar NObject::IsNumeric(const nvar& v){
  return x_->IsNumeric(v);
}

nvar NObject::IsReference(const nvar& v){
  return x_->IsReference(v);
}

nvar NObject::IsPointer(const nvar& v){
  return x_->IsPointer(v);
}

nvar NObject::IsInteger(const nvar& v){
  return x_->IsInteger(v);
}

nvar NObject::IsRational(const nvar& v){
  return x_->IsRational(v);
}

nvar NObject::IsReal(const nvar& v){
  return x_->IsReal(v);
}

nvar NObject::GetStr(const nvar& v){
  return x_->GetStr(v);
}

nvar NObject::GetVec(const nvar& v){
  return x_->GetVec(v);
}

nvar NObject::GetList(const nvar& v){
  return x_->GetList(v);
}

nvar NObject::GetAnySequence(const nvar& v){
  return x_->GetAnySequence(v);
}

nvar NObject::GetMap(const nvar& v){
  return x_->GetMap(v);
}

nvar NObject::GetMultimap(const nvar& v){
  return x_->GetMultimap(v);
}

nvar NObject::GetAnyMap(const nvar& v){
  return x_->GetAnyMap(v);
}

nvar NObject::Append(const nvar& v1, const nvar& v2){
  return x_->Append(v1, v2);
}

nvar NObject::Normalize(const nvar& v){
  return x_->Normalize(v);
}

nvar NObject::Head(const nvar& v){
  return x_->Head(v);
}

nvar NObject::SetHead(const nvar& v1, const nvar& v2){
  return x_->SetHead(v1, v2);
}

nvar NObject::ClearHead(const nvar& v){
  return x_->ClearHead(v);
}

nvar NObject::NumKeys(const nvar& v){
  return x_->NumKeys(v);
}

nvar NObject::Size(const nvar& v){
  return x_->Size(v);
}

nvar NObject::MapEmpty(const nvar& v){
  return x_->MapEmpty(v);
}

nvar NObject::AllEmpty(const nvar& v){
  return x_->AllEmpty(v);
}

nvar NObject::HasVector(const nvar& v){
  return x_->HasVector(v);
}

nvar NObject::HasList(const nvar& v){
  return x_->HasList(v);
}

nvar NObject::HasMap(const nvar& v){
  return x_->HasMap(v);
}

nvar NObject::HasMultimap(const nvar& v){
  return x_->HasMultimap(v);
}

nvar NObject::PopFront(const nvar& v){
  return x_->PopFront(v);
}

nvar NObject::AllKeys(const nvar& v){
  return x_->AllKeys(v);
}

nvar NObject::Open(const nvar& v1, const nvar& v2){
  return x_->Open(v1, v2);
}

nvar NObject::Save(const nvar& v1, const nvar& v2){
  return x_->Save(v1, v2);
}

nvar NObject::Unite(const nvar& v1, const nvar& v2){
  return x_->Unite(v1, v2);
}

nvar NObject::Intersect(const nvar& v1, const nvar& v2){
  return x_->Intersect(v1, v2);
}

nvar NObject::Complement(const nvar& v1, const nvar& v2){
  return x_->Complement(v1, v2);
}

void NObject::foo(nvar& x){
  x_->foo(x);
}

NScope* NObject::currentScope(){
  return x_->currentScope();
}

NScope* NObject::objectScope(){
  return x_->objectScope();
}

void NObject::dumpScopes(){
  x_->dumpScopes();
}

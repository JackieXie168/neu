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

#include <neu/NParser.h>

#include <neu/NRegex.h>
#include <neu/NProc.h>
#include <neu/NVSemaphore.h>
#include <neu/NSys.h>
#include <neu/NBasicMutex.h>
#include <neu/NGuard.h>

#include <iostream>

using namespace std;
using namespace neu;

namespace{

  NBasicMutex _mutex;
  
  typedef typename NParser::LexFunc LexFunc;
  
  typedef typename NParser::ReduceFunc ReduceFunc;
  
  typedef NVector<nstr> StringVec;
  
  typedef typename NParser::Type Type;
  
  static const Type Undefined = NParser::Undefined;
  static const Type False = NParser::False;
  static const Type True = NParser::True;
  static const Type Integer = NParser::Integer;
  static const Type Float = NParser::Float;
  static const Type Real = NParser::Real;
  static const Type Symbol = NParser::Symbol;
  static const Type String = NParser::String;
  
  enum CheckResult{
    CheckFine,
    CheckIndirectLeftRec,
    CheckUseless
  };
  
  enum RuleType{
    RuleNormal,
    RulePrime,
    RuleLeftRec
  };
  
} // end namespace

namespace neu{
  
  class NParser_{
  public:
    class Elem{
    public:
      Elem(bool term)
      : term_(term),
      useCount_(0){}
      
      virtual ~Elem(){}
      
      bool isTerm(){
        return term_;
      }
      
      void use(){
        ++useCount_;
      }
      
      size_t useCount(){
        return useCount_;
      }
      
      virtual nstr toStr() = 0;
      
    private:
      bool term_;
      size_t useCount_;
    };
    
    typedef NVector<Elem*> ElemVec;
    
    class Token{
    public:
      Token(Elem* elem, const nvar& value)
      : elem(elem),
      value(value),
      f(0),
      numArgs(0),
      line(0){}
      
      Token(const Token& t)
      : elem(t.elem),
      value(nvar(t.value, nvar::Copy)),
      f(t.f),
      numArgs(t.numArgs),
      line(t.line){}
      
      Token() : f(0), numArgs(0), line(0) {}
      
      Elem* elem;
      nvar value;
      ReduceFunc f;
      uint16_t numArgs;
      uint32_t line;
      
      nstr toStr() const{
        return elem->toStr() + " = " + value;
      }
      
      bool isTerm(){
        return elem->isTerm();
      }
    };
    
    typedef NQueue<Token> Stack;

    static void dumpStack(ostream& estr, const Stack& stack){
      size_t size = stack.size();
      
      for(size_t i = 0; i < size; ++i){
        const Token& t = stack[i];
        estr << i << ": " << t.toStr() << endl;
      }
    }
    
    static void copyStack(const Stack& from, Stack& to){
      size_t size = from.size();
      
      for(size_t i = 0; i < size; ++i){
        const Token& t = from[i];
        
        to.push_back(Token(t));
      }
    }
    
    class Term : public Elem{
    public:
      Term(const nstr& marker, size_t prec, bool left)
      : Elem(true),
      label_(marker),
      regex_(0),
      type_(Undefined),
      prec_(prec),
      pos_(0),
      left_(left),
      length_(marker.length()),
      discard_(marker.empty()){

        f_ =
        [&](nstr& s) -> nvar{
          if(s.beginsWith(label_)){
            s.erase(0, label_.length());
            return label_;
          }
          
          return none;
        };
      }

      Term(const nstr& name, LexFunc f, size_t prec, bool left)
      : Elem(true),
      label_(name),
      f_(f),
      regex_(0),
      type_(Undefined),
      pos_(0),
      prec_(prec),
      left_(left),
      length_(0),
      discard_(name.empty()){}
      
      Term(const nstr& label,
           const nstr& regex,
           Type type,
           size_t pos,
           size_t prec,
           bool left)
      : Elem(true),
      label_(label),
      type_(type),
      pos_(pos),
      prec_(0),
      left_(true),
      length_(0),
      discard_(label.empty()){
       
        switch(type){
          case Undefined:
          case False:
          case True:
          case Integer:
          case Float:
          case Real:
          case Symbol:
          case String:
            break;
          default:
            NERROR("invalid terminal type: " + type);
        }

        nstr nr = "(" + regex + ")[^]*";
        
        regex_ = new NRegex(nr);
        
        f_ =
        [&](nstr& s) -> nvar{
          nvec mv;
          
          if(regex_->match(s, mv)){
            const nstr& wm = mv[1];
            const nstr& m = mv[pos_ + 1];

            nvar ret;
            
            switch(type_){
              case Undefined:
                break;
              case True:
                ret = true;
              case False:
                ret = false;
                break;
              case Integer:
                ret = int64_t(atoll(m.c_str()));
                break;
              case Float:
                ret = atof(m.c_str());
                break;
              case Real:
                ret = nreal(m.c_str());
                break;
              case Symbol:
                ret = nvar(m, nvar::Sym);
                break;
              case String:
                ret = m;
                break;
            }
          
            s.erase(0, wm.length());
            
            return ret;
          }
          
          return none;
        };
      }
      
      nstr toStr(){
        return label_;
      }
      
      size_t length(){
        return length_;
      }
      
      bool match(nstr& buf, Token& t){
        nvar v = f_(buf);
       
        if(v.some()){
          t.elem = this;
          t.value = move(v);
          return true;
        }
        
        return false;
      }
      
      const nstr& label(){
        return label_;
      }
      
      bool isDiscard(){
        return discard_;
      }
      
      bool isLeft(){
        return left_;
      }
      
      size_t prec(){
        return prec_;
      }
      
    private:
      nstr label_;
      LexFunc f_;
      NRegex* regex_;
      Type type_;
      size_t pos_;
      size_t prec_;
      bool left_;
      bool discard_;
      size_t length_;
    };
    
    typedef NMultimap<size_t, Term*> TermPrecMap;
    typedef NHashSet<Term*> TermSet;
    
    class NonTerm;
    class Rule;
    
    class Return{
    public:
      Return* copy(){
        Return* r = new Return;
        r->caller = caller;

        copyStack(stack, r->stack);
        
        return r;
      }
      
      Rule* caller;
      Stack stack;
    };
    
    class Global{
    public:
      Global()
      : code(none),
      activeCount_(0),
      seq(0){}
      
      nvar code;
      nstr file;
      ostream* pstr;
      NProcTask* task;
      size_t seq;
      
      void start(){
        ++activeCount_;
      }
      
      void end(){
        if(--activeCount_ == 0){
          doneSem_.release();
        }
      }

      void await(){
        doneSem_.acquire();
      }
      
      bool await(double dt){
        return doneSem_.acquire(dt);
      }
      
      void done(nvar& c){
        code = move(c);
        doneSem_.release();
      }
      
      void initRemaining(uint32_t remaining){
        remaining_ = remaining;
      }
      
      bool reduced(uint32_t remaining){
        lock();
        bool best = remaining < remaining_;
        if(best){
          remaining_ = remaining;
        }
        unlock();
        
        return best;
      }
      
      void lock(){
        mutex_.lock();
      }

      void unlock(){
        mutex_.unlock();
      }
      
      uint32_t remaining(){
        return remaining_;
      }
      
    private:
      NVSemaphore doneSem_;
      atomic<uint32_t> activeCount_;
      uint32_t remaining_;
      NBasicMutex mutex_;
    };
    
    class State{
    public:
      ~State(){
        for(Return* ret : returnStack_){
          delete ret;
        }
      }
      
      State* copy(){
        State* s = new State;

        copyStack(stack, s->stack);
        
        s->tokens = tokens;
        s->seq = seq;
        s->line = line;
        s->global = global;
        s->priority = priority;

        for(Return* ret : returnStack_){
          s->returnStack_.push_back(ret->copy());
        }
        
        return s;
      }
      
      Return* popReturn(){
        if(returnStack_.empty()){
          return 0;
        }
        
        return returnStack_.popFront();
      }
      
      void pushReturn(Return* ret){
        return returnStack_.push_front(ret);
      }
      
      void dump(ostream& estr){
        estr << "priority: " << priority << endl;
        estr << "caller stack size: " << returnStack_.size() << endl;
        
        estr << "*******************" << endl;
        
        for(Return* r : returnStack_){
          estr << "++++++" << endl;
          estr << r->caller->toStr() << endl;
          dumpStack(estr, r->stack);
          estr << "++++++" << endl;
        }
        
        estr << "******************" << endl;
        
        estr << "-------- stack" << endl;
        dumpStack(estr, stack);

        estr << "-------- tokens" << endl;
        dumpStack(estr, tokens);
      }
      
       size_t depth(){
        return returnStack_.size();
      }
      
      Stack stack;
      Stack tokens;
      size_t seq;
      uint32_t line;
      Global* global;
      double priority;
      
    private:
      typedef NQueue<Return*> ReturnStack_;

      ReturnStack_ returnStack_;
    };
    
    class Task : public NProcTask{
    public:
      Task(size_t threads) : NProcTask(threads){}
      
      void dealloc(nvar& r){
        State* s = r.ptr<State>();
        delete s;
      }
    };
    
    typedef NVector<Rule*> RuleVec;

    typedef NHashSet<Rule*> RuleSet;
    
    typedef NVector<NonTerm*> NonTermVec;
    
    typedef NHashSet<NonTerm*> NonTermSet;
    
    class NonTerm : public Elem{
    public:
      
      NonTerm(NParser_* parser,
              const nstr& baseName,
              NonTerm* baseNonTerm=0,
              size_t precedence=0,
              bool prime=false)
      : Elem(false),
      parser_(parser),
      baseName_(baseName),
      precedence_(precedence),
      prime_(prime),
      computedNullable_(false),
      nullable_(false),
      reachable_(false){
        
        baseNonTerm_ = baseNonTerm ? baseNonTerm : this;
        
      }
      
      void reach(NonTermSet& vm){
        vm.insert(this);
        reachable_ = true;
        
        for(Rule* p : rules_){
          p->reach(vm);
        }
      }
      
      bool canBeginWith(Elem* e, NonTermSet& ns){
        ns.insert(this);
        
        for(Rule* p : rules_){
          if(p->canBeginWith(e, ns)){
            return true;
          }
        }
        
        return false;
      }
      
      bool isPrime(){
        return prime_;
      }
      
      bool inFirst(Term* t){
        return first_.hasKey(t);
      }
      
      bool reachable(){
        return reachable_;
      }
      
      bool nullable(){
        return nullable_;
      }
      
      bool computedNullable(){
        return computedNullable_;
      }
      
      void setNullable(bool flag){
        nullable_ = flag;
      }
      
      void addFollow(Term* t){
        followTerm_.insert(t);
      }
      
      const TermSet& follow(){
        return followTerm_;
      }
      
      void addFollowNonTerm(NonTerm* n){
        followNonTerm_.insert(n);
      }
      
      void addRule(Rule* rule){
        rule->setNonTerm(this);
        rules_.push_back(rule);
      }
      
      void removeRule(Rule* rule){
        auto itr = find(rules_.begin(), rules_.end(), rule);
        assert(itr != rules_.end());
        
        rules_.erase(itr);
      }
      
      RuleVec& rules(){
        return rules_;
      }
      
      const nstr& baseName(){
        return baseName_;
      }
      
      nstr name(){
        nstr n = baseName_;
        
        if(precedence_ > 0){
          n += nvar(precedence_);
        }
        
        if(prime_){
          n += '\'';
        }
        
        return n;
      }
      
      nstr toStr(){
        return name();
      }
      
      void queue(State* state){
        Global* g = state->global;
        NProcTask* task = g->task;
        
        for(Rule* p : rules_){
          nvar r = state->copy();
          state->global->start();
          task->queue(p, r);
        }
      }
      
      void signalAll(Rule* caller,
                     const Stack& returnStack,
                     State* state,
                     double priority){
        
        Global* global = state->global;
        
        bool log = global->pstr;
        
        ostream& pstr = *global->pstr;
        
        if(state->tokens.empty()){
          for(Rule* p : rules_){
            if(p->isEmpty()){
              State* s = state->copy();
              
              Return* ret = new Return;
              ret->caller = caller;
              ret->stack = returnStack;
              
              s->pushReturn(ret);
              
              caller->signal(p, s);
            }
          }
          
          return;
        }
        
        Token& lookahead = state->tokens.front();
        Term* t = static_cast<Term*>(lookahead.elem);
        
        auto p = dispatchMap_.equal_range(t);
        
        double totalScore;
        typedef NHashMap<Rule*, double> MatchMap;
        MatchMap mm;
        
        if(distance(p.first, p.second) > 1){
          totalScore = 0;
          
          for(auto itr = p.first; itr != p.second; ++itr){
            Rule* p = itr->second;
            
            size_t d = 0;
            int m = p->match(state->tokens, d);

            if(log){
              global->lock();
              pstr << "$$$$$$$$$$$$ THE MATCH PROD" << endl;
              pstr << p->toStr() << endl;
              pstr << "m = " << m << endl;
              pstr << "d = " << d << endl;
              global->unlock();
            }
            
            if(m < 0){
              continue;
            }
            
            double score = double(m)/(d == 0 ? 1 : d);
            totalScore += score;

            mm[p] = score;
          }
          
          if(totalScore == 0){
            totalScore = 1;
          }
        }
        else if(p.first != dispatchMap_.end()){
          totalScore = 1.0;
          mm[p.first->second] = 1.0;
        }
        
        if(log && mm.size() > 1){
          global->lock();
          pstr << "^^^^^^^^^ BRANCH!!!!" << endl;
          global->unlock();
        }
        
        for(auto& itr : mm){
          Rule* p = itr.first;
          double score = itr.second;

          double pn = state->priority;
          
          State* s = state->copy();
          
          s->priority = pn + score/totalScore;

          if(log){
            global->lock();
            pstr << "@@@@@@ SIGNAL ALL PROD" << endl;
            pstr << p->toStr() << endl;
            pstr << "with priority: " << s->priority << endl;
            pstr << "#####################" << endl << endl;
            global->unlock();
          }
          
          Return* ret = new Return;
          ret->caller = caller;
          ret->stack = returnStack;
          
          s->pushReturn(ret);
          
          caller->signal(p, s);
        }
      }
      
      void check();
      
      void normAmbig(TermPrecMap& tm,
                     const RuleSet& ps,
                     NonTermVec& nv,
                     size_t prec);
      
      void normLeftRec(NonTermVec& nv);
      
      void dump(ostream& estr){
        for(Rule* p : rules_){
          estr << p->toStr() << endl;
        }
      }
      
      void checkForDuplicates(){
        typedef NSet<ElemVec> Set;
        
        Set s;
        
        for(Rule* p : rules_){
          if(s.hasKey(p->elems())){
            NERROR("duplicate rule: " + p->toStr());
          }
          
          s.insert(p->elems());
        }
      }
      
      bool computeNullable();
      
      bool computeFirst();

      void computeFollow();
      
      void createDispatch(ostream& estr);
      
      bool expandFollow();
      
      bool addFirst(Term* t){
        if(first_.hasKey(t)){
          return false;
        }
        
        first_.insert(t);
        return true;
      }
      
      const TermSet& first(){
        return first_;
      }
      
      void uniteFirst(const TermSet& f){
        first_.unite(f);
      }
      
      void uniteFollow(const TermSet& f){
        followTerm_.unite(f);
      }
      
      void addDispatch(ostream& estr, Term* t, Rule* p){
        dispatchMap_.insert({t, p});
      }
      
      void initAllTerms(){
        for(Rule* p : rules_){
          const ElemVec& elems = p->elems();
          
          for(Elem* ei : elems){
            if(ei->isTerm()){
              Term* t = static_cast<Term*>(ei);
              allTerms_.insert(t);
            }
          }
        }
      }
      
      bool expandAllTerms(){
        bool changed = false;
        
        for(Rule* p : rules_){
          const ElemVec& elems = p->elems();
          
          for(Elem* ei : elems){
            if(!ei->isTerm()){
              NonTerm* n = static_cast<NonTerm*>(ei);
              size_t size = allTerms_.size();
              allTerms_.unite(n->allTerms_);
              
              if(allTerms_.size() > size){
                changed = true;
              }
            }
          }
        }
        
        return changed;
      }
      
      const TermSet& allTerms(){
        return allTerms_;
      }
      
      bool hasTerm(Term* t){
        return allTerms_.hasKey(t);
      }
      
    private:
      typedef NMultimap<Term*, Rule*> DispatchMap_;
      
      NonTerm() : Elem(false){}
      
      NParser_* parser_;
      RuleVec rules_;
      NonTerm* baseNonTerm_;
      nstr baseName_;
      size_t precedence_;
      bool prime_;
      bool nullable_;
      bool computedNullable_;
      bool reachable_;
      TermSet first_;
      TermSet followTerm_;
      TermSet allTerms_;
      NonTermSet followNonTerm_;
      DispatchMap_ dispatchMap_;
    };
    
    class Rule : public NProc{
    public:
      Rule(NParser_* parser,
           const nstr& ruleText,
           size_t pos,
           RuleType ruleType=RuleNormal)
      : nonTerm_(0),
      ruleText_(ruleText),
      parser_(parser),
      ruleType_(ruleType),
      pos_(pos),
      f_(0),
      computedNullable_(false),
      nullable_(false){}
      
      Rule(NParser_* parser,
           const nstr& ruleText,
           ReduceFunc f,
           RuleType ruleType=RuleNormal)
      : nonTerm_(0),
      ruleText_(ruleText),
      parser_(parser),
      ruleType_(ruleType),
      pos_(0),
      f_(f),
      computedNullable_(false),
      nullable_(false){}
      
      int match(const Stack& tokens, size_t& d){
        size_t size = elems_.size();
        size_t n = tokens.size();
        int m = 0;
        
        size_t j = 0;
        
        for(size_t i = 0; i < size; ++i){
          Elem* ei = elems_[i];
       
          if(ei->isTerm()){
            bool matched = false;
            
            while(j < n){
              Term* tj = static_cast<Term*>(tokens[j].elem);
              
              if(!nonTerm_->hasTerm(tj)){
                return -1;
              }
              
              if(tokens[j].elem == ei){
                matched = true;
                ++m;
                ++j;
                break;
              }
              else{
                ++d;
                ++j;
              }
            }

            if(!matched){
              return -1;
            }

            continue;
          }
          
          NonTerm* ni = static_cast<NonTerm*>(ei);

          bool matched = false;
          
          if(ni->isPrime()){

            while(j < n){
              Term* tj = static_cast<Term*>(tokens[j].elem);
              
              if(!ni->hasTerm(tj)){
                return -1;
              }
              
              if(ni->inFirst(tj)){
                matched = true;
                ++j;
                ++m;
                break;
              }
              else{
                ++j;
              }
            }
            
            if(!matched){
              return -1;
            }
            
            continue;
          }
          
          while(j < n){
            Term* tj = static_cast<Term*>(tokens[j].elem);
            
            if(!ni->hasTerm(tj)){
              return -1;
            }

            if(ni->nullable()){
              matched = true;
              ++j;
              break;
            }
            else if(ni->inFirst(tj)){
              matched = true;
              ++m;
              ++j;
              break;
            }
            else{
              ++d;
              ++j;
            }
          }
          
          if(!matched){
            return -1;
          }
          
        }

        return m;
      }
      
      NonTerm* nonTerm(){
        return nonTerm_;
      }
      
      bool isEmpty(){
        return elems_.empty();
      }
      
      const nstr& ruleText(){
        return ruleText_;
      }

      void setType(RuleType type){
        ruleType_ = type;
      }
      
      RuleType type(){
        return ruleType_;
      }
      
      bool nullable(){
        return nullable_;
      }
      
      void setNullable(bool flag){
        nullable_ = flag;
      }

      nstr toStr(){
        nstr s = nonTerm_->name() + " ->";

        for(Elem* ei : elems_){
          s += " ";
          if(ei->isTerm()){
            Term* t = static_cast<Term*>(ei);
            s += t->label();
          }
          else{
            NonTerm* n = static_cast<NonTerm*>(ei);
            s += n->name();
          }
        }
        
        return s;
      }
      
      void setNonTerm(NonTerm* n){
        nonTerm_ = n;
      }
      
      void addElem(Elem* elem){
        elems_.push_back(elem);
      }
      
      void addElems(const ElemVec& ev){
        elems_.append(ev);
      }
      
      void removeFirst(size_t n){
        assert(n <= elems_.size());
        
        auto itr = elems_.begin();
        itr += n;
        elems_.erase(elems_.begin(), itr);
      }
      
      void prefix(size_t n, ElemVec& ev){
        assert(n < elems_.size());
        
        for(size_t i = 0; i < n; ++i){
          ev.push_back(elems_[i]);
        }
      }
      
      bool beginsWith(const ElemVec& ev){
        size_t size = elems_.size();
        
        if(ev.size() > size){
          return false;
        }
        
        for(size_t i = 0; i < size; ++i){
          if(elems_[i] != ev[i]){
            return false;
          }
        }
        
        return true;
      }
      
      const ElemVec& elems(){
        return elems_;
      }
      
      size_t numElems(){
        return elems_.size();
      }
      
      void reach(NonTermSet& vm){
        for(Elem* ei : elems_){
          if(ei->isTerm()){
            continue;
          }
          
          NonTerm* n = static_cast<NonTerm*>(ei);
          
          if(vm.hasKey(n)){
            continue;
          }
          
          n->reach(vm);
        }
      }
      
      bool canBeginWith(Elem* e, NonTermSet& ns){
        if(elems_.empty()){
          return false;
        }
        
        Elem* fe = elems_[0];
        
        if(fe == e){
          return true;
        }
        
        if(fe->isTerm()){
          return false;
        }
        
        NonTerm* fn = static_cast<NonTerm*>(fe);
        
        if(ns.hasKey(fn)){
          return false;
        }
        
        return fn->canBeginWith(e, ns);
      }
      
      bool normAmbig(Term* t, NonTerm* n){
        if(elems_.size() != 3){
          return false;
        }
        
        if(elems_[1] == t){
          if(t->isLeft()){
            elems_[2] = n;
          }
          else{
            elems_[0] = n;
          }
          
          return true;
        }
        
        return false;
      }
      
      void replace(Elem* f, Elem* r){
        size_t size = elems_.size();
        
        for(size_t i = 0; i < size; ++i){
          if(elems_[i] == f){
            elems_[i] = r;
          }
        }
      }
      
      CheckResult check(){
        size_t size = elems_.size();
        
        if(size == 0){
          return CheckFine;
        }
        
        Elem* first = elems_[0];
        
        if(first->isTerm()){
          return CheckFine;
        }
        
        if(first == nonTerm_){
          return size == 1 ? CheckUseless : CheckFine;
        }
        
        NonTermSet ns;
        return canBeginWith(nonTerm_, ns) ? CheckIndirectLeftRec : CheckFine;
      }
      
      Rule* normLeftRec(NonTerm* n){
        size_t size = elems_.size();
        
        if(size == 0){
          return 0;
        }
        
        if(elems_[0] != nonTerm_){
          return 0;
        }
        
        Rule* p = new Rule(parser_, "", f_, RulePrime);
        f_ = 0;
        pos_ = 1;
        
        for(size_t i = 1; i < size; ++i){
          p->addElem(elems_[i]);
        }
        
        p->addElem(n);
        
        Elem* r = elems_.back();
        elems_.clear();
        elems_.push_back(r);
        elems_.push_back(n);
        
        ruleType_ = RuleLeftRec;
        
        return p;
      }
      
      bool handle(nvar& v, nvar& r){
        r = move(v);
        return true;
      }
      
      bool reduced(State* s){
        return s->global->reduced(s->tokens.size());
      }
      
      void end(State* s){
        s->global->end();
      }
      
      void signal(Rule* r, State* s){
        Global* g = s->global;
        NProcTask* task = g->task;
        
        nvar rs = s;
        g->start();
        NProc::signal(task, r, rs, s->priority);
      }

      void endSignal(Rule* r, State* s){
        Global* g = s->global;
        NProcTask* task = g->task;
        
        nvar rs = s;
        NProc::signal(task, r, rs, s->priority);
      }
      
      void run(nvar& r){
        State* state = r.ptr<State>();
        Global* global = state->global;
        
        bool log = global->pstr;
        
        ostream& pstr = *global->pstr;
        
        if(log){
          global->lock();
          pstr << endl << endl;
          pstr << "=========================" << endl;
          pstr << "running: " << toStr() << endl;
          pstr << "seq: " << state->seq << endl;
          state->dump(pstr);
          global->unlock();
        }

        size_t numElems = elems_.size();
        
        if(numElems == 0){
          Return* ret = state->popReturn();
          assert(ret);
          
          Rule* caller = ret->caller;
          
          size_t seq = ++global->seq;

          if(log){
            global->lock();
            pstr << "NULL RETURN WITH SEQ: " << seq << endl;
            global->unlock();
          }
          
          state->seq = seq;
          state->stack = move(ret->stack);

          Token t(nonTerm_, none);
          state->stack.emplace_back(t);

          delete ret;
          endSignal(caller, state);
          return;
        }
        
        Stack& stack = state->stack;
        Stack& tokens = state->tokens;

        size_t stackSize = stack.size();
        
        Elem* ei;

        for(size_t i = 0; i < numElems; ++i){
          ei = elems_[i];

          if(i < stackSize){
            if(ei != stack[i].elem){
              if(ei->isTerm()){
                end(state);
                delete state;
                return;
              }
              else{
                NonTerm* n = static_cast<NonTerm*>(ei);
                
                Token rt;
                
                if(ruleType_ == RuleLeftRec){
                  while(stack.size() > 1){
                    Token left = move(stack.popFront());
                    Token op = move(stack.popFront());
                    
                    nvec rv = {left.value, op.value};
                    
                    size_t rightArgs = op.numArgs - 2;
                    
                    for(size_t i = 0; i < rightArgs; ++i){
                      rt = move(stack.popFront());
                      
                      rv.emplace_back(move(rt.value));
                    }
                    
                    assert(op.f);
                    
                    rt.elem = nonTerm_;
                    rt.value = move(op.f(rv));
                    
                    if(state->line > 0 && rt.value.isSymbolic()){
                      rt.value.setFile(global->file);
                      rt.value.setLine(state->line);
                    }
                    
                    stack.emplace_front(move(rt));
                  }
                 
                  rt = move(stack.front());
                  
                  Return* ret = state->popReturn();
                  assert(ret);

                  size_t seq = ++global->seq;

                  if(log){
                    global->lock();
                    pstr << "RETURN LEFT REC WITH SEQ: " << seq << endl;
                    global->unlock();
                  }

                  state->seq = seq;
                  state->stack = move(ret->stack);
                  state->stack.emplace_back(move(rt));
                  
                  delete ret;

                  if(reduced(state)){
                    ++state->priority;
                  }
                  
                  endSignal(ret->caller, state);
                  return;
                }
                else if(ruleType_ == RulePrime && n == nonTerm_){
                  break;
                }
                else{
                  end(state);
                  delete state;
                  return;
                }
              }
            }
          }
          else if(ei->isTerm()){
            if(tokens.empty()){
              end(state);
              delete state;
              return;
            }
            
            Token t = move(tokens.popFront());
            
            if(t.elem != ei){
              end(state);
              delete state;
              return;
            }

            if(state->line > 0){
              state->line = t.line;
            }
            
            stack.emplace_back(move(t));
          }
          else{
            NonTerm* n = static_cast<NonTerm*>(ei);

            size_t seq = ++global->seq;
          
            if(log){
              global->lock();
              pstr << "----- SIGNAL ALL ON: " << n->name() << " at pos: " <<
              i << " with seq: " << seq << endl;
              global->unlock();
            }
          
            Stack returnStack = move(stack);
            
            state->stack.clear();
            state->seq = seq;
            
            n->signalAll(this, returnStack, state, state->priority);
            
            global->end();
            delete state;
            return;
          }
        }
        
        size_t pos = pos_;
        
        if(!stack.empty()){
          const Token& b = stack.back();
          
          if(!b.value.some()){
            stack.pop_back();
            if(pos >= stack.size()){
              --pos;
            }
          }
        }

        Return* ret = state->popReturn();
        
        nvec rv;
        
        if(ret && ruleType_ == RulePrime){
          stack[0].f = f_;
          stack[0].numArgs = numElems;
          
          Rule* caller = ret->caller;
          
          size_t seq = ++global->seq;
          
          if(log){
            global->lock();
            pstr << "RULE RETURN WITH SEQ: " << seq << endl;
            global->unlock();
          }

          state->seq = seq;

          Stack currStack = move(stack);
          state->stack = move(ret->stack);
          state->stack.append(currStack);
          
          if(reduced(state)){
            ++state->priority;
          }
          
          delete ret;
          endSignal(caller, state);
          return;
        }
        else{
          for(size_t i = 0; i < numElems; ++i){
            Token& ti = stack[i];
            
            if(ti.isTerm()){
              rv << move(ti.value);
            }
            else{
              rv << move(ti.value);
            }
          }
        }
        
        Token t;
        t.elem = nonTerm_;
        
        if(f_){
          t.value = move(f_(rv));
        }
        else{
          t.value = move(rv[pos]);
        }

        if(reduced(state)){
          ++state->priority;
        }
        
        if(state->line > 0 && t.value.isSymbolic()){
          t.value.setFile(global->file);
          t.value.setLine(state->line);
        }
        
        if(log){
          global->lock();
          pstr << "REDUCE ==================" << endl;
          pstr << t.toStr() << endl;
          global->unlock();
        }
        
        if(ret){
          Rule* caller = ret->caller;
          
          size_t seq = ++global->seq;
          
          if(log){
            global->lock();
            pstr << "RETURN WITH SEQ: " << seq << endl;
            global->unlock();
          }

          state->seq = seq;
          state->stack = move(ret->stack);
          state->stack.emplace_back(move(t));

          delete ret;
          endSignal(caller, state);
          return;
        }

        if(tokens.empty()){
          reduced(state);
          global->done(t.value);
        }
        
        end(state);
        delete state;
      }
    
      nstr fullText();
      
      bool computeNullable(){
        if(computedNullable_){
          return false;
        }
        
        bool known = true;
        
        for(Elem* ei : elems_){
          if(ei->isTerm()){
            nullable_ = false;
            computedNullable_ = true;
            return true;
          }
          else{
            NonTerm* ni = static_cast<NonTerm*>(ei);

            if(ni->computedNullable()){
              if(!ni->nullable()){
                nullable_ = false;
                computedNullable_ = true;
                return true;
              }
            }
            else{
              known = false;
            }
          }
        }

        if(known){
          nullable_ = true;
          computedNullable_ = true;
          
          return true;
        }

        return false;
      }
      
      bool computeFirst(){
        bool changed = false;
        
        for(Elem* ei : elems_){
          if(ei->isTerm()){
            Term* ti = static_cast<Term*>(ei);
            if(!first_.hasKey(ti)){
              first_.insert(ti);
              nonTerm_->addFirst(ti);
              changed = true;
            }
            break;
          }
          else{
            NonTerm* ni = static_cast<NonTerm*>(ei);
            const TermSet& f = ni->first();
            size_t size = first_.size();
            first_.unite(f);

            if(first_.size() > size){
              nonTerm_->uniteFirst(f);
              changed = true;
            }

            if(!ni->nullable()){
              break;
            }
          }
        }
        
        return changed;
      }
      
      void computeFollow(){
        size_t size = elems_.size();
        
        for(size_t i = 0; i < size; ++i){
          Elem* ei = elems_[i];
          
          if(ei->isTerm()){
            continue;
          }
          
          NonTerm* n = static_cast<NonTerm*>(ei);
          
          bool end = true;
          
          for(size_t j = i + 1; j < size; ++j){
            Elem* ej = elems_[j];
            
            if(ej->isTerm()){
              Term* tj = static_cast<Term*>(ej);
              n->addFollow(tj);
              end = false;
              break;
            }
            else{
              NonTerm* nj = static_cast<NonTerm*>(ej);
              n->uniteFollow(nj->first());
              
              if(!nj->nullable()){
                end = false;
                break;
              }
            }
          }
          
          if(end && n != nonTerm_){
            n->addFollowNonTerm(nonTerm_);
          }
        }
      }
      
      bool inFirst(Term* t){
        return first_.hasKey(t);
      }
      
    private:
      Rule(){}
      
      NonTerm* nonTerm_;
      size_t pos_;
      ReduceFunc f_;
      nstr ruleText_;
      ElemVec elems_;
      class NParser_* parser_;
      RuleType ruleType_;
      bool nullable_;
      bool computedNullable_;
      TermSet first_;
    };
    
    NParser_(NParser* o)
    : o_(o),
    initialized_(false){}
    
    ~NParser_(){}
    
    void init_(){
      NBasicGuard guard(mutex_);
      
      if(initialized_){
        return;
      }
      
      startNonTerm_ = 0;

      o_->init();

      TermVec_ ts;
      auto itr = terms_.begin();
      while(itr != terms_.end()){
        Term* t = *itr;
        if(t->length() == 0){
          ts.push_back(t);
          itr = terms_.erase(itr);
        }
        else{
          ++itr;
        }
      }
      
      sort(terms_.begin(), terms_.end(), [](Term* a, Term* b){
        return a->length() > b->length();
      });
      
      terms_.append(ts);
      
      for(auto& itr : nonTermMap_){
        NonTerm* n = itr.second;
        
        RuleVec& pv = n->rules();
        
        for(Rule* p : pv){
          nstr rt = p->ruleText();
          StringVec sv;
          rt.splitOnSpace(sv);

          for(const nstr& si : sv){
            auto itr = termMap_.find(si);
            if(itr != termMap_.end()){
              Term* rt = itr->second;
              rt->use();
              p->addElem(rt);
            }
            else{
              auto nitr = nonTermMap_.find(si);
              if(nitr == nonTermMap_.end()){
                NERROR("unknown element: '" + si + "' in: " +
                       p->fullText());
              }
              
              NonTerm* rn = nitr->second;
              
              if(rn == startNonTerm_){
                NERROR("start non-terminal used in: " + p->fullText());
              }
              
              rn->use();
              p->addElem(rn);
              
            }
          }
        }
      }
      
      for(auto& itr : termMap_){
        Term* t = itr.second;
        
        if(t->useCount() == 0){
          NERROR("unused terminal: " + t->label());
        }
      }

      for(auto& itr : nonTermMap_){
        NonTerm* n = itr.second;
        
        if(n->useCount() == 0){
          NERROR("unused non-terminal: " + n->name());
        }
      }

      for(auto& itr : nonTermMap_){
        NonTerm* n = itr.second;
        
        n->checkForDuplicates();
      }
      
      NonTermSet vm;
      startNonTerm_->reach(vm);
      
      for(auto& itr : nonTermMap_){
        NonTerm* n = itr.second;
        if(!n->reachable()){
          NERROR("unreachable non-terminal: " + n->name());
        }
      }
      
      check();
      
      normAmbig();
      normLeftRec();
      
      computeNullable();
      computeFirst();
      computeFollow();
      createDispatch(pstr_);

      findTerms();
      
      dumpGrammar(pstr_);

      initialized_ = true;
    }
    
    void alphabet(TermSet& s){
      for(auto& itr : termMap_){
        Term* t = itr.second;
        s.insert(t);
      }
    }
    
    void handleNewLines(const nstr& str, size_t& line){
      size_t len = str.length();
      
      for(size_t j = 0; j < len; ++j){
        switch(str[j]){
          case '\n':
            ++line;
            break;
          default:
            return;
        }
      }
    }
    
    void lex(const nstr& file,
             const nstr& str,
             Stack& tokens){
      nstr s = str;
      
      size_t n = terms_.size();
      size_t dn = discardTerms_.size();

      bool done = false;
      
      size_t line = 1;

      Token dt;
      
      for(;;){
        for(size_t i = 0; i < dn; ++i){
          handleNewLines(s, line);
          
          Term* term = discardTerms_[i];
          
          if(term->match(s, dt)){
            break;
          }

          if(s.empty()){
            done = true;
          }
        }
        
        if(done){
          break;
        }
        
        handleNewLines(s, line);

        bool matched = false;
        
        Token t;
        
        for(size_t i = 0; i < n; ++i){
          Term* term = terms_[i];
          
          if(term->match(s, t)){
            matched = true;
            break;
          }
        }

        if(matched){
          t.line = line;
          tokens.emplace_back(move(t));
        }
        else{
          if(s.empty()){
            break;
          }
          
          NERROR("lexer failed at: '" + s + "'");
        }
      }
    }
    
    void check(){
      for(auto& itr : nonTermMap_){
        NonTerm* n = itr.second;
        n->check();
      }
    }
    
    void normAmbig(){
      NonTermVec nv;
      
      TermSet lts;
      auto itr = termPrecMap_.begin();
      assert(itr != termPrecMap_.end());
      
      for(auto& itr : nonTermMap_){
        NonTerm* n = itr.second;

        RuleSet ps;
        n->normAmbig(termPrecMap_, ps, nv, 0);
      }
      
      for(NonTerm* ni : nv){
        nonTermMap_.insert({ni->name(), ni});
      }
    }
    
    void normLeftRec(){
      NonTermVec nv;
      
      for(auto& itr : nonTermMap_){
        NonTerm* n = itr.second;
        n->normLeftRec(nv);
      }
      
      for(NonTerm* ni : nv){
        nonTermMap_.insert({ni->name(), ni});
      }
    }

    void findTerms(){
      for(auto& itr : nonTermMap_){
        NonTerm* n = itr.second;
        n->initAllTerms();
      }
      
      for(;;){
        bool changed = false;
        
        for(auto& itr : nonTermMap_){
          NonTerm* n = itr.second;
          if(n->expandAllTerms()){
            changed = true;
          }
        }
        
        if(!changed){
          break;
        }
      }
    }
    
    void error(ostream& estr,
               const Stack& tokens,
               Global* g,
               const nstr& msg){
      size_t remaining = g->remaining();

      size_t dist = tokens.size() - remaining;
      
      size_t line = 1;
      
      for(size_t i = 0; i < dist; ++i){
        line = tokens[i].line;
      }
      
      const nstr& file = g->file;
      
      if(file.empty()){
        estr << "syntax error at line: " << line << endl;
      }
      else{
        estr << "syntax error at " << file << ":" << line << endl;
      }
    }
    
    nvar parse(const nstr& str,
               const nstr& file,
               bool metadata,
               double timeout,
               ostream* errorLog,
               ostream* parseLog){
      init_();

      Task task(1);
      
      Global global;
      global.task = &task;
      global.file = file;
      global.pstr = parseLog;
      
      State state;
      state.priority = 1;
      state.global = &global;
      state.line = metadata ? 1 : 0;
      state.seq = 0;
      
      nstr s = str;
      
      Stack tokens;
      lex(file, s, tokens);
      state.tokens = tokens;
      
      global.initRemaining(state.tokens.size());
      
      if(parseLog){
        *parseLog << "------- tokens" << endl;
        dumpStack(*parseLog, state.tokens);
        *parseLog << endl;
      }

      startNonTerm_->queue(&state);
      
      if(!global.await(timeout)){
        if(errorLog){
          error(*errorLog, tokens, &global, "time out");
        }
      }
      else if(!global.code.some() && errorLog){
        error(*errorLog, tokens, &global, "parse error");
      }

      task.shutdown();
      
      return global.code;
    }
    
    void addTerm(const nstr& marker, size_t prec, bool left){
      auto itr = termMap_.find(marker);
      if(itr != termMap_.end()){
        NERROR("terminal exists: " + marker);
      }
      
      auto nitr = nonTermMap_.find(marker);
      if(nitr != nonTermMap_.end()){
        NERROR("non-terminal exists: " + marker);
      }
      
      Term* t = new Term(marker, prec, left);
      termMap_[marker] = t;
      termPrecMap_.insert({prec, t});
      terms_.push_back(t);
    }

    void addTerm(const nstr& name, LexFunc f, size_t prec, bool left){
      auto itr = termMap_.find(name);
      if(itr != termMap_.end()){
        NERROR("terminal exists: " + name);
      }
      
      auto nitr = nonTermMap_.find(name);
      if(nitr != nonTermMap_.end()){
        NERROR("non-terminal exists: " + name);
      }
      
      Term* t = new Term(name, f, prec, left);
      termMap_[name] = t;
      termPrecMap_.insert({prec, t});
      terms_.push_back(t);
    }
    
    void addTerm(const nstr& name,
                 const nstr& regex,
                 Type type,
                 size_t pos,
                 size_t prec,
                 bool left){
      auto itr = termMap_.find(name);
      if(itr != termMap_.end()){
        NERROR("terminal exists: " + name);
      }
      
      auto nitr = nonTermMap_.find(name);
      if(nitr != nonTermMap_.end()){
        NERROR("non-terminal exists: " + name);
      }
      
      Term* t = new Term(name, regex, type, pos, prec, left);

      if(t->isDiscard()){
        discardTerms_.push_back(t);
      }
      else{
        termMap_[name] = t;
        termPrecMap_.insert({prec, t});
        terms_.push_back(t);
      }
    }
        
    void addRule(const nstr& name, const nstr& rule, size_t pos){
      auto itr = nonTermMap_.find(name);
      
      NonTerm* n;
      
      if(itr == nonTermMap_.end()){
        auto itr = termMap_.find(name);
        if(itr != termMap_.end()){
          NERROR("terminal exists: " + name);
        }
        
        n = new NonTerm(this, name);
        
        if(!startNonTerm_){
          startNonTerm_ = n;
          startNonTerm_->use();
        }
        
        nonTermMap_[name] = n;
      }
      else{
        n = itr->second;
      }
      
      Rule* p = new Rule(this, rule, pos);
      
      n->addRule(p);
    }
    
    void addRule(const nstr& name, const nstr& rule, NParser::ReduceFunc f){
      auto itr = nonTermMap_.find(name);
      
      NonTerm* n;
      
      if(itr == nonTermMap_.end()){
        auto itr = termMap_.find(name);
        if(itr != termMap_.end()){
          NERROR("terminal exists: " + name);
        }
        
        n = new NonTerm(this, name);
        
        if(!startNonTerm_){
          startNonTerm_ = n;
          startNonTerm_->use();
        }
        
        nonTermMap_[name] = n;
      }
      else{
        n = itr->second;
      }
      
      Rule* p = new Rule(this, rule, f);
      
      n->addRule(p);
    }
    
    void dumpGrammar(ostream& estr){
      startNonTerm_->dump(estr);
      estr << endl;
      
      for(auto& itr : nonTermMap_){
        NonTerm* n = itr.second;
        if(n != startNonTerm_){
          n->dump(estr);
          estr << endl;
        }
      }
      
      for(auto& itr : nonTermMap_){
        NonTerm* n = itr.second;
        estr << "----------------" << endl;
        estr << n->toStr() << endl;
        
        estr << "NULLABLE: " << n->nullable() << endl;

        estr << "FIRST: ";
        const TermSet& f = n->first();
        bool first = true;
        for(Term* t : f){
          if(first){
            first = false;
          }
          else{
            estr << ", ";
          }
          estr << t->toStr();
        }

        estr << endl;
        
        estr << "FOLLOW: ";
        const TermSet& f2 = n->follow();
        first = true;
        for(Term* t : f2){
          if(first){
            first = false;
          }
          else{
            estr << ", ";
          }
          estr << t->toStr();
        }

        estr << endl;
        
        estr << "ALL TERMS: ";
        const TermSet& a = n->allTerms();
        first = true;
        for(Term* t : a){
          if(first){
            first = false;
          }
          else{
            estr << ", ";
          }
          estr << t->toStr();
        }
        
        estr << endl;
      }
    }
    
    void computeNullable(){
      for(;;){
        bool changed = false;
        
        for(auto& itr : nonTermMap_){
          NonTerm* n = itr.second;
          
          if(n->computeNullable()){
            changed = true;
          }
        }
        
        if(!changed){
          break;
        }
      }
    }
    
    void computeFirst(){
      for(;;){
        bool changed = false;
        
        for(auto& itr : nonTermMap_){
          NonTerm* n = itr.second;
          
          if(n->computeFirst()){
            changed = true;
          }
        }
        
        if(!changed){
          break;
        }
      }
    }
    
    void computeFollow(){
      for(auto& itr : nonTermMap_){
        NonTerm* n = itr.second;
        n->computeFollow();
      }
      
      for(;;){
        bool changed = false;

        for(auto& itr : nonTermMap_){
          NonTerm* n = itr.second;
          if(n->expandFollow()){
            changed = true;
          }
        }
        
        if(!changed){
          break;
        }
      }
    }
    
    void createDispatch(ostream& estr){
      for(auto& itr : nonTermMap_){
        NonTerm* n = itr.second;
        n->createDispatch(estr);
      }
    }
    
    void getInitLog(nstr& log){
      init_();
      
      log = pstr_.str();
    }
    
  private:
    typedef NMap<nstr, Term*> TermMap_;
    typedef NVector<Term*> TermVec_;
    typedef NMap<nstr, NonTerm*> NonTermMap_;

    NParser* o_;
    NBasicMutex mutex_;
    TermMap_ termMap_;
    TermPrecMap termPrecMap_;
    TermVec_ terms_;
    TermVec_ discardTerms_;
    NonTermMap_ nonTermMap_;
    NonTerm* startNonTerm_;
    bool initialized_;
    stringstream pstr_;
  };

} // end namespace neu

nstr NParser_::Rule::fullText(){
  return nonTerm_->name() + " -> " + ruleText_;
}

void NParser_::NonTerm::check(){
  for(Rule* p : rules_){
    CheckResult res = p->check();
    
    switch(res){
      case CheckFine:
        break;
      case CheckIndirectLeftRec:
        NERROR("rule uses indirect left recursion: " + p->toStr());
      case CheckUseless:
        NERROR("useless rule: " + p->toStr());
    }
  }
}

void NParser_::NonTerm::normAmbig(TermPrecMap& tm,
                                  const RuleSet& ps,
                                  NonTermVec& nv,
                                  size_t prec){
  TermSet ts;
  auto itr = tm.upper_bound(prec);
  if(itr != tm.end()){
    prec = itr->first;
    while(itr != tm.end() && itr->first == prec){
      ts.insert(itr->second);
      ++itr;
    }
  }
  else{
    for(Rule* p : ps){
      p->replace(this, baseNonTerm_);
    }
    return;
  }

  NonTerm* n = new NonTerm(parser_, baseName_, baseNonTerm_, prec);
  RuleSet nps;
  
  bool match = false;
  
  for(Term* t : ts){
    for(auto& itr : rules_){
      Rule* p = itr;
      
      if(p->normAmbig(t, n)){
        nps.insert(p);
        match = true;
        break;
      }
    }
  }

  RuleSet lps;
  
  if(!match){
    delete n;
    for(Rule* r : rules_){
      lps.insert(r);
    }
    normAmbig(tm, lps, nv, prec);
    return;
  }

  auto pitr = rules_.begin();
  while(pitr != rules_.end()){
    Rule* p = *pitr;
    
    if(nps.hasKey(p)){
      ++pitr;
    }
    else{
      p->replace(this, n);
      n->addRule(p);
      pitr = rules_.erase(pitr);
      lps.insert(p);
    }
  }
  
  Rule* p = new Rule(parser_, "", 0);
  p->addElem(n);
  addRule(p);
  nv.push_back(n);
  
  n->normAmbig(tm, lps, nv, prec);
}

void NParser_::NonTerm::normLeftRec(NonTermVec& nv){
  NonTerm* n = new NonTerm(parser_, baseName_, baseNonTerm_, precedence_, true);
  
  bool match = false;
  auto itr = rules_.begin();
  
  while(itr != rules_.end()){
    Rule* p = *itr;
    
    Rule* pn = p->normLeftRec(n);
    
    if(pn){
      n->addRule(pn);
      
      if(match){
        itr = rules_.erase(itr);
      }
      else{
        Rule* pe = new Rule(parser_, "", 0, RulePrime);
        n->addRule(pe);
        match = true;
        ++itr;
      }
    }
    else{
      ++itr;
    }
  }
  
  if(match){
    nv.push_back(n);
  }
  else{
    delete n;
  }
}

bool NParser_::NonTerm::computeNullable(){
  if(computedNullable_){
    return false;
  }

  bool known = true;
  for(Rule* p : rules_){
    if(p->computeNullable()){
      if(p->nullable()){
        nullable_ = true;
        computedNullable_ = true;
        return true;
      }
    }
    else{
      known = false;
    }
  }
  
  if(known){
    nullable_ = false;
    computedNullable_ = true;
    return true;
  }
  
  return false;
}

bool NParser_::NonTerm::computeFirst(){
  bool changed = false;
  
  for(Rule* p : rules_){
    if(p->computeFirst()){
      changed = true;
    }
  }
  
  return changed;
}

void NParser_::NonTerm::computeFollow(){
  for(Rule* p : rules_){
    p->computeFollow();
  }
}

bool NParser_::NonTerm::expandFollow(){
  size_t size = followTerm_.size();
  
  for(NonTerm* n : followNonTerm_){
    followTerm_.unite(n->followTerm_);
  }
  
  return followTerm_.size() > size;
}

void NParser_::NonTerm::createDispatch(ostream& estr){
  TermSet alphabet;
  parser_->alphabet(alphabet);
  
  for(Term* t : alphabet){
    Rule* rule = 0;
    
    for(Rule* p : rules_){
      if(p->inFirst(t)){
        rule = p;
        addDispatch(estr, t, p);
      }
    }
    
    if(rule){
      continue;
    }
    
    for(Rule* p : rules_){
      if(p->nullable() && followTerm_.hasKey(t)){
        rule = p;
        addDispatch(estr, t, p);
      }
    }
  }
}

namespace{
  
  typedef NHashMap<string, NParser_*> _ParserMap;
  
  _ParserMap _parserMap;
  
} // end namespace

NParser::NParser(const nstr& name){
  _mutex.lock();
  NParser_* p;
  auto itr = _parserMap.find(name);
  if(itr == _parserMap.end()){
    p = new NParser_(this);
  }
  else{
    p = itr->second;
  }
  _mutex.unlock();
  x_ = p;
}

NParser::~NParser(){
  delete x_;
}

nvar NParser::parse(const nstr& str,
                    bool metadata,
                    const nstr& sourceName,
                    double timeout,
                    ostream* errorLog,
                    ostream* parseLog){
  return x_->parse(str, sourceName, metadata, timeout, errorLog, parseLog);
}

void NParser::addTerm(const nstr& marker, size_t prec, bool left){
  x_->addTerm(marker, prec, left);
}

void NParser::addTerm(const nstr& name,
                      const nstr& regex,
                      Type type,
                      size_t pos,
                      size_t prec,
                      bool left){
  x_->addTerm(name, regex, type, pos, prec, left);
}

void NParser::addTerm(const nstr& name, LexFunc f, size_t prec, bool left){
  x_->addTerm(name, f, prec, left);
}

void NParser::addRule(const nstr& name, const nstr& rule, size_t pos){
  x_->addRule(name, rule, pos);
}

void NParser::addRule(const nstr& name,
                      const nstr& rule,
                      ReduceFunc f){
  x_->addRule(name, rule, f);
}

void NParser::getInitLog(nstr& log){
  x_->getInitLog(log);
}

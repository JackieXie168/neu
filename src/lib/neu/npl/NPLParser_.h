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

#ifndef NEU_NPL_PARSER__H
#define NEU_NPL_PARSER__H

#include <unistd.h>
#include <cstring>

#include <neu/NPLParser.h>
#include <neu/NSys.h>
#include <neu/NProgram.h>

#include "parse.h"

using namespace std;
using namespace neu;

namespace neu{
  
  class NPLParser_{
  public:
    
    NPLParser_(NPLParser* o)
    : o_(o),
    estr_(&cerr){
      
    }
    
    ~NPLParser_(){
      
    }
    
    void advance(const char* text, const nstr& tag=""){
      advance(strlen(text), tag);
    }
    
    void advance(size_t count, const nstr& tag=""){
      char_ += count;
      
      if(tags_ && !tag.empty()){
        nvar t;
        t("tag") = tag;
        t("start") = lastChar_;
        t("end") = char_;
        t("line") = line_;
        tags_->pushBack(move(t));
      }
      
      lastChar_ = char_;
    }
    
    nvar token(const char* text, const nstr& tag=""){
      char_ += strlen(text);
      
      nvar token = text;
      token("start") = lastChar_;
      token("end") = char_;
      token("line") = line_;
      
      if(!tag.empty()){
        if(tags_){
          tags_->pushBack(nvar());
          nvar& t = tags_->back();
          t("tag") = tag;
          t("start") = lastChar_;
          t("end") = char_;
          t("line") = line_;
        }
      }
      
      lastChar_ = char_;
      
      return token;
    }
    
    nvar parse(const nstr& code, nvar* tags){
      out_ = undef;
      
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = "";
      
      npl_lex_init(&scanner_);
      npl_set_extra(this, scanner_);
      
      FILE* file = tmpfile();
      fwrite(code.c_str(), 1, code.length(), file);
      rewind(file);
      npl_set_in(file, scanner_);
      npl_parse(this, scanner_);
      fclose(file);
            
      npl_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      return out_;
    }
    
    nvar parseFile(const nstr& path, nvar* tags){
      out_ = undef;
      
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = NSys::basename(path);
      
      npl_lex_init(&scanner_);
      npl_set_extra(this, scanner_);
      
      FILE* file = fopen(path.c_str(), "r");
      
      if(!file){
        NERROR("failed to open: " + path);
      }
      
      npl_set_in(file, scanner_);
      npl_parse(this, scanner_);
      fclose(file);
      
      npl_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      return out_;
    }
    
    nvar newClass(){
      nvar c;
      c("__offset") = 0;
      c("__index") = 0;
      
      return c;
    }
    
    void addExtern(const nvar& f){
      nvar k = {f[1].sym(), f[1].size()};
      
      if(out_.hasKey(k)){
        error("extern exists: " + k);
        return;
      }
      
      out_(k) = move(f);
    }
    
    void addClass(const nstr& c, nvar& cv){
      if(out_.hasKey(c)){
        error("class exists: " + c);
        return;
      }
      
      out_(c) = move(cv);
    }
    
    void addMethod(nvar& c, nvar& f){
      nvar k = {f[1].sym(), f[1].size()};
      
      if(c.hasKey(k)){
        error("method exists: " + k);
        return;
      }
      
      c(k) = move(f);
    }
    
    void addAttribute(nvar& c, nvar& a){
      const nstr& k = a.str();
      
      if(c.hasKey(k)){
        error("attribute exists: " + k);
        return;
      }

      size_t offset = c["__offset"];
      size_t index = c["__index"];
      
      size_t bytes;
      if(a.get("ptr", false)){
        bytes = 8;
      }
      else{
        size_t bits = a["bits"];
        bytes = bits/8 + (bits % 8 > 0 ? 1 : 0);
      }

      size_t len = a.get("len", 0);
      if(len > 0){
        bytes *= len;
        while(offset % 16 != 0){
          ++offset;
        }
      }
      
      a("offset") = offset;
      a("index") = index;
      
      offset += bytes;
      ++index;
      
      c["__offset"] = offset;
      c["__index"] = index;
      
      c(k) = move(a);
    }
    
    void define(const nstr& sym, const nvar& expr){
      defineMap_(sym) = expr;
    }
    
    void setErrorStream(ostream& estr){
      estr_ = &estr;
    }
    
    void emit(nvar& n){
      if(n.some()){
        out_ << move(n);
      }
    }
    
    nvar error(const nvar& n, const nstr& msg, bool warn=false){
      status_ = 1;
      
      ostream& estr = *estr_;
      
      estr << "NPLParser error: " << n.getLocation() << ": " << msg << endl;
      
      return sym("Error");
    }
    
    void error(const nstr& type){
      status_ = 1;
      *estr_ << "NPLParser error: " << getLocation() << ": " << type << endl;
    }
    
    nstr getLocation(){
      nstr loc;
      
      if(!file_.empty()){
        loc += file_ + ":";
      }
      
      loc += nvar(line_);
      
      return loc;
    }
    
    nvar func(const nstr& f){
      nvar v = nfunc(f);
      v.setLine(line_);
      
      if(!file_.empty()){
        v.setFile(file_);
      }
      
      return v;
    }
    
    nvar func(const char* f){
      nvar v = nfunc(f);
      v.setLine(line_);
      
      if(!file_.empty()){
        v.setFile(file_);
      }
      
      return v;
    }
    
    nvar func(const nvar& v){
      return func(v.str());
    }
    
    nvar sym(const nstr& s){
      nvar v;
      
      if(defineMap_.hasKey(s)){
        v = defineMap_[s];
      }
      else{
        v = nsym(s);
      }

      v.setLine(line_);
      
      if(!file_.empty()){
        v.setFile(file_);
      }
      
      return v;
    }
    
    nvar sym(const nvar& v){
      return sym(v.str());
    }
    
    nvar sym(const char* s){
      nvar v;
      
      if(defineMap_.hasKey(s)){
        v = defineMap_[s];
      }
      else{
        v = nsym(s);
      }
      
      v.setLine(line_);
      
      if(!file_.empty()){
        v.setFile(file_);
      }
      
      return v;
    }
    
    nvar var(const nvar& v){
      return v;
    }
    
    void newLine(size_t count){
      line_ += count;
    }
    
    void newLine(){
      ++line_;
      advance(1);
    }
    
    size_t line() const{
      return line_;
    }
    
    void handleGet(const nvar& head, nvar& rest, nvar& out){
      out = head;
      
      for(size_t i = 0; i < rest.size(); ++i){
        nvar& h = rest[i];
        h.pushFront(out);
        out = move(h);
      }
    }
    
    const nvar& getType(const nstr& t);

    bool handleBuiltin(nvar& f);
    
    bool handleVarBuiltin(nvar& f);
    
    nvar createSwitch(const nvar& v, const nvar& cs){
      const nmmap& mm = cs;
      
      nvar ret = func("Switch") << v;
      nvar d = none;
      
      for(auto& itr : mm){
        const nvar& k = itr.first;
        const nvar& v = itr.second;
        
        if(k.isSymbol("__default")){
          d = v;
          continue;
        }
        else if(k.isHidden()){
          continue;
        }
        
        if(ret.hasKey(k)){
          error(k, "duplicate case in switch");
          continue;
        }
        
        if(!k.isInteger()){
          error(k, "case is not integral");
          continue;
        }
        
        ret(k) = v;
      }
      
      ret << d;
      
      return ret;
    }
    
    void addItems(nvar::Type seqType,
                  nvar::Type mapType,
                  nvar& v,
                  nvar& items){
      
      bool noSeq = true;
      bool noMap = true;
      
      size_t size = items.size();
      for(size_t i = 0; i < size; ++i){
        nvar& ii = items[i];
        
        if(ii.isFunction("V_")){
          if(noSeq){
            switch(seqType){
              case nvar::Vector:
                v.intoVector();
                break;
              case nvar::List:
                v.intoList();
                break;
              case nvar::Queue:
                v.intoQueue();
                break;
              default:
                assert(false);
            }
            noSeq = false;
          }
          
          v << move(ii[0]);
        }
        else if(ii.isFunction("KV_")){
          if(noMap){
            switch(mapType){
              case nvar::Map:
                v.intoMap();
                break;
              case nvar::HashMap:
                v.intoHashMap();
                break;
              default:
                assert(false);
            }
            noMap = false;
          }
          
          v(ii[0]) = move(ii[1]);
        }
        else if(ii.isFunction("K_")){
          if(noMap){
            switch(mapType){
              case nvar::Set:
                v.intoSet();
                break;
              case nvar::HashSet:
                v.intoHashSet();
                break;
              default:
                assert(false);
            }
            noMap = false;
          }
          
          v.addKey(ii[0]);
        }
        else{
          assert(false);
        }
      }
    }
    
  private:
    NPLParser* o_;
    ostream* estr_;
    nstr file_;
    size_t line_;
    size_t char_;
    size_t lastChar_;
    int status_;
    nvar* tags_;
    void* scanner_;
    nvar out_;
    nvar defineMap_;
  };
  
} // end namespace neu

#endif // NEU_NPL_PARSER__H

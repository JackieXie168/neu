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

#ifndef NEU_NML_PARSER__H
#define NEU_NML_PARSER__H

#include <cstring>
#include <unistd.h>

#include <neu/NMLParser.h>
#include <neu/NSys.h>
#include <neu/NProgram.h>

#include "parse.h"

using namespace std;
using namespace neu;

namespace neu{
  
  class NMLParser_{
  public:
    
    NMLParser_(NMLParser* o)
    : o_(o),
    estr_(&cerr),
    metadata_(true){}
    
    ~NMLParser_(){}
    
    bool handleVarBuiltin(nvar& f);
    
    void setMetadata(bool flag){
      metadata_ = flag;
    }
    
    bool interactive(){
      return interactive_;
    }
    
    void advance(const char* text, const nstr& tag=""){
      advance(strlen(text), tag);
    }
    
    void advance(size_t count, const nstr& tag=""){
      if(tags_ && !tag.empty()){
        nvar t;
        t("tag") = tag;
        t("start") = char_;
        t("length") = count;
        t("line") = line_;
        tags_->push_back(move(t));
      }
      
      char_ += count;
    }
    
    nvar token(const char* text){
      size_t len = strlen(text);
      nvar token = text;
      
      if(tags_){
        token("tagIndex") = tags_->size();
        
        tags_->push_back(nvar());
        nvar& t = tags_->back();
        t("start") = char_;
        t("length") = len;
        t("line") = line_;
      }
      
      char_ += len;
      
      return token;
    }
    
    void setTag(nvar& token, const nstr& tag){
      if(token.has("tagIndex")){
        size_t index = token["tagIndex"];
        nvar& t = (*tags_)[index];
        t("tag") = tag;
      }
    }
    
    nvar parse(nvec* tags){
      out_ = func("Block");
      
      interactive_ = true;
      openTokens_ = 0;
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = "";
      
      nml_lex_init(&scanner_);
      nml_set_extra(this, scanner_);
      
      FILE* file = readLine();
      if(file){
        nml_set_in(file, scanner_);
        nml_parse(this, scanner_);
      }
      fclose(file);
      
      nml_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      return out_.size() == 1 ? out_[0] : out_;
    }
    
    nvar parse(const nstr& code, nvec* tags){
      out_ = func("Block");
      
      interactive_ = false;
      openTokens_ = 0;
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = "";
      
      nml_lex_init(&scanner_);
      nml_set_extra(this, scanner_);
      
      FILE* file = tmpfile();
      fwrite(code.c_str(), 1, code.length(), file);
      rewind(file);
      
      nml_set_in(file, scanner_);
      nml_parse(this, scanner_);
      fclose(file);
      
      nml_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      return out_.size() == 1 ? out_[0] : out_;
    }
    
    nvar parseFile(const nstr& path, nvec* tags){
      out_ = func("Block");
      
      interactive_ = false;
      openTokens_ = 0;
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = NSys::basename(path);
      
      nml_lex_init(&scanner_);
      nml_set_extra(this, scanner_);
      
      FILE* file = fopen(path.c_str(), "r");
      if(!file){
        NERROR("failed to open: " + path);
      }
      
      nml_set_in(file, scanner_);
      nml_parse(this, scanner_);
      fclose(file);
      
      nml_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      return out_.size() == 1 ? out_[0] : out_;
    }
    
    void setErrorStream(ostream& estr){
      estr_ = &estr;
    }
    
    void emit(nvar& n){
      if(n.some()){
        out_ << move(n);
      }
    }
    
    nvar error(const nvar& n, const nstr& message){
      status_ = 1;
      
      ostream& estr = *estr_;
      
      estr << "NMLParser error: ";
      
      if(metadata_){
        estr << n.getLocation() << ": ";
      }

      estr << message << endl;
      
      return sym("Error");
    }
    
    void error(const nstr& type){
      status_ = 1;
      *estr_ << "NMLParser error: " << getLocation() << ": " << type << endl;
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

      if(metadata_){
        v.setLine(line_);
      
        if(!file_.empty()){
          v.setFile(file_);
        }
      }
      
      return v;
    }
    
    nvar func(const char* f){
      nvar v = nfunc(f);

      if(metadata_){
        v.setLine(line_);
        if(!file_.empty()){
          v.setFile(file_);
        }
      }
      
      return v;
    }
    
    nvar func(const nvar& v){
      return func(v.str());
    }
    
    nvar sym(const nstr& s){
      nvar v = nsym(s);

      if(metadata_){
        v.setLine(line_);
        if(!file_.empty()){
          v.setFile(file_);
        }
      }
      
      return v;
    }
    
    nvar sym(const nvar& v){
      return sym(v.str());
    }
    
    nvar sym(const char* s){
      nvar v = nsym(s);
      
      if(metadata_){
        v.setLine(line_);
        if(!file_.empty()){
          v.setFile(file_);
        }
      }
      
      return v;
    }
    
    nvar var(const nvar& v){
      return v;
    }
    
    void newLine(){
      ++line_;
      advance(1);
    }
    
    void newLine(size_t count){
      line_ += count;
    }
    
    void handleGet(const nvar& head, nvar& rest, nvar& out){
      out = head;
      
      for(size_t i = 0; i < rest.size(); ++i){
        nvar& h = rest[i];
        h.pushFront(out);
        out = move(h);
      }
    }
    
    void createSwitch(nvar& out, const nvar& v, const nvar& cs){
      const nmmap& mm = cs;
      
      out = func("Switch") << v << none << undef;
      nvar& d = out[1];
      nvar& m = out[2];
      
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
        
        if(m.has(k)){
          error(k, "duplicate case in switch");
          continue;
        }
        
        m(k) = v;
      }
    }
    
    void createClass(nvar& c, const nstr& name, nvar& block){
      c = func("Class") << undef;

      nvar& info = c[0];
      
      info("name") = sym(name);
      nvar& ctors = info("ctors") = nmap();
      nvar& stmts = info("stmts") = func("Block");

      for(nvar& bi : block){
        if(bi.isFunction("Ctor", 3)){
          const nvar& f = bi[1];
          if(f.str() != name){
            error(f, "invalid ctor");
            continue;
          }
          
          size_t size = f.size();
          if(ctors.has(size)){
            error(f, "duplicate ctor");
            continue;
          }
          
          ctors(size) = move(bi);
        }
        else if(bi.isFunction("Def", 2)){
          nvar& f = bi[0];
          if(f.isFunction() && f.str() == name){
            size_t size = f.size();
            
            if(ctors.has(size)){
              error(f, "duplicate ctor");
              continue;
            }
            
            nvar ctor = func("Ctor") << func("NObject") << move(f) << move(bi[1]);
            ctors(size) = move(ctor);
          }
          else{
            stmts << move(bi);
          }
        }
        else{
          stmts << move(bi);
        }
      }
    }
    
    FILE* readLine(){
      nstr line;
      if(!o_->readLine(line)){
        return 0;
      }
         
      line += "\n";
         
      int pfd[2];
      
      pipe(pfd);
      
      FILE* file = fdopen(pfd[1], "w");
      
      fwrite(line.c_str(), 1, line.length(), file);
      fclose(file);
      
      return fdopen(pfd[0], "r");
    }
    
    void openToken(){
      ++openTokens_;
    }
    
    void closeToken(){
      if(openTokens_ == 0){
        error("too many closing tokens");
      }
      --openTokens_;
    }
    
    bool hasOpenTokens(){
      return openTokens_ != 0;
    }
    
    void addItems(nvar::Type seqType,
                  nvar::Type mapType,
                  nvar& v,
                  nvar& items){

      bool noSeq = true;
      bool noMap = true;
      bool shouldEval = false;
      
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

          nvar& item = ii[0];
          
          if(item.isSymbolic()){
            shouldEval = true;
          }
          
          v << move(item);
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
              case nvar::Multimap:
                v.intoMultimap();
                break;
              default:
                assert(false);
            }
            noMap = false;
          }
          
          nvar& item = ii[1];
          
          if(item.isSymbolic()){
            shouldEval = true;
          }
          
          v(ii[0]) = move(item);
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
              case nvar::Map:
                v.intoMap();
                break;
              case nvar::HashMap:
                v.intoHashMap();
                break;
              case nvar::Multimap:
                v.intoMultimap();
                break;
              default:
                assert(false);
            }
            noMap = false;
          }
          
          v << move(ii[0]);
        }
        else{
          assert(false);
        }
      }
      
      if(shouldEval){
        v = nfunc("Eval") << move(v);
      }
    }
    
    nvar stringLiteral(const char* str){
      NMLParser* parser = 0;
      size_t i = 1;
      char ci;
      bool err = false;
      
      nvar rv = "";
      nstr& rs = rv.str();
      nvar itp;
      
      for(;;){
        ci = str[i];
        
        if(ci == '%' &&
           str[i + 1] == '{' &&
           str[i - 1] != '\\' &&
           str[i + 2] != '\0'){
        
          char cj;
          size_t open = 1;
          size_t j = i + 2;
          nstr expr;
          bool done = false;
          
          for(;;){
            cj = str[j];

            switch(cj){
              case '\0':
                done = true;
                break;
              case '}':
                --open;
              
                if(open == 0){
                  done = true;
                  break;
                }
                
                expr += cj;
                
                break;
              case '{':
                ++open;
                expr += cj;
                
                break;
              default:
                expr += cj;
                break;
            }
            
            ++j;
            
            if(done){
              break;
            }
          }
          
          if(open > 0){
            error("invalid string interpolate: " + nstr(str));
            err = true;
            break;
          }
          
          if(!parser){
            parser = new NMLParser;
          }
          
          expr += ";";
          
          nvar e = parser->parse(expr);
          
          if(e.isNone()){
            error("invalid expression in string interpolate: " + expr);
            err = true;
            break;
          }
          
          if(itp.isUndef()){
            itp = func("Itp");
          }
          
          itp << e;
          i = j;
          rs += "%_";
        }
        else if(str[i + 1] == '\0'){
          break;
        }
        else{
          rs += ci;
          ++i;
        }
      }
      
      if(parser){
        delete parser;
      }
      
      if(err){
        return sym("Error");
      }
      
      if(itp.isUndef()){
        return rv;
      }

      itp.pushFront(rv);
    
      return itp;
    }

  private:
    NMLParser* o_;
    ostream* estr_;
    nstr file_;
    size_t line_;
    size_t char_;
    int status_;
    nvec* tags_;
    void* scanner_;
    nvar out_;
    bool metadata_;
    bool interactive_;
    size_t openTokens_;
  };
  
} // end namespace neu

#endif // NEU_NML_PARSER__H

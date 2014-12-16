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

#ifndef NEU_NHS_PARSER__H
#define NEU_NHS_PARSER__H

#include <cstring>
#include <unistd.h>

#include <neu/NHSParser.h>
#include <neu/NSys.h>
#include <neu/NProgram.h>

#include "parse.h"

using namespace std;
using namespace neu;

namespace neu{
  
  class NHSParser_{
  public:
    
    NHSParser_(NHSParser* o)
    : o_(o),
    estr_(&cerr){}
    
    ~NHSParser_(){}
    
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
        tags_->pushBack(move(t));
      }
      
      char_ += count;
    }
    
    nvar token(const char* text, const nstr& tag=""){
      size_t len = strlen(text);
      
      nvar token = text;
      token("start") = char_;
      token("length") = len;
      token("line") = line_;
      
      if(!tag.empty()){
        if(tags_){
          tags_->pushBack(nvar());
          nvar& t = tags_->back();
          t("tag") = tag;
          t("start") = char_;
          t("length") = len;
          t("line") = line_;
        }
      }
      
      char_ += len;
      
      return token;
    }
    
    nvar parse(nvar* tags){
      out_ = nfunc("Block");
      
      interactive_ = true;
      openTokens_ = 0;
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = "";
      callStack_.clear();
      pushCall();
      
      nhs_lex_init(&scanner_);
      nhs_set_extra(this, scanner_);
      
      FILE* file = readLine();
      if(file){
        nhs_set_in(file, scanner_);
        nhs_parse(this, scanner_);
      }
      fclose(file);
      
      nhs_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      switch(out_.size()){
        case 0:
          return none;
        case 1:
          return out_[0];
        default:
          return out_;
      }
    }
    
    nvar parse(const nstr& code, nvar* tags){
      out_ = nfunc("Block");
      
      interactive_ = false;
      openTokens_ = 0;
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = "";
      callStack_.clear();
      pushCall();      

      nhs_lex_init(&scanner_);
      nhs_set_extra(this, scanner_);
      
      FILE* file = tmpfile();
      fwrite(code.c_str(), 1, code.length(), file);
      rewind(file);
      
      nhs_set_in(file, scanner_);
      nhs_parse(this, scanner_);
      fclose(file);
      
      nhs_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      switch(out_.size()){
        case 0:
          return none;
        case 1:
          return out_[0];
        default:
          return out_;
      };
    }
    
    nvar parseFile(const nstr& path, nvar* tags){
      out_ = nfunc("Block");
      
      interactive_ = false;
      openTokens_ = 0;
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = NSys::basename(path);
      callStack_.clear();
      pushCall();      

      nhs_lex_init(&scanner_);
      nhs_set_extra(this, scanner_);
      
      FILE* file = fopen(path.c_str(), "r");
      if(!file){
        NERROR("failed to open: " + path);
      }
      
      nhs_set_in(file, scanner_);
      nhs_parse(this, scanner_);
      fclose(file);
      
      nhs_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      switch(out_.size()){
        case 0:
          return none;
        case 1:
          return out_[0];
        default:
          return out_;
      }
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
      
      estr << "NHSParser error: ";
      
      nstr loc = n.getLocation();
      
      if(!loc.empty()){
        estr << loc << ": ";
      }
      
      estr << message << endl;
      
      return nsym("Error");
    }
    
    void error(const nstr& type){
      status_ = 1;
      *estr_ << "NHSParser error: " << getLocation() << ": " << 
        type << endl;
    }
    
    nstr getLocation(){
      nstr loc;
      
      if(!file_.empty()){
        loc += file_ + ":";
      }
      
      loc += nvar(line_);
      
      return loc;
    }
    
    void newLine(){
      ++line_;
      advance(1);
    }
    
    void newLine(size_t count){
      line_ += count;
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
        NERROR("too many closing tokens");
      }
      --openTokens_;
    }
    
    bool hasOpenTokens(){
      return openTokens_ != 0;
    }

    nvar transform(nvar& n){
      if(n.isFunction()){
        if(n.str() == "HTo_"){
          n.str() = "HTo";
        }

        for(nvar& ni : n){
          transform(ni);
        }
      }

      return n;
    }    

    nvar getList(nvar& n){
      if(n.isFunction("HRangeList_")){
        n.str() = "HRangeList";
      }
      else if(n.isFunction("HInfList_")){
        n.str() = "HInfList";
      }
        
      if(n.size() == 2){
        if(n[1].isFunction("HRangeList_")){
          n[1].str() = "HRangeList";
          n[1].pushFront(n[0]);
          return n[1];
        }
        else if(n[1].isFunction("HInfList_")){
          n[1].str() = "HInfList";
          n[1].pushFront(n[0]);
          return n[1];
        }
      }
      else{
        for(nvar& ni : n){
          if(ni.isFunction("HRangeList_")){
            error(n, "invalid list");
            return nsym("Error");
          }
          else if(ni.isFunction("HInfList_")){
            error(n, "invalid list");
            return nsym("Error");
          }
        }
      }

      for(nvar& ni : n){
        if(ni.isSymbolic()){
          return nfunc("Eval") << move(n);
        }
      }
      
      return n;
    }

    nvar getCall(nvar& c, nvar& p){
      if(c.isFunction("HCall")){
        c[0] << p;
        return c;
      }
      else if(c.isSymbol()){
        return nfunc("HCall") << (nfunc(c) << p);
      }
        
      error(c, "invalid call");

      return nsym("Error");
    }

    void pushCall(){
      callStack_.push_back(false);
    }

    void popCall(){
      assert(!callStack_.empty());
      callStack_.pop_back();
    }

    bool inCall(){
      assert(!callStack_.empty());

      return callStack_.back(); 
    }

    void enterCall(){
      assert(!callStack_.empty());
    
      callStack_.back() = true;
    }

    void exitCall(){
      assert(!callStack_.empty());
    
      callStack_.back() = false;
    }

  private:
    typedef NVector<bool> CallStack_;

    NHSParser* o_;
    ostream* estr_;
    nstr file_;
    size_t line_;
    size_t char_;
    int status_;
    nvar* tags_;
    void* scanner_;
    nvar out_;
    bool interactive_;
    size_t openTokens_;
    CallStack_ callStack_;
  };
  
} // end namespace neu

#endif // NEU_NHS_PARSER__H

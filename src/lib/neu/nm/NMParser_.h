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

#ifndef NEU_NM_PARSER__H
#define NEU_NM_PARSER__H

#include <unistd.h>
#include <cstring>

#include <neu/NMParser.h>
#include <neu/NSys.h>
#include <neu/NProgram.h>

#include "parse.h"

using namespace std;
using namespace neu;

namespace neu{
  
  class NMParser_{
  public:
    
    NMParser_(NMParser* o)
    : o_(o),
    estr_(&cerr){}
    
    ~NMParser_(){}
    
    void advance(const char* text, const nstr& tag=""){
      advance(strlen(text), tag);
    }
    
    void advance(size_t count, const nstr& tag=""){
      if(!tag.empty()){
        if(tags_){
          tags_->pushBack(nvar());
          nvar& t = tags_->back();
          t("tag") = tag;
          t("start") = char_;
          t("length") = count;
          t("line") = line_;
        }
        ++token_;
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
      ++token_;
      
      return token;
    }
    
    nvar parse(const nstr& code, nvar* tags){
      out_ = func("Block");
      
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = "";
      
      nm_lex_init(&scanner_);
      nm_set_extra(this, scanner_);
      
      FILE* file = tmpfile();
      fwrite(code.c_str(), 1, code.length(), file);
      rewind(file);
      
      nm_set_in(file, scanner_);
      nm_parse(this, scanner_);
      fclose(file);
      
      nm_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      return out_.size() == 1 ? out_[0] : out_;
    }
    
    nvar parseFile(const nstr& path, nvar* tags){
      out_ = func("Block");
      
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = NSys::basename(path);
      
      nm_lex_init(&scanner_);
      nm_set_extra(this, scanner_);
      
      FILE* file = fopen(path.c_str(), "r");
      
      if(!file){
        NERROR("failed to open: " + path);
      }
      
      nm_set_in(file, scanner_);
      nm_parse(this, scanner_);
      fclose(file);
      
      nm_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      return out_.size() == 1 ? out_[0] : out_;
    }
    
    void emit(nvar& n){
      out_ << move(n);
    }
    
    nvar error(const nvar& n, const nstr& message){
      status_ = 1;
      
      ostream& estr = *estr_;
      
      estr << "NMParser error: " << n.getLocation() << ": " << message << endl;

      return sym("Error");
    }
    
    void error(const nstr& type){
      status_ = 1;
      *estr_ << "NMParser error: " << getLocation() << ": " << type << endl;
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
      v("__token") = token_;
      if(!file_.empty()){
        v.setFile(file_);
      }
      
      return v;
    }
    
    nvar func(const char* f){
      nvar v = nfunc(f);
      v.setLine(line_);
      v("__token") = token_;
      if(!file_.empty()){
        v.setFile(file_);
      }
      
      return v;
    }
    
    nvar func(const nvar& v){
      return func(v.str());
    }
    
    nvar sym(const nstr& s){
      nvar v = nsym(s);
      v.setLine(line_);
      v("__token") = token_;
      if(!file_.empty()){
        v.setFile(file_);
      }
      
      return v;
    }
    
    nvar sym(const nvar& v){
      return sym(v.str());
    }
    
    nvar sym(const char* s){
      nvar v = nsym(s);
      v.setLine(line_);
      v("__token") = token_;
      if(!file_.empty()){
        v.setFile(file_);
      }
      
      return v;
    }
    
    nvar& var(nvar& v){
      v("__token") = token_;
      return v;
    }
    
    nvar var(const nvar& v){
      nvar r = v;
      r("__token") = token_;
      return r;
    }
    
    void newLine(){
      ++line_;
    }
    
    void translate(nvar& v);
    
  private:
    NMParser* o_;
    ostream* estr_;
    nstr file_;
    size_t line_;
    size_t char_;
    int token_;
    int status_;
    nvar* tags_;
    void* scanner_;
    nvar out_;
  };
  
} // end namespace neu

#endif // NEU_NM_PARSER__H

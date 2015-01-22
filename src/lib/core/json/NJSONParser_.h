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


The Neu Framework, Copyright (c) 2013-2015, Andrometa LLC
All rights reserved.

neu@andrometa.net
http://neu.andrometa.net

Neu can be used freely for commercial purposes. If you find Neu
useful, please consider helping to support our work and the evolution
of Neu by making a donation via: http://donate.andrometa.net

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

#ifndef NEU_N_JSON_PARSER__H
#define NEU_N_JSON_PARSER__H

#include <cstring>
#include <unistd.h>

#include <neu/NJSONParser.h>
#include <neu/NSys.h>
#include <neu/NProgram.h>

#include "parse.h"

using namespace std;
using namespace neu;

namespace neu{
  
  class NJSONParser_{
  public:
    
    NJSONParser_(NJSONParser* o)
    : o_(o),
    estr_(&cerr){}
    
    ~NJSONParser_(){}
    
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
    
    nvar parse(const nstr& code, nvec* tags){
      out_ = undef;
      
      openTokens_ = 0;
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = "";
      
      njson_lex_init(&scanner_);
      njson_set_extra(this, scanner_);
      
      FILE* file = tmpfile();
      fwrite(code.c_str(), 1, code.length(), file);
      rewind(file);
      
      njson_set_in(file, scanner_);
      njson_parse(this, scanner_);
      fclose(file);
      
      njson_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      return out_;
    }
    
    nvar parseFile(const nstr& path, nvec* tags){
      out_ = undef;
      
      openTokens_ = 0;
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = NSys::basename(path);
      
      njson_lex_init(&scanner_);
      njson_set_extra(this, scanner_);
      
      FILE* file = fopen(path.c_str(), "r");
      if(!file){
        NERROR("failed to open: " + path);
      }
      
      njson_set_in(file, scanner_);
      njson_parse(this, scanner_);
      fclose(file);
      
      njson_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      return out_;
    }
    
    void setErrorStream(ostream& estr){
      estr_ = &estr;
    }
    
    void emit(nvar& n){
      out_ = move(n);
    }
    
    nvar error(const nvar& n, const nstr& message){
      status_ = 1;
      
      ostream& estr = *estr_;
      
      estr << "NJSONParser error: ";
      
      estr << message << endl;
      
      return nsym("Error");
    }
    
    void error(const nstr& type){
      status_ = 1;
      *estr_ << "NJSONParser error: " << getLocation() << ": " << type << endl;
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
    
  private:
    NJSONParser* o_;
    ostream* estr_;
    nstr file_;
    size_t line_;
    size_t char_;
    int status_;
    nvec* tags_;
    void* scanner_;
    nvar out_;
    size_t openTokens_;
  };
  
} // end namespace neu

#endif // NEU_N_JSON_PARSER__H

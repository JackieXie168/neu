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

#ifndef NEU_N_PARSER_H
#define NEU_N_PARSER_H

#include <neu/nvar.h>

namespace neu{

class NParser{
protected:
  NParser(const nstr& name);
  
public:
  typedef typename nvar::Type Type;
  
  static const Type Undefined = nvar::Undefined;
  static const Type False = nvar::False;
  static const Type True = nvar::True;
  static const Type Integer = nvar::Integer;
  static const Type Float = nvar::Float;
  static const Type Real = nvar::Real;
  static const Type Symbol = nvar::Symbol;
  static const Type String = nvar::String;
  
  typedef std::function<nvar(nstr& s)> LexFunc;

  typedef std::function<nvar(nvec& v)> ReduceFunc;

  virtual ~NParser();

  virtual void init() = 0;

  nvar parse(const nstr& str,
             bool metadata=true,
             const nstr& sourceName="",
             double timeout=5.0,
             std::ostream* errorLog=0,
             std::ostream* parseLog=0);
  
  void discardWhitespace(){
    addDiscard("\\s+");
  }
  
  void discardNewLine(){
    addDiscard("\n+");
  }
  
  void addDiscard(const nstr& regex){
    addTerm("", regex);
  }
  
  void addPunc(const nstr& punc, size_t prec=0, bool left=true){
    addTerm(punc, prec, left);
  }
  
  void addKeyword(const nstr& keyword){
    addTerm(keyword);
  }

  void useReal(const nstr& name){
    addTerm(name,
            "((\\d+\\.\\d+)|(\\.\\d+)|(\\d+)"
            "([Ee][\\+\\-]?\\d+)?)[rR]", Real, 1);
  }

  void useFloat(const nstr& name){
    addTerm(name,
            "(\\d+\\.\\d+)|(\\.\\d+)|(\\d+)"
            "([Ee][\\+\\-]?\\d+)?", Float);
  }
  
  void useInteger(const nstr& name){
    addTerm(name, "\\d+", Integer);
  }
  
  void useString(const nstr& name){
    addTerm(name, "\"(.*?)\"", String, 1);
  }

  void useSingleString(const nstr& name){
    addTerm(name, "\'(.*?)\'", String, 1);
  }
  
  void useSymbol(const nstr& name){
    addTerm(name, "([a-zA-Z]+)|(_+[a-zA-Z]\\w*)|([a-zA-Z]\\w*)", Symbol);
  }
  
  void addTerm(const nstr& marker, size_t prec=0, bool left=true);
  
  void addTerm(const nstr& name,
               const nstr& regex,
               Type type=String,
               size_t pos=0,
               size_t prec=0,
               bool left=true);
  
  void addTerm(const nstr& name, LexFunc f, size_t prec=0, bool left=true);
  
  void addRule(const nstr& name, const nstr& rule, size_t pos=0);
  
  void addRule(const nstr& name, const nstr& rule, ReduceFunc f);
  
  void getInitLog(nstr& log);
  
  int numOpenMarkers(const nstr& s){
    size_t len = s.length();
    
    int open = 0;
    
    bool inString = false;
    bool escaped = false;
    
    for(size_t i = 0; i < len; ++i){
      switch(s[i]){
        case '(':
        case '[':
        case '{':
          if(escaped){
            escaped = false;
            break;
          }
          
          if(inString){
            break;
          }
          
          ++open;
          break;
        case ')':
        case ']':
        case '}':
          if(escaped){
            escaped = false;
            break;
          }
          
          if(inString){
            break;
          }
          
          --open;
          break;
        case '\\':
          escaped = true;
          break;
        case '\'':
        case '"':
          if(escaped){
            escaped = false;
            break;
          }

          inString = !inString;
          break;
        default:
          break;
      }
    }
    
    return open;
  }
  
private:
  class NParser_* x_;
};

} // end namespace neu

#endif // NEU_N_PARSER

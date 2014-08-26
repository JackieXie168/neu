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

#include <neu/NPLParser.h>

#include "NPLParser_.h"

using namespace std;
using namespace neu;

namespace{
  
  class Global{
  public:
    Global(){
      nameMap_("for") = true;
      nameMap_("if") = true;
      nameMap_("else") = true;
      nameMap_("while") = true;
      nameMap_("return") = true;
      nameMap_("throw") = true;
      nameMap_("true") = true;
      nameMap_("false") = true;
      nameMap_("break") = true;
      nameMap_("continue") = true;
      nameMap_("new") = true;
      nameMap_("create") = true;
      nameMap_("delete") = true;
      nameMap_("this") = true;
      nameMap_("try") = true;
      nameMap_("catch") = true;
      
      typeMap_("void") = nml("[]");
      typeMap_("bool") = nml("[bits:1]");
      typeMap_("char") = nml("[bits:8]");
      typeMap_("uchar") = nml("[bits:8, signed:false]");
      typeMap_("short") = nml("[bits:16]");
      typeMap_("ushort") = nml("[bits:16, signed:false]");
      typeMap_("int") = nml("[bits:32]");
      typeMap_("uint") = nml("[bits:32, signed:false]");
      typeMap_("long") = nml("[bits:64]");
      typeMap_("ulong") = nml("[bits:64, signed:false]");
      typeMap_("float") = nml("[bits:32, float:true]");
      typeMap_("double") = nml("[bits:64, float:true]");
      typeMap_("nvar") = nml("[bits:72, var:true]");

      builtinMap_({"shl", 2}) = "ShL";
      builtinMap_({"shr", 2}) = "ShR";
      builtinMap_({"xor", 2}) = "XOr";
      builtinMap_({"bxor", 2}) = "BitXOr";
      builtinMap_({"len", 1}) = "Len";
      builtinMap_({"size", 1}) = "Size";
      builtinMap_({"sqrt", 1}) = "Sqrt";
      builtinMap_({"exp", 1}) = "Exp";
      builtinMap_({"abs", 1}) = "Abs";
      builtinMap_({"log", 1}) = "Log";
      builtinMap_({"log10", 1}) = "Log10";
      builtinMap_({"floor", 1}) = "Floor";
      builtinMap_({"ceil", 1}) = "Ceil";
      builtinMap_({"normalize", 1}) = "Normalize";
      builtinMap_({"dot", 2}) = "DotProduct";
      builtinMap_({"cross", 2}) = "CrossProduct";
      builtinMap_({"cos", 1}) = "Cos";
      builtinMap_({"acos", 1}) = "Acos";
      builtinMap_({"cosh", 1}) = "Cosh";
      builtinMap_({"sin", 1}) = "Sin";
      builtinMap_({"asin", 1}) = "Asin";
      builtinMap_({"sinh", 1}) = "Sinh";
      builtinMap_({"tan", 1}) = "Tan";
      builtinMap_({"atan", 1}) = "Atan";
      builtinMap_({"tanh", 1}) = "Tanh";
      builtinMap_({"uniform", 0}) = "Uniform";
      builtinMap_({"uniform", 2}) = "Uniform";
      
      varBuiltinMap_({"pushBack", 1}) = "PushBack";
      varBuiltinMap_({"intoMultimap", 0}) = "TouchMultimap";
      varBuiltinMap_({"intoList", 0}) = "TouchList";
      varBuiltinMap_({"keys", 0}) = "Keys";
      varBuiltinMap_({"pushFront", 1}) = "PushFront";
      varBuiltinMap_({"popBack", 0}) = "PopBack";
      varBuiltinMap_({"hasKey", 1}) = "HasKey";
      varBuiltinMap_({"insert", 2}) = "Insert";
      varBuiltinMap_({"clear", 0}) = "Clear";
      varBuiltinMap_({"empty", 0}) = "Empty";
      varBuiltinMap_({"back", 0}) = "Back";
      varBuiltinMap_({"get", 1}) = "Get";
      varBuiltinMap_({"get", 2}) = "Get";
      varBuiltinMap_({"erase", 0}) = "Erase";
      varBuiltinMap_({"merge", 1}) = "Merge";
      varBuiltinMap_({"outerMerge", 0}) = "OuterMerge";
    }
    
    bool isReservedName(const nstr& name) const{
      return nameMap_.hasKey(name);
    }
    
    const nvar& getType(const nstr& t){
      return typeMap_.get(t, none);
    }
    
    bool handleBuiltin(nvar& f){
      nvar b = builtinMap_.get({f.str(), f.size()}, none);

      if(b == none){
        return false;
      }

      f.str() = b;
      return true;
    }
    
    bool handleVarBuiltin(nvar& f){
      nvar b = varBuiltinMap_.get({f.str(), f.size()}, none);
      
      if(b == none){
        return false;
      }
      
      f.str() = b;
      return true;
    }
    
    nvar nameMap_;
    nvar typeMap_;
    nvar builtinMap_;
    nvar varBuiltinMap_;
  };
  
  Global _global;
  
} // end namespace

const nvar& NPLParser_::getType(const nstr& t){
  return _global.getType(t);
}

bool NPLParser_::handleBuiltin(nvar& f){
  return _global.handleBuiltin(f);
}

bool NPLParser_::handleVarBuiltin(nvar& f){
  return _global.handleVarBuiltin(f);
}

NPLParser::NPLParser(){
  x_ = new NPLParser_(this);
}

NPLParser::~NPLParser(){
  delete x_;
}

nvar NPLParser::parse(const nstr& code, nvar* tags){
  return x_->parse(code, tags);
}

nvar NPLParser::parseFile(const nstr& path, nvar* tags){
  return x_->parseFile(path, tags);
}

bool NPLParser::isReservedName(const nstr& name){
  return _global.isReservedName(name);
}

void NPLParser::setErrorStream(ostream& estr){
  x_->setErrorStream(estr);
}

nvar NPLParser::parseType(const nstr& t){
  nstr tn = t;
  
  size_t ptr = 0;
  
  while(tn.endsWith("*")){
    ++ptr;
    tn.erase(tn.length() - 1, 1);
  }
  
  size_t len = 0;
  for(size_t i = 0; i < tn.length(); ++i){
    if(nstr::isDigit(tn[i])){
      len = atoi(tn.substr(i, tn.length() - i).c_str());
      tn = tn.substr(0, i);
      break;
    }
  }
  
  nvar ti = _global.getType(tn);
  
  if(ti == none){
    NERROR("invalid type: " + t);
  }
  
  nvar ret = ti;
  
  if(ptr > 0){
    ret("ptr") = ptr;
  }
  
  if(len > 0){
    ret("len") = len;
  }
  
  return ret;
}

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

#include <neu/NMLParser.h>

#include "NMLParser_.h"

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
      nameMap_("this") = true;
      nameMap_("try") = true;
      nameMap_("catch") = true;
      
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
      varBuiltinMap_({"erase", 1}) = "Erase";
      varBuiltinMap_({"merge", 1}) = "Merge";
      varBuiltinMap_({"outerMerge", 1}) = "OuterMerge";
      varBuiltinMap_({"isFalse", 0}) = "IsFalse";
      varBuiltinMap_({"isTrue", 0}) = "IsTrue";
      varBuiltinMap_({"isDefined", 0}) = "IsDefined";
      varBuiltinMap_({"isString", 0}) = "IsString";
      varBuiltinMap_({"isSymbol", 0}) = "IsSymbol";
      varBuiltinMap_({"isFunction", 0}) = "IsFunction";
      varBuiltinMap_({"isFunction", 1}) = "IsFunction";
      varBuiltinMap_({"isFunction", 2}) = "IsFunction";
      varBuiltinMap_({"isSymbolic", 0}) = "IsSymbolic";
      varBuiltinMap_({"isNumeric", 0}) = "IsNumeric";
      varBuiltinMap_({"isReference", 0}) = "IsReference";
      varBuiltinMap_({"isPointer", 0}) = "IsPointer";
      varBuiltinMap_({"isInteger", 0}) = "IsInteger";
      varBuiltinMap_({"isRational", 0}) = "IsRational";
      varBuiltinMap_({"isReal", 0}) = "IsReal";
      varBuiltinMap_({"str", 0}) = "GetStr";
      varBuiltinMap_({"vec", 0}) = "GetVec";
      varBuiltinMap_({"list", 0}) = "GetList";
      varBuiltinMap_({"anySequence", 0}) = "GetAnySequence";
      varBuiltinMap_({"map", 0}) = "GetMap";
      varBuiltinMap_({"multimap", 0}) = "GetMultimap";
      varBuiltinMap_({"anyMap", 0}) = "GetAnyMap";
      varBuiltinMap_({"append", 1}) = "Append";
      varBuiltinMap_({"normalize", 0}) = "Normalize";
      varBuiltinMap_({"head", 0}) = "Head";
      varBuiltinMap_({"setHead", 1}) = "SetHead";
      varBuiltinMap_({"clearHead", 0}) = "ClearHead";
      varBuiltinMap_({"hasKey", 1}) = "HasKey";
      varBuiltinMap_({"numKeys", 0}) = "NumKeys";
      varBuiltinMap_({"size", 0}) = "Size";
      varBuiltinMap_({"mapEmpty", 0}) = "MapEmpty";
      varBuiltinMap_({"allEmpty", 0}) = "AllEmpty";
      varBuiltinMap_({"hasVector", 0}) = "HasVector";
      varBuiltinMap_({"hasList", 0}) = "HasList";
      varBuiltinMap_({"hasMap", 0}) = "HasMap";
      varBuiltinMap_({"hasMultimap", 0}) = "HasMultimap";
      varBuiltinMap_({"popFront", 0}) = "PopFront";
      varBuiltinMap_({"allKeys", 0}) = "AllKeys";
      varBuiltinMap_({"intoVector", 0}) = "TouchVector";
      varBuiltinMap_({"intoMap", 0}) = "TouchMap";
      varBuiltinMap_({"open", 1}) = "Open";
      varBuiltinMap_({"save", 1}) = "Save";
      varBuiltinMap_({"unite", 1}) = "Unite";
      varBuiltinMap_({"unite", 2}) = "Unite";
      varBuiltinMap_({"intersect", 1}) = "Intersect";
      varBuiltinMap_({"intersect", 2}) = "Intersect";
      varBuiltinMap_({"complement", 1}) = "Complement";
    }
    
    bool isReservedName(const nstr& name) const{
      return nameMap_.hasKey(name);
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
    nvar varBuiltinMap_;
  };
  
  Global _global;
  
} // end namespace

bool NMLParser_::handleVarBuiltin(nvar& f){
  return _global.handleVarBuiltin(f);
}

NMLParser::NMLParser(){
  x_ = new NMLParser_(this);
}

NMLParser::~NMLParser(){
  delete x_;
}

nvar NMLParser::parse(nvar* tags){
  return x_->parse(tags);
}

nvar NMLParser::parse(const nstr& code, nvar* tags){
  return x_->parse(code, tags);
}

nvar NMLParser::parseFile(const nstr& path, nvar* tags){
  return x_->parseFile(path, tags);
}

bool NMLParser::isReservedName(const nstr& name){
  return _global.isReservedName(name);
}

void NMLParser::setErrorStream(ostream& estr){
  x_->setErrorStream(estr);
}

void NMLParser::setMetadata(bool flag){
  x_->setMetadata(flag);
}

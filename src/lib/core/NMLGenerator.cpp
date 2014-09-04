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

#include <neu/NMLGenerator.h>

using namespace std;
using namespace neu;

#include <neu/NObject.h>

namespace{
  
  enum SymbolKey{
    
  };
  
  enum FunctionKey{
    FKEY_NO_KEY,
    FKEY_Var_1,
    FKEY_Var_2,
    FKEY_Var_3,
    FKEY_VarSet_2,
    FKEY_Throw_2,
    FKEY_Add_2,
    FKEY_Sub_2,
    FKEY_Mul_2,
    FKEY_Div_2,
    FKEY_Mod_2,
    FKEY_LT_2,
    FKEY_GT_2,
    FKEY_LE_2,
    FKEY_GE_2,
    FKEY_EQ_2,
    FKEY_NE_2,
    FKEY_And_2,
    FKEY_Or_2,
    FKEY_Not_1,
    FKEY_Neg_1,
    FKEY_Inc_1,
    FKEY_PostInc_1,
    FKEY_Dec_1,
    FKEY_PostDec_1,
    FKEY_AddBy_2,
    FKEY_SubBy_2,
    FKEY_MulBy_2,
    FKEY_DivBy_2,
    FKEY_ModBy_2,
    FKEY_Pow_2,
    FKEY_Set_2,
    FKEY_Block_n,
    FKEY_Call_2,
    FKEY_ScopedBlock_n,
    FKEY_Ret_0,
    FKEY_Ret_1,
    FKEY_Break_0,
    FKEY_Continue_0,
    FKEY_If_2,
    FKEY_If_3,
    FKEY_While_2,
    FKEY_For_4,
    FKEY_New_1,
    FKEY_Class_1,
    FKEY_Idx_2,
    FKEY_Dot_2,
    FKEY_DotPut_2,
    FKEY_Cs_1,
    FKEY_Call_1,
    FKEY_Def_2,
    FKEY_PushBack_2,
    FKEY_TouchMultimap_1,
    FKEY_TouchList_1,
    FKEY_Keys_1,
    FKEY_PushFront_2,
    FKEY_PopBack_1,
    FKEY_HasKey_2,
    FKEY_Insert_3,
    FKEY_Clear_1,
    FKEY_Empty_1,
    FKEY_Back_1,
    FKEY_Erase_2,
    FKEY_Merge_2,
    FKEY_OuterMerge_2,
    FKEY_Switch_3,
    FKEY_IsFalse_1,
    FKEY_IsTrue_1,
    FKEY_IsDefined_1,
    FKEY_IsString_1,
    FKEY_IsSymbol_1,
    FKEY_IsFunction_1,
    FKEY_IsSymbolic_1,
    FKEY_IsNumeric_1,
    FKEY_IsReference_1,
    FKEY_IsPointer_1,
    FKEY_IsInteger_1,
    FKEY_IsRational_1,
    FKEY_IsReal_1,
    FKEY_GetStr_1,
    FKEY_GetVec_1,
    FKEY_GetList_1,
    FKEY_GetAnySequence_1,
    FKEY_GetMap_1,
    FKEY_GetMultimap_1,
    FKEY_GetAnyMap_1,
    FKEY_Append_2,
    FKEY_Normalize_1,
    FKEY_Head_1,
    FKEY_SetHead_2,
    FKEY_ClearHead_1,
    FKEY_NumKeys_1,
    FKEY_Size_1,
    FKEY_MapEmpty_1,
    FKEY_AllEmpty_1,
    FKEY_HasVector_1,
    FKEY_HasList_1,
    FKEY_HasMap_1,
    FKEY_HasMultimap_1,
    FKEY_PopFront_1,
    FKEY_AllKeys_1,
    FKEY_TouchVector_1,
    FKEY_TouchMap_1,
    FKEY_Open_2,
    FKEY_Save_2,
    FKEY_Unite_2,
    FKEY_Unite_3,
    FKEY_Intersect_2,
    FKEY_Intersect_3,
    FKEY_Complement_2,
    FKEY_Import_1,
  };
  
  typedef NMap<nstr, SymbolKey> SymbolMap;
  
  typedef NMap<pair<nstr, int>, FunctionKey> FunctionMap;
  
  static SymbolMap _symbolMap;
  
  static FunctionMap _functionMap;
  
  static void _initSymbolMap(){
    
  }
  
  static void _initFunctionMap(){
    _functionMap[{"Var", 1}] = FKEY_Var_1;
    _functionMap[{"Var", 2}] = FKEY_Var_2;
    _functionMap[{"Var", 3}] = FKEY_Var_3;
    _functionMap[{"VarSet", 2}] = FKEY_VarSet_2;
    _functionMap[{"Throw", 2}] = FKEY_Throw_2;
    _functionMap[{"Add", 2}] = FKEY_Add_2;
    _functionMap[{"Sub", 2}] = FKEY_Sub_2;
    _functionMap[{"Mul", 2}] = FKEY_Mul_2;
    _functionMap[{"Div", 2}] = FKEY_Div_2;
    _functionMap[{"Pow", 2}] = FKEY_Pow_2;
    _functionMap[{"Mod", 2}] = FKEY_Mod_2;
    _functionMap[{"Set", 2}] = FKEY_Set_2;
    _functionMap[{"LT", 2}] = FKEY_LT_2;
    _functionMap[{"GT", 2}] = FKEY_GT_2;
    _functionMap[{"LE", 2}] = FKEY_LE_2;
    _functionMap[{"GE", 2}] = FKEY_GE_2;
    _functionMap[{"EQ", 2}] = FKEY_EQ_2;
    _functionMap[{"NE", 2}] = FKEY_NE_2;
    _functionMap[{"Not", 1}] = FKEY_Not_1;
    _functionMap[{"And", 2}] = FKEY_And_2;
    _functionMap[{"Or", 2}] = FKEY_Or_2;
    _functionMap[{"Inc", 1}] = FKEY_Inc_1;
    _functionMap[{"PostInc", 1}] = FKEY_PostInc_1;
    _functionMap[{"Dec", 1}] = FKEY_Dec_1;
    _functionMap[{"PostDec", 1}] = FKEY_PostDec_1;
    _functionMap[{"AddBy", 2}] = FKEY_AddBy_2;
    _functionMap[{"SubBy", 2}] = FKEY_SubBy_2;
    _functionMap[{"MulBy", 2}] = FKEY_MulBy_2;
    _functionMap[{"DivBy", 2}] = FKEY_DivBy_2;
    _functionMap[{"ModBy", 2}] = FKEY_ModBy_2;
    _functionMap[{"Neg", 1}] = FKEY_Neg_1;
    _functionMap[{"Block", -1}] = FKEY_Block_n;
    _functionMap[{"Call", 2}] = FKEY_Call_2;
    _functionMap[{"Pow", 2}] = FKEY_Pow_2;
    _functionMap[{"ScopedBlock", -1}] = FKEY_ScopedBlock_n;
    _functionMap[{"Ret", 0}] = FKEY_Ret_0;
    _functionMap[{"Ret", 1}] = FKEY_Ret_1;
    _functionMap[{"Break", 0}] = FKEY_Break_0;
    _functionMap[{"Continue", 0}] = FKEY_Continue_0;
    _functionMap[{"If", 2}] = FKEY_If_2;
    _functionMap[{"If", 3}] = FKEY_If_3;
    _functionMap[{"While", 2}] = FKEY_While_2;
    _functionMap[{"For", 4}] = FKEY_For_4;
    _functionMap[{"New", 1}] = FKEY_New_1;
    _functionMap[{"Class", 1}] = FKEY_Class_1;
    _functionMap[{"Idx", 2}] = FKEY_Idx_2;
    _functionMap[{"Dot", 2}] = FKEY_Dot_2;
    _functionMap[{"DotPut", 2}] = FKEY_DotPut_2;
    _functionMap[{"Cs", 1}] = FKEY_Cs_1;
    _functionMap[{"Call", 1}] = FKEY_Call_1;
    _functionMap[{"Def", 2}] = FKEY_Def_2;
    _functionMap[{"PushBack", 2}] = FKEY_PushBack_2;
    _functionMap[{"TouchMultimap", 1}] = FKEY_TouchMultimap_1;
    _functionMap[{"TouchList", 1}] = FKEY_TouchList_1;
    _functionMap[{"Keys", 1}] = FKEY_Keys_1;
    _functionMap[{"PushFront", 2}] = FKEY_PushFront_2;
    _functionMap[{"PopBack", 1}] = FKEY_PopBack_1;
    _functionMap[{"HasKey", 2}] = FKEY_HasKey_2;
    _functionMap[{"Insert", 3}] = FKEY_Insert_3;
    _functionMap[{"Clear", 1}] = FKEY_Clear_1;
    _functionMap[{"Empty", 1}] = FKEY_Empty_1;
    _functionMap[{"Back", 1}] = FKEY_Back_1;
    _functionMap[{"Erase", 2}] = FKEY_Erase_2;
    _functionMap[{"Merge", 2}] = FKEY_Merge_2;
    _functionMap[{"OuterMerge", 2}] = FKEY_OuterMerge_2;
    _functionMap[{"Switch", 3}] = FKEY_Switch_3;
    _functionMap[{"IsFalse", 1}] = FKEY_IsFalse_1;
    _functionMap[{"IsTrue", 1}] = FKEY_IsTrue_1;
    _functionMap[{"IsDefined", 1}] = FKEY_IsDefined_1;
    _functionMap[{"IsString", 1}] = FKEY_IsString_1;
    _functionMap[{"IsSymbol", 1}] = FKEY_IsSymbol_1;
    _functionMap[{"IsFunction", 1}] = FKEY_IsFunction_1;
    _functionMap[{"IsSymbolic", 1}] = FKEY_IsSymbolic_1;
    _functionMap[{"IsNumeric", 1}] = FKEY_IsNumeric_1;
    _functionMap[{"IsReference", 1}] = FKEY_IsReference_1;
    _functionMap[{"IsPointer", 1}] = FKEY_IsPointer_1;
    _functionMap[{"IsInteger", 1}] = FKEY_IsInteger_1;
    _functionMap[{"IsRational", 1}] = FKEY_IsRational_1;
    _functionMap[{"IsReal", 1}] = FKEY_IsReal_1;
    _functionMap[{"GetStr", 1}] = FKEY_GetStr_1;
    _functionMap[{"GetVec", 1}] = FKEY_GetVec_1;
    _functionMap[{"GetList", 1}] = FKEY_GetList_1;
    _functionMap[{"GetAnySequence", 1}] = FKEY_GetAnySequence_1;
    _functionMap[{"GetMap", 1}] = FKEY_GetMap_1;
    _functionMap[{"GetMultimap", 1}] = FKEY_GetMultimap_1;
    _functionMap[{"GetAnyMap", 1}] = FKEY_GetAnyMap_1;
    _functionMap[{"Append", 2}] = FKEY_Append_2;
    _functionMap[{"Normalize", 1}] = FKEY_Normalize_1;
    _functionMap[{"Head", 1}] = FKEY_Head_1;
    _functionMap[{"SetHead", 2}] = FKEY_SetHead_2;
    _functionMap[{"ClearHead", 1}] = FKEY_ClearHead_1;
    _functionMap[{"NumKeys", 1}] = FKEY_NumKeys_1;
    _functionMap[{"Size", 1}] = FKEY_Size_1;
    _functionMap[{"MapEmpty", 1}] = FKEY_MapEmpty_1;
    _functionMap[{"AllEmpty", 1}] = FKEY_AllEmpty_1;
    _functionMap[{"HasVector", 1}] = FKEY_HasVector_1;
    _functionMap[{"HasList", 1}] = FKEY_HasList_1;
    _functionMap[{"HasMap", 1}] = FKEY_HasMap_1;
    _functionMap[{"HasMultimap", 1}] = FKEY_HasMultimap_1;
    _functionMap[{"PopFront", 1}] = FKEY_PopFront_1;
    _functionMap[{"AllKeys", 1}] = FKEY_AllKeys_1;
    _functionMap[{"TouchVector", 1}] = FKEY_TouchVector_1;
    _functionMap[{"TouchMap", 1}] = FKEY_TouchMap_1;
    _functionMap[{"Open", 2}] = FKEY_Open_2;
    _functionMap[{"Save", 2}] = FKEY_Save_2;
    _functionMap[{"Unite", 2}] = FKEY_Unite_2;
    _functionMap[{"Unite", 3}] = FKEY_Unite_3;
    _functionMap[{"Intersect", 2}] = FKEY_Intersect_2;
    _functionMap[{"Intersect", 3}] = FKEY_Intersect_3;
    _functionMap[{"Complement", 1}] = FKEY_Complement_2;
    _functionMap[{"Import", 1}] = FKEY_Import_1;
  };
  
  class _FunctionMapLoader{
  public:
    _FunctionMapLoader(){
      _initSymbolMap();
      _initFunctionMap();
    }
  };
  
  static _FunctionMapLoader* _functionMapLoader = new _FunctionMapLoader;
  
} // end namespace

namespace neu{
  
  class NMLGenerator_{
  public:
    NMLGenerator_(NMLGenerator* o)
    : o_(o){
      
    }
    
    ~NMLGenerator_(){
      
    }
    
    void generate(ostream& ostr, const nvar& v){
      emitStatement(ostr, v, "");
    }
    
    FunctionKey getFunctionKey(const nvar& f){
      FunctionMap::const_iterator itr =
      _functionMap.find({f.str(), f.size()});
      
      if(itr == _functionMap.end()){
        itr = _functionMap.find({f.str(), -1});
      }
      
      if(itr == _functionMap.end()){
        return FKEY_NO_KEY;
      }
      
      return itr->second;
    }
    
    void emitStatement(ostream& ostr,
                       const nvar& n,
                       const nstr& indent){
      if(!n.isFunction()){
        emitExpression(ostr, n);
        return;
      }
      
      FunctionKey key = getFunctionKey(n);
      
      switch(key){
        case FKEY_ScopedBlock_n:
        case FKEY_Block_n:{
          for(size_t i = 0; i < n.size(); ++i){
            emitStatement(ostr, n[i], indent);
          }
          break;
        }
        case FKEY_Var_3:
        case FKEY_Var_2:{
          emitExpression(ostr, n[0]);
          ostr << " = ";
          emitExpression(ostr, n[1]);
          ostr << ";" << endl;
          break;
        }
        case FKEY_Ret_0:
          ostr << indent << "return;" << endl;
          break;
        case FKEY_Ret_1:
          ostr << indent << "return ";
          emitExpression(ostr, n[0]);
          ostr << ";" << endl;
          break;
        case FKEY_Break_0:
          ostr << indent << "break;" << endl;
          break;
        case FKEY_Continue_0:
          ostr << indent << "continue;" << endl;
          break;
        case FKEY_Def_2:
          ostr << indent;
          emitFunc(ostr, n[0]);
          ostr << "{" << endl;
          emitStatement(ostr, n[1], indent + "  ");
          ostr << indent << "}" << endl << endl;
          break;
        case FKEY_If_2:
        case FKEY_If_3:
          emitIf(ostr, n, indent);
          break;
        case FKEY_While_2:
          ostr << indent << "while(";
          emitExpression(ostr, n[0]);
          ostr << "){" << endl;
          emitStatement(ostr, n[1], indent + "  ");
          ostr << indent << "}" << endl;
          break;
        case FKEY_For_4:
          ostr << indent << "for(";
          emitExpression(ostr, n[0]);
          ostr << "; ";
          emitExpression(ostr, n[1]);
          ostr << "; ";
          emitExpression(ostr, n[2]);
          ostr << "){" << endl;
          emitStatement(ostr, n[3], indent + "  ");
          ostr << indent << "}" << endl;
          break;
        case FKEY_Class_1:{
          const nvar& c = n[0];
          
          ostr << indent << "class " << c["name"].str() << "{" << endl;

          nstr idt = indent;
          idt += "  ";
          
          if(c.hasKey("ctors")){
            const nmap& m = c["ctors"];
            for(auto& itr : m){
              const nvar& ctor = itr.second;
              ostr << idt;
              
              emitFunc(ostr, ctor[1]);
              
              ostr << " : ";
              emitFunc(ostr, ctor[0]);
              ostr << "{" << endl;
              
              emitStatement(ostr, ctor[2], idt + "  ");
              ostr << idt << "}" << endl;
            }
          
            ostr << endl;
          }
            
          emitStatement(ostr, c["stmts"], idt);
          ostr << indent << "}" << endl << endl;
          break;
        }
        case FKEY_Switch_3:{
          ostr << indent << "switch(";
          emitExpression(ostr, n[0]);
          ostr << "){" << endl;
          
          nstr idt = indent;
          idt += "  ";
          
          const nmap& m = n[2];
          for(auto& itr : m){
            const nvar& k = itr.first;
            const nvar& v = itr.second;
            
            ostr << idt << "case ";
            emitExpression(ostr, k);
            ostr << ":" << endl;
            emitStatement(ostr, v, idt + "  ");
          }
          
          const nvar& d = n[1];
          
          if(d.some()){
            ostr << idt << "default:" << endl;
            emitStatement(ostr, d, idt + "  ");
          }
          
          ostr << indent << "}" << endl;
          break;
        }
        case FKEY_Import_1:{
          ostr << "import " << n[0].str() << ";" << endl;
          break;
        }
        default:{
          ostr << indent;
          emitExpression(ostr, n);
          ostr << ";" << endl;
          break;
        }
      }
    }
    
    void emitIf(ostream& ostr,
                const nvar& n,
                const nstr& indent,
                bool nested=false){
      
      if(n.size() == 3){
        if(!nested){
          ostr << indent;
        }
        ostr << "if(";
        emitExpression(ostr, n[0]);
        ostr << "){" << endl;
        emitStatement(ostr, n[1], indent + "  ");
        ostr << indent << "}" << endl;
        const nvar& e = n[2];
        if(e.isFunction("If")){
          ostr << indent << "else ";
          emitIf(ostr, e, indent, true);
        }
        else{
          ostr << indent << "else{" << endl;
          emitStatement(ostr, e, indent + "  ");
          ostr << indent << "}" << endl;
        }
      }
      else if(n.size() == 2){
        if(!nested){
          ostr << indent;
        }
        ostr << "if(";
        emitExpression(ostr, n[0]);
        ostr << "){" << endl;
        emitStatement(ostr, n[1], indent + "  ");
        ostr << indent << "}" << endl;
      }
    }
    
    void emitBinOp(ostream& ostr,
                   const nvar& n,
                   const nstr& op,
                   int prec){
      int p = NObject::precedence(n);
      
      if(p > prec){
        ostr << "(";
      }
      
      emitExpression(ostr, n[0], p);
      ostr << op;
      emitExpression(ostr, n[1], p);
      
      if(p > prec){
        ostr << ")";
      }
    }
    
    void emitUnaryOp(ostream& ostr,
                     const nvar& n,
                     const nstr& op,
                     int prec){
      int p = NObject::precedence(n);
      
      ostr << op;
      
      if(p > prec){
        ostr << "(";
      }

      emitExpression(ostr, n[0], p);
      
      if(p > prec){
        ostr << ")";
      }
    }
    
    void emitPostUnaryOp(ostream& ostr,
                         const nvar& n,
                         const nstr& op,
                         int prec){
      int p = NObject::precedence(n);
      
      if(p > prec){
        ostr << "(";
      }
      
      emitExpression(ostr, n[0], p);
      
      if(p > prec){
        ostr << ")";
      }
      
      ostr << op;
    }
    
    void emitVarBuiltin(ostream& ostr,
                        const nvar& n,
                        const nstr& name=""){
      emitExpression(ostr, n[0]);
      ostr << ".";
      if(name.empty()){
        ostr << n.str().lowercase();
      }
      else{
        ostr << name;
      }
      
      ostr << "(";
      size_t size = n.size();
      for(size_t i = 1; i < size; ++i){
        if(i > 1){
          ostr << ", ";
        }
        emitExpression(ostr, n[i]);
      }
      ostr << ")";
    }
    
    void emitFunc(ostream& ostr,
                         const nvar& n){

      ostr << n.str() << "(";
      size_t size = n.size();
      for(size_t i = 0; i < size; ++i){
        if(i > 0){
          ostr << ", ";
        }
        emitExpression(ostr, n[i]);
      }
      ostr << ")";
    }
    
    void emitExpression(ostream& ostr,
                        const nvar& n,
                        int prec=100){
      switch(n.type()){
        case nvar::Function:
          break;
        default:
          ostr << n;
          return;
      }
      
      FunctionKey key = getFunctionKey(n);
      
      switch(key){
        case FKEY_NO_KEY:
          emitFunc(ostr, n);
          break;
        case FKEY_PushBack_2:
        case FKEY_TouchMultimap_1:
        case FKEY_TouchList_1:
        case FKEY_Keys_1:
        case FKEY_PushFront_2:
        case FKEY_PopBack_1:
        case FKEY_HasKey_2:
        case FKEY_Insert_3:
        case FKEY_Clear_1:
        case FKEY_Empty_1:
        case FKEY_Back_1:
        case FKEY_Erase_2:
        case FKEY_Merge_2:
        case FKEY_OuterMerge_2:
        case FKEY_IsFalse_1:
        case FKEY_IsTrue_1:
        case FKEY_IsDefined_1:
        case FKEY_IsString_1:
        case FKEY_IsSymbol_1:
        case FKEY_IsFunction_1:
        case FKEY_IsSymbolic_1:
        case FKEY_IsNumeric_1:
        case FKEY_IsReference_1:
        case FKEY_IsPointer_1:
        case FKEY_IsInteger_1:
        case FKEY_IsRational_1:
        case FKEY_IsReal_1:
        case FKEY_Append_2:
        case FKEY_Normalize_1:
        case FKEY_Head_1:
        case FKEY_SetHead_2:
        case FKEY_ClearHead_1:
        case FKEY_NumKeys_1:
        case FKEY_Size_1:
        case FKEY_MapEmpty_1:
        case FKEY_AllEmpty_1:
        case FKEY_HasVector_1:
        case FKEY_HasList_1:
        case FKEY_HasMap_1:
        case FKEY_HasMultimap_1:
        case FKEY_PopFront_1:
        case FKEY_AllKeys_1:
        case FKEY_TouchVector_1:
        case FKEY_TouchMap_1:
        case FKEY_Open_2:
        case FKEY_Save_2:
        case FKEY_Unite_2:
        case FKEY_Unite_3:
        case FKEY_Intersect_2:
        case FKEY_Intersect_3:
        case FKEY_Complement_2:
          emitVarBuiltin(ostr, n);
          break;
        case FKEY_GetStr_1:
          emitVarBuiltin(ostr, n, "str");
          break;
        case FKEY_GetVec_1:
          emitVarBuiltin(ostr, n, "vec");
          break;
        case FKEY_GetList_1:
          emitVarBuiltin(ostr, n, "list");
          break;
        case FKEY_GetAnySequence_1:
          emitVarBuiltin(ostr, n, "anySequence");
          break;
        case FKEY_GetMap_1:
          emitVarBuiltin(ostr, n, "map");
          break;
        case FKEY_GetMultimap_1:
          emitVarBuiltin(ostr, n, "multimap");
          break;
        case FKEY_GetAnyMap_1:
          emitVarBuiltin(ostr, n, "anyMap");
          break;
        case FKEY_New_1:
          ostr << "new ";
          emitExpression(ostr, n[0]);
          break;
        case FKEY_Idx_2:
          emitExpression(ostr, n[0]);
          ostr << "[";
          emitExpression(ostr, n[1]);
          ostr << "]";
          break;
        case FKEY_DotPut_2:
        case FKEY_Dot_2:
          emitExpression(ostr, n[0]);
          ostr << ".";
          emitExpression(ostr, n[1]);
          break;
        case FKEY_Cs_1:
          ostr << "{";
          emitExpression(ostr, n[0]);
          ostr << "}";
          break;
        case FKEY_Call_1:
          emitFunc(ostr, n[0]);
          break;
        case FKEY_Add_2:
          emitBinOp(ostr, n, " + ", prec);
          break;
        case FKEY_Sub_2:
          emitBinOp(ostr, n, " - ", prec);
          break;
        case FKEY_Mul_2:
          emitBinOp(ostr, n, "*", prec);
          break;
        case FKEY_Div_2:
          emitBinOp(ostr, n, "/", prec);
          break;
        case FKEY_Mod_2:
          emitBinOp(ostr, n, "%", prec);
          break;
        case FKEY_Pow_2:
          emitBinOp(ostr, n, "^", prec);
          break;
        case FKEY_VarSet_2:
          emitBinOp(ostr, n, " = ", prec);
          break;
        case FKEY_Set_2:
          emitBinOp(ostr, n, " = ", prec);
          break;
        case FKEY_LT_2:
          emitBinOp(ostr, n, " < ", prec);
          break;
        case FKEY_GT_2:
          emitBinOp(ostr, n, " > ", prec);
          break;
        case FKEY_LE_2:
          emitBinOp(ostr, n, " <= ", prec);
          break;
        case FKEY_GE_2:
          emitBinOp(ostr, n, " >= ", prec);
          break;
        case FKEY_EQ_2:
          emitBinOp(ostr, n, " == ", prec);
          break;
        case FKEY_NE_2:
          emitBinOp(ostr, n, " != ", prec);
          break;
        case FKEY_And_2:
          emitBinOp(ostr, n, " && ", prec);
          break;
        case FKEY_Or_2:
          emitBinOp(ostr, n, " || ", prec);
          break;
        case FKEY_AddBy_2:
          emitBinOp(ostr, n, " += ", prec);
          break;
        case FKEY_SubBy_2:
          emitBinOp(ostr, n, " -= ", prec);
          break;
        case FKEY_MulBy_2:
          emitBinOp(ostr, n, " *= ", prec);
          break;
        case FKEY_DivBy_2:
          emitBinOp(ostr, n, " /= ", prec);
          break;
        case FKEY_ModBy_2:
          emitBinOp(ostr, n, " %= ", prec);
          break;
        case FKEY_Not_1:
          emitUnaryOp(ostr, n, "!", prec);
          break;
        case FKEY_Neg_1:
          emitUnaryOp(ostr, n, "-", prec);
          break;
        case FKEY_Inc_1:
          emitUnaryOp(ostr, n, "++", prec);
          break;
        case FKEY_PostInc_1:
          emitPostUnaryOp(ostr, n, "++", prec);
          break;
        case FKEY_Dec_1:
          emitUnaryOp(ostr, n, "--", prec);
          break;
        case FKEY_PostDec_1:
          emitPostUnaryOp(ostr, n, "--", prec);
          break;
        case FKEY_Call_2:{
          emitExpression(ostr, n[0]);
          ostr << ".";

          const nvar& v1 = n[1];
          
          ostr << v1.str() << "(";
          for(size_t i = 0; i < v1.size(); ++i){
            if(i > 0){
              ostr << ", ";
            }
            emitExpression(ostr, v1[i]);
          }
          ostr << ")";
          break;
        }
        default:
          NERROR("function not implemented: " + n);
      }
    }
    
  private:
    NMLGenerator* o_;
  };
  
} // end namespace neu

NMLGenerator::NMLGenerator(){
  x_ = new NMLGenerator_(this);
}

NMLGenerator::~NMLGenerator(){
  delete x_;
}

void NMLGenerator::generate(std::ostream& ostr, const nvar& v){
  x_->generate(ostr, v);
}

nstr NMLGenerator::toStr(const nvar& v){
  NMLGenerator generator;
  
  stringstream ostr;
  generator.generate(ostr, v);
  
  return ostr.str();
}

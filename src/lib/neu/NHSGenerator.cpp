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

#include <neu/NHSGenerator.h>

#include <neu/NHSObject.h>
#include <neu/NBasicMutex.h>

using namespace std;
using namespace neu;

namespace{
  
  enum FunctionKey{
    FKEY_NO_KEY,
    FKEY_Tuple_n,
    FKEY_Block_n,
    FKEY_Eval_1,
    FKEY_Add_2,
    FKEY_Sub_2,
    FKEY_Mul_2,
    FKEY_Neg_1,
    FKEY_Div_2,
    FKEY_Pow_2,
    FKEY_Mod_2,
    FKEY_LT_2,
    FKEY_GT_2,
    FKEY_LE_2,
    FKEY_GE_2,
    FKEY_EQ_2,
    FKEY_NE_2,
    FKEY_Not_1,
    FKEY_And_2,
    FKEY_Or_2,
    FKEY_HConcat_2,
    FKEY_HCons_2,
    FKEY_Exp_1,
    FKEY_Sqrt_1,
    FKEY_Abs_1,
    FKEY_Ln_1,
    FKEY_Cos_1,
    FKEY_ACos_1,
    FKEY_CosH_1,
    FKEY_Sin_1,
    FKEY_ASin_1,
    FKEY_SinH_1,
    FKEY_Tan_1,
    FKEY_Cot_1,
    FKEY_ATan_1,
    FKEY_TanH_1,
    FKEY_Csc_1,
    FKEY_Sec_1,
    FKEY_SecH_1,
    FKEY_CscH_1,
    FKEY_ASec_1,
    FKEY_ACsc_1,
    FKEY_ASecH_1,
    FKEY_ACscH_1,
    FKEY_CotH_1,
    FKEY_ACotH_1,
    FKEY_ATanH_1,
    FKEY_ASinH_1,
    FKEY_ACosH_1,
    FKEY_Char_1,
    FKEY_HIdx_2,
    FKEY_HTo_n,
    FKEY_HType_2,
    FKEY_HType_3,
    FKEY_HTypeDef_2,
    FKEY_HTC_n,
    FKEY_HLC_n,
    FKEY_HRFunc_n,
    FKEY_HLFunc_2,
    FKEY_HFunc_2,
    FKEY_HLambda_2,
    FKEY_HGuardFunc_2,
    FKEY_HGuardFunc_3,
    FKEY_HSet_2,
    FKEY_HCall_1,
    FKEY_HTopLet_2,
    FKEY_HLet_2,
    FKEY_HWhere_2,
    FKEY_HCase_2,
    FKEY_HIf_3,
    FKEY_HInfList_1,
    FKEY_HInfList_2,
    FKEY_HRangeList_2,
    FKEY_HRangeList_3,
    FKEY_HAny_0,
    FKEY_HPA_2,
    FKEY_HDollar_2,
    FKEY_HCompose_2,
    FKEY_HImport_1,
    FKEY_HImport_2,
    FKEY_HImportHiding_2,
    FKEY_HImportQualified_1,
    FKEY_HImportQualified_2,
    FKEY_HData_2,
    FKEY_HData_3,
    FKEY_HDataParam_n,
    FKEY_HSetRecord_2,
    FKEY_HParamType_n,
    FKEY_HClass_2,
    FKEY_HInstance_3,
    FKEY_HOpFunc_1,
    FKEY_HDo_n,
    FKEY_HOut_n
  };  

  class Global{
  public:
    Global(){
      precedenceMap_["!!"] = 0;    
      precedenceMap_["."] = 0;
      precedenceMap_["^"] = 1;
      precedenceMap_["^^"] = 1;
      precedenceMap_["**"] = 1;
      precedenceMap_["*"] = 2;
      precedenceMap_["/"] = 2;
      precedenceMap_["div"] = 2;
      precedenceMap_["mod"] = 2;
      precedenceMap_["rem"] = 2;
      precedenceMap_["quot"] = 2;
      precedenceMap_["+"] = 3;
      precedenceMap_["-"] = 3;
      precedenceMap_[":"] = 4;
      precedenceMap_["++"] = 4;
      precedenceMap_["=="] = 5;
      precedenceMap_["/="] = 5;
      precedenceMap_["<"] = 5;
      precedenceMap_["<="] = 5;
      precedenceMap_[">"] = 5;
      precedenceMap_[">="] = 5;
      precedenceMap_["elem"] = 5;
      precedenceMap_["notElem"] = 5;
      precedenceMap_["&&"] = 6;
      precedenceMap_["||"] = 7;
      precedenceMap_[">>"] = 8;
      precedenceMap_[">>="] = 8;
      precedenceMap_["$"] = 9;
      precedenceMap_["$!"] = 9;
      precedenceMap_["seq"] = 9;

      functionMap_[{"Tuple", -1}] = 
        {FKEY_Tuple_n, NHSGenerator::Supported};

      functionMap_[{"Block", -1}] = 
        {FKEY_Block_n, NHSGenerator::Supported};

      functionMap_[{"Eval", 1}] =
      {FKEY_Eval_1, NHSGenerator::Supported};
      
      functionMap_[{"Add", 2}] = 
        {FKEY_Add_2, NHSGenerator::Supported};
    
      functionMap_[{"Sub", 2}] = 
        {FKEY_Sub_2, NHSGenerator::Supported};
    
      functionMap_[{"Mul", 2}] = 
        {FKEY_Mul_2, NHSGenerator::Supported};
    
      functionMap_[{"Div", 2}] = 
        {FKEY_Div_2, NHSGenerator::Supported};

      functionMap_[{"Neg", 1}] = 
        {FKEY_Neg_1, NHSGenerator::Supported};
    
      functionMap_[{"Pow", 2}] = 
        {FKEY_Pow_2, NHSGenerator::Supported};
    
      functionMap_[{"Mod", 2}] = 
        {FKEY_Mod_2, NHSGenerator::Supported};
    
      functionMap_[{"LT", 2}] = 
        {FKEY_LT_2, NHSGenerator::Supported};
    
      functionMap_[{"GT", 2}] = 
        {FKEY_GT_2, NHSGenerator::Supported};
    
      functionMap_[{"LE", 2}] = 
        {FKEY_LE_2, NHSGenerator::Supported};
    
      functionMap_[{"GE", 2}] = 
        {FKEY_GE_2, NHSGenerator::Supported};
    
      functionMap_[{"EQ", 2}] =
        {FKEY_EQ_2, NHSGenerator::Supported};
    
      functionMap_[{"NE", 2}] = 
        {FKEY_NE_2, NHSGenerator::Supported};
    
      functionMap_[{"Not", 1}] = 
        {FKEY_Not_1, NHSGenerator::Supported};
    
      functionMap_[{"And", 2}] = 
        {FKEY_And_2, NHSGenerator::Supported};
    
      functionMap_[{"Or", 2}] = 
        {FKEY_Or_2, NHSGenerator::Supported};

      functionMap_[{"HConcat", 2}] =
        {FKEY_HConcat_2, NHSGenerator::Requested};

      functionMap_[{"HCons", 2}] =
        {FKEY_HCons_2, NHSGenerator::Requested};
    
      functionMap_[{"Exp", 1}] = 
        {FKEY_Exp_1, NHSGenerator::Supported};
    
      functionMap_[{"Sqrt", 1}] = 
        {FKEY_Sqrt_1, NHSGenerator::Supported};
    
      functionMap_[{"Abs", 1}] = 
        {FKEY_Abs_1, NHSGenerator::Supported};
    
      functionMap_[{"Ln", 1}] = 
        {FKEY_Ln_1, NHSGenerator::Supported};
    
      functionMap_[{"Cos", 1}] = 
        {FKEY_Cos_1, NHSGenerator::Supported};
    
      functionMap_[{"ACos", 1}] = 
        {FKEY_ACos_1, NHSGenerator::Supported};
    
      functionMap_[{"CosH", 1}] = 
        {FKEY_CosH_1, NHSGenerator::Supported};
    
      functionMap_[{"Sin", 1}] = 
        {FKEY_Sin_1, NHSGenerator::Supported};
    
      functionMap_[{"ASin", 1}] = 
        {FKEY_ASin_1, NHSGenerator::Supported};
    
      functionMap_[{"SinH", 1}] = 
        {FKEY_SinH_1, NHSGenerator::Supported};
    
      functionMap_[{"Tan", 1}] = 
        {FKEY_Tan_1, NHSGenerator::Supported};

      functionMap_[{"Cot", 1}] = 
        {FKEY_Cot_1, NHSGenerator::Supported};
    
      functionMap_[{"ATan", 1}] = 
        {FKEY_ATan_1, NHSGenerator::Supported};
    
      functionMap_[{"TanH", 1}] = 
        {FKEY_TanH_1, NHSGenerator::Supported};
    
      functionMap_[{"Sec", 1}] = 
        {FKEY_Sec_1, NHSGenerator::Supported};

      functionMap_[{"Csc", 1}] = 
        {FKEY_Csc_1, NHSGenerator::Supported};
    
      functionMap_[{"SecH", 1}] = 
        {FKEY_SecH_1, NHSGenerator::Supported};
    
      functionMap_[{"CscH", 1}] = 
        {FKEY_CscH_1, NHSGenerator::Supported};

      functionMap_[{"ASec", 1}] = 
        {FKEY_ASec_1, NHSGenerator::Supported};
    
      functionMap_[{"ACsc", 1}] = 
        {FKEY_ACsc_1, NHSGenerator::Supported};
    
      functionMap_[{"ASecH", 1}] = 
        {FKEY_ASecH_1, NHSGenerator::Supported};
    
      functionMap_[{"ACscH", 1}] = 
        {FKEY_ACscH_1, NHSGenerator::Supported};
    
      functionMap_[{"CotH", 1}] = 
        {FKEY_CotH_1, NHSGenerator::Supported};
    
      functionMap_[{"ATanH", 1}] = 
        {FKEY_ATanH_1, NHSGenerator::Supported};
    
      functionMap_[{"ASinH", 1}] = 
        {FKEY_ASinH_1, NHSGenerator::Supported};
    
      functionMap_[{"ACosH", 1}] = 
        {FKEY_ACosH_1, NHSGenerator::Supported};

      functionMap_[{"Char", 1}] = 
        {FKEY_Char_1, NHSGenerator::Supported};
    
      functionMap_[{"HIdx", 2}] =
        {FKEY_HIdx_2, NHSGenerator::Requested};

      functionMap_[{"HTo", -1}] = 
        {FKEY_HTo_n, NHSGenerator::Requested};

      functionMap_[{"HTC", -1}] = 
        {FKEY_HTC_n, NHSGenerator::Requested};

      functionMap_[{"HLC", -1}] = 
        {FKEY_HLC_n, NHSGenerator::Requested};

      functionMap_[{"HRFunc", -1}] = 
        {FKEY_HRFunc_n, NHSGenerator::Requested};

      functionMap_[{"HLFunc", 2}] = 
        {FKEY_HLFunc_2, NHSGenerator::Requested};

      functionMap_[{"HType", 2}] = 
        {FKEY_HType_2, NHSGenerator::Requested};
    
      functionMap_[{"HType", 3}] = 
        {FKEY_HType_3, NHSGenerator::Requested};

      functionMap_[{"HTypeDef", 2}] = 
        {FKEY_HTypeDef_2, NHSGenerator::Requested};

      functionMap_[{"HFunc", 2}] = 
        {FKEY_HFunc_2, NHSGenerator::Requested};

      functionMap_[{"HLambda", 2}] = 
        {FKEY_HLambda_2, NHSGenerator::Requested};

      functionMap_[{"HGuardFunc", 2}] = 
        {FKEY_HGuardFunc_2, NHSGenerator::Requested};

      functionMap_[{"HGuardFunc", 3}] = 
        {FKEY_HGuardFunc_3, NHSGenerator::Requested};

      functionMap_[{"HSet", 2}] = 
        {FKEY_HSet_2, NHSGenerator::Requested};

      functionMap_[{"HCall", 1}] = 
        {FKEY_HCall_1, NHSGenerator::Requested};

      functionMap_[{"HLet", 2}] = 
        {FKEY_HLet_2, NHSGenerator::Requested};

      functionMap_[{"HTopLet", 2}] = 
        {FKEY_HTopLet_2, NHSGenerator::Requested};

      functionMap_[{"HWhere", 2}] = 
        {FKEY_HWhere_2, NHSGenerator::Requested};

      functionMap_[{"HCase", 2}] = 
        {FKEY_HCase_2, NHSGenerator::Requested};

      functionMap_[{"HIf", 3}] = 
        {FKEY_HIf_3, NHSGenerator::Requested};

      functionMap_[{"HInfList", 1}] = 
        {FKEY_HInfList_1, NHSGenerator::Requested};

      functionMap_[{"HInfList", 2}] = 
        {FKEY_HInfList_2, NHSGenerator::Requested};

      functionMap_[{"HRangeList", 2}] = 
        {FKEY_HRangeList_2, NHSGenerator::Requested};

      functionMap_[{"HRangeList", 3}] = 
        {FKEY_HRangeList_3, NHSGenerator::Requested};

      functionMap_[{"HAny", 0}] = 
        {FKEY_HAny_0, NHSGenerator::Requested};

      functionMap_[{"HPA", 2}] = 
        {FKEY_HPA_2, NHSGenerator::Requested};

      functionMap_[{"HDollar", 2}] = 
        {FKEY_HDollar_2, NHSGenerator::Requested};

      functionMap_[{"HCompose", 2}] = 
        {FKEY_HCompose_2, NHSGenerator::Requested};

      functionMap_[{"HImport", 1}] = 
        {FKEY_HImport_1, NHSGenerator::Requested};

      functionMap_[{"HImport", 2}] = 
        {FKEY_HImport_2, NHSGenerator::Requested};

      functionMap_[{"HImportHiding", 2}] = 
        {FKEY_HImportHiding_2, NHSGenerator::Requested};

      functionMap_[{"HImportQualified", 1}] = 
        {FKEY_HImportQualified_1, NHSGenerator::Requested};

      functionMap_[{"HImportQualified", 2}] = 
        {FKEY_HImportQualified_2, NHSGenerator::Requested};

      functionMap_[{"HData", 2}] = 
        {FKEY_HData_2, NHSGenerator::Requested};

      functionMap_[{"HData", 3}] = 
        {FKEY_HData_3, NHSGenerator::Requested};

      functionMap_[{"HDataParam", -1}] = 
        {FKEY_HDataParam_n, NHSGenerator::Requested};

      functionMap_[{"HSetRecord", 2}] = 
        {FKEY_HSetRecord_2, NHSGenerator::Requested};

      functionMap_[{"HParamType", -1}] = 
        {FKEY_HParamType_n, NHSGenerator::Requested};

      functionMap_[{"HClass", 2}] = 
        {FKEY_HClass_2, NHSGenerator::Requested};

      functionMap_[{"HInstance", 3}] = 
        {FKEY_HInstance_3, NHSGenerator::Requested};

      functionMap_[{"HOpFunc", 1}] = 
        {FKEY_HOpFunc_1, NHSGenerator::Requested};

      functionMap_[{"HDo", -1}] = 
        {FKEY_HDo_n, NHSGenerator::Requested};

      functionMap_[{"HOut", -1}] = 
        {FKEY_HOut_n, NHSGenerator::Requested};
    }

    FunctionKey getFunctionKey(const nstr& func, size_t size){
      auto itr = functionMap_.find({func, size});
      if(itr == functionMap_.end()){
        itr = functionMap_.find({func, -1});
      }
      
      return itr == functionMap_.end() ? FKEY_NO_KEY : itr->second.first;
    }
    
    NHSGenerator::Type type(const nvar& f){
      auto itr = functionMap_.find({f.str(), f.size()});
      
      if(itr == functionMap_.end()){
        itr = functionMap_.find({f.str(), -1});
      }
      
      if(itr == functionMap_.end()){
        return 0;
      }
      
      return itr->second.second;
    }
    
    size_t precedence(const nstr& op){
      auto itr = precedenceMap_.find(op);
      assert(itr != precedenceMap_.end() && "unknown operator");

      return itr->second;
    }

  private:
    typedef NMap<pair<nstr, int8_t>,
    pair<FunctionKey, NHSGenerator::Type>> FunctionMap_;
    typedef NMap<nstr, uint8_t> PrecedenceMap_;

    FunctionMap_ functionMap_;
    PrecedenceMap_ precedenceMap_;
  };

  Global* _global = 0;
  NBasicMutex _mutex;

} // end namespace

namespace neu{
    
  class NHSGenerator_{
  public:
    typedef NMap<nstr, bool> NameMap_;
    
    NHSGenerator_(NHSGenerator* o, NHSObject* obj)
    : o_(o),
      obj_(obj){
        initGlobal();
    }
    
    ~NHSGenerator_(){}

    void initGlobal(){
      _mutex.lock();
      if(!_global){
        _global = new Global;
      }
      _mutex.unlock();
    }
    
    void generate(ostream& ostr, const nvar& v){
      emitExpression(ostr, v);
    }
    
    int precedence(const nstr& op){
      initGlobal();
      return _global->precedence(op);
    }
    
    void error(const nvar& n, const nstr& msg, bool warn=false){
      cerr << "NHS generator error: ";
      
      nstr loc = n.getLocation();
      
      if(!loc.empty()){
        cerr << loc << ": ";
      }
      
      cerr << msg << ": " << n.toStr() << endl;
    }

    void emitCallParams(ostream& ostr, const nvar& n){
      bool first = true;

      for(const nvar& ni : n){
        if(!first){
          ostr << " ";
        }

        if(ni.isFunction()){
          if(ni.str() == "HRFunc" || ni.str() == "HLFunc"){
            emitExpression(ostr, ni);
          }
          else{
            ostr << "(";
            emitExpression(ostr, ni);
            ostr << ")";
          }
        }
        else{
          emitExpression(ostr, ni);
        }
        first = false;
      }
    }

    void emitParams(ostream& ostr, const nvar& n){
      bool first = true;

      for(const nvar& ni : n){
        if(!first){
          ostr << " ";
        }

        emitParam(ostr, ni);

        first = false;
      }
    }
    
    void emitParam(ostream& ostr, const nvar& n){
      if(n.isFunction("HCons")){
        ostr << "(";
        emitExpression(ostr, n);
        ostr << ")";
      }
      else{
        emitExpression(ostr, n);
      }
    }

    void emitCall(ostream& ostr, const nvar& n){
      ostr << n.str() << " ";
      emitCallParams(ostr, n);
    }

    void emitBinOp(ostream& ostr,
                   const nvar& n,
                   const nstr& op,
                   int prec){
      
      nstr normOp = op;
      normOp.strip();
      
      int p = _global->precedence(normOp);
      
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
    
    void emitExpression(ostream& ostr, const nvar& n, int prec=100){
      nvar::Type type = n.type();

      switch(type){
        case nvar::Integer:
        case nvar::Rational:
        case nvar::Float:
        case nvar::Symbol:
        case nvar::String:
        case nvar::StringPointer:
          ostr << n;
          return;
        case nvar::False:
          ostr << "False";
          return;
        case nvar::True:
          ostr << "True";
          return;
        case nvar::Real:
          ostr << n.real().toStr(false);
          return;
        case nvar::Vector:
        case nvar::List:
        case nvar::Queue:{
          ostr << "[";
        
          size_t size = n.size();
          for(size_t i = 0; i < size; ++i){
            if(i > 0){
              ostr << ", ";
            }
            
            emitExpression(ostr, n[i]);
          }
          ostr << "]";
          return;
        }
        case nvar::Function:
          break;
        case nvar::Pointer:
        case nvar::Reference:
          emitExpression(ostr, *n);
          break;
        default:
          NERROR("invalid expression: " + n);
      }

      size_t size = n.size();

      FunctionKey key = _global->getFunctionKey(n.str(), size);
      if(key == FKEY_NO_KEY){
        NERROR("invalid function: " + n);
      }
      
      switch(key){
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
          emitBinOp(ostr, n, " /= ", prec);
          break;
        case FKEY_And_2:
          emitBinOp(ostr, n, " && ", prec);
          break;
        case FKEY_Or_2:
          emitBinOp(ostr, n, " || ", prec);
          break;
        case FKEY_HConcat_2:
          emitBinOp(ostr, n, " ++ ", prec);
          break;
        case FKEY_HCons_2:
          emitBinOp(ostr, n, ":", prec);
          break;
        case FKEY_HIdx_2:
          emitBinOp(ostr, n, " !! ", prec);
          break;
        case FKEY_HDollar_2:
          emitBinOp(ostr, n, " $ ", prec);
          break;
        case FKEY_HCompose_2:
          emitBinOp(ostr, n, " . ", prec);
          break;
        case FKEY_Eval_1:
          emitExpression(ostr, n[0]);
          break;
        case FKEY_Neg_1:
          if(n[0].isNumeric()){
            emitExpression(ostr, -n[0]);
          }
          else{
            ostr << "(-";
            emitExpression(ostr, n[0]);
            ostr << ")";
          }
          break;
        case FKEY_Pow_2:{
          int p = precedence("**");
          
          if(p > prec){
            ostr << "(";
          }

          emitExpression(ostr, n[0], p);

          if(n[1].isInteger()){
            ostr << "^";
          }
          else{
            ostr << "**";
          }

          emitExpression(ostr, n[1], p);
          
          if(p > prec){
            ostr << ")";
          }
          
          break;
        }
        case FKEY_Mod_2:
        {
          int p = precedence("mod");
          
          if(p > prec){
            ostr << "(";
          }
          
          emitExpression(ostr, n[0], p);
          ostr << " `mod` ";
          emitExpression(ostr, n[1], p);

          if(p > prec){
            ostr << ")";
          }
          break;
        }
        case FKEY_Char_1:{
          const nvar& c = n[0];
          
          if(!c.isString()){
            error(c, "invalid char");
          }

          const nstr& cs = c.str();

          if(cs.length() != 1){
            error(c, "invalid char");
          }

          ostr << "'" << cs << "'";
          break;
        }
        case FKEY_HTo_n:{
          bool first = true;

          for(const nvar& ni : n){
            if(!first){
              ostr << " -> ";
            }

            if(ni.isFunction("HTo")){
              ostr << "(";
              emitExpression(ostr, ni);
              ostr << ")";
            }
            else{
              emitExpression(ostr, ni);
            }

            first = false;
          }
          break;
        }
        case FKEY_HTC_n:{
          if(n.empty()){
            break;
          }

          ostr << "(";
          bool first = true;
          for(const nvar& ni : n){
            if(!ni.isFunction(1) || !ni[0].isSymbol()){
              error(ni, "invalid type class");
            }
            
            if(!first){
              ostr << ", ";
            }
            
            ostr << ni.str() << " " << ni[0];
            
            first = false;
          }
          ostr << ") => ";          
          break;
        }
        case FKEY_HLC_n:{
          if(n.size() < 2){
            error(n, "invalid list comprehension");
          }
          ostr << "[";
          emitExpression(ostr, n[0]);
          ostr << " | ";
          for(size_t i = 1; i < n.size(); ++i){
            if(i > 1){
              ostr << ", ";
            }
            emitExpression(ostr, n[i]);
          }
          ostr << "]";
          break;
        }
        case FKEY_HRFunc_n:{
          if(n.size() < 1){
            error(n, "invalid function");
          }

          const nvar& op = n[0];

          if(!op.isString()){
            error(n, "invalid function");
          }

          ostr << "(";
          
          const nstr& s = op;

          if(nstr::isAlpha(s[0])){
            ostr << "`" << s << "`";
          }
          else{
            ostr << s;
          }

          for(size_t i = 1; i < n.size(); ++i){
            ostr << " ";
            emitExpression(ostr, n[i]);
          }
          ostr << ")";
          break;
        }
        case FKEY_HLFunc_2:{
          ostr << "(";
          emitExpression(ostr, n[0]);

          const nvar& op = n[1];

          if(!op.isString()){
            error(n, "invalid function");
          }

          const nstr& s = op;

          if(nstr::isAlpha(s[0])){
            ostr << "`" << s << "`";
          }
          else{
            ostr << s;
          }

          ostr << ")";

          break;
        }
        case FKEY_HType_2:
          emitExpression(ostr, n[0]);
          ostr << " :: ";
          emitExpression(ostr, n[1]);
          break;
        case FKEY_HType_3:
          emitExpression(ostr, n[0]);
          ostr << " :: ";
          emitExpression(ostr, n[1]);
          emitExpression(ostr, n[2]);
          break;
        case FKEY_HTypeDef_2:{
          ostr << "type";

          const nvar& s = n[0];
                    
          for(const nvar& si : s){
            if(!si.isSymbol()){
              error(si, "invalid type definition");
              break;
            }
            ostr << " " << si;
          }

          ostr << " = ";

          emitExpression(ostr, n[1]);

          break;
        }
        case FKEY_HFunc_2:{
          const nvar& f = n[0];

          const nstr& fs = f.str();

          if(nstr::isUpper(fs[0])){
            emitExpression(ostr, f);
          }
          else{
            ostr << f.str() << " ";
            
            emitParams(ostr, f);
          }

          ostr << " = ";
          emitExpression(ostr, n[1]);

          break;
        }
        case FKEY_HLambda_2:{
          const nvar& f = n[0];

          ostr << "\\";

          emitParams(ostr, f);

          ostr << " -> ";
          emitExpression(ostr, n[1]);

          break;
        }
        case FKEY_HGuardFunc_2:
        case FKEY_HGuardFunc_3:{
          const nvar& f = n[0];

          ostr << f.str() << " ";

          emitParams(ostr, f);

          const nvar& g = n[1];
          
          for(const nvar& gi : g){
            ostr << endl << "  | ";
            emitExpression(ostr, gi[0]);
            ostr << " = ";
            emitExpression(ostr, gi[1]);
          }

          if(key != FKEY_HGuardFunc_3){
            break;
          }

          ostr << endl << "  where ";
          const nvar& w = n[2];

          bool first = true;

          for(const nvar& wi : w){
            if(first){
              first = false;
            }
            else{
              ostr << "        ";
            }

            if(wi.isFunction("HFunc")){
              emitExpression(ostr, wi);
            }
            else{
              emitParam(ostr, wi[0]);
              ostr << " = ";
              emitExpression(ostr, wi[1]);              
            }

            ostr << endl;
          }

          break;
        }
        case FKEY_HSet_2:{
          const nvar& s = n[0];

          if(!s.isSymbol() && !s.isFunction("HAny", 0)){
            error(s, "expected a symbol or _");
          }

          emitExpression(ostr, s);
          ostr << " <- ";
          emitExpression(ostr, n[1]);
          break;
        }
        case FKEY_HCall_1:
          emitCall(ostr, n[0]);
          break;
        case FKEY_HWhere_2:{
          emitExpression(ostr, n[0]);

          ostr << endl << "  where ";
          const nvar& w = n[1];

          bool first = true;

          for(const nvar& wi : w){
            if(first){
              first = false;
            }
            else{
              ostr << "        ";
            }

            if(wi.isFunction("HFunc")){
              emitExpression(ostr, wi);
            }
            else{
              emitParam(ostr, wi[0]);
              ostr << " = ";
              emitExpression(ostr, wi[1]);
            }

            ostr << endl;
          }

          break;
        }
        case FKEY_HCase_2:{
          ostr << "case ";
          emitExpression(ostr, n[0]);
          ostr << " of ";
          ostr << endl;

          const nvar& c = n[1];
          for(const nvar& ci : c){
            ostr << "  ";
            emitParam(ostr, ci[0]);
            ostr << " -> ";
            emitExpression(ostr, ci[1]);
            ostr << endl;
          }
          break;
        }
        case FKEY_HTopLet_2:{
          const nvar& s = n[0];

          if(!s.isSymbol()){
            error(s, "invalid let expression");
            break;
          }

          ostr << "let " << s << " = ";
          emitExpression(ostr, n[1]);

          break;
        }
        case FKEY_HLet_2:{
          const nvar& w = n[0];

          if(!w.isFunction()){
            error(w, "invalid let expression");
            break;
          }

          bool multiline  = w.size() > 1;

          if(multiline){
            ostr << endl << "  ";
          }

          ostr << "let ";

          bool first = true;
          
          for(const nvar& wi : w){
            if(first){
              first = false;
            }
            else if(multiline){
              ostr << "      ";
            }

            if(wi.isFunction("HFunc")){
              emitExpression(ostr, wi);
            }
            else{
              emitParam(ostr, wi[0]);
              ostr << " = ";
              emitExpression(ostr, wi[1]);
            }
            
            if(multiline){
              ostr << endl;
            }
          }

          if(multiline){
            ostr << "  ";
          }
          else{
            ostr << " ";
          }
          
          ostr << "in ";
          emitExpression(ostr, n[1]);

          if(multiline){
            ostr << endl;
          }

          break;
        }
        case FKEY_HIf_3:
          ostr << "if ";
          emitExpression(ostr, n[0]);
          ostr << " then ";
          emitExpression(ostr, n[1]);
          ostr << " else ";
          emitExpression(ostr, n[2]);
          break;
        case FKEY_HInfList_1:
          ostr << "[";
          emitExpression(ostr, n[0]);
          ostr << "..";
          ostr << "]";
          break;
        case FKEY_HInfList_2:
          ostr << "[";
          emitExpression(ostr, n[0]);
          ostr << ",";
          emitExpression(ostr, n[1]);
          ostr << "..";
          ostr << "]";
          break;
        case FKEY_HRangeList_2:
          ostr << "[";
          emitExpression(ostr, n[0]);
          ostr << "..";
          emitExpression(ostr, n[1]);
          ostr << "]";
          break;
        case FKEY_HRangeList_3:
          ostr << "[";
          emitExpression(ostr, n[0]);
          ostr << ", ";
          emitExpression(ostr, n[1]);
          ostr << "..";
          emitExpression(ostr, n[2]);
          ostr << "]";
          break;
        case FKEY_HAny_0:
          ostr << "_";
          break;
        case FKEY_HPA_2:{
          const nvar& s = n[0];
          
          if(!s.isSymbol()){
            error(s, "expected a symbol");
          }

          ostr << s << "@(";
          emitExpression(ostr, n[1]);
          ostr << ")";
          break;
        }
        case FKEY_HImport_1:{
          const nvar& s = n[0];

          if(!s.isSymbol()){
            error(s, "invalid import");
            break;
          }

          ostr << "import " << s;
          break;
        }
        case FKEY_HImportHiding_2:
        case FKEY_HImport_2:{
          const nvar& s = n[0];

          if(!s.isSymbol()){
            error(s, "invalid import");
            break;
          }

          ostr << "import " << s << " ";
          
          if(key == FKEY_HImportHiding_2){
            ostr << "hiding ";
          }

          ostr << "(";

          const nvar& f = n[1];

          bool first = true;
          for(const nvar& fi : f){
            if(first){
              first = false;
            }
            else{
              ostr << ", ";
            }
            ostr << fi;
          }

          ostr << ")";

          break;
        }
        case FKEY_HImportQualified_1:
        case FKEY_HImportQualified_2:{
          const nvar& s = n[0];

          if(!s.isSymbol()){
            error(s, "invalid import");
            break;
          }

          ostr << "import qualified " << s;

          if(key == FKEY_HImportQualified_2){
            const nvar& as = n[0];
            
            if(!as.isSymbol()){
              error(s, "invalid import");
              break;
            }

            ostr << " as " << as;
          }

          break;
        }
        case FKEY_HData_2:
        case FKEY_HData_3:{
          ostr << "data";

          const nvar& s = n[0];

          for(const nvar& si : s){
            if(!si.isSymbol()){
              error(si, "invalid data definition");
              break;
            }
            ostr << " " << si;
          }

          ostr << " =";

          const nvar& d = n[1];
          
          bool first = true;
          for(const nvar& di : d){
            if(first){
              first = false;
            }
            else{
              ostr << endl << "  |";
            }

            if(di.isFunction("HRecord", 2)){
              const nvar& dt = di[0];
              for(const nvar& dj : dt){
                if(!dj.isSymbol()){
                  error(dj, "invalid data definition");
                  break;
                }
                ostr << " " << dj;
              }
              ostr << " {";

              const nvar& rs = di[1];

              first = true;
              for(const nvar& rj : rs){
                if(!rj.isFunction("HType", 2)){
                  error(rj, "invalid data definition");
                  break;
                }

                if(first){
                  first = false;
                }
                else{
                  ostr << ", ";
                }

                emitExpression(ostr, rj);
              }

              ostr << "}";
            }
            else{
              for(const nvar& dj : di){
                if(!dj.isSymbol()){
                  error(dj, "invalid data definition");
                  break;
                }
                ostr << " " << dj;
              }              
            }
          }

          if(key == FKEY_HData_3){
            ostr << " deriving (";
            const nvar& v = n[2];
            bool first = true;
            for(const nvar& vi : v){
              if(first){
                first = false;
              }
              else{
                ostr << ",";
              }
              
              ostr << vi;
            }
            ostr << ")";
          }

          break;
        }
        case FKEY_HSetRecord_2:{
          const nvar& s = n[0];

          if(!s.isSymbol()){
            error(s, "invalid set record");
            break;
          }
          
          ostr << s << " {";

          const nvar& rs = n[1];

          bool first = true;
          for(const nvar& ri : rs){
            if(!ri.isFunction(2)){
              error(ri, "invalid set record");
              break;
            }

            if(first){
              first = false;
            }
            else{
              ostr << ", ";
            }

            const nvar& si = ri[0];

            if(!si.isSymbol()){
              error(si, "invalid set record");
            }

            ostr << si << "=";

            emitExpression(ostr, ri[1]);
          }

          ostr << "}";

          break;
        }
        case FKEY_HClass_2:{
          ostr << "class";

          const nvar& s = n[0];

          for(const nvar& si : s){
            if(!si.isSymbol()){
              error(si, "invalid class definition");
              break;
            }
            ostr << " " << si;
          }

          ostr << " where " << endl;

          const nvar& d = n[1];

          for(const nvar& di : d){
            ostr << "  ";
            emitExpression(ostr, di);
            ostr << endl;
          }

          break;
        }
        case FKEY_HInstance_3:{
          ostr << "instance ";

          const nvar& c = n[0];

          if(!c.isSymbol()){
            error(c, "expected a symbol");
          }

          ostr << c << " ";

          const nvar& i = n[1];

          if(!i.isSymbol()){
            error(i, "expected a symbol");
          }          

          ostr << i << " where " << endl;

          const nvar& d = n[2];

          for(const nvar& di : d){
            ostr << "  ";
            emitExpression(ostr, di);
            ostr << endl;
          }

          break;
        }
        case FKEY_HOpFunc_1:
          ostr << "(" << n[0].str() << ")";
          break;
        case FKEY_HDataParam_n:
          ostr << "(";
          emitParams(ostr, n);
          ostr << ")";
          break;
        case FKEY_HParamType_n:
          emitParams(ostr, n);
          break;
        case FKEY_Tuple_n:{
          ostr << "(";
          bool first = true;
          for(const nvar& ni : n){
            if(first){
              first = false;
            }
            else{
              ostr << ",";
            }
            emitExpression(ostr, ni);
          }
          ostr << ")";
          break;
        }
        case FKEY_Block_n:
          for(const nvar& ni : n){
            emitExpression(ostr, ni);
            ostr << endl;
          }
          break;
        case FKEY_HDo_n:
          ostr << "do" << endl;
          for(const nvar& ni : n){
            ostr << "  ";
            emitExpression(ostr, ni);
            ostr << endl;
          }
          break;
        case FKEY_HOut_n:
          for(const nvar& ni : n){
            emitExpression(ostr, ni);
            ostr << endl;
          }
          break;
        default:{
          const nstr& fs = n.str();
          if(nstr::isUpper(fs[0])){
            NERROR("function not implemented: " + n);
          }

          emitCall(ostr, n);
          break;
        }
      }
    }
    
  private:
    NHSGenerator* o_;
    NHSObject* obj_;
  };
  
} // end namespace neu

const NHSGenerator::Type NHSGenerator::Supported;
const NHSGenerator::Type NHSGenerator::Requested;

NHSGenerator::NHSGenerator(NHSObject* obj){
  x_ = new NHSGenerator_(this, obj);
}

NHSGenerator::~NHSGenerator(){
  delete x_;
}

void NHSGenerator::generate(std::ostream& ostr, const nvar& v){
  x_->generate(ostr, v);
}

NHSGenerator::Type NHSGenerator::type(const nvar& v){
  return _global->type(v);
}

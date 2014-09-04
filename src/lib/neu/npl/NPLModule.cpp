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

#include <neu/NPLModule.h>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"

#include <neu/NPLParser.h>
#include <neu/NBasicMutex.h>

using namespace std;
using namespace llvm;
using namespace neu;

static const double _RAND_MAX = RAND_MAX;

extern "C"
double __n_uniform(){
  return rand()/_RAND_MAX;
}

namespace{

  typedef NVector<Type*> TypeVec;
  typedef NVector<Value*> ValueVec;
  typedef NMap<nvar, Function*, nvarLess<nvar>> FunctionMap;
  
  enum FunctionKey{
    FKEY_NO_KEY,
    FKEY_Add_2,
    FKEY_Sub_2,
    FKEY_Mul_2,
    FKEY_Div_2,
    FKEY_Mod_2,
    FKEY_And_2,
    FKEY_Or_2,
    FKEY_XOr_2,
    FKEY_ShL_2,
    FKEY_ShR_2,
    FKEY_BitAnd_2,
    FKEY_BitOr_2,
    FKEY_BitXOr_2,
    FKEY_BitComplement_1,
    FKEY_Not_1,
    FKEY_EQ_2,
    FKEY_NE_2,
    FKEY_GT_2,
    FKEY_GE_2,
    FKEY_LT_2,
    FKEY_LE_2,
    FKEY_If_2,
    FKEY_If_3,
    FKEY_Select_3,
    FKEY_Switch_2,
    FKEY_Local_1,
    FKEY_Local_2,
    FKEY_While_2,
    FKEY_For_4,
    FKEY_AddBy_2,
    FKEY_SubBy_2,
    FKEY_MulBy_2,
    FKEY_DivBy_2,
    FKEY_ModBy_2,
    FKEY_Block_n,
    FKEY_ScopedBlock_n,
    FKEY_Set_2,
    FKEY_Inc_1,
    FKEY_PostInc_1,
    FKEY_Dec_1,
    FKEY_PostDec_1,
    FKEY_Idx_2,
    FKEY_Arrow_2,
    FKEY_Size_1,
    FKEY_Neg_1,
    FKEY_Sqrt_1,
    FKEY_Pow_2,
    FKEY_Exp_1,
    FKEY_Vec_n,
    FKEY_Abs_1,
    FKEY_Log_1,
    FKEY_Log10_1,
    FKEY_Floor_1,
    FKEY_Ceil_1,
    FKEY_Break_0,
    FKEY_Continue_0,
    FKEY_Ret_1,
    FKEY_Ret_0,
    FKEY_Normalize_1,
    FKEY_Magnitude_1,
    FKEY_DotProduct_2,
    FKEY_CrossProduct_2,
    FKEY_Call_1,
    FKEY_Ptr_1,
    FKEY_DePtr_1,
    FKEY_Float_1,
    FKEY_Put_2,
    FKEY_PushBack_2,
    FKEY_TouchVector_1,
    FKEY_TouchList_1,
    FKEY_TouchQueue_1,
    FKEY_TouchSet_1,
    FKEY_TouchHashSet_1,
    FKEY_TouchMap_1,
    FKEY_TouchHashMap_1,
    FKEY_TouchMultimap_1,
    FKEY_Keys_1,
    FKEY_PushFront_2,
    FKEY_PopBack_1,
    FKEY_PopFront_1,
    FKEY_HasKey_2,
    FKEY_Insert_3,
    FKEY_Clear_1,
    FKEY_Empty_1,
    FKEY_Back_1,
    FKEY_Get_2,
    FKEY_Get_3,
    FKEY_Erase_2,
    FKEY_Cos_1,
    FKEY_Acos_1,
    FKEY_Cosh_1,
    FKEY_Sin_1,
    FKEY_Asin_1,
    FKEY_Sinh_1,
    FKEY_Tan_1,
    FKEY_Atan_1,
    FKEY_Tanh_1,
    FKEY_Merge_2,
    FKEY_OuterMerge_2,
    FKEY_Uniform_0,
    FKEY_Uniform_2
  };
  
  typedef NMap<pair<nstr, int>, FunctionKey> FunctionKeyMap;
  
  static FunctionKeyMap _functionMap;
  
  enum SymbolKey{
    SKEY_NO_KEY,
    SKEY_this,
  };
  
  typedef NMap<nstr, SymbolKey> SymbolKeyMap;
  
  static SymbolKeyMap _symbolMap;
  
  static void _initFunctionMap(){
    _functionMap[{"Add", 2}] = FKEY_Add_2;
    _functionMap[{"Sub", 2}] = FKEY_Sub_2;
    _functionMap[{"Mul", 2}] = FKEY_Mul_2;
    _functionMap[{"Div", 2}] = FKEY_Div_2;
    _functionMap[{"Mod", 2}] = FKEY_Mod_2;
    _functionMap[{"And", 2}] = FKEY_And_2;
    _functionMap[{"Or", 2}] = FKEY_Or_2;
    _functionMap[{"XOr", 2}] = FKEY_XOr_2;
    _functionMap[{"ShR", 2}] = FKEY_ShR_2;
    _functionMap[{"ShL", 2}] = FKEY_ShL_2;
    _functionMap[{"BitAnd", 2}] = FKEY_BitAnd_2;
    _functionMap[{"BitOr", 2}] = FKEY_BitOr_2;
    _functionMap[{"BitXOr", 2}] = FKEY_BitXOr_2;
    _functionMap[{"BitComplement", 1}] = FKEY_BitComplement_1;
    _functionMap[{"Not", 1}] = FKEY_Not_1;
    _functionMap[{"EQ", 2}] = FKEY_EQ_2;
    _functionMap[{"NE", 2}] = FKEY_NE_2;
    _functionMap[{"LT", 2}] = FKEY_LT_2;
    _functionMap[{"LE", 2}] = FKEY_LE_2;
    _functionMap[{"GT", 2}] = FKEY_GT_2;
    _functionMap[{"GE", 2}] = FKEY_GE_2;
    _functionMap[{"If", 2}] = FKEY_If_2;
    _functionMap[{"If", 3}] = FKEY_If_3;
    _functionMap[{"Select", 3}] = FKEY_Select_3;
    _functionMap[{"Switch", 2}] = FKEY_Switch_2;
    _functionMap[{"Set", 2}] = FKEY_Set_2;
    _functionMap[{"AddBy", 2}] = FKEY_AddBy_2;
    _functionMap[{"SubBy", 2}] = FKEY_SubBy_2;
    _functionMap[{"MulBy", 2}] = FKEY_MulBy_2;
    _functionMap[{"DivBy", 2}] = FKEY_DivBy_2;
    _functionMap[{"ModBy", 2}] = FKEY_ModBy_2;
    _functionMap[{"Block", -1}] = FKEY_Block_n;
    _functionMap[{"ScopedBlock", -1}] = FKEY_ScopedBlock_n;
    _functionMap[{"Inc", 1}] = FKEY_Inc_1;
    _functionMap[{"PostInc", 1}] = FKEY_PostInc_1;
    _functionMap[{"Dec", 1}] = FKEY_Dec_1;
    _functionMap[{"PostDec", 1}] = FKEY_PostDec_1;
    _functionMap[{"Local", 1}] = FKEY_Local_1;
    _functionMap[{"Local", 2}] = FKEY_Local_2;
    _functionMap[{"While", 2}] = FKEY_While_2;
    _functionMap[{"For", 4}] = FKEY_For_4;
    _functionMap[{"Idx", 2}] = FKEY_Idx_2;
    _functionMap[{"Arrow", 2}] = FKEY_Arrow_2;
    _functionMap[{"Size", 1}] = FKEY_Size_1;
    _functionMap[{"Neg", 1}] = FKEY_Neg_1;
    _functionMap[{"Sqrt", 1}] = FKEY_Sqrt_1;
    _functionMap[{"Pow", 2}] = FKEY_Pow_2;
    _functionMap[{"Exp", 1}] = FKEY_Exp_1;
    _functionMap[{"Vec", -1}] = FKEY_Vec_n;
    _functionMap[{"Abs", 1}] = FKEY_Abs_1;
    _functionMap[{"Log", 1}] = FKEY_Log_1;
    _functionMap[{"Log10", 1}] = FKEY_Log10_1;
    _functionMap[{"Floor", 1}] = FKEY_Floor_1;
    _functionMap[{"Ceil", 1}] = FKEY_Ceil_1;
    _functionMap[{"Break", 0}] = FKEY_Break_0;
    _functionMap[{"Continue", 0}] = FKEY_Continue_0;
    _functionMap[{"Ret", 1}] = FKEY_Ret_1;
    _functionMap[{"Ret", 0}] = FKEY_Ret_0;
    _functionMap[{"Normalize", 1}] = FKEY_Normalize_1;
    _functionMap[{"Magnitude", 1}] = FKEY_Magnitude_1;
    _functionMap[{"DotProduct", 2}] = FKEY_DotProduct_2;
    _functionMap[{"CrossProduct", 2}] = FKEY_CrossProduct_2;
    _functionMap[{"Call", 1}] = FKEY_Call_1;
    _functionMap[{"Ptr", 1}] = FKEY_Ptr_1;
    _functionMap[{"DePtr", 1}] = FKEY_DePtr_1;
    _functionMap[{"Float", 1}] = FKEY_Float_1;
    _functionMap[{"Put", 2}] = FKEY_Put_2;
    _functionMap[{"PushBack", 2}] = FKEY_PushBack_2;
    _functionMap[{"TouchVector", 1}] = FKEY_TouchVector_1;
    _functionMap[{"TouchList", 1}] = FKEY_TouchList_1;
    _functionMap[{"TouchQueue", 1}] = FKEY_TouchQueue_1;
    _functionMap[{"TouchSet", 1}] = FKEY_TouchSet_1;
    _functionMap[{"TouchHashSet", 1}] = FKEY_TouchHashSet_1;
    _functionMap[{"TouchMap", 1}] = FKEY_TouchMap_1;
    _functionMap[{"TouchHashMap", 1}] = FKEY_TouchHashMap_1;
    _functionMap[{"TouchMultimap", 1}] = FKEY_TouchMultimap_1;
    _functionMap[{"Keys", 1}] = FKEY_Keys_1;
    _functionMap[{"PushFront", 2}] = FKEY_PushFront_2;
    _functionMap[{"PopBack", 1}] = FKEY_PopBack_1;
    _functionMap[{"PopFront", 1}] = FKEY_PopFront_1;
    _functionMap[{"HasKey", 2}] = FKEY_HasKey_2;
    _functionMap[{"Insert", 3}] = FKEY_Insert_3;
    _functionMap[{"Clear", 1}] = FKEY_Clear_1;
    _functionMap[{"Empty", 1}] = FKEY_Empty_1;
    _functionMap[{"Back", 1}] = FKEY_Back_1;
    _functionMap[{"Get", 2}] = FKEY_Get_2;
    _functionMap[{"Get", 3}] = FKEY_Get_3;
    _functionMap[{"Erase", 2}] = FKEY_Erase_2;
    _functionMap[{"Cos", 1}] = FKEY_Cos_1;
    _functionMap[{"Acos", 1}] = FKEY_Acos_1;
    _functionMap[{"Cosh", 1}] = FKEY_Cosh_1;
    _functionMap[{"Sin", 1}] = FKEY_Sin_1;
    _functionMap[{"Asin", 1}] = FKEY_Asin_1;
    _functionMap[{"Sinh", 1}] = FKEY_Sinh_1;
    _functionMap[{"Tan", 1}] = FKEY_Tan_1;
    _functionMap[{"Atan", 1}] = FKEY_Atan_1;
    _functionMap[{"Tanh", 1}] = FKEY_Tanh_1;
    _functionMap[{"Merge", 2}] = FKEY_Merge_2;
    _functionMap[{"OuterMerge", 2}] = FKEY_OuterMerge_2;
    _functionMap[{"Uniform", 0}] = FKEY_Uniform_0;
    _functionMap[{"Uniform", 2}] = FKEY_Uniform_2;
  }
  
  static void _initSymbolMap(){
    _symbolMap["this"] = SKEY_this;
  }
  
  class NPLCompiler;
  
  class Global{
  public:
    Global();
    
    void init();
    
    Function* getFunction(const nstr& f);
    
    void createFunction(const nstr& f, const nstr& mf);
    
    StructType* varType;
    
  private:
    LLVMContext& context_;
    Module module_;
    FunctionMap functionMap_;
    NPLCompiler* compiler_;
  };
  
  NBasicMutex _mutex;
  Global* _global;
  
  Function* globalFunc(const nstr& f){
    return _global->getFunction(f);
  }
  
  class NPLCompiler{
  public:
    
    class LocalScope{
    public:
      Value* get(const nstr& s){
        auto itr = scopeMap_.find(s);
        
        if(itr == scopeMap_.end()){
          return 0;
        }
        
        return itr->second;
      }
      
      void put(const nstr& s, Value* vp){
        scopeMap_[s] = vp;
      }
    
      void putVar(Value* v){
        assert(!varMap_.hasKey(v));
        varMap_[v] = true;
      }
      
      void claimVar(Value* v){
        auto itr = varMap_.find(v);
        assert(itr != varMap_.end());
        varMap_.erase(itr);
      }
      
      void deleteVars(NPLCompiler* compiler){
        for(auto& itr : varMap_){
          compiler->deleteVar(itr.first);
        }
      }
      
    private:
      typedef NMap<nstr, Value*> ScopeMap_;
      typedef NMap<Value*, bool> VarMap_;
      
      ScopeMap_ scopeMap_;
      VarMap_ varMap_;
    };
    
    NPLCompiler(Module& module, FunctionMap& functionMap, ostream& estr)
    : module_(module),
    context_(module.getContext()),
    builder_(context_),
    estr_(&estr),
    functionMap_(functionMap){

    }
    
    ~NPLCompiler(){
      for(auto& itr : structMap_){
        delete itr.second;
      }
    }
    
    TypeVec typeVec(const nvec& v){
      TypeVec tv;
      for(const nstr& vi : v){
        tv.push_back(type(vi));
      }
      
      return tv;
    }
    
    Type* type(const nvar& t){
      if(t.isString()){
        return type(NPLParser::parseType(t));
      }
      
      size_t bits = t.get("bits", 0);
      size_t ptr = t.get("ptr", 0);
      
      if(bits == 0){
        if(ptr > 0){

          Type* pt;
          nstr c = t.get("class", "");
          if(c.empty()){
            pt = PointerType::get(Type::getIntNTy(context_, 8), 0);
          }
          else{
            auto itr = structMap_.find(c);

            if(itr == structMap_.end()){
              NERROR("unknown class: " + c);
            }
            
            Struct* s = itr->second;
            pt = PointerType::get(s->structType, 0);
          }

          for(size_t i = 1; i < ptr; ++i){
            pt = PointerType::get(pt, 0);
          }
          
          return pt;
        }
        
        return Type::getVoidTy(context_);
      }
      
      size_t len = t.get("len", 0);
      bool isFloat = t.get("float", false);
      
      Type* baseType;
      
      if(t.get("var", false)){
        baseType = _global->varType;
      }
      else if(isFloat){
        if(bits == 64){
          baseType = Type::getDoubleTy(context_);
        }
        else if(bits == 32){
          baseType = Type::getFloatTy(context_);
        }
        else{
          NERROR("invalid float type: " + t);
        }
      }
      else{
        baseType = Type::getIntNTy(context_, bits);
      }
      
      if(len > 0){
        Type* vecType = VectorType::get(baseType, len);
        
        if(ptr > 0){
          Type* pt = PointerType::get(vecType, 0);

          for(size_t i = 1; i < ptr; ++i){
            pt = PointerType::get(pt, 0);
          }
          
          return pt;
        }

        return vecType;
      }
      else if(ptr){
        Type* pt = PointerType::get(baseType, 0);
        
        for(size_t i = 1; i < ptr; ++i){
          pt = PointerType::get(pt, 0);
        }
        
        return pt;
      }

      return baseType;
    }

    Function* createFunction(const nstr& name,
                             Type* returnType,
                             const TypeVec& argTypes,
                             bool external=true){
      FunctionType* ft =
      FunctionType::get(returnType, argTypes.vector(), false);
      
      Function* f =
      Function::Create(ft,
                       external ?
                       Function::ExternalLinkage : Function::InternalLinkage,
                       name.c_str(), &module_);
      
      return f;
    }
    
    Function* createFunction(const nstr& name,
                             const nstr& returnType,
                             const nvec& argTypes,
                             bool external=true){
      return createFunction(name, type(returnType), typeVec(argTypes), external);
    }
    
    StructType* createStructType_(const nstr& name,
                                  const TypeVec& fieldTypes){
      return StructType::create(context_, fieldTypes.vector(), name.c_str());
    }

    StructType* createStructType(const nstr& name,
                                 const nvec& fieldTypes){
      return createStructType_(name, typeVec(fieldTypes));
    }
    
    Value* error(const nstr& msg, const nvar& n){
      *estr_ << "NPL compiler error: ";
      
      nstr loc = n.getLocation();
      
      if(!loc.empty()){
        *estr_ << loc << ": ";
      }
      
      *estr_ << msg << ": " << n.toStr() << endl;
      
      foundError_ = true;
      
      return 0;
    }

    ConstantPointerNull* null(){
      return
      ConstantPointerNull::get(PointerType::get(Type::getInt8Ty(context_), 0));
    }
    
    Value* getDouble(double v){
      return ConstantFP::get(context_, APFloat(v));
    }
    
    Value* getFloat(float v){
      return ConstantFP::get(context_, APFloat(v));
    }
    
    ConstantInt* getInt1(bool v){
      return ConstantInt::get(context_, APInt(1, v));
    }
    
    ConstantInt* getInt8(int8_t v){
      return ConstantInt::get(context_, APInt(8, v, true));
    }
    
    ConstantInt* getUInt8(uint8_t v){
      ConstantInt* vi = ConstantInt::get(context_, APInt(8, v, false));
      
      setUnsigned(vi);
      
      return vi;
    }
    
    ConstantInt* getInt16(int16_t v){
      return ConstantInt::get(context_, APInt(16, v, true));
    }
    
    ConstantInt* getUInt16(uint16_t v){
      ConstantInt* vi = ConstantInt::get(context_, APInt(16, v, false));
      
      setUnsigned(vi);
      
      return vi;
    }
    
    ConstantInt* getInt32(int32_t v){
      return ConstantInt::get(context_, APInt(32, v, true));
    }
    
    ConstantInt* getUInt32(int32_t v){
      ConstantInt* vi = ConstantInt::get(context_, APInt(32, v, false));
      
      setUnsigned(vi);
      
      return vi;
    }
    
    ConstantInt* getInt64(int64_t v){
      return ConstantInt::get(context_, APInt(64, v, true));
    }
    
    ConstantInt* getUInt64(uint64_t v){
      ConstantInt* vi = ConstantInt::get(context_, APInt(64, v, false));
      
      setUnsigned(vi);
      
      return vi;
    }

    Value* getString(const nstr& str){
      Value* gs = builder_.CreateGlobalStringPtr(str.c_str());
      return builder_.CreateBitCast(gs, type("char*"));
    }
    
    void deleteVar(Value* v){
      globalCall("void nvar::~nvar(nvar*)", {v});
    }
            
    Value* getLValue(const nvar& n){
      if(n.isSymbol()){
        Value* v = getLocal(n);
        if(v){
          return v;
        }
        
        v = getAttribute(n);
        
        if(v){
          return v;
        }
        
        error("undefined symbol: " + n, n);
        
        return 0;
      }
      
      Value* v = compile(n);
      
      if(!v){
        return 0;
      }
      
      if(!v->getType()->isPointerTy()){
        return error("not an l-value", n);
      }

      return v;
    }
  
    Value* convertNum(Value* from, Value* to, bool trunc=true){
      return convertNum(from, to->getType(), trunc);
    }
    
    Value* completeVar(Value* h, const nvar& v){
      Value* vv = 0;
      
      if(v.hasVector()){
        vv = toVar(h);
        
        globalCall("void nvar::intoVector(nvar*)", {vv});
        
        const nvec& vec = v;
        for(size_t i = 0; i < vec.size(); ++i){
          Value* vi = compile(v[i]);
          if(!vi){
            return 0;
          }
          
          pushBack(vv, toVar(vi));
        }
      }
      else if(v.hasList()){
        vv = toVar(h);

        globalCall("void nvar::intoList(nvar*)", {vv});
        
        const nlist& l = v;
        for(size_t i = 0; i < l.size(); ++i){
          Value* vi = compile(l[i]);
          if(!vi){
            return 0;
          }
          
          pushBack(vv, toVar(vi));
        }
      }
      else if(v.hasQueue()){
        vv = toVar(h);
        
        globalCall("void nvar::intoQueue(nvar*)", {vv});
        
        const nqueue& q = v;
        for(size_t i = 0; i < q.size(); ++i){
          Value* vi = compile(v[i]);
          if(!vi){
            return 0;
          }
          
          pushBack(vv, toVar(vi));
        }
      }

      if(v.hasSet()){
        vv = vv ? vv : toVar(h);
        
        globalCall("void nvar::intoSet(nvar*)", {vv});
        
        const nset& s = v;
        for(auto& itr : s){
          if((*itr).isHidden()){
            continue;
          }
          
          Value* k = compile(*itr);
          
          if(!k){
            return 0;
          }
          
          globalCall("void nvar::addKey(nvar*, nvar*)", {vv, k});
        }
      }
      else if(v.hasHashSet()){
        vv = vv ? vv : toVar(h);
        
        globalCall("void nvar::intoHashSet(nvar*)", {vv});
        
        const nhset& s = v;
        for(auto& itr : s){
          if((*itr).isHidden()){
            continue;
          }
          
          Value* k = compile(*itr);
          
          if(!k){
            return 0;
          }
          
          globalCall("void nvar::addKey(nvar*, nvar*)", {vv, k});
        }
      }
      else if(v.hasMap()){
        bool has = false;
        const nmap& m = v;
        for(auto& itr : m){
          const nvar& k = itr.first;
          const nvar& val = itr.second;
          
          if(!k.isHidden()){
            has = true;
            break;
          }
        }

        if(has){
          vv = vv ? vv : toVar(h);
          
          globalCall("void nvar::intoMap(nvar*)", {vv});
          
          const nmap& m = v;
          for(auto& itr : m){
            const nvar& k = itr.first;
            const nvar& val = itr.second;
            
            if(k.isHidden()){
              continue;
            }
            
            Value* kv;
            
            if(k.hasString()){
              kv = getString(k);
            }
            else{
              kv = compile(k);
            }
            
            if(!kv){
              return 0;
            }
            
            Value* p = put(vv, kv);
            
            Value* vc = compile(val);
            
            if(!vc){
              return 0;
            }
            
            store(vc, p);
          }
        }
      }
      else if(v.hasHashMap()){
        vv = vv ? vv : toVar(h);
        
        globalCall("void nvar::intoHashMap(nvar*)", {vv});
        
        const nhmap& m = v;
        for(auto& itr : m){
          const nvar& k = itr.first;
          const nvar& val = itr.second;
          
          if(k.isHidden()){
            continue;
          }
          
          Value* kv;
          
          if(k.hasString()){
            kv = getString(k);
          }
          else{
            kv = compile(k);
          }
          
          if(!kv){
            return 0;
          }
          
          Value* p = put(vv, kv);
          
          Value* vc = compile(val);
          
          if(!vc){
            return 0;
          }
          
          store(vc, p);
        }
      }
      else if(v.hasMultimap()){
        vv = vv ? vv : toVar(h);
        
        globalCall("void nvar::intoMultimap(nvar*)", {vv});
        
        const nmmap& m = v;
        for(auto& itr : m){
          const nvar& k = itr.first;
          const nvar& val = itr.second;
          
          if(k.isHidden()){
            continue;
          }
          
          Value* kv;
          
          if(k.hasString()){
            kv = getString(k);
          }
          else{
            kv = compile(k);
          }
          
          if(!kv){
            return 0;
          }
          
          Value* p = put(vv, kv);
          
          Value* vc = compile(val);
          
          if(!vc){
            return 0;
          }
          
          store(vc, p);
        }
      }
      
      if(vv){
        return vv;
      }
      
      return h;
    }
    
    Value* convertNum(Value* from, Type* toType, bool trunc=true){
      if(isVar(from)){
        if(toType->isIntegerTy()){
          Value* v = globalCall("long nvar::toLong(nvar*)", {from});
          return convertNum(v, toType);
        }
        else if(toType->isFloatingPointTy()){
          Value* v = globalCall("double nvar::toDouble(nvar*)", {from});
          return convertNum(v, toType);
        }

        return from;
      }
      
      if(toType->isPointerTy()){
        toType = elementType(toType);
      }
      
      nstr name = from->getName().str();
      
      Type* fromType = from->getType();
      Type* fromElementType = elementType(fromType);
      Type* toElementType = elementType(toType);
      
      if(IntegerType* toIntType = dyn_cast<IntegerType>(toElementType)){
        if(IntegerType* fromIntType = dyn_cast<IntegerType>(fromElementType)){
          size_t fromBits = fromIntType->getBitWidth();
          size_t toBits = toIntType->getBitWidth();
          name += ".i" + nvar(toBits);
          
          if(fromBits > toBits){
            return trunc ? builder_.CreateTrunc(from, toType, name.c_str()) : 0;
          }
          else if(fromBits < toBits){
            return builder_.CreateSExt(from, toType, name.c_str());
          }
          else{
            return from;
          }
        }
        else if(fromType->isDoubleTy()){
          return trunc ? builder_.CreateFPToSI(from, toType, name.c_str()) : 0;
        }
        else if(fromType->isFloatTy()){
          return trunc ? builder_.CreateFPToSI(from, toType, name.c_str()) : 0;
        }
      }
      else if(toElementType->isDoubleTy()){
        if(IntegerType* fromIntType = dyn_cast<IntegerType>(fromElementType)){
          size_t fromBits = fromIntType->getBitWidth();

          name += ".f64";
          
          return trunc || fromBits <= 32 ?
          builder_.CreateSIToFP(from, toType, name.c_str()) : 0;
        }
        else if(fromElementType->isDoubleTy()){
          return from;
        }
        else if(fromElementType->isFloatTy()){
          return builder_.CreateFPExt(from, toType, name.c_str());
        }
      }
      else if(toElementType->isFloatTy()){
        name += ".f32";
        
        if(IntegerType* fromIntType = dyn_cast<IntegerType>(fromElementType)){
          size_t fromBits = fromIntType->getBitWidth();
          
          return trunc || fromBits <= 32 ?
          builder_.CreateSIToFP(from, toType, name.c_str()) : 0;
        }
        else if(fromElementType->isDoubleTy()){
          return trunc ? builder_.CreateFPTrunc(from, toType, name.c_str()) : 0;
        }
        else if(fromElementType->isFloatTy()){
          return from;
        }
      }
      else if(isVar(toElementType)){
        return toVar(from);
      }
      
      return 0;
    }

    Value* createVar_(Value* v, const nstr& name){
      if(!v){
        Value* ret = createAlloca("nvar", name);
        Value* t = createStructGEP(ret, 1, "t_");
        createStore(getInt8(nvar::Undefined), t);
        return ret;
      }
    
      Type* t = v->getType();
      
      if(t->isVectorTy()){
        Value* nv = createAlloca(t, "vec.conv");
        createStore(v, nv);
        v = nv;
        t = v->getType();
      }
      
      if(t->isIntegerTy()){
        Value* ret = createAlloca("nvar", name);
        Value* h = createStructGEP(ret, 0, "h_");
        Value* t = createStructGEP(ret, 1, "t_");
        Value* vl = convert(v, "long");
        
        createStore(vl, h);
        createStore(getInt8(nvar::Integer), t);
        return ret;
      }
      else if(t->isFloatingPointTy()){
        Value* ret = createAlloca("nvar", name);
        Value* h = createStructGEP(ret, 0, "h_");
        h = builder_.CreateBitCast(h, type("double*"));
        Value* t = createStructGEP(ret, 1, "t_");
        
        Value* vd = convert(v, "double");
        createStore(vd, h);
        createStore(getInt8(nvar::Float), t);
        return ret;
      }
      else if(t->isPointerTy()){
        Type* pt = elementType(t);
        
        if(pt->isIntegerTy(8)){
          Value* ret = createAlloca("nvar", name);
          globalCall("void nvar::nvar(nvar*, char*)", {ret, v});
          return ret;
        }
        else if(VectorType* vt = dyn_cast<VectorType>(pt)){
          Type* et = elementType(vt);
          Type* at = pointerType(et);
          Value* vp = builder_.CreateBitCast(v, at);
          Value* n = getInt32(vectorLength(vt));
          Value* ret = createAlloca("nvar", name);
        
          if(IntegerType* intType = dyn_cast<IntegerType>(et)){
            size_t bits = intType->getBitWidth();
            switch(bits){
              case 8:
                globalCall("void nvar::nvar(nvar*, char*, int)", {ret, vp, n});
                return ret;
              case 16:
                globalCall("void nvar::nvar(nvar*, short*, int)", {ret, vp, n});
                return ret;
              case 32:
                globalCall("void nvar::nvar(nvar*, int*, int)", {ret, vp, n});
                return ret;
              case 64:
                globalCall("void nvar::nvar(nvar*, long*, int)", {ret, vp, n});
                return ret;
              default:
                NERROR("invalid vector integer type");
            }
            return 0;
          }
          else if(et->isDoubleTy()){
            globalCall("void nvar::nvar(nvar*, double*, int)", {ret, vp, n});
            return ret;
          }
          else if(et->isFloatTy()){
            globalCall("void nvar::nvar(nvar*, float*, int)", {ret, vp, n});
            return ret;
          }
        }
      }
      else if(isVar(t)){
        Value* ret = createAlloca("nvar", name);
        globalCall("void nvar::nvar(nvar*, nvar*)", {ret, v});
        return ret;
      }
      
      return 0;
    }
    
    Value* toVar(Value* v, const nstr& name="var"){
      if(isVar(v)){
        return v;
      }

      Value* ret = createVar_(v, name);
      if(ret){
        putVar(ret);
      }

      return ret;
    }

    Value* create(const nstr& t, const nstr& name, Value* v=0){
      return create(type(t), name, v);
    }
    
    Value* create(const nvar& t, const nstr& name, Value* v=0){
      return create(type(t), name, v);
    }
    
    Value* create(Type* t, const nstr& name, Value* v=0){
      if(isVar(t)){
        return createVar(name, v);
      }

      Value* ret = createAlloca(t, name);
      if(v){
        v = convert(v, elementType(ret));
        if(!v){
          return 0;
        }
        
        store(v, ret);
      }
      
      return ret;
    }
    
    Value* createVar(const nstr& name="var", Value* v=0){
      Value* ret = createVar_(v, name);
      if(ret){
        putVar(ret);
        return ret;
      }
      return ret;
    }
    
    Value* convert(Value* from, const nstr& toType, bool trunc=true){
      return convert(from, type(toType), trunc);
    }
    
    Value* convert(Value* from, Type* toType, bool trunc=true){
      Type* fromType = from->getType();
      
      if(VectorType* fromVecType = dyn_cast<VectorType>(fromType)){
        if(VectorType* toVecType = dyn_cast<VectorType>(toType)){
          size_t fromN = fromVecType->getNumElements();
          size_t toN = toVecType->getNumElements();
          
          if(fromN != toN){
            return 0;
          }

          return convertNum(from, toVecType, trunc);
        }

        return 0;
      }
      else if(VectorType* toVecType = dyn_cast<VectorType>(toType)){
        Value* e = convertNum(from, elementType(toVecType), trunc);
        if(!e){
          return 0;
        }

        Value* vp = createAlloca(toVecType, "vec");
        Value* v = createLoad(vp);
        
        size_t len = toVecType->getNumElements();
        for(size_t i = 0; i < len; ++i){
          v = builder_.CreateInsertElement(v, e, getInt32(i));
        }
        
        return v;
      }
      
      return convertNum(from, toType, trunc);
    }
    
    ValueVec normalize(Value* v1, Value* v2, bool trunc=true){
      if(!v1 || !v2){
        return ValueVec();
      }
      
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      Type* e1 = elementType(t1);
      Type* e2 = elementType(t2);
      
      if(isVar(e1) || isVar(e2)){
        return {convert(v1, "nvar"), convert(v2, "nvar")};
      }
      
      Type* tn = 0;
      
      if(IntegerType* intType1 = dyn_cast<IntegerType>(e1)){
        size_t bits1 = intType1->getBitWidth();

        if(IntegerType* intType2 = dyn_cast<IntegerType>(e2)){
          size_t bits2 = intType2->getBitWidth();
          
          if(bits1 >= bits2){
            tn = t1;
          }
          else{
            tn = t2;
          }
        }
        else if(e2->isDoubleTy() || e2->isFloatTy()){
          tn = trunc ? t2 : 0;
        }
      }
      else if(e1->isDoubleTy()){
        if(e2->isIntegerTy()){
          tn = trunc ? t1 : 0;
        }
        else if(e2->isFloatingPointTy()){
          tn = t1;
        }
      }
      else if(e1->isFloatTy()){
        if(e2->isIntegerTy()){
          tn = trunc ? t2 : 0;
        }
        else if(e2->isFloatingPointTy()){
          tn = t2;
        }
      }
      
      if(!tn){
        return ValueVec();
      }
      
      if(!tn->isVectorTy()){
        if(t1->isVectorTy()){
          tn = VectorType::get(tn, vectorLength(t1));
        }
        else if(t2->isVectorTy()){
          tn = VectorType::get(tn, vectorLength(t2));
        }
      }
      
      return {convert(v1, tn), convert(v2, tn)};
    }
    
    Value* convert(Value* from, Value* to, bool trunc=true){
      return convert(from, to->getType(), trunc);
    }
    
    Type* elementType(Value* v){
      return elementType(v->getType());
    }
    
    Type* elementType(Type* t){
      if(SequentialType* st = dyn_cast<SequentialType>(t)){
        return st->getElementType();
      }
      
      return t;
    }
    
    bool isString(Value* v){
      return isString(v->getType());
    }
    
    bool isString(Type* t){
      return t->isPointerTy() && elementType(t)->isIntegerTy(8);
    }
    
    size_t vectorLength(VectorType* vt){
      return vt->getNumElements();
    }
    
    size_t vectorLength(Type* t){
      VectorType* vt = dyn_cast<VectorType>(t);
      if(!vt){
        return 0;
      }
      
      return vectorLength(vt);
    }
    
    size_t vectorLength(Value* v){
      return vectorLength(v->getType());
    }
    
    Value* getNumeric(const nvar& x, Value* v){
      return getNumeric(x, v ? v->getType() : 0);
    }
    
    Value* getNumeric(const nvar& x, const nstr& t){
      return getNumeric(x, type(t));
    }

    Value* getNumeric(const nvar& x){
      Type* t = 0;
      return getNumeric(x, t);
    }
    
    Value* getNumeric(const nvar& x, Type* l){
      if(l){
        Type* t = elementType(l);
        if(IntegerType* it = dyn_cast<IntegerType>(t)){
          return ConstantInt::get(it, x.toLong());
        }
        else if(t->isFloatingPointTy()){
          return ConstantFP::get(t, x.toDouble());
        }
      }
      
      switch(x.type()){
        case nvar::False:
          return getInt1(false);
        case nvar::True:
          return getInt1(true);
        case nvar::Integer:
          return getInt64(x);
        case nvar::Float:
          return getDouble(x.asDouble());
        default:
          NERROR("invalid numeric type");
      }
      
      return 0;
    }
    
    Value* getLocal(const nstr& s){
      for(LocalScope* localScope : scopeStack_){
        Value* v = localScope->get(s);
        if(v){
          return v;
        }
      }
      
      return 0;
    }

    Type* pointerType(Value* v){
      return pointerType(v->getType());
    }
    
    Type* pointerType(Type* t){
      return PointerType::get(t, 0);
    }
    
    void dump(Value* v){
      cerr << v->getName().str() << ": ---------------" << endl;
      v->dump();
      cerr << endl;
    }
    
    void dump(Type* t){
      cerr << "TYPE ---------------" << endl;
      t->dump();
      cerr << endl;
    }
    
    Value* getAttribute(const nstr& s){
      auto itr = attributeMap_.find(s);
      
      if(itr == attributeMap_.end()){
        const nvar& c = currentClass();
        
        if(c.hasKey(s)){
          const nvar& a = c[s];

          BasicBlock* cb = builder_.GetInsertBlock();
          builder_.SetInsertPoint(entry_);
          
          Value* ap = createStructGEP(this_, a["index"], s);

          builder_.SetInsertPoint(cb);
          
          attributeMap_[s] = ap;
          
          return ap;
        }
        
        return 0;
      }
      
      return itr->second;
    }
   
    Value* createGEP(Value* ptr, Value* index){
      nstr name = ptr->getName().str();
      name += ".gep";
      return builder_.CreateGEP(ptr, index, name.c_str());
    }
    
    Value* createStructGEP(Value* structPtr,
                           size_t index,
                           const nstr& fieldName){
      nstr name = structPtr->getName().str();
      name.findReplace(".ptr", "");
      name += "." + fieldName + ".ptr";
      return builder_.CreateStructGEP(structPtr, index, name.c_str());
    }
    
    Value* createLoad(Value* ptr){
      nstr name = ptr->getName().str();
      
      if(name.endsWith(".ptr")){
        name.erase(name.length() - 4, 4);
      }
      
      return builder_.CreateLoad(ptr, name.c_str());
    }

    Value* createAlloca(const nstr& t, const nstr& name){
      return createAlloca(type(t), name);
    }
    
    Value* createAlloca(Type* t, const nstr& name){
      nstr n = name + ".ptr";
      return builder_.CreateAlloca(t, 0, n.c_str());
    }
    
    Value* createAdd(Value* v1, Value* v2){
      if(isIntegral(v1)){
        return builder_.CreateAdd(v1, v2, "add.out");
      }

      return builder_.CreateFAdd(v1, v2, "fadd.out");
    }
    
    Value* add(Value* v1, Value* v2){
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      if(isVar(t1)){
        Value* ret = createVar();
        
        if(t2->isIntegerTy()){
          Value* vl = convert(v2, "long");
          globalCall("void nvar::operator+(nvar*, nvar*, long)",
                     {ret, v1, vl});
          return ret;
        }
        else if(t2->isFloatingPointTy()){
          Value* vd = convert(v2, "double");
          globalCall("void nvar::operator+(nvar*, nvar*, double)",
                     {ret, v1, vd});
          return ret;
        }

        Value* v = toVar(v2);
        
        if(!v){
          return 0;
        }
        
        globalCall("void nvar::operator+(nvar*, nvar*, nvar*)",
                   {ret, v1, v});
        
        return ret;
      }
      else if(isVar(t2)){
        return add(v2, v1);
      }
      
      ValueVec v = normalize(v1, v2);
      
      if(v.empty()){
        return 0;
      }
      
      return createAdd(v[0], v[1]);
    }
    
    Value* createSub(Value* v1, Value* v2){
      if(isIntegral(v1)){
        return builder_.CreateSub(v1, v2, "sub.out");
      }

      return builder_.CreateFSub(v1, v2, "fsub.out");
    }
    
    Value* sub(Value* v1, Value* v2){
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      if(isVar(t1)){
        Value* ret = createVar();
        
        if(t2->isIntegerTy()){
          Value* vl = convert(v2, "long");
          globalCall("void nvar::operator-(nvar*, nvar*, long)",
                     {ret, v1, vl});
          return ret;
        }
        else if(t2->isFloatingPointTy()){
          Value* vd = convert(v2, "double");
          globalCall("void nvar::operator-(nvar*, nvar*, double)",
                     {ret, v1, vd});
          return ret;
        }
        
        Value* v = toVar(v2);
        
        if(!v){
          return 0;
        }
        
        globalCall("void nvar::operator-(nvar*, nvar*, nvar*)",
                   {ret, v1, v});
        
        return ret;
      }
      else if(isVar(t2)){
        return sub(toVar(v1), v2);
      }
      
      ValueVec v = normalize(v1, v2);
      
      if(v.empty()){
        return 0;
      }
      
      return createSub(v[0], v[1]);
    }
    
    Value* createMul(Value* v1, Value* v2){
      if(isIntegral(v1)){
        return builder_.CreateMul(v1, v2, "mul.out");
      }

      return builder_.CreateFMul(v1, v2, "fmul.out");
    }
    
    Value* mul(Value* v1, Value* v2){
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      if(isVar(t1)){
        Value* ret = createVar();
        
        if(t2->isIntegerTy()){
          Value* vl = convert(v2, "long");
          globalCall("void nvar::operator*(nvar*, nvar*, long)",
                     {ret, v1, vl});
          return ret;
        }
        else if(t2->isFloatingPointTy()){
          Value* vd = convert(v2, "double");
          globalCall("void nvar::operator*(nvar*, nvar*, double)",
                     {ret, v1, vd});
          return ret;
        }
        
        Value* v = toVar(v2);
        
        if(!v){
          return 0;
        }
        
        globalCall("void nvar::operator*(nvar*, nvar*, nvar*)",
                   {ret, v1, v});
        
        return ret;
      }
      else if(isVar(t2)){
        return mul(v2, v1);
      }
      
      ValueVec v = normalize(v1, v2);
      
      if(v.empty()){
        return 0;
      }
      
      return createMul(v[0], v[1]);
    }
    
    Value* createDiv(Value* v1, Value* v2){
      if(isIntegral(v1)){
        if(isUnsigned(v1) && isUnsigned(v2)){
          return builder_.CreateUDiv(v1, v2, "udiv.out");
        }
      
        return builder_.CreateSDiv(v1, v2, "sdiv.out");
      }

      return builder_.CreateFDiv(v1, v2, "fdiv.out");
    }
    
    Value* div(Value* v1, Value* v2){
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      if(isVar(t1)){
        Value* ret = createVar();
        
        if(t2->isIntegerTy()){
          Value* vl = convert(v2, "long");
          globalCall("void nvar::operator/(nvar*, nvar*, long)",
                     {ret, v1, vl});
          return ret;
        }
        else if(t2->isFloatingPointTy()){
          Value* vd = convert(v2, "double");
          globalCall("void nvar::operator/(nvar*, nvar*, double)",
                     {ret, v1, vd});
          return ret;
        }
        
        Value* v = toVar(v2);
        
        if(!v){
          return 0;
        }
        
        globalCall("void nvar::operator/(nvar*, nvar*, nvar*)",
                   {ret, v1, v});
        
        return ret;
      }
      else if(isVar(t2)){
        return div(toVar(v1), v2);
      }
      
      ValueVec v = normalize(v1, v2);
      
      if(v.empty()){
        return 0;
      }
      
      return createDiv(v[0], v[1]);
    }
    
    Value* createRem(Value* v1, Value* v2){
      if(isIntegral(v1)){
        if(isUnsigned(v1) && isUnsigned(v2)){
          Value* ret = builder_.CreateURem(v1, v2, "urem.out");
          setUnsigned(ret);
          return ret;
        }
        
        return builder_.CreateSRem(v1, v2, "srem.out");
      }
      
      return builder_.CreateFRem(v1, v2, "frem.out");
    }
    
    Value* mod(Value* v1, Value* v2){
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      if(isVar(t1)){
        Value* ret = createVar();
        
        if(t2->isIntegerTy()){
          Value* vl = convert(v2, "long");
          globalCall("void nvar::operator%(nvar*, nvar*, long)",
                     {ret, v1, vl});
          return ret;
        }
        else if(t2->isFloatingPointTy()){
          Value* vd = convert(v2, "double");
          globalCall("void nvar::operator%(nvar*, nvar*, double)",
                     {ret, v1, vd});
          return ret;
        }
        
        Value* v = toVar(v2);
        
        if(!v){
          return 0;
        }
        
        globalCall("void nvar::operator%(nvar*, nvar*, nvar*)",
                   {ret, v1, v});
        
        return ret;
      }
      else if(isVar(t2)){
        return mod(toVar(v1), v2);
      }
      
      ValueVec v = normalize(v1, v2);
      
      if(v.empty()){
        return 0;
      }
      
      return createRem(v[0], v[1]);
    }
    
    Value* addBy(Value* l, Value* r){
      Type* tl = l->getType();
      Type* tr = r->getType();
      
      if(isVar(tl)){
        if(tr->isIntegerTy()){
          Value* vl = convert(r, "long");
          return globalCall("nvar* nvar::operator+=(nvar*, long)",
                            {l, vl});
        }
        else if(tr->isFloatingPointTy()){
          Value* vd = convert(r, "double");
          return globalCall("nvar* nvar::operator+=(nvar*, double)",
                            {l, vd});
        }
        
        Value* vr = toVar(r);
        
        if(!vr){
          return 0;
        }
        
        return globalCall("nvar* nvar::operator+=(nvar*, nvar*)",
                          {l, vr});
        
      }
      
      Value* lv = createLoad(l);
      Value* rc = convert(r, lv);
      
      if(!rc){
        return 0;
      }
    
      Value* o = createAdd(lv, rc);
    
      store(o, l);
    
      return l;
    }
    
    Value* subBy(Value* l, Value* r){
      Type* tl = l->getType();
      Type* tr = r->getType();
      
      if(isVar(tl)){
        if(tr->isIntegerTy()){
          Value* vl = convert(r, "long");
          return globalCall("nvar* nvar::operator-=(nvar*, long)",
                            {l, vl});
        }
        else if(tr->isFloatingPointTy()){
          Value* vd = convert(r, "double");
          return globalCall("nvar* nvar::operator-=(nvar*, double)",
                            {l, vd});
        }
        
        Value* vr = toVar(r);
        
        if(!vr){
          return 0;
        }
        
        return globalCall("nvar* nvar::operator-=(nvar*, nvar*)",
                          {l, vr});
        
      }
      
      Value* lv = createLoad(l);
      Value* rc = convert(r, lv);
      
      if(!rc){
        return 0;
      }
      
      Value* o = createSub(lv, rc);
      
      store(o, l);
      
      return l;
    }
    
    Value* mulBy(Value* l, Value* r){
      Type* tl = l->getType();
      Type* tr = r->getType();
      
      if(isVar(tl)){
        if(tr->isIntegerTy()){
          Value* vl = convert(r, "long");
          return globalCall("nvar* nvar::operator*=(nvar*, long)",
                            {l, vl});
        }
        else if(tr->isFloatingPointTy()){
          Value* vd = convert(r, "double");
          return globalCall("nvar* nvar::operator*=(nvar*, double)",
                            {l, vd});
        }
        
        Value* vr = toVar(r);
        
        if(!vr){
          return 0;
        }
        
        return globalCall("nvar* nvar::operator*=(nvar*, nvar*)",
                          {l, vr});
        
      }
      
      Value* lv = createLoad(l);
      Value* rc = convert(r, lv);
      
      if(!rc){
        return 0;
      }
      
      Value* o = createMul(lv, rc);
      
      store(o, l);
      
      return l;
    }
    
    Value* divBy(Value* l, Value* r){
      Type* tl = l->getType();
      Type* tr = r->getType();
      
      if(isVar(tl)){
        if(tr->isIntegerTy()){
          Value* vl = convert(r, "long");
          return globalCall("nvar* nvar::operator/=(nvar*, long)",
                            {l, vl});
        }
        else if(tr->isFloatingPointTy()){
          Value* vd = convert(r, "double");
          return globalCall("nvar* nvar::operator/=(nvar*, double)",
                            {l, vd});
        }
        
        Value* vr = toVar(r);
        
        if(!vr){
          return 0;
        }
        
        return globalCall("nvar* nvar::operator/=(nvar*, nvar*)",
                          {l, vr});
        
      }
      
      Value* lv = createLoad(l);
      Value* rc = convert(r, lv);
      
      if(!rc){
        return 0;
      }
      
      Value* o = createDiv(lv, rc);
      
      store(o, l);
      
      return l;
    }
    
    Value* modBy(Value* l, Value* r){
      Type* tl = l->getType();
      Type* tr = r->getType();
      
      if(isVar(tl)){
        if(tr->isIntegerTy()){
          Value* vl = convert(r, "long");
          return globalCall("nvar* nvar::operator%=(nvar*, long)",
                            {l, vl});
        }
        else if(tr->isFloatingPointTy()){
          Value* vd = convert(r, "double");
          return globalCall("nvar* nvar::operator%=(nvar*, double)",
                            {l, vd});
        }
        
        Value* vr = toVar(r);
        
        if(!vr){
          return 0;
        }
        
        return globalCall("nvar* nvar::operator%=(nvar*, nvar*)",
                          {l, vr});
        
      }
      
      Value* lv = createLoad(l);
      Value* rc = convert(r, lv);
      
      if(!rc){
        return 0;
      }
      
      Value* o = createRem(lv, rc);
      
      store(o, l);
      
      return l;
    }
  
    Value* eq(Value* v1, Value* v2){
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      if(isVar(t1)){
        Value* ret = createVar("eq");
        
        if(t2->isIntegerTy()){
          Value* vl = convert(v2, "long");
          globalCall("void nvar::operator==(nvar*, nvar*, long)",
                     {ret, v1, vl});
          return ret;
        }
        else if(t2->isFloatingPointTy()){
          Value* vd = convert(v2, "double");
          globalCall("void nvar::operator==(nvar*, nvar*, double)",
                     {ret, v1, vd});
          return ret;
        }
        
        Value* v = toVar(v2);
        
        if(!v){
          return 0;
        }
        
        globalCall("void nvar::operator==(nvar*, nvar*, nvar*)",
                   {ret, v1, v});
        
        return ret;
      }
      else if(isVar(t2)){
        return eq(v2, v1);
      }
      
      ValueVec v = normalize(v1, v2);
      
      if(v.empty()){
        return 0;
      }
      
      if(isIntegral(v[0])){
        return builder_.CreateICmpEQ(v[0], v[1], "eq.out");
      }
      else{
        return builder_.CreateFCmpUEQ(v[0], v[1], "feq.out");
      }
    }
    
    Value* ne(Value* v1, Value* v2){
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      if(isVar(t1)){
        Value* ret = createVar("ne");
        
        if(t2->isIntegerTy()){
          Value* vl = convert(v2, "long");
          globalCall("void nvar::operator!=(nvar*, nvar*, long)",
                     {ret, v1, vl});
          return ret;
        }
        else if(t2->isFloatingPointTy()){
          Value* vd = convert(v2, "double");
          globalCall("void nvar::operator!=(nvar*, nvar*, double)",
                     {ret, v1, vd});
          return ret;
        }
        
        Value* v = toVar(v2);
        
        if(!v){
          return 0;
        }
        
        globalCall("void nvar::operator!=(nvar*, nvar*, nvar*)",
                   {ret, v1, v});
        
        return ret;
      }
      else if(isVar(t2)){
        return ne(v2, v1);
      }
      
      ValueVec v = normalize(v1, v2);
      
      if(v.empty()){
        return 0;
      }
      
      if(isIntegral(v[0])){
        return builder_.CreateICmpNE(v[0], v[1], "ne.out");
      }
      else{
        return builder_.CreateFCmpUNE(v[0], v[1], "fne.out");
      }
    }
    
    Value* lt(Value* v1, Value* v2){
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      if(isVar(t1)){
        Value* ret = createVar("lt");
        
        if(t2->isIntegerTy()){
          Value* vl = convert(v2, "long");
          globalCall("void nvar::operator<(nvar*, nvar*, long)",
                     {ret, v1, vl});
          return ret;
        }
        else if(t2->isFloatingPointTy()){
          Value* vd = convert(v2, "double");
          globalCall("void nvar::operator<(nvar*, nvar*, double)",
                     {ret, v1, vd});
          return ret;
        }
        
        Value* v = toVar(v2);
        
        if(!v){
          return 0;
        }
        
        globalCall("void nvar::operator<(nvar*, nvar*, nvar*)",
                   {ret, v1, v});
        
        return ret;
      }
      else if(isVar(t2)){
        return gt(v2, v1);
      }
      
      ValueVec v = normalize(v1, v2);
      
      if(v.empty()){
        return 0;
      }
      
      if(isIntegral(v[0])){
        if(isUnsigned(v[0]) && isUnsigned(v[1])){
          return builder_.CreateICmpULT(v[0], v[1], "lt.out");
        }
        
        return builder_.CreateICmpSLT(v[0], v[1], "lt.out");
      }
      
      return builder_.CreateFCmpULT(v[0], v[1], "flt.out");
    }
    
    Value* le(Value* v1, Value* v2){
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      if(isVar(t1)){
        Value* ret = createVar("le");
        
        if(t2->isIntegerTy()){
          Value* vl = convert(v2, "long");
          globalCall("void nvar::operator<=(nvar*, nvar*, long)",
                     {ret, v1, vl});
          return ret;
        }
        else if(t2->isFloatingPointTy()){
          Value* vd = convert(v2, "double");
          globalCall("void nvar::operator<=(nvar*, nvar*, double)",
                     {ret, v1, vd});
          return ret;
        }
        
        Value* v = toVar(v2);
        
        if(!v){
          return 0;
        }
        
        globalCall("void nvar::operator<=(nvar*, nvar*, nvar*)",
                   {ret, v1, v});
        
        return ret;
      }
      else if(isVar(t2)){
        return ge(v2, v1);
      }
      
      ValueVec v = normalize(v1, v2);
      
      if(v.empty()){
        return 0;
      }
      
      if(isIntegral(v[0])){
        if(isUnsigned(v[0]) && isUnsigned(v[1])){
          return builder_.CreateICmpULE(v[0], v[1], "le.out");
        }
        
        return builder_.CreateICmpSLE(v[0], v[1], "le.out");
      }
      
      return builder_.CreateFCmpULE(v[0], v[1], "fle.out");
    }
    
    Value* gt(Value* v1, Value* v2){
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      if(isVar(t1)){
        Value* ret = createVar("gt");
        
        if(t2->isIntegerTy()){
          Value* vl = convert(v2, "long");
          globalCall("void nvar::operator>(nvar*, nvar*, long)",
                     {ret, v1, vl});
          return ret;
        }
        else if(t2->isFloatingPointTy()){
          Value* vd = convert(v2, "double");
          globalCall("void nvar::operator>(nvar*, nvar*, double)",
                     {ret, v1, vd});
          return ret;
        }
        
        Value* v = toVar(v2);
        
        if(!v){
          return 0;
        }
        
        globalCall("void nvar::operator>(nvar*, nvar*, nvar*)",
                   {ret, v1, v});
        
        return ret;
      }
      else if(isVar(t2)){
        return lt(v2, v1);
      }
      
      ValueVec v = normalize(v1, v2);
      
      if(v.empty()){
        return 0;
      }
      
      if(isIntegral(v[0])){
        if(isUnsigned(v[0]) && isUnsigned(v[1])){
          return builder_.CreateICmpUGT(v[0], v[1], "gt.out");
        }
        
        return builder_.CreateICmpSGT(v[0], v[1], "gt.out");
      }
      
      return builder_.CreateFCmpUGT(v[0], v[1], "fgt.out");
    }
    
    Value* ge(Value* v1, Value* v2){
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      if(isVar(t1)){
        Value* ret = createVar("ge");
        
        if(t2->isIntegerTy()){
          Value* vl = convert(v2, "long");
          globalCall("void nvar::operator>=(nvar*, nvar*, long)",
                     {ret, v1, vl});
          return ret;
        }
        else if(t2->isFloatingPointTy()){
          Value* vd = convert(v2, "double");
          globalCall("void nvar::operator>=(nvar*, nvar*, double)",
                     {ret, v1, vd});
          return ret;
        }
        
        Value* v = toVar(v2);
        
        if(!v){
          return 0;
        }
        
        globalCall("void nvar::operator>=(nvar*, nvar*, nvar*)",
                   {ret, v1, v});
        
        return ret;
      }
      else if(isVar(t2)){
        return le(v2, v1);
      }
      
      ValueVec v = normalize(v1, v2);
      
      if(v.empty()){
        return 0;
      }
      
      if(isIntegral(v[0])){
        if(isUnsigned(v[0]) && isUnsigned(v[1])){
          return builder_.CreateICmpUGE(v[0], v[1], "ge.out");
        }
        
        return builder_.CreateICmpSGE(v[0], v[1], "ge.out");
      }
      
      return builder_.CreateFCmpUGE(v[0], v[1], "fge.out");
    }
    
    Value* vectorIndexVec(const nvar& i){
      if(!i.isFunction("Idx", 2)){
        return 0;
      }
      
      Value* vp = getLValue(i[0]);
      if(!vp){
        return 0;
      }
      
      if(elementType(vp)->isVectorTy()){
        return vp;
      }
      
      return 0;
    }
    
    Value* vectorIndexOp(Value* vp, const nvar& n){
      const nvar& i = n[0];
      assert(i.isFunction("Idx", 2));

      Value* idx = compile(i[1]);
      if(!idx){
        error("invalid vector index[1]", i);
        return 0;
      }
      
      idx = convert(idx, "int");
      if(!idx){
        error("invalid vector index[1]", i);
        return 0;
      }
      
      Value* v = createLoad(vp);
      assert(vectorLength(v) > 0);

      Value* r = compile(n[1]);
      
      FunctionKey key = getFunctionKey(n);

      Value* vi;
      if(key != FKEY_Set_2){
        vi = builder_.CreateExtractElement(v, idx);
      }
      
      switch(key){
        case FKEY_AddBy_2:
          r = add(vi, r);
          break;
        case FKEY_SubBy_2:
          r = sub(vi, r);
          break;
        case FKEY_MulBy_2:
          r = mul(vi, r);
          break;
        case FKEY_DivBy_2:
          r = div(vi, r);
          break;
        case FKEY_ModBy_2:
          r = div(vi, r);
          break;
        case FKEY_Set_2:
          break;
        default:
          assert(false && "invalid op");
      }

      r = convert(r, elementType(v));
      if(!r){
        error("invalid value[1]", n);
        return 0;
      }
      
      v = builder_.CreateInsertElement(v, r, idx);
      createStore(v, vp);
      
      return vp;
    }
    
    Value* idx(Value* v1, Value* v2){
      if(vectorLength(v1) > 0){
        Value* i = convert(v2, "int");
        
        Value* vi = builder_.CreateExtractElement(v1, i);
        
        if(isUnsigned(vi)){
          setUnsigned(vi);
        }
        
        return vi;
      }
      
      if(isVar(v1)){
        if(isVar(v2)){
          return globalCall("nvar* nvar::operator[](nvar*, nvar*)",
                            {v1, v2});
        }
        else{
          Value* i = convert(v2, "int");
          if(!i){
            return 0;
          }
          
          return globalCall("nvar* nvar::operator[](nvar*, int)",
                            {v1, i});
        }
      }
      
      return 0;
    }
    
    Value* size(Value* v){
      if(isVar(v)){
        return globalCall("long nvar::size[](nvar*)",
                          {v});
      }
      
      return getUInt64(vectorLength(v));
    }
    
    Value* neg(Value* v){
      if(isVar(v)){
        Value* ret = createVar("neg");
        globalCall("void nvar::operator-(nvar*, nvar*)",
                   {ret, v});
        return ret;
      }
      
      ValueVec vs = normalize(getInt64(0), v);
      
      if(vs.empty()){
        return 0;
      }
      
      return createSub(vs[0], vs[1]);
    }
    
    Value* pow(Value* v1, Value* v2){
      if(isVar(v1) || isVar(v2)){
        ValueVec v = normalize(v1, v2);
        if(v.empty()){
          return 0;
        }
        
        Value* ret = createVar("pow");
        globalCall("void nvar::pow(nvar*, nvar*, nvar*, void*)",
                   {ret, v[0], v[1], null()});
        
        return ret;
      }
      
      v1 = convert(v1, "double");
      if(!v1){
        return 0;
      }
      
      v2 = convert(v2, "double");
      if(!v2){
        return 0;
      }

      return globalCall("double pow(double, double)", {v1, v2});
    }
    
    Value* sqrt(Value* v){
      if(isVar(v)){
        Value* ret = createVar("sqrt");
        
        globalCall("void nvar::sqrt(nvar*, nvar*, void*)",
                   {ret, v, null()});
        
        return ret;
      }
      
      v = convert(v, "double");
      if(!v){
        return 0;
      }
      
      return globalCall("double sqrt(double)", {v});
    }
    
    Value* exp(Value* v){
      if(isVar(v)){
        Value* ret = createVar("exp");
        
        globalCall("void nvar::exp(nvar*, nvar*, void*)",
                   {ret, v, null()});
        
        return ret;
      }
      
      v = convert(v, "double");
      if(!v){
        return 0;
      }
      
      return globalCall("double exp(double)", {v});
    }
    
    Value* log(Value* v){
      if(isVar(v)){
        Value* ret = createVar("log");
        
        globalCall("void nvar::log(nvar*, nvar*, void*)",
                   {ret, v, null()});
        
        return ret;
      }
      
      v = convert(v, "double");
      if(!v){
        return 0;
      }
      
      return globalCall("double log(double)", {v});
    }
    
    Value* floor(Value* v){
      if(isVar(v)){
        Value* ret = createVar("floor");
        
        globalCall("void nvar::floor(nvar*, nvar*, void*)",
                   {ret, v, null()});
        
        return ret;
      }
      
      v = convert(v, "double");
      if(!v){
        return 0;
      }
      
      return globalCall("double floor(double)", {v});
    }
    
    Value* put(Value* l, Value* r){
      if(isString(r)){
        return globalCall("nvar* nvar::put(nvar*, char*)",
                          {l, r});
      }
      
      Value* rc = toVar(r);
      if(!rc){
        return 0;
      }
      
      return globalCall("nvar* nvar::put(nvar*, nvar*)",
                        {l, rc});
    }
    
    void pushBack(Value* l, Value* r){
      globalCall("void nvar::pushBack(nvar*, nvar*)",
                 {l, r});
    }
    
    Value* createShl(Value* v1, Value* v2){
      Value* ret = builder_.CreateShl(v1, v2, "shl.out");
      if(isUnsigned(v1) && isUnsigned(v2)){
        setUnsigned(ret);
      }
      
      return ret;
    }
    
    Value* createLShr(Value* v1, Value* v2){
      Value* ret = builder_.CreateLShr(v1, v2, "lshr.out");
      if(isUnsigned(v1) && isUnsigned(v2)){
        setUnsigned(ret);
      }
      
      return ret;
    }
    
    Value* createAnd(Value* v1, Value* v2){
      Value* ret = builder_.CreateAnd(v1, v2, "and.out");
      if(isUnsigned(v1) && isUnsigned(v2)){
        setUnsigned(ret);
      }
      
      return ret;
    }
    
    Value* createOr(Value* v1, Value* v2){
      Value* ret = builder_.CreateOr(v1, v2, "or.out");
      if(isUnsigned(v1) && isUnsigned(v2)){
        setUnsigned(ret);
      }
      
      return ret;
    }
    
    Value* createXor(Value* v1, Value* v2){
      Value* ret = builder_.CreateXor(v1, v2, "xor.out");
      if(isUnsigned(v1) && isUnsigned(v2)){
        setUnsigned(ret);
      }
      
      return ret;
    }
    
    void createStore(Value* v, Value* ptr){
      builder_.CreateStore(v, ptr);
    }
    
    void store(Value* v, Value* ptr){
      if(isVar(ptr)){
        Type* t = v->getType();
        
        if(t->isIntegerTy()){
          Value* vl = convert(v, "long");
          globalCall("nvar* nvar::operator=(nvar*, long)", {ptr, vl});
        }
        else if(t->isFloatingPointTy()){
          Value* vd = convert(v, "double");
          globalCall("nvar* nvar::operator=(nvar*, double)", {ptr, vd});
        }
        
        Value* vr = toVar(v);
        globalCall("nvar* nvar::operator=(nvar*, nvar*)", {ptr, vr});
      }
      else{
        builder_.CreateStore(v, ptr);
      }
    }
    
    FunctionKey getFunctionKey(const nvar& n){
      FunctionKeyMap::const_iterator itr = _functionMap.find({n.str(), n.size()});
      
      if(itr == _functionMap.end()){
        itr = _functionMap.find({n.str(), -1});
      }
      
      if(itr == _functionMap.end()){
        return FKEY_NO_KEY;
      }
      
      return itr->second;
    }
    
    SymbolKey getSymbolKey(const nstr& s){
      SymbolKeyMap::const_iterator itr = _symbolMap.find(s);
      
      if(itr == _symbolMap.end()){
        return SKEY_NO_KEY;
      }

      return itr->second;
    }

    Value* compile(const nvar& n){
      //cerr << "compiling: " << n << endl;
      
      if(n.isNumeric()){
        Value* v = getNumeric(n);
        v = completeVar(v, n);
        return v;
      }
      else if(n.isSymbol()){
        SymbolKey key = getSymbolKey(n);
        
        switch(key){
          case SKEY_this:
            return this_;
          default:
            break;
        }
        
        Value* v = getLocal(n);
        if(v){
          return isVar(v) ? v : createLoad(v);
        }
        
        v = getAttribute(n);
        
        if(v){
          return isVar(v) ? v : createLoad(v);
        }
        
        error("undefined symbol", n);
        
        return 0;
      }
      else if(n.isString()){
        Value* v = getString(n);
        v = completeVar(v, n);
        return v;
      }
      else if(!n.isFunction()){
        Value* v = createVar();
        v = completeVar(v, n);
        return v;
      }
      
      FunctionKey key = getFunctionKey(n);
      
      switch(key){
        case FKEY_NO_KEY:
          return error("invalid function", n);
        case FKEY_Add_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = add(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_Sub_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = sub(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_Mul_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = mul(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_Div_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = div(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_Mod_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = mod(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_ShL_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }

          if(isVar(l) || isVar(r)){
            return error("invalid operands", n);
          }
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return createShl(v[0], v[1]);
        }
        case FKEY_ShR_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          if(isVar(l) || isVar(r)){
            return error("invalid operands", n);
          }
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            return error("invalid operands", n);
          }
          
          return createLShr(v[0], v[1]);
        }
        case FKEY_BitAnd_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          if(isVar(l) || isVar(r)){
            return error("invalid operands", n);
          }
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            return error("invalid operands", n);
          }
          
          return createAnd(v[0], v[1]);
        }
        case FKEY_BitOr_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            return error("invalid operands", n);
          }
          
          return createOr(v[0], v[1]);
        }
        case FKEY_BitXOr_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          if(isVar(l) || isVar(r)){
            return error("invalid operands", n);
          }
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            return error("invalid operands", n);
          }
          
          return createXor(v[0], v[1]);
        }
        case FKEY_Block_n:{
          if(n.empty()){
            return getInt64(0);
          }
          
          Value* rv;
          bool ok = true;
          for(size_t i = 0; i < n.size(); ++i){
            rv = compile(n[i]);
            if(!rv){
              ok = false;
            }
          }
          
          return ok ? rv : 0;
        }
        case FKEY_ScopedBlock_n:{
          if(n.empty()){
            return getInt64(0);
          }
         
          pushScope();
          
          Value* rv;
          bool ok = true;
          for(size_t i = 0; i < n.size(); ++i){
            rv = compile(n[i]);
            if(!rv){
              ok = false;
            }
          }
          
          popScope();
          
          return ok ? rv : 0;
        }
        case FKEY_Local_1:{
          const nstr& s = n[0];

          Value* v = create(n[0], s);
          if(!v){
            return error("invalid operand[0]", n);
          }
          
          putLocal(s, v);
          
          return getInt64(0);
        }
        case FKEY_Local_2:{
          const nstr& s = n[0];

          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[0]", n);
          }
          
          Value* v = create(n[0], s, r);
          if(!v){
            return error("invalid operand[1]", n);
          }
          
          putLocal(s, v);
          
          return getInt64(0);
        }
        case FKEY_Set_2:{
          Value* vp = vectorIndexVec(n[0]);
          if(vp){
            return vectorIndexOp(vp, n);
          }
          
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          r = convert(r, elementType(l));
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          store(r, l);
          
          return l;
        }
        case FKEY_AddBy_2:{
          Value* vp = vectorIndexVec(n[0]);
          if(vp){
            return vectorIndexOp(vp, n);
          }
          
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = addBy(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_SubBy_2:{
          Value* vp = vectorIndexVec(n[0]);
          if(vp){
            return vectorIndexOp(vp, n);
          }
          
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = subBy(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_MulBy_2:{
          Value* vp = vectorIndexVec(n[0]);
          if(vp){
            return vectorIndexOp(vp, n);
          }
          
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = mulBy(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_DivBy_2:{
          Value* vp = vectorIndexVec(n[0]);
          if(vp){
            return vectorIndexOp(vp, n);
          }
          
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = divBy(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_ModBy_2:{
          Value* vp = vectorIndexVec(n[0]);
          if(vp){
            return vectorIndexOp(vp, n);
          }
          
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = modBy(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_Not_1:{
          Value* r = compile(n[0]);

          if(!r){
            return error("invalid operand[0]", n);
          }
          
          if(isVar(r)){
            return globalCall("void nvar::operator!(nvar*, nvar*)", {r});
          }
          
          Value* rc = convert(r, "bool");
          
          if(!rc){
            return error("invalid operand[0]", n);
          }
          
          Value* c = builder_.CreateICmpNE(rc, getInt1(0), "cmp.out");
          
          return builder_.CreateSelect(c, getInt1(0), getInt1(1), "not.out");
        }
        case FKEY_XOr_2:{
          Value* l = compile(n[0]);
          Value* r = compile(n[1]);
          
          if(isVar(l) || isVar(r)){
            error("invalid operands", n);
          }
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return builder_.CreateXor(v[0], v[1], "xor.out");
        }
        case FKEY_And_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          ValueVec v = normalize(l, r);

          if(v.empty()){
            return error("invalid operands", n);
          }
          
          if(isVar(v[0])){
            Value* ret = createVar("and");
            return globalCall("void nvar::operator&&(nvar*, nvar*, nvar*)",
                              {ret, v[0], v[1]});
          }
          
          return builder_.CreateAnd(v[0], v[1], "and.out");
        }
        case FKEY_Or_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            return error("invalid operands", n);
          }
          
          if(isVar(v[0])){
            Value* ret = createVar("or");
            return globalCall("void nvar::operator||(nvar*, nvar*, nvar*)",
                              {ret, v[0], v[1]});
          }
          
          return builder_.CreateOr(v[0], v[1], "or.out");
        }
        case FKEY_EQ_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = eq(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_NE_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = ne(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_LT_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = lt(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_LE_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = le(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_GT_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = gt(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_GE_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = ge(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_If_2:
        {
          Value* cv = compile(n[0]);
          if(!cv){
            return error("invalid operand[0]", n);
          }
          
          if(!isIntegral(cv)){
            cv = convert(cv, "int");
          }
          
          if(!cv){
            return error("not a boolean", n[0]);
          }
          
          cv = builder_.CreateICmpNE(cv, getInt1(0), "if.cond");
          
          BasicBlock* tb = BasicBlock::Create(context_, "then", func_);
          BasicBlock* mb = BasicBlock::Create(context_, "if.merge");
          
          builder_.CreateCondBr(cv, tb, mb);
          
          builder_.SetInsertPoint(tb);
          
          Value* tv = compile(n[1]);
          
          if(!tv){
            return error("invalid operand[1]", n);
          }
          
          if(!builder_.GetInsertBlock()->getTerminator()){
            builder_.CreateBr(mb);
          }
          
          func_->getBasicBlockList().push_back(mb);
          
          builder_.SetInsertPoint(mb);
          
          return getInt64(0);
        }
        case FKEY_If_3:{
          Value* cv = compile(n[0]);
          
          if(!cv){
            return error("invalid operand[0]", n);
          }
          
          if(!isIntegral(cv)){
            cv = convert(cv, "int");
          }
          
          if(!cv){
            return error("not a boolean", n[0]);
          }
          
          cv = builder_.CreateICmpNE(cv, getInt1(0), "if.cond");
          
          BasicBlock* tb = BasicBlock::Create(context_, "then", func_);
          BasicBlock* eb = BasicBlock::Create(context_, "else");
          BasicBlock* mb = BasicBlock::Create(context_, "if.merge");
          
          builder_.CreateCondBr(cv, tb, eb);
          
          builder_.SetInsertPoint(tb);
          
          Value* tv = compile(n[1]);
          if(!tv){
            return error("invalid operand[1]", n);
          }
          
          if(!builder_.GetInsertBlock()->getTerminator()){
            builder_.CreateBr(mb);
          }
          
          tb = builder_.GetInsertBlock();
          
          func_->getBasicBlockList().push_back(eb);
          builder_.SetInsertPoint(eb);
          
          Value* ev = compile(n[2]);
          if(!ev){
            return error("invalid operand[2]", n);
          }
          
          if(!builder_.GetInsertBlock()->getTerminator()){
            builder_.CreateBr(mb);
          }
          
          eb = builder_.GetInsertBlock();
          
          func_->getBasicBlockList().push_back(mb);
          
          builder_.SetInsertPoint(mb);
          
          return getInt64(0);
        }
        case FKEY_Select_3:{
          Value* cv = compile(n[0]);
          
          if(!cv){
            return error("invalid operand[0]", n);
          }
          
          Value* tv = compile(n[1]);
          if(!tv){
            return error("invalid operand[1]", n);
          }
          
          Value* fv = compile(n[2]);
          if(!fv){
            return error("invalid operand[2]", n);
          }
          
          return builder_.CreateSelect(cv, tv, fv, "select");
        }
        case FKEY_Switch_2:{
          Value* v = compile(n[0]);
          
          v = convert(v, "long");
          
          if(!v){
            return error("invalid operand", n[0]);
          }
          
          loopMerge_ = BasicBlock::Create(context_, "switch.merge", func_);
          
          const nvar& d = n[1];
          
          BasicBlock* db = 0;
          BasicBlock* cb = builder_.GetInsertBlock();
          
          if(d.some()){
            db = BasicBlock::Create(context_, "case", func_);
            builder_.SetInsertPoint(db);
            
            if(!compile(d)){
              loopMerge_ = 0;
              return 0;
            }
            
            builder_.CreateBr(loopMerge_);
          }

          builder_.SetInsertPoint(cb);
          
          const nmap& m = n;
          
          SwitchInst* s = builder_.CreateSwitch(v, db ? db : loopMerge_, m.size());
          
          for(auto& itr : m){
            const nvar& k = itr.first;

            if(k.isHidden()){
              continue;
            }
            
            if(!k.isInteger()){
              error("invalid case: " + k, n);
              return 0;
            }

            const nvar& c = itr.second;
            
            BasicBlock* b = BasicBlock::Create(context_, "case", func_);
            builder_.SetInsertPoint(b);
            
            compile(c);
            builder_.CreateBr(loopMerge_);
            
            s->addCase(getInt64(k), b);
          }

          builder_.SetInsertPoint(loopMerge_);
          
          loopMerge_ = 0;
          
          return s;
        }
        case FKEY_While_2:{
          loopContinue_ = BasicBlock::Create(context_, "while.cond", func_);
          
          builder_.CreateBr(loopContinue_);
          
          builder_.SetInsertPoint(loopContinue_);
          
          Value* cv = compile(n[0]);
          if(!cv){
            return error("invalid operand[0]", n);
          }
          
          if(!isIntegral(cv)){
            cv = convert(cv, "int");
          }
          
          if(!cv){
            return error("not a boolean", n[0]);
          }
          
          cv = builder_.CreateICmpNE(cv, getInt1(0), "while.cmp");
          
          BasicBlock* lb = BasicBlock::Create(context_, "while.body");
          loopMerge_ = BasicBlock::Create(context_, "while.merge");
          
          func_->getBasicBlockList().push_back(lb);
          func_->getBasicBlockList().push_back(loopMerge_);
          
          builder_.CreateCondBr(cv, loopContinue_, loopMerge_);
          
          builder_.SetInsertPoint(lb);
          Value* lv = compile(n[1]);
          if(!lv){
            return error("invalid operand[1]", n);
          }
          
          builder_.CreateBr(loopContinue_);
          
          builder_.SetInsertPoint(loopMerge_);
          
          loopContinue_ = 0;
          loopMerge_ = 0;
          
          return getInt64(0);
        }
        case FKEY_For_4:{
          Value* iv = compile(n[0]);
          if(!iv){
            return error("invalid operand[0]", n);
          }
          
          BasicBlock* lb = BasicBlock::Create(context_, "for.body", func_);
          loopMerge_ = BasicBlock::Create(context_, "for.merge");
          
          builder_.CreateBr(lb);
          builder_.SetInsertPoint(lb);
          
          Value* bv = compile(n[3]);
          if(!bv){
            return error("invalid operand[3]", n);
          }
          
          Value* nv = compile(n[2]);
          if(!nv){
            return error("invalid operand[2]", n);
          }
          
          loopContinue_ = BasicBlock::Create(context_, "for.cond");
          func_->getBasicBlockList().push_back(loopContinue_);
          builder_.CreateBr(loopContinue_);
          builder_.SetInsertPoint(loopContinue_);
          
          Value* cv = compile(n[1]);
          if(!cv){
            return error("invalid operand[1]", n);
          }
          
          if(!isIntegral(cv)){
            cv = convert(cv, "int");
          }
          
          if(!cv){
            return error("not a boolean", n[0]);
          }
          
          cv = builder_.CreateICmpNE(cv, getInt1(0), "for.cmp");
          
          func_->getBasicBlockList().push_back(loopMerge_);
          
          builder_.CreateCondBr(cv, lb, loopMerge_);
          
          builder_.SetInsertPoint(loopMerge_);
          
          loopContinue_ = 0;
          loopMerge_ = 0;
          
          return getInt64(0);
        }
        case FKEY_Break_0:{
          if(!loopMerge_){
            return error("break not in the context of a loop", n);
          }
          
          builder_.CreateBr(loopMerge_);
          return getInt64(0);
        }
        case FKEY_Continue_0:{
          if(!loopContinue_){
            return error("continue not in the context of a loop", n);
          }
          
          builder_.CreateBr(loopContinue_);
          return getInt64(0);
        }
        case FKEY_Ret_0:{
          builder_.CreateRetVoid();
          
          foundReturn_ = true;
          
          return getInt64(0);
        }
        case FKEY_Ret_1:{
          if(!rt_){
            return error("attempt to return from a non-function", n);
          }
          else if(rt_->isVoidTy()){
            return error("attempt to return a value from a void function", n);
          }
          
          Value* r = compile(n[0]);
          
          if(!r){
            return error("invalid operand[0]", n);
          }
          
          r = convert(r, rt_);
          
          if(!r){
            return error("invalid conversion to return type", n);
          }

          Value* rp = createStructGEP(args_, 2, "ret.ptr");
          store(r, rp);
          
          builder_.CreateRet(r);
          
          foundReturn_ = true;
          
          return getInt64(0);
        }
        case FKEY_Inc_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* v = addBy(l, getNumeric(1));
          
          return createLoad(v);
        }
        case FKEY_PostInc_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* vp = createLoad(l);
          
          addBy(l, getNumeric(1));
          
          return vp;
        }
        case FKEY_Dec_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* v = subBy(l, getNumeric(1));
          
          return createLoad(v);
        }
        case FKEY_PostDec_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* vp = createLoad(l);
          
          subBy(l, getNumeric(1));
          
          return vp;
        }
        case FKEY_Idx_2:{
          Value* v = compile(n[0]);
          if(!v){
            return error("invalid operand[0]", n);
          }
          
          Value* i = compile(n[1]);
          if(!i){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = idx(v, i);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_Arrow_2:{
          Value* v = compile(n[0]);
          if(!v){
            return error("invalid operand[0]", n);
          }
          
          PointerType* pt = dyn_cast<PointerType>(v->getType());
          
          if(!pt){
            return error("not a pointer[0]", n);
          }
          
          StructType* st = dyn_cast<StructType>(pt->getElementType());
          
          if(!st){
            return error("not a class[0]", n);
          }
          
          nstr c = st->getName().str();
          
          auto itr = structMap_.find(c);
          if(itr == structMap_.end()){
            return error("unknown class: " + c, n);
          }

          Struct* s = itr->second;
          size_t pos = s->getPos(n[1]);
          
          Value* r = builder_.CreateStructGEP(v, pos);
          
          return builder_.CreateLoad(r);
        }
        case FKEY_Size_1:{
          Value* v = compile(n[0]);
          if(!v){
            return error("invalid operand[0]", n);
          }
          
          Value* ret = size(v);
          if(!ret){
            return error("invalid operand", n);
          }
          
          return ret;
        }
        case FKEY_Neg_1:{
          Value* v = compile(n[0]);
          if(!v){
            return error("invalid operand[0]", n);
          }
          
          Value* ret = neg(v);
          if(!ret){
            return error("invalid operand", n);
          }
          
          return ret;
        }
        case FKEY_Vec_n:{
          size_t size = n.size();
          
          ValueVec v;
          for(size_t i = 0; i < size; ++i){
            Value* vi = compile(n[i]);
            if(!vi){
              return error("invalid operand[" + nvar(i) + "]", n);
            }
            v.push_back(vi);
          }
                        
          VectorType* vt = VectorType::get(v[0]->getType(), size);
          
          Value* vp = createAlloca(vt, "vec");
          Value* vr = createLoad(vp);
          
          for(size_t i = 0; i < size; ++i){
            vr = builder_.CreateInsertElement(vr, v[i], getInt32(i));
          }
          
          createStore(vr, vp);
          
          return vp;
        }
        case FKEY_Pow_2:{
          Value* l = compile(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = pow(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_Sqrt_1:{
          Value* v = compile(n[0]);
          if(!v){
            return error("invalid operand[0]", n);
          }

          Value* ret = sqrt(v);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_Exp_1:{
          Value* v = compile(n[0]);
          if(!v){
            return error("invalid operand[0]", n);
          }
          
          Value* ret = exp(v);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_Log_1:{
          Value* v = compile(n[0]);
          if(!v){
            return error("invalid operand[0]", n);
          }

          Value* ret = log(v);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_Floor_1:
        {
          Value* v = compile(n[0]);
          if(!v){
            return error("invalid operand[0]", n);
          }
          
          Value* ret = floor(v);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_Normalize_1:{
          Value* v = compile(n[0]);
          if(!v){
            return error("invalid operand[0]", n);
          }
          
          VectorType* vt = dyn_cast<VectorType>(v->getType());
          if(!vt){
            return v;
          }
          
          size_t length = vt->getNumElements();
          
          Type* et = elementType(vt);
          Value* d;
          if(et->isDoubleTy()){
            d = getDouble(0);
          }
          else if(et->isFloatTy()){
            d = getFloat(0);
          }
          else{
            return error("not a float/double vector", n[0]);
          }
          
          for(size_t i = 0; i < length; ++i){
            Value* e = builder_.CreateExtractElement(v, getInt32(i));
            e = builder_.CreateFMul(e, e);
            d = builder_.CreateFAdd(d, e);
          }
          
          ValueVec args;
          args.push_back(d);
          
          Value* sr;
          
          if(et->isDoubleTy()){
            sr = builder_.CreateCall(globalFunc("double sqrt(double)"),
                                     args.vector(), "sqrt");
          }
          else{
            sr = builder_.CreateCall(globalFunc("float sqrt(float)"),
                                     args.vector(), "sqrt");
          }
          
          Value* vd = createAlloca(vt, "vec");
          vd = builder_.CreateLoad(vd);
          
          for(size_t i = 0; i < length; ++i){
            vd = builder_.CreateInsertElement(vd, sr, getInt32(i));
          }
          
          return builder_.CreateFDiv(v, vd);
        }
        case FKEY_Magnitude_1:{
          Value* v = compile(n[0]);
          if(!v){
            return error("invalid operand[0]", n);
          }
          
          VectorType* vt = dyn_cast<VectorType>(v->getType());
          if(!vt){
            return v;
          }
          
          size_t length = vt->getNumElements();
          
          Type* et = elementType(vt);
          Value* d;
          if(et->isDoubleTy()){
            d = getDouble(0);
          }
          else if(et->isFloatTy()){
            d = getFloat(0);
          }
          else{
            return error("not a float/double vector", n[0]);
          }
          
          for(size_t i = 0; i < length; ++i){
            Value* e = builder_.CreateExtractElement(v, getInt32(i));
            e = builder_.CreateFMul(e, e);
            d = builder_.CreateFAdd(d, e);
          }
          
          ValueVec args;
          args.push_back(d);
          
          if(et->isDoubleTy()){
            return builder_.CreateCall(globalFunc("double sqrt(double)"),
                                       args.vector(), "sqrt");
          }

          return builder_.CreateCall(globalFunc("float sqrt(float)"),
                                     args.vector(), "sqrt");
        }
        case FKEY_DotProduct_2:{
          Value* lv = compile(n[0]);
          if(!lv){
            return error("invalid operand[0]", n);
          }
          
          size_t length = vectorLength(lv);
          if(length == 0){
            return error("expected a vector", n[0]);
          }
          
          Value* rv = compile(n[1]);
          if(!rv){
            return error("invalid operand[1]", n);
          }
          
          size_t length2 = vectorLength(rv);
          if(length == 0){
            return error("expected a vector", n[1]);
          }
          
          if(length != length2){
            return error("vector length mismatch", n);
          }
          
          ValueVec v = normalize(lv, rv);
          
          if(v.empty()){
            return error("type mismatch", n);
          }
          
          Value* r = getNumeric(0, elementType(v[0]));

          for(size_t i = 0; i < length; ++i){
            Value* e1 = builder_.CreateExtractElement(lv, getInt32(i));
            Value* e2 = builder_.CreateExtractElement(rv, getInt32(i));
            r = createAdd(r, createMul(e1, e2));
          }
          
          return r;
        }
        case FKEY_CrossProduct_2:{
          Value* lv = compile(n[0]);
          if(!lv){
            return error("invalid operand[0]", n);
          }
          
          if(vectorLength(lv) != 3){
            return error("expected a vector of length 3", n[0]);
          }
          
          Value* rv = compile(n[1]);
          if(!rv){
            return error("invalid operand[1]", n);
          }
          
          if(vectorLength(rv) != 3){
            return error("expected a vector of length 3", n[1]);
          }
          
          ValueVec v = normalize(lv, rv);
          
          if(v.empty()){
            return error("type mismatch", n);
          }
          
          VectorType* vt = cast<VectorType>(v[0]->getType());
          Type* et = vt->getElementType();
          
          Value* vr = createLoad(createAlloca(vt, "vec"));

          Value* e1 = builder_.CreateExtractElement(v[0], getInt32(1));
          Value* e2 = builder_.CreateExtractElement(v[1], getInt32(2));
          Value* e3 = builder_.CreateExtractElement(v[0], getInt32(2));
          Value* e4 = builder_.CreateExtractElement(v[1], getInt32(1));
          
          Value* e = createSub(createMul(e1, e2), createMul(e3, e4));
          
          vr = builder_.CreateInsertElement(vr, e, getInt32(0));
          
          e1 = builder_.CreateExtractElement(v[0], getInt32(2));
          e2 = builder_.CreateExtractElement(v[1], getInt32(0));
          e3 = builder_.CreateExtractElement(v[0], getInt32(0));
          e4 = builder_.CreateExtractElement(v[1], getInt32(2));
          
          e = createSub(createMul(e1, e2), createMul(e3, e4));
          vr = builder_.CreateInsertElement(vr, e, getInt32(1));
          
          e1 = builder_.CreateExtractElement(v[0], getInt32(0));
          e2 = builder_.CreateExtractElement(v[1], getInt32(1));
          e3 = builder_.CreateExtractElement(v[0], getInt32(1));
          e4 = builder_.CreateExtractElement(v[1], getInt32(0));
          
          e = createSub(createMul(e1, e2), createMul(e3, e4));
          vr = builder_.CreateInsertElement(vr, e, getInt32(2));
          
          return vr;
        }
        case FKEY_Call_1:{
          const nvar& fs = n[0];
          
          if(!fs.isFunction()){
            return error("invalid call", fs);
          }
          
          Value* ret;
          
          size_t size = fs.size();
          nstr name = fs;
          
          auto itr = functionMap_.find({className_, name, size});
          if(itr == functionMap_.end()){
            itr = functionMap_.find({name, size});
          
            if(itr == functionMap_.end()){
              return error("unknown function", n[0]);
            }
            
            Function* f = itr->second;

            ValueVec args;
            
            for(size_t i = 0; i < size; ++i){
              const nvar& fi = fs[i];
              
              Value* vi = compile(fi);
              args.push_back(vi);
            }
            
            if(f->getReturnType()->isVoidTy()){
              builder_.CreateCall(f, args.vector());
              return getInt64(0);
            }
            
            return builder_.CreateCall(f, args.vector(), name.c_str());
          }
          else{
            Function* f = itr->second;

            Value* as = f->arg_begin();
            Value* ap = createAlloca(elementType(as->getType()), "args");
            Value* tp = createStructGEP(ap, 1, "this");
            createStore(this_, tp);
            
            for(size_t i = 0; i < size; ++i){
              const nvar& fi = fs[i];
              
              Value* vi = compile(fi);
              Value* vp = createStructGEP(ap, i + 2, fi.str());
              vi = convert(vi, vp);
              if(!vi){
                error("invalid operand", fi);
              }
              
              createStore(vi, vp);
            }
            
            if(f->getReturnType()->isVoidTy()){
              builder_.CreateCall(f, {ap});
              return getInt64(0);
            }

            return builder_.CreateCall(f, {ap}, name.c_str());
          }
          
          return 0;
        }
        case FKEY_Ptr_1:{
          return getLValue(n[0]);
        }
        case FKEY_DePtr_1:{
          return compile(n[0]);
        }
        case FKEY_Float_1:{
          return getNumeric(n[0], "float");
        }
        case FKEY_Abs_1:{
          nvar f = nfunc("Select") << (nfunc("LT") << n[0] << 0)
          << (nfunc("Neg") << n[0]) << n[0];
          
          return compile(f);
        }
        case FKEY_Log10_1:{
          nvar f = nfunc("Div") << (nfunc("Log") << n[0]) <<
          (nfunc("Log") << 10.0);
          
          return compile(f);
        }
        case FKEY_Ceil_1:{
          nvar f = nfunc("Floor") << (nfunc("Add") << n[0] << 1.0);
          
          return compile(f);
        }
        case FKEY_BitComplement_1:{
          nvar f = nfunc("Sub") << (nfunc("Neg") << n[0]) << 1;

          return compile(f);
        }
        case FKEY_Put_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("invalid operand[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* ret = put(l, r);
          if(!ret){
            return error("invalid operands", n);
          }
          
          return ret;
        }
        case FKEY_PushBack_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* rc = toVar(r);
          
          pushBack(l, rc);

          return getInt64(0);
        }
        case FKEY_TouchVector_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          return globalCall("void nvar::intoVector(nvar*)", {l});
        }
        case FKEY_TouchList_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          return globalCall("void nvar::intoList(nvar*)", {l});
        }
        case FKEY_TouchQueue_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          return globalCall("void nvar::intoQueue(nvar*)", {l});
        }
        case FKEY_TouchSet_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          return globalCall("void nvar::intoSet(nvar*)", {l});
        }
        case FKEY_TouchHashSet_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          return globalCall("void nvar::intoHashSet(nvar*)", {l});
        }
        case FKEY_TouchMap_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          return globalCall("void nvar::intoMap(nvar*)", {l});
        }
        case FKEY_TouchHashMap_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          return globalCall("void nvar::intoHashMap(nvar*)", {l});
        }
        case FKEY_TouchMultimap_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }

          return globalCall("void nvar::intoMultimap(nvar*)", {l});
        }
        case FKEY_Keys_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          Value* ret = createVar("keys");
          return globalCall("void nvar::keys(nvar*, nvar*)", {ret, l});
        }
        case FKEY_PushFront_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return r;
          }
          
          Value* rv = toVar(r);
          
          return globalCall("void nvar::pushFront(nvar*, nvar*)", {l, rv});
        }
        case FKEY_PopBack_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          Value* ret = createVar("popBack");
          return globalCall("void nvar::popBack(nvar*, nvar*)", {ret, l});
        }
        case FKEY_PopFront_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);;
          }
          
          Value* ret = createVar("popFront");
          return globalCall("void nvar::popFront(nvar*, nvar*)", {ret, l});
        }
        case FKEY_HasKey_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* rv = toVar(r);
          
          return globalCall("bool nvar::hasKey(nvar*, nvar*)", {l, rv});
        }
        case FKEY_Insert_3:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          Value* i = compile(n[1]);
          if(!i){
            return error("invalid operand[1]", n);
          }
          
          i = convert(i, "long");
          
          Value* r = compile(n[2]);
          if(!r){
            return error("invalid operand[2]", n);
          }
          
          Value* rv = toVar(r);
          
          return globalCall("void nvar::insert(nvar*, long, nvar*)", {l, i, rv});
        }
        case FKEY_Clear_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          return globalCall("void nvar::clear(nvar*)", {l});
        }
        case FKEY_Empty_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          return globalCall("bool nvar::empty(nvar*)", {l});
        }
        case FKEY_Back_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          Value* ret = createVar("back");
          return globalCall("void nvar::back(nvar*, nvar*)", {ret, l});
        }
        case FKEY_Get_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* rv = toVar(r);
          
          Value* ret = createVar("get");
          return globalCall("void nvar::get(nvar*, nvar*, nvar*)", {ret, l, rv});
        }
        case FKEY_Get_3:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* rv = toVar(r);
          
          Value* d = compile(n[2]);
          if(!d){
            return error("invalid operand[2]", n);
          }
          
          Value* dv = toVar(d);
          
          Value* ret = createVar("get");
          return globalCall("void nvar::get(nvar*, nvar*, nvar*)", {ret, l, rv, dv});
        }
        case FKEY_Erase_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* rv = toVar(r);
          
          return globalCall("void nvar::erase(nvar*, nvar*)", {l, rv});
        }
        case FKEY_Cos_1:{
          Value* r = compile(n[0]);
          if(!r){
            return error("invalid operand[0]", n);
          }
          
          if(isVar(r)){
            Value* ret = createVar("cos");
            
            return globalCall("void nvar::cos(nvar*, nvar*, void*)",
                              {ret, r, null()});
          }
          
          Type* t = r->getType();
          
          if(t->isDoubleTy()){
            return globalCall("double cos(double)", {r});
          }
          else if(t->isFloatTy()){
            return globalCall("float cos(float)", {r});
          }
          
          Value* rv = toVar(r);
          if(!rv){
            error("not a var[0]", n);
          }
          
          Value* ret = createVar("cos");
          
          return globalCall("void nvar::cos(nvar*, nvar*, void*)",
                            {ret, rv, null()});        }
        case FKEY_Acos_1:{
          Value* r = compile(n[0]);
          if(!r){
            return error("invalid operand[0]", n);
          }
          
          Value* rv = toVar(r);
          if(!rv){
            error("not a var[0]", n);
          }
          
          Value* ret = createVar("acos");
          
          return globalCall("void nvar::acos(nvar*, nvar*, void*)", {ret, rv, null()});
        }
        case FKEY_Cosh_1:{
          Value* r = compile(n[0]);
          if(!r){
            return error("invalid operand[0]", n);
          }
          
          Value* rv = toVar(r);
          if(!rv){
            error("not a var[0]", n);
          }
          
          Value* ret = createVar("cosh");
          
          return globalCall("void nvar::cosh(nvar*, nvar*, void*)", {ret, rv, null()});
        }
        case FKEY_Sin_1:{
          Value* r = compile(n[0]);
          if(!r){
            return error("invalid operand[0]", n);
          }
          
          if(isVar(r)){
            Value* ret = createVar("sin");
            
            return globalCall("void nvar::sin(nvar*, nvar*, void*)",
                              {ret, r, null()});
          }
          
          Type* t = r->getType();
          
          if(t->isDoubleTy()){
            return globalCall("double sin(double)", {r});
          }
          else if(t->isFloatTy()){
            return globalCall("float sin(float)", {r});
          }
          
          Value* rv = toVar(r);
          if(!rv){
            error("not a var[0]", n);
          }
          
          Value* ret = createVar("sin");
          
          return globalCall("void nvar::sin(nvar*, nvar*, void*)",
                            {ret, rv, null()});
        }
        case FKEY_Asin_1:{
          Value* r = compile(n[0]);
          if(!r){
            return error("invalid operand[0]", n);
          }
          
          Value* rv = toVar(r);
          if(!rv){
            error("not a var[0]", n);
          }
          
          Value* ret = createVar("asin");
          
          return globalCall("void nvar::asin(nvar*, nvar*, void*)", {ret, rv, null()});
        }
        case FKEY_Sinh_1:{
          Value* r = compile(n[0]);
          if(!r){
            return error("invalid operand[0]", n);
          }
          
          Value* rv = toVar(r);
          if(!rv){
            error("not a var[0]", n);
          }
          
          Value* ret = createVar("sinh");
          
          return globalCall("void nvar::sinh(nvar*, nvar*, void*)", {ret, rv, null()});
        }
        case FKEY_Tan_1:{
          Value* r = compile(n[0]);
          if(!r){
            return error("invalid operand[0]", n);
          }
          
          Value* rv = toVar(r);
          if(!rv){
            error("not a var[0]", n);
          }
          
          Value* ret = createVar("tan");
          
          return globalCall("void nvar::tan(nvar*, nvar*, void*)", {ret, rv, null()});
        }
        case FKEY_Atan_1:{
          Value* r = compile(n[0]);
          if(!r){
            return error("invalid operand[0]", n);
          }
          
          Value* rv = toVar(r);
          if(!rv){
            error("not a var[0]", n);
          }
          
          Value* ret = createVar("atan");
          
          return globalCall("void nvar::atan(nvar*, nvar*, void*)", {ret, rv, null()});
        }
        case FKEY_Tanh_1:{
          Value* r = compile(n[0]);
          if(!r){
            return error("invalid operand[0]", n);
          }
          
          Value* rv = toVar(r);
          if(!rv){
            error("not a var[0]", n);
          }
          
          Value* ret = createVar("tanh");
          
          return globalCall("void nvar::tanh(nvar*, nvar*, void*)", {ret, rv, null()});
        }
        case FKEY_Merge_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);;
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* rv = toVar(r);
          if(!rv){
            error("not a var[0]", n);
          }
          
          return globalCall("void nvar::merge(nvar*, nvar*)", {l, rv});
        }
        case FKEY_OuterMerge_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return error("invalid operand[0]", n);
          }
          
          if(!isVar(l)){
            return error("not a var[0]", n);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return error("invalid operand[1]", n);
          }
          
          Value* rv = toVar(r);
          if(!rv){
            error("not a var[0]", n);
          }
          
          return globalCall("void nvar::outerMerge(nvar*, nvar*)", {l, rv});
        }
        case FKEY_Uniform_0:{
          return globalCall("double __n_uniform()");
        }
        case FKEY_Uniform_2:{
          nvar f = nfunc("Add") << n[0] <<
          (nfunc("Mul") << (nfunc("Sub") << n[1] << n[0]) << nfunc("Uniform"));

          return compile(f);
        }
        default:
          func_->dump();
          NERROR("unimplemented function: " + n);
      }
      
      return 0;
    }
    
    const nvar& code(){
      return *code_;
    }
    
    const nvar& currentClass(){
      return *currentClass_;
    }
    
    const nvar& currentFunc(){
      return *currentFunc_;
    }
    
    bool compileTop(const nvar& code){
      foundError_ = false;
      
      code_ = &code;
      
      nvec cs;
      code.keys(cs);

      for(size_t i = 0; i < cs.size(); ++i){
        const nvar& ck = cs[i];
        const nvar& ci = code[ck];
        
        if(ck.size() == 2){
          Type* rt = type(ci[0]);
          
          TypeVec args;
          
          const nvar& fs = ci[1];
          
          for(const nvar& a : fs){
            args.push_back(type(a));
          }
          
          Function* e = createFunction(fs, rt, args);
          functionMap_[{fs.sym(), fs.size()}] = e;
        }
      }
      
      for(size_t i = 0; i < cs.size(); ++i){
        const nvar& ck = cs[i];
        const nvar& ci = code[ck];
        
        if(ck.size() == 2){
          continue;
        }
        
        currentClass_ = &ci;
        
        nvec ms;
        ci.keys(ms);
        
        TypeVec tv;
       
        nvar am;
        
        for(size_t j = 0; j < ms.size(); ++j){
          const nvar& mk = ms[j];
          const nvar& mj = ci[mk];
          
          if(mk.size() == 0){
            am(mj["index"]) = mj;
          }
        }
        
        Struct* s = new Struct;
        
        nvec keys;
        am.keys(keys);
        int pos = 0;
        for(const nvar& k : keys){
          s->putField(am[k], pos);
          tv.push_back(type(am[k]));
          ++pos;
        }
        
        classStruct_ = StructType::create(context_, tv.vector(), ck.c_str());
        s->structType = classStruct_;
        structMap_[ck] = s;

        for(size_t j = 0; j < ms.size(); ++j){
          const nvar& mk = ms[j];
          const nvar& mj = ci[mk];
          
          if(mk.size() == 2){
            functionMap_[{ck, mk[0], mk[1]}] = createPrototype(ck, mj);
          }
        }
        
        for(size_t j = 0; j < ms.size(); ++j){
          const nvar& mk = ms[j];
          const nvar& mj = ci[mk];

          if(mk.size() == 2){
            currentFunc_ = &mj;
            
            compileFunction(ck, mj);
          }
        }
      }
      
      // **** for debugging, uncomment to dump the module
      //module_.dump();
      
      return !foundError_;
    }
    
    Function* createPrototype(const nstr& className, const nvar& f){
      const nvar& fs = f[1];
      
      nstr n = className + "_" + fs.str() + "_" + fs.size();
      
      TypeVec args;
      args.push_back(type("void*"));
      args.push_back(pointerType(classStruct_));
      
      Type* rt = type(f[0]);
      
      if(!rt->isVoidTy()){
        args.push_back(rt);
      }
      
      for(const nvar& a : fs){
        args.push_back(type(a));
      }
      
      nstr an = n + "_args";
      
      Type* argsStruct =
      StructType::create(context_, args.vector(), an.c_str());
      
      return createFunction(n, rt, {pointerType(argsStruct)});
    }
    
    Function* compileFunction(const nstr& className, const nvar& f){
      scopeStack_.clear();
      attributeMap_.clear();
      loopContinue_ = 0;
      loopMerge_ = 0;
      
      className_ = className;
      
      const nvar& fs = f[1];
      
      func_ = functionMap_[{className_, fs.str(), fs.size()}];

      rt_ = func_->getReturnType();
      
      Function::arg_iterator aitr = func_->arg_begin();
      aitr->setName("args");
      args_ = aitr;
      
      entry_ =
      BasicBlock::Create(context_, "entry", func_);
      
      builder_.SetInsertPoint(entry_);

      this_ = createLoad(createStructGEP(args_, 1, "this"));
      
      pushScope();
      
      size_t offset = rt_->isVoidTy() ? 2 : 3;
      
      for(size_t i = 0; i < fs.size(); ++i){
        const nstr& s = fs[i];
        
        Value* v = createStructGEP(args_, offset + i, s);
        putLocal(s, v);
      }
      
      begin_ =
      BasicBlock::Create(context_, "begin", func_);
      
      builder_.SetInsertPoint(begin_);
      
      foundReturn_ = false;
      
      Value* b = compile(f[2]);
      
      popScope();
      
      if(!b){
        func_->eraseFromParent();
        return 0;
      }
      
      if(rt_->isVoidTy()){
        builder_.CreateRetVoid();
      }
      else if(!foundReturn_){
        error("no return found with non-void return type", f);
        func_->eraseFromParent();
        return 0;
      }
      
      builder_.SetInsertPoint(entry_);
      builder_.CreateBr(begin_);
      
      return func_;
    }
    
    LocalScope* pushScope(){
      LocalScope* scope = new LocalScope;
      scopeStack_.push_back(scope);
      
      return scope;
    }
    
    void popScope(){
      assert(!scopeStack_.empty());
      
      LocalScope* scope = scopeStack_.back();
      scope->deleteVars(this);
      delete scope;
      scopeStack_.pop_back();
    }

    void putLocal(const nstr& s, Value* v){
      assert(!scopeStack_.empty());
      
      LocalScope* scope = scopeStack_.back();
      scope->put(s, v);
    }
    
    void putVar(Value* v){
      assert(!scopeStack_.empty());
      
      LocalScope* scope = scopeStack_.back();
      scope->putVar(v);
    }
    
    nvar& getInfo(Value* v){
      auto itr = infoMap_.find(v);
      
      if(itr == infoMap_.end()){
        auto itr = infoMap_.insert({v, undef});
        return itr.first->second;
      }
      
      return itr->second;
    }
    
    bool isUnsigned(Value* v){
      const nvar& info = getInfo(v);
      return !info.get("signed", true);
    }
    
    void setUnsigned(Value* v){
      nvar& info = getInfo(v);
      info("signed") = false;
    }
    
    bool isIntegral(Value* v){
      return isIntegral(v->getType());
    }
    
    bool isIntegral(Type* t){
      return dyn_cast<IntegerType>(elementType(t));
    }
    
    bool isVar(Type* t){
      return elementType(t) == _global->varType;
    }
    
    bool isVar(Value* v){
      return isVar(v->getType());
    }
    
    Value* globalCall(const nstr& c){
      Function* f = globalFunc(c);
      
      Type* rt = f->getReturnType();
      
      if(rt->isVoidTy()){
        return builder_.CreateCall(f);
      }
      else{
        return builder_.CreateCall(f, c.c_str());
      }
    }
    
    Value* globalCall(const nstr& c, const ValueVec& v){
      Function* f = globalFunc(c);

      Type* rt = f->getReturnType();
      
      if(rt->isVoidTy()){
        return builder_.CreateCall(f, v.vector());
      }
      else{
        return builder_.CreateCall(f, v.vector(), c.c_str());
      }
    }
    
  private:
    class Struct{
    public:
      StructType* structType;

      void putField(const nstr& field, int pos){
        fieldMap_[field] = pos;
      }
      
      int getPos(const nstr& field){
        auto itr = fieldMap_.find(field);
        if(itr == fieldMap_.end()){
          return -1;
        }
        
        return itr->second;
      }
      
    private:
      typedef NMap<nstr, int> FieldMap_;

      FieldMap_ fieldMap_;
    };
    
    typedef NVector<LocalScope*> ScopeStack_;
    typedef NMap<Value*, nvar> InfoMap_;
    typedef NMap<nstr, Value*> AttributeMap_;
    typedef NMap<nstr, Struct*> StructMap_;
    
    LLVMContext& context_;
    Module& module_;
    IRBuilder<> builder_;

    ostream* estr_;
    FunctionMap& functionMap_;
    StructMap_ structMap_;
    ScopeStack_ scopeStack_;
    AttributeMap_ attributeMap_;
    InfoMap_ infoMap_;
    Value* args_;
    Type* rt_;
    Function* func_;
    const nvar* code_;
    const nvar* currentClass_;
    const nvar* currentFunc_;
    BasicBlock* loopContinue_;
    BasicBlock* loopMerge_;
    BasicBlock* entry_;
    BasicBlock* begin_;
    bool foundReturn_;
    bool foundError_;
    StructType* classStruct_;
    Value* this_;
    nstr className_;
  };
  
  Global::Global()
  : context_(getGlobalContext()),
  module_("global", context_){
    
    InitializeNativeTarget();
    
    _initFunctionMap();
    _initSymbolMap();
    
    compiler_ = new NPLCompiler(module_, functionMap_, cerr);
    
    varType = compiler_->createStructType("nvar", {"long", "char"});
  }
  
  void Global::init(){
    createFunction("double sqrt(double)",
                   "llvm.sqrt.f64");

    createFunction("float sqrt(float)",
                   "llvm.sqrt.f32");
    
    createFunction("double pow(double, double)",
                   "llvm.pow.f64");
    
    createFunction("double log(double)",
                   "llvm.sqrt.f64");
    
    createFunction("float log10(float)",
                   "llvm.log10.f32");
    
    createFunction("double log10(double)",
                   "llvm.log10.f64");
    
    createFunction("float log2(float)",
                   "llvm.log2.f32");
    
    createFunction("double log2(double)",
                   "llvm.log2.f64");
    
    createFunction("double exp(double)",
                   "llvm.sqrt.f64");

    createFunction("float sin(float)",
                   "llvm.sin.f32");
    
    createFunction("double sin(double)",
                   "llvm.sin.f64");
    
    createFunction("float cos(float)",
                   "llvm.cos.f32");
    
    createFunction("double cos(double)",
                   "llvm.cos.f64");

    createFunction("float pow(float, float)",
                   "llvm.pow.f32");
    
    createFunction("double pow(double, double)",
                   "llvm.pow.f64");
    
    createFunction("float fabs(float)",
                   "llvm.fabs.f32");
    
    createFunction("double fabs(double)",
                   "llvm.fabs.f64");
    
    createFunction("float floor(float)",
                   "llvm.floor.f32");
    
    createFunction("double floor(double)",
                   "llvm.floor.f64");
    
    createFunction("float ceil(float)",
                   "llvm.ceil.f32");
    
    createFunction("double ceil(double)",
                   "llvm.ceil.f64");
    
    createFunction("float round(float)",
                   "llvm.round.f32");
    
    createFunction("double round(double)",
                   "llvm.round.f64");
    
    createFunction("double __n_uniform()",
                   "__n_uniform");
    
    createFunction("void nvar::nvar(nvar*, nvar*)",
                   "_ZN3neu4nvarC1ERKS0_");
    
    createFunction("void nvar::nvar(nvar*, char*)",
                   "_ZN3neu4nvarC1EPKc");

    createFunction("void nvar::nvar(nvar*, char*, int)",
                   "_ZN3neu4nvarC1EPai");
    
    createFunction("void nvar::nvar(nvar*, short*, int)",
                   "_ZN3neu4nvarC1EPsi");
    
    createFunction("void nvar::nvar(nvar*, int*, int)",
                   "_ZN3neu4nvarC1EPii");
    
    createFunction("void nvar::nvar(nvar*, long*, int)",
                   "_ZN3neu4nvarC1EPxi");
    
    createFunction("void nvar::nvar(nvar*, float*, int)",
                   "_ZN3neu4nvarC1EPfi");
    
    createFunction("void nvar::nvar(nvar*, double*, int)",
                   "_ZN3neu4nvarC1EPdi");
    
    createFunction("void nvar::~nvar(nvar*)",
                   "_ZN3neu4nvarD1Ev");
    
    createFunction("long nvar::toLong(nvar*)",
                   "_ZNK3neu4nvar6toLongEv");
    
    createFunction("double nvar::toDouble(nvar*)",
                   "_ZNK3neu4nvar8toDoubleEv");
    
    createFunction("nvar* nvar::operator=(nvar*, long)",
#ifdef __APPLE__
                   "_ZN3neu4nvaraSEx");
#else
                   "_ZN3neu4nvaraSEl");
#endif
    
    createFunction("nvar* nvar::operator=(nvar*, double)",
                   "_ZN3neu4nvaraSEd");
    
    createFunction("nvar* nvar::operator=(nvar*, nvar*)",
                   "_ZN3neu4nvaraSERKS0_");
    
    createFunction("void nvar::operator+(nvar*, nvar*, long)",
#ifdef __APPLE__
                   "_ZNK3neu4nvarplEx");
#else
                   "_ZNK3neu4nvarplEl");
#endif
    
    createFunction("void nvar::operator+(nvar*, nvar*, double)",
                   "_ZNK3neu4nvarplEd");

    createFunction("void nvar::operator+(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvarplERKS0_");
    
    createFunction("void nvar::operator-(nvar*, nvar*, long)",
#ifdef __APPLE__
                   "_ZNK3neu4nvarmiEx");
#else
                   "_ZNK3neu4nvarmiEl");
#endif
    
    createFunction("void nvar::operator-(nvar*, nvar*, double)",
                   "_ZNK3neu4nvarmiEd");
    
    createFunction("void nvar::operator-(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvarmiERKS0_");
    
    createFunction("void nvar::operator*(nvar*, nvar*, long)",
#ifdef __APPLE__
                   "_ZNK3neu4nvarplEx");
#else
                   "_ZNK3neu4nvarplEl");
#endif
    
    createFunction("void nvar::operator*(nvar*, nvar*, double)",
                   "_ZNK3neu4nvarplEd");
    
    createFunction("void nvar::operator*(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvarpl");
    
    createFunction("void nvar::operator/(nvar*, nvar*, long)",
#ifdef __APPLE__
                   "_ZNK3neu4nvardvEx");
#else
                   "_ZNK3neu4nvardvEl");
#endif
    
    createFunction("void nvar::operator/(nvar*, nvar*, double)",
                   "_ZNK3neu4nvardvEd");
    
    createFunction("void nvar::operator/(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvardvERKS0_");
    
    createFunction("void nvar::operator%(nvar*, nvar*, long)",
#ifdef __APPLE__
                   "_ZNK3neu4nvarrmEx");
#else
                  "_ZNK3neu4nvarrmEl");
#endif
    
    createFunction("void nvar::operator%(nvar*, nvar*, double)",
                   "_ZNK3neu4nvarrmEd");
    
    createFunction("void nvar::operator%(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvarrmERKS0_");
    
    createFunction("nvar* nvar::operator+=(nvar*, long)",
#ifdef __APPLE__
                   "_ZN3neu4nvarpLEx");
#else
                   "_ZN3neu4nvarpLEl");
#endif
    
    createFunction("nvar* nvar::operator+=(nvar*, double)",
                   "_ZN3neu4nvarpLEd");
    
    createFunction("nvar* nvar::operator+=(nvar*, nvar*)",
                   "_ZN3neu4nvarpLERKS0_");
    
    createFunction("nvar* nvar::operator-=(nvar*, long)",
#ifdef __APPLE__
                   "_ZN3neu4nvarmIEx");
#else
                   "_ZN3neu4nvarmIEl");
#endif
    
    createFunction("nvar* nvar::operator-=(nvar*, double)",
                   "_ZN3neu4nvarmIEd");
    
    createFunction("nvar* nvar::operator-=(nvar*, nvar*)",
                   "_ZN3neu4nvarmIERKS0_");
    
    createFunction("nvar* nvar::operator*=(nvar*, long)",
#ifdef __APPLE__
                   "_ZN3neu4nvarmLEx");
#else
                   "_ZN3neu4nvarmLEl");
#endif
    
    createFunction("nvar* nvar::operator*=(nvar*, double)",
                   "_ZN3neu4nvarmLEd");
    
    createFunction("nvar* nvar::operator*=(nvar*, nvar*)",
                   "_ZN3neu4nvarmLERKS0_");
    
    createFunction("nvar* nvar::operator/=(nvar*, long)",
#ifdef __APPLE__
                   "_ZN3neu4nvardVEx");
#else
                   "_ZN3neu4nvardVEl");
#endif
    
    createFunction("nvar* nvar::operator/=(nvar*, double)",
                   "_ZN3neu4nvardVEd");
    
    createFunction("nvar* nvar::operator/=(nvar*, nvar*)",
                   "_ZN3neu4nvardVERKS0_");
    
    createFunction("nvar* nvar::operator%=(nvar*, long)",
#ifdef __APPLE__
                   "_ZN3neu4nvarrMEx");
#else
                   "_ZN3neu4nvarrMEl");
#endif
    
    createFunction("nvar* nvar::operator%=(nvar*, double)",
                   "_ZN3neu4nvarrMEd");
    
    createFunction("nvar* nvar::operator%=(nvar*, nvar*)",
                   "_ZN3neu4nvarrMERKS0_");
    
    createFunction("nvar* nvar::operator%=(nvar*, nvar*)",
                   "_ZN3neu4nvarrMERKS0_");
    
    createFunction("void nvar::operator!(nvar*, nvar*)",
                   "_ZNK3neu4nvarntEv");
    
    createFunction("void nvar::operator&&(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvaraaERKS0_");
    
    createFunction("void nvar::operator||(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvarooERKS0_");
    
    createFunction("void nvar::operator==(nvar*, nvar*, long)",
#ifdef __APPLE__
                   "_ZNK3neu4nvareqEx");
#else
                   "_ZNK3neu4nvareqEl");
#endif
    
    createFunction("void nvar::operator==(nvar*, nvar*, double)",
                   "_ZNK3neu4nvareqEd");
    
    createFunction("void nvar::operator==(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvareqERKS0_");

    createFunction("void nvar::operator!=(nvar*, nvar*, long)",
#ifdef __APPLE__
                   "_ZNK3neu4nvarneEx");
#else
                   "_ZNK3neu4nvarneExl");
#endif
    
    createFunction("void nvar::operator!=(nvar*, nvar*, double)",
                   "_ZNK3neu4nvarneEd");
    
    createFunction("void nvar::operator!=(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvarneERKS0_");
    
    createFunction("void nvar::operator<(nvar*, nvar*, long)",
#ifdef __APPLE__
                   "_ZNK3neu4nvarltEx");
#else
                   "_ZNK3neu4nvarltEl");
#endif
    
    createFunction("void nvar::operator<(nvar*, nvar*, double)",
                   "_ZNK3neu4nvarltEd");
    
    createFunction("void nvar::operator<(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvarltERKS0_");
    
    createFunction("void nvar::operator<=(nvar*, nvar*, long)",
#ifdef __APPLE__
                   "_ZNK3neu4nvarleEx");
#else
                   "_ZNK3neu4nvarleEl");
#endif
    
    createFunction("void nvar::operator<=(nvar*, nvar*, double)",
                   "_ZNK3neu4nvarleEd");
    
    createFunction("void nvar::operator<=(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvarleERKS0_");
    
    createFunction("void nvar::operator>(nvar*, nvar*, long)",
#ifdef __APPLE__
                   "_ZNK3neu4nvargtEx");
#else
                   "_ZNK3neu4nvargtEl");
#endif
    
    createFunction("void nvar::operator>(nvar*, nvar*, double)",
                   "_ZNK3neu4nvargtEd");
    
    createFunction("void nvar::operator>(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvargtERKS0_");
    
    createFunction("void nvar::operator>=(nvar*, nvar*, long)",
#ifdef __APPLE__
                   "_ZNK3neu4nvargeEx");
#else
                   "_ZNK3neu4nvargeEl");
#endif
    
    createFunction("void nvar::operator>=(nvar*, nvar*, double)",
                   "_ZNK3neu4nvargeEd");
    
    createFunction("void nvar::operator>=(nvar*, nvar*, nvar*)",
                   "_ZNK3neu4nvargeERKS0_");
    
    createFunction("nvar* nvar::operator[](nvar*, int)",
                   "_ZN3neu4nvarixEi");
    
    createFunction("nvar* nvar::operator[](nvar*, nvar*)",
                   "_ZN3neu4nvarixERKS0_");
    
    createFunction("long nvar::size(nvar*)",
                   "_ZNK3neu4nvar4sizeEv");
    
    createFunction("void nvar::operator-(nvar*, nvar*)",
                   "_ZNK3neu4nvarngEv");
    
    createFunction("void nvar::pow(nvar*, nvar*, nvar*, void*)",
                   "_ZN3neu4nvar3powERKS0_S2_PNS_7NObjectE");
    
    createFunction("void nvar::sqrt(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar4sqrtERKS0_PNS_7NObjectE");
    
    createFunction("void nvar::exp(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar3expERKS0_PNS_7NObjectE");
    
    createFunction("void nvar::log(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar3logERKS0_PNS_7NObjectE");
    
    createFunction("void nvar::floor(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar5floorERKS0_");

    createFunction("nvar* nvar::put(nvar*, char*)",
                   "_ZN3neu4nvarclEPKc");
    
    createFunction("nvar* nvar::put(nvar*, nvar*)",
                   "_ZN3neu4nvarclERKS0_");
    
    createFunction("void nvar::pushBack(nvar*, nvar*)",
                   "_ZN3neu4nvar8pushBackERKS0_");
    
    createFunction("void nvar::intoVector(nvar*)",
                   "_ZN3neu4nvar10intoVectorEv");
    
    createFunction("void nvar::intoList(nvar*)",
                   "_ZN3neu4nvar8intoListEv");

    createFunction("void nvar::intoQueue(nvar*)",
                   "_ZN3neu4nvar9intoQueueEv");
    
    createFunction("void nvar::intoSet(nvar*)",
                   "_ZN3neu4nvar7intoSetEv");
    
    createFunction("void nvar::intoHashSet(nvar*)",
                   "_ZN3neu4nvar11intoHashSetEv");
    
    createFunction("void nvar::intoMap(nvar*)",
                   "_ZN3neu4nvar7intoMapEv");
    
    createFunction("void nvar::intoHashMap(nvar*)",
                   "_ZN3neu4nvar11intoHashMapEv");
    
    createFunction("void nvar::intoMultimap(nvar*)",
                   "_ZN3neu4nvar13intoMultimapEv");
    
    createFunction("bool nvar::toBool(nvar*)",
                   "_ZNK3neu4nvar6toBoolEv");
    
    createFunction("void nvar::keys(nvar*, nvar*)",
                   "_ZNK3neu4nvar4keysEv");

    createFunction("void nvar::addKey(nvar*, nvar*)",
                   "__ZN3neu4nvar6addKeyERKS0_");
    
    createFunction("void nvar::pushFront(nvar*, nvar*)",
                   "_ZN3neu4nvar9pushFrontERKS0_");
    
    createFunction("void nvar::popBack(nvar*, nvar*)",
                   "_ZN3neu4nvar7popBackEv");
    
    createFunction("void nvar::popFront(nvar*, nvar*)",
                   "_ZN3neu4nvar8popFrontEv");
    
    createFunction("bool nvar::hasKey(nvar*, nvar*)",
                   "_ZNK3neu4nvar6hasKeyERKS0_");
    
    createFunction("void nvar::insert(nvar*, long, nvar*)",
                   "_ZN3neu4nvar6insertEmRKS0_");
    
    createFunction("void nvar::clear(nvar*)",
                   "_ZN3neu4nvar5clearEv");
    
    createFunction("bool nvar::empty(nvar*)",
                   "_ZNK3neu4nvar5emptyEv");
    
    createFunction("nvar* nvar::back(nvar*)",
                   "_ZN3neu4nvar4backEv");
    
    createFunction("nvar* nvar::get(nvar*, nvar*)",
                   "_ZN3neu4nvar3getERKS0_");
    
    createFunction("nvar* nvar::get(nvar*, nvar*, nvar*)",
                   "_ZN3neu4nvar3getERKS0_RS0_");
    
    createFunction("void nvar::erase(nvar*, nvar*)",
                   "_ZN3neu4nvar5eraseERKS0_");
    
    createFunction("void nvar::cos(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar3cosERKS0_PNS_7NObjectE");
    
    createFunction("void nvar::acos(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar4acosERKS0_PNS_7NObjectE");
    
    createFunction("void nvar::cosh(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar4coshERKS0_PNS_7NObjectE");
    
    createFunction("void nvar::sin(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar3sinERKS0_PNS_7NObjectE");
    
    createFunction("void nvar::asin(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar4asinERKS0_PNS_7NObjectE");
    
    createFunction("void nvar::sinh(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar4sinhERKS0_PNS_7NObjectE");
    
    createFunction("void nvar::tan(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar3tanERKS0_PNS_7NObjectE");
    
    createFunction("void nvar::atan(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar4atanERKS0_PNS_7NObjectE");
    
    createFunction("void nvar::tanh(nvar*, nvar*, void*)",
                   "_ZN3neu4nvar4tanhERKS0_PNS_7NObjectE");
    
    createFunction("void nvar::merge(nvar*, nvar*)",
                   "_ZN3neu4nvar5mergeERKS0_");
    
    createFunction("void nvar::outerMerge(nvar*, nvar*)",
                   "_ZN3neu4nvar10outerMergeERKS0_");
    
    delete compiler_;
  }
  
  void Global::createFunction(const nstr& fs, const nstr& mf){
    nvar fv = nvar::parseFuncSpec(fs.c_str());
    
    Function* f =
    compiler_->createFunction(mf,
                              compiler_->type(fv["ret"]),
                              compiler_->typeVec(fv["args"]));
    
    functionMap_[fs] = f;
  }
  
  Function* Global::getFunction(const nstr& f){
    auto itr = functionMap_.find(f);
    assert(itr != functionMap_.end());
    return itr->second;
  }
  
} // end namespace

namespace neu{
  
  class NPLModule_{
  public:
    NPLModule_(NPLModule* o)
    : o_(o),
    context_(getGlobalContext()),
    module_("module", context_),
    estr_(&cerr){
      
      initGlobal();
      
      engine_ = EngineBuilder(&module_).setUseMCJIT(true).create();
    }
    
    ~NPLModule_(){
      
    }
    
    bool compile(const nvar& code){
      NPLCompiler compiler(module_, functionMap_, *estr_);
      
      return compiler.compileTop(code);
    }
    
    void getFunc(const nvar& func, NPLFunc* f){
      auto itr = functionMap_.find(func);
      
      if(itr == functionMap_.end()){
        NERROR("invalid function: " + func);
      }
      
      f->fp = (NPLFunc::FP)engine_->getPointerToFunction(itr->second);
      
      functionPtrMap_[f->fp] = itr->second;
    }
    
    void release(NPLFunc* f){
      auto itr = functionPtrMap_.find(f->fp);
      
      if(itr == functionPtrMap_.end()){
        NERROR("invalid function");
      }
      
      engine_->freeMachineCodeForFunction(itr->second);
    }
    
    void initGlobal(){
      _mutex.lock();
      if(!_global){
        _global = new Global;
        _global->init();
      }
      _mutex.unlock();
    }
    
    void setErrorStream(ostream& estr){
      estr_ = &estr;
    }
    
  private:
    typedef NMap<NPLFunc::FP, Function*> FunctionPtrMap_;
    
    NPLModule* o_;
    
    LLVMContext& context_;
    Module module_;
    ExecutionEngine* engine_;
    FunctionMap functionMap_;
    FunctionPtrMap_ functionPtrMap_;
    ostream* estr_;
  };
  
} // end namespace neu

NPLModule::NPLModule(){
  x_ = new NPLModule_(this);
}

NPLModule::~NPLModule(){
  delete x_;
}

bool NPLModule::compile(const nvar& code){
  return x_->compile(code);
}

void NPLModule::getFunc(const nvar& func, NPLFunc* f){
  x_->getFunc(func, f);
}

void NPLModule::release(NPLFunc* f){
  x_->release(f);
}

void NPLModule::setErrorStream(ostream& estr){
  x_->setErrorStream(estr);
}

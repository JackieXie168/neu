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

#include <neu/NMGenerator.h>

#include <neu/NObject.h>
#include <neu/NSys.h>
#include <neu/NMObject.h>

using namespace std;
using namespace neu;

namespace{
  
  enum SymbolKey{
    SKEY_NO_KEY,
    SKEY_Pi,
    SKEY_Eu,
    SKEY_Inf,
    SKEY_NegInf
  };
  
  enum FunctionKey{
    FKEY_NO_KEY,
    FKEY_Add_2,
    FKEY_Sub_2,
    FKEY_Mul_2,
    FKEY_Div_2,
    FKEY_Pow_2,
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
    FKEY_Set_2,
    FKEY_Exp_1,
    FKEY_Sqrt_1,
    FKEY_Abs_1,
    FKEY_Log_1,
    FKEY_Cos_1,
    FKEY_Acos_1,
    FKEY_Cosh_1,
    FKEY_Sin_1,
    FKEY_Asin_1,
    FKEY_Sinh_1,
    FKEY_Tan_1,
    FKEY_Cot_1,
    FKEY_Atan_1,
    FKEY_Tanh_1,
    FKEY_Csc_1,
    FKEY_Sec_1,
    FKEY_Sech_1,
    FKEY_Csch_1,
    FKEY_Asec_1,
    FKEY_Acsc_1,
    FKEY_Asech_1,
    FKEY_Acsch_1,
    FKEY_Coth_1,
    FKEY_Acoth_1,
    FKEY_Atanh_1,
    FKEY_Asinh_1,
    FKEY_Acosh_1,
    FKEY_Round_1,
    FKEY_Integrate_2,
    FKEY_Derivative_2,
    FKEY_Derivative_3,
    FKEY_Simplify_1,
    FKEY_Simplify_2,
    FKEY_FullSimplify_1,
    FKEY_FullSimplify_2,
    FKEY_Solve_1,
    FKEY_Solve_2,
    FKEY_Coefficient_2,
    FKEY_Coefficient_3,
    FKEY_IsPrime_1,
    FKEY_GCD_n,
    FKEY_LCM_n,
    FKEY_Factorial_1,
    FKEY_Factor_1,
    FKEY_Factor_2,
    FKEY_FactorInteger_1,
    FKEY_FactorInteger_2,
    FKEY_Limit_2,
    FKEY_Plot_2,
    FKEY_Plot_3,
    FKEY_Plot3D_3,
    FKEY_Plot3D_4,
    FKEY_ListPlot_1,
    FKEY_ListPlot_2,
    FKEY_ListLinePlot_1,
    FKEY_ListLinePlot_2,
    FKEY_GraphPlot_1,
    FKEY_GraphPlot_2,
    FKEY_TreePlot_1,
    FKEY_TreePlot_2,
    FKEY_TreePlot_3,
    FKEY_Histogram_1,
    FKEY_Histogram_2,
    FKEY_Histogram_3,
    FKEY_ContourPlot_3,
    FKEY_ParametricPlot_2,
    FKEY_ParametricPlot_3,
    FKEY_ParametricPlot3D_2,
    FKEY_ParametricPlot3D_3,
    FKEY_DensityPlot_3,
    FKEY_PolarPlot_2,
    FKEY_TraditionalForm_1,
    FKEY_TreeForm_1,
    FKEY_MatrixForm_1,
    FKEY_GrayLevel_1,
    FKEY_Numeric_1,
    FKEY_Numeric_2,
    FKEY_TrigExpand_1,
    FKEY_TrigReduce_1,
    FKEY_TrigToExp_1,
    FKEY_ExpToTrig_1,
    FKEY_Expand_1,
    FKEY_PowerExpand_1,
    FKEY_LogicalExpand_1,
    FKEY_Together_1,
    FKEY_Apart_1,
    FKEY_Cancel_1,
    FKEY_Numerator_1,
    FKEY_Denominator_1,
    FKEY_Table_2,
    FKEY_Nest_3,
    FKEY_NRoots_2,
    FKEY_NSolve_2,
    FKEY_NSolve_3,
    FKEY_FindRoot_2,
    FKEY_Flatten_1,
    FKEY_Flatten_2,
    FKEY_Flatten_3,
    FKEY_NIntegrate_n,
    FKEY_Minimize_1,
    FKEY_Minimize_2,
    FKEY_Minimize_3,
    FKEY_Maximize_1,
    FKEY_Maximize_2,
    FKEY_Maximize_3,
    FKEY_NMinimize_2,
    FKEY_NMaximize_2,
    FKEY_Sum_n,
    FKEY_Product_n,
    FKEY_Composition_n,
    FKEY_Fit_3,
    FKEY_Prime_1,
    FKEY_InterpolatingPolynomial_2,
    FKEY_Refine_1,
    FKEY_Refine_2,
    FKEY_FindMinimum_2,
    FKEY_FindMaximum_2,
    FKEY_Correlation_1,
    FKEY_Correlation_2,
    FKEY_Correlation_3,
    FKEY_ImportData_1,
    FKEY_ImportData_2,
    FKEY_ExportData_2,
    FKEY_ExportData_3,
    FKEY_FindSequenceFunction_1,
    FKEY_FindSequenceFunction_2,
    FKEY_IdentityMatrix_1,
    FKEY_DiagonalMatrix_1,
    FKEY_DiagonalMatrix_2,
    FKEY_DiagonalMatrix_3,
    FKEY_ContinuedFraction_1
  };
  
  typedef NMap<nstr, SymbolKey> SymbolMap;
  
  typedef NMap<pair<nstr, int>,
  pair<FunctionKey, NMGenerator::Type>> FunctionMap;
  
  static SymbolMap _symbolMap;
  
  static FunctionMap _functionMap;
  
  static void _initSymbolMap(){
    _symbolMap["Pi"] = SKEY_Pi;
    _symbolMap["Eu"] = SKEY_Eu;
    _symbolMap["Inf"] = SKEY_Inf;
    _symbolMap["NegInf"] = SKEY_NegInf;
  }
  
  static void _initFunctionMap(){
    auto S = NMGenerator::Supported;
    auto R = NMGenerator::Requested;
    
    _functionMap[{"Add", 2}] = {FKEY_Add_2, S};
    _functionMap[{"Sub", 2}] = {FKEY_Sub_2, S};
    _functionMap[{"Mul", 2}] = {FKEY_Mul_2, S};
    _functionMap[{"Div", 2}] = {FKEY_Div_2, S};
    _functionMap[{"Pow", 2}] = {FKEY_Pow_2, S};
    _functionMap[{"Mod", 2}] = {FKEY_Mod_2, S};
    _functionMap[{"Set", 2}] = {FKEY_Set_2, S};
    _functionMap[{"LT", 2}] = {FKEY_LT_2, S};
    _functionMap[{"GT", 2}] = {FKEY_GT_2, S};
    _functionMap[{"LE", 2}] = {FKEY_LE_2, S};
    _functionMap[{"GE", 2}] = {FKEY_GE_2, S};
    _functionMap[{"EQ", 2}] = {FKEY_EQ_2, S};
    _functionMap[{"NE", 2}] = {FKEY_NE_2, S};
    _functionMap[{"Not", 1}] = {FKEY_Not_1, S};
    _functionMap[{"Neg", 1}] = {FKEY_Neg_1, S};
    _functionMap[{"And", 2}] = {FKEY_And_2, S};
    _functionMap[{"Or", 2}] = {FKEY_Or_2, S};
    _functionMap[{"Inc", 1}] = {FKEY_Inc_1, S};
    _functionMap[{"PostInc", 1}] = {FKEY_PostInc_1, S};
    _functionMap[{"Dec", 1}] = {FKEY_Dec_1, S};
    _functionMap[{"PostDec", 1}] = {FKEY_PostDec_1, S};
    _functionMap[{"AddBy", 2}] = {FKEY_AddBy_2, S};
    _functionMap[{"SubBy", 2}] = {FKEY_SubBy_2, S};
    _functionMap[{"MulBy", 2}] = {FKEY_MulBy_2, S};
    _functionMap[{"DivBy", 2}] = {FKEY_DivBy_2, S};
    _functionMap[{"ModBy", 2}] = {FKEY_ModBy_2, S};
    _functionMap[{"Exp", 1}] = {FKEY_Exp_1, S};
    _functionMap[{"Sqrt", 1}] = {FKEY_Sqrt_1, S};
    _functionMap[{"Abs", 1}] = {FKEY_Abs_1, S};
    _functionMap[{"Log", 1}] = {FKEY_Log_1, S};
    _functionMap[{"Cos", 1}] = {FKEY_Cos_1, S};
    _functionMap[{"Acos", 1}] = {FKEY_Acos_1, S};
    _functionMap[{"Cosh", 2}] = {FKEY_Cosh_1, S};
    _functionMap[{"Sin", 1}] = {FKEY_Sin_1, S};
    _functionMap[{"Asin", 1}] = {FKEY_Asin_1, S};
    _functionMap[{"Sinh", 2}] = {FKEY_Sinh_1, S};
    _functionMap[{"Tan", 1}] = {FKEY_Tan_1, S};
    _functionMap[{"Cot", 1}] = {FKEY_Cot_1, R};
    _functionMap[{"Atan", 1}] = {FKEY_Atan_1, S};
    _functionMap[{"Tanh", 2}] = {FKEY_Tanh_1, S};
    _functionMap[{"Csc", 1}] = {FKEY_Csc_1, R};
    _functionMap[{"Sec", 1}] = {FKEY_Sec_1, R};
    _functionMap[{"Sech", 1}] = {FKEY_Sech_1, R};
    _functionMap[{"Csch", 1}] = {FKEY_Csch_1, R};
    _functionMap[{"Asec", 1}] = {FKEY_Asec_1, R};
    _functionMap[{"Acsc", 1}] = {FKEY_Acsc_1, R};
    _functionMap[{"Asech", 1}] = {FKEY_Asech_1, R};
    _functionMap[{"Acsch", 1}] = {FKEY_Acsch_1, R};
    _functionMap[{"Coth", 1}] = {FKEY_Coth_1, R};
    _functionMap[{"Acoth", 1}] = {FKEY_Acoth_1, R};
    _functionMap[{"Atanh", 1}] = {FKEY_Atanh_1, R};
    _functionMap[{"Asinh", 1}] = {FKEY_Asinh_1, R};
    _functionMap[{"Acosh", 1}] = {FKEY_Acosh_1, R};
    _functionMap[{"Round", 1}] = {FKEY_Round_1, R};
    _functionMap[{"Integrate", 2}] = {FKEY_Integrate_2, R};
    _functionMap[{"Derivative", 2}] = {FKEY_Derivative_2, R};
    _functionMap[{"Derivative", 3}] = {FKEY_Derivative_3, R};
    _functionMap[{"Simplify", 1}] = {FKEY_Simplify_1, R};
    _functionMap[{"Simplify", 2}] = {FKEY_Simplify_2, R};
    _functionMap[{"FullSimplify", 1}] = {FKEY_FullSimplify_1, R};
    _functionMap[{"FullSimplify", 2}] = {FKEY_FullSimplify_2, R};
    _functionMap[{"Solve", 1}] = {FKEY_Solve_1, R};
    _functionMap[{"Solve", 2}] = {FKEY_Solve_2, R};
    _functionMap[{"Coefficient", 2}] = {FKEY_Coefficient_2, R};
    _functionMap[{"Coefficient", 3}] = {FKEY_Coefficient_3, R};
    _functionMap[{"IsPrime", 2}] = {FKEY_IsPrime_1, R};
    _functionMap[{"GCD", -1}] = {FKEY_GCD_n, R};
    _functionMap[{"LCM", -1}] = {FKEY_LCM_n, R};
    _functionMap[{"Factorial", 1}] = {FKEY_Factorial_1, R};
    _functionMap[{"Factor", 1}] = {FKEY_Factor_1, R};
    _functionMap[{"Factor", 2}] = {FKEY_Factor_2, R};
    _functionMap[{"FactorInteger", 1}] = {FKEY_FactorInteger_1, R};
    _functionMap[{"FactorInteger", 2}] = {FKEY_FactorInteger_2, R};
    _functionMap[{"Limit", 2}] = {FKEY_Limit_2, R};
    _functionMap[{"Plot", 2}] = {FKEY_Plot_2, R};
    _functionMap[{"Plot", 3}] = {FKEY_Plot_3, R};
    _functionMap[{"Plot3D", 3}] = {FKEY_Plot3D_3, R};
    _functionMap[{"Plot3D", 4}] = {FKEY_Plot3D_4, R};
    _functionMap[{"ListPlot", 1}] = {FKEY_ListPlot_1, R};
    _functionMap[{"ListPlot", 2}] = {FKEY_ListPlot_2, R};
    _functionMap[{"ListLinePlot", 1}] = {FKEY_ListLinePlot_1, R};
    _functionMap[{"ListLinePlot", 2}] = {FKEY_ListLinePlot_2, R};
    _functionMap[{"GraphPlot", 1}] = {FKEY_GraphPlot_1, R};
    _functionMap[{"GraphPlot", 2}] = {FKEY_GraphPlot_2, R};
    _functionMap[{"TreePlot", 1}] = {FKEY_TreePlot_1, R};
    _functionMap[{"TreePlot", 2}] = {FKEY_TreePlot_2, R};
    _functionMap[{"TreePlot", 3}] = {FKEY_TreePlot_3, R};
    _functionMap[{"Histogram", 1}] = {FKEY_Histogram_1, R};
    _functionMap[{"Histogram", 2}] = {FKEY_Histogram_2, R};
    _functionMap[{"Histogram", 3}] = {FKEY_Histogram_3, R};
    _functionMap[{"ContourPlot", 3}] = {FKEY_ContourPlot_3, R};
    _functionMap[{"ParametricPlot", 2}] = {FKEY_ParametricPlot_2, R};
    _functionMap[{"ParametricPlot", 3}] = {FKEY_ParametricPlot_3, R};
    _functionMap[{"ParametricPlot3D", 2}] = {FKEY_ParametricPlot3D_2, R};
    _functionMap[{"ParametricPlot3D", 2}] = {FKEY_ParametricPlot3D_3, R};
    _functionMap[{"DensityPlot", 3}] = {FKEY_DensityPlot_3, R};
    _functionMap[{"PolarPlot", 2}] = {FKEY_PolarPlot_2, R};
    _functionMap[{"TraditionalForm", 1}] = {FKEY_TraditionalForm_1, R};
    _functionMap[{"TreeForm", 1}] = {FKEY_TreeForm_1, R};
    _functionMap[{"MatrixForm", 1}] = {FKEY_MatrixForm_1, R};
    _functionMap[{"GrayLevel", 1}] = {FKEY_GrayLevel_1, R};
    _functionMap[{"Numeric", 1}] = {FKEY_Numeric_1, R};
    _functionMap[{"Numeric", 2}] = {FKEY_Numeric_2, R};
    _functionMap[{"TrigExpand", 1}] = {FKEY_TrigExpand_1, R};
    _functionMap[{"TrigReduce", 1}] = {FKEY_TrigReduce_1, R};
    _functionMap[{"TrigToExp", 1}] = {FKEY_TrigToExp_1, R};
    _functionMap[{"ExpToTrig", 1}] = {FKEY_ExpToTrig_1, R};
    _functionMap[{"Expand", 1}] = {FKEY_Expand_1, R};
    _functionMap[{"PowerExpand", 1}] = {FKEY_PowerExpand_1, R};
    _functionMap[{"LogicalExpand", 1}] = {FKEY_LogicalExpand_1, R};
    _functionMap[{"Together", 1}] = {FKEY_Together_1, R};
    _functionMap[{"Apart", 1}] = {FKEY_Apart_1, R};
    _functionMap[{"Cancel", 1}] = {FKEY_Cancel_1, R};
    _functionMap[{"Numerator", 1}] = {FKEY_Numerator_1, R};
    _functionMap[{"Denominator", 1}] = {FKEY_Denominator_1, R};
    _functionMap[{"Table", 2}] = {FKEY_Table_2, R};
    _functionMap[{"Nest", 3}] = {FKEY_Nest_3, R};
    _functionMap[{"NRoots", 2}] = {FKEY_NRoots_2, R};
    _functionMap[{"NSolve", 2}] = {FKEY_NSolve_2, R};
    _functionMap[{"NSolve", 3}] = {FKEY_NSolve_3, R};
    _functionMap[{"FindRoot", 2}] = {FKEY_FindRoot_2, R};
    _functionMap[{"Flatten", 1}] = {FKEY_Flatten_1, R};
    _functionMap[{"Flatten", 2}] = {FKEY_Flatten_2, R};
    _functionMap[{"Flatten", 3}] = {FKEY_Flatten_3, R};
    _functionMap[{"NIntegrate", -1}] = {FKEY_NIntegrate_n, R};
    _functionMap[{"Minimize", 1}] = {FKEY_Minimize_1, R};
    _functionMap[{"Minimize", 2}] = {FKEY_Minimize_2, R};
    _functionMap[{"Minimize", 3}] = {FKEY_Minimize_3, R};
    _functionMap[{"Maximize", 1}] = {FKEY_Maximize_1, R};
    _functionMap[{"Maximize", 2}] = {FKEY_Maximize_2, R};
    _functionMap[{"Maximize", 3}] = {FKEY_Maximize_3, R};
    _functionMap[{"NMinimize", 2}] = {FKEY_NMinimize_2, R};
    _functionMap[{"NMaximize", 2}] = {FKEY_NMaximize_2, R};
    _functionMap[{"Sum", -1}] = {FKEY_Sum_n, R};
    _functionMap[{"Product", -1}] = {FKEY_Product_n, R};
    _functionMap[{"Composition", -1}] = {FKEY_Composition_n, R};
    _functionMap[{"Fit", 3}] = {FKEY_Fit_3, R};
    _functionMap[{"Prime", 1}] = {FKEY_Prime_1, R};
    _functionMap[{"Refine", 1}] = {FKEY_Refine_1, R};
    _functionMap[{"Refine", 2}] = {FKEY_Refine_2, R};
    _functionMap[{"FindMinimum", 2}] = {FKEY_FindMinimum_2, R};
    _functionMap[{"FindMaximum", 2}] = {FKEY_FindMaximum_2, R};
    _functionMap[{"Correlation", 1}] = {FKEY_Correlation_1, R};
    _functionMap[{"Correlation", 2}] = {FKEY_Correlation_2, R};
    _functionMap[{"Correlation", 3}] = {FKEY_Correlation_3, R};
    _functionMap[{"ImportData", 1}] = {FKEY_ImportData_1, R};
    _functionMap[{"ImportData", 2}] = {FKEY_ImportData_2, R};
    _functionMap[{"ExportData", 2}] = {FKEY_ExportData_2, R};
    _functionMap[{"ExportData", 3}] = {FKEY_ExportData_3, R};
    _functionMap[{"IdentityMatrix", 1}] = {FKEY_IdentityMatrix_1, R};
    _functionMap[{"DiagonalMatrix", 1}] = {FKEY_DiagonalMatrix_1, R};
    _functionMap[{"DiagonalMatrix", 2}] = {FKEY_DiagonalMatrix_2, R};
    _functionMap[{"DiagonalMatrix", 3}] = {FKEY_DiagonalMatrix_3, R};
    _functionMap[{"ContinuedFraction", 1}] = {FKEY_ContinuedFraction_1, R};
    
    _functionMap[{"InterpolatingPolynomial", 2}] =
    {FKEY_InterpolatingPolynomial_2, R};
    
    _functionMap[{"FindSequenceFunction", 1}] =
    {FKEY_FindSequenceFunction_1, R};
    
    _functionMap[{"FindSequenceFunction", 2}] =
    {FKEY_FindSequenceFunction_2, R};
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
  
  class NMGenerator_{
  public:
    NMGenerator_(NMGenerator* o, NMObject* obj)
    : o_(o),
    obj_(obj){}
    
    ~NMGenerator_(){}
    
    void generate(ostream& ostr, const nvar& v){
      emitExpression(ostr, v);
      ostr << " // FullForm\n";
    }
    
    FunctionKey getFunctionKey(const nvar& f){
      auto itr = _functionMap.find({f.str(), f.size()});
      
      if(itr == _functionMap.end()){
        itr = _functionMap.find({f.str(), -1});
      }
      
      if(itr == _functionMap.end()){
        return FKEY_NO_KEY;
      }
      
      return itr->second.first;
    }
    
    SymbolKey getSymbolKey(const nvar& f){
      auto itr = _symbolMap.find(f);
      
      if(itr == _symbolMap.end()){
        return SKEY_NO_KEY;
      }
      
      return itr->second;
    }
    
    void emitFunc(ostream& ostr, const nvar& n, const nstr& func=""){
      if(func.empty()){
        ostr << n.str();
      }
      else{
        ostr << func;
      }
      
      ostr << "[";
      
      size_t size = n.size();
      for(size_t i = 0; i < size; ++i){
        if(i > 0){
          ostr << ", ";
        }
        emitExpression(ostr, n[i]);
      }
      
      ostr << "]";
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
    
    bool emitSequence(ostream& ostr, const nvar& n){
      size_t size = n.size();

      if(size == 0){
        return false;
      }
      
      for(size_t i = 0; i < size; ++i){
        if(i > 0){
          ostr << ", ";
        }
        emitExpression(ostr, n[i]);
      }
      
      return true;
    }
    
    void emitMap(ostream& ostr, const nvar& n, bool first=true){
      const nmap& m = n;

      for(auto& itr : m){
        if(first){
          first = false;
        }
        else{
          ostr << ", ";
        }
      
        emitExpression(ostr, itr.first);
        ostr << "-> ";
        emitExpression(ostr, itr.second);
      }
    }

    void emitMultimap(ostream& ostr, const nvar& n, bool first=true){
      const nmmap& m = n;
      
      for(auto& itr : m){
        if(first){
          first = false;
        }
        else{
          ostr << ", ";
        }
        
        emitExpression(ostr, itr.first);
        ostr << "-> ";
        emitExpression(ostr, itr.second);
      }
    }
    
    void emitPDF(ostream& ostr, const nvar& n){
      nstr p = NSys::tempFilePath("pdf");
      
      ostr << "Export[\"" << p << "\", ";
      emitFunc(ostr, n);
      ostr << "]";
    }
    
    void emitExpression(ostream& ostr,
                        const nvar& n,
                        int prec=100){
      switch(n.type()){
        case nvar::False:
          ostr << "False";
          return;
        case nvar::True:
          ostr << "True";
          return;
        case nvar::Real:
          ostr << n.real().toStr(false);
          return;
        case nvar::Rational:
        case nvar::Integer:
        case nvar::String:
          ostr << n;
          return;
        case nvar::Float:{
          nstr s = n.toStr(false);
          s.findReplace("e", "`*^");
          ostr << s;
          return;
        }
        case nvar::List:
        case nvar::Vector:{
          ostr << "{";
          emitSequence(ostr, n);
          ostr << "}";
          return;
        }
        case nvar::Map:{
          ostr << "{";
          emitMap(ostr, n);
          ostr << "}";
          return;
        }
        case nvar::Multimap:{
          ostr << "{";
          emitMultimap(ostr, n);
          ostr << "}";
          return;
        }
        case nvar::SequenceMap:{
          ostr << "{";
          bool first = emitSequence(ostr, n);

          if(n.hasMap()){
            emitMap(ostr, n, first);
          }
          else{
            emitMultimap(ostr, n, first);
          }
          ostr << "}";
          return;
        }
        case nvar::Symbol:{
          SymbolKey key = getSymbolKey(n);
          switch(key){
            case SKEY_Pi:
              ostr << "Pi";
              return;
            case SKEY_Eu:
              ostr << "E";
              return;
            case SKEY_Inf:
              ostr << "Infinity";
              return;
            case SKEY_NegInf:
              ostr << "-Infinity";
              return;
            default:{
              nvar p = obj_->run(n);
              if(p.isSymbol() && p.str() == n.str()){
                ostr << p;
              }
              else{
                emitExpression(ostr, p);
              }
              
              return;
            }
          }
        }
        case nvar::Function:
          break;
        default:
          cout << "type is: " << int(n.type()) << endl;
          NERROR("invalid expression: " + n);
      }
      
      FunctionKey key = getFunctionKey(n);
      
      switch(key){
        case FKEY_NO_KEY:
          NERROR("invalid function: " + n);
        case FKEY_Log_1:
        case FKEY_Exp_1:
        case FKEY_Sqrt_1:
        case FKEY_Abs_1:
        case FKEY_Cos_1:
        case FKEY_Sin_1:
        case FKEY_Tan_1:
        case FKEY_Cot_1:
        case FKEY_Csc_1:
        case FKEY_Sec_1:
        case FKEY_Round_1:
        case FKEY_Simplify_1:
        case FKEY_Simplify_2:
        case FKEY_FullSimplify_1:
        case FKEY_FullSimplify_2:
        case FKEY_Solve_1:
        case FKEY_Solve_2:
        case FKEY_Coefficient_2:
        case FKEY_Coefficient_3:
        case FKEY_LCM_n:
        case FKEY_GCD_n:
        case FKEY_Factorial_1:
        case FKEY_Factor_1:
        case FKEY_Factor_2:
        case FKEY_FactorInteger_1:
        case FKEY_FactorInteger_2:
        case FKEY_GrayLevel_1:
        case FKEY_TrigExpand_1:
        case FKEY_TrigReduce_1:
        case FKEY_ExpToTrig_1:
        case FKEY_TrigToExp_1:
        case FKEY_Expand_1:
        case FKEY_PowerExpand_1:
        case FKEY_LogicalExpand_1:
        case FKEY_Together_1:
        case FKEY_Apart_1:
        case FKEY_Cancel_1:
        case FKEY_Numerator_1:
        case FKEY_Denominator_1:
        case FKEY_Table_2:
        case FKEY_Nest_3:
        case FKEY_NRoots_2:
        case FKEY_NSolve_2:
        case FKEY_NSolve_3:
        case FKEY_FindRoot_2:
        case FKEY_Flatten_1:
        case FKEY_Flatten_2:
        case FKEY_Flatten_3:
        case FKEY_Minimize_1:
        case FKEY_Minimize_2:
        case FKEY_Minimize_3:
        case FKEY_Maximize_1:
        case FKEY_Maximize_2:
        case FKEY_Maximize_3:
        case FKEY_NMinimize_2:
        case FKEY_NMaximize_2:
        case FKEY_NIntegrate_n:
        case FKEY_Sum_n:
        case FKEY_Product_n:
        case FKEY_Fit_3:
        case FKEY_Prime_1:
        case FKEY_InterpolatingPolynomial_2:
        case FKEY_Refine_1:
        case FKEY_Refine_2:
        case FKEY_FindMinimum_2:
        case FKEY_FindMaximum_2:
        case FKEY_Correlation_1:
        case FKEY_Correlation_2:
        case FKEY_Correlation_3:
        case FKEY_FindSequenceFunction_1:
        case FKEY_FindSequenceFunction_2:
        case FKEY_IdentityMatrix_1:
        case FKEY_DiagonalMatrix_1:
        case FKEY_DiagonalMatrix_2:
        case FKEY_DiagonalMatrix_3:
        case FKEY_ContinuedFraction_1:
        case FKEY_Cosh_1:
        case FKEY_Sinh_1:
        case FKEY_Tanh_1:
        case FKEY_Coth_1:
        case FKEY_Integrate_2:
        case FKEY_Limit_2:
          emitFunc(ostr, n);
          break;
        case FKEY_Plot_2:
        case FKEY_Plot_3:
        case FKEY_Plot3D_3:
        case FKEY_Plot3D_4:
        case FKEY_ListPlot_1:
        case FKEY_ListPlot_2:
        case FKEY_ListLinePlot_1:
        case FKEY_ListLinePlot_2:
        case FKEY_GraphPlot_1:
        case FKEY_GraphPlot_2:
        case FKEY_TreePlot_1:
        case FKEY_TreePlot_2:
        case FKEY_TreePlot_3:
        case FKEY_Histogram_1:
        case FKEY_Histogram_2:
        case FKEY_Histogram_3:
        case FKEY_PolarPlot_2:
        case FKEY_ContourPlot_3:
        case FKEY_ParametricPlot_2:
        case FKEY_ParametricPlot_3:
        case FKEY_ParametricPlot3D_2:
        case FKEY_ParametricPlot3D_3:
        case FKEY_DensityPlot_3:
        case FKEY_TraditionalForm_1:
        case FKEY_TreeForm_1:
        case FKEY_MatrixForm_1:
          emitPDF(ostr, n);
          break;
        case FKEY_Acos_1:
          emitFunc(ostr, n, "ArcCos");
          break;
        case FKEY_Asin_1:
          emitFunc(ostr, n, "ArcSin");
          break;
        case FKEY_Atan_1:
          emitFunc(ostr, n, "ArcTan");
          break;
        case FKEY_Asec_1:
          emitFunc(ostr, n, "ArcSec");
          break;
        case FKEY_Acsc_1:
          emitFunc(ostr, n, "ArcCsc");
          break;
        case FKEY_Asech_1:
          emitFunc(ostr, n, "ArcSech");
          break;
        case FKEY_Acsch_1:
          emitFunc(ostr, n, "ArcCsch");
          break;
        case FKEY_Acoth_1:
          emitFunc(ostr, n, "ArcCoth");
          break;
        case FKEY_Atanh_1:
          emitFunc(ostr, n, "ArcTanh");
          break;
        case FKEY_Asinh_1:
          emitFunc(ostr, n, "ArcSinh");
          break;
        case FKEY_Acosh_1:
          emitFunc(ostr, n, "ArcCosh");
          break;
        case FKEY_Numeric_1:
        case FKEY_Numeric_2:
          emitFunc(ostr, n, "N");
          break;
        case FKEY_ImportData_1:
        case FKEY_ImportData_2:
          emitFunc(ostr, "Import", n);
          break;
        case FKEY_ExportData_2:
        case FKEY_ExportData_3:
          emitFunc(ostr, "Export", n);
          break;
        case FKEY_Derivative_2:
        case FKEY_Derivative_3:
          emitFunc(ostr, n, "D");
          break;
        case FKEY_IsPrime_1:
          emitFunc(ostr, n, "PrimeQ");
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
        default:
          NERROR("function not implemented: " + n);
      }
    }
    
  private:
    NMGenerator* o_;
    NMObject* obj_;
  };
  
} // end namespace neu

const NMGenerator::Type NMGenerator::Supported;
const NMGenerator::Type NMGenerator::Requested;

NMGenerator::NMGenerator(NMObject* obj){
  x_ = new NMGenerator_(this, obj);
}

NMGenerator::~NMGenerator(){
  delete x_;
}

void NMGenerator::generate(std::ostream& ostr, const nvar& v){
  x_->generate(ostr, v);
}

NMGenerator::Type NMGenerator::type(const nvar& v){
  auto itr = _functionMap.find({v.str(), v.size()});
  
  if(itr == _functionMap.end()){
    itr = _functionMap.find({v.str(), -1});
  }
  
  if(itr == _functionMap.end()){
    return 0;
  }
  
  return itr->second.second;
}

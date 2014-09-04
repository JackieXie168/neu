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

#ifndef NEU_N_OBJECT_H
#define NEU_N_OBJECT_H

#include <neu/NObjectBase.h>
#include <neu/nvar.h>

namespace neu{
  
  class NScope;
  class NBroker;
  
  class NObject : public NObjectBase{
  public:
    static const uint32_t classId;
    
    NObject();
    
    NObject(NScope* sharedScope);
    
    NObject(NBroker* broker);
    
    ~NObject();
    
    static const uint32_t Delegated = 0x00000001;
    
    nvar remoteProcess(const nvar& v);
    
    nvar process(const nvar& v, uint32_t flags=0);
    
    virtual NFunc handle(const nvar& v, uint32_t flags=0);
    
    virtual bool instanceOf(uint32_t classId) const{
      return classId == NObject::classId;
    }
    
    void enableThreading();
    
    void setStrict(bool flag);
    
    void setExact(bool flag);
    
    bool isRemote();
    
    void foo(nvar& x);
    
    static int precedence(const nvar& f);
    
    NScope* currentScope();
    
    NScope* objectScope();
    
    nvar Import(const nvar& v);
    
    nvar Reset();
    
    nvar Throw(const nvar& v1, const nvar& v2);
    
    nvar Add(const nvar& v1, const nvar& v2);
    
    nvar Sub(const nvar& v1, const nvar& v2);
    
    nvar Mul(const nvar& v1, const nvar& v2);
    
    nvar Div(const nvar& v1, const nvar& v2);
    
    nvar Mod(const nvar& v1, const nvar& v2);
    
    nvar Neg(const nvar& v);
    
    nvar AddBy(const nvar& v1, const nvar& v2);
    
    nvar SubBy(const nvar& v1, const nvar& v2);
    
    nvar MulBy(const nvar& v1, const nvar& v2);
    
    nvar DivBy(const nvar& v1, const nvar& v2);
    
    nvar ModBy(const nvar& v1, const nvar& v2);
    
    nvar Inc(const nvar& v);
    
    nvar PostInc(const nvar& v);
    
    nvar Dec(const nvar& v);
    
    nvar PostDec(const nvar& v);
    
    nvar LT(const nvar& v1, const nvar& v2);
    
    nvar LE(const nvar& v1, const nvar& v2);
    
    nvar GT(const nvar& v1, const nvar& v2);
    
    nvar GE(const nvar& v1, const nvar& v2);
    
    nvar EQ(const nvar& v1, const nvar& v2);
    
    nvar NE(const nvar& v1, const nvar& v2);
    
    nvar And(const nvar& v1, const nvar& v2);
    
    nvar Or(const nvar& v1, const nvar& v2);
    
    nvar Not(const nvar& v);

    nvar Pow(const nvar& v1, const nvar& v2);
    
    nvar Sqrt(const nvar& v);
    
    nvar Exp(const nvar& v);
    
    nvar Abs(const nvar& v);
    
    nvar Floor(const nvar& v);
    
    nvar Ceil(const nvar& v);
    
    nvar Log10(const nvar& v);
    
    nvar Log(const nvar& v);
    
    nvar Cos(const nvar& v);
    
    nvar Acos(const nvar& v);
    
    nvar Cosh(const nvar& v);
    
    nvar Sin(const nvar& v);
    
    nvar Asin(const nvar& v);
    
    nvar Sinh(const nvar& v);
    
    nvar Tan(const nvar& v);
    
    nvar Atan(const nvar& v);
    
    nvar Tanh(const nvar& v);
    
    nvar Var(const nvar& v);
    
    nvar Var(const nvar& v1, const nvar& v2);
    
    nvar Var(const nvar& v1, const nvar& v2, const nvar& v3);
    
    nvar Set(const nvar& v1, const nvar& v2);
    
    nvar VarSet(const nvar& v1, const nvar& v2);
    
    nvar Get(const nvar& v);
    
    nvar Get(const nvar& v1, const nvar& v2);
    
    nvar Idx(const nvar& v1, const nvar& v2);
    
    nvar Dot(const nvar& v1, const nvar& v2);
    
    nvar Put(const nvar& v1, const nvar& v2);
    
    nvar DotPut(const nvar& v1, const nvar& v2);
    
    nvar Cs(const nvar& v);
    
    nvar In(const nvar& v1, const nvar& v2);
    
    nvar Call(const nvar& v);
    
    nvar Call(const nvar& v1, const nvar& v2);
    
    nvar Def(const nvar& v1, const nvar& v2, const nvar& v3);
    
    nvar Def(const nvar& v1, const nvar& v2);

    nvar DefSym(const nvar& v1, const nvar& v2, const nvar& v3);
    
    nvar DefSym(const nvar& v1, const nvar& v2);
    
    nvar New(const nvar& v);
    
    nvar New(const nvar& v1, const nvar& v2);
    
    nvar PushScope(const nvar& v);
    
    nvar PopScope();
    
    nvar PushBack(const nvar& v1, const nvar& v2);

    nvar TouchVector(const nvar& v1);
    
    nvar TouchList(const nvar& v1);
    
    nvar TouchQueue(const nvar& v1);
    
    nvar TouchSet(const nvar& v1);
    
    nvar TouchHashSet(const nvar& v1);
    
    nvar TouchMap(const nvar& v1);
    
    nvar TouchHashMap(const nvar& v1);
    
    nvar TouchMultimap(const nvar& v1);
    
    nvar Keys(const nvar& v1);
    
    nvar PushFront(const nvar& v1, const nvar& v2);
    
    nvar PopBack(const nvar& v1);
    
    nvar HasKey(const nvar& v1, const nvar& v2);
    
    nvar Insert(const nvar& v1, const nvar& v2, const nvar& v3);
    
    nvar Clear(const nvar& v1);
    
    nvar Empty(const nvar& v1);
    
    nvar Back(const nvar& v1);
    
    nvar Erase(const nvar& v1, const nvar& v2);
    
    nvar Merge(const nvar& v1, const nvar& v2);
    
    nvar OuterMerge(const nvar& v1, const nvar& v2);
    
    nvar Class(const nvar& v1);
    
    nvar If(const nvar& v1, const nvar& v2);
    
    nvar If(const nvar& v1, const nvar& v2, const nvar& v3);
    
    nvar For(const nvar& v1, const nvar& v2, const nvar& v3, const nvar& v4);
    
    nvar While(const nvar& v1, const nvar& v2);
    
    nvar Switch(const nvar& v1, const nvar& v2, const nvar& v3);
    
    nvar IsFalse(const nvar& v);
    
    nvar IsTrue(const nvar& v);
    
    nvar IsDefined(const nvar& v);
    
    nvar IsString(const nvar& v);
    
    nvar IsSymbol(const nvar& v);
    
    nvar IsFunction(const nvar& v);
    
    nvar IsFunction(const nvar& v1, const nvar& v2);
    
    nvar IsFunction(const nvar& v1, const nvar& v2, const nvar& v3);
    
    nvar IsSymbolic(const nvar& v);
    
    nvar IsNumeric(const nvar& v);
    
    nvar IsReference(const nvar& v);
    
    nvar IsPointer(const nvar& v);
    
    nvar IsInteger(const nvar& v);
    
    nvar IsRational(const nvar& v);
    
    nvar IsReal(const nvar& v);
    
    nvar GetStr(const nvar& v);
    
    nvar GetVec(const nvar& v);
    
    nvar GetList(const nvar& v);
    
    nvar GetAnySequence(const nvar& v);
    
    nvar GetMap(const nvar& v);
    
    nvar GetMultimap(const nvar& v);
    
    nvar GetAnyMap(const nvar& v);
    
    nvar Append(const nvar& v1, const nvar& v2);
    
    nvar Normalize(const nvar& v);
    
    nvar Head(const nvar& v);
    
    nvar SetHead(const nvar& v1, const nvar& v2);
    
    nvar ClearHead(const nvar& v);
        
    nvar NumKeys(const nvar& v);
    
    nvar Size(const nvar& v);
    
    nvar MapEmpty(const nvar& v);
    
    nvar AllEmpty(const nvar& v);
    
    nvar HasVector(const nvar& v);
    
    nvar HasList(const nvar& v);
    
    nvar HasMap(const nvar& v);
    
    nvar HasMultimap(const nvar& v);
    
    nvar PopFront(const nvar& v);
    
    nvar AllKeys(const nvar& v);
    
    nvar Open(const nvar& v1, const nvar& v2);
    
    nvar Save(const nvar& v1, const nvar& v2);
    
    nvar Unite(const nvar& v1, const nvar& v2);
    
    nvar Intersect(const nvar& v1, const nvar& v2);
    
    nvar Complement(const nvar& v1, const nvar& v2);
    
    nvar Inf();
    
    nvar NegInf();
    
    nvar Nan();
    
    nvar Min();
    
    nvar Max();
    
    nvar Epsilon();
    
    nvar Max(const nvar& v1, const nvar& v2);
    
    nvar Min(const nvar& v1, const nvar& v2);
    
    nvar Func(const nvar& v);
    
    nvar Sym(const nvar& v);
    
    nvar VarRef(const nvar& v);
    
    nvar VarPtr(const nvar& v);
    
    nvar LRec(const nvar& n);
    
    nvar NML(const nvar& v);
    
    void dumpScopes();
    
    friend class NObject_;
    
  private:
    class NObject_* x_;
  };
  
} // end namespace neu

#endif // NEU_N_OBJECT_H

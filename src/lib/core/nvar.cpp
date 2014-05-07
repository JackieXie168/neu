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

#include <neu/nvar.h>

#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cstring>

#include <neu/global.h>
#include <neu/compress.h>
#include <neu/NObject.h>
#include <neu/NSys.h>
#include <neu/NMLParser.h>

using namespace std;
using namespace neu;

namespace neu{
  
  const nvar undef;
  const nvar none(nvar::None, nvar::Head());
  const nvec _emptyVec;
  
  const nvar::CopyFlag* nvar::Copy = 0;
  const nvar::PointerFlag* nvar::Ptr = 0;
  const nvar::SymbolFlag* nvar::Sym = 0;
  const nvar::FunctionFlag* nvar::Func = 0;
  const nvar::HeadSequenceFlag* nvar::HeadSequenceType = 0;
  const nvar::HeadMapFlag* nvar::HeadMapType = 0;
  const nvar::SequenceMapFlag* nvar::SequenceMapType = 0;
  const nvar::HeadSequenceMapFlag* nvar::HeadSequenceMapType = 0;
  
} // end namespace Meta

namespace{
  
  static const nvar::Type Pack0 =             254;
  static const nvar::Type Pack1 =             253;
  static const nvar::Type Pack2 =             252;
  static const nvar::Type Pack3 =             251;
  static const nvar::Type Pack4 =             250;
  static const nvar::Type Pack5 =             249;
  static const nvar::Type Pack6 =             248;
  static const nvar::Type Pack7 =             247;
  static const nvar::Type Pack8 =             246;
  static const nvar::Type Pack9 =             245;
  static const nvar::Type Pack10 =            244;
  static const nvar::Type Pack11 =            243;
  static const nvar::Type Pack12 =            242;
  static const nvar::Type Pack13 =            241;
  static const nvar::Type Pack14 =            240;
  static const nvar::Type Pack15 =            239;
  static const nvar::Type Pack16 =            238;
  static const nvar::Type Pack17 =            237;
  static const nvar::Type Pack18 =            236;
  static const nvar::Type Pack19 =            235;
  static const nvar::Type Pack20 =            234;
  static const nvar::Type Pack21 =            233;
  static const nvar::Type Pack22 =            232;
  static const nvar::Type Pack23 =            231;
  static const nvar::Type Pack24 =            230;
  static const nvar::Type Pack25 =            229;
  static const nvar::Type Pack26 =            228;
  static const nvar::Type Pack27 =            227;
  static const nvar::Type Pack28 =            226;
  static const nvar::Type Pack29 =            225;
  static const nvar::Type Pack30 =            224;
  static const nvar::Type Pack31 =            223;
  static const nvar::Type Pack32 =            222;
  static const nvar::Type Pack33 =            221;
  static const nvar::Type Pack34 =            220;
  static const nvar::Type Pack35 =            219;
  static const nvar::Type Pack36 =            218;
  static const nvar::Type Pack37 =            217;
  static const nvar::Type Pack38 =            216;
  static const nvar::Type Pack39 =            215;
  static const nvar::Type Pack40 =            214;
  static const nvar::Type Pack41 =            213;
  static const nvar::Type Pack42 =            212;
  static const nvar::Type Pack43 =            211;
  static const nvar::Type Pack44 =            210;
  static const nvar::Type Pack45 =            209;
  static const nvar::Type Pack46 =            208;
  static const nvar::Type Pack47 =            207;
  static const nvar::Type Pack48 =            206;
  static const nvar::Type Pack49 =            205;
  static const nvar::Type Pack50 =            204;
  static const nvar::Type PackInt8 =          203;
  static const nvar::Type PackInt16 =         202;
  static const nvar::Type PackInt32 =         201;
  static const nvar::Type PackFloat32 =       200;
  static const nvar::Type PackShortString =   199;
  static const nvar::Type PackLongString =    198;
  static const nvar::Type PackLongSymbol =    197;
  static const nvar::Type PackSmallVector =   196;
  static const nvar::Type PackShortVector =   195;
  static const nvar::Type PackLongVector =    194;
  static const nvar::Type PackShortList =     193;
  static const nvar::Type PackLongList =      192;
  static const nvar::Type PackLongFunction =  191;
  static const nvar::Type PackShortMap =      190;
  static const nvar::Type PackLongMap =       189;
  static const nvar::Type PackShortMultimap = 188;
  static const nvar::Type PackLongMultimap =  187;

  void uniteList(nlist& l, nlist& l2, bool outer){
    l.unique();
    l.sort();
    l2.unique();
    l2.sort();
    
    auto itr = l.begin();
    auto itr2 = l2.begin(); 
    
    if(itr2 == l2.end()){
      return;
    }
    
    if(itr == l.end()){
      while(itr2 != l2.end()){
        l.push_back(*itr2);
        ++itr2;
      }
      return;
    }
    
    for(;;){
      if(itr2->head() == itr->head()){
        if(itr2->isDefined()){
          itr->unite(*itr2, outer);
          ++itr2;
          if(itr2 == l2.end()){
            return;
          }
        }
        else if(*itr2 != *itr){
          l.insert(itr, *itr2);
          ++itr2;
          if(itr2 == l2.end()){
            return;
          }
        }
        else{
          ++itr2;
          if(itr2 == l2.end()){
            return;
          }
        }
        
        ++itr;
        if(itr == l.end()){
          while(itr2 != l2.end()){
            l.push_back(*itr2);
            ++itr2;
          }
          return;
        }
      }
      else if(*itr2 > *itr){
        ++itr;
        if(itr == l.end()){
          while(itr2 != l2.end()){
            l.push_back(*itr2);
            ++itr2;
          }
          return;
        }
      }
      else{
        l.insert(itr, *itr2);
        ++itr2;
        if(itr2 == l2.end()){
          return;
        }
      }
    }
  }
  
  void uniteMap(nmap& m, nmap& m2, bool outer){
    auto itr = m.begin();
    auto itr2 = m2.begin(); 
    
    if(itr2 == m2.end()){
      return;
    }
    
    if(itr == m.end()){
      while(itr2 != m2.end()){
        m.insert({itr2->first, itr2->second});
        ++itr2;
      }
      return;
    }
    
    for(;;){
      if(itr2->first == itr->first){
        itr->second.unite(itr2->second, outer);
        ++itr2;
        if(itr2 == m2.end()){
          return;
        }
        ++itr;
        if(itr == m.end()){
          while(itr2 != m2.end()){
            m.insert({itr2->first, itr2->second});
            ++itr2;
          }
          return;
        }
      }
      else if(itr2->first > itr->first){
        ++itr;
        if(itr == m.end()){
          while(itr2 != m2.end()){
            m.insert({itr2->first, itr2->second});
            ++itr2;
          }
          return;
        }
      }
      else{
        m.insert({itr2->first, itr2->second});
        ++itr2;
        if(itr2 == m2.end()){
          return;
        }
      }
    }
  }
  
  void intersectList(nlist& l, nlist& l2, bool outer){
    l.unique();
    l.sort();
    l2.unique();
    l2.sort();
    auto itr = l.begin();
    auto itr2 = l2.begin(); 
    
    if(itr == l.end()){
      return;
    }
    
    if(itr2 == l2.end()){
      while(itr != l.end()){
        itr = l.erase(itr);
      }
      return;
    }
    
    for(;;){
      if(itr2->head() == itr->head()){
        if(itr2->isDefined()){
          itr->intersect(*itr2, outer);
          ++itr;
          if(itr == l.end()){
            return;
          }
        }
        else if(*itr2 == *itr){
          ++itr;
          if(itr == l.end()){
            return;
          }
        }
        ++itr2;
        if(itr2 == l2.end()){
          while(itr != l.end()){
            itr = l.erase(itr);
          }
          return;
        }
      }
      else if(*itr2 > *itr){
        itr = l.erase(itr);
        if(itr == l.end()){
          return;
        }
      }
      else{
        ++itr2;
        if(itr2 == l2.end()){
          while(itr != l.end()){
            itr = l.erase(itr);
          }
          return;
        }
      }
    }
  }
  
  void intersectMap(nmap& m, nmap& m2, bool outer){
    auto itr = m.begin();
    auto itr2 = m2.begin(); 
    
    if(itr == m.end()){
      return;
    }
    
    if(itr2 == m2.end()){
      while(itr != m.end()){
        m.erase(itr++);
      }
      return;
    }
    
    for(;;){
      if(itr2->first == itr->first){
        itr->second.intersect(itr2->second, outer);
        ++itr;
        if(itr == m.end()){
          return;
        }
        ++itr2;
        if(itr2 == m2.end()){
          while(itr != m.end()){
            m.erase(itr++);
          }
          return;
        }
      }
      else if(itr2->first > itr->first){
        m.erase(itr++);
        if(itr == m.end()){
          return;
        }
      }
      else{
        ++itr2;
        if(itr2 == m2.end()){
          while(itr != m.end()){
            m.erase(itr++);
          }
          return;
        }
      }
    }
  }
  
  void complementList(nlist& l, nlist& l2){
    l.unique();
    l.sort();
    l2.unique();
    l2.sort();
    
    auto itr = l.begin();
    auto itr2 = l2.begin(); 
    
    if(itr == l.end()){
      return;
    }
    
    if(itr2 == l2.end()){
      return;
    }
    
    for(;;){
      if(itr2->head() == itr->head()){
        if(itr2->isDefined()){
          itr->complement(*itr2);
          if(itr->allEmpty()){
            itr = l.erase(itr);
          }
          if(itr == l.end()){
            return;
          }
        }
        else if(*itr2 == *itr){
          itr = l.erase(itr);
          if(itr == l.end()){
            return;
          }
        }
        ++itr2;
        if(itr2 == l2.end()){
          return;
        }
      }
      else if(*itr2 > *itr){
        ++itr;
        if(itr == l.end()){
          return;
        }
      }
      else{
        ++itr2;
        if(itr2 == l2.end()){
          return;
        }
      }
    }
  }
  
  void complementMap(nmap& m, nmap& m2){
    auto itr = m.begin();
    auto itr2 = m2.begin(); 
    
    if(itr == m.end()){
      return;
    }
    
    if(itr2 == m2.end()){
      return;
    }
    
    for(;;){
      if(itr2->first == itr->first){
        itr->second.complement(itr2->second);
        if(itr->second.allEmpty()){
          m.erase(itr++);
          if(itr == m.end()){
            return;
          }
        }
        ++itr2;
        if(itr2 == m2.end()){
          return;
        }
      }
      else if(itr2->first > itr->first){
        ++itr;
        if(itr == m.end()){
          return;
        }
      }
      else{
        ++itr2;
        if(itr2 == m2.end()){
          return;
        }
      }
    }
  }
  
} // end namespace  

void nvar::streamOutput_(ostream& ostr, bool concise) const{
  switch(t_){
    case None:
      ostr << "none";
      break;
    case Undefined:
      ostr << "undef";
      break;
    case False:
      ostr << "false";
      break;
    case True:
      ostr << "true";
      break;
    case Integer:
      ostr << h_.i;
      break;
    case Rational:
      ostr << *h_.r;
      break;
    case Float:
      ostr << h_.d;
      break;
    case Real:
      ostr << *h_.x;
      break;
    case Symbol:
      ostr << *h_.s;
      break;
    case StringPointer:
    case String:
      ostr << "\"" << *h_.s << "\"";
      break;
    case Binary:
      ostr << "<<BINARY:" << h_.s->length() << ">>";
      break;
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      ostr << h_.o;
      break;
    case RawPointer:
      ostr << h_.p;
      break;
    case Vector:{
      ostr << "[";
      bool first = true;
      streamOutputVector_(ostr, *h_.v, first, concise);
      ostr << "]";
      break;
    }
    case List:{
      ostr << "(";
      bool first = true;
      streamOutputList_(ostr, *h_.l, first, concise);
      ostr << ")";
      break;
    }
    case Function:{
      ostr << h_.f->f << "(";
      
      bool first = true;
      const nvec& args = h_.f->v;
      for(const nvar& a : args){
        if(first){
          first = false;
        }
        else{
          ostr << ",";
        }
        a.streamOutput_(ostr, concise);
      }
      
      if(h_.f->m){
        streamOutputFuncMap_(ostr, *h_.f->m, first, concise);
      }
      
      ostr << ")";
      break;
    }
    case HeadSequence:{
      bool first = false;
      if(h_.hs->s->t_ == Vector){
        ostr << "[:";
        h_.hs->h->streamOutput_(ostr, concise);
        streamOutputVector_(ostr, *h_.hs->s->h_.v, first, concise);
        ostr << "]";
      }
      else{
        ostr << "(:";
        h_.hs->h->streamOutput_(ostr, concise);
        streamOutputList_(ostr, *h_.hs->s->h_.l, first, concise);
        ostr << ")";
      }
      break;
    }
    case Map:{
      stringstream sstr;
      bool first = true;
      bool found = streamOutputMap_(sstr, *h_.m, first, concise);
      if(found){
        ostr << "[";
        ostr << sstr.str();
        ostr << "]";
      }
      else{
        ostr << "undef";
      }
      break;
    }
    case Multimap:{
      stringstream sstr;
      bool first = true;
      bool found = streamOutputMultimap_(sstr, *h_.mm, first, concise);
      if(found){
        ostr << "[|";
        ostr << sstr.str();
        ostr << "]";
      }
      else{
        ostr << "undef";
      }
      break;
    }
    case HeadMap:{
      bool first = false;
      bool mm;
      
      bool found;
      
      if(h_.hm->m->t_ == Map){
        mm = false;
        stringstream sstr;
        found = streamOutputMap_(sstr, *h_.hm->m->h_.m, first, concise);
        if(found){
          ostr << "[:";
          h_.hm->h->streamOutput_(ostr, concise);
          ostr << sstr.str();
        }
        else{
          h_.hm->h->streamOutput_(ostr, concise);
        }
      }
      else{
        mm = true;
        
        stringstream sstr;
        found = streamOutputMultimap_(sstr, *h_.hm->m->h_.mm, first, concise);
        if(found){
          ostr << "[|:";
          h_.hm->h->streamOutput_(ostr, concise);
          ostr << sstr.str();
        }
        else{
          h_.hm->h->streamOutput_(ostr, concise);
        }
      }
      
      if(found){
        ostr << "]";
      }
      break;
    }
    case SequenceMap:{
      bool first = true;
      bool vec = h_.sm->s->t_ == Vector;
      bool mm =  h_.sm->m->t_ == Multimap;
      
      if(vec){
        if(mm){
          ostr << "[|";
        }
        else{
          ostr << "[";
        }
        
        streamOutputVector_(ostr, *h_.sm->s->h_.v, first, concise);
      }
      else{
        if(mm){
          ostr << "(|";
        }
        else{
          ostr << "(";
        }
        
        streamOutputList_(ostr, *h_.sm->s->h_.l, first, concise);
      }
      
      if(mm){
        streamOutputMultimap_(ostr, *h_.sm->m->h_.mm, first, concise);
      }
      else{
        streamOutputMap_(ostr, *h_.sm->m->h_.m, first, concise);
      }
      
      if(vec){
        ostr << "]";
      }
      else{
        ostr << ")";
      }
      break;
    }
    case HeadSequenceMap:{
      bool first = false;
      bool vec = h_.sm->s->t_ == Vector;
      bool mm =  h_.sm->m->t_ == Multimap;
      
      if(vec){
        if(mm){
          ostr << "[|:";
        }
        else{
          ostr << "[:";
        }
        h_.hsm->h->streamOutput_(ostr, concise);
        streamOutputVector_(ostr, *h_.sm->s->h_.v, first, concise);
      }
      else{
        if(mm){
          ostr << "(|:";
        }
        else{
          ostr << "(:";
        }
        
        h_.hsm->h->streamOutput_(ostr, concise);
        streamOutputList_(ostr, *h_.sm->s->h_.l, first, concise);
      }
      
      if(mm){
        streamOutputMultimap_(ostr, *h_.sm->m->h_.mm, first, concise);
      }
      else{
        streamOutputMap_(ostr, *h_.sm->m->h_.m, first, concise);
      }
      
      if(vec){
        ostr << "]";
      }
      else{
        ostr << ")";
      }
      break;
    }
    case Reference:
      h_.ref->v->streamOutput_(ostr, concise);
      break;
    case Pointer:
      h_.vp->streamOutput_(ostr, concise);
      break;
  }
}

nvar::nvar(const nvar& x, const CopyFlag*)
: t_(x.t_){
  switch(t_){
    case Integer:
      h_.i = x.h_.i;
      break;
    case Float:
      h_.d = x.h_.d;
      break;
    case Rational:
      h_.r = new nrat(*x.h_.r);
      break;
    case Real:
      h_.x = new nreal(*x.h_.x);
      break;
    case String:
    case Binary:
    case Symbol:
      h_.s = new nstr(*x.h_.s);
      break;
    case StringPointer:
      h_.s = x.h_.s;
      break;
    case RawPointer:
      h_.p = x.h_.p;
      break;
    case LocalObject:
      h_.o = x.h_.o->clone();
      break;
    case ObjectPointer:
      h_.o = x.h_.o;
      break;
    case SharedObject:
      h_.o = x.h_.o;
      h_.o->ref();
      break;
    case Vector:{
      h_.v = new nvec;
      
      nvec& v = *h_.v;
      const nvec& xv = *x.h_.v;
      size_t size = xv.size();
      v.reserve(size);
      
      for(size_t i = 0; i < size; ++i){
        v.emplace_back(nvar(xv[i], Copy));
      }
      
      break;
    }
    case List:{
      h_.l = new nlist;
      nlist& l = *h_.l;
      const nlist& xl = *x.h_.l;
      
      for(auto& itr : xl){
        l.emplace_back(nvar(itr, Copy));
      }
      
      break;
    }
    case Function:{
      h_.f = new CFunction(x.h_.f->f);
      
      nvec& v = h_.f->v;
      const nvec& xv = x.h_.f->v;
      size_t size = xv.size();
      v.reserve(size);
      
      for(size_t i = 0; i < size; ++i){
        v.emplace_back(nvar(xv[i], Copy));
      }
      
      if(x.h_.f->m){
        nmap* m = new nmap;
        h_.f->m = m;
        
        const nmap& xm = *x.h_.f->m;
        
        for(auto& itr : xm){
          m->emplace(nvar(itr.first, Copy), nvar(itr.second, Copy));
        }
      }
      
      break;
    }
    case HeadSequence:
      h_.hs =
      new CHeadSequence(new nvar(*x.h_.hs->h, Copy),
                        new nvar(*x.h_.hs->s, Copy));
      break;
    case Map:{
      h_.m = new nmap;
      nmap& m = *h_.m;
      const nmap& xm = *x.h_.m;
      
      for(auto& itr : xm){
        m.emplace(nvar(itr.first, Copy), nvar(itr.second, Copy));
      }
      
      break;
    }
    case Multimap:{
      h_.mm = new nmmap;
      nmmap& mm = *h_.mm;
      const nmmap& xmm = *x.h_.mm;
      
      for(auto& itr : xmm){
        mm.emplace(nvar(itr.first, Copy), nvar(itr.second, Copy));
      }
      
      break;
    }
    case HeadMap:
      h_.hm =
      new CHeadMap(new nvar(*x.h_.hm->h, Copy),
                   new nvar(*x.h_.hm->m, Copy));
      break;
    case SequenceMap:
      h_.sm =
      new CSequenceMap(new nvar(*x.h_.sm->s, Copy),
                       new nvar(*x.h_.sm->m, Copy));
      break;
    case HeadSequenceMap:
      h_.hsm =
      new CHeadSequenceMap(new nvar(*x.h_.hsm->h, Copy),
                           new nvar(*x.h_.hsm->s, Copy),
                           new nvar(*x.h_.hsm->m, Copy));
      break;
    case Reference:
      h_.ref = new CReference(new nvar(*x.h_.ref->v, Copy));
      break;
    case Pointer:
      h_.vp = h_.vp;
      break;
  }
}

nvar::~nvar(){
  switch(t_){
    case Rational:
      delete h_.r;
      break;
    case Real:
      delete h_.x;
      break;
    case String:
    case Binary:
    case Symbol:
      delete h_.s;
      break;
    case LocalObject:
      delete h_.o;
      break;
    case SharedObject:
      if(h_.o->deref()){
        delete h_.o;
      }
      break;
    case Vector:
      delete h_.v;
      break;
    case List:
      delete h_.l;
      break;
    case Function:
      delete h_.f;
      break;
    case HeadSequence:
      h_.hs->dealloc();
      delete h_.hs;
      break;
    case Map:
      delete h_.m;
      break;
    case Multimap:
      delete h_.mm;
      break;
    case HeadMap:
      h_.hm->dealloc();
      delete h_.hm;
      break;
    case SequenceMap:
      h_.sm->dealloc();
      delete h_.sm;
      break;
    case HeadSequenceMap:
      h_.hsm->dealloc();
      delete h_.hsm;
      break;
    case Reference:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      break;
  }
}

bool nvar::toBool() const{
  switch(t_){
    case False:
      return false;
    case True:
      return true;
    case Integer:
      return h_.i != 0;
    case Float:
      return h_.d != 0.0;
    case Rational:
      return *h_.r != 0;
    case Real:
      return *h_.x != 0;
    case Symbol:
    case String:
    case Binary:
    case StringPointer:
      return !h_.s->empty();
    case RawPointer:
      return h_.p;
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.o;
    case HeadSequence:
      return h_.hs->h->toBool();
    case HeadMap:
      return h_.hm->h->toBool();
    case HeadSequenceMap:
      return h_.hsm->h->toBool();
    case Reference:
      return h_.ref->v->toBool();
    case Pointer:
      return h_.vp;
    default:
      return false;
  }
}

int64_t nvar::toLong() const{
  switch(t_){
    case False:
      return 0;
    case True:
      return 1;
    case Integer:
      return h_.i;
    case Float:
      return static_cast<int64_t>(h_.d);
    case Rational:
      return h_.r->toLong();
    case Real:
      return h_.x->toLong();
    case Symbol:
    case String:
    case StringPointer:
    case Binary:
      return atoll(h_.s->c_str());
    case HeadSequence:
      return h_.hs->h->toLong();
    case HeadMap:
      return h_.hm->h->toLong();
    case HeadSequenceMap:
      return h_.hsm->h->toLong();
    case Reference:
      return h_.ref->v->toLong();
    case Pointer:
      return h_.vp->toLong();
    default:
      NERROR("undefined operand");
  }
}

double nvar::toDouble() const{
  switch(t_){
    case False:
      return 0.0;
    case True:
      return 1.0;
    case Integer:
      return h_.i;
    case Float:
      return h_.d;
    case Rational:
      return h_.r->toDouble();
    case Real:
      return h_.x->toDouble();
    case Symbol:
    case String:
    case StringPointer:
    case Binary:
      return atof(h_.s->c_str());
    case HeadSequence:
      return h_.hs->h->toDouble();
    case HeadMap:
      return h_.hm->h->toDouble();
    case HeadSequenceMap:
      return h_.hsm->h->toDouble();
    case Reference:
      return h_.ref->v->toDouble();
    case Pointer:
      return h_.vp->toDouble();
    default:
      NERROR("undefined operand");
  }
}

void nvar::pushBack(const nvar& x){
  switch(t_){
    case None:
    case Undefined:
      t_ = Vector;
      h_.v = new nvec(1, x);
      break;
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case String:
    case StringPointer:
    case Binary:
    case RawPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:{
      Head h;
      h.v = new nvec(1, x);
      h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
      t_ = HeadSequence;
      break;
    }
    case Vector:
      h_.v->push_back(x);
      break;
    case List:
      h_.l->push_back(x);
      break;
    case Function:
      h_.f->v.push_back(x);
      break;
    case HeadSequence:
      h_.hs->s->pushBack(x);
      break;
    case Map:{
      Head hv;
      hv.v = new nvec(1, x);
      
      Head hm;
      hm.m = h_.m;
      
      h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Map, hm));
      t_ = SequenceMap;
      break;
    }
    case Multimap:{
      Head hv;
      hv.v = new nvec(1, x);
      
      Head hm;
      hm.mm = h_.mm;
      
      h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Multimap, hm));
      t_ = SequenceMap;
      break;
    }
    case HeadMap:{
      Head hv;
      hv.v = new nvec(1, x);
      
      CHeadMap* hm = h_.hm;
      h_.hsm = new CHeadSequenceMap(h_.hm->h, new nvar(Vector, hv), h_.hm->m);
      delete hm;
      t_ = HeadSequenceMap;
      break;
    }
    case SequenceMap:
      h_.sm->s->pushBack(x);
      break;
    case HeadSequenceMap:
      h_.hsm->s->pushBack(x);
      break;
    case Reference:
      h_.ref->v->pushBack(x);
      break;
    case Pointer:
      h_.vp->pushBack(x);
      break;
  }
}

void nvar::pushFront(const nvar& x){
  switch(t_){
    case None:
    case Undefined:
      t_ = Vector;
      h_.v = new nvec(1, x);
      break;
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case String:
    case StringPointer:
    case Binary:
    case RawPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:{
      Head h;
      h.v = new nvec(1, x);
      h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
      t_ = HeadSequence;
      break;
    }
    case Vector:
      h_.v->pushFront(x);
      break;
    case List:
      h_.l->push_front(x);
      break;
    case Function:
      h_.f->v.pushFront(x);
      break;
    case HeadSequence:
      h_.hs->s->pushFront(x);
      break;
    case Map:{
      Head hv;
      hv.v = new nvec(1, x);
      
      Head hm;
      hm.m = h_.m;
      
      h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Map, hm));
      t_ = SequenceMap;
      break;
    }
    case Multimap:{
      Head hv;
      hv.v = new nvec(1, x);
      
      Head hm;
      hm.mm = h_.mm;
      
      h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Multimap, hm));
      t_ = SequenceMap;
      break;
    }
    case HeadMap:{
      Head hv;
      hv.v = new nvec(1, x);
      
      CHeadMap* hm = h_.hm;
      h_.hsm = new CHeadSequenceMap(h_.hm->h, new nvar(Vector, hv), h_.hm->m);
      delete hm;
      t_ = HeadSequenceMap;
      break;
    }
    case SequenceMap:
      h_.sm->s->pushFront(x);
      break;
    case HeadSequenceMap:
      h_.hsm->s->pushFront(x);
      break;
    case Reference:
      h_.ref->v->pushFront(x);
      break;
    case Pointer:
      h_.vp->pushFront(x);
      break;
  }
}

nvar nvar::popBack(){
  switch(t_){
    case Vector:
      return h_.v->popBack();
    case List:
      return h_.l->popBack();
    case Function:
      return h_.f->v.popBack();
    case HeadSequence:
      return h_.hs->s->popBack();
    case SequenceMap:
      return h_.sm->s->popBack();
    case HeadSequenceMap:
      return h_.hsm->s->popBack();
    case Reference:
      return h_.ref->v->popBack();
    case Pointer:
      return h_.vp->popBack();
    default:
      NERROR("var does not hold a sequence");
  }
}

nvar nvar::popFront(){
  switch(t_){
    case Vector:
      return h_.v->popFront();
    case List:
      return h_.l->popFront();
    case Function:
      return h_.f->v.popFront();
    case HeadSequence:
      return h_.hs->s->popFront();
    case SequenceMap:
      return h_.sm->s->popFront();
    case HeadSequenceMap:
      return h_.hsm->s->popFront();
    case Reference:
      return h_.ref->v->popFront();
    case Pointer:
      return h_.vp->popFront();
    default:
      NERROR("var does not hold a sequence");
  }
}

bool nvar::hasKey(const nvar& key) const{
  switch(t_){
    case Function:
      return h_.f->m && h_.f->m->hasKey(key);
    case Map:
      return h_.m->hasKey(key);
    case Multimap:
      return h_.mm->count(key) > 0;
    case HeadMap:
      return h_.hm->m->hasKey(key);
    case SequenceMap:
      return h_.sm->m->hasKey(key);
    case HeadSequenceMap:
      return h_.hsm->m->hasKey(key);
    case Reference:
      return h_.ref->v->hasKey(key);
    case Pointer:
      return h_.vp->hasKey(key);
    default:
      return false;
  }
}

size_t nvar::numKeys(const nvar& key){
  switch(t_){
    case Function:
      return h_.f->m && h_.f->m->hasKey(key) ? 1 : 0;
    case Map:
      return h_.m->hasKey(key) ? 1 : 0;
    case Multimap:
      return h_.mm->count(key);
    case HeadMap:
      return h_.hm->m->numKeys(key);
    case SequenceMap:
      return h_.sm->m->numKeys(key);
    case HeadSequenceMap:
      return h_.hsm->m->numKeys(key);
    case Reference:
      return h_.ref->v->numKeys(key);
    case Pointer:
      return h_.vp->numKeys(key);
    default:
      return 0;
  }
}

void nvar::insert(size_t pos, const nvar& x){
  switch(t_){
    case Vector:{
      if(pos > h_.v->size()){
        NERROR("invalid position");
      }
      auto itr = h_.v->begin();
      advance(itr, pos);
      h_.v->insert(itr, x);
      break;
    }
    case List:{
      if(pos > h_.l->size()){
        NERROR("invalid position");
      }
      auto itr = h_.l->begin();
      advance(itr, pos);
      h_.l->insert(itr, x);
      break;
    }
    case Function:{
      auto itr = h_.f->v.begin();
      advance(itr, pos);
      h_.f->v.insert(itr, x);
      break;
    }
    case HeadSequence:
      h_.hs->s->insert(pos, x);
      break;
    case SequenceMap:
      h_.sm->s->insert(pos, x);
      break;
    case HeadSequenceMap:
      h_.hsm->s->insert(pos, x);
      break;
    case Reference:
      h_.ref->v->insert(pos, x);
      break;
    case Pointer:
      h_.vp->insert(pos, x);
      break;
    default:
      NERROR("var does not hold a sequence");
  }
}

void nvar::clear(){
  switch(t_){
    case Vector:
      h_.v->clear();
      break;
    case List:
      h_.l->clear();
      break;
    case Function:
      h_.f->v.clear();
      if(h_.f->m){
        delete h_.f->m;
        h_.f->m = 0;
      }
      break;
    case HeadSequence:
      h_.hs->s->clear();
      break;
    case Map:
      h_.m->clear();
      break;
    case Multimap:
      h_.mm->clear();
      break;
    case HeadMap:
      h_.hm->m->clear();
      break;
    case SequenceMap:
      h_.sm->s->clear();
      h_.sm->m->clear();
      break;
    case HeadSequenceMap:
      h_.hsm->s->clear();
      h_.hsm->m->clear();
      break;
    case Reference:
      h_.ref->v->clear();
      break;
    case Pointer:
      h_.vp->clear();
      break;
    default:
      break;
  }
}

size_t nvar::size() const{
  switch(t_){
    case Vector:
      return h_.v->size();
    case List:
      return h_.l->size();
    case Function:
      return h_.f->v.size();
    case HeadSequence:
      return h_.hs->s->size();
    case SequenceMap:
      return h_.sm->s->size();
    case HeadSequenceMap:
      return h_.hsm->s->size();
    case Reference:
      return h_.ref->v->size();
    case Pointer:
      return h_.vp->size();
    default:
      return 0;
  }
}

size_t nvar::numKeys() const{
  switch(t_){
    case Map:
      return h_.m->size();
    case Multimap:
      return h_.mm->size();
    case HeadMap:
      return h_.hm->m->size();
    case SequenceMap:
      return h_.sm->m->size();
    case HeadSequenceMap:
      return h_.hsm->m->size();
    case Reference:
      return h_.ref->v->size();
    case Pointer:
      return h_.vp->size();
    default:
      return 0;
  }
}

nvar& nvar::operator=(int64_t x){
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Float:
    case StringPointer:
    case RawPointer:
    case ObjectPointer:
    case Pointer:
      t_ = Integer;
      h_.i = x;
      return *this;
    case Integer:
      h_.i = x;
      return *this;
    case Rational:
      delete h_.r;
      t_ = Integer;
      h_.i = x;
      return *this;
    case Real:
      delete h_.x;
      t_ = Integer;
      h_.i = x;
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.s;
      t_ = Integer;
      h_.i = x;
      return *this;
    case Vector:
      delete h_.v;
      t_ = Integer;
      h_.i = x;
      return *this;
    case List:
      delete h_.l;
      t_ = Integer;
      h_.i = x;
      return *this;
    case Function:
      delete h_.f;
      t_ = Integer;
      h_.i = x;
      return *this;
    case Map:
      delete h_.m;
      t_ = Integer;
      h_.i = x;
      return *this;
    case Multimap:
      delete h_.mm;
      t_ = Integer;
      h_.i = x;
      return *this;
    case Reference:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = Integer;
      h_.i = x;
      return *this;
    case HeadSequence:
      h_.hs->dealloc();
      delete h_.hs;
      break;
    case HeadMap:
      h_.hm->dealloc();
      delete h_.hm;
      break;
    case LocalObject:
      delete h_.o;
      break;
    case SharedObject:
      if(h_.o->deref()){
        delete h_.o;
      }
      break;
    case SequenceMap:
      h_.sm->dealloc();
      delete h_.sm;
      break;
    case HeadSequenceMap:
      h_.hsm->dealloc();
      delete h_.hsm;
      break;
  }
  
  t_ = Integer;
  h_.i = x;
  return *this;
}

nvar& nvar::operator=(const char* x){
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Integer:
    case Float:
    case StringPointer:
    case RawPointer:
    case ObjectPointer:
    case Pointer:
      t_ = String;
      h_.s = new nstr(x);
      return *this;
    case Rational:
      delete h_.r;
      t_ = String;
      h_.s = new nstr(x);
      return *this;
    case Real:
      delete h_.x;
      t_ = String;
      h_.s = new nstr(x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = String;
      *h_.s = x;
      return *this;
    case Vector:
      delete h_.v;
      t_ = String;
      h_.s = new nstr(x);
      return *this;
    case List:
      delete h_.l;
      t_ = String;
      h_.s = new nstr(x);
      return *this;
    case Function:
      delete h_.f;
      t_ = String;
      h_.s = new nstr(x);
      return *this;
    case Map:
      delete h_.m;
      t_ = String;
      h_.s = new nstr(x);
      return *this;
    case Multimap:
      delete h_.mm;
      t_ = String;
      h_.s = new nstr(x);
      return *this;
    case Reference:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = String;
      h_.s = new nstr(x);
      return *this;
    case HeadSequence:
      h_.hs->dealloc();
      delete h_.hs;
      break;
    case HeadMap:
      h_.hm->dealloc();
      delete h_.hm;
      break;
    case LocalObject:
      delete h_.o;
      break;
    case SharedObject:
      if(h_.o->deref()){
        delete h_.o;
      }
      break;
    case SequenceMap:
      h_.sm->dealloc();
      delete h_.sm;
      break;
    case HeadSequenceMap:
      h_.hsm->dealloc();
      delete h_.hsm;
      break;
  }
  
  t_ = String;
  h_.s = new nstr(x);
  return *this;
}

nvar& nvar::operator=(void* p){
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Integer:
    case Float:
    case StringPointer:
    case RawPointer:
    case ObjectPointer:
    case Pointer:
      t_ = Pointer;
      h_.p = p;
      return *this;
    case Rational:
      delete h_.r;
      t_ = Pointer;
      h_.p = p;
      return *this;
    case Real:
      delete h_.x;
      t_ = Pointer;
      h_.p = p;
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.s;
      t_ = Pointer;
      h_.p = p;
      return *this;
    case Vector:
      delete h_.v;
      t_ = Pointer;
      h_.p = p;
      return *this;
    case List:
      delete h_.l;
      t_ = Pointer;
      h_.p = p;
      return *this;
    case Function:
      delete h_.f;
      t_ = Pointer;
      h_.p = p;
      return *this;
    case Map:
      delete h_.m;
      t_ = Pointer;
      h_.p = p;
      return *this;
    case Multimap:
      delete h_.mm;
      t_ = Pointer;
      h_.p = p;
      return *this;
    case Reference:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = Pointer;
      h_.p = p;
      return *this;
    case HeadSequence:
      h_.hs->dealloc();
      delete h_.hs;
      break;
    case HeadMap:
      h_.hm->dealloc();
      delete h_.hm;
      break;
    case LocalObject:
      delete h_.o;
      break;
    case SharedObject:
      if(h_.o->deref()){
        delete h_.o;
      }
      break;
    case SequenceMap:
      h_.sm->dealloc();
      delete h_.sm;
      break;
    case HeadSequenceMap:
      h_.hsm->dealloc();
      delete h_.hsm;
      break;
  }

  t_ = Pointer;
  h_.p = p;
  return *this;
}

nvar& nvar::operator=(double x){
  switch(t_){
    case Float:
      h_.d = x;
      return *this;
    case Rational:
      delete h_.r;
      t_ = Float;
      h_.d = x;
      return *this;
    case Real:
      delete h_.x;
      t_ = Float;
      h_.d = x;
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.s;
      t_ = Float;
      h_.d = x;
      return *this;
    case Vector:
      delete h_.v;
      t_ = Float;
      h_.d = x;
      return *this;
    case List:
      delete h_.l;
      t_ = Float;
      h_.d = x;
      return *this;
    case Function:
      delete h_.f;
      t_ = Float;
      h_.d = x;
      return *this;
    case Map:
      delete h_.m;
      t_ = Float;
      h_.d = x;
      return *this;
    case Multimap:
      delete h_.mm;
      t_ = Float;
      h_.d = x;
      return *this;
    case Reference:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = Float;
      h_.d = x;
      return *this;
    case HeadSequence:
      h_.hs->dealloc();
      delete h_.hs;
      break;
    case HeadMap:
      h_.hm->dealloc();
      delete h_.hm;
      break;
    case LocalObject:
      delete h_.o;
      break;
    case SharedObject:
      if(h_.o->deref()){
        delete h_.o;
      }
      break;
    case SequenceMap:
      h_.sm->dealloc();
      delete h_.sm;
      break;
    case HeadSequenceMap:
      h_.hsm->dealloc();
      delete h_.hsm;
      break;
    default:
      t_ = Float;
      h_.d = x;
      return *this;
  }
  
  t_ = Float;
  h_.d = x;
  return *this;
}

nvar& nvar::operator=(bool x){
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Integer:
    case Float:
    case StringPointer:
    case RawPointer:
    case ObjectPointer:
    case Pointer:
      t_ = x ? True : False;
      return *this;
    case Rational:
      delete h_.r;
      t_ = x ? True : False;
      return *this;
    case Real:
      delete h_.x;
      t_ = x ? True : False;
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.s;
      t_ = x ? True : False;
      return *this;
    case Vector:
      delete h_.v;
      t_ = x ? True : False;
      return *this;
    case List:
      delete h_.l;
      t_ = x ? True : False;
      return *this;
    case Function:
      delete h_.f;
      t_ = x ? True : False;
      return *this;
    case Map:
      delete h_.m;
      t_ = x ? True : False;
      return *this;
    case Multimap:
      delete h_.mm;
      t_ = x ? True : False;
      return *this;
    case Reference:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = x ? True : False;
      return *this;
    case HeadSequence:
      h_.hs->dealloc();
      delete h_.hs;
      break;
    case HeadMap:
      h_.hm->dealloc();
      delete h_.hm;
      break;
    case LocalObject:
      delete h_.o;
      break;
    case SharedObject:
      if(h_.o->deref()){
        delete h_.o;
      }
      break;
    case SequenceMap:
      h_.sm->dealloc();
      delete h_.sm;
      break;
    case HeadSequenceMap:
      h_.hsm->dealloc();
      delete h_.hsm;
      break;
  }
  
  t_ = x ? True : False;
  return *this;
}

nvar& nvar::operator=(const nvar& x){
  switch(t_){
    case None:
      switch(x.t_){
        case None:
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Undefined:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case False:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case True:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Integer:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Integer:
          h_.i = x.h_.i;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Rational:
      switch(x.t_){
        case None:
          delete h_.r;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.r;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.r;
          t_ = False;
          return *this;
        case True:
          delete h_.r;
          t_ = True;
          return *this;
        case Rational:
          *h_.r = *x.h_.r;
          return *this;
        case Real:
          delete h_.r;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.r;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.r;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.r;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.r;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          delete h_.r;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          delete h_.r;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.r;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.r;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          delete h_.r;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.r;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.r;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.r;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.r;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.r;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Float:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Float:
          h_.d = x.h_.d;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Real:
      switch(x.t_){
        case None:
          delete h_.x;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.x;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.x;
          t_ = False;
          return *this;
        case True:
          delete h_.x;
          t_ = True;
          return *this;
        case Real:
          *h_.x = *x.h_.x;
          return *this;
        case Rational:
          delete h_.x;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.x;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.x;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.x;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.x;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          delete h_.x;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          delete h_.x;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.x;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.x;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          delete h_.x;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.x;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.x;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.x;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.x;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.x;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case String:
    case Binary:
    case Symbol:
      switch(x.t_){
        case None:
          delete h_.s;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.s;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.s;
          t_ = False;
          return *this;
        case True:
          delete h_.s;
          t_ = True;
          return *this;
        case Rational:
          delete h_.s;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          delete h_.s;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          *h_.s = *x.h_.s;
          return *this;
        case LocalObject:
          delete h_.s;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.s;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.s;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          delete h_.s;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          delete h_.s;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.s;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.s;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          delete h_.s;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.s;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.s;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.s;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.s;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.s;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case StringPointer:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case RawPointer:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case RawPointer:
          h_.p = x.h_.p;
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case ObjectPointer:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case ObjectPointer:
          h_.o = x.h_.o;
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Vector:
      switch(x.t_){
        case None:
          delete h_.v;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.v;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.v;
          t_ = False;
          return *this;
        case True:
          delete h_.v;
          t_ = True;
          return *this;
        case Rational:
          delete h_.v;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          delete h_.v;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.v;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.v;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.v;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          *h_.v = *x.h_.v;
          return *this;
        case List:
          delete h_.v;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          delete h_.v;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.v;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.v;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          delete h_.v;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.v;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.v;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.v;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.v;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.v;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case List:
      switch(x.t_){
        case None:
          delete h_.l;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.l;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.l;
          t_ = False;
          return *this;
        case True:
          delete h_.l;
          t_ = True;
          return *this;
        case Rational:
          delete h_.l;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          delete h_.l;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.l;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.l;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.l;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.l;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          *h_.l = *x.h_.l;
          return *this;
        case Function:
          delete h_.l;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.l;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.l;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          delete h_.l;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.l;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.l;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.l;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.l;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.l;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Function:
      switch(x.t_){
        case None:
          delete h_.f;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.f;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.f;
          t_ = False;
          return *this;
        case True:
          delete h_.f;
          t_ = True;
          return *this;
        case Rational:
          delete h_.f;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          delete h_.f;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.f;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.f;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.f;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.f;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          delete h_.f;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          h_.f->f = x.h_.f->f;
          h_.f->v = x.h_.f->v;
          return *this;
        case HeadSequence:
          delete h_.f;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.f;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          delete h_.f;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.f;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.f;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.f;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.f;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.f;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Map:
      switch(x.t_){
        case None:
          delete h_.m;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.m;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.m;
          t_ = False;
          return *this;
        case True:
          delete h_.m;
          t_ = True;
          return *this;
        case Rational:
          delete h_.m;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          delete h_.m;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.m;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.m;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.m;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.m;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          delete h_.l;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          delete h_.m;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.m;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          *h_.m = *x.h_.m;
          return *this;
        case Multimap:
          delete h_.m;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.m;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.m;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.m;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.m;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.m;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Multimap:
      switch(x.t_){
        case None:
          delete h_.mm;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.mm;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.mm;
          t_ = False;
          return *this;
        case True:
          delete h_.mm;
          t_ = True;
          return *this;
        case Rational:
          delete h_.mm;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          delete h_.mm;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.mm;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.mm;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.mm;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.mm;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          delete h_.l;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          delete h_.mm;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.mm;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.mm;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          *h_.mm = *x.h_.mm;
          return *this;
        case HeadMap:
          delete h_.mm;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.mm;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.mm;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.mm;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.mm;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Reference:
      switch(x.t_){
        case None:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = None;
          return *this;
        case Undefined:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = Undefined;
          return *this;
        case False:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = False;
          return *this;
        case True:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = True;
          return *this;
        case Rational:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Pointer:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        case Pointer:
          h_.vp = x.h_.vp;
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case LocalObject:
      delete h_.o;
      break;
    case SharedObject:
      if(h_.o->deref()){
        delete h_.o;
      }
      break;
    case HeadSequence:
      h_.hs->dealloc();
      delete h_.hs;
      break;
    case HeadMap:
      h_.hm->dealloc();
      delete h_.hm;
      break;
    case SequenceMap:
      h_.sm->dealloc();
      delete h_.sm;
      break;
    case HeadSequenceMap:
      h_.hsm->dealloc();
      delete h_.hsm;
      break;
  }
  
  t_ = x.t_;
  
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
      return *this;
    case Rational:
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      h_.i = x.h_.i;
      return *this;
  }
  
  return *this;
}

nvar& nvar::set(const nvar& x){
  switch(t_){
    case None:
      switch(x.t_){
        case None:
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Undefined:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case False:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case True:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Integer:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Integer:
          h_.i = x.h_.i;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Rational:
      switch(x.t_){
        case None:
          delete h_.r;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.r;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.r;
          t_ = False;
          return *this;
        case True:
          delete h_.r;
          t_ = True;
          return *this;
        case Rational:
          *h_.r = *x.h_.r;
          return *this;
        case Real:
          delete h_.r;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.r;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.r;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.r;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.r;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          delete h_.r;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          delete h_.r;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.r;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.r;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          delete h_.r;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.r;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.r;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.r;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.r;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.r;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Float:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Float:
          h_.d = x.h_.d;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Real:
      switch(x.t_){
        case None:
          delete h_.x;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.x;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.x;
          t_ = False;
          return *this;
        case True:
          delete h_.x;
          t_ = True;
          return *this;
        case Real:
          *h_.x = *x.h_.x;
          return *this;
        case Rational:
          delete h_.x;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.x;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.x;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.x;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.x;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          delete h_.x;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          delete h_.x;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.x;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.x;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          delete h_.x;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.x;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.x;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.x;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.x;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.x;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case String:
    case Binary:
      switch(x.t_){
        case None:
          delete h_.s;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.s;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.s;
          t_ = False;
          return *this;
        case True:
          delete h_.s;
          t_ = True;
          return *this;
        case Rational:
          delete h_.s;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          delete h_.s;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          *h_.s = *x.h_.s;
          return *this;
        case LocalObject:
          delete h_.s;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.s;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.s;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          delete h_.s;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          delete h_.s;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.s;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.s;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          delete h_.s;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.s;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.s;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.s;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.s;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.s;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case StringPointer:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case RawPointer:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case RawPointer:
          h_.p = x.h_.p;
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case ObjectPointer:
      switch(x.t_){
        case None:
          t_ = None;
          return *this;
        case Undefined:
          t_ = Undefined;
          return *this;
        case False:
          t_ = False;
          return *this;
        case True:
          t_ = True;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case ObjectPointer:
          h_.o = x.h_.o;
          return *this;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Vector:
      switch(x.t_){
        case None:
          delete h_.v;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.v;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.v;
          t_ = False;
          return *this;
        case True:
          delete h_.v;
          t_ = True;
          return *this;
        case Rational:
          delete h_.v;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          delete h_.v;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.v;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.v;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.v;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          *h_.v = *x.h_.v;
          return *this;
        case List:
          delete h_.v;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          delete h_.v;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.v;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.v;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          delete h_.v;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.v;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.v;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.v;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.v;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.v;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case List:
      switch(x.t_){
        case None:
          delete h_.l;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.l;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.l;
          t_ = False;
          return *this;
        case True:
          delete h_.l;
          t_ = True;
          return *this;
        case Rational:
          delete h_.l;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          delete h_.l;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.l;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.l;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.l;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.l;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          *h_.l = *x.h_.l;
          return *this;
        case Function:
          delete h_.l;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.l;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.l;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          delete h_.l;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.l;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.l;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.l;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.l;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.l;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Set");
      f->v.push_back(*this);
      f->v.push_back(nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Map:
      switch(x.t_){
        case None:
          delete h_.m;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.m;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.m;
          t_ = False;
          return *this;
        case True:
          delete h_.m;
          t_ = True;
          return *this;
        case Rational:
          delete h_.m;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          delete h_.m;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.m;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.m;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.m;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.m;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          delete h_.l;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          delete h_.m;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.m;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          *h_.m = *x.h_.m;
          return *this;
        case Multimap:
          delete h_.m;
          t_ = Multimap;
          h_.mm = new nmmap(*x.h_.mm);
          return *this;
        case HeadMap:
          delete h_.m;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.m;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.m;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.m;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.m;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Multimap:
      switch(x.t_){
        case None:
          delete h_.mm;
          t_ = None;
          return *this;
        case Undefined:
          delete h_.mm;
          t_ = Undefined;
          return *this;
        case False:
          delete h_.mm;
          t_ = False;
          return *this;
        case True:
          delete h_.mm;
          t_ = True;
          return *this;
        case Rational:
          delete h_.mm;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          return *this;
        case Real:
          delete h_.mm;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.mm;
          t_ = x.t_;
          h_.s = new nstr(*x.h_.s);
          return *this;
        case LocalObject:
          delete h_.mm;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          return *this;
        case SharedObject:
          delete h_.mm;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          return *this;
        case Vector:
          delete h_.mm;
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          return *this;
        case List:
          delete h_.l;
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          return *this;
        case Function:
          delete h_.mm;
          t_ = Function;
          h_.f = x.h_.f->clone();
          return *this;
        case HeadSequence:
          delete h_.mm;
          t_ = HeadSequence;
          h_.hs = x.h_.hs->clone();
          return *this;
        case Map:
          delete h_.mm;
          t_ = Map;
          h_.m = new nmap(*x.h_.m);
          return *this;
        case Multimap:
          *h_.mm = *x.h_.mm;
          return *this;
        case HeadMap:
          delete h_.mm;
          t_ = HeadMap;
          h_.hm = x.h_.hm->clone();
          return *this;
        case SequenceMap:
          delete h_.mm;
          t_ = SequenceMap;
          h_.sm = x.h_.sm->clone();
          return *this;
        case HeadSequenceMap:
          delete h_.mm;
          t_ = HeadSequenceMap;
          h_.hsm = x.h_.hsm->clone();
          return *this;
        case Reference:
          delete h_.mm;
          t_ = Reference;
          h_.ref = x.h_.ref;
          h_.ref->ref();
          return *this;
        default:
          delete h_.mm;
          t_ = x.t_;
          h_.i = x.h_.i;
          return *this;
      }
      return *this;
    case Reference:
      return h_.ref->v->set(x);
    case Pointer:
      return h_.vp->set(x);
    case LocalObject:
      delete h_.o;
      break;
    case SharedObject:
      if(h_.o->deref()){
        delete h_.o;
      }
      break;
    case HeadSequence:
      h_.hs->dealloc();
      delete h_.hs;
      break;
    case HeadMap:
      h_.hm->dealloc();
      delete h_.hm;
      break;
    case SequenceMap:
      h_.sm->dealloc();
      delete h_.sm;
      break;
    case HeadSequenceMap:
      h_.hsm->dealloc();
      delete h_.hsm;
      break;
  }
  
  t_ = x.t_;
  
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
      return *this;
    case Rational:
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      h_.i = x.h_.i;
      return *this;
  }
  
  return *this;
}

nvar nvar::operator-() const{
  switch(t_){
    case Integer:
      return -h_.i;
    case Rational:
      return -*h_.r;
    case Float:
      return -h_.d;
    case Real:
      return -*h_.x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Neg");
      h.f->v.push_back(new nvar(*this, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      nvar ret;
      ret.t_ = Vector;
      ret.h_.v = new nvec(-*h_.v);
      return ret;
    }
    case HeadSequence:
      return -*h_.hs->s;
    case HeadMap:
      return -*h_.hm->h;
    case SequenceMap:
      return -*h_.sm->s;
    case HeadSequenceMap:
      return -*h_.hsm->s;
    case Reference:
      return -*h_.ref->v;
    case Pointer:
      return -*h_.vp;
  }
  
  NERROR("invalid operand");
}

nvar& nvar::operator+=(int64_t x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      h_.i += x;
      return *this;
    case Rational:
      *h_.r += x;
      return *this;
    case Float:
      h_.d += x;
      return *this;
    case Real:
      *h_.x += x;
      return *this;
    case String:
    case StringPointer:
      NERROR("invalid operands");
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Add");
      f->v.push_back(new nvar(std::move(*this)));
      f->v.push_back(x);
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v += x;
      return *this;
    case HeadSequence:
      *h_.hs->s += x;
      return *this;
    case HeadMap:
      *h_.hm->h += x;
      return *this;
    case SequenceMap:
      *h_.sm->s += x;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s += x;
      return *this;
    case Reference:
      *h_.ref->v += x;
      return *this;
    case Pointer:
      *h_.vp += x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
}

nvar& nvar::operator+=(double x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      t_ = Float;
      h_.d = h_.i;
      h_.d += x;
      return *this;
    case Rational:{
      double d = h_.r->toDouble();
      delete h_.r;
      t_ = Float;
      h_.d = d + x;
      return *this;
    }
    case Float:
      h_.d += x;
      return *this;
    case Real:{
      double d = h_.x->toDouble();
      delete h_.x;
      h_.d = d + x;
      t_ = Float;
      break;
    }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Add");
      f->v.push_back(new nvar(std::move(*this)));
      f->v.push_back(x);
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v += x;
      return *this;
    case HeadSequence:
      *h_.hs->s += x;
      return *this;
    case HeadMap:
      *h_.hm->h += x;
      return *this;
    case SequenceMap:
      *h_.sm->s += x;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s += x;
      return *this;
    case Reference:
      *h_.ref->v += x;
      return *this;
    case Pointer:
      *h_.vp += x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
  
  return *this;
}


nvar& nvar::operator+=(const nvar& x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          h_.i += x.h_.i;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(h_.i);
          *h_.r += *x.h_.r;
          if(h_.r->denominator() == 1){
            t_ = Integer;
            nrat* r = h_.r;
            h_.i = r->numerator();
            delete r;
            return *this;
          }
          return *this;
        case Float:
          t_ = Float;
          h_.d = h_.i;
          h_.d += x.h_.d;
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(h_.i);
          *h_.x += *x.h_.x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Add");
          f->v.push_back(h_.i);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.i);
          *v += *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
          *v += *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this += *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
          *v += *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
          *v += *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this += *x.h_.ref->v;
        case Pointer:
          return *this += *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          *h_.r += x.h_.i;
          return *this;
        case Rational:
          *h_.r += *x.h_.r;
          if(h_.r->denominator() == 1){
            nrat* r = h_.r;
            h_.i = h_.r->numerator();
            t_ = Integer;
            delete r;
          }
          return *this;
        case Float:{
          double d = h_.r->toDouble();
          delete h_.r;
          t_ = Float;
          h_.d = d + x.h_.d;
          return *this;
        }
        case Real:{
          nreal* y = new nreal(*h_.r);
          delete h_.r;
          t_ = Real;
          *y += *x.h_.x;
          h_.x = y;
          return *this;
        }
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Add");
          f->v.push_back(*h_.r);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), *h_.r);
          *v += *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), *h_.r);
          *v += *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this += *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), *h_.r);
          *v += *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), *h_.r);
          *v += *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this += *x.h_.ref->v;
        case Pointer:
          return *this += *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          h_.d += x.h_.i;
          return *this;
        case Rational:
          h_.d += x.h_.r->toDouble();
          return *this;
        case Float:
          h_.d += x.h_.d;
          return *this;
        case Real:
          h_.d += x.h_.x->toDouble();
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Add");
          f->v.push_back(h_.d);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.d);
          *v += *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.d);
          *v += *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this += *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.d);
          *v += *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.d);
          *v += *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this += *x.h_.ref->v;
        case Pointer:
          return *this += *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          *h_.x += x.h_.i;
          return *this;
        case Rational:
          *h_.x += *x.h_.r;
          return *this;
        case Float:{
          double d = h_.x->toDouble();
          d += x.h_.d;
          delete h_.x;
          h_.d = d;
          t_ = Float;
        }
        case Real:
          *h_.x += *x.h_.x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Add");
          f->v.push_back(*h_.x);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), *h_.x);
          *v += *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), *h_.x);
          *v += *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this += *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), *h_.x);
          *v += *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), *h_.x);
          *v += *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this += *x.h_.ref->v;
        case Pointer:
          return *this += *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case String:
    case StringPointer:
      switch(x.t_){
        case String:
        case StringPointer:
          *h_.s += *x.h_.s;
          return *this;
        case Reference:
          return *this += *x.h_.ref->v;
        case Pointer:
          return *this += *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Add");
      f->v.emplace_back(new nvar(*this, Copy));
      f->v.emplace_back(nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.v += x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Add");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v += *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.v += *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this += *x.h_.hm->h;
        case SequenceMap:
          *h_.v += *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.v += *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this += *x.h_.ref->v;
        case Pointer:
          return *this += *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequence:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.hs->s += x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Add");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.hs->s += *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.hs->s += *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this += *x.h_.hm->h;
        case SequenceMap:
          *h_.hs->s += *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.hs->s += *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this += *x.h_.ref->v;
        case Pointer:
          return *this += *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadMap:
      return *h_.hm->h += x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.sm->s += x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Add");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.sm->s += *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.sm->s += *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this += *x.h_.hm->h;
        case SequenceMap:
          *h_.sm->s += *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.sm->s += *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this += *x.h_.ref->v;
        case Pointer:
          return *this += *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.hsm->s += x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Add");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          *h_.hsm->s += *x.h_.v;
          return *this;
        }
        case HeadSequence:
          *h_.hsm->s += *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this += *x.h_.hm->h;
        case SequenceMap:
          *h_.hsm->s += *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s += *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this += *x.h_.ref->v;
        case Pointer:
          return *this += *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Reference:
      *h_.ref->v += x;
      return *this;
    case Pointer:
      *h_.vp += x;
      return *this;
    default:
      cout << "type is: " << int(t_) << endl;
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator+(int64_t x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      return h_.i + x;
    case Rational:{
      Head h;
      h.r = new nrat(*h_.r);
      h.r += x;
      return nvar(Rational, h);
    }
    case Float:
      return h_.d + x;
    case Real:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x += x;
      return nvar(Real, h);
    }
    case Binary:
    case String:
    case StringPointer:
      NERROR("invalid operands");
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Add");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v + x;
    case HeadSequence:
      return *h_.hs->s + x;
    case HeadMap:
      return *h_.hm->h + x;
    case SequenceMap:
      return *h_.sm->s += x;
    case HeadSequenceMap:
      return *h_.hsm->s + x;
    case Reference:
      return *h_.ref->v + x;
    case Pointer:
      return *h_.vp + x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator+(double x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      return h_.i + x;
    case Rational:
      return h_.r->toDouble() + x;
    case Float:
      return h_.d + x;
    case Real:
      return h_.x->toDouble() + x;
    case Binary:
    case String:
    case StringPointer:
      NERROR("invalid operands");
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Add");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v + x;
    case HeadSequence:
      return *h_.hs->s + x;
    case HeadMap:
      return *h_.hm->h + x;
    case SequenceMap:
      return *h_.sm->s = x;
    case HeadSequenceMap:
      return *h_.hsm->s + x;
    case Reference:
      return *h_.ref->v + x;
    case Pointer:
      return *h_.vp + x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator+(const nvar& x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return h_.i + x.h_.i;
        case Rational:{
          Head h;
          h.r = new nrat(h_.i);
          *h.r += *x.h_.r;
          if(h.r->denominator() == 1){
            nrat* r = h.r;
            h.i = r->numerator();
            delete r;
            return nvar(Integer, h);
          }
          return nvar(Rational, h);
        }
        case Float:
          return h_.i + x.h_.d;
        case Real:{
          Head h;
          h.x = new nreal(h_.i);
          *h.x += *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Add");
          h.f->v.push_back(h_.i);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.i);
          *v += *x.h_.v;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
          *v += *x.h_.hs->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this + *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
          *v += *x.h_.sm->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
          *v += *x.h_.hsm->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case Reference:
          return *this + *x.h_.ref->v;
        case Pointer:
          return *this + *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          Head h;
          h.r = new nrat(*h_.r);
          *h.r += x.h_.i;
          return nvar(Rational, h);
        case Rational:{
          Head h;
          h.r = new nrat(*h_.r);
          *h.r += *x.h_.r;
          if(h.r->denominator() == 1){
            nrat* r = h.r;
            h.i = h_.r->numerator();
            delete r;
            return nvar(Integer, h);
          }
          return nvar(Rational, h);
        }
        case Float:
          return h_.r->toDouble() + x.h_.d;
        case Real:{
          Head h;
          h.x = new nreal(*h_.r);
          *h.x += *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Add");
          h.f->v.push_back(*h_.r);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.r);
          *h.v += *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.r);
          *h.v += *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this + *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.r);
          *h.v += *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
          *h.v += *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this + *x.h_.ref->v;
        case Pointer:
          return *this + *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return h_.d + x.h_.i;
        case Rational:
          return h_.d + x.h_.r->toDouble();
        case Float:
          return h_.d + x.h_.d;
        case Real:
          return h_.d + x.h_.x->toDouble();
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Add");
          h.f->v.push_back(h_.d);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), h_.d);
          *h.v += *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), h_.d);
          *h.v += *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this + *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), h_.d);
          *h.v += *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), h_.d);
          *h.v += *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this + *x.h_.ref->v;
        case Pointer:
          return *this + *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          Head h;
          h.x = new nreal(*h_.x);
          *h.x += x.h_.i;
          return nvar(Real, h);
        case Rational:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x += *x.h_.r;
          return nvar(Real, h);
        }
        case Float:
          return h_.x->toDouble() + x.h_.d;
        case Real:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x += *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Add");
          h.f->v.push_back(*h_.x);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.x);
          *h.v += *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.x);
          *h.v += *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this + *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.x);
          *h.v += *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
          *h.v += *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this + *x.h_.ref->v;
        case Pointer:
          return *this + *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case String:
    case StringPointer:
      switch(x.t_){
        case String:
        case StringPointer:{
          Head h;
          h.s = new nstr(*h_.s);
          *h.s += *x.h_.s;
          return nvar(String, h);
        }
        case HeadMap:
          return *this + *x.h_.hm->h;
        case Reference:
          return *this + *x.h_.ref->v;
        case Pointer:
          return *this + *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("Add");
      h.f->v.emplace_back(new nvar(*this, Copy));
      h.f->v.emplace_back(nvar(x, Copy));
      
      return new nvar(Function, h);
    }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.v + x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Add");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v + *x.h_.v;
        case HeadSequence:
          return *h_.v + *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this + *x.h_.hm->h;
        case SequenceMap:
          return *h_.v + *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.v + *x.h_.hsm->s;
        case Reference:
          return *this + *x.h_.ref->v;
        case Pointer:
          return *this + *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequence:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hs->s + x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Add");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hs->s + *x.h_.v;
        case HeadSequence:
          return *h_.hs->s + *x.h_.hs->s;
        case HeadMap:
          return *this + *x.h_.hm->h;
        case SequenceMap:
          return *h_.hs->s + *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hs->s + *x.h_.hsm->s;
        case Reference:
          return *this + *x.h_.ref->v;
        case Pointer:
          return *this + *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadMap:
      return *h_.hm->h + x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.sm->s + x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Add");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.sm->s + *x.h_.v;
        case HeadSequence:
          return *h_.sm->s + *x.h_.hs->s;
        case HeadMap:
          return *this + *x.h_.hm->h;
        case SequenceMap:
          return *h_.sm->s + *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.sm->s + *x.h_.hsm->s;
        case Reference:
          return *this + *x.h_.ref->v;
        case Pointer:
          return *this + *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hsm->s + x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Add");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hsm->s + *x.h_.v;
        case HeadSequence:
          return *h_.hsm->s + *x.h_.hs->s;
        case HeadMap:
          return *this + *x.h_.hm->h;
        case SequenceMap:
          return *h_.hsm->s + *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hsm->s + *x.h_.hsm->s;
        case Reference:
          return *this + *x.h_.ref->v;
        case Pointer:
          return *this + *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Reference:
      return *h_.ref->v + x;
    case Pointer:
      return *h_.vp + x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar& nvar::operator-=(int64_t x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      h_.i -= x;
      return *this;
    case Rational:
      *h_.r -= x;
      return *this;
    case Float:
      h_.d -= x;
      return *this;
    case Real:
      *h_.x -= x;
      return *this;
    case String:
    case StringPointer:
      NERROR("invalid operands");
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Sub");
      f->v.push_back(new nvar(std::move(*this)));
      f->v.push_back(x);
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v -= x;
      return *this;
    case HeadSequence:
      *h_.hs->s -= x;
      return *this;
    case HeadMap:
      *h_.hm->h -= x;
      return *this;
    case SequenceMap:
      *h_.sm->s -= x;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s -= x;
      return *this;
    case Reference:
      *h_.ref->v -= x;
      return *this;
    case Pointer:
      *h_.vp -= x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
}

nvar& nvar::operator-=(double x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      t_ = Float;
      h_.d = h_.i;
      h_.d -= x;
      return *this;
    case Rational:{
      double d = h_.r->toDouble();
      delete h_.r;
      t_ = Float;
      h_.d = d + x;
      return *this;
    }
    case Float:
      h_.d -= x;
      return *this;
    case Real:{
      double d = h_.x->toDouble();
      d -= x;
      delete h_.x;
      h_.d = d;
      t_ = Float;
      break;
    }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Sub");
      f->v.push_back(new nvar(std::move(*this)));
      f->v.push_back(x);
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v -= x;
      return *this;
    case HeadSequence:
      *h_.hs->s -= x;
      return *this;
    case HeadMap:
      *h_.hm->h -= x;
      return *this;
    case SequenceMap:
      *h_.sm->s -= x;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s -= x;
      return *this;
    case Reference:
      *h_.ref->v -= x;
      return *this;
    case Pointer:
      *h_.vp -= x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
  
  return *this;
}

nvar& nvar::operator-=(const nvar& x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          h_.i -= x.h_.i;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(h_.i);
          *h_.r -= *x.h_.r;
          if(h_.r->denominator() == 1){
            t_ = Integer;
            nrat* r = h_.r;
            h_.i = r->numerator();
            delete r;
            return *this;
          }
          return *this;
        case Float:
          t_ = Float;
          h_.d = h_.i;
          h_.d -= x.h_.d;
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(h_.i);
          *h_.x -= *x.h_.x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Sub");
          f->v.push_back(h_.i);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.i);
          *v -= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
          *v -= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this -= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
          *v -= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
          *v -= *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this -= *x.h_.ref->v;
        case Pointer:
          return *this -= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          *h_.r -= x.h_.i;
          return *this;
        case Rational:
          *h_.r -= *x.h_.r;
          if(h_.r->denominator() == 1){
            nrat* r = h_.r;
            h_.i = h_.r->numerator();
            t_ = Integer;
            delete r;
          }
          return *this;
        case Float:{
          double d = h_.r->toDouble();
          delete h_.r;
          t_ = Float;
          h_.d = d + x.h_.d;
          return *this;
        }
        case Real:{
          nreal* y = new nreal(*h_.r);
          delete h_.r;
          t_ = Real;
          *y -= *x.h_.x;
          h_.x = y;
          return *this;
        }
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Sub");
          f->v.push_back(*h_.r);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), *h_.r);
          *v -= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), *h_.r);
          *v -= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this -= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), *h_.r);
          *v -= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), *h_.r);
          *v -= *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this -= *x.h_.ref->v;
        case Pointer:
          return *this -= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          h_.d -= x.h_.i;
          return *this;
        case Rational:
          h_.d -= x.h_.r->toDouble();
          return *this;
        case Float:
          h_.d -= x.h_.d;
          return *this;
        case Real:
          h_.d -= x.h_.x->toDouble();
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Sub");
          f->v.push_back(h_.d);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.d);
          *v -= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.d);
          *v -= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this -= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.d);
          *v -= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.d);
          *v -= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this -= *x.h_.ref->v;
        case Pointer:
          return *this -= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          *h_.x -= x.h_.i;
          return *this;
        case Rational:
          *h_.x -= *x.h_.r;
          return *this;
        case Float:{
          double d = h_.x->toDouble();
          d -= x.h_.d;
          delete h_.x;
          h_.d = d;
          t_ = Float;
        }
        case Real:
          *h_.x -= *x.h_.x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Sub");
          f->v.push_back(*h_.x);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), *h_.x);
          *v -= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), *h_.x);
          *v -= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this -= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), *h_.x);
          *v -= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), *h_.x);
          *v -= *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this -= *x.h_.ref->v;
        case Pointer:
          return *this -= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Sub");
      f->v.emplace_back(new nvar(*this, Copy));
      f->v.emplace_back(nvar(x, Copy));
      
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.v -= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Sub");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v -= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.v -= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this -= *x.h_.hm->h;
        case SequenceMap:
          *h_.v -= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.v -= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this -= *x.h_.ref->v;
        case Pointer:
          return *this -= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequence:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.hs->s -= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Sub");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          *h_.hs->s -= *x.h_.v;
          return *this;
        }
        case HeadSequence:
          *h_.hs->s -= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this -= *x.h_.hm->h;
        case SequenceMap:
          *h_.hs->s -= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.hs->s -= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this -= *x.h_.ref->v;
        case Pointer:
          return *this -= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadMap:
      return *h_.hm->h -= x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.sm->s -= x;
          return *this;
        case Vector:
          *h_.sm->s -= *x.h_.v;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Sub");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case HeadSequence:
          *h_.sm->s -= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this -= *x.h_.hm->h;
        case SequenceMap:
          *h_.sm->s -= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.sm->s -= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this -= *x.h_.ref->v;
        case Pointer:
          return *this -= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.hsm->s -= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Sub");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.hsm->s -= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.hsm->s -= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this -= *x.h_.hm->h;
        case SequenceMap:
          *h_.hsm->s -= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s -= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this -= *x.h_.ref->v;
        case Pointer:
          return *this -= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Reference:
      *h_.ref->v -= x;
      return *this;
    case Pointer:
      *h_.vp -= x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator-(int64_t x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      return h_.i - x;
    case Rational:{
      Head h;
      h.r = new nrat(*h_.r);
      h.r -= x;
      return nvar(Rational, h);
    }
    case Float:
      return h_.d - x;
    case Real:{
      Head h;
      h.x = new nreal(*h_.x);
      h.x -= x;
      return nvar(Real, h);
    }
    case Binary:
    case String:
    case StringPointer:
      NERROR("invalid operands");
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Sub");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v - x;
    case HeadSequence:
      return *h_.hs->s - x;
    case HeadMap:
      return *h_.hm->h - x;
    case SequenceMap:
      return *h_.sm->s - x;
    case HeadSequenceMap:
      return *h_.hsm->s - x;
    case Reference:
      return *h_.ref->v - x;
    case Pointer:
      return *h_.vp - x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator-(double x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      return h_.i - x;
    case Rational:
      return h_.r->toDouble() - x;
    case Float:
      return h_.d - x;
    case Real:
      return h_.x->toDouble() - x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Sub");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v - x;
    case HeadSequence:
      return *h_.hs->s - x;
    case HeadMap:
      return *h_.hm->h - x;
    case SequenceMap:
      return *h_.sm->s -= x;
    case HeadSequenceMap:
      return *h_.hsm->s - x;
    case Reference:
      return *h_.ref->v - x;
    case Pointer:
      return *h_.vp - x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator-(const nvar& x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return h_.i - x.h_.i;
        case Rational:{
          Head h;
          h.r = new nrat(h_.i);
          *h.r -= *x.h_.r;
          if(h.r->denominator() == 1){
            nrat* r = h.r;
            h.i = r->numerator();
            delete r;
            return nvar(Integer, h);
          }
          return nvar(Rational, h);
        }
        case Float:
          return h_.i - x.h_.d;
        case Real:{
          Head h;
          h.x = new nreal(h_.i);
          *h.x -= *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Sub");
          h.f->v.push_back(h_.i);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.i);
          *v -= *x.h_.v;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
          *v -= *x.h_.hs->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this - *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
          *v -= *x.h_.sm->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
          *v -= *x.h_.hsm->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case Reference:
          return *this - *x.h_.ref->v;
        case Pointer:
          return *this - *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          Head h;
          h.r = new nrat(*h_.r);
          h.r -= x.h_.i;
          return nvar(Rational, h);
        case Rational:{
          Head h;
          h.r = new nrat(*h_.r);
          *h.r -= *x.h_.r;
          if(h.r->denominator() == 1){
            nrat* r = h.r;
            h.i = h_.r->numerator();
            delete r;
            return nvar(Integer, h);
          }
          return nvar(Rational, h);
        }
        case Float:
          return h_.r->toDouble() - x.h_.d;
        case Real:{
          Head h;
          h.x = new nreal(*h_.r);
          *h.x -= *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Sub");
          h.f->v.push_back(*h_.r);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.r);
          *h.v -= *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.r);
          *h.v -= *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this - *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.r);
          *h.v -= *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
          *h.v -= *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this - *x.h_.ref->v;
        case Pointer:
          return *this - *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return h_.d - x.h_.i;
        case Rational:
          return h_.d - x.h_.r->toDouble();
        case Float:
          return h_.d - x.h_.d;
        case Real:
          return h_.d - x.h_.x->toDouble();
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Sub");
          h.f->v.push_back(h_.d);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), h_.d);
          *h.v -= *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), h_.d);
          *h.v -= *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this - *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), h_.d);
          *h.v -= *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), h_.d);
          *h.v -= *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this - *x.h_.ref->v;
        case Pointer:
          return *this - *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          Head h;
          h.x = new nreal(*h_.x);
          h.x -= x.h_.i;
          return nvar(Real, h);
        case Rational:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x -= *x.h_.r;
          return nvar(Real, h);
        }
        case Float:
          return h_.x->toDouble() - x.h_.d;
        case Real:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x -= *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Sub");
          h.f->v.push_back(*h_.x);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.x);
          *h.v -= *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.x);
          *h.v -= *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this - *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.x);
          *h.v -= *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
          *h.v -= *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this - *x.h_.ref->v;
        case Pointer:
          return *this - *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("Sub");
      h.f->v.emplace_back(new nvar(*this, Copy));
      h.f->v.emplace_back(nvar(x, Copy));
      
      return new nvar(Function, h);
    }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.v - x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Sub");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v - *x.h_.v;
        case HeadSequence:
          return *h_.v - *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this - *x.h_.hm->h;
        case SequenceMap:
          return *h_.v - *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.v - *x.h_.hsm->s;
        case Reference:
          return *this - *x.h_.ref->v;
        case Pointer:
          return *this - *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequence:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hs->s - x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Sub");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hs->s - *x.h_.v;
        case HeadSequence:
          return *h_.hs->s - *x.h_.hs->s;
        case HeadMap:
          return *this - *x.h_.hm->h;
        case SequenceMap:
          return *h_.hs->s - *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hs->s - *x.h_.hsm->s;
        case Reference:
          return *this - *x.h_.ref->v;
        case Pointer:
          return *this - *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadMap:
      return *h_.hm->h - x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.sm->s - x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Sub");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.sm->s - *x.h_.v;
        case HeadSequence:
          return *h_.sm->s - *x.h_.hs->s;
        case HeadMap:
          return *this - *x.h_.hm->h;
        case SequenceMap:
          return *h_.sm->s - *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.sm->s - *x.h_.hsm->s;
        case Reference:
          return *this - *x.h_.ref->v;
        case Pointer:
          return *this - *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hsm->s - x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Sub");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hsm->s - *x.h_.v;
        case HeadSequence:
          return *h_.hsm->s - *x.h_.hs->s;
        case HeadMap:
          return *this - *x.h_.hm->h;
        case SequenceMap:
          return *h_.hsm->s - *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hsm->s - *x.h_.hsm->s;
        case Reference:
          return *this - *x.h_.ref->v;
        case Pointer:
          return *this - *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Reference:
      return *h_.ref->v - x;
    case Pointer:
      return *h_.vp - x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar& nvar::operator*=(int64_t x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      h_.i *= x;
      return *this;
    case Rational:
      *h_.r *= x;
      return *this;
    case Float:
      h_.d *= x;
      return *this;
    case Real:
      *h_.x *= x;
      return *this;
    case String:
    case StringPointer:
      NERROR("invalid operands");
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mul");
      f->v.push_back(new nvar(std::move(*this)));
      f->v.push_back(x);
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v *= x;
      return *this;
    case HeadSequence:
      *h_.hs->s *= x;
      return *this;
    case HeadMap:
      *h_.hm->h *= x;
      return *this;
    case SequenceMap:
      *h_.sm->s *= x;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s *= x;
      return *this;
    case Reference:
      *h_.ref->v *= x;
      return *this;
    case Pointer:
      *h_.vp *= x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
}

nvar& nvar::operator*=(double x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      t_ = Float;
      h_.d = h_.i;
      h_.d *= x;
      return *this;
    case Rational:{
      double d = h_.r->toDouble();
      delete h_.r;
      t_ = Float;
      h_.d = d + x;
      return *this;
    }
    case Float:
      h_.d *= x;
      return *this;
    case Real:{
      double d = h_.x->toDouble();
      d *= x;
      delete h_.x;
      h_.d = d;
      t_ = Float;
      break;
    }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mul");
      f->v.push_back(new nvar(std::move(*this)));
      f->v.push_back(x);
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v *= x;
      return *this;
    case HeadSequence:
      *h_.hs->s *= x;
      return *this;
    case HeadMap:
      *h_.hm->h *= x;
      return *this;
    case SequenceMap:
      *h_.sm->s *= x;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s *= x;
      return *this;
    case Reference:
      *h_.ref->v *= x;
      return *this;
    case Pointer:
      *h_.vp *= x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
  
  return *this;
}

nvar& nvar::operator*=(const nvar& x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          h_.i *= x.h_.i;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(h_.i);
          *h_.r *= *x.h_.r;
          if(h_.r->denominator() == 1){
            t_ = Integer;
            nrat* r = h_.r;
            h_.i = r->numerator();
            delete r;
            return *this;
          }
          return *this;
        case Float:
          t_ = Float;
          h_.d = h_.i;
          h_.d *= x.h_.d;
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(h_.i);
          *h_.x *= *x.h_.x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mul");
          f->v.push_back(h_.i);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.i);
          *v *= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
          *v *= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this *= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
          *v *= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
          *v *= *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this *= *x.h_.ref->v;
        case Pointer:
          return *this *= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          *h_.r *= x.h_.i;
          return *this;
        case Rational:
          *h_.r *= *x.h_.r;
          if(h_.r->denominator() == 1){
            nrat* r = h_.r;
            h_.i = h_.r->numerator();
            t_ = Integer;
            delete r;
          }
          return *this;
        case Float:{
          double d = h_.r->toDouble();
          delete h_.r;
          t_ = Float;
          h_.d = d + x.h_.d;
          return *this;
        }
        case Real:{
          nreal* y = new nreal(*h_.r);
          delete h_.r;
          t_ = Real;
          *y *= *x.h_.x;
          h_.x = y;
          return *this;
        }
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mul");
          f->v.push_back(*h_.r);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), *h_.r);
          *v *= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), *h_.r);
          *v *= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this *= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), *h_.r);
          *v *= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), *h_.r);
          *v *= *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this *= *x.h_.ref->v;
        case Pointer:
          return *this *= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          h_.d *= x.h_.i;
          return *this;
        case Rational:
          h_.d *= x.h_.r->toDouble();
          return *this;
        case Float:
          h_.d *= x.h_.d;
          return *this;
        case Real:
          h_.d *= x.h_.x->toDouble();
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mul");
          f->v.push_back(h_.d);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.d);
          *v *= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.d);
          *v *= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this *= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.d);
          *v *= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.d);
          *v *= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this *= *x.h_.ref->v;
        case Pointer:
          return *this *= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          *h_.x *= x.h_.i;
          return *this;
        case Rational:
          *h_.x *= *x.h_.r;
          return *this;
        case Float:{
          double d = h_.x->toDouble();
          d *= x.h_.d;
          delete h_.x;
          h_.d = d;
          t_ = Float;
        }
        case Real:
          *h_.x *= *x.h_.x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mul");
          f->v.push_back(*h_.x);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), *h_.x);
          *v *= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), *h_.x);
          *v *= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this *= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), *h_.x);
          *v *= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), *h_.x);
          *v *= *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this *= *x.h_.ref->v;
        case Pointer:
          return *this *= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mul");
      f->v.emplace_back(new nvar(*this, Copy));
      f->v.emplace_back(nvar(x, Copy));
      
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.v *= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mul");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v *= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.v *= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this *= *x.h_.hm->h;
        case SequenceMap:
          *h_.v *= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.v *= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this *= *x.h_.ref->v;
        case Pointer:
          return *this *= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequence:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.hs->s *= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mul");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.hs->s *= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.hs->s *= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this *= *x.h_.hm->h;
        case SequenceMap:
          *h_.hs->s *= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.hs->s *= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this *= *x.h_.ref->v;
        case Pointer:
          return *this *= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadMap:
      return *h_.hm->h *= x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.sm->s *= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mul");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.sm->s *= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.sm->s *= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this *= *x.h_.hm->h;
        case SequenceMap:
          *h_.sm->s *= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.sm->s *= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this *= *x.h_.ref->v;
        case Pointer:
          return *this *= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.hsm->s *= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mul");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.hsm->s *= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.hsm->s *= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this *= *x.h_.hm->h;
        case SequenceMap:
          *h_.hsm->s *= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s *= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this *= *x.h_.ref->v;
        case Pointer:
          return *this *= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Reference:
      *h_.ref->v *= x;
      return *this;
    case Pointer:
      *h_.vp *= x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator*(const nvar& x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return h_.i * x.h_.i;
        case Rational:{
          Head h;
          h.r = new nrat(h_.i);
          *h.r *= *x.h_.r;
          if(h.r->denominator() == 1){
            nrat* r = h.r;
            h.i = r->numerator();
            delete r;
            return nvar(Integer, h);
          }
          return nvar(Rational, h);
        }
        case Float:
          return h_.i * x.h_.d;
        case Real:{
          Head h;
          h.x = new nreal(h_.i);
          *h.x *= *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mul");
          h.f->v.push_back(h_.i);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.i);
          *v *= *x.h_.v;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
          *v *= *x.h_.hs->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this * *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
          *v *= *x.h_.sm->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
          *v *= *x.h_.hsm->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case Reference:
          return *this * *x.h_.ref->v;
        case Pointer:
          return *this * *x.h_.vp;
        default:
          cout << "x.t_ is: " << int(x.t_) << endl;
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          Head h;
          h.r = new nrat(*h_.r);
          *h.r *= x.h_.i;
          return nvar(Rational, h);
        case Rational:{
          Head h;
          h.r = new nrat(*h_.r);
          *h.r *= *x.h_.r;
          if(h.r->denominator() == 1){
            nrat* r = h.r;
            h.i = h_.r->numerator();
            delete r;
            return nvar(Integer, h);
          }
          return nvar(Rational, h);
        }
        case Float:
          return h_.r->toDouble() * x.h_.d;
        case Real:{
          Head h;
          h.x = new nreal(*h_.r);
          *h.x *= *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mul");
          h.f->v.push_back(*h_.r);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.r);
          *h.v *= *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.r);
          *h.v *= *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this * *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.r);
          *h.v *= *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
          *h.v *= *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this * *x.h_.ref->v;
        case Pointer:
          return *this * *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return h_.d * x.h_.i;
        case Rational:
          return h_.d * x.h_.r->toDouble();
        case Float:
          return h_.d * x.h_.d;
        case Real:
          return h_.d * x.h_.x->toDouble();
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mul");
          h.f->v.push_back(h_.d);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), h_.d);
          *h.v *= *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), h_.d);
          *h.v *= *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this * *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), h_.d);
          *h.v *= *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), h_.d);
          *h.v *= *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this * *x.h_.ref->v;
        case Pointer:
          return *this * *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          Head h;
          h.x = new nreal(*h_.x);
          *h.x *= x.h_.i;
          return nvar(Real, h);
        case Rational:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x *= *x.h_.r;
          return nvar(Real, h);
        }
        case Float:
          return h_.x->toDouble() * x.h_.d;
        case Real:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x *= *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mul");
          h.f->v.push_back(*h_.x);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.x);
          *h.v *= *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.x);
          *h.v *= *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this * *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.x);
          *h.v *= *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
          *h.v *= *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this * *x.h_.ref->v;
        case Pointer:
          return *this * *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("Mul");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(nvar(x, Copy));
      
      return new nvar(Function, h);
    }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.v * x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mul");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v * *x.h_.v;
        case HeadSequence:
          return *h_.v * *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this * *x.h_.hm->h;
        case SequenceMap:
          return *h_.v * *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.v * *x.h_.hsm->s;
        case Reference:
          return *this * *x.h_.ref->v;
        case Pointer:
          return *this * *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequence:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hs->s * x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mul");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hs->s * *x.h_.v;
        case HeadSequence:
          return *h_.hs->s * *x.h_.hs->s;
        case HeadMap:
          return *this * *x.h_.hm->h;
        case SequenceMap:
          return *h_.hs->s * *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hs->s * *x.h_.hsm->s;
        case Reference:
          return *this * *x.h_.ref->v;
        case Pointer:
          return *this * *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadMap:
      return *h_.hm->h * x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.sm->s = x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mul");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.sm->s * *x.h_.v;
        case HeadSequence:
          return *h_.sm->s * *x.h_.hs->s;
        case HeadMap:
          return *this * *x.h_.hm->h;
        case SequenceMap:
          return *h_.sm->s * *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.sm->s * *x.h_.hsm->s;
        case Reference:
          return *this * *x.h_.ref->v;
        case Pointer:
          return *this * *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hsm->s * x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mul");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hsm->s * *x.h_.v;
        case HeadSequence:
          return *h_.hsm->s * *x.h_.hs->s;
        case HeadMap:
          return *this * *x.h_.hm->h;
        case SequenceMap:
          return *h_.hsm->s * *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hsm->s * *x.h_.hsm->s;
        case Reference:
          return *this * *x.h_.ref->v;
        case Pointer:
          return *this * *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Reference:
      return *h_.ref->v * x;
    case Pointer:
      return *h_.vp * x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator*(int64_t x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      return h_.i * x;
    case Rational:{
      Head h;
      h.r = new nrat(*h_.r);
      *h.r *= x;
      return nvar(Rational, h);
    }
    case Float:
      return h_.d * x;
    case Real:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x *= x;
      return nvar(Real, h);
    }
    case Binary:
    case String:
    case StringPointer:
      NERROR("invalid operands");
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mul");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v * x;
    case HeadSequence:
      return *h_.hs->s * x;
    case HeadMap:
      return *h_.hm->h * x;
    case SequenceMap:
      return *h_.sm->s *= x;
    case HeadSequenceMap:
      return *h_.hsm->s * x;
    case Reference:
      return *h_.ref->v * x;
    case Pointer:
      return *h_.vp * x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator*(double x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      return h_.i * x;
    case Rational:
      return h_.r->toDouble() * x;
    case Float:
      return h_.d * x;
    case Real:
      return h_.x->toDouble() * x;
    case Binary:
    case String:
    case StringPointer:
      NERROR("invalid operands");
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mul");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v * x;
    case HeadSequence:
      return *h_.hs->s * x;
    case HeadMap:
      return *h_.hm->h * x;
    case SequenceMap:
      return *h_.sm->s * x;
    case HeadSequenceMap:
      return *h_.hsm->s * x;
    case Reference:
      return *h_.ref->v * x;
    case Pointer:
      return *h_.vp * x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar& nvar::operator/=(int64_t x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      h_.i /= x;
      return *this;
    case Rational:
      *h_.r /= x;
      return *this;
    case Float:
      h_.d /= x;
      return *this;
    case Real:
      *h_.x /= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Div");
      f->v.push_back(new nvar(std::move(*this)));
      f->v.push_back(x);
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v /= x;
      return *this;
    case HeadSequence:
      *h_.hs->s /= x;
      return *this;
    case HeadMap:
      *h_.hm->h /= x;
      return *this;
    case SequenceMap:
      *h_.sm->s /= x;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s /= x;
      return *this;
    case Reference:
      *h_.ref->v /= x;
      return *this;
    case Pointer:
      *h_.vp /= x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
}

nvar& nvar::operator/=(double x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      t_ = Float;
      h_.d = h_.i;
      h_.d /= x;
      return *this;
    case Rational:{
      double d = h_.r->toDouble();
      delete h_.r;
      t_ = Float;
      h_.d = d + x;
      return *this;
    }
    case Float:
      h_.d /= x;
      return *this;
    case Real:{
      double d = h_.x->toDouble();
      d /= x;
      delete h_.x;
      h_.d = d;
      t_ = Float;
      break;
    }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Div");
      f->v.push_back(new nvar(std::move(*this)));
      f->v.push_back(x);
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v /= x;
      return *this;
    case HeadSequence:
      *h_.hs->s /= x;
      return *this;
    case HeadMap:
      *h_.hm->h /= x;
      return *this;
    case SequenceMap:
      *h_.sm->s /= x;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s /= x;
      return *this;
    case Reference:
      *h_.ref->v /= x;
      return *this;
    case Pointer:
      *h_.vp /= x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
  
  return *this;
}

nvar& nvar::operator/=(const nvar& x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          h_.i /= x.h_.i;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(h_.i);
          *h_.r /= *x.h_.r;
          if(h_.r->denominator() == 1){
            t_ = Integer;
            nrat* r = h_.r;
            h_.i = r->numerator();
            delete r;
            return *this;
          }
          return *this;
        case Float:
          t_ = Float;
          h_.d = h_.i;
          h_.d /= x.h_.d;
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(h_.i);
          *h_.x /= *x.h_.x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Div");
          f->v.push_back(h_.i);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.i);
          *v /= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
          *v /= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this /= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
          *v /= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
          *v /= *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this /= *x.h_.ref->v;
        case Pointer:
          return *this /= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          *h_.r /= x.h_.i;
          return *this;
        case Rational:
          *h_.r /= *x.h_.r;
          if(h_.r->denominator() == 1){
            nrat* r = h_.r;
            h_.i = h_.r->numerator();
            t_ = Integer;
            delete r;
          }
          return *this;
        case Float:{
          double d = h_.r->toDouble();
          delete h_.r;
          t_ = Float;
          h_.d = d + x.h_.d;
          return *this;
        }
        case Real:{
          nreal* y = new nreal(*h_.r);
          delete h_.r;
          t_ = Real;
          *y /= *x.h_.x;
          h_.x = y;
          return *this;
        }
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Div");
          f->v.push_back(*h_.r);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), *h_.r);
          *v /= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), *h_.r);
          *v /= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this /= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), *h_.r);
          *v /= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), *h_.r);
          *v /= *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this /= *x.h_.ref->v;
        case Pointer:
          return *this /= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          h_.d /= x.h_.i;
          return *this;
        case Rational:
          h_.d /= x.h_.r->toDouble();
          return *this;
        case Float:
          h_.d /= x.h_.d;
          return *this;
        case Real:
          h_.d /= x.h_.x->toDouble();
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Div");
          f->v.push_back(h_.d);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.d);
          *v /= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.d);
          *v /= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this /= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.d);
          *v /= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.d);
          *v /= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this /= *x.h_.ref->v;
        case Pointer:
          return *this /= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          *h_.x /= x.h_.i;
          return *this;
        case Rational:
          *h_.x /= *x.h_.r;
          return *this;
        case Float:{
          double d = h_.x->toDouble();
          d /= x.h_.d;
          delete h_.x;
          h_.d = d;
          t_ = Float;
        }
        case Real:
          *h_.x /= *x.h_.x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Div");
          f->v.push_back(*h_.x);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), *h_.x);
          *v /= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), *h_.x);
          *v /= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this /= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), *h_.x);
          *v /= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), *h_.x);
          *v /= *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this /= *x.h_.ref->v;
        case Pointer:
          return *this /= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Div");
      f->v.emplace_back(new nvar(*this, Copy));
      f->v.emplace_back(nvar(x, Copy));
      
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.v /= x;
          return *this;
        case Vector:
          *h_.v /= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.v /= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this /= *x.h_.hm->h;
        case SequenceMap:
          *h_.v /= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.v /= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this /= *x.h_.ref->v;
        case Pointer:
          return *this /= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequence:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.hs->s /= x;
          return *this;
        case Vector:
          *h_.hs->s /= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.hs->s /= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this /= *x.h_.hm->h;
        case SequenceMap:
          *h_.hs->s /= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.hs->s /= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this /= *x.h_.ref->v;
        case Pointer:
          return *this /= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadMap:
      return *h_.hm->h /= x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.sm->s /= x;
          return *this;
        case Vector:
          *h_.sm->s /= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.sm->s /= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this /= *x.h_.hm->h;
        case SequenceMap:
          *h_.sm->s /= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.sm->s /= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this /= *x.h_.ref->v;
        case Pointer:
          return *this /= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.hsm->s /= x;
          return *this;
        case Vector:
          *h_.hsm->s /= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.hsm->s /= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this /= *x.h_.hm->h;
        case SequenceMap:
          *h_.hsm->s /= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s /= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this /= *x.h_.ref->v;
        case Pointer:
          return *this /= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Reference:
      *h_.ref->v /= x;
      return *this;
    case Pointer:
      *h_.vp /= x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
}


nvar nvar::operator/(int64_t x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      return h_.i / x;
    case Rational:{
      Head h;
      h.r = new nrat(*h_.r);
      *h.r /= x;
      return nvar(Rational, h);
    }
    case Float:
      return h_.d / x;
    case Real:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x /= x;
      return nvar(Real, h);
    }
    case Binary:
    case String:
    case StringPointer:
      NERROR("invalid operands");
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Div");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v / x;
    case HeadSequence:
      return *h_.hs->s / x;
    case HeadMap:
      return *h_.hm->h / x;
    case SequenceMap:
      return *h_.sm->s / x;
    case HeadSequenceMap:
      return *h_.hsm->s / x;
    case Reference:
      return *h_.ref->v / x;
    case Pointer:
      return *h_.vp / x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator/(double x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      return h_.i / x;
    case Rational:
      return h_.r->toDouble() / x;
    case Float:
      return h_.d / x;
    case Real:
      return h_.x->toDouble() / x;
    case Binary:
    case String:
    case StringPointer:
      NERROR("invalid operands");
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Div");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v / x;
    case HeadSequence:
      return *h_.hs->s / x;
    case HeadMap:
      return *h_.hm->h / x;
    case SequenceMap:
      return *h_.sm->s / x;
    case HeadSequenceMap:
      return *h_.hsm->s / x;
    case Reference:
      return *h_.ref->v / x;
    case Pointer:
      return *h_.vp / x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator/(const nvar& x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return h_.i / x.h_.i;
        case Rational:{
          Head h;
          h.r = new nrat(h_.i);
          *h.r /= *x.h_.r;
          if(h.r->denominator() == 1){
            nrat* r = h.r;
            h.i = r->numerator();
            delete r;
            return nvar(Integer, h);
          }
          return nvar(Rational, h);
        }
        case Float:
          return h_.i / x.h_.d;
        case Real:{
          Head h;
          h.x = new nreal(h_.i);
          *h.x /= *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Div");
          h.f->v.push_back(h_.i);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.i);
          *v /= *x.h_.v;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
          *v /= *x.h_.hs->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this / *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
          *v /= *x.h_.sm->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
          *v /= *x.h_.hsm->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case Reference:
          return *this / *x.h_.ref->v;
        case Pointer:
          return *this / *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          Head h;
          h.r = new nrat(*h_.r);
          *h.r /= x.h_.i;
          return nvar(Rational, h);
        case Rational:{
          Head h;
          h.r = new nrat(*h_.r);
          *h.r /= *x.h_.r;
          if(h.r->denominator() == 1){
            nrat* r = h.r;
            h.i = h_.r->numerator();
            delete r;
            return nvar(Integer, h);
          }
          return nvar(Rational, h);
        }
        case Float:
          return h_.r->toDouble() / x.h_.d;
        case Real:{
          Head h;
          h.x = new nreal(*h_.r);
          *h.x /= *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Div");
          h.f->v.push_back(*h_.r);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.r);
          *h.v /= *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.r);
          *h.v /= *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this / *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.r);
          *h.v /= *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
          *h.v /= *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this / *x.h_.ref->v;
        case Pointer:
          return *this / *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return h_.d / x.h_.i;
        case Rational:
          return h_.d / x.h_.r->toDouble();
        case Float:
          return h_.d / x.h_.d;
        case Real:
          return h_.d / x.h_.x->toDouble();
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Div");
          h.f->v.push_back(h_.d);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), h_.d);
          *h.v /= *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), h_.d);
          *h.v /= *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this / *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), h_.d);
          *h.v /= *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), h_.d);
          *h.v /= *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this / *x.h_.ref->v;
        case Pointer:
          return *this / *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          Head h;
          h.x = new nreal(*h_.x);
          *h.x /= x.h_.i;
          return nvar(Real, h);
        case Rational:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x /= *x.h_.r;
          return nvar(Real, h);
        }
        case Float:
          return h_.x->toDouble() / x.h_.d;
        case Real:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x /= *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Div");
          h.f->v.push_back(*h_.x);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.x);
          *h.v /= *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.x);
          *h.v /= *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this / *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.x);
          *h.v /= *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
          *h.v /= *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this / *x.h_.ref->v;
        case Pointer:
          return *this / *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("Div");
      h.f->v.emplace_back(new nvar(*this, Copy));
      h.f->v.emplace_back(nvar(x, Copy));
      
      return new nvar(Function, h);
    }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.v / x;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Div");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          Head h;
          h.f = f;
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v / *x.h_.v;
        case HeadSequence:
          return *h_.v / *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this / *x.h_.hm->h;
        case SequenceMap:
          return *h_.v / *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.v / *x.h_.hsm->s;
        case Reference:
          return *this / *x.h_.ref->v;
        case Pointer:
          return *this / *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequence:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hs->s / x;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Div");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          Head h;
          h.f = f;
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hs->s / *x.h_.v;
        case HeadSequence:
          return *h_.hs->s / *x.h_.hs->s;
        case HeadMap:
          return *this / *x.h_.hm->h;
        case SequenceMap:
          return *h_.hs->s / *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hs->s / *x.h_.hsm->s;
        case Reference:
          return *this / *x.h_.ref->v;
        case Pointer:
          return *this / *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadMap:
      return *h_.hm->h / x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.sm->s / x;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Div");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          Head h;
          h.f = f;
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.sm->s / *x.h_.v;
        case HeadSequence:
          return *h_.sm->s / *x.h_.hs->s;
        case HeadMap:
          return *this / *x.h_.hm->h;
        case SequenceMap:
          return *h_.sm->s / *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.sm->s / *x.h_.hsm->s;
        case Reference:
          return *this / *x.h_.ref->v;
        case Pointer:
          return *this / *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hsm->s / x;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Div");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          Head h;
          h.f = f;
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hsm->s / *x.h_.v;
        case HeadSequence:
          return *h_.hsm->s / *x.h_.hs->s;
        case HeadMap:
          return *this / *x.h_.hm->h;
        case SequenceMap:
          return *h_.hsm->s / *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hsm->s / *x.h_.hsm->s;
        case Reference:
          return *this / *x.h_.ref->v;
        case Pointer:
          return *this / *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Reference:
      return *h_.ref->v / x;
    case Pointer:
      return *h_.vp / x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar& nvar::operator%=(int64_t x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      h_.i %= x;
      return *this;
    case Rational:
      *h_.r = *h_.r - *h_.r / x;
      return *this;
    case Float:
      h_.d = fmod(h_.d, static_cast<double>(x));
      return *this;
    case Real:
      *h_.x %= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mod");
      f->v.push_back(new nvar(std::move(*this)));
      f->v.push_back(x);
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v %= x;
      return *this;
    case HeadSequence:
      *h_.hs->s %= x;
      return *this;
    case HeadMap:
      *h_.hm->h %= x;
      return *this;
    case SequenceMap:
      *h_.sm->s %= x;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s %= x;
      return *this;
    case Reference:
      *h_.ref->v %= x;
      return *this;
    case Pointer:
      *h_.vp %= x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
}

nvar& nvar::operator%=(double x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      t_ = Float;
      h_.d = fmod(h_.i, x);
      return *this;
    case Rational:{
      double d = h_.r->toDouble();
      delete h_.r;
      t_ = Float;
      h_.d = fmod(d, x);
      return *this;
    }
    case Float:
      h_.d = fmod(h_.d, x);
      return *this;
    case Real:{
      double d = h_.x->toDouble();
      delete h_.x;
      h_.d = fmod(d, x);
      t_ = Float;
      break;
    }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mod");
      f->v.push_back(new nvar(std::move(*this)));
      f->v.push_back(x);
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v %= x;
      return *this;
    case HeadSequence:
      *h_.hs->s %= x;
      return *this;
    case HeadMap:
      *h_.hm->h %= x;
      return *this;
    case SequenceMap:
      *h_.sm->s %= x;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s %= x;
      return *this;
    case Reference:
      *h_.ref->v %= x;
      return *this;
    case Pointer:
      *h_.vp %= x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
  
  return *this;
}

nvar& nvar::operator%=(const nvar& x){
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          h_.i %= x.h_.i;
          return *this;
        case Rational:
          t_ = Rational;
          h_.r = new nrat(h_.i);
          *h_.r = *h_.r - *h_.r / *x.h_.r;
          if(h_.r->denominator() == 1){
            t_ = Integer;
            nrat* r = h_.r;
            h_.i = r->numerator();
            delete r;
            return *this;
          }
        case Float:
          t_ = Float;
          h_.d = fmod(h_.i, x.h_.d);
          return *this;
        case Real:
          t_ = Real;
          h_.x = new nreal(h_.i);
          *h_.x %= *x.h_.x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mod");
          f->v.push_back(h_.i);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.i);
          *v %= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
          *v %= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this %= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
          *v %= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
          *v %= *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this %= *x.h_.ref->v;
        case Pointer:
          return *this %= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          *h_.r = *h_.r - *h_.r / x.h_.i;
          return *this;
        case Rational:
          *h_.r = *h_.r - *h_.r / *x.h_.r;
          if(h_.r->denominator() == 1){
            nrat* r = h_.r;
            h_.i = h_.r->numerator();
            t_ = Integer;
            delete r;
          }
          return *this;
        case Float:{
          double d = h_.r->toDouble();
          delete h_.r;
          t_ = Float;
          h_.d = fmod(d, x.h_.d);
          return *this;
        }
        case Real:{
          nreal* y = new nreal(*h_.r);
          delete h_.r;
          t_ = Real;
          *y %= *x.h_.x;
          h_.x = y;
          return *this;
        }
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mod");
          f->v.push_back(*h_.r);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), *h_.r);
          *v %= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), *h_.r);
          *v %= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this %= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), *h_.r);
          *v %= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), *h_.r);
          *v %= *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this %= *x.h_.ref->v;
        case Pointer:
          return *this %= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          h_.d = fmod(h_.d, static_cast<double>(x.h_.i));
          return *this;
        case Rational:
          h_.d = fmod(h_.d, x.h_.r->toDouble());
          return *this;
        case Float:
          h_.d = fmod(h_.d, x.h_.d);
          return *this;
        case Real:
          h_.d = fmod(h_.d, x.h_.x->toDouble());
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mod");
          f->v.push_back(h_.d);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.d);
          *v %= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.d);
          *v %= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this %= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.d);
          *v %= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.d);
          *v %= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this %= *x.h_.ref->v;
        case Pointer:
          return *this %= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          *h_.x %= x.h_.i;
          return *this;
        case Rational:
          *h_.x %= *x.h_.r;
          return *this;
        case Float:{
          double d = h_.x->toDouble();
          delete h_.x;
          h_.d = fmod(d, x.h_.d);
          t_ = Float;
          break;
        }
        case Real:
          *h_.x %= *x.h_.x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mod");
          f->v.push_back(*h_.x);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), *h_.x);
          *v %= *x.h_.v;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), *h_.x);
          *v %= *x.h_.hs->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadMap:
          return *this %= *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), *h_.x);
          *v %= *x.h_.sm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), *h_.x);
          *v %= *x.h_.hsm->s;
          h_.v = v;
          t_ = Vector;
          return *this;
        }
        case Reference:
          return *this %= *x.h_.ref->v;
        case Pointer:
          return *this %= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mod");
      f->v.emplace_back(new nvar(*this), Copy);
      f->v.emplace_back(nvar(x, Copy));
      
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.v %= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mod");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v %= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.v %= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this %= *x.h_.hm->h;
        case SequenceMap:
          *h_.v %= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.v %= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this %= *x.h_.ref->v;
        case Pointer:
          return *this %= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequence:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.hs->s %= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mod");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.hs->s %= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.hs->s %= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this %= *x.h_.hm->h;
        case SequenceMap:
          *h_.hs->s %= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.hs->s %= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this %= *x.h_.ref->v;
        case Pointer:
          return *this %= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadMap:
      return *h_.hm->h %= x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.sm->s %= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mod");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.sm->s %= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.sm->s %= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this %= *x.h_.hm->h;
        case SequenceMap:
          *h_.sm->s %= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.sm->s %= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this %= *x.h_.ref->v;
        case Pointer:
          return *this %= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          *h_.hsm->s %= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mod");
          f->v.emplace_back(*this);
          f->v.push_back(new nvar(x, Copy));
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.hsm->s %= *x.h_.v;
          return *this;
        case HeadSequence:
          *h_.hsm->s %= *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this %= *x.h_.hm->h;
        case SequenceMap:
          *h_.hsm->s %= *x.h_.sm->s;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s %= *x.h_.hsm->s;
          return *this;
        case Reference:
          return *this %= *x.h_.ref->v;
        case Pointer:
          return *this %= *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Reference:
      *h_.ref->v %= x;
      return *this;
    case Pointer:
      *h_.vp %= x;
      return *this;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator%(int64_t x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      return h_.i % x;
    case Rational:{
      Head h;
      h.r = new nrat(*h_.r);
      *h.r = *h.r - *h.r / x;
      return nvar(Rational, h);
    }
    case Float:
      return fmod(h_.d, static_cast<double>(x));
    case Real:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x %= x;
      return nvar(Real, h);
    }
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("Mod");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v % x;
    case HeadSequence:
      return *h_.hs->s % x;
    case HeadMap:
      return *h_.hm->h % x;
    case SequenceMap:
      return *h_.sm->s % x;
    case HeadSequenceMap:
      return *h_.hsm->s % x;
    case Reference:
      return *h_.ref->v % x;
    case Pointer:
      return *h_.vp % x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator%(double x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      return fmod(h_.i, x);
    case Rational:
      return fmod(h_.r->toDouble(), x);
    case Float:
      return fmod(h_.d, x);
    case Real:
      return fmod(h_.x->toDouble(), x);
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("Mod");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v % x;
    case HeadSequence:
      return *h_.hs->s % x;
    case HeadMap:
      return *h_.hm->h % x;
    case SequenceMap:
      return *h_.sm->s % x;
    case HeadSequenceMap:
      return *h_.hsm->s % x;
    case Reference:
      return *h_.ref->v % x;
    case Pointer:
      return *h_.vp % x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator%(const nvar& x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return h_.i % x.h_.i;
        case Rational:{
          Head h;
          h.r = new nrat(h_.i);
          *h.r = *h.r - *h.r / *x.h_.r;
          if(h.r->denominator() == 1){
            nrat* r = h.r;
            h.i = r->numerator();
            delete r;
            return nvar(Integer, h);
          }
          return nvar(Rational, h);
        }
        case Float:
          return fmod(h_.i, x.h_.d);
        case Real:{
          Head h;
          h.x = new nreal(h_.i);
          *h.x %= *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mod");
          h.f->v.push_back(h_.i);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.i);
          *v %= *x.h_.v;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
          *v %= *x.h_.hs->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this % *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
          *v %= *x.h_.sm->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
          *v %= *x.h_.hsm->s;
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case Reference:
          return *this % *x.h_.ref->v;
        case Pointer:
          return *this % *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:{
          Head h;
          h.r = new nrat(*h_.r);
          *h.r = *h.r - *h.r / x.h_.i;
          return nvar(Rational, h);
        }
        case Rational:{
          Head h;
          h.r = new nrat(*h_.r);
          *h.r = *h.r - *h.r / *x.h_.r;
          if(h.r->denominator() == 1){
            nrat* r = h.r;
            h.i = h_.r->numerator();
            delete r;
            return nvar(Integer, h);
          }
          return nvar(Rational, h);
        }
        case Float:
          return fmod(h_.r->toDouble(), x.h_.d);
        case Real:{
          Head h;
          h.x = new nreal(*h_.r);
          *h.x %= *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mod");
          h.f->v.push_back(*h_.r);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.r);
          *h.v %= *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.r);
          *h.v %= *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this % *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.r);
          *h.v %= *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
          *h.v %= *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this % *x.h_.ref->v;
        case Pointer:
          return *this % *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return fmod(h_.d, static_cast<double>(x.h_.i));
        case Rational:
          return fmod(h_.d, x.h_.r->toDouble());
        case Float:
          return fmod(h_.d, x.h_.d);
        case Real:
          return fmod(h_.d, x.h_.x->toDouble());
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mod");
          h.f->v.push_back(h_.d);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), h_.d);
          *h.v %= *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), h_.d);
          *h.v %= *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this % *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), h_.d);
          *h.v %= *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), h_.d);
          *h.v %= *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this % *x.h_.ref->v;
        case Pointer:
          return *this % *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          Head h;
          h.x = new nreal(*h_.x);
          *h.x %= x.h_.i;
          return nvar(Real, h);
        case Rational:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x %= *x.h_.r;
          return nvar(Real, h);
        }
        case Float:
          return fmod(h_.x->toDouble(), x.h_.d);
        case Real:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x %= *x.h_.x;
          return nvar(Real, h);
        }
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mod");
          h.f->v.push_back(*h_.x);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.x);
          *h.v %= *x.h_.v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.x);
          *h.v %= *x.h_.hs->s;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this % *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.x);
          *h.v %= *x.h_.sm->s;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
          *h.v %= *x.h_.hsm->s;
          return nvar(Vector, h);
        }
        case Reference:
          return *this % *x.h_.ref->v;
        case Pointer:
          return *this % *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("Mod");
      h.f->v.emplace_back(new nvar(*this, Copy));
      h.f->v.emplace_back(nvar(x, Copy));
      
      return new nvar(Function, h);
    }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.v % x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mod");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v % *x.h_.v;
        case HeadSequence:
          return *h_.v % *x.h_.hs->s;
        case HeadMap:
          return *this % *x.h_.hm->h;
        case SequenceMap:
          return *h_.v % *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.v % *x.h_.hsm->s;
        case Reference:
          return *this % *x.h_.ref->v;
        case Pointer:
          return *this % *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequence:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hs->s % x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mod");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hs->s % *x.h_.v;
        case HeadSequence:
          return *h_.hs->s % *x.h_.hs->s;
        case HeadMap:
          return *this % *x.h_.hm->h;
        case SequenceMap:
          return *h_.hs->s % *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hs->s % *x.h_.hsm->s;
        case Reference:
          return *this % *x.h_.ref->v;
        case Pointer:
          return *this % *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadMap:
      return *h_.hm->h % x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.sm->s % x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mod");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.sm->s % *x.h_.v;
        case HeadSequence:
          return *h_.sm->s % *x.h_.hs->s;
        case HeadMap:
          return *this % *x.h_.hm->h;
        case SequenceMap:
          return *h_.sm->s % *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.sm->s % *x.h_.hsm->s;
        case Reference:
          return *this % *x.h_.ref->v;
        case Pointer:
          return *this % *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hsm->s % x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mod");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hsm->s % *x.h_.v;
        case HeadSequence:
          return *h_.hsm->s % *x.h_.hs->s;
        case HeadMap:
          return *this % *x.h_.hm->h;
        case SequenceMap:
          return *h_.hsm->s % *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hsm->s % *x.h_.hsm->s;
        case Reference:
          return *this % *x.h_.ref->v;
        case Pointer:
          return *this % *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Reference:
      return *h_.ref->v % x;
    case Pointer:
      return *h_.vp % x;
    default:
      NERROR("left operand is invalid");
  }
}

bool nvar::less(const nvar& x) const{
  switch(t_){
    case None:
      switch(x.t_){
        case None:
          return false;
        case HeadSequence:
          return less(*x.h_.hs->h);
        case HeadMap:
          return less(*x.h_.hm->h);
        case HeadSequenceMap:
          return less(*x.h_.hsm->h);
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case Undefined:
      switch(x.t_){
        case None:
        case Undefined:
          return false;
        case HeadSequence:
          return less(*x.h_.hs->h);
        case HeadMap:
          return less(*x.h_.hm->h);
        case HeadSequenceMap:
          return less(*x.h_.hsm->h);
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case False:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return false;
        case HeadSequence:
          return less(*x.h_.hs->h);
        case HeadMap:
          return less(*x.h_.hm->h);
        case HeadSequenceMap:
          return less(*x.h_.hsm->h);
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case True:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
          return false;
        case HeadSequence:
          return less(*x.h_.hs->h);
        case HeadMap:
          return less(*x.h_.hm->h);
        case HeadSequenceMap:
          return less(*x.h_.hsm->h);
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
          return false;
        case Integer:
          return h_.i < x.h_.i;
        case Rational:
          return h_.i < *x.h_.r;
        case Float:
          return h_.i < x.h_.d;
        case Real:
          return h_.i < x.h_.x->toLong();
        case HeadSequence:
          return less(*x.h_.hs->h);
        case HeadMap:
          return less(*x.h_.hm->h);
        case HeadSequenceMap:
          return less(*x.h_.hsm->h);
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
          return false;
        case Integer:
          return *h_.r < x.h_.i;
        case Rational:
          return *h_.r < *x.h_.r;
        case Float:
          return *h_.r < x.h_.d;
        case Real:
          return *h_.r < *x.h_.x;
        case HeadSequence:
          return less(*x.h_.hs->h);
        case HeadMap:
          return less(*x.h_.hm->h);
        case HeadSequenceMap:
          return less(*x.h_.hsm->h);
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
          return false;
        case Integer:
          return h_.d < x.h_.i;
        case Rational:
          return h_.d < *x.h_.r;
        case Float:
          return h_.d < x.h_.d;
        case Real:
          return h_.d < x.h_.x->toDouble();
        case HeadSequence:
          return less(*x.h_.hs->h);
        case HeadMap:
          return less(*x.h_.hm->h);
        case HeadSequenceMap:
          return less(*x.h_.hsm->h);
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
          return false;
        case Integer:
          return *h_.x < x.h_.i;
        case Rational:
          return *h_.x < nreal(*x.h_.r);
        case Float:
          return *h_.x < x.h_.d;
        case Real:
          return *h_.x < *x.h_.x;
        case HeadSequence:
          return less(*x.h_.hs->h);
        case HeadMap:
          return less(*x.h_.hm->h);
        case HeadSequenceMap:
          return less(*x.h_.hsm->h);
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case StringPointer:
    case String:
    case Binary:
    case Symbol:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
          return false;
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
          return *h_.s < *x.h_.s;
        case HeadSequence:
          return less(*x.h_.hs->h);
        case HeadMap:
          return less(*x.h_.hm->h);
        case HeadSequenceMap:
          return less(*x.h_.hsm->h);
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case RawPointer:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
          return false;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.p < x.h_.o;
        case RawPointer:
          return h_.p < x.h_.p;
        case HeadSequence:
          return less(*x.h_.hs->h);
        case HeadMap:
          return less(*x.h_.hm->h);
        case HeadSequenceMap:
          return less(*x.h_.hsm->h);
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return false;
      }
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
          return false;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.o < x.h_.o;
        case RawPointer:
          return h_.o < x.h_.p;
        case HeadSequence:
          return less(*x.h_.hs->h);
        case HeadMap:
          return less(*x.h_.hm->h);
        case HeadSequenceMap:
          return less(*x.h_.hsm->h);
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
          return false;
        case Vector:{
          size_t s1 = h_.v->size();
          size_t s2 = x.h_.v->size();
          
          if(s1 > s2){
            return true;
          }
          else if(s2 > s1){
            return false;
          }
          
          for(size_t i = 0; i < s1; ++i){
            if((*h_.v)[i].less((*x.h_.v)[i])){
              return true;
            }
            else if((*x.h_.v)[i].less((*h_.v)[i])){
              return false;
            }
          }
          
          return false;
        }
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case List:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
          return false;
        case List:{
          size_t s1 = h_.l->size();
          size_t s2 = x.h_.l->size();
          
          if(s1 > s2){
            return true;
          }
          else if(s2 > s1){
            return false;
          }
          
          for(size_t i = 0; i < s1; ++i){
            if((*h_.l)[i].less((*x.h_.l)[i])){
              return true;
            }
            else if((*x.h_.v)[i].less((*h_.v)[i])){
              return false;
            }
          }
          
          return false;
        }
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case Function:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
          return false;
        case Function:{
          int y = h_.f->v.size() - x.h_.f->v.size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return h_.f->f < x.h_.f->f;
        }
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case HeadSequence:
      return h_.hs->h->less(x);
    case Map:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case Function:
        case HeadSequence:
          return false;
        case Map:{
          int y = h_.m->size() - x.h_.m->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.m < *x.h_.m;
        }
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case Multimap:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case Function:
        case HeadSequence:
        case Map:
          return false;
        case Multimap:{
          int y = h_.mm->size() - x.h_.mm->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.mm < *x.h_.mm;
        }
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case HeadMap:
      return h_.hm->h->less(x);
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case Function:
        case HeadSequence:
        case Map:
        case Multimap:
        case HeadMap:
          return false;
        case SequenceMap:{
          int y = h_.sm->s->size() - x.h_.sm->s->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.sm->s < *x.h_.sm->s;
        }
        case Reference:
          return less(*x.h_.ref->v);
        case Pointer:
          return less(*x.h_.vp);
        default:
          return true;
      }
    case HeadSequenceMap:
      return h_.hsm->h->less(x);
    case Reference:
      return h_.ref->v->less(x);
    case Pointer:
      return h_.vp->less(x);
    default:
      return false;
  }
}

bool nvar::greater(const nvar& x) const{
  switch(t_){
    case None:
      switch(x.t_){
        case HeadSequence:
          return greater(*x.h_.hs->h);
        case HeadMap:
          return greater(*x.h_.hm->h);
        case HeadSequenceMap:
          return greater(*x.h_.hsm->h);
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case Undefined:
      switch(x.t_){
        case None:
          return true;
        case HeadSequence:
          return greater(*x.h_.hs->h);
        case HeadMap:
          return greater(*x.h_.hm->h);
        case HeadSequenceMap:
          return greater(*x.h_.hsm->h);
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case False:
      switch(x.t_){
        case None:
        case Undefined:
          return true;
        case HeadSequence:
          return greater(*x.h_.hs->h);
        case HeadMap:
          return greater(*x.h_.hm->h);
        case HeadSequenceMap:
          return greater(*x.h_.hsm->h);
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case True:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return true;
        case HeadSequence:
          return greater(*x.h_.hs->h);
        case HeadMap:
          return greater(*x.h_.hm->h);
        case HeadSequenceMap:
          return greater(*x.h_.hsm->h);
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
          return true;
        case Integer:
          return h_.i > x.h_.i;
        case Rational:
          return h_.i > *x.h_.r;
        case Float:
          return h_.i > x.h_.d;
        case Real:
          return h_.i > x.h_.x->toLong();
        case HeadSequence:
          return greater(*x.h_.hs->h);
        case HeadMap:
          return greater(*x.h_.hm->h);
        case HeadSequenceMap:
          return greater(*x.h_.hsm->h);
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
          return true;
        case Integer:
          return *h_.r > x.h_.i;
        case Rational:
          return *h_.r > *x.h_.r;
        case Float:
          return *h_.r > x.h_.d;
        case Real:
          return *h_.r > *x.h_.x;
        case HeadSequence:
          return greater(*x.h_.hs->h);
        case HeadMap:
          return greater(*x.h_.hm->h);
        case HeadSequenceMap:
          return greater(*x.h_.hsm->h);
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
          return true;
        case Integer:
          return h_.d > x.h_.i;
        case Rational:
          return h_.d > *x.h_.r;
        case Float:
          return h_.d > x.h_.d;
        case Real:
          return h_.d > x.h_.x->toDouble();
        case HeadSequence:
          return greater(*x.h_.hs->h);
        case HeadMap:
          return greater(*x.h_.hm->h);
        case HeadSequenceMap:
          return greater(*x.h_.hsm->h);
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
          return true;
        case Integer:
          return *h_.x > x.h_.i;
        case Rational:
          return *h_.x > nreal(*x.h_.r);
        case Float:
          return *h_.x > x.h_.d;
        case Real:
          return *h_.x > *x.h_.x;
        case HeadSequence:
          return greater(*x.h_.hs->h);
        case HeadMap:
          return greater(*x.h_.hm->h);
        case HeadSequenceMap:
          return greater(*x.h_.hsm->h);
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case StringPointer:
    case String:
    case Binary:
    case Symbol:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
          return true;
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
          return *h_.s > *x.h_.s;
        case HeadSequence:
          return greater(*x.h_.hs->h);
        case HeadMap:
          return greater(*x.h_.hm->h);
        case HeadSequenceMap:
          return greater(*x.h_.hsm->h);
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case RawPointer:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
          return true;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.p > x.h_.o;
        case RawPointer:
          return h_.p > x.h_.p;
        case HeadSequence:
          return greater(*x.h_.hs->h);
        case HeadMap:
          return greater(*x.h_.hm->h);
        case HeadSequenceMap:
          return greater(*x.h_.hsm->h);
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
          return true;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.o > x.h_.o;
        case RawPointer:
          return h_.o > x.h_.p;
        case HeadSequence:
          return greater(*x.h_.hs->h);
        case HeadMap:
          return greater(*x.h_.hm->h);
        case HeadSequenceMap:
          return greater(*x.h_.hsm->h);
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
          return true;
        case Vector:{
          size_t s1 = h_.v->size();
          size_t s2 = x.h_.v->size();
          
          if(s1 > s2){
            return true;
          }
          else if(s2 > s1){
            return false;
          }
          
          for(size_t i = 0; i < s1; ++i){
            if((*h_.v)[i].greater((*x.h_.v)[i])){
              return true;
            }
            else if((*x.h_.v)[i].greater((*h_.v)[i])){
              return false;
            }
          }
          
          return false;
        }
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case List:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
          return true;
        case List:{
          size_t s1 = h_.l->size();
          size_t s2 = x.h_.l->size();
          
          if(s1 > s2){
            return true;
          }
          else if(s2 > s1){
            return false;
          }
          
          for(size_t i = 0; i < s1; ++i){
            if((*h_.l)[i].greater((*x.h_.l)[i])){
              return true;
            }
            else if((*x.h_.v)[i].greater((*h_.v)[i])){
              return false;
            }
          }
          
          return false;
        }
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case Function:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
          return true;
        case Function:{
          int y = h_.f->v.size() - x.h_.f->v.size();
          
          if(y > 0){
            return true;
          }
          else if(y < 0){
            return false;
          }
          
          return h_.f->f > x.h_.f->f;
        }
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case HeadSequence:
      return h_.hs->h->greater(x);
    case Map:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case Function:
        case HeadSequence:
          return true;
        case Map:{
          int y = h_.m->size() - x.h_.m->size();
          
          if(y > 0){
            return true;
          }
          else if(y < 0){
            return false;
          }
          
          return *h_.m > *x.h_.m;
        }
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case Multimap:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case Function:
        case HeadSequence:
        case Map:
          return true;
        case Multimap:{
          int y = h_.mm->size() - x.h_.mm->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.mm > *x.h_.mm;
        }
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case HeadMap:
      return h_.hm->h->greater(x);
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case Function:
        case HeadSequence:
        case Map:
        case Multimap:
        case HeadMap:
          return true;
        case SequenceMap:{
          int y = h_.sm->s->size() - x.h_.sm->s->size();
          
          if(y > 0){
            return true;
          }
          else if(y < 0){
            return false;
          }
          
          return *h_.sm->s > *x.h_.sm->s;
        }
        case Reference:
          return greater(*x.h_.ref->v);
        case Pointer:
          return greater(*x.h_.vp);
        default:
          return false;
      }
    case HeadSequenceMap:
      return h_.hsm->h->greater(x);
    case Reference:
      return h_.ref->v->greater(x);
    case Pointer:
      return h_.vp->greater(x);
    default:
      return false;
  }
}

bool nvar::equal(const nvar& x) const{
  switch(t_){
    case None:
      switch(x.t_){
        case None:
          return true;
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case Undefined:
      switch(x.t_){
        case Undefined:
          return true;
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case False:
      switch(x.t_){
        case False:
          return true;
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case True:
      switch(x.t_){
        case True:
          return true;
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case Integer:
      switch(x.t_){
        case Integer:
          return h_.i == x.h_.i;
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case Rational:
      switch(x.t_){
        case Rational:
          return *h_.r == *x.h_.r;
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case Float:
      switch(x.t_){
        case Float:
          return h_.d == x.h_.d;
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case Real:
      switch(x.t_){
        case Real:
          return *h_.x == *x.h_.x;
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case StringPointer:
    case String:
    case Binary:
      switch(x.t_){
        case Binary:
        case String:
        case StringPointer:
          return *h_.s == *x.h_.s;
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case Symbol:
      switch(x.t_){
        case Symbol:
          return *h_.s == *x.h_.s;
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case RawPointer:
      switch(x.t_){
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.p == x.h_.o;
        case RawPointer:
          return h_.p == x.h_.p;
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      switch(x.t_){
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.o == x.h_.o;
        case RawPointer:
          return h_.o == x.h_.p;
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case Vector:
      switch(x.t_){
        case Vector:{
          size_t size = h_.v->size();
          
          if(size != x.h_.v->size()){
            return false;
          }
          
          for(size_t i = 0; i < size; ++i){
            if(!(*h_.v)[i].equal((*x.h_.v)[i])){
              return false;
            }
          }
          
          return true;
        }
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case List:
      switch(x.t_){
        case List:{
          size_t size = h_.l->size();
          
          if(size != x.h_.l->size()){
            return false;
          }
          
          for(size_t i = 0; i < size; ++i){
            if(!(*h_.l)[i].equal((*x.h_.l)[i])){
              return false;
            }
          }
          
          return true;
        }
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case Function:
      switch(x.t_){
        case Function:{
          size_t size = h_.f->v.size();
          
          if(size != h_.f->v.size()){
            return false;
          }

          for(size_t i = 0; i < size; ++i){
            if(!h_.f->v[i].equal((x.h_.f->v[i]))){
              return false;
            }
          }
          
          return true;
        }
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case HeadSequence:
      switch(x.t_){
        case HeadSequence:
          return h_.hs->h->equal(*x.h_.hs->h) &&
          h_.hs->s->equal(*x.h_.hs->s);
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case Map:
      switch(x.t_){
        case Map:{
          if(h_.m->size() != x.h_.m->size()){
            return false;
          }

          for(auto& itr : *h_.m){
            const nvar& k = itr.first;
            const nvar& v = itr.second;
            
            auto itr2 = x.h_.m->find(k);
            if(itr2 == x.h_.m->end()){
              return false;
            }
            
            const nvar& v2 = itr2->second;
            if(!v.equal(v2)){
              return false;
            }
          }
          
          return true;
        }
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case Multimap:
      switch(x.t_){
        case Multimap:{
          if(h_.mm->size() != x.h_.mm->size()){
            return false;
          }
          
          for(auto& itr : *h_.mm){
            const nvar& k = itr.first;
            const nvar& v = itr.second;
            
            auto itr2 = x.h_.mm->find(k);
            if(itr2 == x.h_.mm->end()){
              return false;
            }
            
            const nvar& v2 = itr2->second;
            if(!v.equal(v2)){
              return false;
            }
          }
          
          return true;
        }
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case HeadMap:
      switch(x.t_){
        case HeadMap:
          return h_.hm->h->equal(*x.h_.hm->h) &&
          h_.hm->m->equal(*x.h_.hm->m);
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case SequenceMap:
      switch(x.t_){
        case SequenceMap:
          return h_.sm->s->equal(*x.h_.sm->s) &&
          h_.sm->m->equal(*x.h_.sm->m);
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case HeadSequenceMap:
      switch(x.t_){
        case HeadSequenceMap:
          return h_.hsm->h->equal(*x.h_.hsm->h) &&
          h_.hsm->s->equal(*x.h_.hsm->s) &&
          h_.hsm->m->equal(*x.h_.hsm->m);
        case Reference:
          return equal(*x.h_.ref->v);
        case Pointer:
          return equal(*x.h_.vp);
        default:
          return false;
      }
    case Reference:
      return h_.ref->v->equal(x);
    case Pointer:
      return h_.vp->equal(x);
    default:
      return false;
  }
}

nvar nvar::operator<(int64_t x) const{
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
      return true;
    case Integer:
      return h_.i < x;
    case Rational:
      return *h_.r < x;
    case Float:
      return h_.d < x;
    case Real:
      return *h_.x < x;
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("LT");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case HeadSequence:
      return *h_.hs->h < x;
    case HeadMap:
      return *h_.hm->h < x;
    case HeadSequenceMap:
      return *h_.hsm->h < x;
    case Reference:
      return *h_.ref->v < x;
    case Pointer:
      return *h_.vp < x;
    default:
      return false;
  }
}

nvar nvar::operator<(double x) const{
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
      return true;
    case Integer:
      return h_.i < x;
    case Rational:
      return *h_.r < x;
    case Float:
      return h_.d < x;
    case Real:
      return *h_.x < x;
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("LT");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case HeadSequence:
      return *h_.hs->h < x;
    case HeadMap:
      return *h_.hm->h < x;
    case HeadSequenceMap:
      return *h_.hsm->h < x;
    case Reference:
      return *h_.ref->v < x;
    case Pointer:
      return *h_.vp < x;
    default:
      return false;
  }
}

nvar nvar::operator<(const nvar& x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case False:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
          return false;
        case HeadSequence:
          return *this < *x.h_.hs->h;
        case HeadMap:
          return *this < *x.h_.hm->h;
        case HeadSequenceMap:
          return *this < *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case True:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return false;
        case HeadSequence:
          return *this < *x.h_.hs->h;
        case HeadMap:
          return *this < *x.h_.hm->h;
        case HeadSequenceMap:
          return *this < *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return false;
        case Integer:
          return h_.i < x.h_.i;
        case Rational:
          return h_.i < *x.h_.r;
        case Float:
          return h_.i < x.h_.d;
        case Real:
          return h_.i < x.h_.x->toLong();
        case HeadSequence:
          return *this < *x.h_.hs->h;
        case HeadMap:
          return *this < *x.h_.hm->h;
        case HeadSequenceMap:
          return *this < *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return false;
        case Integer:
          return *h_.r < x.h_.i;
        case Rational:
          return *h_.r < *x.h_.r;
        case Float:
          return *h_.r < x.h_.d;
        case Real:
          return *h_.r < *x.h_.x;
        case HeadSequence:
          return *this < *x.h_.hs->h;
        case HeadMap:
          return *this < *x.h_.hm->h;
        case HeadSequenceMap:
          return *this < *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return false;
        case Integer:
          return h_.d < x.h_.i;
        case Rational:
          return h_.d < *x.h_.r;
        case Float:
          return h_.d < x.h_.d;
        case Real:
          return h_.d < x.h_.x->toDouble();
        case HeadSequence:
          return *this < *x.h_.hs->h;
        case HeadMap:
          return *this < *x.h_.hm->h;
        case HeadSequenceMap:
          return *this < *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return false;
        case Integer:
          return *h_.x < x.h_.i;
        case Rational:
          return *h_.x < nreal(*x.h_.r);
        case Float:
          return *h_.x < x.h_.d;
        case Real:
          return *h_.x < *x.h_.x;
        case HeadSequence:
          return *this < *x.h_.hs->h;
        case HeadMap:
          return *this < *x.h_.hm->h;
        case HeadSequenceMap:
          return *this < *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case StringPointer:
    case String:
    case Binary:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
          return false;
        case Binary:
        case String:
        case StringPointer:
          return *h_.s < *x.h_.s;
        case HeadSequence:
          return *this < *x.h_.hs->h;
        case HeadMap:
          return *this < *x.h_.hm->h;
        case HeadSequenceMap:
          return *this < *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case RawPointer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
          return false;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.p < x.h_.o;
        case RawPointer:
          return h_.p < x.h_.p;
        case HeadSequence:
          return *this < *x.h_.hs->h;
        case HeadMap:
          return *this < *x.h_.hm->h;
        case HeadSequenceMap:
          return *this < *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return false;
      }
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
          return false;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.o < x.h_.o;
        case RawPointer:
          return h_.o < x.h_.p;
        case HeadSequence:
          return *this < *x.h_.hs->h;
        case HeadMap:
          return *this < *x.h_.hm->h;
        case HeadSequenceMap:
          return *this < *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case Symbol:
    case Function:
      return nfunc("LT") << new nvar(*this, Copy) << nvar(x, Copy);
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
          return false;
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Vector:{
          int y = h_.v->size() - x.h_.v->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.v < *x.h_.v;
        }
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case List:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
          return false;
        case List:{
          int y = h_.l->size() - x.h_.l->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.l < *x.h_.l;
        }
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case HeadSequence:
      return *h_.hs->h < x;
    case Map:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case HeadSequence:
          return false;
        case Map:{
          int y = h_.m->size() - x.h_.m->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.m < *x.h_.m;
        }
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case Multimap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case HeadSequence:
        case Map:
          return false;
        case Multimap:{
          int y = h_.mm->size() - x.h_.mm->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.mm < *x.h_.mm;
        }
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case HeadMap:
      return *h_.hm->h < x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case HeadSequence:
        case Map:
        case Multimap:
        case HeadMap:
          return false;
        case SequenceMap:{
          int y = h_.sm->s->size() - x.h_.sm->s->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.sm->s < *x.h_.sm->s;
        }
        case Symbol:
        case Function:
          return nfunc("LT") << *this << new nvar(x, Copy);
        case Reference:
          return *this < *x.h_.ref->v;
        case Pointer:
          return *this < *x.h_.vp;
        default:
          return true;
      }
    case HeadSequenceMap:
      return *h_.hsm->h < x;
    case Reference:
      return *h_.ref->v < x;
    case Pointer:
      return *h_.vp < x;
    default:
      return false;
  }
}

nvar nvar::operator<=(int64_t x) const{
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
      return true;
    case Integer:
      return h_.i <= x;
    case Rational:
      return *h_.r <= x;
    case Float:
      return h_.d <= x;
    case Real:
      return *h_.x <= x;
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("LE");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case HeadSequence:
      return *h_.hs->h <= x;
    case HeadMap:
      return *h_.hm->h <= x;
    case HeadSequenceMap:
      return *h_.hsm->h <= x;
    case Reference:
      return *h_.ref->v <= x;
    case Pointer:
      return *h_.vp <= x;
    default:
      return false;
  }
}

nvar nvar::operator<=(double x) const{
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
      return true;
    case Integer:
      return h_.i <= x;
    case Rational:
      return *h_.r <= x;
    case Float:
      return h_.d <= x;
    case Real:
      return *h_.x <= x;
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("LE");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case HeadSequence:
      return *h_.hs->h <= x;
    case HeadMap:
      return *h_.hm->h <= x;
    case HeadSequenceMap:
      return *h_.hsm->h <= x;
    case Reference:
      return *h_.ref->v <= x;
    case Pointer:
      return *h_.vp <= x;
    default:
      return false;
  }
}

nvar nvar::operator<=(const nvar& x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case False:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case HeadSequence:
          return *this <= *x.h_.hs->h;
        case HeadMap:
          return *this <= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this <= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case True:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
          return false;
        case HeadSequence:
          return *this <= *x.h_.hs->h;
        case HeadMap:
          return *this <= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this <= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return false;
        case Integer:
          return h_.i <= x.h_.i;
        case Rational:
          return h_.i <= *x.h_.r;
        case Float:
          return h_.i <= x.h_.d;
        case Real:
          return h_.i <= x.h_.x->toLong();
        case HeadSequence:
          return *this <= *x.h_.hs->h;
        case HeadMap:
          return *this <= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this <= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return false;
        case Integer:
          return *h_.r <= x.h_.i;
        case Rational:
          return *h_.r <= *x.h_.r;
        case Float:
          return *h_.r <= x.h_.d;
        case Real:
          return *h_.r <= *x.h_.x;
        case HeadSequence:
          return *this <= *x.h_.hs->h;
        case HeadMap:
          return *this <= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this <= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return false;
        case Integer:
          return h_.d <= x.h_.i;
        case Rational:
          return h_.d <= *x.h_.r;
        case Float:
          return h_.d <= x.h_.d;
        case Real:
          return h_.d <= x.h_.x->toDouble();
        case HeadSequence:
          return *this <= *x.h_.hs->h;
        case HeadMap:
          return *this <= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this <= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return false;
        case Integer:
          return *h_.x <= x.h_.i;
        case Rational:
          return *h_.x <= nreal(*x.h_.r);
        case Float:
          return *h_.x <= x.h_.d;
        case Real:
          return *h_.x <= *x.h_.x;
        case HeadSequence:
          return *this <= *x.h_.hs->h;
        case HeadMap:
          return *this <= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this <= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case StringPointer:
    case String:
    case Binary:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
          return false;
        case Binary:
        case String:
        case StringPointer:
          return *h_.s <= *x.h_.s;
        case HeadSequence:
          return *this <= *x.h_.hs->h;
        case HeadMap:
          return *this <= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this <= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case RawPointer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
          return false;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.p <= x.h_.o;
        case RawPointer:
          return h_.p <= x.h_.p;
        case HeadSequence:
          return *this <= *x.h_.hs->h;
        case HeadMap:
          return *this <= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this <= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return false;
      }
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
          return false;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.o <= x.h_.o;
        case RawPointer:
          return h_.o <= x.h_.p;
        case HeadSequence:
          return *this <= *x.h_.hs->h;
        case HeadMap:
          return *this <= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this <= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
          return false;
        case Vector:{
          int y = h_.v->size() - x.h_.v->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.v <= *x.h_.v;
        }
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case List:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
          return false;
        case List:{
          int y = h_.l->size() - x.h_.l->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.l <= *x.h_.l;
        }
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case Symbol:
    case Function:
      return nfunc("LE") << new nvar(*this, Copy) << nvar(x, Copy);
    case HeadSequence:
      return *h_.hs->h <= x;
    case Map:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case HeadSequence:
          return false;
        case Map:{
          int y = h_.m->size() - x.h_.m->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.m <= *x.h_.m;
        }
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case Multimap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case HeadSequence:
        case Map:
          return false;
        case Multimap:{
          int y = h_.mm->size() - x.h_.mm->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.mm <= *x.h_.mm;
        }
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case HeadMap:
      return *h_.hm->h <= x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case HeadSequence:
        case Map:
        case Multimap:
        case HeadMap:
          return false;
        case SequenceMap:{
          int y = h_.sm->s->size() - x.h_.sm->s->size();
          
          if(y < 0){
            return true;
          }
          else if(y > 0){
            return false;
          }
          
          return *h_.sm->s <= *x.h_.sm->s;
        }
        case Symbol:
        case Function:
          return nfunc("LE") << *this << new nvar(x, Copy);
        case Reference:
          return *this <= *x.h_.ref->v;
        case Pointer:
          return *this <= *x.h_.vp;
        default:
          return true;
      }
    case HeadSequenceMap:
      return *h_.hsm->h <= x;
    case Reference:
      return *h_.ref->v <= x;
    case Pointer:
      return *h_.vp <= x;
    default:
      return false;
  }
}

nvar nvar::operator>(int64_t x) const{
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
      return false;
    case Integer:
      return h_.i > x;
    case Rational:
      return *h_.r > x;
    case Float:
      return h_.d > x;
    case Real:
      return *h_.x > x;
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("GT");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case HeadSequence:
      return *h_.hs->h > x;
    case HeadMap:
      return *h_.hm->h > x;
    case HeadSequenceMap:
      return *h_.hsm->h > x;
    case Reference:
      return *h_.ref->v > x;
    case Pointer:
      return *h_.vp > x;
    default:
      return true;
  }
}

nvar nvar::operator>(double x) const{
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
      return false;
    case Integer:
      return h_.i > x;
    case Rational:
      return *h_.r > x;
    case Float:
      return h_.d > x;
    case Real:
      return *h_.x > x;
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("GT");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case HeadSequence:
      return *h_.hs->h > x;
    case HeadMap:
      return *h_.hm->h > x;
    case HeadSequenceMap:
      return *h_.hsm->h > x;
    case Reference:
      return *h_.ref->v > x;
    case Pointer:
      return *h_.vp > x;
    default:
      return true;
  }
}

nvar nvar::operator>(const nvar& x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case False:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case HeadSequence:
          return *this > *x.h_.hs->h;
        case HeadMap:
          return *this > *x.h_.hm->h;
        case HeadSequenceMap:
          return *this > *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case True:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
          return true;
        case HeadSequence:
          return *this > *x.h_.hs->h;
        case HeadMap:
          return *this > *x.h_.hm->h;
        case HeadSequenceMap:
          return *this > *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return true;
        case Integer:
          return h_.i > x.h_.i;
        case Rational:
          return h_.i > *x.h_.r;
        case Float:
          return h_.i > x.h_.d;
        case Real:
          return h_.i > x.h_.x->toLong();
        case HeadSequence:
          return *this > *x.h_.hs->h;
        case HeadMap:
          return *this > *x.h_.hm->h;
        case HeadSequenceMap:
          return *this > *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return true;
        case Integer:
          return *h_.r > x.h_.i;
        case Rational:
          return *h_.r > *x.h_.r;
        case Float:
          return *h_.r > x.h_.d;
        case Real:
          return *h_.r > *x.h_.x;
        case HeadSequence:
          return *this > *x.h_.hs->h;
        case HeadMap:
          return *this > *x.h_.hm->h;
        case HeadSequenceMap:
          return *this > *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return true;
        case Integer:
          return h_.d > x.h_.i;
        case Rational:
          return h_.d > *x.h_.r;
        case Float:
          return h_.d > x.h_.d;
        case Real:
          return h_.d > x.h_.x->toDouble();
        case HeadSequence:
          return *this > *x.h_.hs->h;
        case HeadMap:
          return *this > *x.h_.hm->h;
        case HeadSequenceMap:
          return *this > *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return true;
        case Integer:
          return *h_.x > x.h_.i;
        case Rational:
          return *h_.x > nreal(*x.h_.r);
        case Float:
          return *h_.x > x.h_.d;
        case Real:
          return *h_.x > *x.h_.x;
        case HeadSequence:
          return *this > *x.h_.hs->h;
        case HeadMap:
          return *this > *x.h_.hm->h;
        case HeadSequenceMap:
          return *this > *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case StringPointer:
    case String:
    case Binary:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
          return true;
        case Binary:
        case String:
        case StringPointer:
          return *h_.s > *x.h_.s;
        case HeadSequence:
          return *this > *x.h_.hs->h;
        case HeadMap:
          return *this > *x.h_.hm->h;
        case HeadSequenceMap:
          return *this > *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case RawPointer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
          return true;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.p > x.h_.o;
        case RawPointer:
          return h_.p > x.h_.p;
        case HeadSequence:
          return *this > *x.h_.hs->h;
        case HeadMap:
          return *this > *x.h_.hm->h;
        case HeadSequenceMap:
          return *this > *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
          return true;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.o > x.h_.o;
        case RawPointer:
          return h_.o > x.h_.p;
        case HeadSequence:
          return *this > *x.h_.hs->h;
        case HeadMap:
          return *this > *x.h_.hm->h;
        case HeadSequenceMap:
          return *this > *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
          return true;
        case Vector:{
          int y = h_.v->size() - x.h_.v->size();
          
          if(y > 0){
            return false;
          }
          else if(y > 0){
            return true;
          }
          
          return *h_.v > *x.h_.v;
        }
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case List:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
          return true;
        case List:{
          int y = h_.l->size() - x.h_.l->size();
          
          if(y > 0){
            return false;
          }
          else if(y > 0){
            return true;
          }
          
          return *h_.l > *x.h_.l;
        }
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case Symbol:
    case Function:
      return nfunc("GT") << new nvar(*this, Copy) << nvar(x, Copy);
    case HeadSequence:
      return *h_.hs->h > x;
    case Map:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case HeadSequence:
          return true;
        case Map:{
          int y = h_.m->size() - x.h_.m->size();
          
          if(y > 0){
            return false;
          }
          else if(y > 0){
            return true;
          }
          
          return *h_.m > *x.h_.m;
        }
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case Multimap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case HeadSequence:
        case Map:
          return true;
        case Multimap:{
          int y = h_.mm->size() - x.h_.mm->size();
          
          if(y > 0){
            return false;
          }
          else if(y > 0){
            return true;
          }
          
          return *h_.mm > *x.h_.mm;
        }
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case HeadMap:
      return *h_.hm->h > x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case HeadSequence:
        case Map:
        case Multimap:
        case HeadMap:
          return true;
        case SequenceMap:{
          int y = h_.sm->s->size() - x.h_.sm->s->size();
          
          if(y > 0){
            return false;
          }
          else if(y > 0){
            return true;
          }
          
          return *h_.sm->s > *x.h_.sm->s;
        }
        case Symbol:
        case Function:
          return nfunc("GT") << *this << new nvar(x, Copy);
        case Reference:
          return *this > *x.h_.ref->v;
        case Pointer:
          return *this > *x.h_.vp;
        default:
          return false;
      }
    case HeadSequenceMap:
      return *h_.hsm->h > x;
    case Reference:
      return *h_.ref->v > x;
    case Pointer:
      return *h_.vp > x;
    default:
      return true;
  }
}

nvar nvar::operator>=(int64_t x) const{
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
      return false;
    case Integer:
      return h_.i >= x;
    case Rational:
      return *h_.r >= x;
    case Float:
      return h_.d >= x;
    case Real:
      return *h_.x >= x;
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("GE");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case HeadSequence:
      return *h_.hs->h >= x;
    case HeadMap:
      return *h_.hm->h >= x;
    case HeadSequenceMap:
      return *h_.hsm->h >= x;
    case Reference:
      return *h_.ref->v >= x;
    case Pointer:
      return *h_.vp >= x;
    default:
      return true;
  }
}

nvar nvar::operator>=(double x) const{
  switch(t_){
    case None:
    case Undefined:
    case False:
    case True:
      return false;
    case Integer:
      return h_.i >= x;
    case Rational:
      return *h_.r >= x;
    case Float:
      return h_.d >= x;
    case Real:
      return *h_.x >= x;
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("GE");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case HeadSequence:
      return *h_.hs->h >= x;
    case HeadMap:
      return *h_.hm->h >= x;
    case HeadSequenceMap:
      return *h_.hsm->h >= x;
    case Reference:
      return *h_.ref->v >= x;
    case Pointer:
      return *h_.vp >= x;
    default:
      return true;
  }
}

nvar nvar::operator>=(const nvar& x) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case False:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
          return true;
        case HeadSequence:
          return *this >= *x.h_.hs->h;
        case HeadMap:
          return *this >= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this >= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case True:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return true;
        case HeadSequence:
          return *this >= *x.h_.hs->h;
        case HeadMap:
          return *this >= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this >= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return true;
        case Integer:
          return h_.i >= x.h_.i;
        case Rational:
          return h_.i >= *x.h_.r;
        case Float:
          return h_.i >= x.h_.d;
        case Real:
          return h_.i >= x.h_.x->toLong();
        case HeadSequence:
          return *this >= *x.h_.hs->h;
        case HeadMap:
          return *this >= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this >= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return true;
        case Integer:
          return *h_.r >= x.h_.i;
        case Rational:
          return *h_.r >= *x.h_.r;
        case Float:
          return *h_.r >= x.h_.d;
        case Real:
          return *h_.r >= *x.h_.x;
        case HeadSequence:
          return *this >= *x.h_.hs->h;
        case HeadMap:
          return *this >= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this >= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return true;
        case Integer:
          return h_.d >= x.h_.i;
        case Rational:
          return h_.d >= *x.h_.r;
        case Float:
          return h_.d >= x.h_.d;
        case Real:
          return h_.d >= x.h_.x->toDouble();
        case HeadSequence:
          return *this >= *x.h_.hs->h;
        case HeadMap:
          return *this >= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this >= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
          return true;
        case Integer:
          return *h_.x >= x.h_.i;
        case Rational:
          return *h_.x >= nreal(*x.h_.r);
        case Float:
          return *h_.x >= x.h_.d;
        case Real:
          return *h_.x >= *x.h_.x;
        case HeadSequence:
          return *this >= *x.h_.hs->h;
        case HeadMap:
          return *this >= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this >= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case StringPointer:
    case String:
    case Binary:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
          return true;
        case Binary:
        case String:
        case StringPointer:
          return *h_.s >= *x.h_.s;
        case HeadSequence:
          return *this >= *x.h_.hs->h;
        case HeadMap:
          return *this >= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this >= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case RawPointer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
          return true;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.p >= x.h_.o;
        case RawPointer:
          return h_.p >= x.h_.p;
        case HeadSequence:
          return *this >= *x.h_.hs->h;
        case HeadMap:
          return *this >= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this >= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
          return true;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.o >= x.h_.o;
        case RawPointer:
          return h_.o >= x.h_.p;
        case HeadSequence:
          return *this >= *x.h_.hs->h;
        case HeadMap:
          return *this >= *x.h_.hm->h;
        case HeadSequenceMap:
          return *this >= *x.h_.hsm->h;
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
          return true;
        case Vector:{
          int y = h_.v->size() - x.h_.v->size();
          
          if(y < 0){
            return false;
          }
          else if(y > 0){
            return true;
          }
          
          return *h_.v >= *x.h_.v;
        }
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case List:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
          return true;
        case List:{
          int y = h_.l->size() - x.h_.l->size();
          
          if(y < 0){
            return false;
          }
          else if(y > 0){
            return true;
          }
          
          return *h_.l >= *x.h_.l;
        }
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case Symbol:
    case Function:
      return nfunc("GE") << new nvar(*this, Copy) << nvar(x, Copy);
    case HeadSequence:
      return *h_.hs->h >= x;
    case Map:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case HeadSequence:
          return true;
        case Map:{
          int y = h_.m->size() - x.h_.m->size();
          
          if(y < 0){
            return false;
          }
          else if(y > 0){
            return true;
          }
          
          return *h_.m >= *x.h_.m;
        }
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case Multimap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case HeadSequence:
        case Map:
          return true;
        case Multimap:{
          int y = h_.mm->size() - x.h_.mm->size();
          
          if(y < 0){
            return false;
          }
          else if(y > 0){
            return true;
          }
          
          return *h_.mm >= *x.h_.mm;
        }
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case HeadMap:
      return *h_.hm->h >= x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Binary:
        case String:
        case StringPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
        case RawPointer:
        case Vector:
        case List:
        case HeadSequence:
        case Map:
        case Multimap:
        case HeadMap:
          return true;
        case SequenceMap:{
          int y = h_.sm->s->size() - x.h_.sm->s->size();
          
          if(y < 0){
            return false;
          }
          else if(y > 0){
            return true;
          }
          
          return *h_.sm->s >= *x.h_.sm->s;
        }
        case Symbol:
        case Function:
          return nfunc("GE") << *this << new nvar(x, Copy);
        case Reference:
          return *this >= *x.h_.ref->v;
        case Pointer:
          return *this >= *x.h_.vp;
        default:
          return false;
      }
    case HeadSequenceMap:
      return *h_.hsm->h >= x;
    case Reference:
      return *h_.ref->v >= x;
    case Pointer:
      return *h_.vp >= x;
    default:
      return true;
  }
}

nvar nvar::operator==(int64_t x) const{
  switch(t_){
    case Integer:
      return h_.i == x;
    case Rational:
      return *h_.r == x;
    case Float:
      return h_.d == x;
    case Real:
      return *h_.x == x;
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("EQ");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Reference:
      return *h_.ref->v == x;
    case Pointer:
      return *h_.vp == x;
    default:
      return false;
  }
}

nvar nvar::operator==(double x) const{
  switch(t_){
    case Integer:
      return h_.i == x;
    case Rational:
      return *h_.r == x;
    case Float:
      return h_.d == x;
    case Real:
      return *h_.x == x;
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("EQ");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Reference:
      return *h_.ref->v == x;
    case Pointer:
      return *h_.vp == x;
    default:
      return false;
  }
}

nvar nvar::operator==(const nvar& x) const{
  switch(t_){
    case None:
      switch(x.t_){
        case None:
          return true;
        default:
          return false;
      }
    case Undefined:
      switch(x.t_){
        case Undefined:
          return true;
        default:
          return false;
      }
    case False:
      switch(x.t_){
        case False:
          return true;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case True:
      switch(x.t_){
        case True:
          return true;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case Integer:
      switch(x.t_){
        case Integer:
          return h_.i == x.h_.i;
        case Rational:
          return h_.i == *x.h_.r;
        case Float:
          return h_.i == x.h_.d;
        case Real:
          return h_.i == x.h_.x->toLong();
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case Rational:
      switch(x.t_){
        case Integer:
          return *h_.r == x.h_.i;
        case Rational:
          return *h_.r == *x.h_.r;
        case Float:
          return *h_.r == x.h_.d;
        case Real:
          return *h_.r == *x.h_.x;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case Float:
      switch(x.t_){
        case Integer:
          return h_.d == x.h_.i;
        case Rational:
          return h_.d == x.h_.r->toDouble();
        case Float:
          return h_.d == x.h_.d;
        case Real:
          return h_.d == x.h_.x->toDouble();
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case Real:
      switch(x.t_){
        case Integer:
          return *h_.x == x.h_.i;
        case Rational:
          return *h_.x == nreal(*x.h_.r);
        case Float:
          return *h_.x == x.h_.d;
        case Real:
          return *h_.x == *x.h_.x;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case StringPointer:
    case String:
    case Binary:
      switch(x.t_){
        case Binary:
        case String:
        case StringPointer:
          return *h_.s == *x.h_.s;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case RawPointer:
      switch(x.t_){
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.p == x.h_.o;
        case RawPointer:
          return h_.p == x.h_.p;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      switch(x.t_){
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.o == x.h_.o;
        case RawPointer:
          return h_.o == x.h_.p;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case Vector:
      switch(x.t_){
        case Vector:
          return *h_.v == *x.h_.v;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case List:
      switch(x.t_){
        case List:
          return *h_.l == *x.h_.l;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case Symbol:
    case Function:
      return nfunc("EQ") << new nvar(*this, Copy) << nvar(x, Copy);
    case HeadSequence:
      switch(x.t_){
        case HeadSequence:
          return *h_.hs->h == *x.h_.hs->h && *h_.hs->s == *x.h_.hs->s;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case Map:
      switch(x.t_){
        case Map:
          return *h_.m == *x.h_.m;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case Multimap:
      switch(x.t_){
        case Multimap:
          return *h_.mm == *x.h_.mm;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case HeadMap:
      switch(x.t_){
        case HeadMap:
          return *h_.hm->h == *x.h_.hm->h && *h_.hm->m == *x.h_.hm->m;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case SequenceMap:
      switch(x.t_){
        case SequenceMap:
          return *h_.sm->s == *x.h_.sm->s && *h_.sm->m == *x.h_.sm->m;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case HeadSequenceMap:
      switch(x.t_){
        case HeadSequenceMap:
          return *h_.hsm->h == *x.h_.hsm->h &&
          *h_.hsm->s == *x.h_.hsm->s &&
          *h_.hsm->m == *x.h_.hsm->m;
        case Symbol:
        case Function:
          return nfunc("EQ") << *this << new nvar(x, Copy);
        case Reference:
          return *this == *x.h_.ref->v;
        case Pointer:
          return *this == *x.h_.vp;
        default:
          return false;
      }
    case Reference:
      return *h_.ref->v == x;
    case Pointer:
      return *h_.vp == x;
    default:
      return false;
  }
}

nvar nvar::operator!=(int64_t x) const{
  switch(t_){
    case Integer:
      return h_.i != x;
    case Rational:
      return *h_.r != x;
    case Float:
      return h_.d != x;
    case Real:
      return *h_.x != x;
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("NE");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Reference:
      return *h_.ref->v != x;
    case Pointer:
      return *h_.vp != x;
    default:
      return true;
  }
}

nvar nvar::operator!=(double x) const{
  switch(t_){
    case Integer:
      return h_.i != x;
    case Rational:
      return *h_.r != x;
    case Float:
      return h_.d != x;
    case Real:
      return *h_.x != x;
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("NE");
      h.f->v.push_back(new nvar(*this, Copy));
      h.f->v.push_back(x);
      return new nvar(Function, h);
    }
    case Reference:
      return *h_.ref->v != x;
    case Pointer:
      return *h_.vp != x;
    default:
      return true;
  }
}

nvar nvar::operator!=(const nvar& x) const{
  switch(t_){
    case None:
      switch(x.t_){
        case None:
          return false;
        default:
          return true;
      }
    case Undefined:
      switch(x.t_){
        case Undefined:
          return false;
        default:
          return true;
      }
    case False:
      switch(x.t_){
        case False:
          return false;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case True:
      switch(x.t_){
        case True:
          return false;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case Integer:
      switch(x.t_){
        case Integer:
          return h_.i != x.h_.i;
        case Rational:
          return h_.i != *x.h_.r;
        case Float:
          return h_.i != x.h_.d;
        case Real:
          return h_.i != x.h_.x->toLong();
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case Rational:
      switch(x.t_){
        case Integer:
          return *h_.r != x.h_.i;
        case Rational:
          return *h_.r != *x.h_.r;
        case Float:
          return *h_.r != x.h_.d;
        case Real:
          return *h_.r != *x.h_.x;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case Float:
      switch(x.t_){
        case Integer:
          return h_.d != x.h_.i;
        case Rational:
          return h_.d != *x.h_.r;
        case Float:
          return h_.d != x.h_.d;
        case Real:
          return h_.d != x.h_.x->toDouble();
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case Real:
      switch(x.t_){
        case Integer:
          return *h_.x != x.h_.i;
        case Rational:
          return *h_.x != nreal(*x.h_.r);
        case Float:
          return *h_.x != x.h_.d;
        case Real:
          return *h_.x != *x.h_.x;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case StringPointer:
    case String:
    case Binary:
      switch(x.t_){
        case Binary:
        case String:
        case StringPointer:
          return *h_.s != *x.h_.s;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case RawPointer:
      switch(x.t_){
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.p != x.h_.o;
        case RawPointer:
          return h_.p != x.h_.p;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      switch(x.t_){
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.o != x.h_.o;
        case RawPointer:
          return h_.o != x.h_.p;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case Vector:
      switch(x.t_){
        case Vector:
          return *h_.v != *x.h_.v;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case List:
      switch(x.t_){
        case List:
          return *h_.l != *x.h_.l;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case Symbol:
    case Function:
      return nfunc("NE") << new nvar(*this, Copy) << nvar(x, Copy);
    case HeadSequence:
      switch(x.t_){
        case HeadSequence:
          return *h_.hs->h != *x.h_.hs->h && *h_.hs->s != *x.h_.hs->s;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case Map:
      switch(x.t_){
        case Map:
          return *h_.m != *x.h_.m;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case Multimap:
      switch(x.t_){
        case Multimap:
          return *h_.mm != *x.h_.mm;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case HeadMap:
      switch(x.t_){
        case HeadMap:
          return *h_.hm->h != *x.h_.hm->h && *h_.hm->m != *x.h_.hm->m;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case SequenceMap:
      switch(x.t_){
        case SequenceMap:
          return *h_.sm->s != *x.h_.sm->s && *h_.sm->m != *x.h_.sm->m;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case HeadSequenceMap:
      switch(x.t_){
        case HeadSequenceMap:
          return *h_.hsm->h != *x.h_.hsm->h &&
          *h_.hsm->s != *x.h_.hsm->s &&
          *h_.hsm->m != *x.h_.hsm->m;
        case Symbol:
        case Function:
          return nfunc("NE") << *this << new nvar(x, Copy);
        case Reference:
          return *this != *x.h_.ref->v;
        case Pointer:
          return *this != *x.h_.vp;
        default:
          return true;
      }
    case Reference:
      return *h_.ref->v != x;
    case Pointer:
      return *h_.vp != x;
    default:
      return true;
  }
}

nvar nvar::operator&&(const nvar& x) const{
  switch(t_){
    case None:
    case False:
    case Undefined:
      return false;
    case True:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return false;
        case True:
          return true;
        case Integer:
          return x.h_.i != 0;
        case Rational:
          return *x.h_.r != 0;
        case Float:
          return x.h_.d != 0.0;
        case Real:
          return *x.h_.x != 0;
        case Function:
        case Symbol:
        case Vector:
        case HeadSequence:
        case HeadMap:
        case SequenceMap:
        case HeadSequenceMap:
          return *this;
        case Reference:
          return *x.h_.ref->v;
        case Pointer:
          return *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return false;
        case True:
          return h_.i != 0;
        case Integer:
          return h_.i != 0 && x.h_.i != 0;
        case Rational:
          return h_.i != 0 && *x.h_.r != 0;
        case Float:
          return h_.i != 0 && x.h_.d != 0.0;
        case Real:
          return h_.i != 0 && *x.h_.x != 0;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("And");
          h.f->v.push_back(h_.i);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.i);
          v->andBy(*x.h_.v);
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
          v->andBy(*x.h_.hs->s);
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this && *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
          v->andBy(*x.h_.sm->s);
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
          v->andBy(*x.h_.hsm->s);
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case Reference:
          return *this && *x.h_.ref->v;
        case Pointer:
          return *this && *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return false;
        case True:
          return *h_.r != 0;
        case Integer:
          return *h_.r != 0 && x.h_.i != 0;
        case Rational:
          return *h_.r != 0 && *x.h_.r != 0;
        case Float:
          return *h_.r != 0 && x.h_.d != 0.0;
        case Real:
          return *h_.r != 0 && *x.h_.x != 0;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("And");
          h.f->v.push_back(*h_.r);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.r);
          h.v->andBy(*x.h_.v);
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.r);
          h.v->andBy(x.h_.hs->s);
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this && *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.r);
          h.v->andBy(*x.h_.sm->s);
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
          h.v->andBy(*x.h_.hsm->s);
          return nvar(Vector, h);
        }
        case Reference:
          return *this && *x.h_.ref->v;
        case Pointer:
          return *this && *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return false;
        case True:
          return h_.d != 0.0;
        case Integer:
          return h_.d != 0.0 && x.h_.i != 0;
        case Rational:
          return h_.d != 0.0 && *x.h_.r != 0;
        case Float:
          return h_.d != 0.0 && x.h_.d != 0.0;
        case Real:
          return h_.d != 0.0 && *x.h_.x != 0;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("And");
          h.f->v.push_back(h_.d);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), h_.d);
          h.v->andBy(*x.h_.v);
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), h_.d);
          h.v->andBy(*x.h_.hs->s);
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this && *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), h_.d);
          h.v->andBy(*x.h_.sm->s);
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), h_.d);
          h.v->andBy(*x.h_.hsm->s);
          return nvar(Vector, h);
        }
        case Reference:
          return *this && *x.h_.ref->v;
        case Pointer:
          return *this && *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return false;
        case True:
          return *h_.x != 0;
        case Integer:
          return *h_.x != 0 && x.h_.i != 0;
        case Rational:
          return *h_.x != 0 && *x.h_.r != 0;
        case Float:
          return *h_.x != 0.0 && x.h_.d != 0.0;
        case Real:
          return *h_.x != 0 && *x.h_.x != 0;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("And");
          h.f->v.push_back(*h_.x);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.x);
          h.v->andBy(*x.h_.v);
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.x);
          h.v->andBy(*x.h_.hs->s);
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this && *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.x);
          h.v->andBy(*x.h_.sm->s);
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
          h.v->andBy(*x.h_.hsm->s);
          return nvar(Vector, h);
        }
        case Reference:
          return *this && *x.h_.ref->v;
        case Pointer:
          return *this && *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case String:
    case StringPointer:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return false;
        case True:
          return !h_.s->empty();
        case Integer:
          return !h_.s->empty() && x.h_.i != 0;
        case Rational:
          return !h_.s->empty() && *x.h_.r != 0;
        case Float:
          return !h_.s->empty() && x.h_.d != 0.0;
        case Real:
          return !h_.s->empty() && *x.h_.x != 0;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("And");
          h.f->v.push_back(*h_.s);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.x);
          h.v->andBy(*x.h_.v);
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.x);
          h.v->andBy(*x.h_.hs->s);
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this && *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.x);
          h.v->andBy(*x.h_.sm->s);
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
          h.v->andBy(*x.h_.hsm->s);
          return nvar(Vector, h);
        }
        case Reference:
          return *this && *x.h_.ref->v;
        case Pointer:
          return *this && *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("And");
      h.f->v.emplace_back(new nvar(*this, Copy));
      h.f->v.emplace_back(nvar(x, Copy));
      
      return new nvar(Function, h);
    }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return false;
        case True:
          return *this;
        case Integer:
        case Rational:
        case Float:
        case Real:
          return h_.v && x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("And");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v && *x.h_.v;
        case HeadSequence:
          return *h_.v && *x.h_.hs->s;
        case HeadMap:
          return *this && *x.h_.hm->h;
        case SequenceMap:
          return *h_.v && *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.v && *x.h_.hsm->s;
        case Reference:
          return *this && *x.h_.ref->v;
        case Pointer:
          return *this && *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequence:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return false;
        case True:
          return *this;
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hs->s && x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("And");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hs->s && *x.h_.v;
        case HeadSequence:
          return *h_.hs->s && *x.h_.hs->s;
        case HeadMap:
          return *this && *x.h_.hm->h;
        case SequenceMap:
          return *h_.hs->s && *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hs->s && *x.h_.hsm->s;
        case Reference:
          return *this && *x.h_.ref->v;
        case Pointer:
          return *this && *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadMap:
      return *h_.hm->h && x;
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return false;
        case True:
          return *this;
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.sm->s && x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("And");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.sm->s && *x.h_.v;
        case HeadSequence:
          return *h_.sm->s && *x.h_.hs->s;
        case HeadMap:
          return *this && *x.h_.hm->h;
        case SequenceMap:
          return *h_.sm->s && *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.sm->s && *x.h_.hsm->s;
        case Reference:
          return *this && *x.h_.ref->v;
        case Pointer:
          return *this && *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return false;
        case True:
          return *this;
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hsm->s && x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("And");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hsm->s && *x.h_.v;
        case HeadSequence:
          return *h_.hsm->s && *x.h_.hs->s;
        case HeadMap:
          return *this && *x.h_.hm->h;
        case SequenceMap:
          return *h_.hsm->s && *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hsm->s && *x.h_.hsm->s;
        case Reference:
          return *this && *x.h_.ref->v;
        case Pointer:
          return *this && *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Reference:
      return *h_.ref->v && x;
    case Pointer:
      return *h_.vp && x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator||(const nvar& x) const{
  switch(t_){
    case None:
    case Undefined:
    case False:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return false;
        case True:
          return true;
        case Integer:
          return x.h_.i != 0;
        case Rational:
          return *x.h_.r != 0;
        case Float:
          return x.h_.d != 0.0;
        case Real:
          return *x.h_.x != 0;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Or");
          h.f->v.push_back(undef);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), undef);
          v->orBy(*x.h_.v);
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), undef);
          v->orBy(*x.h_.hs->s);
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), undef);
          v->orBy(*x.h_.sm->s);
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), undef);
          v->orBy(*x.h_.hsm->s);
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case Reference:
          return *this || *x.h_.ref->v;
        case Pointer:
          return *this || *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case True:
      return true;
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return h_.i != 0;
        case True:
          return true;
        case Integer:
          return h_.i != 0 || x.h_.i != 0;
        case Rational:
          return h_.i != 0 || *x.h_.r != 0;
        case Float:
          return h_.i != 0 || x.h_.d != 0.0;
        case Real:
          return h_.i != 0 || *x.h_.x != 0;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Or");
          h.f->v.push_back(h_.i);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          nvec* v = new nvec(x.h_.v->size(), h_.i);
          v->orBy(*x.h_.v);
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequence:{
          nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
          v->orBy(*x.h_.hs->s);
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this || *x.h_.hm->h;
        case SequenceMap:{
          nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
          v->orBy(*x.h_.sm->s);
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
          v->orBy(*x.h_.hsm->s);
          Head h;
          h.v = v;
          return nvar(Vector, h);
        }
        case Reference:
          return *this || *x.h_.ref->v;
        case Pointer:
          return *this || *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return *h_.r != 0;
        case True:
          return true;
        case Integer:
          return *h_.r != 0 || x.h_.i != 0;
        case Rational:
          return *h_.r != 0 || *x.h_.r != 0;
        case Float:
          return *h_.r != 0 || x.h_.d != 0.0;
        case Real:
          return *h_.r != 0 || *x.h_.x != 0;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Or");
          h.f->v.push_back(*h_.r);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.r);
          h.v->orBy(*x.h_.v);
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.r);
          h.v->orBy(x.h_.hs->s);
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this || *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.r);
          h.v->orBy(*x.h_.sm->s);
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
          h.v->orBy(*x.h_.hsm->s);
          return nvar(Vector, h);
        }
        case Reference:
          return *this || *x.h_.ref->v;
        case Pointer:
          return *this || *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return h_.d != 0.0;
        case True:
          return true;
        case Integer:
          return h_.d != 0.0 || x.h_.i != 0;
        case Rational:
          return h_.d != 0.0 || *x.h_.r != 0;
        case Float:
          return h_.d != 0.0 || x.h_.d != 0.0;
        case Real:
          return h_.d != 0.0 || *x.h_.x != 0;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Or");
          h.f->v.push_back(h_.d);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), h_.d);
          h.v->orBy(*x.h_.v);
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), h_.d);
          h.v->orBy(*x.h_.hs->s);
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this || *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), h_.d);
          h.v->orBy(*x.h_.sm->s);
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), h_.d);
          h.v->orBy(*x.h_.hsm->s);
          return nvar(Vector, h);
        }
        case Reference:
          return *this || *x.h_.ref->v;
        case Pointer:
          return *this || *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return *h_.x != 0;
        case True:
          return true;
        case Integer:
          return *h_.x != 0 || x.h_.i != 0;
        case Rational:
          return *h_.x != 0 || *x.h_.r != 0;
        case Float:
          return h_.d != 0.0 || x.h_.d != 0.0;
        case Real:
          return *h_.x != 0 || *x.h_.x != 0;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Or");
          h.f->v.push_back(*h_.x);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.x);
          h.v->orBy(*x.h_.v);
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.x);
          h.v->orBy(*x.h_.hs->s);
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this || *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.x);
          h.v->orBy(*x.h_.sm->s);
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
          h.v->orBy(*x.h_.hsm->s);
          return nvar(Vector, h);
        }
        case Reference:
          return *this || *x.h_.ref->v;
        case Pointer:
          return *this || *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case String:
    case StringPointer:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return !h_.s->empty();
        case True:
          return true;
        case Integer:
          return !h_.s->empty() || x.h_.i != 0;
        case Rational:
          return !h_.s->empty() || *x.h_.r != 0;
        case Float:
          return !h_.s->empty() || x.h_.d != 0.0;
        case Real:
          return !h_.s->empty() || *x.h_.x != 0;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Or");
          h.f->v.push_back(*h_.s);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          Head h;
          h.v = new nvec(x.h_.v->size(), *h_.x);
          h.v->orBy(*x.h_.v);
          return nvar(Vector, h);
        }
        case HeadSequence:{
          Head h;
          h.v = new nvec(x.h_.hs->s->size(), *h_.x);
          h.v->orBy(*x.h_.hs->s);
          return nvar(Vector, h);
        }
        case HeadMap:
          return *this || *x.h_.hm->h;
        case SequenceMap:{
          Head h;
          h.v = new nvec(x.h_.sm->s->size(), *h_.x);
          h.v->orBy(*x.h_.sm->s);
          return nvar(Vector, h);
        }
        case HeadSequenceMap:{
          Head h;
          h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
          h.v->orBy(*x.h_.hsm->s);
          return nvar(Vector, h);
        }
        case Reference:
          return *this || *x.h_.ref->v;
        case Pointer:
          return *this || *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("Or");
      h.f->v.emplace_back(new nvar(*this, Copy));
      h.f->v.emplace_back(nvar(x, Copy));
      
      return new nvar(Function, h);
    }
    case Vector:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return h_.v;
        case True:
          return true;
        case Integer:
        case Rational:
        case Float:
        case Real:
          return h_.v || x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Or");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v || *x.h_.v;
        case HeadSequence:
          return *h_.v || *x.h_.hs->s;
          return *this;
        case HeadMap:
          return *this || *x.h_.hm->h;
        case SequenceMap:
          return *h_.v || *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.v || *x.h_.hsm->s;
        case Reference:
          return *this || *x.h_.ref->v;
        case Pointer:
          return *this || *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequence:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return *h_.hs->s;
        case True:
          return true;
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hs->s || x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Or");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hs->s || *x.h_.v;
        case HeadSequence:
          return *h_.hs->s || *x.h_.hs->s;
        case HeadMap:
          return *this || *x.h_.hm->h;
        case SequenceMap:
          return *h_.hs->s || *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hs->s || *x.h_.hsm->s;
        case Reference:
          return *this || *x.h_.ref->v;
        case Pointer:
          return *this || *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case SequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return *h_.sm->s;
        case True:
          return true;
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.sm->s || x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Or");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.sm->s || *x.h_.v;
        case HeadSequence:
          return *h_.sm->s || *x.h_.hs->s;
        case HeadMap:
          return *this || *x.h_.hm->h;
        case SequenceMap:
          return *h_.sm->s || *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.sm->s || *x.h_.hsm->s;
        case Reference:
          return *this || *x.h_.ref->v;
        case Pointer:
          return *this || *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case HeadSequenceMap:
      switch(x.t_){
        case None:
        case Undefined:
        case False:
          return *h_.hsm->s;
        case True:
          return true;
        case Integer:
        case Rational:
        case Float:
        case Real:
          return *h_.hsm->s || x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Or");
          h.f->v.push_back(*this);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.hsm->s || *x.h_.v;
        case HeadSequence:
          return *h_.hsm->s || *x.h_.hs->s;
        case HeadMap:
          return *this || *x.h_.hm->h;
        case SequenceMap:
          return *h_.hsm->s || *x.h_.sm->s;
        case HeadSequenceMap:
          return *h_.hsm->s || *x.h_.hsm->s;
        case Reference:
          return *this || *x.h_.ref->v;
        case Pointer:
          return *this || *x.h_.vp;
        default:
          NERROR("invalid operands");
      }
    case Reference:
      return *h_.ref->v || x;
    case Pointer:
      return *h_.vp || x;
    default:
      NERROR("left operand is invalid");
  }
}

nvar nvar::operator!() const{
  switch(t_){
    case None:
    case Undefined:
    case False:
      return true;
    case True:
      return false;
    case Integer:
      return h_.i == 0;
    case Rational:
      return *h_.r == 0;
    case Float:
      return h_.d == 0;
    case Real:
      return *h_.x == 0;
    case String:
    case StringPointer:
      return false;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Not");
      h.f->v.push_back(new nvar(*this, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      nvar ret;
      ret.t_ = Vector;
      ret.h_.v = new nvec(!*h_.v);
      return ret;
    }
    case HeadSequence:
      return !*h_.hs->s;
    case HeadMap:
      return !*h_.hm->h;
    case SequenceMap:
      return !*h_.sm->s;
    case HeadSequenceMap:
      return !*h_.hsm->s;
    default:
      NERROR("invalid operand");
  }
}

nvar& nvar::operator[](const nvar& key){
  switch(key.t_){
    case Integer:
      switch(t_){
        case Vector:{
          size_t k = key.asLong();
          if(k >= h_.v->size()){
            NERROR("index out of range: " + key);
          }
          return (*h_.v)[k];
        }
        case List:{
          size_t k = key.asLong();
          if(k >= h_.l->size()){
            NERROR("index out of range: " + key);
          }
          return (*h_.l)[k];
        }
        case Function:{
          size_t k = key.asLong();
          if(k >= h_.f->v.size()){
            NERROR("index out of range: "  + key);
          }
          return h_.f->v[k];
        }
        case HeadSequence:
          return (*h_.hs->s)[key];
        case Map:{
          auto itr = h_.m->find(key);
          if(itr == h_.m->end()){
            NERROR("invalid key: " + key);
          }
          return itr->second;
        }
        case HeadMap:
          return (*h_.hm->m)[key];
        case SequenceMap:
          return (*h_.sm->s)[key];
        case HeadSequenceMap:
          return (*h_.hsm->s)[key];
        case Reference:
          return (*h_.ref->v)[key];
        case Pointer:
          return (*h_.vp)[key];
        default:
          NERROR("invalid operand");
      }
    case Reference:
      return (*this)[*key.h_.ref->v];
    case Pointer:
      return (*this)[*key.h_.vp];
    default:
      switch(t_){
        case Function:
          if(h_.f->m){
            auto itr = h_.f->m->find(key);
            if(itr == h_.f->m->end()){
              NERROR("invalid key: " + key);
            }
            return itr->second;
          }
          NERROR("function does not hold a map");
        case Map:{
          auto itr = h_.m->find(key);
          if(itr == h_.m->end()){
            NERROR("invalid key: " + key);
          }
          return itr->second;
          break;
        }
        case HeadMap:
          return (*h_.hm->m)[key];
        case SequenceMap:
          return (*h_.sm->m)[key];
        case HeadSequenceMap:
          return (*h_.hsm->m)[key];
        case Reference:
          return (*h_.ref->v)[key];
        case Pointer:
          return (*h_.vp)[key];
        default:
          NERROR("invalid operand");
      }
  }
}

nvar& nvar::operator[](int k){
  switch(t_){
    case Vector:
      if(k >= h_.v->size()){
        NERROR("index out of range: " + nstr::toStr(k));
      }
      return (*h_.v)[k];
    case List:
      if(k >= h_.l->size()){
        NERROR("index out of range: " + nstr::toStr(k));
      }
      return (*h_.l)[k];
    case Function:
      if(k >= h_.f->v.size()){
        NERROR("index out of range: " + nstr::toStr(k));
      }
      return h_.f->v[k];
    case HeadSequence:
      return (*h_.hs->s)[k];
    case Map:{
      auto itr = h_.m->find(k);
      if(itr == h_.m->end()){
        NERROR("invalid key: " + nstr::toStr(k));
      }
      return itr->second;
    }
    case HeadMap:
      return (*h_.hm->m)[k];
    case SequenceMap:
      return (*h_.sm->s)[k];
    case HeadSequenceMap:
      return (*h_.hsm->s)[k];
    case Reference:
      return (*h_.ref->v)[k];
    case Pointer:
      return (*h_.vp)[k];
    default:
      NERROR("invalid operand");
  }
}

nvar& nvar::operator[](const char* key){
  switch(t_){
    case Map:{
      auto itr = h_.m->find(key);
      if(itr == h_.m->end()){
        NERROR("invalid key: " + nstr::toStr(key));
      }
      return itr->second;
    }
    case Function:
      if(h_.f->m){
        auto itr = h_.f->m->find(key);
        if(itr == h_.f->m->end()){
          NERROR("invalid key: " + nstr::toStr(key));
        }
        return itr->second;
      }
      NERROR("function does not hold a map");
    case HeadMap:
      return (*h_.hm->m)[key];
    case SequenceMap:
      return (*h_.sm->m)[key];
    case HeadSequenceMap:
      return (*h_.hsm->m)[key];
    case Reference:
      return (*h_.ref->v)[key];
    case Pointer:
      return (*h_.vp)[key];
    default:
      NERROR("invalid operand");
  }
}

nvar& nvar::get(const nvar& key){
  switch(t_){
    case Function:
      if(h_.f->m){
        auto itr = h_.f->m->find(key);
        if(itr == h_.f->m->end()){
          NERROR("invalid key: " + key);
        }
        return itr->second;
      }
      NERROR("function does not hold a map");
    case Map:{
      auto itr = h_.m->find(key);
      if(itr == h_.m->end()){
        NERROR("invalid key: " + key);
      }
      return itr->second;
      break;
    }
    case HeadMap:
      return (*h_.hm->m)[key];
    case SequenceMap:
      return (*h_.sm->m)[key];
    case HeadSequenceMap:
      return (*h_.hsm->m)[key];
    case Reference:
      return h_.ref->v->get(key);
    case Pointer:
      return h_.vp->get(key);
    default:
      NERROR("invalid operand");
  }
}

const nvar& nvar::get(const nvar& key, const nvar& def) const{
  switch(t_){
    case Function:
      return h_.f->m ? h_.f->m->get(key, def) : def;
    case Map:
      return h_.m->get(key, def);
    case HeadMap:
      return h_.hm->m->get(key, def);
    case SequenceMap:
      return h_.sm->m->get(key, def);
    case HeadSequenceMap:
      return h_.hsm->m->get(key, def);
    case Reference:
      return h_.ref->v->get(key, def);
    case Pointer:
      return h_.vp->get(key, def);
    default:
      return def;
  }
}

nvar& nvar::get(const nvar& key, nvar& def){
  switch(t_){
    case Function:
      return h_.f->m ? h_.f->m->get(key, def) : def;
    case Map:
      return h_.m->get(key, def);
    case HeadMap:
      return h_.hm->m->get(key, def);
    case SequenceMap:
      return h_.sm->m->get(key, def);
    case HeadSequenceMap:
      return h_.hsm->m->get(key, def);
    case Reference:
      return h_.ref->v->get(key, def);
    case Pointer:
      return h_.vp->get(key, def);
    default:
      return def;
  }
}

nvar& nvar::operator()(const char* k){
  if(nstr::isSymbol(k)){
    return (*this)(nvar(k, Sym));
  }
  
  return (*this)(nvar(k));
}

nvar& nvar::operator()(const nvar& key){
  switch(t_){
    case None:
    case Undefined:
      t_ = Map;
      h_.m = new nmap;
      return (*h_.m)[key];
    case Vector:
    case List:{
      Head h;
      h.m = new nmap;
      h_.sm = new CSequenceMap(new nvar(t_, h_), new nvar(Map, h));
      t_ = SequenceMap;
      return (*h.m)[key];
    }
    case Function:{
      if(!h_.f->m){
        h_.f->m = new nmap;
      }
      return (*h_.f->m)[key];
    }
    case HeadSequence:{
      Head h;
      h.m = new nmap;
      CHeadSequenceMap* hsm =
      new CHeadSequenceMap(h_.hs->h, h_.hs->s, new nvar(Map, h));
      delete h_.hs;
      h_.hsm = hsm;
      t_ = HeadSequenceMap;
      return (*h.m)[key];
    }
    case Map:
      return (*h_.m)[key];
    case Multimap:{
      auto itr = h_.mm->insert({key, nvar()});
      return itr->second;
    }
    case HeadMap:
      return (*h_.hm->m)(key);
    case SequenceMap:
      return (*h_.sm->m)(key);
    case HeadSequenceMap:
      return (*h_.hsm->m)(key);
    case Reference:
      return (*h_.ref->v)(key);
    case Pointer:
      return (*h_.vp)(key);
    default:{
      Head h;
      h.m = new nmap;
      h_.hm = new CHeadMap(new nvar(t_, h_), new nvar(Map, h));
      t_ = HeadMap;
      return (*h.m)[key];
    }
  }
}

void nvar::erase(const nvar& key){
  switch(t_){
    case Function:
      if(h_.f->m){
        h_.f->m->erase(key);
      }
      break;
    case Map:
      h_.m->erase(key);
      break;
    case Multimap:
      h_.mm->erase(key);
      break;
    case HeadMap:
      h_.hm->m->erase(key);
      break;
    case HeadSequenceMap:
      h_.hsm->m->erase(key);
      break;
    case Reference:
      h_.ref->v->erase(key);
      break;
    case Pointer:
      h_.vp->erase(key);
      break;
  }
}

nvar nvar::keys() const{
  nvec ks;
  keys(ks);
  return nvar(move(ks));
}

nvar nvar::allKeys() const{
  nvec ks;
  allKeys(ks);
  return nvar(move(ks));
}

void nvar::touchVector(){
  switch(t_){
    case None:
    case Undefined:
      t_ = Vector;
      h_.v = new nvec;
      break;
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case String:
    case StringPointer:
    case Binary:
    case RawPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:{
      Head h;
      h.v = new nvec;
      h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
      t_ = HeadSequence;
      break;
    }
    case Vector:
      break;
    case List:{
      nvec* v = new nvec(h_.l->begin(), h_.l->end());
      delete h_.l;
      h_.v = v;
      t_ = Vector;
      break;
    }
    case HeadSequence:
      h_.hs->s->touchVector();
      break;
    case Map:
    case Multimap:{
      Head h;
      h.v = new nvec;
      h_.sm = new CSequenceMap(new nvar(Vector, h), new nvar(t_, h_));
      t_ = SequenceMap;
      break;
    }
    case HeadMap:{
      Head h;
      h.v = new nvec;
      CHeadMap* hm = h_.hm;
      h_.hsm =
      new CHeadSequenceMap(hm->h, new nvar(t_, h_), hm->m);
      t_ = HeadSequenceMap;
      delete hm;
      break;
    }
    case SequenceMap:
      h_.sm->s->touchVector();
      break;
    case HeadSequenceMap:
      h_.hsm->s->touchVector();
      break;
    case Reference:
      h_.ref->v->touchVector();
      break;
    case Pointer:
      h_.vp->touchVector();
      break;
    default:
      NERROR("invalid operand");
  }
}

void nvar::touchVector(size_t size, const nvar& v){
  switch(t_){
    case None:
    case Undefined:
      t_ = Vector;
      h_.v = new nvec(size, v);
      break;
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case String:
    case StringPointer:
    case Binary:
    case RawPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:{
      Head h;
      h.v = new nvec(size, v);
      h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
      t_ = HeadSequence;
      break;
    }
    case Map:
    case Multimap:{
      Head h;
      h.v = new nvec(size, v);
      h_.sm = new CSequenceMap(new nvar(Vector, h), new nvar(t_, h_));
      t_ = SequenceMap;
      break;
    }
    case HeadMap:{
      Head h;
      h.v = new nvec(size, v);
      CHeadMap* hm = h_.hm;
      h_.hsm =
      new CHeadSequenceMap(hm->h, new nvar(t_, h_), hm->m);
      t_ = HeadSequenceMap;
      delete hm;
      break;
    }
    case Reference:
      h_.ref->v->touchVector(size, v);
      break;
    case Pointer:
      h_.vp->touchVector(size, v);
      break;
    default:
      NERROR("invalid operand");
  }
}

void nvar::touchList(){
  switch(t_){
    case None:
    case Undefined:
      t_ = List;
      h_.l = new nlist;
      break;
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case String:
    case StringPointer:
    case Binary:
    case RawPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:{
      Head h;
      h.l = new nlist;
      h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(List, h));
      t_ = HeadSequence;
      break;
    }
    case Vector:{
      nlist* l = new nlist(h_.v->begin(), h_.v->end());
      delete h_.v;
      h_.l = l;
      t_ = List;
      break;
    }
    case List:
      break;
    case HeadSequence:
      h_.hs->s->touchList();
      break;
    case Map:
    case Multimap:{
      Head h;
      h.l = new nlist;
      h_.sm = new CSequenceMap(new nvar(List, h), new nvar(t_, h_));
      t_ = SequenceMap;
      break;
    }
    case HeadMap:{
      Head h;
      h.l = new nlist;
      CHeadMap* hm = h_.hm;
      h_.hsm =
      new CHeadSequenceMap(hm->h, new nvar(t_, h_), hm->m);
      t_ = HeadSequenceMap;
      delete hm;
      break;
    }
    case SequenceMap:
      h_.sm->s->touchList();
      break;
    case HeadSequenceMap:
      h_.hsm->s->touchList();
      break;
    case Reference:
      h_.ref->v->touchList();
      break;
    case Pointer:
      h_.vp->touchList();
      break;
    default:
      NERROR("invalid operand: " + *this);
  }
}

void nvar::touchMap(){
  switch(t_){
    case None:
    case Undefined:
      t_ = Map;
      h_.m = new nmap;
      break;
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case String:
    case StringPointer:
    case Binary:
    case RawPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:{
      Head h;
      h.m = new nmap;
      h_.hm = new CHeadMap(new nvar(t_, h_), new nvar(Map, h));
      t_ = HeadMap;
      break;
    }
    case Vector:
    case List:{
      Head h;
      h.m = new nmap;
      h_.sm = new CSequenceMap(new nvar(t_, h_), new nvar(Map, h));
      t_ = SequenceMap;
      break;
    }
    case Function:
      if(h_.f->m){
        break;
      }
      h_.f->m = new nmap;
      break;
    case HeadSequence:{
      Head h;
      h.m = new nmap;
      CHeadSequence* hs = h_.hs;
      h_.hsm = new CHeadSequenceMap(hs->h, hs->s, new nvar(Map, h));
      t_ = HeadSequenceMap;
      delete hs;
      break;
    }
    case Map:
      break;
    case Multimap:{
      const nmmap& mm = *h_.mm;
      nmap* m = new nmap;
      for(auto& itr : mm){
        auto itr2 = m->find(itr.first);
        if(itr2 == m->end()){
          m->insert({itr.first, nvec(1, itr.second)});
        }
        else{
          itr2->second.pushBack(itr.second);
        }
      }
      delete h_.mm;
      h_.m = m;
      t_ = Map;
      break;
    }
    case HeadMap:
      h_.hm->m->touchMap();
      break;
    case SequenceMap:
      h_.sm->m->touchMap();
      break;
    case HeadSequenceMap:
      h_.hsm->m->touchMap();
      break;
    case Reference:
      h_.ref->v->touchMap();
      break;
    case Pointer:
      h_.vp->touchMap();
      break;
    default:
      NERROR("invalid operand");
  }
}

void nvar::touchMultimap(){
  switch(t_){
    case None:
    case Undefined:
      t_ = Multimap;
      h_.mm = new nmmap;
      break;
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case String:
    case StringPointer:
    case Binary:
    case RawPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:{
      Head h;
      h.mm = new nmmap;
      h_.hm = new CHeadMap(new nvar(t_, h_), new nvar(Multimap, h));
      t_ = HeadMap;
      break;
    }
    case Vector:
    case List:{
      Head h;
      h.mm = new nmmap;
      h_.sm = new CSequenceMap(new nvar(t_, h_), new nvar(Multimap, h));
      t_ = SequenceMap;
      break;
    }
    case HeadSequence:{
      Head h;
      h.mm = new nmmap;
      CHeadSequence* hs = h_.hs;
      h_.hsm =
      new CHeadSequenceMap(hs->h, hs->s, new nvar(Multimap, h));
      t_ = HeadSequenceMap;
      delete hs;
      break;
    }
    case Map:{
      const nmap& m = *h_.m;
      h_.mm = new nmmap(m.begin(), m.end());
      t_ = Multimap;
      break;
    }
    case Multimap:
      break;
    case HeadMap:
      h_.hm->m->touchMultimap();
    case SequenceMap:
      h_.sm->m->touchMultimap();
      break;
    case HeadSequenceMap:
      h_.hsm->m->touchMultimap();
      break;
    case Reference:
      h_.ref->v->touchMultimap();
      break;
    case Pointer:
      h_.vp->touchMultimap();
      break;
    default:
      NERROR("invalid operand");
  }
}

void nvar::append(const nvar& x){
  switch(t_){
    case None:
    case Undefined:
      switch(x.t_){
        case Vector:
          t_ = Vector;
          h_.v = new nvec(*x.h_.v);
          break;
        case List:
          t_ = List;
          h_.l = new nlist(*x.h_.l);
          break;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          break;
        case HeadSequence:
          append(*x.h_.hs->s);
          break;
        case SequenceMap:
          append(*x.h_.sm->s);
          break;
        case HeadSequenceMap:
          append(*x.h_.hsm->s);
          break;
        case Reference:
          append(*x.h_.ref->v);
          break;
        case Pointer:
          append(*x.h_.vp);
          break;
      }
      break;
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case String:
    case StringPointer:
    case Binary:
    case RawPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      switch(x.t_){
        case Vector:{
          Head h;
          h.v = new nvec(*x.h_.v);
          h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
          t_ = HeadSequence;
          break;
        }
        case List:{
          Head h;
          h.l = new nlist(*x.h_.l);
          h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(List, h));
          t_ = HeadSequence;
          break;
        }
        case Function:{
          Head h;
          h.v = new nvec(x.h_.f->v);
          h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
          t_ = HeadSequence;
          break;
        }
        case HeadSequence:
          append(*x.h_.hs->s);
          break;
        case SequenceMap:
          append(*x.h_.sm->s);
          break;
        case HeadSequenceMap:
          append(*x.h_.hsm->s);
          break;
        case Reference:
          append(*x.h_.ref->v);
          break;
        case Pointer:
          append(*x.h_.vp);
          break;
      }
      break;
    case Vector:
      switch(x.t_){
        case Vector:
          h_.v->append(*x.h_.v);
          break;
        case List:
          h_.v->insert(h_.v->begin(), x.h_.l->begin(), x.h_.l->end());
          break;
        case Function:
          h_.v->append(x.h_.f->v);
          break;
        case HeadSequence:
          append(*x.h_.hs->s);
          break;
        case SequenceMap:
          append(*x.h_.sm->s);
          break;
        case HeadSequenceMap:
          append(*x.h_.hsm->s);
          break;
        case Reference:
          append(*x.h_.ref->v);
          break;
        case Pointer:
          append(*x.h_.vp);
          break;
      }
      break;
    case List:
      switch(x.t_){
        case Vector:
          h_.l->insert(h_.l->begin(), x.h_.v->begin(), x.h_.v->end());
          break;
        case List:
          h_.l->append(*x.h_.l);
          break;
        case Function:
          h_.l->insert(h_.l->begin(), x.h_.f->v.begin(), x.h_.f->v.end());
          break;
        case HeadSequence:
          append(*x.h_.hs->s);
          break;
        case SequenceMap:
          append(*x.h_.sm->s);
          break;
        case HeadSequenceMap:
          append(*x.h_.hsm->s);
          break;
        case Reference:
          append(*x.h_.ref->v);
          break;
        case Pointer:
          append(*x.h_.vp);
          break;
      }
      break;
    case Function:
      switch(x.t_){
        case Vector:
          h_.f->v.append(*x.h_.v);
          break;
        case List:
          h_.f->v.insert(h_.v->begin(), x.h_.l->begin(), x.h_.l->end());
          break;
        case Function:
          h_.f->v.append(x.h_.f->v);
          break;
        case HeadSequence:
          append(*x.h_.hs->s);
          break;
        case SequenceMap:
          append(*x.h_.sm->s);
          break;
        case HeadSequenceMap:
          append(*x.h_.hsm->s);
          break;
        case Reference:
          append(*x.h_.ref->v);
          break;
        case Pointer:
          append(*x.h_.vp);
          break;
      }
      break;
    case HeadSequence:
      h_.hs->s->append(x);
      break;
    case Map:
      switch(x.t_){
        case Vector:{
          Head hv;
          hv.v = new nvec(*x.h_.v);
          
          Head hm;
          hm.m = h_.m;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Map, hm));
          t_ = SequenceMap;
          break;
        }
        case List:{
          Head hl;
          hl.l = new nlist(*x.h_.l);
          
          Head hm;
          hm.m = h_.m;
          
          h_.sm = new CSequenceMap(new nvar(List, hl), new nvar(Map, hm));
          t_ = SequenceMap;
          break;
        }
        case Function:{
          Head hv;
          hv.v = new nvec(x.h_.f->v);
          
          Head hm;
          hm.m = h_.m;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Map, hm));
          t_ = SequenceMap;
          break;
        }
        case HeadSequence:
          append(*x.h_.hs->s);
          break;
        case SequenceMap:
          append(*x.h_.sm->s);
          break;
        case HeadSequenceMap:
          append(*x.h_.hsm->s);
          break;
        case Reference:
          append(*x.h_.ref->v);
          break;
        case Pointer:
          append(*x.h_.vp);
          break;
      }
      break;
    case Multimap:
      switch(x.t_){
        case Vector:{
          Head hv;
          hv.v = new nvec(*x.h_.v);
          
          Head hm;
          hm.mm = h_.mm;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Multimap, hm));
          t_ = SequenceMap;
          break;
        }
        case List:{
          Head hl;
          hl.l = new nlist(*x.h_.l);
          
          Head hm;
          hm.mm = h_.mm;
          
          h_.sm = new CSequenceMap(new nvar(List, hl), new nvar(Multimap, hm));
          t_ = SequenceMap;
          break;
        }
        case Function:{
          Head hv;
          hv.v = new nvec(x.h_.f->v);
          
          Head hm;
          hm.mm = h_.mm;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Multimap, hm));
          t_ = SequenceMap;
          break;
        }
        case HeadSequence:
          append(*x.h_.hs->s);
          break;
        case SequenceMap:
          append(*x.h_.sm->s);
          break;
        case HeadSequenceMap:
          append(*x.h_.hsm->s);
          break;
        case Reference:
          append(*x.h_.ref->v);
          break;
        case Pointer:
          append(*x.h_.vp);
          break;
      }
      break;
    case HeadMap:
      switch(x.t_){
        case Vector:{
          Head hv;
          hv.v = new nvec(*x.h_.v);
          
          h_.hsm =
          new CHeadSequenceMap(h_.hm->h, new nvar(Vector, hv), h_.hm->m);
          t_ = HeadSequenceMap;
          break;
        }
        case List:{
          Head hl;
          hl.l = new nlist(*x.h_.l);
          
          h_.hsm =
          new CHeadSequenceMap(h_.hm->h, new nvar(List, hl), h_.hm->m);
          t_ = HeadSequenceMap;
          break;
        }
        case Function:{
          Head hv;
          hv.v = new nvec(x.h_.f->v);
          
          h_.hsm =
          new CHeadSequenceMap(h_.hm->h, new nvar(Vector, hv), h_.hm->m);
          t_ = HeadSequenceMap;
          break;
        }
        case HeadSequence:
          append(*x.h_.hs->s);
          break;
        case SequenceMap:
          append(*x.h_.sm->s);
          break;
        case HeadSequenceMap:
          append(*x.h_.hsm->s);
          break;
        case Reference:
          append(*x.h_.ref->v);
          break;
        case Pointer:
          append(*x.h_.vp);
          break;
      }
      break;
    case SequenceMap:
      h_.sm->s->append(x);
      break;
    case HeadSequenceMap:
      h_.hsm->s->append(x);
      break;
    case Reference:
      h_.ref->v->append(x);
      break;
    case Pointer:
      h_.vp->append(x);
      break;
  }
}

void nvar::setHead(const nvar& x){
  switch(t_){
    case Rational:
      switch(x.t_){
        case Rational:
          *h_.r = *x.h_.r;
          break;
        case Real:
          delete h_.r;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          break;
        case Symbol:
          delete h_.r;
          t_ = Symbol;
          h_.s = new nstr(*x.h_.s);
          break;
        case Binary:
          delete h_.r;
          t_ = Binary;
          h_.s = new nstr(*x.h_.s);
          break;
        case String:
          delete h_.r;
          t_ = String;
          h_.s = new nstr(*x.h_.s);
          break;
        case LocalObject:
          delete h_.r;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          break;
        case SharedObject:
          delete h_.r;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          break;
        case Function:
          delete h_.r;
          t_ = Function;
          h_.f = x.h_.f->clone();
          break;
        case HeadSequence:
          setHead(*x.h_.hs->h);
          break;
        case HeadMap:
          setHead(*x.h_.hm->h);
          break;
        case HeadSequenceMap:
          setHead(*x.h_.hsm->h);
          break;
        case Reference:
          setHead(*x.h_.ref->v);
          break;
        case Pointer:
          setHead(*x.h_.vp);
          break;
        default:
          delete h_.r;
          t_ = x.t_;
          h_ = x.h_;
          break;
      }
    case Real:
      switch(x.t_){
        case Rational:
          delete h_.x;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          break;
        case Real:
          *h_.x = *x.h_.x;
          break;
        case Symbol:
          delete h_.x;
          t_ = Symbol;
          h_.s = new nstr(*x.h_.s);
          break;
        case Binary:
          delete h_.x;
          t_ = Binary;
          h_.s = new nstr(*x.h_.s);
          break;
        case String:
          delete h_.x;
          t_ = String;
          h_.s = new nstr(*x.h_.s);
          break;
        case LocalObject:
          delete h_.x;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          break;
        case SharedObject:
          delete h_.x;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          break;
        case Function:
          delete h_.x;
          t_ = Function;
          h_.f = x.h_.f->clone();
          break;
        case HeadSequence:
          setHead(*x.h_.hs->h);
          break;
        case HeadMap:
          setHead(*x.h_.hm->h);
          break;
        case HeadSequenceMap:
          setHead(*x.h_.hsm->h);
          break;
        case Reference:
          setHead(*x.h_.ref->v);
          break;
        case Pointer:
          setHead(*x.h_.vp);
          break;
        default:
          delete h_.x;
          t_ = x.t_;
          h_ = x.h_;
          break;
      }
    case Symbol:
      switch(x.t_){
        case Rational:
          delete h_.s;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          break;
        case Real:
          delete h_.s;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          break;
        case Symbol:
          *h_.s = *x.h_.s;
          break;
        case Binary:
          t_ = Binary;
          *h_.s = *x.h_.s;
          break;
        case String:
          t_ = String;
          *h_.s = *x.h_.s;
          break;
        case LocalObject:
          delete h_.s;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          break;
        case SharedObject:
          delete h_.s;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          break;
        case Function:
          delete h_.s;
          t_ = Function;
          h_.f = x.h_.f->clone();
          break;
        case HeadSequence:
          setHead(*x.h_.hs->h);
          break;
        case HeadMap:
          setHead(*x.h_.hm->h);
          break;
        case HeadSequenceMap:
          setHead(*x.h_.hsm->h);
          break;
        case Reference:
          setHead(*x.h_.ref->v);
          break;
        case Pointer:
          setHead(*x.h_.vp);
          break;
        default:
          delete h_.s;
          t_ = x.t_;
          h_ = x.h_;
          break;
      }
    case String:
      switch(x.t_){
        case Rational:
          delete h_.s;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          break;
        case Real:
          delete h_.s;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          break;
        case Symbol:
          t_ = Symbol;
          *h_.s = *x.h_.s;
          break;
        case Binary:
          t_ = Binary;
          *h_.s = *x.h_.s;
          break;
        case String:
          *h_.s = *x.h_.s;
          break;
        case LocalObject:
          delete h_.s;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          break;
        case SharedObject:
          delete h_.s;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          break;
        case Function:
          delete h_.s;
          t_ = Function;
          h_.f = x.h_.f->clone();
          break;
        case HeadSequence:
          setHead(*x.h_.hs->h);
          break;
        case HeadMap:
          setHead(*x.h_.hm->h);
          break;
        case HeadSequenceMap:
          setHead(*x.h_.hsm->h);
          break;
        case Reference:
          setHead(*x.h_.ref->v);
          break;
        case Pointer:
          setHead(*x.h_.vp);
          break;
        default:
          delete h_.s;
          t_ = x.t_;
          h_ = x.h_;
          break;
      }
    case Binary:
      switch(x.t_){
        case Rational:
          delete h_.s;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          break;
        case Real:
          delete h_.s;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          break;
        case Symbol:
          t_ = Symbol;
          *h_.s = *x.h_.s;
          break;
        case Binary:
          *h_.s = *x.h_.s;
          break;
        case String:
          t_ = String;
          *h_.s = *x.h_.s;
          break;
        case LocalObject:
          delete h_.s;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          break;
        case SharedObject:
          delete h_.s;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          break;
        case Function:
          delete h_.s;
          t_ = Function;
          h_.f = x.h_.f->clone();
          break;
        case HeadSequence:
          setHead(*x.h_.hs->h);
          break;
        case HeadMap:
          setHead(*x.h_.hm->h);
          break;
        case HeadSequenceMap:
          setHead(*x.h_.hsm->h);
          break;
        case Reference:
          setHead(*x.h_.ref->v);
          break;
        case Pointer:
          setHead(*x.h_.vp);
          break;
        default:
          delete h_.s;
          t_ = x.t_;
          h_ = x.h_;
          break;
      }
    case LocalObject:
      switch(x.t_){
        case Rational:
          delete h_.o;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          break;
        case Real:
          delete h_.o;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          break;
        case Symbol:
          delete h_.o;
          t_ = Symbol;
          h_.s = new nstr(*x.h_.s);
          break;
        case Binary:
          delete h_.o;
          t_ = Binary;
          h_.s = new nstr(*x.h_.s);
          break;
        case String:
          delete h_.o;
          t_ = String;
          h_.s = new nstr(*x.h_.s);
          break;
        case LocalObject:
          delete h_.o;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          break;
        case SharedObject:
          delete h_.o;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          break;
        case Function:
          delete h_.o;
          t_ = Function;
          h_.f = x.h_.f->clone();
          break;
        case HeadSequence:
          setHead(*x.h_.hs->h);
          break;
        case HeadMap:
          setHead(*x.h_.hm->h);
          break;
        case HeadSequenceMap:
          setHead(*x.h_.hsm->h);
          break;
        case Reference:
          setHead(*x.h_.ref->v);
          break;
        case Pointer:
          setHead(*x.h_.vp);
          break;
        default:
          delete h_.o;
          t_ = x.t_;
          h_ = x.h_;
          break;
      }
    case SharedObject:
      switch(x.t_){
        case Rational:
          if(h_.o->deref()){
            delete h_.o;
          }
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          break;
        case Real:
          if(h_.o->deref()){
            delete h_.o;
          }
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          break;
        case Symbol:
          if(h_.o->deref()){
            delete h_.o;
          }
          t_ = Symbol;
          h_.s = new nstr(*x.h_.s);
          break;
        case Binary:
          if(h_.o->deref()){
            delete h_.o;
          }
          t_ = Binary;
          h_.s = new nstr(*x.h_.s);
          break;
        case String:
          if(h_.o->deref()){
            delete h_.o;
          }
          t_ = String;
          h_.s = new nstr(*x.h_.s);
          break;
        case LocalObject:
          if(h_.o->deref()){
            delete h_.o;
          }
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          break;
        case SharedObject:
          if(h_.o->deref()){
            delete h_.o;
          }
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          break;
        case Function:
          if(h_.o->deref()){
            delete h_.o;
          }
          t_ = Function;
          h_.f = x.h_.f->clone();
          break;
        case HeadSequence:
          setHead(*x.h_.hs->h);
          break;
        case HeadMap:
          setHead(*x.h_.hm->h);
          break;
        case HeadSequenceMap:
          setHead(*x.h_.hsm->h);
          break;
        case Reference:
          setHead(*x.h_.ref->v);
          break;
        case Pointer:
          setHead(*x.h_.vp);
          break;
        default:
          delete h_.o;
          t_ = x.t_;
          h_ = x.h_;
          break;
      }
    case Vector:{
      nvar* h = new nvar;
      h->setHead(x);
      
      Head hv;
      hv.v = h_.v;
      
      t_ = HeadSequence;
      h_.hs = new CHeadSequence(h, new nvar(Vector, hv));
      break;
    }
    case List:{
      nvar* h = new nvar;
      h->setHead(x);
      
      Head hl;
      hl.l = h_.l;
      
      t_ = HeadSequence;
      h_.hs = new CHeadSequence(h, new nvar(List, hl));
      break;
    }
    case Function:
      switch(x.t_){
        case Rational:
          delete h_.f;
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          break;
        case Real:
          delete h_.f;
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          break;
        case Symbol:
          delete h_.f;
          t_ = Symbol;
          h_.s = new nstr(*x.h_.s);
          break;
        case Binary:
          delete h_.f;
          t_ = Binary;
          h_.s = new nstr(*x.h_.s);
          break;
        case String:
          delete h_.f;
          t_ = String;
          h_.s = new nstr(*x.h_.s);
          break;
        case LocalObject:
          delete h_.f;
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          break;
        case SharedObject:
          delete h_.f;
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          break;
        case Function:
          h_.f->f = x.h_.f->f;
          h_.f->v = x.h_.f->v;
          break;
        case HeadSequence:
          setHead(*x.h_.hs->h);
          break;
        case HeadMap:
          setHead(*x.h_.hm->h);
          break;
        case HeadSequenceMap:
          setHead(*x.h_.hsm->h);
          break;
        case Reference:
          setHead(*x.h_.ref->v);
          break;
        case Pointer:
          setHead(*x.h_.vp);
          break;
        default:
          delete h_.f;
          t_ = x.t_;
          h_ = x.h_;
          break;
      }
    case HeadSequence:
      h_.hs->h->setHead(x);
      break;
    case Map:{
      nvar* h = new nvar;
      h->setHead(x);
      
      Head hm;
      hm.m = h_.m;
      
      t_ = HeadMap;
      h_.hm = new CHeadMap(h, new nvar(Map, hm));
      break;
    }
    case Multimap:{
      nvar* h = new nvar;
      h->setHead(x);
      
      Head hm;
      hm.mm = h_.mm;
      
      t_ = HeadMap;
      h_.hm = new CHeadMap(h, new nvar(Multimap, hm));
      break;
    }
    case HeadMap:
      h_.hm->h->setHead(x);
      break;
    case SequenceMap:{
      nvar* h = new nvar;
      h->setHead(x);
      
      t_ = HeadSequenceMap;
      h_.hsm = new CHeadSequenceMap(h, h_.sm->s, h_.sm->m);
      break;
    }
    case HeadSequenceMap:
      h_.hsm->h->setHead(x);
      break;
    case Pointer:
      h_.vp->setHead(x);
      break;
    case Reference:
      h_.ref->v->setHead(x);
      break;
    default:
      switch(x.t_){
        case Rational:
          t_ = Rational;
          h_.r = new nrat(*x.h_.r);
          break;
        case Real:
          t_ = Real;
          h_.x = new nreal(*x.h_.x);
          break;
        case Symbol:
          t_ = Symbol;
          h_.s = new nstr(*x.h_.s);
          break;
        case Binary:
          t_ = Binary;
          h_.s = new nstr(*x.h_.s);
          break;
        case String:
          t_ = String;
          h_.s = new nstr(*x.h_.s);
          break;
        case LocalObject:
          t_ = LocalObject;
          h_.o = x.h_.o->clone();
          break;
        case SharedObject:
          t_ = SharedObject;
          h_.o = x.h_.o;
          h_.o->ref();
          break;
        case Function:
          t_ = Function;
          h_.f = x.h_.f->clone();
          break;
        case HeadSequence:
          setHead(*x.h_.hs->h);
          break;
        case HeadMap:
          setHead(*x.h_.hm->h);
          break;
        case HeadSequenceMap:
          setHead(*x.h_.hsm->h);
          break;
        case Reference:
          setHead(*x.h_.ref->v);
          break;
        case Pointer:
          setHead(*x.h_.vp);
          break;
        default:
          t_ = x.t_;
          h_ = x.h_;
          break;
      }
  }
}

void nvar::merge(const nvar& x){
  switch(t_){
    case Map:
      switch(x.t_){
        case Map:
          h_.m->merge(*x.h_.m);
          break;
        case Multimap:
          touchMultimap();
          h_.mm->merge(*x.h_.mm);
          break;
        case Function:
          if(x.h_.f->m){
            h_.m->merge(*x.h_.f->m);
          }
          break;
        case HeadMap:
          merge(*x.h_.hm->m);
          break;
        case HeadSequenceMap:
          merge(*x.h_.hsm->m);
          break;
        case Reference:
          merge(*x.h_.ref->v);
          break;
        case Pointer:
          merge(*x.h_.vp);
          break;
        default:
          break;
      }
      break;
    case Multimap:
      switch(x.t_){
        case Map:
          h_.mm->insert(x.h_.m->begin(), x.h_.m->end());
          break;
        case Multimap:
          h_.mm->merge(*x.h_.mm);
          break;
        case Function:
          if(x.h_.f->m){
            h_.mm->insert(x.h_.f->m->begin(), x.h_.f->m->end());
          }
          break;
        case HeadMap:
          merge(*x.h_.hm->m);
          break;
        case HeadSequenceMap:
          merge(*x.h_.hsm->m);
          break;
        case Reference:
          merge(*x.h_.ref->v);
          break;
        case Pointer:
          merge(*x.h_.vp);
          break;
        default:
          break;
      }
      break;
    case Function:
      switch(x.t_){
        case Map:
          if(h_.f->m){
            h_.f->m->merge(*x.h_.m);
          }
          else{
            h_.f->m = new nmap(*x.h_.m);
          }
          break;
        case Multimap:
          NERROR("right operand is invalid");
        case Function:
          if(h_.f->m){
            h_.f->m->merge(*x.h_.f->m);
          }
          else{
            h_.f->m = new nmap(*x.h_.f->m);
          }
          break;
        case HeadMap:
          merge(*x.h_.hm->m);
          break;
        case HeadSequenceMap:
          merge(*x.h_.hsm->m);
          break;
        case Reference:
          merge(*x.h_.ref->v);
          break;
        case Pointer:
          merge(*x.h_.vp);
          break;
        default:
          break;
      }
      break;
    case HeadMap:
      h_.hm->m->merge(x);
      break;
    case SequenceMap:
      h_.sm->m->merge(x);
      break;
    case HeadSequenceMap:
      h_.hsm->m->merge(x);
      break;
    case Reference:
      h_.ref->v->merge(x);
      break;
    case Pointer:
      h_.vp->merge(x);
      break;
    default:
      switch(x.t_){
        case Map:
        case Multimap:
        case Function:
        case HeadMap:
        case HeadSequenceMap:
          touchMap();
          merge(x);
          break;
        case Reference:
          merge(*x.h_.ref->v);
          break;
        case Pointer:
          merge(*x.h_.vp);
          break;
        default:
          break;
      }
  }
}

void nvar::outerMerge(const nvar& x){
  switch(t_){
    case Map:
      switch(x.t_){
        case Map:
          h_.m->outerMerge(*x.h_.m);
          break;
        case Multimap:
          touchMultimap();
          h_.mm->merge(*x.h_.mm);
          break;
        case Function:
          if(x.h_.f->m){
            h_.m->outerMerge(*x.h_.f->m);
          }
          break;
        case HeadMap:
          outerMerge(*x.h_.hm->m);
          break;
        case HeadSequenceMap:
          outerMerge(*x.h_.hsm->m);
          break;
        case Reference:
          outerMerge(*x.h_.ref->v);
          break;
        case Pointer:
          outerMerge(*x.h_.vp);
          break;
        default:
          break;
      }
      break;
    case Multimap:
      switch(x.t_){
        case Map:
          h_.mm->insert(x.h_.m->begin(), x.h_.m->end());
          break;
        case Multimap:
          h_.mm->merge(*x.h_.mm);
          break;
        case Function:
          if(x.h_.f->m){
            h_.mm->insert(x.h_.f->m->begin(), x.h_.f->m->end());
          }
          break;
        case HeadMap:
          outerMerge(*x.h_.hm->m);
          break;
        case HeadSequenceMap:
          outerMerge(*x.h_.hsm->m);
          break;
        case Reference:
          outerMerge(*x.h_.ref->v);
          break;
        case Pointer:
          outerMerge(*x.h_.vp);
          break;
        default:
          break;
      }
      break;
    case Function:
      switch(x.t_){
        case Map:
          if(h_.f->m){
            h_.f->m->outerMerge(*x.h_.m);
          }
          else{
            h_.f->m = new nmap(*x.h_.m);
          }
          break;
        case Multimap:
          NERROR("right operand is invalid");
        case Function:
          if(h_.f->m){
            h_.f->m->outerMerge(*x.h_.f->m);
          }
          else{
            h_.f->m = new nmap(*x.h_.f->m);
          }
          break;
        case HeadMap:
          outerMerge(*x.h_.hm->m);
          break;
        case HeadSequenceMap:
          outerMerge(*x.h_.hsm->m);
          break;
        case Reference:
          outerMerge(*x.h_.ref->v);
          break;
        case Pointer:
          outerMerge(*x.h_.vp);
          break;
        default:
          break;
      }
      break;
    case HeadMap:
      h_.hm->m->outerMerge(x);
      break;
    case SequenceMap:
      h_.sm->m->outerMerge(x);
      break;
    case HeadSequenceMap:
      h_.hsm->m->outerMerge(x);
      break;
    case Reference:
      h_.ref->v->outerMerge(x);
      break;
    case Pointer:
      h_.vp->outerMerge(x);
      break;
    default:
      switch(x.t_){
        case Map:
        case Multimap:
        case Function:
        case HeadMap:
        case HeadSequenceMap:
          touchMap();
          outerMerge(x);
          break;
        case Reference:
          outerMerge(*x.h_.ref->v);
          break;
        case Pointer:
          outerMerge(*x.h_.vp);
          break;
        default:
          break;
      }
  }
}

nvar nvar::fromStr(const nstr& str){
  NMLParser parser;

  nvar ret = parser.parse(str + ";");

  if(ret.some()){
    return ret;
  }
  
  NERROR("failed to parse: " + str);
}

char* nvar::pack(uint32_t& size, bool compress) const{
  unsigned int psize = _packBlockSize;
  char* pbuf = (char*)malloc(psize);
  
  uint32_t pos = 0;
  pbuf = pack_(pbuf, psize, pos);
  
  if(compress){
    unsigned int csize = psize*2;
    char* cbuf = (char*)malloc(csize);
    
    csize = zlib_compress_(pbuf, cbuf, psize, csize);
    free(pbuf);
    size = csize;
    
    return cbuf;
  }
  
  size = pos;
  return pbuf;
}

char* nvar::pack_(char* buf, uint32_t& size, uint32_t& pos) const{
  if(size - pos < _packBlockSize){
    size += _packBlockSize;
    buf = (char*)realloc(buf, size);
  }
  
  switch(t_){
    case None:
      buf[pos++] = None;
      break;
    case Undefined:
      buf[pos++] = Undefined;
      break;
    case False:
      buf[pos++] = False;
      break;
    case True:
      buf[pos++] = True;
      break;
    case Integer:{
      size_t bytes = intBytes(h_.i);

      switch(bytes){
        case 8:
          buf[pos++] = Integer;
          memcpy(buf + pos, &h_.i, 8);
          pos += 8;
          break;
        case 4:{
          buf[pos++] = PackInt32;
          int32_t j = h_.i;
          memcpy(buf + pos, &j, 4);
          pos += 4;
          break;
        }
        case 2:{
          buf[pos++] = PackInt16;
          int16_t j = h_.i;
          memcpy(buf + pos, &j, 2);
          pos += 2;
          break;
        }
        case 1:{
          int8_t j = h_.i;
          
          switch(j){
            case 0:
              buf[pos++] = Pack0;
              break;
            case 1:
              buf[pos++] = Pack1;
              break;
            case 2:
              buf[pos++] = Pack2;
              break;
            case 3:
              buf[pos++] = Pack3;
              break;
            case 4:
              buf[pos++] = Pack4;
              break;
            case 5:
              buf[pos++] = Pack5;
              break;
            case 6:
              buf[pos++] = Pack6;
              break;
            case 7:
              buf[pos++] = Pack7;
              break;
            case 8:
              buf[pos++] = Pack8;
              break;
            case 9:
              buf[pos++] = Pack9;
              break;
            case 10:
              buf[pos++] = Pack10;
              break;
            case 11:
              buf[pos++] = Pack11;
              break;
            case 12:
              buf[pos++] = Pack12;
              break;
            case 13:
              buf[pos++] = Pack13;
              break;
            case 14:
              buf[pos++] = Pack14;
              break;
            case 15:
              buf[pos++] = Pack15;
              break;
            case 16:
              buf[pos++] = Pack16;
              break;
            case 17:
              buf[pos++] = Pack17;
              break;
            case 18:
              buf[pos++] = Pack18;
              break;
            case 19:
              buf[pos++] = Pack19;
              break;
            case 20:
              buf[pos++] = Pack20;
              break;
            case 21:
              buf[pos++] = Pack21;
              break;
            case 22:
              buf[pos++] = Pack22;
              break;
            case 23:
              buf[pos++] = Pack23;
              break;
            case 24:
              buf[pos++] = Pack24;
              break;
            case 25:
              buf[pos++] = Pack25;
              break;
            case 26:
              buf[pos++] = Pack26;
              break;
            case 27:
              buf[pos++] = Pack27;
              break;
            case 28:
              buf[pos++] = Pack28;
              break;
            case 29:
              buf[pos++] = Pack29;
              break;
            case 30:
              buf[pos++] = Pack30;
              break;
            case 31:
              buf[pos++] = Pack31;
              break;
            case 32:
              buf[pos++] = Pack32;
              break;
            case 33:
              buf[pos++] = Pack33;
              break;
            case 34:
              buf[pos++] = Pack34;
              break;
            case 35:
              buf[pos++] = Pack35;
              break;
            case 36:
              buf[pos++] = Pack36;
              break;
            case 37:
              buf[pos++] = Pack37;
              break;
            case 38:
              buf[pos++] = Pack38;
              break;
            case 39:
              buf[pos++] = Pack39;
              break;
            case 40:
              buf[pos++] = Pack40;
              break;
            case 41:
              buf[pos++] = Pack41;
              break;
            case 42:
              buf[pos++] = Pack42;
              break;
            case 43:
              buf[pos++] = Pack43;
              break;
            case 44:
              buf[pos++] = Pack44;
              break;
            case 45:
              buf[pos++] = Pack45;
              break;
            case 46:
              buf[pos++] = Pack46;
              break;
            case 47:
              buf[pos++] = Pack47;
              break;
            case 48:
              buf[pos++] = Pack48;
              break;
            case 49:
              buf[pos++] = Pack49;
              break;
            case 50:
              buf[pos++] = Pack50;
              break;
            default:
              buf[pos++] = PackInt8;
              buf[pos++] = j;
              break;
          }
          break;
        }
      }
      break;
    }
    case Rational:{
      buf[pos++] = Rational;
      
      const nrat& r = *h_.r;
      int64_t n = r.numerator();
      int64_t d = r.denominator();
      memcpy(buf + pos, &n, 8);
      pos += 8;
      memcpy(buf + pos, &d, 8);
      pos += 8;
      break;
    }
    case Float:{
      float f = h_.d;
      double d = f;
      if(d == h_.d){
        buf[pos++] = PackFloat32;
        memcpy(buf + pos, &f, 4);
        pos += 4;
      }
      else{
        buf[pos++] = Float;
        memcpy(buf + pos, &h_.d, 8);
        pos += 8;
      }
      break;
    }
    case Real:{
      buf[pos++] = Real;
      const nreal& x = *h_.x;
      nstr s = x.toStr();
      uint16_t len = s.length();
      
      if(size - pos < len){
        size += len + _packBlockSize;
        buf = (char*)realloc(buf, size);
      }
      
      memcpy(buf + pos, &len, 2);
      pos += 2;
      
      memcpy(buf + pos, s.c_str(), len);
      pos += len;
      break;
    }
    case Symbol:{
      const nstr& sbuf = *h_.s;
      uint32_t len = sbuf.length();
      
      if(len <= 255){
        buf[pos++] = Symbol;
        buf[pos++] = len;
      }
      else{
        buf[pos++] = PackLongSymbol;
        memcpy(buf + pos, &len, 4);
        ++pos;
      }
      
      if(size - pos < len){
        size += len + _packBlockSize;
        buf = (char*)realloc(buf, size);
      }
      
      sbuf.copy(buf + pos, len, 0);
      pos += len;
      break;
    }
    case String:
    case StringPointer:{
      const nstr& sbuf = *h_.s;
      uint32_t len = sbuf.length();
      
      if(len <= 255){
        buf[pos++] = PackShortString;
        buf[pos++] = len;
      }
      else if(len <= 65535){
        buf[pos++] = String;
        uint16_t plen = len;
        memcpy(buf + pos, &plen, 2);
        pos += 2;
      }
      else{
        Type t = PackLongString;
        buf[pos++] = t;
        memcpy(buf + pos, &len, 4);
        pos += 4;
      }
      
      if(size - pos < len){
        size += len + _packBlockSize;
        buf = (char*)realloc(buf, size);
      }
      
      sbuf.copy(buf + pos, len, 0);
      pos += len;
      break; 
    }
    case Binary:{
      const nstr& sbuf = *h_.s;
      uint32_t len = sbuf.length();
      buf[pos++] = Binary;
      
      memcpy(buf + pos, &len, 4);
      pos += 4;
      
      if(size - pos < len){
        size += len + _packBlockSize;
        buf = (char*)realloc(buf, size);
      }
      
      sbuf.copy(buf + pos, len, 0);
      pos += len;
      break;
    }
    case RawPointer:
      NERROR("attempt to pack pointer");
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      NERROR("attempt to pack object");
    case Vector:{
      const nvec& v = *h_.v;
      
      uint32_t len = v.size();
      if(len <= 255){
        buf[pos++] = PackShortVector;
        buf[pos++] = len;
      }
      else if(len <= 65535){
        buf[pos++] = Vector;
        uint16_t plen = len;
        memcpy(buf + pos, &plen, 2);
        pos += 2;
      }
      else{
        buf[pos++] = PackLongVector;
        memcpy(buf + pos, &len, 4);
        pos += 4;
      }
      
      for(size_t i = 0; i < len; ++i){
        buf = v[i].pack_(buf, size, pos);
      }
      
      break;
    }
    case List:{
      const nlist& l = *h_.l;
      
      uint32_t len = l.size();
      if(len <= 255){
        buf[pos++] = PackShortList;
        buf[pos++] = len;
      }
      else if(len <= 65535){
        buf[pos++] = List;
        uint16_t plen = len;
        memcpy(buf + pos, &plen, 2);
        pos += 2;
      }
      else{
        buf[pos++] = PackLongList;
        memcpy(buf + pos, &len, 4);
        pos += 4;
      }
      
      for(auto& itr : l){
        buf = itr.pack_(buf, size, pos);
      }
      break;
    }
    case Function:{
      const nvec& v = h_.f->v;
      
      const nstr& sbuf = h_.f->f;
      uint32_t len = sbuf.length();
      uint32_t n = v.size();
      uint32_t m = h_.f->m ? h_.f->m->size() : 0;
      bool isShort;
      
      if(len <= 255 && n <= 255 && m <= 255){
        isShort = true;
        buf[pos++] = Function;
        buf[pos++] = len;
        
        if(size - pos < len){
          size += len + _packBlockSize;
          buf = (char*)realloc(buf, size);
        }
        
        sbuf.copy(buf + pos, len, 0);
        pos += len;
        buf[pos++] = n;
      }
      else{
        isShort = false;
        buf[pos++] = PackLongFunction;
        memcpy(buf + pos, &len, 4);
        pos += 4;
        
        if(size - pos < len){
          size += len + _packBlockSize;
          buf = (char*)realloc(buf, size);
        }
        
        sbuf.copy(buf + pos, len, 0);
        pos += len;
        memcpy(buf + pos, &n, 4);
        pos += 4;
      }
      
      for(size_t i = 0; i < n; ++i){
        buf = v[i].pack_(buf, size, pos);
      }
      
      if(isShort){
        if(m > 0){
          buf[pos++] = m;

          for(auto& itr : *h_.f->m){
            buf = itr.first.pack_(buf, size, pos);
            buf = itr.second.pack_(buf, size, pos);
          }
        }
        else{
          buf[pos++] = 0;
        }
      }  
      else if(m > 0){
        uint32_t mlen = m;
        memcpy(buf + pos, &mlen, 4);
        pos += 4;
        for(auto& itr : *h_.f->m){
          buf = itr.first.pack_(buf, size, pos);
          buf = itr.second.pack_(buf, size, pos);
        }
      }
      else{
        uint32_t mlen = 0;
        memcpy(buf + pos, &mlen, 4);
        pos += 4;
      }
      
      break;
    }
    case HeadSequence:{
      buf[pos++] = HeadSequence;
      nvar h = head();
      buf = h.pack_(buf, size, pos);
      buf = h_.hs->s->pack_(buf, size, pos);
      break;
    }
    case Map:{
      uint32_t len = h_.m->size();
      if(len <= 255){
        buf[pos++] = PackShortMap;
        buf[pos++] = len;
      }
      else if(len <= 65535){
        buf[pos++] = Map;
        uint16_t plen = len;
        memcpy(buf + pos, &plen, 2);
        pos += 2;
      }
      else{
        buf[pos++] = PackLongMap;
        memcpy(buf + pos, &len, 4);
        pos += 4;
      }
      
      for(auto& itr : *h_.m){
        buf = itr.first.pack_(buf, size, pos);
        buf = itr.second.pack_(buf, size, pos);
      }
      break;    
    }
    case Multimap:{
      uint32_t len = h_.mm->size();
      if(len <= 255){
        buf[pos++] = PackShortMultimap;
        buf[pos++] = len;
      }
      else if(len <= 65535){
        buf[pos++] = Multimap;
        uint16_t plen = len;
        memcpy(buf + pos, &plen, 2);
        pos += 2;
      }
      else{
        buf[pos++] = PackLongMultimap;
        memcpy(buf + pos, &len, 4);
        pos += 4;
      }
      
      for(auto& itr : *h_.mm){
        buf = itr.first.pack_(buf, size, pos);
        buf = itr.second.pack_(buf, size, pos);
      }
      break;    
    }
    case HeadMap:{
      buf[pos++] = HeadMap;
      nvar h = head();
      buf = h.pack_(buf, size, pos);
      buf = h_.hm->m->pack_(buf, size, pos);
      break;
    }
    case SequenceMap:{
      buf[pos++] = SequenceMap;
      buf = h_.sm->s->pack_(buf, size, pos);
      buf = h_.sm->m->pack_(buf, size, pos);
      break;
    }
    case HeadSequenceMap:{
      buf[pos++] = HeadSequenceMap;
      nvar h = head();
      buf = h.pack_(buf, size, pos);
      buf = h_.sm->s->pack_(buf, size, pos);
      buf = h_.sm->m->pack_(buf, size, pos);
      break;
    }
    case Reference:
      buf[pos++] = Reference;
      buf = h_.ref->v->pack_(buf, size, pos);
      break;
    case Pointer:
      buf = h_.vp->pack_(buf, size, pos);
      break;
    default:
      assert(false);
  }
  
  return buf;
}

void nvar::unpack(char* buf, uint32_t size, bool compressed){
  assert(t_ == Undefined);
  
  uint32_t pos = 0;
  
  if(compressed){
    unsigned int psize = size*2;
    char* pbuf = (char*)malloc(psize);
    
    pbuf = zlib_decompress_(buf, size, pbuf, &psize, true);
    unpack_(pbuf, pos);
    free(pbuf);
  }
  else{
    unpack_(buf, pos);
  }
}

void nvar::unpack_(char* buf, uint32_t& pos){
  Type t = buf[pos++];
  
  switch(t){
    case None:
      t_ = None;
      break;
    case Undefined:
      t_ = Undefined;
      break;
    case False:
      t_ = False;
      break;
    case True:
      t_ = True;
      break;
    case Pack0:
      t_ = Integer;
      h_.i = 0;
      break;
    case Pack1:
      t_ = Integer;
      h_.i = 1;
      break;
    case Pack2:
      t_ = Integer;
      h_.i = 2;
      break;
    case Pack3:
      t_ = Integer;
      h_.i = 3;
      break;
    case Pack4:
      t_ = Integer;
      h_.i = 4;
      break;
    case Pack5:
      t_ = Integer;
      h_.i = 5;
      break;
    case Pack6:
      t_ = Integer;
      h_.i = 6;
      break;
    case Pack7:
      t_ = Integer;
      h_.i = 7;
      break;
    case Pack8:
      t_ = Integer;
      h_.i = 8;
      break;
    case Pack9:
      t_ = Integer;
      h_.i = 9;
      break;
    case Pack10:
      t_ = Integer;
      h_.i = 10;
      break;
    case Pack11:
      t_ = Integer;
      h_.i = 11;
      break;
    case Pack12:
      t_ = Integer;
      h_.i = 12;
      break;
    case Pack13:
      t_ = Integer;
      h_.i = 13;
      break;
    case Pack14:
      t_ = Integer;
      h_.i = 14;
      break;
    case Pack15:
      t_ = Integer;
      h_.i = 15;
      break;
    case Pack16:
      t_ = Integer;
      h_.i = 16;
      break;
    case Pack17:
      t_ = Integer;
      h_.i = 17;
      break;
    case Pack18:
      t_ = Integer;
      h_.i = 18;
      break;
    case Pack19:
      t_ = Integer;
      h_.i = 19;
      break;
    case Pack20:
      t_ = Integer;
      h_.i = 20;
      break;
    case Pack21:
      t_ = Integer;
      h_.i = 21;
      break;
    case Pack22:
      t_ = Integer;
      h_.i = 22;
      break;
    case Pack23:
      t_ = Integer;
      h_.i = 23;
      break;
    case Pack24:
      t_ = Integer;
      h_.i = 24;
      break;
    case Pack25:
      t_ = Integer;
      h_.i = 25;
      break;
    case Pack26:
      t_ = Integer;
      h_.i = 26;
      break;
    case Pack27:
      t_ = Integer;
      h_.i = 27;
      break;
    case Pack28:
      t_ = Integer;
      h_.i = 28;
      break;
    case Pack29:
      t_ = Integer;
      h_.i = 29;
      break;
    case Pack30:
      t_ = Integer;
      h_.i = 30;
      break;
    case Pack31:
      t_ = Integer;
      h_.i = 31;
      break;
    case Pack32:
      t_ = Integer;
      h_.i = 32;
      break;
    case Pack33:
      t_ = Integer;
      h_.i = 33;
      break;
    case Pack34:
      t_ = Integer;
      h_.i = 34;
      break;
    case Pack35:
      t_ = Integer;
      h_.i = 35;
      break;
    case Pack36:
      t_ = Integer;
      h_.i = 36;
      break;
    case Pack37:
      t_ = Integer;
      h_.i = 37;
      break;
    case Pack38:
      t_ = Integer;
      h_.i = 38;
      break;
    case Pack39:
      t_ = Integer;
      h_.i = 39;
      break;
    case Pack40:
      t_ = Integer;
      h_.i = 40;
      break;
    case Pack41:
      t_ = Integer;
      h_.i = 41;
      break;
    case Pack42:
      t_ = Integer;
      h_.i = 42;
      break;
    case Pack43:
      t_ = Integer;
      h_.i = 43;
      break;
    case Pack44:
      t_ = Integer;
      h_.i = 44;
      break;
    case Pack45:
      t_ = Integer;
      h_.i = 45;
      break;
    case Pack46:
      t_ = Integer;
      h_.i = 46;
      break;
    case Pack47:
      t_ = Integer;
      h_.i = 47;
      break;
    case Pack48:
      t_ = Integer;
      h_.i = 48;
      break;
    case Pack49:
      t_ = Integer;
      h_.i = 49;
      break;
    case Pack50:
      t_ = Integer;
      h_.i = 50;
      break;
    case PackInt8:{
      t_ = Integer;
      h_.i = *(uint8_t*)(buf + pos);
      ++pos;
      break;
    }
    case PackInt16:{
      int16_t i;
      memcpy(&i, buf + pos, 2);
      pos += 2;
      t_ = Integer;
      h_.i = i;
      break;
    }
    case PackInt32:{
      uint32_t i;
      memcpy(&i, buf + pos, 4);
      pos += 4;
      t_ = Integer;
      h_.i = i;
      break;
    }
    case Integer:{
      int64_t i;
      memcpy(&i, buf + pos, 8);
      pos += 8;
      t_ = Integer;
      h_.i = i;
      break;
    }
    case Rational:{
      int64_t n;
      memcpy(&n, buf + pos, 8);
      pos += 8;
      
      int64_t d;
      memcpy(&d, buf + pos, 8);
      pos += 8;
      
      t_ = Rational;
      h_.r = new nrat(n, d);
      break;
    }
    case PackFloat32:{
      float f;
      memcpy(&f, buf + pos, 4);
      pos += 4;
      t_ = Float;
      h_.d = f;
      break;
    }
    case Float:{
      double d;
      memcpy(&d, buf + pos, 8);
      pos += 8;
      t_ = Float;
      h_.d = d;
      break;
    }
    case Real:{
      uint16_t len;
      memcpy(&len, buf + pos, 2);
      pos += 2;
      nstr s;
      s.insert(0, buf + pos, len);
      pos += len;
      t_ = Real;
      h_.x = new nreal(s.c_str());
      break;
    }
    case Symbol:{
      uint8_t len = *(uint8_t*)(buf + pos);
      ++pos;
      t_ = Symbol;
      h_.s = new nstr;
      h_.s->insert(0, buf + pos, len);
      pos += len;
      break;
    }
    case PackLongSymbol:{
      uint32_t len;
      memcpy(&len, buf + pos, 4);
      pos += 4;
      t_ = Symbol;
      h_.s = new nstr;
      h_.s->insert(0, buf + pos, len);
      pos += len;
      break;
    }
    case PackShortString:{
      uint8_t len = *(uint8_t*)(buf + pos);
      ++pos;
      t_ = String;
      h_.s = new nstr;
      h_.s->insert(0, buf + pos, len);
      pos += len;
      break;
    }
    case String:{
      uint16_t len;
      memcpy(&len, buf + pos, 2);
      pos += 2;
      t_ = String;
      h_.s = new nstr;
      h_.s->insert(0, buf + pos, len);
      pos += len;
      break;
    }
    case PackLongString:{
      uint32_t len;
      memcpy(&len, buf + pos, 4);
      pos += 4;
      t_ = String;
      h_.s = new nstr;
      h_.s->insert(0, buf + pos, len);
      pos += len;
      break;
    }
    case Binary:{
      uint32_t len;
      memcpy(&len, buf + pos, 4);
      pos += 4;
      t_ = Binary;
      h_.s = new nstr;
      h_.s->insert(0, buf + pos, len);
      pos += len;
      break;
    }
    case PackShortVector:{
      uint8_t len = *(uint8_t*)(buf + pos);
      ++pos;
      t_ = Vector;
      h_.v = new nvec(len);
      nvec& v = *h_.v;
      for(size_t i = 0; i < len; ++i){
        v[i].unpack_(buf, pos);
      }
      break;
    }
    case Vector:{
      uint16_t len;
      memcpy(&len, buf + pos, 2);
      pos += 2;
      t_ = Vector;
      h_.v = new nvec(len);
      nvec& v = *h_.v;
      for(size_t i = 0; i < len; ++i){
        v[i].unpack_(buf, pos);
      }
      break;
    }
    case PackLongVector:{
      uint32_t len;
      memcpy(&len, buf + pos, 4);
      pos += 4;
      t_ = Vector;
      h_.v = new nvec(len);
      nvec& v = *h_.v;
      for(size_t i = 0; i < len; ++i){
        v[i].unpack_(buf, pos);
      }
      break;
    }
    case PackShortList:{
      uint8_t len = *(uint8_t*)(buf + pos);
      ++pos;
      t_ = List;
      h_.l = new nlist(len);
      nlist& l = *h_.l;
      for(auto& itr : l){
        itr.unpack_(buf, pos);
      }
      break;
    }
    case List:{
      uint16_t len;
      memcpy(&len, buf + pos, 2);
      pos += 2;
      t_ = List;
      h_.l = new nlist(len);
      nlist& l = *h_.l;
      for(auto& itr : l){
        itr.unpack_(buf, pos);
      }
      break;
    }
    case PackLongList:{
      uint32_t len;
      memcpy(&len, buf + pos, 4);
      pos += 4;
      t_ = List;
      h_.l = new nlist(len);
      nlist& l = *h_.l;
      for(auto& itr : l){
        itr.unpack_(buf, pos);
      }
      break;
    }
    case Function:{
      uint8_t slen = *(uint8_t*)(buf + pos);
      ++pos;
      
      nstr f;
      f.insert(0, buf + pos, slen);
      pos += slen;
      
      uint8_t n = *(uint8_t*)(buf + pos);
      ++pos;
      
      t_ = Function;
      h_.f = new CFunction(move(f), n);
      nvec& v = h_.f->v;
      
      for(size_t i = 0; i < n; ++i){
        v[i].unpack_(buf, pos);
      }
      
      uint8_t mlen = *(uint8_t*)(buf + pos);
      ++pos;
      
      if(mlen > 0){
        nmap* m = new nmap;
        h_.f->m = m;
        
        for(size_t i = 0; i < mlen; ++i){
          nvar k;
          k.unpack_(buf, pos);
          
          nvar v;
          v.unpack_(buf, pos);
          
          m->emplace(move(k), move(v));
        }      
      }

      break;
    }
    case PackLongFunction:{
      uint32_t slen;
      memcpy(&slen, buf + pos, 4);
      pos += 4;
      
      nstr f;
      f.insert(0, buf + pos, slen);
      pos += slen;
      
      uint32_t n;
      memcpy(&n, buf + pos, 4);
      pos += 4;
      
      t_ = Function;
      h_.f = new CFunction(move(f), n);
      nvec& v = h_.f->v;
      
      for(size_t i = 0; i < n; ++i){
        v[i].unpack_(buf, pos);
      }
      
      uint32_t mlen;
      memcpy(&mlen, buf + pos, 4);
      pos += 4;
      
      if(mlen > 0){
        nmap* m = new nmap;
        h_.f->m = m;
        
        for(size_t i = 0; i < mlen; ++i){
          nvar k;
          k.unpack_(buf, pos);
          
          nvar v;
          v.unpack_(buf, pos);
          
          m->emplace(move(k), move(v));
        }      
      }
      
      break;
    }
    case HeadSequence:{
      nvar* h = new nvar;
      h->unpack_(buf, pos);
      
      nvar* s = new nvar;
      s->unpack_(buf, pos);
      
      t_ = HeadSequence;
      h_.hs = new CHeadSequence(h, s);
      break;
    }
    case PackShortMap:{
      uint8_t len = *(uint8_t*)(buf + pos);
      ++pos;
      
      t_ = Map;
      h_.m = new nmap;
      nmap& m = *h_.m;
      
      for(size_t i = 0; i < len; ++i){
        nvar k;
        k.unpack_(buf, pos);
        
        nvar v;
        v.unpack_(buf, pos);
        
        m.emplace(move(k), move(v));
      }
      break;
    }
    case Map:{
      uint16_t len;
      memcpy(&len, buf + pos, 2);
      pos += 2;
      
      t_ = Map;
      h_.m = new nmap;
      nmap& m = *h_.m;
      
      for(size_t i = 0; i < len; ++i){
        nvar k;
        k.unpack_(buf, pos);
        
        nvar v;
        v.unpack_(buf, pos);
        
        m.emplace(move(k), move(v));
      }
      break;
    }
    case PackLongMap:{
      uint32_t len;
      memcpy(&len, buf + pos, 4);
      pos += 4;
      
      t_ = Map;
      h_.m = new nmap;
      nmap& m = *h_.m;
      
      for(size_t i = 0; i < len; ++i){
        nvar k;
        k.unpack_(buf, pos);
        
        nvar v;
        v.unpack_(buf, pos);
        
        m.emplace(move(k), move(v));
      }
      break;
    }
    case PackShortMultimap:{
      uint8_t len = *(uint8_t*)(buf + pos);
      ++pos;
      
      t_ = Multimap;
      h_.mm = new nmmap;
      nmmap& mm = *h_.mm;
      
      for(size_t i = 0; i < len; ++i){
        nvar k;
        k.unpack_(buf, pos);
        
        nvar v;
        v.unpack_(buf, pos);
        
        mm.emplace(move(k), move(v));
      }
      break;
    }
    case Multimap:{
      uint16_t len;
      memcpy(&len, buf + pos, 2);
      pos += 2;
      
      t_ = Multimap;
      h_.mm = new nmmap;
      nmmap& mm = *h_.mm;
      
      for(size_t i = 0; i < len; ++i){
        nvar k;
        k.unpack_(buf, pos);
        
        nvar v;
        v.unpack_(buf, pos);
        
        mm.emplace(move(k), move(v));
      }
      break;
    }
    case PackLongMultimap:{
      uint32_t len;
      memcpy(&len, buf + pos, 4);
      pos += 4;
      
      t_ = Multimap;
      h_.mm = new nmmap;
      nmmap& mm = *h_.mm;
      
      for(size_t i = 0; i < len; ++i){
        nvar k;
        k.unpack_(buf, pos);
        
        nvar v;
        v.unpack_(buf, pos);
        
        mm.emplace(move(k), move(v));
      }
      break;
    }
    case HeadMap:{
      nvar* h = new nvar;
      h->unpack_(buf, pos);
      
      nvar* m = new nvar;
      m->unpack_(buf, pos);
      
      t_ = HeadMap;
      h_.hm = new CHeadMap(h, m);
      break;
    }
    case SequenceMap:{
      nvar* s = new nvar;
      s->unpack_(buf, pos);
      
      nvar* m = new nvar;
      m->unpack_(buf, pos);
      
      t_ = SequenceMap;
      h_.sm = new CSequenceMap(s, m);
      break;
    }
    case HeadSequenceMap:{
      nvar* h = new nvar;
      h->unpack_(buf, pos);
      
      nvar* s = new nvar;
      s->unpack_(buf, pos);
      
      nvar* m = new nvar;
      m->unpack_(buf, pos);
      
      t_ = HeadSequenceMap;
      h_.hsm = new CHeadSequenceMap(h, s, m);
      break;
    }
    case Reference:{
      t_ = Reference;
      nvar* v = new nvar;
      v->unpack_(buf, pos);
      h_.ref = new CReference(v);
      break;
    }
    default:
      NERROR("unpack error");
  }
}

void nvar::save(const nstr& path) const{
  nstr tempPath = NSys::tempFilePath();
  
  FILE* file = fopen(tempPath.c_str(), "wb");
  
  if(!file){
    NERROR("failed to create file: " + tempPath);
  }
  
  uint32_t len;
  char* buf = pack(len);
  uint32_t n = fwrite(buf, 1, len, file);
  
  fclose(file);
  free(buf);
  
  if(n != len){
    remove(tempPath.c_str());
    NERROR("failed to write file: " + tempPath);
  }
  
  if(!NSys::rename(tempPath, path)){
    remove(tempPath.c_str());
    NERROR("failed to move file into place: " + path);
  }
}

void nvar::open(const nstr& path){
  FILE* file = fopen(path.c_str(), "rb");
  
  if(!file){
    NERROR("failed to open file: " + path);
  }
  
  fseek(file, 0, SEEK_END);
  long n = ftell(file);
  
  if(n < 0){
    NERROR("[1] failed to read file: " + path);
  }
  
  uint32_t size = n;
  
  rewind(file);
  
  char* buf = (char*)malloc(size);
  
  n = fread(buf, 1, size, file);
  fclose(file);
  
  if(n < size){
    NERROR("[2] failed to read file: " + path);
  }
  
  unpack(buf, size);
  
  free(buf);
}

void nvar::sqrt(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Sqrt") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::sqrt(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Sqrt") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::sqrt(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::sqrt(h_.d);
      break;
    case Real:
      *h_.x = nreal::sqrt(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Sqrt") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].sqrt(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->sqrt(o);
      break;
    case HeadMap:
      h_.hm->h->sqrt(o);
      break;
    case SequenceMap:
      h_.sm->s->sqrt(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->sqrt(o);
      break;
    case Reference:
      h_.ref->v->sqrt(o);
      break;
    case Pointer:
      h_.vp->sqrt(o);
      break;
  }
}

void nvar::exp(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Exp") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::exp(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Exp") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::exp(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::exp(h_.d);
      break;
    case Real:
      *h_.x = nreal::exp(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Exp") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].exp(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->exp(o);
      break;
    case HeadMap:
      h_.hm->h->exp(o);
      break;
    case SequenceMap:
      h_.sm->s->exp(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->exp(o);
      break;
    case Reference:
      h_.ref->v->exp(o);
      break;
    case Pointer:
      h_.vp->exp(o);
      break;
  }
}

nvar nvar::pow(const nvar& x, NObject* o) const{
  switch(t_){
    case None:
    case Undefined:
      NERROR("left operand is undefined");
    case False:
    case True:
      NERROR("left operand is invalid");
    case Integer:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return std::pow(h_.i, x.h_.i);
        case Rational:
          if(o){
            return o->process(nfunc("Pow") << h_.i << *x.h_.r, NObject::Delegated);
          }
          
          return std::pow(h_.i, x.h_.r->toDouble());
        case Float:
          return std::pow(h_.i, x.h_.d);
        case Real:
          return nreal::pow(h_.i, *x.h_.x);
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Pow");
          h.f->v.push_back(h_.i);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case HeadSequence:
          return pow(*x.h_.hs->h, o);
        case HeadMap:
          return pow(*x.h_.hm->h, o);
        case HeadSequenceMap:
          return pow(*x.h_.hsm->h, o);
        case Reference:
          return pow(*x.h_.ref->v, o);
        case Pointer:
          return pow(*x.h_.vp, o);
        default:
          NERROR("invalid operands");
      }
    case Rational:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          if(o){
            return o->process(nfunc("Pow") << *h_.r << x.h_.i, NObject::Delegated);
          }
          
          return std::pow(h_.r->toDouble(), x.h_.i);
        case Rational:
          if(o){
            return o->process(nfunc("Pow") << *h_.r << *x.h_.r, NObject::Delegated);
          }
          
          return std::pow(h_.r->toDouble(), x.h_.r->toDouble());
        case Float:
          return std::pow(h_.r->toDouble(), x.h_.d);
        case Real:
          return nreal::pow(*h_.r, *x.h_.x);
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Pow");
          h.f->v.push_back(*h_.r);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case HeadSequence:
          return pow(*x.h_.hs->h, o);
        case HeadMap:
          return pow(*x.h_.hm->h, o);
        case HeadSequenceMap:
          return pow(*x.h_.hsm->h, o);
        case Reference:
          return pow(*x.h_.ref->v, o);
        case Pointer:
          return pow(*x.h_.vp, o);
        default:
          NERROR("invalid operands");
      } 
    case Float:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return std::pow(h_.d, x.h_.i);
        case Rational:
          return std::pow(h_.d, x.h_.r->toDouble());
        case Float:
          return std::pow(h_.d, x.h_.d);
        case Real:
          return std::pow(h_.d, x.h_.x->toDouble());
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Pow");
          h.f->v.push_back(h_.d);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case HeadSequence:
          return pow(*x.h_.hs->h, o);
        case HeadMap:
          return pow(*x.h_.hm->h, o);
        case HeadSequenceMap:
          return pow(*x.h_.hsm->h, o);
        case Reference:
          return pow(*x.h_.ref->v);
        case Pointer:
          return pow(*x.h_.vp, o);
        default:
          NERROR("invalid operands");
      }
    case Real:
      switch(x.t_){
        case None:
        case Undefined:
          NERROR("right operand is undefined");
        case Integer:
          return nreal::pow(*h_.x, x.h_.i); 
        case Rational:
          return nreal::pow(*h_.x, *x.h_.r); 
        case Float:
          return std::pow(h_.x->toDouble(), x.h_.d);
        case Real:
          return nreal::pow(*h_.x, *x.h_.x); 
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Pow");
          h.f->v.push_back(*h_.x);
          h.f->v.push_back(new nvar(x, Copy));
          return new nvar(Function, h);
        }
        case HeadSequence:
          return pow(*x.h_.hs->h, o);
        case HeadMap:
          return pow(*x.h_.hm->h, o);
        case HeadSequenceMap:
          return pow(*x.h_.hsm->h, o);
        case Reference:
          return pow(*x.h_.ref->v);
        case Pointer:
          return pow(*x.h_.vp, o);
        default:
          NERROR("invalid operands");
      }  
    case Symbol:
    case Function:{
      Head h;
      h.f = new CFunction("Pow");
      h.f->v.emplace_back(new nvar(*this, Copy));
      h.f->v.emplace_back(nvar(x, Copy));
      
      return new nvar(Function, h);
    }
    case HeadSequence:
      return h_.hs->h->pow(x, o);
    case HeadMap:
      return h_.hm->h->pow(x, o);
    case HeadSequenceMap:
      return h_.hsm->h->pow(x, o);
    case Reference:
      return h_.ref->v->pow(x);
    case Pointer:
      return h_.vp->pow(x);
    default:
      NERROR("left operand is invalid");
  }  
}

void nvar::abs(){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      h_.i = std::abs(h_.i);
      break;
    case Rational:
      if(*h_.r < 0){
        *h_.r = - *h_.r;
      }
      break;
    case Float:
      h_.d = std::abs(h_.d);
      break;
    case Real:
      *h_.x = nreal::abs(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Abs") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].abs();
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->abs();
      break;
    case HeadMap:
      h_.hm->h->abs();
      break;
    case SequenceMap:
      h_.sm->s->abs();
      break;
    case HeadSequenceMap:
      h_.hsm->s->abs();
      break;
    case Reference:
      h_.ref->v->abs();
      break;
    case Pointer:
      h_.vp->abs();
      break;
  }
}

void nvar::log10(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Log10") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::log10(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Log10") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::log10(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::log10(h_.d);
      break;
    case Real:
      *h_.x = nreal::log10(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Log10") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].log10(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->log10(o);
      break;
    case HeadMap:
      h_.hm->h->log10(o);
      break;
    case SequenceMap:
      h_.sm->s->log10(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->log10(o);
      break;
    case Reference:
      h_.ref->v->log10(o);
      break;
    case Pointer:
      h_.vp->log10(o);
      break;
  }
}

void nvar::log(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Log") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::log(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Log") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::log(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::log(h_.d);
      break;
    case Real:
      *h_.x = nreal::log(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Log") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].log(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->log(o);
      break;
    case HeadMap:
      h_.hm->h->log(o);
      break;
    case SequenceMap:
      h_.sm->s->log(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->log(o);
      break;
    case Reference:
      h_.ref->v->log(o);
      break;
    case Pointer:
      h_.vp->log(o);
      break;
  }
}

void nvar::cos(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Cos") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::cos(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Cos") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::cos(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::cos(h_.d);
      break;
    case Real:
      *h_.x = nreal::cos(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Cos") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].cos(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->cos(o);
      break;
    case HeadMap:
      h_.hm->h->cos(o);
      break;
    case SequenceMap:
      h_.sm->s->cos(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->cos(o);
      break;
    case Reference:
      h_.ref->v->cos(o);
      break;
    case Pointer:
      h_.vp->cos(o);
      break;
  }
}

void nvar::acos(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Acos") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::acos(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Acos") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::acos(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::acos(h_.d);
      break;
    case Real:
      *h_.x = nreal::acos(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Acos") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].acos(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->acos(o);
      break;
    case HeadMap:
      h_.hm->h->acos(o);
      break;
    case SequenceMap:
      h_.sm->s->acos(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->acos(o);
      break;
    case Reference:
      h_.ref->v->acos(o);
      break;
    case Pointer:
      h_.vp->acos(o);
      break;
  }
}

void nvar::cosh(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Cosh") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::cosh(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Cosh") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::cosh(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::cosh(h_.d);
      break;
    case Real:
      *h_.x = nreal::cosh(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Cosh") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].cosh(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->cosh(o);
      break;
    case HeadMap:
      h_.hm->h->cosh(o);
      break;
    case SequenceMap:
      h_.sm->s->cosh(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->cosh(o);
      break;
    case Reference:
      h_.ref->v->cosh(o);
      break;
    case Pointer:
      h_.vp->cosh(o);
      break;
  }
}

void nvar::sin(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Sin") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::sin(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Sin") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::sin(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::sin(h_.d);
      break;
    case Real:
      *h_.x = nreal::sin(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Sin") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].sin(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->sin(o);
      break;
    case HeadMap:
      h_.hm->h->sin(o);
      break;
    case SequenceMap:
      h_.sm->s->sin(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->sin(o);
      break;
    case Reference:
      h_.ref->v->sin(o);
      break;
    case Pointer:
      h_.vp->sin(o);
      break;
  }
}

void nvar::asin(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Asin") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::asin(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Asin") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::asin(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::asin(h_.d);
      break;
    case Real:
      *h_.x = nreal::asin(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Asin") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].asin(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->asin(o);
      break;
    case HeadMap:
      h_.hm->h->asin(o);
      break;
    case SequenceMap:
      h_.sm->s->asin(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->asin(o);
      break;
    case Reference:
      h_.ref->v->asin(o);
      break;
    case Pointer:
      h_.vp->asin(o);
      break;
  }
}

void nvar::sinh(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Sinh") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::sinh(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Sinh") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::sinh(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::sinh(h_.d);
      break;
    case Real:
      *h_.x = nreal::sinh(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Sinh") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].sinh(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->sinh(o);
      break;
    case HeadMap:
      h_.hm->h->sinh(o);
      break;
    case SequenceMap:
      h_.sm->s->sinh(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->sinh(o);
      break;
    case Reference:
      h_.ref->v->sinh(o);
      break;
    case Pointer:
      h_.vp->sinh(o);
      break;
  }
}

void nvar::tan(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Tan") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::tan(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Tan") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::tan(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::tan(h_.d);
      break;
    case Real:
      *h_.x = nreal::tan(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Tan") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].tan(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->tan(o);
      break;
    case HeadMap:
      h_.hm->h->tan(o);
      break;
    case SequenceMap:
      h_.sm->s->tan(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->tan(o);
      break;
    case Reference:
      h_.ref->v->tan(o);
      break;
    case Pointer:
      h_.vp->tan(o);
      break;
  }
}

void nvar::atan(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Atan") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::atan(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Atan") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::atan(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::atan(h_.d);
      break;
    case Real:
      *h_.x = nreal::atan(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Atan") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].atan(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->atan(o);
      break;
    case HeadMap:
      h_.hm->h->atan(o);
      break;
    case SequenceMap:
      h_.sm->s->atan(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->atan(o);
      break;
    case Reference:
      h_.ref->v->atan(o);
      break;
    case Pointer:
      h_.vp->atan(o);
      break;
  }
}

void nvar::tanh(NObject* o){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      if(o){
        *this = o->process(nfunc("Tanh") << *this, NObject::Delegated);
        break;
      }
      
      h_.d = h_.i;
      h_.d = std::tanh(h_.d);
      t_ = Float;
      break;
    case Rational:{
      if(o){
        *this = o->process(nfunc("Tanh") << *this, NObject::Delegated);
        break;
      }
      
      double d = h_.r->toDouble();
      delete h_.r;
      h_.d = std::tanh(d);
      t_ = Float;
      break;
    }
    case Float:
      h_.d = std::tanh(h_.d);
      break;
    case Real:
      *h_.x = nreal::tanh(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Tanh") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].tanh(o);
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->tanh(o);
      break;
    case HeadMap:
      h_.hm->h->tanh(o);
      break;
    case SequenceMap:
      h_.sm->s->tanh(o);
      break;
    case HeadSequenceMap:
      h_.hsm->s->tanh(o);
      break;
    case Reference:
      h_.ref->v->tanh(o);
      break;
    case Pointer:
      h_.vp->tanh(o);
      break;
  }
}

void nvar::floor(){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      break;
    case Rational:
      h_.i = h_.r->numerator() / h_.r->denominator();
      break;
    case Float:
      h_.d = std::floor(h_.d);
      break;
    case Real:
      *h_.x = nreal::floor(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Floor") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].floor();
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->floor();
      break;
    case HeadMap:
      h_.hm->h->floor();
      break;
    case SequenceMap:
      h_.sm->s->floor();
      break;
    case HeadSequenceMap:
      h_.hsm->s->floor();
      break;
    case Reference:
      h_.ref->v->floor();
      break;
    case Pointer:
      h_.vp->floor();
      break;
  }
}

void nvar::ceil(){
  switch(t_){
    case None:
    case Undefined:
      NERROR("operand is undefined");
    case False:
    case True:
      NERROR("operand is invalid");
    case Integer:
      break;
    case Rational:{
      int64_t n = h_.r->numerator();
      int64_t d = h_.r->denominator();
      
      h_.i = n / d + n % d > 0 ? 1 : 0;
      t_ = Integer;
      break;
    }
    case Float:
      h_.d = std::ceil(h_.d);
      break;
    case Real:
      *h_.x = nreal::ceil(*h_.x);
      break;
    case Symbol:
    case Function:
      *this = nfunc("Ceil") << move(*this);
      break;
    case Vector:{
      nvec& v = *h_.v;
      size_t size = v.size();
      for(size_t i = 0; i < size; ++i){
        v[i].ceil();
      }
      break;
    }
    case HeadSequence:
      h_.hs->s->ceil();
      break;
    case HeadMap:
      h_.hm->h->ceil();
      break;
    case SequenceMap:
      h_.sm->s->ceil();
      break;
    case HeadSequenceMap:
      h_.hsm->s->ceil();
      break;
    case Reference:
      h_.ref->v->ceil();
      break;
    case Pointer:
      h_.vp->ceil();
      break;
  }
}

nvar& nvar::unite(const nvar& x, bool outer){
  bool v = hasVector();

  touchList();
  touchMap();

  nvar xc = x;
  xc.touchList();
  xc.touchMap();

  uniteList(list(), xc.list(), outer);
  uniteMap(map(), xc.map(), outer);

  if(v){
    touchVector();
  }

  normalize();

  return *this;
}

nvar& nvar::intersect(const nvar& x, bool outer){
  bool v = hasVector();

  touchList();
  touchMap();

  nvar xc = x;
  xc.touchList();
  xc.touchMap();

  intersectList(list(), xc.list(), outer);
  intersectMap(map(), xc.map(), outer);

  if(v){
    touchVector();
  }

  normalize();

  return *this;
}

nvar& nvar::complement(const nvar& x){
  bool v = hasVector();

  touchList();
  touchMap();

  nvar xc = x;
  xc.touchList();
  xc.touchMap();

  complementList(list(), xc.list());
  complementMap(map(), xc.map());

  if(v){
    touchVector();
  }

  normalize();

  return *this;
}

void nvar::foldLeft(){
  if(!isFunction()){
    return;
  }
  
  while(size() > 2){
    nvar f = nfunc(str()) << move((*this)[0]) << move((*this)[1]);
    popFront();
    popFront();
    pushFront(f);
  }
}

void nvar::foldRight(){
  if(!isFunction()){
    return;
  }
  
  for(;;){
    size_t s = size();

    if(s <= 2){
      break;
    }
    
    nvar f = nfunc(str()) << move((*this)[s - 2]) << move((*this)[s - 1]);
    popBack();
    popBack();
    (*this) << move(f);
  }
}

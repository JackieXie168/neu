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

#include <neu/NJSONGenerator.h>

#include <neu/NObject.h>

using namespace std;
using namespace neu;

namespace neu{
  
  class NJSONGenerator_{
  public:
    NJSONGenerator_(NJSONGenerator* o)
    : o_(o){}
    
    ~NJSONGenerator_(){}
    
    void generate(ostream& ostr, const nvar& v){
      emitValue(ostr, v, "  ");
    }
    
    void emitKey(ostream& ostr, const nvar& k){
      ostr << "\"" << k.toString() << "\"";
    }
    
    template<class T>
    void emitSequence(ostream& ostr, const T& seq, const nstr& indent){
      ostr << "[";
      size_t size = seq.size();
      for(size_t i = 0; i < size; ++i){
        if(i > 0){
          ostr << ",";
        }
        
        emitValue(ostr, seq[i], indent);
      }
      ostr << "]";
    }
    
    void emitValue(ostream& ostr,
                   const nvar& n,
                   const nstr& indent){
      
      switch(n.fullType()){
        case nvar::None:
          ostr << "null";
          break;
        case nvar::Undefined:
          ostr << "\"@undef\"";
          break;
        case nvar::False:
          ostr << "false";
          break;
        case nvar::True:
          ostr << "true";
          break;
        case nvar::Rational:{
          const nrat& r = n.rat();
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::Rational) << "," << endl;
          
          ostr << indent << "\"@numerator\": " << r.numerator() << "," << endl;
          ostr << indent << "\"@denominator\": " << r.denominator() << endl;
          ostr << indent << "}";
          break;
        }
        case nvar::Real:{
          const nreal& r = n.real();
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::Real) << "," << endl;
          
          ostr << indent << "\"@real\": \"" << r.toStr() << "\"" << "," << endl;
          ostr << indent << "}";
          break;
        }
        case nvar::Symbol:{
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::Symbol) << "," << endl;
          
          ostr << indent << "\"@symbol\": \"" << n << "\"" << endl;
          ostr << indent << "}";
          break;
        }
        case nvar::RawPointer:
        case nvar::ObjectPointer:
        case nvar::LocalObject:
        case nvar::SharedObject:
          NERROR("found pointer or object");
        case nvar::Binary:
          assert(false && "unimplemented");
        case nvar::Vector:
          emitSequence(ostr, n.vec(), indent + "  ");
          break;
        case nvar::List:{
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::List) << "," << endl;
          
          ostr << indent << "\"@list\": ";
          emitSequence(ostr, n.list(), indent + "  ");
          ostr << endl;
          ostr << indent << "}";
          break;
        }
        case nvar::Queue:{
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::Queue) << "," << endl;
          
          ostr << indent << "\"@queue\": ";
          emitSequence(ostr, n.queue(), indent + "  ");
          ostr << endl;
          ostr << indent << "}";
          break;
        }
        case nvar::Function:{
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::Function) << "," << endl;
          
          ostr << indent << "\"@function\": \"" << n << "\"" << endl;
          ostr << indent << "}";
          break;
        }
        case nvar::HeadSequence:{
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::HeadSequence) <<
          "," << endl;
          
          ostr << indent << "\"@head\": ";
          emitValue(ostr, n.head(), indent + "  ");
          ostr << "," << endl;
          
          ostr << indent << "\"@sequence\": ";
          emitValue(ostr, n.anySequence(), indent + "  ");
          ostr << endl;
          ostr << indent << "}";
          break;
        }
        case nvar::Set:{
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::Set);
          
          const nset& s = n.set();
          for(const nvar& k : s){
            ostr << "," << endl;
            ostr << indent;
            emitKey(ostr, k);
            ostr << ": " << "true";
          }
          ostr << endl << indent << "}";
          break;
        }
        case nvar::HashSet:{
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::HashSet) << endl;
          
          const nhset& s = n.hset();
          for(const nvar& k : s){
            ostr << "," << endl;
            ostr << indent;
            emitKey(ostr, k);
            ostr << ": " << "true";
          }
          
          ostr << indent << "}";
          break;
        }
        case nvar::Map:{
          ostr << "{" << endl;

          bool first = true;
          
          const nmap& m = n.map();
          for(auto& itr : m){
            if(first){
              first = false;
            }
            else{
              ostr << "," << endl;
            }
            ostr << indent;
            emitKey(ostr, itr.first);
            ostr << ": ";
            emitValue(ostr, itr.second, indent + "  ");
          }
          
          ostr << indent << "}";
          break;
        }
        case nvar::HashMap:{
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::HashMap) << endl;
          
          const nhmap& m = n.hmap();
          for(auto& itr : m){
            ostr << "," << endl;
            ostr << indent;
            emitKey(ostr, itr.first);
            ostr << ": ";
            emitValue(ostr, itr.second, indent + "  ");
          }
          
          ostr << indent << "}";
          break;
        }
        case nvar::Multimap:{
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::Multimap) << endl;
          
          nvar nm = n;
          nm.intoMap();
          
          const nmap& m = nm.map();
          for(auto& itr : m){
            ostr << "," << endl;
            ostr << indent;
            emitKey(ostr, itr.first);
            ostr << ": ";
            emitValue(ostr, itr.second, indent + "  ");
          }
          
          ostr << indent << "}";
          break;
        }
        case nvar::HeadMap:{
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::HeadMap) << endl;
          
          ostr << indent << "\"@head\": ";
          emitValue(ostr, n.head(), indent + "  ");
          ostr << endl;
          
          ostr << indent << "\"@map\": ";
          emitValue(ostr, n.anyMap(), indent + "  ");
          ostr << endl;
          ostr << indent << "}";
          break;
        }
        case nvar::SequenceMap:{
          ostr << "{" << endl;
          ostr << indent << "\"@type\": " << int(nvar::SequenceMap) << endl;
          
          ostr << indent << "\"@sequence\": ";
          emitValue(ostr, n.anySequence(), indent + "  ");
          ostr << endl;
          
          ostr << indent << "\"@map\": ";
          emitValue(ostr, n.anyMap(), indent + "  ");
          ostr << endl;
          ostr << indent << "}";
          break;
        }
        case nvar::HeadSequenceMap:{
          ostr << "{" << endl;
          
          ostr << indent << "\"@type\": " << int(nvar::HeadSequenceMap) << endl;
          
          ostr << indent << "\"@head\": ";
          emitValue(ostr, n.head(), indent + "  ");
          ostr << endl;
          
          ostr << indent << "\"@sequence\": ";
          emitValue(ostr, n.anySequence(), indent + "  ");
          ostr << endl;
          
          ostr << indent << "\"@map\": ";
          emitValue(ostr, n.anyMap(), indent + "  ");
          ostr << endl;
          
          ostr << indent << "}";
          break;
        }
        default:
          ostr << n;
          break;
      }
    }
    
  private:
    NJSONGenerator* o_;
  };
  
} // end namespace neu

NJSONGenerator::NJSONGenerator(){
  x_ = new NJSONGenerator_(this);
}

NJSONGenerator::~NJSONGenerator(){
  delete x_;
}

void NJSONGenerator::generate(std::ostream& ostr, const nvar& v){
  x_->generate(ostr, v);
}

nstr NJSONGenerator::toStr(const nvar& v){
  NJSONGenerator generator;
  
  stringstream ostr;
  generator.generate(ostr, v);
  
  return ostr.str();
}

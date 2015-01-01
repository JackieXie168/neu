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

#include <neu/NError.h>

#include <cassert>
#include <sstream>
#include <cstring>

#include <neu/nvar.h>
#include <neu/global.h>

using namespace std;
using namespace neu;

namespace neu{
  
  class NError_{
  public:
    NError_(NError* o, const nstr& msg)
    : o_(o),
    msg_(msg){

      if(_abort){
        cerr << "ABORT: " << msg << endl;
        assert(false);
      }
    }
    
    NError_(NError* o, const nstr& msg, const nvar& var)
    : o_(o),
    msg_(msg),
    var_(var){

      if(_abort){
        cerr << "ABORT: " << msg << endl;
        assert(false);
      }
    }
    
    NError_(NError* o, const NError_& error)
    : o_(o),
    msg_(error.msg_),
    var_(error.var_){}
    
    const char* what() const{
      stringstream ostr;
      ostr << "### NError: ";
      
      nstr loc = var_.getLocation();
      if(!loc.empty()){
        ostr << loc << ": ";
      }
      
      ostr << msg_;
      what_ = ostr.str();
      return what_.c_str();
    }
    
    const nstr& msg() const{
      return msg_;
    }
    
    nvar& var(){
      return var_;
    }
    
  private:
    NError* o_;
    nstr msg_;
    mutable nstr what_;
    nvar var_;
  };
  
} // end namespace neu

NError::NError(const nstr& msg) throw(){
  x_ = new NError_(this, msg);
}

NError::NError(const nstr& msg, const nvar& var) throw(){
  x_ = new NError_(this, msg, var);
}

NError::NError(const NError& error){
  x_ = new NError_(this, *error.x_);
}

NError::~NError() throw(){
  delete x_;
}

const char* NError::what() const throw(){
  return x_->what();
}

const nstr& NError::msg() const throw(){
  return x_->msg();
}

nvar& NError::var() throw(){
  return x_->var();
}

const nvar& NError::var() const throw(){
  return x_->var();
}

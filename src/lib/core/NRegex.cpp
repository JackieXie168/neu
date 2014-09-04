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

#include <neu/NRegex.h>

#include <cstring>
#include <neu/NError.h>

#ifdef __APPLE__
#include <regex>
#else
#include <boost/regex.hpp>
using namespace boost;
#endif

using namespace std;
using namespace neu;

namespace neu{
  
  class NRegex_{
  public:
    
    NRegex_(NRegex* o, const nstr& pattern, uint32_t flags)
    : o_(o){
      
      try{
#ifdef __APPLE__
	regex_ = new regex(pattern.c_str());
#else
	nstr normPattern = pattern;
	normPattern.findReplace("[^]", ".");
	regex_ = new regex(normPattern.c_str());
#endif
      }
      catch(std::exception& e){
        NERROR("invalid pattern: " + pattern);
      }
    }
    
    ~NRegex_(){
      delete regex_;
    }
    
    bool match(const char* text, nvec& vmatch) const{
      cmatch m;
      
      if(regex_match(text, m, *regex_)){
        for(size_t i = 0; i < m.size(); ++i){
          vmatch.push_back(nstr(m[i].first, m[i].second));
        }
        
        return true;
      }
      
      return false;
    }
    
    bool match(const char* text) const{
      cmatch m;
      
      if(regex_match(text, *regex_)){
        return true;
      }
      
      return false;
    }
    
    bool findAll(const char* text, nvec& vmatch) const{
      size_t n = regex_->mark_count() + 1;
      
      vector<int> sm;
      for(size_t i = 0; i < n; ++i){
        sm.push_back(i);
      }
      
      cregex_token_iterator itr(text, text + strlen(text), *regex_, sm);
      
      bool matched = false;
      
      cregex_token_iterator itrEnd;
      while(itr != itrEnd){
        vmatch.push_back(nvec());
        nvec& mi = vmatch.back();
        for(size_t i = 0; i < n; ++i){
          mi.push_back(itr->str());
          ++itr;
        }
        matched = true;
      }
      return matched;
    }
    
  private:
    NRegex* o_;
    regex* regex_;
  };
  
} // end namespace neu

NRegex::NRegex(const nstr& pattern, uint32_t flags){
  x_ = new NRegex_(this, pattern, flags);
}

NRegex::~NRegex(){
  delete x_;
}

bool NRegex::match(const char* text, nvec& vmatch) const{
  return x_->match(text, vmatch);
}

bool NRegex::match(const char* text) const{
  return x_->match(text);
}

bool NRegex::findAll(const char* text, nvec& vmatch) const{
  return x_->findAll(text, vmatch);
}

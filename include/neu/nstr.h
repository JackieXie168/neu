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

#ifndef NEU_N_STR_H
#define NEU_N_STR_H

#include <string>
#include <sstream>

#include <neu/NVector.h>

namespace neu{
  
  class nvar;
  
  class nstr{
  public:
    typedef std::string::iterator iterator;
    typedef std::string::const_iterator const_iterator;
    typedef std::string::reverse_iterator reverse_iterator;
    typedef std::string::const_reverse_iterator const_reverse_iterator;
    
    nstr(const nstr& s)
    : s_(s.s_){}
    
    nstr(const std::string& str)
    : s_(str){}
    
    nstr(const char* s)
    : s_(s){}
    
    nstr(const char* s, size_t n)
    : s_(s, n){}
    
    nstr(size_t n, char c)
    : s_(n, c){}
    
    nstr(nstr&& s)
    : s_(move(s.s_)){}
    
    nstr(std::string&& s)
    : s_(move(s)){}
    
    nstr(std::initializer_list<char> il)
    : s_(il){}
    
    template<class InputIterator>
    nstr(InputIterator begin, InputIterator end) : s_(begin, end){}
    
    nstr(){}
    
    ~nstr(){}
    
    static const size_t npos = std::string::npos;
    
    bool operator!=(const nstr& str) const{
      return s_ != str.s_;
    }
    
    bool operator==(const nstr& str) const{
      return s_ == str.s_;
    }
    
    nstr& operator=(const nstr& str){
      s_ = str.s_;
      return *this;
    }
    
    nstr& operator=(nstr&& str){
      s_ = move(str.s_);
      return *this;
    }
    
    nstr& operator=(std::initializer_list<char> il){
      s_ = il;
      return *this;
    }
        
    bool operator<(const nstr& str) const{
      return s_ < str.s_;
    }
    
    bool operator>(const nstr& str) const{
      return s_ > str.s_;
    }
    
    bool operator<=(const nstr& str) const{
      return s_ <= str.s_;
    }
    
    bool operator>=(const nstr& str) const{
      return s_ >= str.s_;
    }
    
    nstr operator+(const nstr& str) const{
      return move(s_ + str.s_);
    }

    nstr operator+(const nvar& v) const{
      return move(s_ + toStr(v).s_);
    }
    
    nstr operator+(const char* s) const{
      return move(s_ + std::string(s));
    }
    
    nstr& operator+=(const nstr& str){
      s_ += str.s_;
      return *this;
    }

    nstr& operator+=(const std::string& str){
      s_ += str;
      return *this;
    }
    
    nstr& operator+=(const char* str){
      s_ += str;
      return *this;
    }
    
    nstr& operator+=(char c){
      s_ += c;
      return *this;
    }
    
    nstr& operator+=(std::initializer_list<char> il){
      s_ += il;
      return *this;
    }
    
    const char& operator[](size_t pos) const{
      return s_[pos];
    }
    
    char& operator[](size_t pos){
      return s_[pos];
    }
    
    const char& at(size_t pos) const{
      return s_.at(pos);
    }
    
    char& at(size_t pos){
      return s_.at(pos);
    }
    
    const char& front() const{
      return s_.front();
    }
    
    char& front(){
      return s_.front();
    }
    
    const char& back() const{
      return s_.back();
    }
    
    char& back(){
      return s_.back();
    }
    
    void pop_back(){
      s_.pop_back();
    }
    
    nstr& append(const nstr& str){
      s_.append(str.s_);
      return *this;
    }
    
    nstr& append(const nstr& str, size_t pos, size_t n){
      s_.append(str, pos, n);
      return *this;
    }
    
    nstr& append(const char* s, size_t n){
      s_.append(s, n);
      return *this;
    }
    
    nstr& append(const char* s){
      s_.append(s);
      return *this;
    }
    
    nstr& append(size_t n, char c){
      s_.append(n, c);
      return *this;
    }
    
    template<class InputIterator>
    nstr& append(InputIterator first, InputIterator last){
      s_.append(first, last);
      return *this;
    }

    nstr& append(std::initializer_list<char> il){
      s_.append(il);
      return *this;
    }
    
    iterator begin() noexcept{
      return s_.begin();
    }
    
    const_iterator begin() const noexcept{
      return s_.begin();
    }
    
    iterator end() noexcept{
      return s_.end();
    }
    
    const_iterator end() const noexcept{
      return s_.end();
    }
    
    reverse_iterator rbegin() noexcept{
      return s_.rbegin();
    }
    
    const_reverse_iterator rbegin() const noexcept{
      return s_.rbegin();
    }
    
    reverse_iterator rend() noexcept{
      return s_.rend();
    }
    
    const_reverse_iterator rend() const noexcept{
      return s_.rend();
    }
    
    const_iterator cbegin() const noexcept{
      return s_.cbegin();
    }
    
    const_iterator cend() const noexcept{
      return s_.cend();
    }
    
    const_reverse_iterator crbegin() const noexcept{
      return s_.crbegin();
    }
    
    const_reverse_iterator crend() const noexcept{
      return s_.crend();
    }
    
    nstr& insert(size_t pos1, const nstr& str){
      s_.insert(pos1, str.s_);
      return *this;
    }
    
    nstr& insert(size_t pos1, const nstr& str, size_t pos2, size_t n){
      s_.insert(pos1, str.s_, pos2, n);
      return *this;
    }
    
    nstr& insert(size_t pos1, const char* s, size_t n){
      s_.insert(pos1, s, n);
      return *this;
    }
    
    nstr& insert(size_t pos1, const char* s){
      s_.insert(pos1, s);
      return *this;
    }
    
    nstr& insert(size_t pos1, size_t n, char c){
      s_.insert(pos1, n, c);
      return *this;
    }
    
    iterator insert(iterator p, char c){
      return s_.insert(p, c);
    }
    
    void insert(iterator p, size_t n, char c){
      s_.insert(p, n, c);
    }
    
    template<class InputIterator>
    iterator insert(const_iterator p, InputIterator first, InputIterator last){
      return s_.insert(p, first, last);
    }

    bool beginsWith(const nstr& str) const{
      size_t length = str.length();
      
      if(s_.length() < length){
        return false;
      }
      
      return s_.substr(0, length) == str.s_;
    }
    
    bool endsWith(const nstr& str) const{
      size_t length = str.length();
      
      if(s_.length() < length){
        return false;
      }
      
      return s_.substr(s_.length() - length, length) == str.s_;
    }
    
    nstr after(const nstr& str) const{
      size_t pos = s_.rfind(str.s_);
      if(pos == std::string::npos){
        return *this;
      }
      
      return s_.substr(pos);
    }
    
    nstr before(const nstr& str) const{
      size_t pos = s_.find(str.s_);
      if(pos == std::string::npos){
        return *this;
      }
      
      return s_.substr(0, pos);
    }
    
    template<class InputIterator>
    void insert(iterator p, InputIterator first, InputIterator last){
      s_.insert(p, first, last);
    }
    
    size_t copy(char* s, size_t n, size_t pos=0) const{
      return s_.copy(s, n, pos);
    }
    
    nstr substr(size_t pos=0, size_t n=std::string::npos) const{
      return s_.substr(pos, n);
    }
    
    nstr endStr(size_t pos) const{
      return s_.substr(pos, s_.length() - pos);
    }
    
    const char* c_str() const{
      return s_.c_str();
    }
    
    size_t find(const nstr& str, size_t pos=0) const noexcept{
      return s_.find(str, pos);
    }
    
    size_t find(const char* s, size_t pos, size_t n) const noexcept{
      return s_.find(s, pos, n);
    }
    
    size_t find(const char* s, size_t pos=0) const noexcept{
      return s_.find(s, pos);
    }
    
    size_t find(char c, size_t pos=0) const noexcept{
      return s_.find(c, pos);
    }
    
    size_t rfind(const nstr& str, size_t pos=npos) const noexcept{
      return s_.rfind(str.s_, pos);
    }
    
    size_t rfind(const char* s, size_t pos, size_t n) const noexcept{
      return s_.rfind(s, pos, n);
    }
    
    size_t rfind(const char* s, size_t pos=npos) const noexcept{
      return s_.rfind(s, pos);
    }
    
    size_t rfind(char c, size_t pos=npos) const noexcept{
      return s_.rfind(c, pos);
    }
    
    size_t findReplace(const nstr& value,
                       const nstr& replacement,
                       bool all=true,
                       size_t pos=0){
      size_t count = 0;
      size_t len = value.length();
      size_t rlen = replacement.length();
      for(;;){
        pos = find(value, pos);
        if(pos == npos){
          return count;
        }
        ++count;
        replace(pos, len, replacement);
        if(!all){
          return count;
        }
        pos += rlen;
      }
    }
    
    size_t findCount(const nstr& str) const{
      size_t count = 0;
      size_t len = str.length();
      size_t pos = 0;
      for(;;){
        pos = find(str, pos);
        if(pos == npos){
          return count;
        }
        ++count;
        pos += len;
      }
    }
    
    nstr unescapeUTF8() const;
    
    operator std::string() const{
      return s_;
    }
    
    std::string& str(){
      return s_;
    }
    
    const std::string& str() const{
      return s_;
    }
    
    size_t length() const{
      return s_.length();
    }
    
    void resize(size_t n, char c){
      s_.resize(n, c);
    }
    
    void resize(size_t n){
      s_.resize(n);
    }
    
    size_t size() const{
      return s_.size();
    }
    
    size_t max_size() const{
      return s_.max_size();
    }
    
    size_t capacity() const{
      return s_.capacity();
    }
    
    void reserve(size_t res_arg=0){
      s_.reserve(res_arg);
    }
    
    void shrink_to_fit(){
      s_.shrink_to_fit();
    }
    
    void clear() noexcept{
      s_.clear();
    }
    
    nstr& erase(size_t pos=0, size_t n=npos){
      s_.erase(pos, n);
      return *this;
    }
    
    nstr& replace(size_t pos1, size_t n1, const nstr& str){
      s_.replace(pos1, n1, str.s_);
      return *this;
    }
    
    nstr& replace(size_t pos1,
                  size_t n1,
                  const nstr& str,
                  size_t pos2,
                  size_t n2){
      s_.replace(pos1, n1, str.s_, pos2, n2);
      return *this;
    }
    
    nstr& replace(size_t pos,
                  size_t n1,
                  const char* s,
                  size_t n2){
      s_.replace(pos, n1, s, n2);
      return *this;
    }

    nstr& replace(size_t pos, size_t n1, const char* s){
      s_.replace(pos, n1, s);
      return *this;
    }

    nstr& replace(size_t pos, size_t n1, size_t n2, char c){
      s_.replace(pos, n1, n2, c);
      return *this;
    }
    
    void toAllCaps(){
      transform(s_.begin(), s_.end(), s_.begin(), ::toupper);
    }
    
    void toAllLower(){
      transform(s_.begin(), s_.end(), s_.begin(), ::tolower);
    }
    
    void toUppercase(){
      transform(s_.begin(), s_.begin()+1, s_.begin(), ::toupper);
    }
    
    void toLowercase(){
      transform(s_.begin(), s_.begin()+1, s_.begin(), ::tolower);
    }
    
    nstr allCaps() const{
      nstr ret = s_;
      transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
      return ret;
    }
    
    nstr allLower() const{
      nstr ret = s_;
      transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
      return ret;
    }
    
    nstr uppercase() const{
      nstr ret = s_;
      transform(ret.begin(), ret.begin()+1, ret.begin(), ::toupper);
      return ret;
    }
    
    nstr lowercase() const{
      nstr ret = s_;
      transform(ret.begin(), ret.begin()+1, ret.begin(), ::tolower);
      return ret;
    }
    
    void strip(bool start=true, bool end=true){
      if(start){
        for(;;){
          if(s_.empty()){
            break;
          }
          
          if(s_[0] == '\t' || s_[0] == ' ' || s_[0] == '\n'){
            s_.erase(0, 1);
          }
          else{
            break;
          }
        }
      }
      
      if(end){
        for(;;){
          if(s_.empty()){
            break;
          }
          
          size_t last = s_.length() - 1;
          if(s_[last] == '\t' || s_[last] == ' ' || s_[last] == '\n'){
            s_.erase(last, 1);
          }
          else{
            break;
          }
        }
      }
    }
    
    static nstr toStr(const nvar& v, bool concise=true);
    
    static nstr asStr(const nvar& v);
    
    nstr quote() const{
      nstr out = "\"";
      out += s_;
      size_t i = 1;
      for(;;){
        i = out.find("\"", i);
        if(i == std::string::npos){
          break;
        }
        if(i == 0 || out[i-1] != '\\'){
          out.replace(i, 1, "\\\"");
          i += 2;
        }
        else{
          i += 1;
        }
      }
      out += "\"";
      return out;
    }
    
    static bool isSymbol(const nstr& k){
      return isSymbol(k.c_str());
    }
    
    static bool isSymbol(const char* k){
      bool match = false;
      
      size_t i = 0;
      for(;;){
        switch(k[i]){
          case '\0':
            return match;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            if(match){
              break;
            }
            return false;
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
          case 'f':
          case 'g':
          case 'h':
          case 'i':
          case 'j':
          case 'k':
          case 'l':
          case 'm':
          case 'n':
          case 'o':
          case 'p':
          case 'q':
          case 'r':
          case 's':
          case 't':
          case 'u':
          case 'v':
          case 'w':
          case 'x':
          case 'y':
          case 'z':
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
          case 'F':
          case 'G':
          case 'H':
          case 'I':
          case 'J':
          case 'K':
          case 'L':
          case 'M':
          case 'N':
          case 'O':
          case 'P':
          case 'Q':
          case 'R':
          case 'S':
          case 'T':
          case 'U':
          case 'V':
          case 'W':
          case 'X':
          case 'Y':
          case 'Z':
            match = true;
            break;
          case '_':
            break;
          default:
            return false;
        }
        ++i;
      }
      
      return match;
    }
    
    static bool isSymbolNoUnderscore(const nstr& k){
      return isSymbolNoUnderscore(k.c_str());
    }
    
    static bool isSymbolNoUnderscore(const char* k){
      bool match = false;
      
      size_t i = 0;
      for(;;){
        switch(k[i]){
          case '\0':
            return match;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            if(match){
              break;
            }
            return false;
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
          case 'f':
          case 'g':
          case 'h':
          case 'i':
          case 'j':
          case 'k':
          case 'l':
          case 'm':
          case 'n':
          case 'o':
          case 'p':
          case 'q':
          case 'r':
          case 's':
          case 't':
          case 'u':
          case 'v':
          case 'w':
          case 'x':
          case 'y':
          case 'z':
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
          case 'F':
          case 'G':
          case 'H':
          case 'I':
          case 'J':
          case 'K':
          case 'L':
          case 'M':
          case 'N':
          case 'O':
          case 'P':
          case 'Q':
          case 'R':
          case 'S':
          case 'T':
          case 'U':
          case 'V':
          case 'W':
          case 'X':
          case 'Y':
          case 'Z':
            match = true;
            break;
          default:
            return false;
        }
        ++i;
      }
      
      return match;
    }
    
    static bool isSymbolAllCaps(const nstr& k){
      return isSymbolAllCaps(k.c_str());
    }
    
    static bool isSymbolAllCaps(const char* k){
      bool match = false;
      
      size_t i = 0;
      for(;;){
        switch(k[i]){
          case '\0':
            return match;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            if(match){
              break;
            }
            return false;
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
          case 'F':
          case 'G':
          case 'H':
          case 'I':
          case 'J':
          case 'K':
          case 'L':
          case 'M':
          case 'N':
          case 'O':
          case 'P':
          case 'Q':
          case 'R':
          case 'S':
          case 'T':
          case 'U':
          case 'V':
          case 'W':
          case 'X':
          case 'Y':
          case 'Z':
            match = true;
            break;
          case '_':
            break;
          default:
            return false;
        }
        ++i;
      }
      
      return match;
    }
    
    static nstr getB62Id(uint64_t id);
    
    static bool isLower(int c){
      return std::islower(c);
    }
    
    static bool isUpper(int c){
      return std::isupper(c);
    }
    
    static bool isDigit(int c){
      return std::isdigit(c);
    }
    
    static bool isAlpha(int c){
      return std::isalpha(c);
    }
    
    static int toLower(int c){
      return std::tolower(c);
    }
    
    static int toUpper(int c){
      return std::toupper(c);
    }
    
    bool isNumeric() const;
    
    bool empty() const noexcept{
      return s_.empty();
    }
    
    void convertEscapes(){
      findReplace("\\n", "\n");
      findReplace("\\t", "\t");
      findReplace("\\\\", "\\");
    }
    
    void escapeForC(){
      findReplace("\n", "\\n");
      findReplace("\t", "\\t");
      findReplace("\"", "\\\"");
    }
    
    template<typename T>
    static nstr join(const T& v,
                     const nstr& delimiter,
                     bool concise=true){
      std::stringstream ostr;
      typename T::const_iterator itr = v.begin();
      while(itr != v.end()){
        if(itr != v.begin()){
          ostr << delimiter.s_;
        }
        ostr << nstr::asStr(*itr);
        ++itr;
      }
      return ostr.str();
    }
    
    template<typename T>
    void split(T& out, const nstr& delimiter, int maxTokens=-1) const{
      size_t i = 0;
      size_t count = 0;
      for(;;){
        if(maxTokens > 0 && count >= maxTokens){
          out.push_back(substr(i, length() - i));
          break;
        }
        size_t pos = find(delimiter, i);
        if(pos == nstr::npos){
          out.push_back(substr(i, length() - i));
          break;
        }
        out.push_back(substr(i, pos-i));
        i = pos + delimiter.length();
        ++count;
      }
    }
    
    template<typename T>
    void splitOnSpace(T& out) const{
      size_t i = 0;
      size_t len = s_.length();
      
      nstr si;
      bool match = false;
      for(;;){
        if(i >= len){
          break;
        }
        
        if(s_[i] == ' '){
          if(match){
            out.push_back(si);
            si = "";
            match = false;
          }
          
          ++i;

          for(;;){
            if(i >= len){
              return;
            }
            
            if(s_[i] != ' '){
              break;
            }
          }
        }
        else{
          si += s_[i++];
          match = true;
        }
      }
      
      if(match){
        out.push_back(si);
      }
    }
    
    void push_back(char c){
      s_.push_back(c);
    }
    
    nstr& assign(const nstr& str){
      s_.assign(str.s_);
      return *this;
    }
    
    nstr& assign(nstr&& str){
      s_.assign(std::move(str.s_));
      return *this;
    }
    
    nstr& assign(const nstr& str, size_t pos, size_t n){
      s_.assign(str.s_, pos, n);
      return *this;
    }
    
    nstr& assign(const char* s, size_t n){
      s_.assign(s, n);
      return *this;
    }
    
    nstr& assign(const char* s){
      s_.assign(s);
      return *this;
    }
    
    nstr& assign(size_t n, char c){
      s_.assign(n, c);
      return *this;
    }
    
    template <class InputIterator>
    nstr& assign(InputIterator first, InputIterator last){
      s_.assign(first, last);
      return *this;
    }
    
    nstr& assign(std::initializer_list<char> il){
      s_.assign(il);
      return *this;
    }
    
    void swap(nstr& str){
      s_.swap(str.s_);
    }
    
    const char* data() const{
      return s_.data();
    }
    
    std::allocator<char> get_allocator() const{
      return s_.get_allocator();
    }
    
    size_t find_first_of(const nstr& str, size_t pos=0) const noexcept{
      return s_.find_first_of(str.s_, pos);
    }
    
    size_t find_first_of(const char* s, size_t pos, size_t n) const noexcept{
      return s_.find_first_of(s, pos, n);
    }
    
    size_t find_first_of(const char* s, size_t pos=0) const noexcept{
      return s_.find_first_of(s, pos);
    }
    
    size_t find_first_of(char c, size_t pos=0) const noexcept{
      return s_.find_first_of(c, pos);
    }
    
    size_t find_last_of(const nstr& str, size_t pos=npos) const noexcept{
      return s_.find_last_of(str.s_, pos);
    }
    
    size_t find_last_of(const char* s, size_t pos, size_t n) const noexcept{
      return s_.find_last_of(s, pos, n);
    }
    
    size_t find_last_of(const char* s, size_t pos=npos) const noexcept{
      return s_.find_last_of(s, pos);
    }
    
    size_t find_last_of(char c, size_t pos=npos) const noexcept{
      return s_.find_last_of(c, pos);
    }
    
    size_t find_first_not_of(const nstr& str, size_t pos=0) const noexcept{
      return s_.find_first_not_of(str.s_, pos);
    }
    
    size_t find_first_not_of(const char* s,
                             size_t pos,
                             size_t n) const noexcept{
      return s_.find_first_not_of(s, pos, n);
    }
    
    size_t find_first_not_of(const char* s, size_t pos=0) const noexcept{
      return s_.find_first_not_of(s, pos);
    }
    
    size_t find_first_not_of(char c, size_t pos=0) const noexcept{
      return s_.find_first_not_of(c, pos);
    }
    
    size_t find_last_not_of(const nstr& str, size_t pos=npos) const noexcept{
      return s_.find_last_not_of(str.s_, pos);
    }
    
    size_t find_last_not_of(const char* s, size_t pos, size_t n) const noexcept{
      return s_.find_last_not_of(s, pos, n);
    }
    
    size_t find_last_not_of(const char* s, size_t pos=npos) const noexcept{
      return s_.find_last_not_of(s, pos);
    }
    
    size_t find_last_not_of(char c, size_t pos=npos) const noexcept{
      return s_.find_last_not_of(c, pos);
    }
    
  private:
    std::string s_;
  };
  
  inline std::ostream& operator<<(std::ostream& ostr, const nstr& v){
    ostr << v.str();
    return ostr;
  }
  
  inline std::istream& operator>>(std::istream& istr, nstr& str){
    istr >> str.str();
    return istr;
  }
  
  inline nstr operator+(const char* s, const nstr& str){
    return s + str.str();
  }
  
} // end namespace neu

namespace std{

  template<>
  struct hash<neu::nstr>{
    size_t operator()(const neu::nstr& s) const{
      return hash<string>()(s.str());
    }
  };
  
  template<>
  struct hash<pair<neu::nstr, neu::nstr>>{
    size_t operator()(const pair<neu::nstr, neu::nstr>& p) const{
      return hash<string>()(p.first.str()) ^ hash<string>()(p.second.str());
    }
  };
  
} // end namespace std

#endif // NEU_N_STR_H

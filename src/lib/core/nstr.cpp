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

#include <neu/nstr.h>

#include <algorithm>
#include <sstream>
#include <cstring>

#include <neu/nvar.h>
#include <neu/NRegex.h>

using namespace std;
using namespace neu;

namespace{
  
  static NRegex _numericRegex("^[ \t]*(([0-9]+\\.[0-9]+)|(\\.[0-9]+)|"
                              "([0-9]+))([Ee][\\+\\-]?[0-9]+)?[ \t]*$");
  
  static inline int hex_digit(char c){
    return ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'F') ||
            (c >= 'a' && c <= 'f'));
  }
  
  static inline int octal_digit(char c){
    return (c >= '0' && c <= '7');
  }
  
  static inline int u8_wc_toutf8(char* dest, u_int32_t ch){
    if(ch < 0x80){
      dest[0] = (char)ch;
      return 1;
    }
    if(ch < 0x800){
      dest[0] = (ch>>6) | 0xC0;
      dest[1] = (ch & 0x3F) | 0x80;
      return 2;
    }
    if(ch < 0x10000){
      dest[0] = (ch>>12) | 0xE0;
      dest[1] = ((ch>>6) & 0x3F) | 0x80;
      dest[2] = (ch & 0x3F) | 0x80;
      return 3;
    }
    if(ch < 0x110000){
      dest[0] = (ch>>18) | 0xF0;
      dest[1] = ((ch>>12) & 0x3F) | 0x80;
      dest[2] = ((ch>>6) & 0x3F) | 0x80;
      dest[3] = (ch & 0x3F) | 0x80;
      return 4;
    }
    return 0;
  }
  
  static inline int u8_read_escape_sequence(char* str, u_int32_t* dest){
    u_int32_t ch;
    char digs[9] = "\0\0\0\0\0\0\0\0";
    int dno = 0, i = 1;
    
    ch = (u_int32_t)str[0];    /* take literal character */
    if(str[0] == 'n'){
      ch = L'\n';
    }
    else if(str[0] == 't'){
      ch = L'\t';
    }
    else if(str[0] == 'r'){
      ch = L'\r';
    }
    else if(str[0] == 'b'){
      ch = L'\b';
    }
    else if(str[0] == 'f'){
      ch = L'\f';
    }
    else if(str[0] == 'v'){
      ch = L'\v';
    }
    else if(str[0] == 'a'){
      ch = L'\a';
    }
    else if(octal_digit(str[0])){
      i = 0;
      do{
        digs[dno++] = str[i++];
      } while (octal_digit(str[i]) && dno < 3);
      ch = strtol(digs, NULL, 8);
    }
    else if(str[0] == 'x'){
      while(hex_digit(str[i]) && dno < 2){
        digs[dno++] = str[i++];
      }
      if(dno > 0){
        ch = strtol(digs, NULL, 16);
      }
    }
    else if(str[0] == 'u'){
      while (hex_digit(str[i]) && dno < 4){
        digs[dno++] = str[i++];
      }
      if(dno > 0){
        ch = strtol(digs, NULL, 16);
      }
    }
    else if(str[0] == 'U'){
      while(hex_digit(str[i]) && dno < 8){
        digs[dno++] = str[i++];
      }
      if(dno > 0){
        ch = strtol(digs, NULL, 16);
      }
    }
    *dest = ch;
    
    return i;
  }
  
  static inline int u8_unescape(char* buf, int sz, char* src){
    int c = 0, amt;
    u_int32_t ch;
    char temp[4];
    
    while(*src && c < sz){
      if(*src == '\\'){
        src++;
        amt = u8_read_escape_sequence(src, &ch);
      }
      else{
        ch = (u_int32_t)*src;
        amt = 1;
      }
      src += amt;
      amt = u8_wc_toutf8(temp, ch);
      if(amt > sz-c){
        break;
      }
      memcpy(&buf[c], temp, amt);
      c += amt;
    }
    if(c < sz){
      buf[c] = '\0';
    }
    
    return c;
  }
  
}

nstr nstr::unescapeUTF8() const{
  size_t inLength = length();
  char* buf = (char*)malloc(inLength);
  
  size_t outLength = u8_unescape(buf, inLength, (char*)data());
  
  nstr ret;
  ret.append(buf, outLength);
  
  free(buf);
  
  return ret;
}

nstr nstr::toStr(const nvar& v, bool concise){
  return v.toStr(concise);
}

nstr nstr::asStr(const nvar& v){
  return v.str();
}

bool nstr::isNumeric() const{
  return _numericRegex.match(s_);
}

nstr nstr::getB62Id(uint64_t id){
  nstr ret;
  
  for(size_t i = 0; i < 10; ++i){
    char c = id % 62;
    
    if(c < 10){
      ret.insert(0, 1, char(48 + c));
    }
    else if(c < 36){
      ret.insert(0, 1, char(55 + c));
    }
    else{
      ret.insert(0, 1, char(61 + c));
    }
    
    id /= 62;
    
    if(id == 0){
      break;
    }
  }
  
  return ret;
}

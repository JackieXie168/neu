%option noyywrap

%{
  
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
  
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  
  #include <iostream>
  
  #include "NMParser_.h"
  #include "parse.tab.h"
  #include "parse.h"
  
  #define PS yyget_extra(yyscanner)
  
  #if defined (__i386__) || defined (__APPLE__)
  
  #define M_INTEGER_MAX 9223372036854775807LL
  
  #else
  
  #define M_INTEGER_MAX 9223372036854775807L
  
  #endif
  
  using namespace std;
  using namespace neu;
  
  static size_t precision(const nstr& s){
    size_t count = 0;
    bool past = false;
    
    for(size_t i = 0; i < s.size(); ++i){
      if(s[i] == '0'){
        if(past){
          ++count;
        }
      }
      else if(nstr::isDigit(s[i])){
        ++count;
        past = true;
      }
    }
    
    return count;
  }
	
%}

%option reentrant bison-bridge
%option always-interactive

IDENTIFIER [a-zA-Z\$][a-zA-Z0-9\$]*
INTEGER -?[0-9]+
FLOAT -?(([0-9]+\.[0-9]*)|(\.[0-9]+)|([0-9]+))
FLOAT2 -?(([0-9]+\.[0-9]*)|(\.[0-9]+)|([0-9]+))`([0-9]+\.)?
FLOAT3 -?(([0-9]+\.[0-9]*)|(\.[0-9]+)|([0-9]+))`\*^[\+\-]?[0-9]+?

STRING	            \"([^\"]|\\\")*\"

%%

{INTEGER} {
  nreal r(yytext);
  PS->advance(yytext, "numeric");
  if(r <= M_INTEGER_MAX && r >= -M_INTEGER_MAX){
    yylval->v = r.toLong();
    return INTEGER;
  }
  yylval->v = r;
  return REAL;
}

{FLOAT} {
  nstr s = yytext;
  
  PS->advance(yytext, "numeric");
  
  if(precision(s) > 17){
    yylval->v = nreal(s.c_str());
    return REAL;
  }
  
  double d;
  sscanf(s.c_str(), "%lf", &d);
  yylval->v = d;
  return DOUBLE;
}

{FLOAT2} {
  nstr s = yytext;
  
  size_t pos = s.find('`');
  
  s.erase(pos, s.length() - pos);
  
  PS->advance(yytext, "numeric");
  
  if(precision(s) > 17){
    yylval->v = nreal(s.c_str());
    return REAL;
  }
  
  double d;
  sscanf(s.c_str(), "%lf", &d);
  yylval->v = d;
  return DOUBLE;
}

{FLOAT3} {
  nstr s = yytext;
  
  s.findReplace("`*^", "e");
  
  PS->advance(yytext, "numeric");
  
  if(precision(s) > 17){
    yylval->v = nreal(s.c_str());
    return REAL;
  }
  
  double d;
  sscanf(s.c_str(), "%lf", &d);
  yylval->v = d;
  return DOUBLE;
}

{IDENTIFIER} {
  yylval->v = PS->token(yytext);
  return IDENTIFIER;
}

{STRING} {
  yylval->v = yytext;
  nstr& s = yylval->v.str();
  
  s.erase(0, 1);
  s.erase(s.length() - 1, 1);
  s.findReplace("\\\n \n>    ", "");
  PS->advance(yytext, "string");
  return STRING;
}

[\n] {
  PS->newLine();
  PS->advance(1);
}

[ \t>] {
  PS->advance(1);
}

[\[\]\,] {
  PS->advance(1, "operator");
  return *yytext;
}

. { 
  nstr msg = nstr("invalid character: '") + yytext + "'";
  nm_error(PS, yyscanner, msg.c_str());
}

%%

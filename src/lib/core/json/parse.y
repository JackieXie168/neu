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
  
#include <string>
#include <iostream>
  
#include "NJSONParser_.h"
#include "parse.h"
  
using namespace std;
using namespace neu;
  
%}

%name-prefix="njson_"
%pure-parser
%parse-param {NJSONParser_* PS}
%parse-param {void* scanner}
%lex-param {yyscan_t* scanner}

%token<v> STRING_LITERAL KW_TRUE KW_FALSE KW_NULL DOUBLE INTEGER

%type<v> value object elements members

%left ',' ':'

%%

input: object {
  PS->emit($1);
}
;

object: '{' members '}' {
  $$ = move($2);
  PS->convertObject($$);
}
;

value: STRING_LITERAL {
  if($1 == "@undef"){
    $$ = undef;
  }
  else{
    $$ = $1;
  }
}
| DOUBLE {
  $$ = $1;
}
| INTEGER {
  $$ = $1;
}
| object {
  $$ = $1;
}
| '[' elements ']' {
  $$ = $2;
}
| KW_TRUE {
  $$ = true;
}
| KW_FALSE {
  $$ = false;
}
| KW_NULL {
  $$ = none;
}
;

elements: /* empty */ {
  $$ = nvec();
}
| value {
  $$ = nvec();
  $$ << $1;
}
| elements ',' value {
  $$ = move($1);
  $$ << move($3);
}
;

members: /* empty */ {
  $$ = undef;
}
| STRING_LITERAL ':' value {
  $$ = undef;
  $$($1.str()) = move($3);
}
| members ',' STRING_LITERAL ':' value {
  $$ = move($1);
  $$($3.str()) = move($5);
}
;

%%

int njson_error(NJSONParser_* parser, const void*, const char *s){
  parser->error(s);
  return 1;
}

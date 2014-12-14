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
  
  #include <iostream>
  #include <cstdio>
  #include <cstdlib>
  #include <cstring>
  
  #include "NMLParser_.h"
  #include "parse.tab.h"
  #include "parse.h"
  
  #define PS yyget_extra(yyscanner)
  
  using namespace std;
  using namespace neu;
  
%}

%option reentrant bison-bridge
%option always-interactive

COMMENT "//".+\n
MULTILINE_COMMENT "/*"[^"*/"]*"*/"
IDENTIFIER ([a-zA-Z])|(_+[a-zA-Z][a-zA-Z0-9_]*)|([a-zA-Z][a-zA-Z0-9_]*)
INTEGER [0-9]+
DOUBLE (([0-9]+\.[0-9]+)|(\.[0-9]+)|([0-9]+))([Ee][\+\-]?[0-9]+)?
REAL (([0-9]+\.[0-9]+)|(\.[0-9]+)|([0-9]+))([Ee][\+\-]?[0-9]+)?[rR]
STRING_LITERAL \"([^\"]|\\\")*\"
STRING_LITERAL_SQ	\'([^\']|\\\')*\'
EQ "=="
NE "!="
GE ">="
LE "<="
ADD_BY "+="
SUB_BY "-="
INC "++"
DEC "--"
MUL_BY "*="
DIV_BY "/="
MOD_BY "%="
AND "&&"
OR "||"
PUSH "<<"
VAR "=:"
KW_TRUE true
KW_FALSE false
KW_NONE none
KW_UNDEF undef
KW_NEW new
KW_IF if
KW_ELSE else
KW_FOR for
KW_WHILE while
KW_RETURN return
KW_BREAK break
KW_CONTINUE continue
KW_SWITCH switch
KW_CASE case
KW_DEFAULT default
KW_CLASS class
KW_IMPORT import

%%

{COMMENT} {
  PS->newLine(1);
  PS->advance(yytext, "comment");
}

{MULTILINE_COMMENT} {
  size_t count = nstr(yytext).findCount("\n");
  PS->newLine(count);
  PS->advance(yytext, "comment");
}

{EQ} {
  PS->advance(yytext, "operator");
  return EQ;
}

{NE} {
  PS->advance(yytext, "operator");
  return NE;
}

{GE} {
  PS->advance(yytext, "operator");
  return GE;
}

{LE} {
  PS->advance(yytext, "operator");
  return LE;
}

{INC} {
  PS->advance(yytext, "operator");
  return INC;
}

{DEC} {
  PS->advance(yytext, "operator");
  return DEC;
}

{ADD_BY} {
  PS->advance(yytext, "operator");
  return ADD_BY;
}

{SUB_BY} {
  PS->advance(yytext, "operator");
  return SUB_BY;
}

{MUL_BY} {
  PS->advance(yytext, "operator");
  return MUL_BY;
}

{DIV_BY} {
  PS->advance(yytext, "operator");
  return DIV_BY;
}

{MOD_BY} {
  PS->advance(yytext, "operator");
  return MOD_BY;
}

{AND} {
  PS->advance(yytext, "operator");
  return AND;
}

{OR} {
  PS->advance(yytext, "operator");
  return OR;
}

{PUSH} {
  PS->advance(yytext, "operator");
  return PUSH;
}

{VAR} {
  PS->advance(yytext, "operator");
  return VAR;
}

{KW_TRUE} {
  PS->advance(yytext, "keyword");
  return KW_TRUE;
}

{KW_FALSE} {
  PS->advance(yytext, "keyword");
  return KW_FALSE;
}

{KW_NEW} {
  PS->advance(yytext, "keyword");
  return KW_NEW;
}

{KW_NONE} {
  PS->advance(yytext, "keyword");
  return KW_NONE;
}

{KW_UNDEF} {
  PS->advance(yytext, "keyword");
  return KW_UNDEF;
}

{KW_IF} {
  PS->advance(yytext, "keyword");
  return KW_IF;
}

{KW_ELSE} {
  PS->advance(yytext, "keyword");
  return KW_ELSE;
}

{KW_FOR} {
  PS->advance(yytext, "keyword");
  return KW_FOR;
}

{KW_WHILE} {
  PS->advance(yytext, "keyword");
  return KW_WHILE;
}

{KW_RETURN} {
  PS->advance(yytext, "keyword");
  return KW_RETURN;
}

{KW_BREAK} {
  PS->advance(yytext, "keyword");
  return KW_BREAK;
}

{KW_CONTINUE} {
  PS->advance(yytext, "keyword");
  return KW_CONTINUE;
}

{KW_SWITCH} {
  PS->advance(yytext, "keyword");
  return KW_SWITCH;
}

{KW_CASE} {
  PS->advance(yytext, "keyword");
  return KW_CASE;
}

{KW_DEFAULT} {
  PS->advance(yytext, "keyword");
  return KW_DEFAULT;
}

{KW_CLASS} {
  PS->advance(yytext, "keyword");
  return KW_CLASS;
}

{KW_IMPORT} {
  PS->advance(yytext, "keyword");
  return KW_IMPORT;
}

{INTEGER} {
  nreal r(yytext);
  PS->advance(yytext, "numeric");
  if(r <= numeric_limits<int64_t>::max() &&
     r >= numeric_limits<int64_t>::min()){
    yylval->v = r.toLong();
    return INTEGER;
  }
  yylval->v = r;
  return REAL;
}

{DOUBLE} {
  nstr s = yytext;
  PS->advance(yytext, "numeric");
  
  if(nvar::precision(s) > 16){
    yylval->v = nreal(s.c_str());
    return REAL;
  }
  
  double d;
  sscanf(yytext, "%lf", &d);
  yylval->v = d;
  return DOUBLE;
}

{REAL} {
  nstr r = yytext;
  r.erase(r.length() - 1, 1);
  yylval->v = nreal(r.c_str());
  PS->advance(yytext, "numeric");
  return REAL;
}

{IDENTIFIER} {
  yylval->v = PS->token(yytext);
  return IDENTIFIER;
}

{STRING_LITERAL} {
  yylval->v = PS->stringLiteral(yytext);
  PS->advance(yytext, "string");
  return STRING_LITERAL;
}

{STRING_LITERAL_SQ} {
  yylval->v = PS->stringLiteral(yytext);
  PS->advance(yytext, "string");
  return STRING_LITERAL;
}

[\+\-\/\%\*\,\.\;\=\<\>\:\!\|\^\@\`] {
  PS->advance(1, "operator");
  return *yytext;
}

[\{] {
  PS->advance(1, "openBrace");
  PS->openToken();
  return *yytext;
}

[\(] {
  PS->advance(1, "openParen");
  PS->openToken();
  return *yytext;
}

[\[] {
  PS->advance(1, "openBracket");
  PS->openToken();
  return *yytext;
}

[\}] {
  PS->advance(1, "closeBrace");
  PS->closeToken();
  return *yytext;
}

[\)] {
  PS->advance(1, "closeParen");
  PS->closeToken();
  return *yytext;
}

[\]] {
  PS->advance(1, "closeBracket");
  PS->closeToken();
  return *yytext;
}

[\n] {
  PS->newLine();
  if(PS->interactive() && !PS->hasOpenTokens()){
    return ENDL;
  }
}

[ \t] { PS->advance(1); }

<<EOF>> {
  bool terminate = true;
  if(PS->interactive() && PS->hasOpenTokens()){
    FILE* file = PS->readLine();
    if(file){
      yyin = file;
      terminate = false;
    }
  }
  
  if(terminate){
    yyterminate();
  }
}

. { 
  nstr msg = nstr("invalid character: '") + yytext + "'";
  nml_error(PS, yyscanner, msg.c_str());
}

%%

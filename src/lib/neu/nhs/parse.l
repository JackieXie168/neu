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

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <neu/NHSParser.h>

#include "NHSParser_.h"
#include "parse.tab.h"
#include "parse.h"

#define PS yyget_extra(yyscanner)

using namespace std;
using namespace neu;
  
%}

%option reentrant bison-bridge
%option always-interactive

COMMENT --.+\n
EXPR EXPR_
LO [ \t]*"<<<<<"[ \t]*
RO [ \t]*">>>>>"[ \t]*
COMMA [ \t]*","[ \t]*
EQUALS [ \t]*"="[ \t]*
TO [ \t]*"->"[ \t]*
CTO [ \t]*"=>"[ \t]*
TSPEC [ \t]*"::"[ \t]*
DOTDOT [ \t]*".."[ \t]*
SET [ \t]*"<-"[ \t]*
OR [ \t]*"||"[ \t]*
PIPE [ \t]*"|"[ \t]*
AND [ \t]*"&&"[ \t]*
EQ [ \t]*"=="[ \t]*
NE [ \t]*"/="[ \t]*
LE [ \t]*"<="[ \t]*
LT [ \t]*"<"[ \t]*
GE [ \t]*">="[ \t]*
GT [ \t]*">"[ \t]*
CONCAT [ \t]*"++"[ \t]*
ADD [ \t]*"+"[ \t]*
SUB [ \t]*"-"[ \t]*
MUL [ \t]*"*"[ \t]*
DIV [ \t]*"/"[ \t]*
POW [ \t]*"^"[ \t]*
POWS [ \t]*"**"[ \t]*
IDX [ \t]*"!!"[ \t]*
CONS [ \t]*":"[ \t]*
BT [ \t]*"`"[ \t]*
DOLLAR [ \t]*"$"[ \t]*
DOT [ \t]*"."[ \t]*
BS "\\"
WC "_"
LB "["[ \t]*
RB [ \t]*"]"
LC [ \t]*"{"[ \t]*
RC [ \t]*"}"[ \t]*
LP "("[ \t]*
RP [ \t]*")"
ENDL ([ \t]*\n[ \t]*)+
IDENTIFIER ([a-zA-Z])|(_+[a-zA-Z][a-zA-Z0-9_'\.]*)|([a-zA-Z][a-zA-Z0-9_'\.]*)   
INTEGER -?[0-9]+
REAL -?(([0-9]+\.[0-9]+)|(\.[0-9]+)|([0-9]+))([Ee][\+\-]?[0-9]+)?
STRING_LITERAL \"([^\"]|\\\")*\"
CHAR_LITERAL \'([^\']|\\\')\'
OP [ \t]*[\+\-\*\/\^\>\=\<\&\!]+[ \t]*
SP [ \t]*
KW_IF [ \t]*if[^a-zA-Z0-9]
KW_THEN [ \t]*then[^a-zA-Z0-9]
KW_ELSE [ \t]*else[^a-zA-Z0-9]
KW_LET [ \t]*let[^a-zA-Z0-9]
KW_WHERE [ \t]*where[^a-zA-Z0-9]
KW_IN [ \t]*in[^a-zA-Z0-9]
KW_CASE [ \t]*case[^a-zA-Z0-9]
KW_OF [ \t]*of[^a-zA-Z0-9]
KW_IMPORT [ \t]*import[^a-zA-Z0-9]
KW_HIDING [ \t]*hiding[^a-zA-Z0-9]
KW_QUALIFIED [ \t]*qualified[^a-zA-Z0-9]
KW_AS [ \t]*as[^a-zA-Z0-9]
KW_DATA [ \t]*data[^a-zA-Z0-9]
KW_DERIVING [ \t]*deriving[^a-zA-Z0-9]
KW_TYPE [ \t]*type[^a-zA-Z0-9]
KW_CLASS [ \t]*class[^a-zA-Z0-9]
KW_INSTANCE [ \t]*instance[^a-zA-Z0-9]
KW_DO [ \t]*do[^a-zA-Z0-9]

%%

{COMMENT} { 
  PS->newLine(1);
  PS->advance(yytext, "comment");
}

{LO} {
  PS->exitCall();
  PS->advance(strlen(yytext), "operator");
  return LO;
}

{RO} {
  PS->exitCall();
  PS->advance(strlen(yytext), "operator");
  return RO;
}

{EXPR} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return EXPR;
}

{TO} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return TO;
}

{EQUALS} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return EQUALS;
}

{COMMA} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return COMMA;
}

{CTO} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return CTO;
}

{TSPEC} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return TSPEC;
}

{DOTDOT} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return DOTDOT;
}

{SET} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return SET;
}

{OR} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return OR;
}

{PIPE} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return PIPE;
}

{AND} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return AND;
}

{EQ} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return EQ;
}

{NE} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return NE;
}

{LE} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return LE;
}

{LT} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return LT;
}

{GE} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return GE;
}

{GT} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return GT;
}

{CONCAT} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return CONCAT;
}

{ADD} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return ADD;
}

{SUB} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return SUB;
}

{MUL} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return MUL;
}

{DIV} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return DIV;
}

{POW} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return POW;
}

{POWS} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return POWS;
}

{IDX} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return IDX;
}

{BT} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return BT;
}

{DOLLAR} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return DOLLAR;
}

{DOT} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return DOT;
}

{BS} {
  PS->enterCall();
  PS->advance(yytext, "operator");
  return BS;
}

{WC} {
  PS->advance(yytext, "operator");
  return WC;
}

{CONS} {
  PS->exitCall();
  PS->advance(yytext, "operator");
  return CONS;
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

{REAL} {
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

{IDENTIFIER} {
  PS->enterCall();
  yylval->v = PS->token(yytext);
  return IDENTIFIER;
}

{STRING_LITERAL} {
  yylval->v = yytext;
  nstr& s = yylval->v;
  s.erase(0, 1);
  s.erase(s.length() - 1, 1);
  PS->advance(yytext, "string");
  return STRING_LITERAL;
}

{CHAR_LITERAL} {
  yylval->v = yytext;
  nstr& s = yylval->v;
  s.erase(0, 1);
  s.erase(s.length() - 1, 1);
  PS->advance(yytext, "char");
  return CHAR_LITERAL;
}

{OP} {
  yylval->v = yytext;
  yylval->v.str().strip();
  PS->advance(yytext, "operator");
  return OP;
}

{ENDL} {
  nstr s = yytext;
  PS->exitCall();

  for(size_t i = 0; i < s.length(); ++i){
    if(s[i] == '\n'){
      PS->newLine(1);
    }
  }

  PS->advance(strlen(yytext));

  if(s.back() == '\n'){
    if(!PS->hasOpenTokens()){
      return ENDL;
    }    
  }
  else{
    return IDT_ENDL;
  }
}

{SP} {
  PS->advance(strlen(yytext));
  if(PS->inCall()){
    return SP;
  } 
}

[\;\@] {
  PS->exitCall();
  PS->advance(1, "operator");
  return *yytext;
}

{LP} {
  PS->pushCall();
  PS->advance(strlen(yytext), "operator");
  PS->openToken();
  return LP;
}

{RP} {
  bool c = PS->inCall();
  PS->popCall();

  if(c){
    PS->enterCall();
  }

  PS->advance(strlen(yytext), "operator");
  PS->closeToken();
  return RP;
}

{LB} {
  PS->pushCall();
  PS->advance(strlen(yytext), "operator");
  PS->openToken();
  return LB;
}

{RB} {
  PS->popCall();
  PS->advance(strlen(yytext), "operator");
  PS->closeToken();
  return RB;
}

{LC} {
  PS->exitCall();
  PS->advance(strlen(yytext), "operator");
  PS->openToken();
  return LC;
}

{RC} {
  PS->exitCall();
  PS->advance(strlen(yytext), "operator");
  PS->closeToken();
  return RC;
}

{KW_IF} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_IF;
}

{KW_THEN} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_THEN;
}

{KW_ELSE} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_ELSE;
}

{KW_LET} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_LET;
}

{KW_WHERE} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_WHERE;
}

{KW_IN} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_IN;
}

{KW_CASE} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_CASE;
}

{KW_OF} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_OF;
}

{KW_IMPORT} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_IMPORT;
}

{KW_HIDING} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_HIDING;
}

{KW_QUALIFIED} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_QUALIFIED;
}

{KW_AS} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_AS;
}

{KW_DATA} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_DATA;
}

{KW_TYPE} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_TYPE;
}

{KW_CLASS} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_CLASS;
}

{KW_INSTANCE} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_INSTANCE;
}

{KW_DERIVING} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_DERIVING;
}

{KW_DO} {
  PS->exitCall();
  nstr s = yytext;
  unput(s.back());
  PS->advance(s.length(), "keyword");
  return KW_DO;
}

. { 
  nstr msg = nstr("invalid character: '") + yytext + "'";
  nhs_error(PS, yyscanner, msg.c_str());
}

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

%%

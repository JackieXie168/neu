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

#include <string>
#include <iostream>
  
#include "NPLParser_.h"
#include "parse.h"
  
using namespace std;
using namespace neu;
  
%}

%name-prefix="npl_"
%pure-parser
%parse-param {NPLParser_* PS}
%parse-param {void* scanner}
%lex-param {yyscan_t* scanner}

%token<v> IDENTIFIER STRING_LITERAL EQ NE GE LE INC ADD_BY SUB_BY MUL_BY DIV_BY MOD_BY AND OR KW_THIS KW_TRUE KW_FALSE KW_FOR KW_IF KW_ELSE KW_WHILE KW_RETURN KW_BREAK KW_CONTINUE KW_CLASS KW_SWITCH KW_CASE KW_DEFAULT KW_EXTERN DEFINE DOUBLE INTEGER TYPE PTR_TYPE FLOAT

%type<v> stmt expr expr_num expr_map exprs get gets func_def args params block stmts if_stmt class_defs case_stmts case_stmt case_label case_labels var_decl

%left ','
%right '=' ADD_BY SUB_BY MUL_BY DIV_BY MOD_BY
%right '?' ':'
%left OR
%left AND
%left '|'
%left '&'
%left EQ NE
%left '<' '>' GE LE
%right '+' '-'
%right '*' '/' '%'
%left '^'
%right '!' '~'
%right INC DEC
%left '.' ARROW

%%

input: /* empty */
| input KW_CLASS IDENTIFIER '{' class_defs '}' {
  PS->addClass($3, $5);
}
| input DEFINE IDENTIFIER expr {
  PS->define($3, $4);
}
| input KW_EXTERN func_def ';' {
  PS->addExtern($3);
}
;

var_decl: TYPE IDENTIFIER {
  $1.setHead(PS->sym($2));
  $$ = move($1);
}
| IDENTIFIER IDENTIFIER {
  $$ = PS->sym($2);
  $$("class") = PS->sym($1);
}
| PTR_TYPE IDENTIFIER {
  $$ = move($1);
  $$.setHead(PS->sym($2));
}
;

class_defs: func_def block {
  $$ = PS->newClass();
  $1 << move($2);
  PS->addMethod($$, $1);
}
| var_decl ';' {
  $$ = PS->newClass();
  PS->addAttribute($$, $1);
}
| class_defs func_def block {
  $$ = move($1);
  $2 << move($3);
  PS->addMethod($$, $2);
}
| class_defs var_decl ';' {
  $$ = move($1);
  PS->addAttribute($$, $2);
}

expr_num: DOUBLE {
  $$ = PS->var($1);
}
| INTEGER {
  $$ = PS->var($1);
}
| FLOAT {
  $$ = PS->func("Float") << $1;
}
;

expr: expr_num {
  $$ = move($1);
}
| IDENTIFIER {
  $$ = PS->sym($1);
}
| '&' IDENTIFIER {
  $$ = PS->func("Ptr") << PS->sym($2);
}
| '*' IDENTIFIER {
  $$ = PS->func("DePtr") << PS->sym($2);
}
| KW_TRUE {
  $$ = PS->var(true);
}
| KW_FALSE {
  $$ = PS->var(false);
}
| KW_THIS {
  $$ = PS->sym("this");
}
| STRING_LITERAL {
  $$ = move($1);
}
| '-' expr {
  if($2.isNumeric()){
    $$ = PS->var(-$2);
  }
  else{
    $$ = PS->func("Neg") << move($2);
  }
}
| '!' expr {
  $$ = PS->func("Not") << move($2);
}
| expr '+' expr {
  $$ = PS->func("Add") << move($1) << move($3);
}
| expr '-' expr {
  $$ = PS->func("Sub") << move($1) << move($3);
}
| expr '*' expr {
  $$ = PS->func("Mul") << move($1) << move($3);
}
| expr '/' expr {
  $$ = PS->func("Div") << move($1) << move($3);
}
| expr '^' expr {
  $$ = PS->func("Pow") << move($1) << move($3);
}
| expr '%' expr {
  $$ = PS->func("Mod") << move($1) << move($3);
}
| expr INC {
  $$ = PS->func("PostInc") << move($1);
}
| INC expr {
  $$ = PS->func("Inc") << move($2);
}
| expr DEC {
  $$ = PS->func("PostDec") << move($1);
}
| DEC expr {
  $$ = PS->func("Dec") << move($2);
}
| expr ADD_BY expr {
  $$ = PS->func("AddBy") << move($1) << move($3);
}
| expr SUB_BY expr {
  $$ = PS->func("SubBy") << move($1) << move($3);
}
| expr MUL_BY expr {
  $$ = PS->func("MulBy") << move($1) << move($3);
}
| expr DIV_BY expr {
  $$ = PS->func("DivBy") << move($1) << move($3);
}
| expr MOD_BY expr {
  $$ = PS->func("ModBy") << move($1) << move($3);
}
| expr '=' expr {
  $$ = PS->func("Set") << move($1) << move($3);
}
| expr '<' expr {
  $$ = PS->func("LT") << move($1) << move($3);
}
| expr LE expr {
  $$ = PS->func("LE") << move($1) << move($3);
}
| expr '>' expr {
  $$ = PS->func("GT") << move($1) << move($3);
}
| expr GE expr {
  $$ = PS->func("GE") << move($1) << move($3);
}
| expr EQ expr {
  $$ = PS->func("EQ") << move($1) << move($3);
}
| expr NE expr {
  $$ = PS->func("NE") << move($1) << move($3);
}
| expr AND expr {
  $$ = PS->func("And") << move($1) << move($3);
}
| expr OR expr {
  $$ = PS->func("Or") << move($1) << move($3);
}
| '~' expr {
  $$ = PS->func("BitComplement") << move($2);
}
| expr '|' expr {
  $$ = PS->func("BitOr") << move($1) << move($3);
}
| expr '&' expr {
  $$ = PS->func("BitAnd") << move($1) << move($3);
}
| IDENTIFIER '(' args ')' {
  nvar c = PS->func($1);
  c.append($3);
  
  if(PS->handleBuiltin(c)){
    $$ = move(c);
  }
  else{
    $$ = PS->func("Call") << move(c);
  }
}
| '{' args '}' {
  $$ = PS->func("Vec");
  $$.append($2);
}
| expr '?' expr ':' expr  {
  $$ = PS->func("Select") << move($1) << move($3) << move($5);
}
| '[' exprs ']' {
  $$ = undef;
  PS->addItems(nvar::Vector, nvar::Map, $$, $2);
}
| '[' ':' expr ',' exprs ']' {
  $$ = undef;
  $$.setHead($3);
  
  PS->addItems(nvar::Vector, nvar::Map, $$, $5);
}
| '[' '%' exprs ']' {
  $$ = undef;
  PS->addItems(nvar::Vector, nvar::Set, $$, $3);
}
| '[' '%' ':' expr ',' exprs ']' {
  $$ = undef;
  $$.setHead($4);
  PS->addItems(nvar::Vector, nvar::Set, $$, $6);
}
| '[' '^' exprs ']' {
  $$ = undef;
  PS->addItems(nvar::Vector, nvar::HashSet, $$, $3);
}
| '[' '^' ':' expr ',' exprs ']' {
  $$ = undef;
  $$.setHead($4);
  PS->addItems(nvar::Vector, nvar::HashSet, $$, $6);
}
| '[' '=' exprs ']' {
  $$ = undef;
  PS->addItems(nvar::Vector, nvar::HashMap, $$, $3);
}
| '[' '=' ':' expr ',' exprs ']' {
  $$ = undef;
  $$.setHead($4);
  PS->addItems(nvar::Vector, nvar::HashMap, $$, $6);
}
| '[' '|' exprs ']' {
  $$ = undef;
  PS->addItems(nvar::Vector, nvar::Multimap, $$, $3);
}
| '[' '|' ':' expr ',' exprs ']' {
  $$ = undef;
  $$.setHead($4);
  PS->addItems(nvar::Vector, nvar::Multimap, $$, $6);
}
| '(' exprs ')' {
  if($2.size() == 1){
    $$ = move($2[0][0]);
  }
  else{
    $$ = undef;
    PS->addItems(nvar::List, nvar::Map, $$, $2);
  }
}
| '(' exprs ',' ')' {
  $$ = undef;
  PS->addItems(nvar::List, nvar::Map, $$, $2);
}
| '(' ':' expr ',' exprs ')' {
  $$ = undef;
  $$.setHead($3);
  PS->addItems(nvar::List, nvar::Map, $$, $5);
}
| '(' '|' exprs ')' {
  $$ = undef;
  PS->addItems(nvar::List, nvar::Multimap, $$, $3);
}
| '(' '|' exprs ',' ')' {
  $$ = undef;
  PS->addItems(nvar::List, nvar::Multimap, $$, $3);
}
| '(' '|' ':' expr ',' exprs ')' {
  $$ = undef;
  $$.setHead($4);
  PS->addItems(nvar::List, nvar::Multimap, $$, $6);
}
| '(' '%' exprs ')' {
  $$ = undef;
  PS->addItems(nvar::List, nvar::Set, $$, $3);
}
| '(' '%' exprs ',' ')' {
  $$ = undef;
  PS->addItems(nvar::List, nvar::Set, $$, $3);
}
| '(' '%' ':' expr ',' exprs ')' {
  $$ = undef;
  $$.setHead($4);
  PS->addItems(nvar::List, nvar::Set, $$, $6);
}
| '(' '^' exprs ')' {
  $$ = undef;
  PS->addItems(nvar::List, nvar::HashSet, $$, $3);
}
| '(' '^' exprs ',' ')' {
  $$ = undef;
  PS->addItems(nvar::List, nvar::HashSet, $$, $3);
}
| '(' '^' ':' expr ',' exprs ')' {
  $$ = undef;
  $$.setHead($4);
  PS->addItems(nvar::List, nvar::HashSet, $$, $6);
}
| '(' '=' exprs ')' {
  $$ = undef;
  PS->addItems(nvar::List, nvar::HashMap, $$, $3);
}
| '(' '=' exprs ',' ')' {
  $$ = undef;
  PS->addItems(nvar::List, nvar::HashMap, $$, $3);
}
| '(' '=' ':' expr ',' exprs ')' {
  $$ = undef;
  $$.setHead($4);
  PS->addItems(nvar::List, nvar::HashMap, $$, $6);
}
| '@' '[' exprs ']' {
  $$ = undef;
  PS->addItems(nvar::Queue, nvar::Map, $$, $3);
}
| '@' '[' ':' expr ',' exprs ']' {
  $$ = undef;
  $$.setHead($4);
  
  PS->addItems(nvar::Queue, nvar::Map, $$, $6);
}
| '@' '[' '=' exprs ']' {
  $$ = undef;
  PS->addItems(nvar::Queue, nvar::HashMap, $$, $4);
}
| '@' '[' '=' ':' expr ',' exprs ']' {
  $$ = undef;
  $$.setHead($5);
  PS->addItems(nvar::Queue, nvar::HashMap, $$, $7);
}
| '@' '[' '|' exprs ']' {
  $$ = undef;
  PS->addItems(nvar::Queue, nvar::Multimap, $$, $4);
}
| '@' '[' '|' ':' expr ',' exprs ']' {
  $$ = undef;
  $$.setHead($5);
  PS->addItems(nvar::Queue, nvar::Multimap, $$, $7);
}
| '@' '[' '%' exprs ']' {
  $$ = undef;
  PS->addItems(nvar::Queue, nvar::Set, $$, $4);
}
| '@' '[' '%' ':' expr ',' exprs ']' {
  $$ = undef;
  $$.setHead($5);
  PS->addItems(nvar::Queue, nvar::Set, $$, $7);
}
| '@' '[' '^' exprs ']' {
  $$ = undef;
  PS->addItems(nvar::Queue, nvar::HashSet, $$, $4);
}
| '@' '[' '^' ':' expr ',' exprs ']' {
  $$ = undef;
  $$.setHead($5);
  PS->addItems(nvar::Queue, nvar::HashSet, $$, $7);
}
| IDENTIFIER gets {
  $$ = undef;
  PS->handleGet(PS->sym($1), $2, $$);
}
;

func_def: TYPE IDENTIFIER '(' params ')' {
  nvar f = PS->func($2);
  f.append($4);
  $$ = PS->func("TypedFunc") << move($1) << move(f);
}
;

params: /* empty */ {
  $$ = undef;
}
| params ',' var_decl {
  $$ = move($1);
  $$ << move($3);
}
| var_decl {
  $$ = nvec();
  $$ << move($1);
}
;

args: /* empty */ {
  $$ = nvec();
}
| args ',' expr {
  $$ = move($1);
  $$ << move($3);
}
| expr {
  $$ = nvec();
  $$ << move($1);
}
;

expr_map: expr ':' expr {
  $$ = nfunc("KV_") << move($1) << move($3);
}
| expr ':' {
  $$ = nfunc("K_") << move($1);
}

exprs: /* empty */ {
  $$ = nvec();
}
| exprs ',' expr {
  $$ = move($1);
  $$ << (nfunc("V_") << move($3));
}
| expr {
  $$ = nvec();
  $$ << (nfunc("V_") << move($1));
}
| expr_map {
  $$ = nvec();
  $$ << move($1);
}
| exprs ',' expr_map {
  $$ = move($1);
  $$ << move($3);
}
;

get: '[' expr ']' {
  $$ = PS->func("Idx") << move($2);
}
| '.' IDENTIFIER {
  $$ = PS->func("Dot") << PS->sym($2);
}
| ARROW IDENTIFIER {
  $$ = PS->func("Arrow") << PS->sym($2);
}
| '{' expr '}' {
  $$ = PS->func("Put") << move($2);
}
| '.' IDENTIFIER '(' args ')' {
  $$ = PS->func($2);
  $$.append($4);
  if(!PS->handleVarBuiltin($$)){
    $$ = PS->func("In") << move($$);
  }
}
;

gets: gets get {
  $$ = move($1);
  $$ << move($2);
}
| get {
  $$ = nvec();
  $$ << move($1);
}
;

stmt: expr ';' {
  $$ = move($1);
}
| ';' {
  $$ = none;
}
| var_decl ';' {
  $$ = PS->func("Local");
  $$ << move($1);
}
| var_decl '=' expr ';' {
  $$ = PS->func("Local");
  $$ << move($1) << move($3);
}
| KW_RETURN ';' {
  $$ = PS->func("Ret");
}
| KW_RETURN expr ';' {
  $$ = PS->func("Ret") << move($2);
}
| KW_BREAK ';' {
  $$ = PS->func("Break");
}
| KW_CONTINUE ';' {
  $$ = PS->func("Continue");
}
| if_stmt {
  $$ = move($1);
}
| KW_WHILE '(' expr ')' block {
  $$ = PS->func("While") << move($3) << move($5);
}
| KW_FOR '(' stmt stmt expr ')' block {
  $$ = PS->func("For") << move($3) << move($4) << move($5) << move($7);
}
| KW_SWITCH '(' expr ')' '{' case_stmts '}' {
  $$ = PS->createSwitch($3, $6);
}
;

if_stmt: KW_IF '(' expr ')' block {
  $$ = PS->func("If") << move($3) << move($5);
}
| KW_IF '(' expr ')' block KW_ELSE block {
  $$ = PS->func("If") << move($3) << move($5) << move($7);
}
| KW_IF '(' expr ')' block KW_ELSE if_stmt {
  $$ = PS->func("If") << move($3) << move($5) << move($7);
}
;

case_stmts: case_stmts case_stmt {
  $$ = move($1);
  $$.merge($2);
}
| case_stmt {
  $$ = move($1);
}
;

case_stmt: case_labels '{' stmts '}' {
  $$ = nmmap();
  for(const nvar& k : $1){
    $3.str() = "ScopedBlock";
    $$(k) = $3;
  }
}
| case_labels stmts {
  $$ = nmmap();
  for(const nvar& k : $1){
    $$(k) = $2;
  }
}
;

case_label: KW_CASE expr ':' {
  $$ = move($2);
}
| KW_DEFAULT ':' {
  $$ = PS->sym("__default");
}
;

case_labels: case_labels case_label {
  $$ = move($1);
  $$ << move($2);
}
| case_label {
  $$ = nvec() << move($1);
}
;

block: '{' stmts '}' {
  $$ = $2;
  $$.str() = "ScopedBlock";
}
| '{' '}' {
  $$ = PS->func("ScopedBlock");
}
;

stmts: stmts stmt {
  $$ = move($1);
  $$ << move($2);
}
| stmt {
  $$ = PS->func("Block") << move($1);
}
;

%%

int npl_error(NPLParser_* parser, const void*, const char *s){
  parser->error(s);
  return 1;
}

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
  
#include <string>
#include <iostream>
  
#include "NHSParser_.h"
#include "parse.h"
  
using namespace std;
using namespace neu;
  
%}

%name-prefix="nhs_"
%pure-parser
%parse-param {NHSParser_* PS}
%parse-param {void* scanner}
%lex-param {yyscan_t* scanner}

%token <v> IDENTIFIER DOUBLE INTEGER REAL STRING_LITERAL CHAR_LITERAL TSPEC ENDL TO CTO KW_IF KW_ELSE KW_LET KW_IN KW_CASE KW_OF KW_IMPORT KW_HIDING KW_QUALIFIED KW_AS KW_DATA KW_DERIVING KW_TYPE KW_CLASS KW_INSTANCE KW_DO OR AND EQ NE LT LE GT GE CONCAT ADD SUB MUL DIV POW POWS IDX OP LP LB LC RP PB RC RBRC EXPR DOLLAR DOT

%type <v> expr numeric bin_expr rfunc lfunc op_spec exprs expr_outs type types class_spec class_spec_item class_specs type_spec type_specs func_def dos class_func_def guard_func_def param named_param params params_list param_tuples opt_break set lcs lc guards guard wheres where cases case imports type_name data set_records derivings classes

%left LO RO
%right DOLLAR
%right DOT
%left KW_IF KW_THEN KW_ELSE IDENTIFIER KW_LET KW_WHERE KW_IN KW_CASE KW_OF KW_IMPORT KW_HIDING KW_QUALIFIED KW_AS KW_DATA KW_DERIVING KW_TYPE KW_CLASS KW_INSTANCE KW_DO WC ENDL EXPR
%left IDT_ENDL
%left ';'
%left COMMA
%left DOTDOT
%left PIPE
%right TO
%left CTO
%right EQUALS SET
%left LC RC
%left LP RP
%left RB LB
%right TSPEC
%right '$'
%left BS
%right OR
%right AND
%left EQ NE LT LE GT GE 
%right CONS CONCAT
%left ADD SUB
%left MUL DIV
%right POW POWS
%left BT
%right '.'
%left IDX
%left SP

%%

input: /* empty */
| input ENDL
| input IDT_ENDL
| input EXPR ENDL
| input EXPR expr ENDL {
  PS->transform($3);
  PS->emit($3);
}
| input LO expr_outs RO ENDL {
  PS->transform($3);
  PS->emit($3);
}
| input EXPR SUB expr ENDL {
  nvar n = nfunc("Neg") << move($4);
  PS->transform(n);
  PS->emit(n);
}
| input type_spec ENDL {
  PS->transform($2);
  PS->emit($2);
}
| input func_def ENDL {
  PS->transform($2);
  PS->emit($2);
}
| input guard_func_def ENDL {
  PS->transform($2);
  PS->emit($2);
}
| input EXPR KW_LET IDENTIFIER EQUALS expr ENDL {
  nvar f = nfunc("HTopLet") << nsym($4) << move($6);
  PS->transform(f);
  PS->emit(f);
}
| input KW_IMPORT IDENTIFIER ENDL {
  nvar f = nfunc("HImport") << nsym($3);
  PS->emit(f);
}
| input KW_IMPORT IDENTIFIER SP LP imports RP ENDL {
  nvar f = nfunc("HImport") << nsym($3) << move($6);
  PS->emit(f);
}
| input KW_IMPORT IDENTIFIER KW_HIDING LP imports RP ENDL {
  nvar f = nfunc("HImportHiding") << nsym($3) << move($6);
  PS->emit(f);
}
| input KW_IMPORT KW_QUALIFIED IDENTIFIER ENDL {
  nvar f = nfunc("HImportQualified") << nsym($4);
  PS->emit(f);
}
| input KW_IMPORT KW_QUALIFIED IDENTIFIER KW_AS IDENTIFIER ENDL {
  nvar f = nfunc("HImportQualified") << nsym($4) << nsym($6);
  PS->emit(f);
}
| input KW_DATA type_name EQUALS data {
  nvar f = nfunc("HData") << move($3) << move($5);
  PS->emit(f);
}
| input KW_DATA type_name EQUALS data KW_DERIVING LP derivings RP {
  nvar f = nfunc("HData") << move($3) << move($5) << move($8);
  PS->emit(f);
}
| input KW_TYPE type_name EQUALS type {
  nvar t = nfunc("HTypeDef") << move($3) << move($5);
  PS->emit(t);
}
| input KW_CLASS type_name KW_WHERE IDT_ENDL classes ENDL {
  nvar f = nfunc("HClass") << move($3) << move($6);
  PS->emit(f);
}
| input KW_INSTANCE IDENTIFIER SP IDENTIFIER KW_WHERE IDT_ENDL classes ENDL {
  nvar f = nfunc("HInstance") << nsym($3) << nsym($5) << move($8);
  PS->emit(f);
}
;

classes: classes IDT_ENDL type_spec {
  $$ = move($1);
  $$ << move($3);  
}
| classes IDT_ENDL class_func_def {
  $$ = move($1);
  $$ << move($3);
}
| type_spec {
  $$ = nvec();
  $$ << move($1);
}
| class_func_def {
  $$ = nvec();
  $$ << move($1);
}
;

opt_break: /* empty */ { }
| IDT_ENDL { }
;

bin_expr: expr OR expr {
  $$ = nfunc("Or") << move($1) << move($3);
}
| expr AND expr {
  $$ = nfunc("And") << move($1) << move($3);
}
| expr EQ expr {
  $$ = nfunc("EQ") << move($1) << move($3);
}
| expr NE expr {
  $$ = nfunc("NE") << move($1) << move($3);
}
| expr LT expr {
  $$ = nfunc("LT") << move($1) << move($3);
}
| expr LE expr {
  $$ = nfunc("LE") << move($1) << move($3);
}
| expr GT expr {
  $$ = nfunc("GT") << move($1) << move($3);
}
| expr GE expr {
  $$ = nfunc("GE") << move($1) << move($3);
}
| expr CONCAT expr {
  $$ = nfunc("HConcat") << move($1) << move($3);
}
| expr ADD expr {
  $$ = nfunc("Add") << move($1) << move($3);
}
| expr SUB expr {
  $$ = nfunc("Sub") << move($1) << move($3);
}
| expr MUL expr {
  $$ = nfunc("Mul") << move($1) << move($3);
}
| expr DIV expr {
  $$ = nfunc("Div") << move($1) << move($3);
}
| expr POW expr {
  $$ = nfunc("Pow") << move($1) << move($3);
}
| expr POWS expr {
  $$ = nfunc("Pow") << move($1) << move($3);
}
| expr IDX expr {
  $$ = nfunc("HIdx") << move($1) << move($3);
}
;

lfunc: LP expr BT IDENTIFIER BT RP {
  $$ = nfunc("HLFunc") << move($2) << $4;
}
| LP expr OR RP {
  $$ = nfunc("HLFunc") << move($2) << "||";
}
| LP expr AND RP {
  $$ = nfunc("HLFunc") << move($2) << "&&";
}
| LP expr EQ RP {
  $$ = nfunc("HLFunc") << move($2) << "==";
}
| LP expr NE RP {
  $$ = nfunc("HLFunc") << move($2) << "/=";
}
| LP expr LT RP {
  $$ = nfunc("HLFunc") << move($2) << "<";
}
| LP expr LE RP {
  $$ = nfunc("HLFunc") << move($2) << "<=";
}
| LP expr GT RP {
  $$ = nfunc("HLFunc") << move($2) << ">";
}
| LP expr GE RP {
  $$ = nfunc("HLFunc") << move($2) << ">=";
}
| LP expr CONCAT RP {
  $$ = nfunc("HLFunc") << move($2) << "++";
}
| LP expr ADD RP {
  $$ = nfunc("HLFunc") << move($2) << "+";
}
| LP expr MUL RP {
  $$ = nfunc("HLFunc") << move($2) << "*";
}
| LP expr DIV RP {
  $$ = nfunc("HLFunc") << move($2) << "/";
}
| LP expr POW RP {
  $$ = nfunc("HLFunc") << move($2) << "^";
}
| LP expr POWS RP {
  $$ = nfunc("HLFunc") << move($2) << "**";
}
| LP expr IDX RP {
  $$ = nfunc("HLFunc") << move($2) << "!!";
}
| LP expr CONS RP {
  $$ = nfunc("HLFunc") << move($2) << ":";
}
;

rfunc: LP BT IDENTIFIER BT expr RP {
  $$ = nfunc("HRFunc") << $3 << move($5); 
}
| LP OR expr RP {
  $$ = nfunc("HRFunc") << "||" << move($3);
}
| LP OR RP {
  $$ = nfunc("HRFunc") << "||";
}
| LP AND expr RP {
  $$ = nfunc("HRFunc") << "&&" << move($3);
}
| LP AND RP {
  $$ = nfunc("HRFunc") << "&&";
}
| LP EQ expr RP {
  $$ = nfunc("HRFunc") << "==" << move($3);
}
| LP EQ RP{
  $$ = nfunc("HRFunc") << "==";
}
| LP NE expr RP {
  $$ = nfunc("HRFunc") << "/=" << move($3);
}
| LP NE RP {
  $$ = nfunc("HRFunc") << "/=";
}
| LP LT expr RP {
  $$ = nfunc("HRFunc") << "<" << move($3);
}
| LP LT RP {
  $$ = nfunc("HRFunc") << "<";
}
| LP LE expr RP {
  $$ = nfunc("HRFunc") << "<=" << move($3);
}
| LP LE RP {
  $$ = nfunc("HRFunc") << "<=";
}
| LP GT expr RP {
  $$ = nfunc("HRFunc") << ">" << move($3);
}
| LP GT RP {
  $$ = nfunc("HRFunc") << ">";
}
| LP GE expr RP {
  $$ = nfunc("HRFunc") << ">=" << move($3);
}
| LP GE RP {
  $$ = nfunc("HRFunc") << ">=";
}
| LP CONCAT expr RP {
  $$ = nfunc("HRFunc") << "++" << move($3);
}
| LP CONCAT RP {
  $$ = nfunc("HRFunc") << "++";
}
| LP ADD expr RP {
  $$ = nfunc("HRFunc") << "+" << move($3);
}
| LP ADD RP {
  $$ = nfunc("HRFunc") << "+";
}
| LP MUL expr RP {
  $$ = nfunc("HRFunc") << "*" << move($3);
}
| LP MUL RP {
  $$ = nfunc("HRFunc") << "*";
}
| LP DIV expr RP {
  $$ = nfunc("HRFunc") << "/" << move($3);
}
| LP DIV RP {
  $$ = nfunc("HRFunc") << "/";
}
| LP POW expr RP {
  $$ = nfunc("HRFunc") << "^" << move($3);
}
| LP POW RP {
  $$ = nfunc("HRFunc") << "^";
}
| LP POWS expr RP {
  $$ = nfunc("HRFunc") << "**" << move($3);
}
| LP POWS RP {
  $$ = nfunc("HRFunc") << "**";
}
| LP IDX expr RP {
  $$ = nfunc("HRFunc") << "!!" << move($3);
}
| LP IDX RP {
  $$ = nfunc("HRFunc") << "!!";
}
| LP CONS expr RP {
  $$ = nfunc("HRFunc") << ":" << move($3);
}
| LP CONS RP {
  $$ = nfunc("HRFunc") << ":";
}
;

op_spec: LP OR RP {
  $$ = nfunc("HOpFunc") << "||";
}
| LP AND RP {
  $$ = nfunc("HOpFunc") << "&&";
}
| LP EQ RP {
  $$ = nfunc("HOpFunc") << "==";
}
| LP NE RP {
  $$ = nfunc("HOpFunc") << "/=";
}
| LP LT RP {
  $$ = nfunc("HOpFunc") << "<";
}
| LP LE RP {
  $$ = nfunc("HOpFunc") << "<=";
}
| LP GT RP {
  $$ = nfunc("HOpFunc") << ">";
}
| LP GE RP {
  $$ = nfunc("HOpFunc") << ">=";
}
| LP CONCAT RP {
  $$ = nfunc("HOpFunc") << "++";
}
| LP ADD RP {
  $$ = nfunc("HOpFunc") << "+";
}
| LP MUL RP {
  $$ = nfunc("HOpFunc") << "*";
}
| LP DIV RP {
  $$ = nfunc("HOpFunc") << "/";
}
| LP POW RP {
  $$ = nfunc("HOpFunc") << "^";
}
| LP POWS RP {
  $$ = nfunc("HOpFunc") << "**";
}
| LP IDX RP {
  $$ = nfunc("HOpFunc") << "!!";
}
| LP CONS RP {
  $$ = nfunc("HOpFunc") << ":";
}
;

numeric: DOUBLE {
  $$ = $1;
}
| INTEGER {
  $$ = $1;
}
| REAL {
  $$ = $1;
}
;

expr: numeric {
  $$ = $1;
}
| LP SUB expr RP {
  $$ = nfunc("Neg");
  $$ << move($3);
}
| bin_expr {
  $$ = move($1);
}
| rfunc {
  $$ = move($1);
}
| lfunc {
  $$ = move($1);
}
| LB exprs RB {
  $$ = PS->getList($2);
}
| LP exprs RP {
  if($2.size() == 1){
    $$ = move($2[0]);
  }
  else{
    $$ = nfunc("Tuple");
    $$.append($2);
  }
}
| IDENTIFIER {
  $$ = nsym($1);
}
| expr SP expr {
  $$ = PS->getCall($1, $3);
}
| expr DOLLAR expr {
  $$ = nfunc("HDollar") << move($1) << move($3);
}
| expr DOT expr {
  $$ = nfunc("HCompose") << move($1) << move($3);
}
| expr BT IDENTIFIER BT expr {
  $$ = nfunc("HCall") << (nfunc($3) << move($1) << move($5));
}
| BS param params TO expr {
  $3.pushFront($2);
  $$ = nfunc("HLambda") << move($3) << move($5);
}
| BS param TO expr {
  $$ = nfunc("HLambda") << (nvec() << move($2)) << move($4);
}
| STRING_LITERAL {
  $$ = $1;
}
| CHAR_LITERAL {
  $$ = nfunc("Char") << move($1);
}
| KW_LET wheres opt_break KW_IN expr {
  $$ = nfunc("HLet") << move($2) << move($5);
} 
| KW_IF expr opt_break KW_THEN expr opt_break KW_ELSE expr {
  $$ = nfunc("HIf") << move($2) << move($5) << move($8);
} 
| expr CONS expr {
  $$ = nfunc("HCons") << move($1) << move($3);
}
| LB expr PIPE lcs RB {
  $$ = nfunc("HLC") << move($2);
  $$.append($4);
}
| expr IDT_ENDL KW_WHERE wheres {
  $$ = nfunc("HWhere") << move($1) << move($4);
}
| expr KW_WHERE wheres {
  $$ = nfunc("HWhere") << move($1) << move($3);
}
| KW_CASE expr KW_OF cases {
  $$ = nfunc("HCase") << move($2) << move($4);
}
| IDENTIFIER LC set_records RC {
  $$ = nfunc("HSetRecord") << nsym($1) << move($3);
}
;

exprs: /* empty */ {
  $$ = nvec();
}
| exprs COMMA expr {
  $$ = move($1);
  $$ << move($3);
}
| exprs COMMA expr DOTDOT expr {
  $$ = move($1) << (nfunc("HRangeList_") << move($3) << move($5));
}
| exprs COMMA expr DOTDOT {
  $$ = move($1) << (nfunc("HInfList_") << move($3));
 }
| expr {
  $$ = nvec();
  $$ << move($1);
}
| expr DOTDOT expr {
  $$ = nfunc("HRangeList_") << move($1) << move($3);
}
| expr DOTDOT {
  $$ = nfunc("HInfList_") << move($1);
}
;

expr_outs: expr_outs ENDL expr {
  $$ = move($1);
  $$ << move($3);
}
| expr {
  $$ = nfunc("HOut") << move($1);
}
;

type: type_name {
  if($1.size() == 1){
    $$ = move($1[0]);
  }
  else{
    $$ = move($1);
    $$.str() = "HParamType";
  }
}
| type TO type {
  if($1.isFunction("HTo")){
    $$ = move($1);
  }
  else{
    $$ = nfunc("HTo");
    $$ << move($1);
  }

  if($3.isFunction("HTo")){
    $$.append($3);
  }
  else{
    $$ << move($3);
  }
}
| LP type TO type RP {
  $$ = nfunc("HTo_") << move($2) << move($4);
}
| LB type RB {
  $$ = nvec();
  $$ << move($2);
}
| LP types RP {
  $$ = move($2);
}
;

types: types COMMA type {
  $$ = move($1);
  $$ << move($3);
}
| type {
  $$ = nfunc("Tuple") << move($1);
}
;

class_spec: LP class_specs RP {
  $$ = move($2);
} 
| class_spec_item {
  $$ = nfunc("HTC") << move($1);
}
;

class_spec_item: IDENTIFIER SP IDENTIFIER {
  $$ = nfunc($1) << nsym($3);
}
;

class_specs: class_specs COMMA class_spec_item {
  $$ = move($1);
  $$ << move($3);
}
| class_spec_item {
  $$ = nfunc("HTC");
  $$ << move($1);
}
;

type_spec: IDENTIFIER TSPEC type {
  $$ = nfunc("HType") << nsym($1) << move($3);
}
| IDENTIFIER TSPEC class_spec CTO type {
  $$ = nfunc("HType") << nsym($1) << move($3) << move($5);
}
| op_spec TSPEC type {
  $$ = nfunc("HType") << move($1) << move($3);
}
;

type_specs: type_specs COMMA opt_break type_spec {
  $$ = move($1);
  $$ << move($4);
}
| type_spec {
  $$ = nvec();
  $$ << move($1);
}
;

param: named_param {
  $$ = $1;
}
| WC {
  $$ = nfunc("HAny");
}
| numeric {
  $$ = $1;
}
| STRING_LITERAL {
  $$ = $1;
}
| CHAR_LITERAL {
  $$ = nfunc("Char") << move($1);
}
| IDENTIFIER '@' param {
  $$ = nfunc("HPA") << nsym($1) << $3;
}
| LP IDENTIFIER params RP {
  $$ = nfunc("HDataParam") << nsym($2);
  $$.append($3);
}
| LP IDENTIFIER LC set_records RC RP {
  $$ = nfunc("HSetRecord") << nsym($2) << move($4);
}
;

named_param: IDENTIFIER {
  $$ = nsym($1);
}
| LP param_tuples RP {
  $$ = move($2);
}
| LB params_list RB {
  $$ = move($2);
}
;

param_tuples: param_tuples COMMA param {
  if($1.isFunction("Tuple")){
    $$ = move($1);
    $$ << move($3);
  }
  else{
    $$ = nfunc("Tuple") << move($1) << move($3);
  }
}
| param_tuples CONS param {
  $$ = nfunc("HCons") << move($1) << move($3);
}
| param {
  $$ = move($1);
}
;

params_list: /* empty */ {
  $$ = nvec();
}
| params_list COMMA param {
  $$ = move($1);
  $$ << move($3);
}
| param {
  $$ = nvec();
  $$ << move($1);
}
;

class_func_def: IDENTIFIER params EQUALS opt_break expr {
  nvar f = nfunc($1);
  f.append($2);
  $$ = nfunc("HFunc") << move(f) << move($5);
}
| IDENTIFIER EQUALS opt_break expr {
  $$ = nfunc("HFunc") << nfunc($1) << $4;
}
| param BT IDENTIFIER BT param EQUALS opt_break expr {
  $$ = nfunc("HFunc") << (nfunc($3) << move($1) << move($5)) << move($8);
}
| param OR param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("Or") << move($1) << move($3)) << $5;
}
| param AND param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("And") << move($1) << move($3)) << $5;
}
| param EQ param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("EQ") << move($1) << move($3)) << $5;
}
| param NE param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("NE") << move($1) << move($3)) << $5;
}
| param LT param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("LT") << move($1) << move($3)) << $5;
}
| param LE param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("LE") << move($1) << move($3)) << $5;
}
| param GT param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("GT") << move($1) << move($3)) << $5;
}
| param GE param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("GE") << move($1) << move($3)) << $5;
}
| param CONCAT param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("HConcat") << move($1) << move($3)) << $5;
}
| param ADD param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("Add") << move($1) << move($3)) << $5;
}
| param SUB param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("Sub") << move($1) << move($3)) << $5;
}
| param MUL param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("Mul") << move($1) << move($3)) << $5;
}
| param DIV param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("Div") << move($1) << move($3)) << $5;
}
| param POW param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("Pow") << move($1) << move($3)) << $5;
}
| param POWS param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("Pow") << move($1) << move($3)) << $5;
}
| param IDX param EQUALS expr {
  $$ = nfunc("HFunc") << (nfunc("HIdx") << move($1) << move($3)) << $5;
}
;

func_def: IDENTIFIER params EQUALS opt_break expr {
  nvar f = nfunc($1);
  f.append($2);
  $$ = nfunc("HFunc") << f << move($5);
}
| IDENTIFIER EQUALS opt_break expr {
  nvar f = nfunc($1);
  $$ = nfunc("HFunc") << f << move($4);
}
| param BT IDENTIFIER BT param EQUALS opt_break expr {
  $$ = nfunc("HFunc") << (nfunc($3) << move($1) << move($5)) << move($8);
}
| IDENTIFIER params EQUALS KW_DO dos {
  nvar f = nfunc($1);
  f.append($2);
  $$ = nfunc("HFunc") << f << move($5);
}
| IDENTIFIER EQUALS KW_DO dos {
  $$ = nfunc("HFunc") << nfunc($1) << move($4);
}
| param BT IDENTIFIER BT param EQUALS KW_DO dos {
  $$ = nfunc("HFunc") << (nfunc($3) << move($1) << move($5)) << move($8);
}
;

dos: dos IDT_ENDL expr {
  $$ = move($1);
  $$ << move($3);
}
| IDT_ENDL expr {
  $$ = nfunc("HDo") << move($2);
}
;

guard_func_def: IDENTIFIER params opt_break guards {
  nvar f = nfunc($1);
  f.append($2);
  $$ = nfunc("HGuardFunc") << move(f) << move($4);
}
| IDENTIFIER params opt_break guards IDT_ENDL KW_WHERE wheres {
  nvar f = nfunc($1);
  f.append($2);
  $$ = nfunc("HGuardFunc") << move(f) << move($4) << move($7);
}
;

guards: guards opt_break guard {
  $$ = move($1);
  $$ << move($3);
}
| guard {
  $$ = nvec();
  $$ << move($1);
}
;

guard: PIPE expr EQUALS expr {
  $$ = nfunc("Tuple") << move($2) << move($4);
}
;

wheres: wheres IDT_ENDL where {
  $$ = move($1);
  $$ << move($3);
}
| where {
  $$ = nfunc("Tuple") << move($1);
}
;

where: IDENTIFIER params EQUALS expr {
  nvar f = nfunc($1);
  f.append($2);
  $$ = nfunc("HFunc") << move(f) << move($4);
}
| IDENTIFIER EQUALS expr {
  $$ = nvec();
  $$ << nsym($1) << move($3);
}
| LP param_tuples RP EQUALS expr {
  $$ = nvec();
  $$ << move($2) << move($5);
}
;

params: params SP param {
  $$ = move($1);
  $$ << move($3);
}
| SP param {
  $$ = nvec();
  $$ << move($2);
}
;

lcs: lcs COMMA lc {
  $$ = move($1);
  $$ << move($3);
}
| lc {
  $$ = nvec();
  $$ << move($1);
}
;

lc: expr {
  $$ = move($1);
}
| set {
  $$ = move($1);
}
;

cases: cases IDT_ENDL case {
  $$ = move($1);
  $$ << move($3);
}
| case {
  $$ = nvec();
  $$ << move($1);
}
;

case: param TO expr {
  $$ = nvec();
  $$ << move($1) << move($3);
}
;

set: IDENTIFIER SET expr {
  $$ = nfunc("HSet") << nsym($1) << $3;
}
| WC SET expr {
  $$ = nfunc("HSet") << nfunc("HAny") << $3;
}
;

imports: imports COMMA IDENTIFIER {
  $$ = move($1);
  $$ << nsym($3);
}
| IDENTIFIER {
  $$ = nvec();
  $$ << nsym($1);
}
;

type_name: type_name SP IDENTIFIER {
  $$ = move($1);
  $$ << nsym($3);
}
| IDENTIFIER {
  $$ = nfunc("Tuple") << nsym($1);
}
;

data: data opt_break PIPE type_name {
  $$ = move($1);
  $$ << move($4);
}
| data opt_break PIPE type_name LC type_specs opt_break RC {
  $$ = move($1);
  $$ << nfunc("HRecord") << move($4) << move($6);
}
| type_name {
  $$ = nvec();
  $$ << move($1);
}
| type_name LC type_specs opt_break RC {
  $$ = nvec();
  $$ << (nfunc("HRecord") << move($1) << move($3));
}
;

derivings: derivings COMMA IDENTIFIER {
  $$ = move($1);
  $$ << nsym($3);
}
| IDENTIFIER {
  $$ = nvec() << nsym($1);
}
;

set_records: set_records COMMA IDENTIFIER EQUALS expr {
  $$ = move($1);
  $$ << (nfunc("Tuple") << nsym($3) << move($5));
}
| IDENTIFIER EQUALS expr {
  $$ = nfunc("Tuple") << (nfunc("Tuple") << nsym($1) << move($3));
}
;

%%

int nhs_error(NHSParser_* parser, const void*, const char *s){
  parser->error(s);
  return 1;
}

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

#include <neu/global.h>
#include <neu/NObject.h>
#include <neu/NHSObject.h>
#include <neu/NProgram.h>
#include <neu/NHSParser.h>

#include <editline/readline.h>

using namespace std;
using namespace neu;

class Program : public NProgram{
public:
  Program(int argc, char** argv)
  : NProgram(argc, argv){}
  
  void handleSignal(int signal, bool fatal){
    if(fatal){
      cout << endl;
      NProgram::exit(0);
    }
  }
};

class Parser : public NHSParser{
public:
  Parser()
 : done_(false),
  first_(true){}

  void reset(){
    first_ = true;
  }
  
  bool readLine(nstr& line){
    char* l = readline(first_ ? ">>> " : "");
    first_ = false;

    if(!l){
      done_ = true;
      return false;
    }
    
    line = "EXPR_ ";
    line += l;
    line.strip();
    if(!line.empty()){
      add_history(l);
    }

    free(l);

    return true;
  }

  bool done(){
    return done_;
  }

private:
  bool done_;
  bool first_;
};

void printUsage(){
  cout << NProgram::usage("neu-haskell [OPTION]... [HASKELL FILE]...");
}

int main(int argc, char** argv){
  Program::opt("help", "h", false, "Display usage");

  Program::opt("haskellPath", "", 100, "Path to Haskell command (ghci)");

  Program::opt("show", "s", false, "Show N input/output");  

  Program::opt("history", "", 100, "Number of lines to keep in history"); 

  Program program(argc, argv);

  nvar args = program.args();

  NHSObject::setHaskellPath(args["haskellPath"]);
  
  if(args["help"]){
    printUsage();
    program.exit(0);
  }

  bool show = args["show"];

  Parser parser;
  NHSObject o;
  
  if(args.size() == 1){
    nvar n = parser.parseFile(args[0]);
    
    if(show){
      cout << n << endl;
    }
    
    if(n.isNone()){
      program.exit(1);
    }
    
    o.load(n);
  }
  else if(args.size() > 1){
    printUsage();
    program.exit(1);
  }
  
  stifle_history(args["history"]);
  
  for(;;){
    parser.reset();
    nvar n = parser.parse();
    
    if(parser.done()){
      cout << endl;
      NProgram::exit(0);
    }
    
    if(n.isNone()){
      continue;
    }
    
    if(show){
      cout << "<< " << n << endl;
    }
    
    try{
      nvar r = o.run(n);

      cout << r << endl;
    }
    catch(NError& e){
      cerr << e << endl;
    }
  }
  
  NProgram::exit(0);
  
  return 0;
}

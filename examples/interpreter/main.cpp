/*

This example demonstrates the use of NParser to define a simple
language, parser, and interpreter that is capable of performing
arithmetical and logical operations on scalars and vectors and enables
control flow with: if/else/for/while.

*/

#include <iostream>

#include <neu/nvar.h>
#include <neu/NProgram.h>
#include <neu/NParser.h>
#include <neu/NSys.h>
#include <neu/NMLGenerator.h>
#include <neu/NObject.h>

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

class Parser : public NParser{
public:
  Parser() : NParser("Parser"){}

  void init(){
    discardWhitespace();
    discardNewLine();

    // add some punctuators with no precedence
    addPunc(";");
    addPunc("(");
    addPunc(")");
    addPunc("{");
    addPunc("}");
    addPunc("[");
    addPunc("]");
    
    // add keywords
    addKeyword("if");
    addKeyword("else");
    addKeyword("break");
    addKeyword("continue");
    addKeyword("while");
    addKeyword("for");

    // add precedence punctuators, lower values mean lower precedence.
    // by default, they are left-associative unless false is passed
    // as the third parameter
    addPunc(",", 1);
    addPunc("=", 1, false);
    addPunc("+=", 1, false);
    addPunc("-=", 1, false);
    addPunc("*=", 1, false);
    addPunc("/=", 1, false);
    addPunc("%=", 1, false);

    addPunc("||", 2, false);

    addPunc("&&", 3, false);

    addPunc("==", 4, false);
    addPunc("!=", 4, false);

    addPunc("<", 5, false);
    addPunc("<=", 5, false);
    addPunc(">", 5, false);
    addPunc(">=", 5, false);

    addPunc("<<", 6);

    addPunc("+", 7);
    addPunc("-", 7);

    addPunc("*", 8);
    addPunc("/", 8);
    addPunc("%", 8);

    addPunc("^", 9);

    addPunc("!", 10);
    addPunc("++", 10);
    addPunc("--", 10);

    // the following are convenience methods for common literals such
    // as: strings, symbols, numerics, etc.
    useString("str");
    useSymbol("sym");
    useReal("real");    
    useFloat("float");
    useInteger("int");

    // the start rule is the first rule given
    addRule("Start", "Stmts");

    // add a non-terminal to catch all numerics
    addRule("Num", "int");
    addRule("Num", "float");
    addRule("Num", "real");

    // begin defining expressions
    addRule("E", "Num");
    addRule("E", "sym");
    addRule("E", "str");

    // elements (terminals or non-terminals) are always separated by
    // spaces

    // here we define the reduction by position (starting at 0 for the
    // first element). if a position is not specified, it defaults to 0
    addRule("E", "( E )", 1);

    // instead of using position, the following rules for expression
    // use a reduction function/lambda
    addRule("E", "E = E", [&](nvec& v){
        return nfunc("VarSet") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E += E", [&](nvec& v){
        return nfunc("AddBy") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E -= E", [&](nvec& v){
        return nfunc("SubBy") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E *= E", [&](nvec& v){
        return nfunc("MulBy") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E /= E", [&](nvec& v){
        return nfunc("DivBy") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E %= E", [&](nvec& v){
        return nfunc("ModBy") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E || E", [&](nvec& v){
        return nfunc("Or") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E && E", [&](nvec& v){
        return nfunc("And") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E == E", [&](nvec& v){
        return nfunc("EQ") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E != E", [&](nvec& v){
        return nfunc("NE") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E < E", [&](nvec& v){
        return nfunc("LT") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E <= E", [&](nvec& v){
        return nfunc("LE") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E > E", [&](nvec& v){
        return nfunc("GT") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E >= E", [&](nvec& v){
        return nfunc("GE") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E << E", [&](nvec& v){
        return nfunc("PushBack") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E + E", [&](nvec& v){
        return nfunc("Add") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E - E", [&](nvec& v){
        return nfunc("Sub") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E * E", [&](nvec& v){
        return nfunc("Mul") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E / E", [&](nvec& v){
        return nfunc("Div") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E % E", [&](nvec& v){
        return nfunc("Mod") << move(v[0]) << move(v[2]);
      });

    addRule("E", "E ^ E", [&](nvec& v){
        return nfunc("Pow") << move(v[0]) << move(v[2]);
      });

    addRule("E", "! E", [&](nvec& v){
        return nfunc("Not") << move(v[0]);
      });

    addRule("E", "++ E", [&](nvec& v){
        return nfunc("Inc") << move(v[1]);
      });

    addRule("E", "-- E", [&](nvec& v){
        return nfunc("Dec") << move(v[1]);
      });

    addRule("E", "sym ++", [&](nvec& v){
        return nfunc("PostInc") << move(v[0]);
      });

    addRule("E", "sym --", [&](nvec& v){
        return nfunc("PostDec") << move(v[0]);
      });

    addRule("Vec", "Vec , Vec", [&](nvec& v){
        v[0].append(v[2]);
        
        return v[0];
      });

    addRule("Vec", "E", [&](nvec& v){
        return nvec() << v[0];
      });

    addRule("E", "[ Vec ]", 1);

    addRule("Params", "Params , Params", [&](nvec& v){
        v[0].append(v[2]);
        
        return v[0];
      });

    addRule("Params", "E", [&](nvec& v){
        return nvec() << v[0];
      });
    
    addRule("E", "sym ( Params )", [&](nvec& v){
        nvar c = nfunc(v[0]);
        c.append(v[2]);
        return move(c);
      });

    // finally, define statements
    addRule("Stmt", "E ;");

    addRule("Stmt", ";", [&](nvec& v){
        return none;
      });

    addRule("Stmt", "break ;", [&](nvec& v){
        return nfunc("Break");
      });

    addRule("Stmt", "continue ;", [&](nvec& v){
        return nfunc("Continue");
      });

    addRule("Stmt", "IfStmt");

    // different types of if stmts

    addRule("IfStmt", "if ( E ) { Stmts }", [&](nvec& v){
        if(v[5].isFunction("Block")){
          v[5].str() = "ScopedBlock";
        }
        return nfunc("If") << move(v[2]) << move(v[5]);
      });

    addRule("IfStmt", "if ( E ) { Stmts } else { Stmts }", [&](nvec& v){
        if(v[5].isFunction("Block")){
          v[5].str() = "ScopedBlock";
        }
        return nfunc("If") << move(v[2]) << move(v[5]) << move(v[9]);
      });

    addRule("IfStmt", "if ( E ) { Stmts } else IfStmt", [&](nvec& v){
        if(v[5].isFunction("Block")){
          v[5].str() = "ScopedBlock";
        }
        return nfunc("If") << move(v[2]) << move(v[5]) << move(v[8]);
      });

    // while stmt

    addRule("Stmt", "while ( E ) { Stmts }", [&](nvec& v){
        return nfunc("While") << move(v[2]) << move(v[5]);
      });

    // for stmt

    addRule("Stmt", "for ( Stmt Stmt E ) { Stmts }", [&](nvec& v){
        return nfunc("For") << move(v[2]) << move(v[3]) << 
          move(v[4]) << move(v[7]);
      });

    // a sequence of statements, defined using left recursion

    addRule("Stmts", "Stmts Stmt", [&](nvec& v){
        if(v[0].isFunction("Block")){
          v[0] << move(v[1]);
          return v[0];
        }
        
        return nfunc("Block") << move(v[0]) << move(v[1]);
      });

    addRule("Stmts", "Stmt");

  }
};

int main(int argc, char** argv){
  Program::opt("help", "h", false, "Display usage");

  Program::opt("show", "s", false, "Show N input/output");  

  Program::opt("initLog", "i", false, "Output parser init log"); 

  Program::opt("runLog", "l", false, "Output to run log"); 

  Program::opt("history", "", 100, "Number of lines to keep in history"); 

  Program::opt("threads", "t", 1, "Number of threads to use in parser");

  Program program(argc, argv);

  nvar args = program.args();
    
  if(args["help"]){
    cout << program.usage("interpreter [OPTION]... [SOURCE FILE]...");
    program.exit(0);
  }

  bool show = args["show"];
  bool runLog = args["runLog"];
  bool initLog = args["initLog"];
  size_t threads = args["threads"];

  // use a plain NObject to interpret our parsed results
  NObject o;
  
  Parser parser;

  nstr log;
  parser.getInitLog(log);
  if(initLog){
    cout << log << endl;
  }

  // this is not NML, but we will use an NMLGenerator to
  // display the final output

  NMLGenerator generator;

  if(args.empty()){
    stifle_history(args["history"]);
    
    bool done = false;

    for(;;){

      nstr code;
      bool first = true;

      for(;;){
        char* l = readline(first ? ">>> " : "");

        if(l){
          nstr line = l;

          line.strip();
          if(!line.empty()){
            add_history(l);
          }
          
          code += line;

          // if we have open braces, brackets, or parens, keep reading
          int open = parser.numOpenMarkers(code);
          if(open <= 0){
            break;
          }
        }
        else{
          done = true;
          break;
        }
      }

      if(done){
        cout << endl;
        break;
      }

      code += ";";

      nvar n = 
        parser.parse(code, true, "", 5.0, &cerr,
                     runLog ? &cout : 0, threads);

      if(n == none){
        continue;
      }

      if(show){
        cout << "<< " << n << endl;
      }

      try{
        // interpret the parsed statements
        nvar r = o.run(n);

        if(show){
          cout << ">> " << r << endl;
        }

        if(r != none){
          generator.generate(cout, r);
          cout << endl;
        }
      }
      catch(NError& e){
        cerr << e << endl;
      }
    }    
  }

  for(size_t i = 0; i < args.size(); ++i){
    const nstr& path = args[i];

    nstr file = NSys::basename(path);
    nstr str = NSys::fileToStr(path);

    nvar n = 
      parser.parse(str, true, file, 5.0, &cerr,
                   runLog ? &cout : 0, threads);

    if(n == none){
      NProgram::exit(1);
    }
    
    if(show){
      cout << "<< " << n << endl;
    }
    
    // interpret the parsed statements
    nvar r = o.run(n);

    if(show){
      cout << ">> " << r << endl;
    }
  }   
  
  NProgram::exit(0);

  return 0;
}

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
#include <fstream>

#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Sema/Sema.h"
#include "clang/AST/Comment.h"

#include <neu/nvar.h>
#include <neu/NSys.h>
#include <neu/NBasicMutex.h>
#include <neu/NProgram.h>
#include <neu/global.h>

using namespace std;
using namespace llvm;
using namespace clang;
using namespace tooling;
using namespace comments;
using namespace neu;
  
class MetaGenerator : public RecursiveASTVisitor<MetaGenerator>{
public:
  typedef vector<string> StringVec;
  typedef vector<CompileCommand> CompileCommandVec;
    
  class Consumer : public ASTConsumer{
  public:
    Consumer(CompilerInstance* ci, MetaGenerator* visitor)
      : ci_(ci),
        visitor_(visitor){}
      
    void HandleTranslationUnit(ASTContext& context){
      visitor_->init(ci_);
      visitor_->TraverseDecl(context.getTranslationUnitDecl());
    }
      
  private:
    MetaGenerator* visitor_;
    CompilerInstance* ci_;
  };
    
  class Action : public ASTFrontendAction {
  public:
    Action(MetaGenerator* visitor)
    : visitor_(visitor){}
      
    ASTConsumer* CreateASTConsumer(CompilerInstance& compilerInstance,
                                   StringRef file){
        
      return new Consumer(&compilerInstance, visitor_);
    }
      
  private:
    MetaGenerator* visitor_;
  };
    
  class Database : public CompilationDatabase{
  public:
    Database(const nstr& resourceDir, const nvec& includes)
    : resourceDir_(resourceDir),
    includes_(includes){}
      
    CompileCommandVec getCompileCommands(StringRef path) const{
      nstr p = path.str();
        
      CompileCommandVec cv;
        
      CompileCommand c;
      c.Directory = ".";
        
      c.CommandLine =
      {"clang-tool", "-std=c++11", "-Wno-undefined-internal",
       "-resource-dir", resourceDir_};

#ifdef __APPLE__
      c.CommandLine.push_back("-stdlib=libc++");
#endif
      
      for(const nstr& i : includes_){
        c.CommandLine.push_back("-I" + i);
      }

      if(p.endsWith(".h")){
        c.CommandLine.push_back("-x");
        c.CommandLine.push_back("c++-header");
      }
        
      c.CommandLine.push_back(p);
      
      cv.push_back(c);
        
      return cv;
    }
      
    CompileCommandVec getAllCompileCommands() const{
      return CompileCommandVec();
    }
      
    StringVec getAllFiles() const{
      return StringVec();
    }
      
  private:
    const nvec& includes_;
    nstr resourceDir_;
  };
    
  class Factory : public FrontendActionFactory{
  public:
    Factory(MetaGenerator* visitor)
    : visitor_(visitor){}
      
    Action* create(){
      return new Action(visitor_);
    }
      
  private:
    MetaGenerator* visitor_;
  };
    
  MetaGenerator()
    : found_(false){
    
    NProgram::use();
    
    includes_.push_back(_home + "/include");
    
    nTypes_.add("void", "Void");
    nTypes_.add("bool", "Bool");
    nTypes_.add("const bool", "Bool");
    nTypes_.add("const bool &", "Bool");
    nTypes_.add("_Bool", "Bool");
    nTypes_.add("const _Bool", "Bool");
    nTypes_.add("const _Bool &", "Bool");
    nTypes_.add("unsigned char", "Char");
    nTypes_.add("const unsigned char", "Char");
    nTypes_.add("const unsigned char &", "Char");
    nTypes_.add("signed char", "Char");
    nTypes_.add("const signed char", "Char");
    nTypes_.add("const signed char &", "Char");
    nTypes_.add("char", "Char");
    nTypes_.add("const char", "Char");
    nTypes_.add("const char &", "Char");
    nTypes_.add("short", "Short");
    nTypes_.add("const short", "Short");
    nTypes_.add("const short &", "Short");
    nTypes_.add("unsigned short", "Short");
    nTypes_.add("const unsigned short", "Short");
    nTypes_.add("const unsigned short &", "Short");
    nTypes_.add("int", "Int");
    nTypes_.add("const int", "Int");
    nTypes_.add("const int &", "Int");
    nTypes_.add("unsigned int", "Int");
    nTypes_.add("const unsigned int", "Int");
    nTypes_.add("const unsigned int &", "Int");
    nTypes_.add("long", "Long");
    nTypes_.add("const long", "Long");
    nTypes_.add("const long &", "Long");
    nTypes_.add("long long", "Long");
    nTypes_.add("const long long", "Long");
    nTypes_.add("const long long &", "Long");
    nTypes_.add("unsigned long", "Long");
    nTypes_.add("const unsigned long", "Long");
    nTypes_.add("const unsigned long &", "Long");
    nTypes_.add("unsigned long long", "Long");
    nTypes_.add("const unsigned long long", "Long");
    nTypes_.add("const unsigned long long &", "Long");
    nTypes_.add("float", "Float");
    nTypes_.add("const float", "Float");
    nTypes_.add("const float &", "Float");
    nTypes_.add("double", "Double");
    nTypes_.add("const double", "Double");
    nTypes_.add("const double &", "Double");

    nTypes_.add("class neu::nvar", "Var");
    nTypes_.add("const class neu::nvar &", "Var");
    nTypes_.add("class neu::nvar &", "VarRef");
    
    nTypes_.add("const class neu::nstr &", "String");
    nTypes_.add("class neu::nstr", "String");
  }
  
  ~MetaGenerator(){}

  bool isNType(const nstr& t){
    return nTypes_.has(t);
  }    

  nstr getType(QualType ct){
    const Type* t = ct.getTypePtr();

    nstr ts = ct.getAsString();
    
    auto itr = nTypes_.find(ts);
    assert(itr != nTypes_.end());

    return itr->second;
  }

  void addInclude(const nstr& path){
    includes_.push_back(path);
  }
  
  void setResourceDir(const nstr& path){
    resourceDir_ = path;
  }
  
  void setClassName(const nstr& className){
    className_ = className;
  }

  void setIncludePrefix(const nstr& includePrefix){
    includePrefix_ = includePrefix;
  }
  
  bool generate(const nstr& outputPath,
                const nstr& headerPath){

    nstr fileName = NSys::fileName(headerPath);

    nstr p = outputPath + "/" + fileName + ".cpp";

    ostr_.open(p.c_str());
    if(ostr_.fail()){
      cerr << "failed to open output file: " << p << endl;
      NProgram::exit(1);
    }

    if(includePrefix_.empty()){
      nvec ps;
      headerPath.split(ps, "/"); 
      nstr includePath;
      bool user = true;

      while(!ps.empty()){
        nstr pi = ps.popBack();
        if(pi == "include"){
          user = false;
          break;
        }

        if(includePath.empty()){
          includePath = pi;
        }
        else{
          includePath.insert(0, pi + "/");
        }
      }

      if(headerPath.beginsWith("/")){
        includePath.insert(0, "/");
      }

      if(user){
        ostr_ << "#include \"" << includePath << "\"" << endl;
      }
      else{
        ostr_ << "#include <" << includePath << ">" << endl;
      }
    }
    else{
      ostr_ << "#include <" << includePrefix_ << "/" << fileName << ">" << endl; 
    }

    ostr_ << endl;

    Database db(resourceDir_, includes_);

    StringVec files = {headerPath};
    ClangTool tool(db, files);
      
    Factory factory(this);

    int status = tool.run(&factory);

    ostr_.close();
    
    if(!found_){
      NERROR("no NJObject subclass found");
    }

    return status == 0;
  }
  
  bool isInMainFile(Decl* d){
    return sema_->SourceMgr.isInMainFile(d->getLocStart());
  }
  
  nstr exprToStr(Expr* expr){
    nstr ret;
    raw_string_ostream ostr(ret.str());
    
    expr->printPretty(ostr, 0, PrintingPolicy(ci_->getLangOpts()));
    
    return ret;
  }
  
  bool VisitCXXRecordDecl(CXXRecordDecl* d){
    if(!d->isCompleteDefinition()){
      return true;
    }

    if(!isInMainFile(d)){
      return true;
    }

    bool found = false;

    for(CXXRecordDecl::base_class_iterator bitr = d->bases_begin(),
        bitrEnd = d->bases_end(); bitr != bitrEnd; ++bitr){

      CXXBaseSpecifier b = *bitr;
      QualType qt = b.getType();
      QualType ct = sema_->Context.getCanonicalType(qt);
      
      if(const RecordType* rt = dyn_cast<RecordType>(ct.getTypePtr())){
        if(CXXRecordDecl* sd = dyn_cast<CXXRecordDecl>(rt->getDecl())){
          if(ct.getAsString() == "class neu::NJObject"){
            found = true;
            break;
          }
        }
      }
    }

    if(!found){
      return true;
    }

    if(found_){
      NERROR("found more than one NJObject subclass");
    }
    
    found_ = true;

    nstr fullClassName = getQualifiedName(d);
    nstr className = d->getNameAsString();

    ostr_ << "namespace{" << endl << endl;
    ostr_ << "  class MethodMap : public neu::NJMethodMap{" << endl;
    ostr_ << "  public:" << endl;
    ostr_ << "    MethodMap() : NJMethodMap(\"" << className_ << "\"){" << endl;

    size_t methodId = 0;

    stringstream mstr;

    for(CXXRecordDecl::method_iterator mitr = d->method_begin(),
          mitrEnd = d->method_end(); mitr != mitrEnd; ++mitr){
      CXXMethodDecl* md = *mitr;
      
      if(!md->isUserProvided()){
        continue;
      }

      nstr methodName = md->getNameAsString();

      if(methodName.empty()){
        continue;
      }

      if(md->getAccess() != AS_public){
        NERROR("non-public method found: " + methodName);
      }

      if(md->isOverloadedOperator() || methodName.beginsWith("operator ")){
        NERROR("operator overload method found: " + methodName);
      }

      if(!isCallable(md)){
        NERROR("method has incompatible return or param types: " + methodName);
      }

      stringstream cstr;

      bool isStatic = md->isStatic();
      bool isVoid = md->getReturnType().getTypePtr()->isVoidType();
      bool isCtor = isa<CXXConstructorDecl>(md);
      
      QualType rct = context_->getCanonicalType(md->getReturnType());

      if(isCtor){
        ostr_ << "      addCtor(" << methodId << ", ";
        mstr << fullClassName << "::" << className << "(";
        cstr << "construct(" << methodId << ", ";
      }
      else{
        ostr_ << "      add(";

        if(isVoid){
          mstr << "void ";
        }
        else{
          mstr << cleanType(rct.getAsString()) << " ";
          cstr << "return ";
        }

        mstr << fullClassName << "::" << methodName << "(";

        if(isStatic){
          cstr << "staticCall(\"" << className_ << "\", " << methodId << ", ";
        }
        else{
          cstr << "call(" << methodId << ", ";
        }

        ostr_ << methodId << ", \"" << methodName << "\", " << 
          getType(rct) << ", ";
      }

      found = false;

      for(FunctionDecl::param_iterator itr = md->param_begin(),
            itrEnd = md->param_end(); itr != itrEnd; ++itr){

        if(found){
          ostr_ << ", ";
          cstr << ", ";
          mstr << ", ";
        }
        else{
          cstr << "{";
          ostr_ << "{";
        }

        ParmVarDecl* p = *itr;
          
        QualType ct = context_->getCanonicalType(p->getType());

        nstr ts = cleanType(ct.getAsString());

        mstr << ts << " " << p->getName().str();

        ts = getType(ct);

        cstr << p->getName().str();

        if(ts == "Var" || ts == "VarRef"){
          cstr << ".toPtr()";
        }

        ostr_ << ts;

        found = true;
      }

      if(found){
        cstr << "}";
        ostr_ << "}";
      }
      else{
        ostr_ << "TypeVec()";
        cstr << "neu::nvec()";
      }

      mstr << ")";

      if(isCtor){
        mstr << " : NJObject(\"" << className_ << "\")";
      }

      mstr << "{" << endl;

      mstr << "  " << cstr.str() << ");" << endl;
      mstr << "}" << endl << endl;

      if(!isCtor){
        if(isStatic){
          ostr_ << ", true";
        }
        else{
          ostr_ << ", false";
        }
      }

      ostr_ << ");" << endl;

      ++methodId;
    }

    ostr_ << "    }" << endl;
    ostr_ << "  };" << endl << endl;
    
    ostr_ << "  MethodMap _methodMap;" << endl << endl;
    ostr_ << "} // end namespace" << endl << endl;
    
    ostr_ << mstr.str();

    return true;
  }
    
  void init(CompilerInstance* ci){
    ci_ = ci;
    sema_ = &ci_->getSema();
    context_ = &ci_->getASTContext();
  }
    
  bool isCompatibleType(QualType qt){
    QualType ct = sema_->Context.getCanonicalType(qt);
      
    return isNType(ct.getAsString());
  }
    
  bool isCallable(FunctionDecl* fd){
    if(!isCompatibleType(fd->getReturnType())){
      return false;
    }
      
    for(CXXMethodDecl::param_iterator itr = fd->param_begin(),
          itrEnd = fd->param_end(); itr != itrEnd; ++itr){
      if(!isCompatibleType((*itr)->getType())){
        return false;
      }
    }
      
    return true;
  }
    
  nstr getQualifiedName(NamedDecl* decl){
    nstr ret = decl->getQualifiedNameAsString();
    ret.findReplace("(anonymous namespace)::", "");
      
    return ret;
  }
  
  nstr cleanType(const nstr& type){
    if(type == "_Bool"){
      return "bool";
    }
  
    return type;
  }

private:
  typedef NMap<nstr, nstr> NTypeMap_;

  NTypeMap_ nTypes_;
  ofstream ostr_;
  CompilerInstance* ci_;
  Sema* sema_;
  ASTContext* context_;
  nvec includes_;
  nstr resourceDir_;
  nstr className_;
  nstr includePrefix_;
  bool found_;
};

void printUsage(){
  cout << NProgram::usage("neu-java [OPTIONS] <header file>") << endl;
}

int main(int argc, char** argv){
  NProgram::opt("help", "h", false,
                "Display usage.");
  
  NProgram::opt("outputPath", "o", "",
                "Path to generate output files in.");

  NProgram::opt("class", "c", "",
                "Java fully qualified class name.", true);
  
  NProgram::opt("resourceDir", "", "",
                "Clang resource dir.",
                true);

  NProgram::opt("includePrefix", "p", "",
                "Include file prefix.");  

  NProgram::opt("sdk", "", "",
                "Apple SDK path.");
  
  NProgram program(argc, argv);

  const nvar& args = program.args();

  if(args["help"]){
    printUsage();
    program.exit(0);
  }
  
  if(args.size() != 1){
    printUsage();
    program.exit(1);
  }
  
  const nstr& headerPath = args[0];
  nstr outputPath = args["outputPath"];

  if(outputPath.empty()){
    outputPath = NSys::currentDir();
  }

  nstr foundSDK;
  
  if(!NSys::getEnv("SDKROOT", foundSDK)){
    const nstr& sdk = args["sdk"];
    
    NSys::setEnv("SDKROOT", sdk);
    NSys::setEnv("SDK_DIR", sdk);
  }
  
  MetaGenerator gen;
  
  gen.setResourceDir(args["resourceDir"]);

  const nstr& className = args["class"];
  gen.setClassName(className);

  const nstr& includePrefix = args["includePrefix"];
  gen.setIncludePrefix(includePrefix);

  const nvar& is = args["include"];

  for(const nstr& i : is){
    gen.addInclude(i);
  }

  if(!gen.generate(outputPath, headerPath)){
    return 1;
  }

  return 0;
}

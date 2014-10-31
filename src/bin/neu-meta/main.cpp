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
#include <neu/NRegex.h>
#include <neu/NBasicMutex.h>
#include <neu/NProgram.h>
#include <neu/global.h>

using namespace std;
using namespace llvm;
using namespace clang;
using namespace tooling;
using namespace comments;
using namespace neu;

NRegex _pointerRegex("(?:const )?class .*::(\\w+) \\*$");
  
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
      {"clang-tool", "-std=c++11", "-DMETA_GUARD", "-Wno-undefined-internal",
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
  : enableHandle_(true),
  enableClass_(true),
  enableMetadata_(true),
  enableOuter_(true){
    
    NProgram::use();
    
    includes_.push_back(_home + "/include");
    
    nTypes_ << "void";
    nTypes_ << "bool";
    nTypes_ << "const bool";
    nTypes_ << "const bool &";
    nTypes_ << "_Bool";
    nTypes_ << "const _Bool";
    nTypes_ << "const _Bool &";
    nTypes_ << "unsigned char";
    nTypes_ << "const unsigned char";
    nTypes_ << "const unsigned char &";
    nTypes_ << "signed char";
    nTypes_ << "const signed char";
    nTypes_ << "const signed char &";
    nTypes_ << "char";
    nTypes_ << "const char";
    nTypes_ << "const char &";
    nTypes_ << "short";
    nTypes_ << "const short";
    nTypes_ << "const short &";
    nTypes_ << "unsigned short";
    nTypes_ << "const unsigned short";
    nTypes_ << "const unsigned short &";
    nTypes_ << "int";
    nTypes_ << "const int";
    nTypes_ << "const int &";
    nTypes_ << "unsigned int";
    nTypes_ << "const unsigned int";
    nTypes_ << "const unsigned int &";
    nTypes_ << "long";
    nTypes_ << "const long";
    nTypes_ << "const long &";
    nTypes_ << "long long";
    nTypes_ << "const long long";
    nTypes_ << "const long long &";
    nTypes_ << "unsigned long";
    nTypes_ << "const unsigned long";
    nTypes_ << "const unsigned long &";
    nTypes_ << "unsigned long long";
    nTypes_ << "const unsigned long long";
    nTypes_ << "const unsigned long long &";
    nTypes_ << "float";
    nTypes_ << "const float";
    nTypes_ << "const float &";
    nTypes_ << "double";
    nTypes_ << "const double";
    nTypes_ << "const double &";
    nTypes_ << "class neu::nvar";
    nTypes_ << "class neu::nvar *";
    nTypes_ << "const class neu::nvar";
    nTypes_ << "const class neu::nvar &";
    nTypes_ << "class neu::nvar &";
    nTypes_ << "class neu::nstr";
    nTypes_ << "const class neu::nstr";
    nTypes_ << "const class neu::nstr &";
    nTypes_ << "class neu::nstr &";
    nTypes_ << "class std::basic_string<char>";
    nTypes_ << "const class std::basic_string<char>";
    nTypes_ << "const class std::basic_string<char> &";
    
    nTypes_ << "class neu::NVector<class neu::nvar, "
                  "class std::allocator<class neu::nvar> >";
    nTypes_ << "class neu::NVector<class neu::nvar, "
                  "class std::__1::allocator<class neu::nvar> >";
    
    nTypes_ << "const class neu::NVector<class neu::nvar, "
                  "class std::allocator<class neu::nvar> >";
    nTypes_ << "const class neu::NVector<class neu::nvar, "
                  "class std::__1::allocator<class neu::nvar> >";
    
    nTypes_ << "const class neu::NVector<class neu::nvar, "
                  "class std::allocator<class neu::nvar> > &";
    nTypes_ << "const class neu::NVector<class neu::nvar, "
                  "class std::__1::allocator<class neu::nvar> > &";
    
    nTypes_ << "class neu::NList<class neu::nvar, "
                  "class std::allocator<class neu::nvar> >";
    nTypes_ << "class neu::NList<class neu::nvar, "
                  "class std::__1::allocator<class neu::nvar> >";
    
    nTypes_ << "const class neu::NList<class neu::nvar, "
                  "class std::allocator<class neu::nvar> >";
    nTypes_ << "const class neu::NList<class neu::nvar, "
                  "class std::__1::allocator<class neu::nvar> >";
    
    nTypes_ << "const class neu::NList<class neu::nvar, "
                  "class std::allocator<class neu::nvar> > &";
    nTypes_ << "const class neu::NList<class neu::nvar, "
                  "class std::__1::allocator<class neu::nvar> > &";

    nTypes_ << "class neu::NQueue<class neu::nvar, "
    "class std::allocator<class neu::nvar> >";

    nTypes_ << "class neu::NQueue<class neu::nvar, "
    "class std::__1::allocator<class neu::nvar> >";
    
    nTypes_ << "const class neu::NQueue<class neu::nvar, "
    "class std::allocator<class neu::nvar> >";
    
    nTypes_ << "const class neu::NQueue<class neu::nvar, "
    "class std::__1::allocator<class neu::nvar> >";
    
    nTypes_ << "const class neu::NQueue<class neu::nvar, "
    "class std::allocator<class neu::nvar> > &";
    
    nTypes_ << "const class neu::NQueue<class neu::nvar, "
    "class std::__1::allocator<class neu::nvar> > &";
    
    nTypes_ << "class neu::NSet<class neu::nvar, "
    "struct neu::nvarLess<class neu::nvar>, "
    "class std::allocator<class neu::nvar> >";
    
    nTypes_ << "class neu::NSet<class neu::nvar, "
    "struct neu::nvarLess<class neu::nvar>, "
    "class std::__1::allocator<class neu::nvar> >";
    
    nTypes_ << "const class neu::NSet<class neu::nvar, "
    "struct neu::nvarLess<class neu::nvar>, "
    "class std::allocator<class neu::nvar> >";
    
    nTypes_ << "const class neu::NSet<class neu::nvar, "
    "struct neu::nvarLess<class neu::nvar>, "
    "class std::__1::allocator<class neu::nvar> >";
    
    nTypes_ << "const class neu::NSet<class neu::nvar, "
    "struct neu::nvarLess<class neu::nvar>, "
    "class std::allocator<class neu::nvar> > &";
    
    nTypes_ << "const class neu::NSet<class neu::nvar, "
    "struct neu::nvarLess<class neu::nvar>, "
    "class std::__1::allocator<class neu::nvar> > &";
    
    nTypes_ << "class NHashSet<class neu::nvar, "
    "struct neu::nvarHash<class neu::nvar>, "
    "struct std::equal_to<class neu::nvar>, "
    "class std::allocator<class neu::nvar> >";
    
    nTypes_ << "class NHashSet<class neu::nvar, "
    "struct neu::nvarHash<class neu::nvar>, "
    "struct std::equal_to<class neu::nvar>, "
    "class std::allocator<class neu::nvar> >";
    
    nTypes_ << "const class NHashSet<class neu::nvar, "
    "struct neu::nvarHash<class neu::nvar>, "
    "struct std::equal_to<class neu::nvar>, "
    "class std::allocator<class neu::nvar> >";
    
    nTypes_ << "const class NHashSet<class neu::nvar, "
    "struct neu::nvarHash<class neu::nvar>, "
    "struct std::equal_to<class neu::nvar>, "
    "class std::allocator<class neu::nvar> >";
    
    nTypes_ << "const class NHashSet<class neu::nvar, "
    "struct neu::nvarHash<class neu::nvar>, "
    "struct std::equal_to<class neu::nvar>, "
    "class std::allocator<class neu::nvar> > &";
    
    nTypes_ << "const class NHashSet<class neu::nvar, "
    "struct neu::nvarHash<class neu::nvar>, "
    "struct std::equal_to<class neu::nvar>, "
    "class std::allocator<class neu::nvar> > &";

    nTypes_ << "class neu::NMap<class neu::nvar, "
    "class neu::nvar, struct neu::nvarLess<class neu::nvar>, "
    "class std::allocator<struct std::pair"
    "<const class neu::nvar, class neu::nvar> > >";
    
    nTypes_ << "class neu::NMap<class neu::nvar, "
    "class neu::nvar, struct neu::nvarLess<class neu::nvar>, "
    "class std::__1::allocator<struct std::__1::pair"
    "<const class neu::nvar, class neu::nvar> > >";
    
    nTypes_ << "const class neu::NMap<class neu::nvar, "
    "class neu::nvar, struct neu::nvarLess<class neu::nvar>, "
    "class std::allocator<struct std::pair"
    "<const class neu::nvar, class neu::nvar> > >";
    
    nTypes_ << "const class neu::NMap<class neu::nvar, "
    "class neu::nvar, struct neu::nvarLess<class neu::nvar>, "
    "class std::__1::allocator<struct std::__1::pair"
    "<const class neu::nvar, class neu::nvar> > >";
    
    nTypes_ << "const class neu::NMap<class neu::nvar, "
    "class neu::nvar, struct neu::nvarLess<class neu::nvar>, "
    "class std::allocator<struct std::pair"
    "<const class neu::nvar, class neu::nvar> > > &";
    
    nTypes_ << "const class neu::NMap<class neu::nvar, "
    "class neu::nvar, struct neu::nvarLess<class neu::nvar>, "
    "class std::__1::allocator<struct std::__1::pair"
    "<const class neu::nvar, class neu::nvar> > > &";
    
    nTypes_ << "class neu::NHashMap<class neu::nvar, class neu::nvar, "
    "struct neu::nvarHash<class neu::nvar>, struct neu::nvarEqual<class "
    "neu::nvar>, class std::allocator<struct std::pair<const "
    "class neu::nvar, class neu::nvar> > >";
    
    nTypes_ << "class neu::NHashMap<class neu::nvar, class neu::nvar, "
    "struct neu::nvarHash<class neu::nvar>, struct neu::nvarEqual<class "
    "neu::nvar>, class std::__1::allocator<struct std::__1::pair<const "
    "class neu::nvar, class neu::nvar> > >";
    
    nTypes_ << "const class neu::NHashMap<class neu::nvar, class neu::nvar, "
    "struct neu::nvarHash<class neu::nvar>, struct neu::nvarEqual<class "
    "neu::nvar>, class std::allocator<struct std::pair<const "
    "class neu::nvar, class neu::nvar> > >";
    
    nTypes_ << "const class neu::NHashMap<class neu::nvar, class neu::nvar, "
    "struct neu::nvarHash<class neu::nvar>, struct neu::nvarEqual<class "
    "neu::nvar>, class std::__1::allocator<struct std::__1::pair<const "
    "class neu::nvar, class neu::nvar> > >";
    
    nTypes_ << "const class neu::NHashMap<class neu::nvar, class neu::nvar, "
    "struct neu::nvarHash<class neu::nvar>, struct neu::nvarEqual<class "
    "neu::nvar>, class std::allocator<struct std::pair<const "
    "class neu::nvar, class neu::nvar> > > &";
    
    nTypes_ << "const class neu::NHashMap<class neu::nvar, class neu::nvar, "
    "struct neu::nvarHash<class neu::nvar>, struct neu::nvarEqual<class "
    "neu::nvar>, class std::__1::allocator<struct std::__1::pair<const "
    "class neu::nvar, class neu::nvar> > > &";
    
    nTypes_ << "class neu::NMultimap<class neu::nvar, class neu::nvar, "
    "struct neu::nvarLess<class neu::nvar>, class std::allocator<struct "
    "std::pair<const class neu::nvar, class neu::nvar> > >";
    
    nTypes_ << "class neu::NMultimap<class neu::nvar, class neu::nvar, "
    "struct neu::nvarLess<class neu::nvar>, class std::__1::allocator<struct "
    "std::__1::pair<const class neu::nvar, class neu::nvar> > >";
    
    nTypes_ << "const class neu::NMultimap<class neu::nvar, class neu::nvar, "
    "struct neu::nvarLess<class neu::nvar>, class std::allocator<struct "
    "std::pair<const class neu::nvar, class neu::nvar> > >";
    
    nTypes_ << "const class neu::NMultimap<class neu::nvar, class neu::nvar, "
    "struct neu::nvarLess<class neu::nvar>, class std::__1::allocator<struct "
    "std::__1::pair<const class neu::nvar, class neu::nvar> > >";
    
    nTypes_ << "const class neu::NMultimap<class neu::nvar, class neu::nvar, "
    "struct neu::nvarLess<class neu::nvar>, class std::allocator<struct "
    "std::pair<const class neu::nvar, class neu::nvar> > > &";
    
    nTypes_ << "const class neu::NMultimap<class neu::nvar, class neu::nvar, "
    "struct neu::nvarLess<class neu::nvar>, class std::__1::allocator<struct "
    "std::__1::pair<const class neu::nvar, class neu::nvar> > > &";
    
    nTypes_ << "class neu::nrat";
    nTypes_ << "const class neu::nrat";
    nTypes_ << "const class neu::nrat &";
    nTypes_ << "class neu::nreal";
    nTypes_ << "const class neu::nreal";
    nTypes_ << "const class neu::nreal &";
    nTypes_ << "class neu::nstr *";
  }
  
  ~MetaGenerator(){}

  bool isNType(const nstr& t){
    return nTypes_.has(t);
  }    

  void enableHandle(bool flag){
    enableHandle_ = flag;
  }
    
  void enableClass(bool flag){
    enableClass_ = flag;
  }
    
  void enableMetadata(bool flag){
    enableMetadata_ = flag;
  }

  void enableOuter(bool flag){
    enableOuter_ = flag;
  }    

  void addInclude(const nstr& path){
    includes_.push_back(path);
  }
  
  void setResourceDir(const nstr& path){
    resourceDir_ = path;
  }
  
  bool generate(ostream& ostr, const nstr& filePath, const nstr& className){
    ostr_ = &ostr;
    className_ = className;
    fullClassName_ = className_.find("::") != nstr::npos;

    ostr << "#include <neu/nvar.h>" << endl;
      
    if(enableHandle_){
      ostr << "#include <neu/NFuncMap.h>" << endl;
    }
      
    if(enableClass_){
      ostr << "#include <neu/NClass.h>" << endl;
    }

    ostr << endl;

    Database db(resourceDir_, includes_);

    StringVec files = {filePath};
    ClangTool tool(db, files);
      
    Factory factory(this);

    int status = tool.run(&factory);

    return status == 0;
  }
  
  CXXRecordDecl* getFirstSubClass(CXXRecordDecl* rd,
                                  const string& className){
    if(!rd->hasDefinition()){
      return 0;
    }
      
    for(CXXRecordDecl::base_class_iterator bitr = rd->bases_begin(),
          bitrEnd = rd->bases_end(); bitr != bitrEnd; ++bitr){
        
      CXXBaseSpecifier b = *bitr;
      QualType qt = b.getType();
        
      QualType ct = sema_->Context.getCanonicalType(qt);
      
      if(ct.getAsString() == "class " + className){
        return rd;
      }
      
      const Type* t = ct.getTypePtr();
        
      if(const RecordType* rt = dyn_cast<RecordType>(t)){
        if(CXXRecordDecl* srd = dyn_cast<CXXRecordDecl>(rt->getDecl())){
          CXXRecordDecl* s = getFirstSubClass(srd, className);
          if(s){
            return s;
          }
        }
      }
    }
      
    return 0;
  }

  CXXRecordDecl* getNextSubClass(CXXRecordDecl* rd,
                                 const string& className){
    if(!rd->hasDefinition()){
      return 0;
    }
    
    for(CXXRecordDecl::base_class_iterator bitr = rd->bases_begin(),
          bitrEnd = rd->bases_end(); bitr != bitrEnd; ++bitr){
        
      CXXBaseSpecifier b = *bitr;
      QualType qt = b.getType();
        
      QualType ct = sema_->Context.getCanonicalType(qt);
      
      if(ct.getAsString() == "class " + className){
        return rd;
      }
      
      const Type* t = ct.getTypePtr();
        
      if(const RecordType* rt = dyn_cast<RecordType>(t)){
        if(CXXRecordDecl* srd = dyn_cast<CXXRecordDecl>(rt->getDecl())){
          if(getFirstSubClass(srd, className)){
            return srd;
          }
        }
      }
    }
    
    return 0;
  }
    
  bool isInMainFile(Decl* d){
    return sema_->SourceMgr.isInMainFile(d->getLocStart());
  }

  bool VisitCXXRecordDecl(CXXRecordDecl* d){
    if(!d->isCompleteDefinition()){
      return true;
    }
    
    nstr className;
    if(fullClassName_){
      className = getQualifiedName(d);
    }
    else{
      className = d->getNameAsString();
    }

    if(className != className_){
      return true;
    }

    if(enableOuter_){
      generateOuter(d);
    }
    
    CXXRecordDecl* s = getFirstSubClass(d, "neu::NObject");
    
    if(!s){
      return true;
    }
    
    if(enableHandle_){
      generateHandler(s, d);
    }
    
    if(enableClass_){
      generateClass(d);
    }

    return true;
  }
    
  void init(CompilerInstance* ci){
    ci_ = ci;
    sema_ = &ci_->getSema();
    context_ = &ci_->getASTContext();
  }
    
  bool isBaseType(QualType qt, const string& baseType){
    QualType bct = sema_->Context.getCanonicalType(qt);
      
    if(const RecordType* rt = dyn_cast<RecordType>(bct)){
      if(CXXRecordDecl* rd = dyn_cast<CXXRecordDecl>(rt->getDecl())){
        if(!rd->hasDefinition()){
          return false;
        }
          
        for(CXXRecordDecl::base_class_iterator bitr = rd->bases_begin(),
              bitrEnd = rd->bases_end(); bitr != bitrEnd; ++bitr){
          CXXBaseSpecifier b = *bitr;
            
          QualType ct = sema_->Context.getCanonicalType(b.getType());
            
          if(ct.getAsString() == "class " + baseType){
            return true;
          }
            
          if(isBaseType(ct, baseType)){
            return true;
          }
        }
      }
    }

    return false;
  }
    
  int isNCompatibleType(QualType qt){
    QualType ct = sema_->Context.getCanonicalType(qt);
      
    if(isNType(ct.getAsString())){
      return 1;
    }
      
    const Type* t = qt.getTypePtr();
      
    if(t->isPointerType()){
      if(isBaseType(t->getPointeeType(), "neu::NObjectBase")){
        return 2;
      }
    }
    else if(isBaseType(qt, "neu::NObjectBase")){
      return 3;
    }
      
    return 0;
  }
    
  int isNCallable(FunctionDecl* fd){
    if(!isNCompatibleType(fd->getReturnType())){
      return 0;
    }
      
    int t = 1;
    for(CXXMethodDecl::param_iterator itr = fd->param_begin(),
          itrEnd = fd->param_end(); itr != itrEnd; ++itr){
      int ti = isNCompatibleType((*itr)->getType());
      if(!ti){
        return 0;
      }
        
      if(ti == 2){
        if(t < 2){
          t = 2;
        }
      }
      else if(ti == 3){
        t = 3;
      }
    }
      
    return t;
  }
    
  void generateHandler(CXXRecordDecl* srd, CXXRecordDecl* rd){
    stringstream ostr;
      
    nstr className = rd->getNameAsString();
    nstr fullClassName = getQualifiedName(rd);
      
    ostr << "namespace{" << endl << endl;

    ostr << "class " << className << 
      "_FuncMap : public neu::NFuncMap{" << endl;
    ostr << "public:" << endl;
    ostr << "  " << className << "_FuncMap(){" << endl;
      
    typedef NVector<CXXMethodDecl*> MethodVec;
    MethodVec mv;
      
    for(CXXRecordDecl::method_iterator mitr = rd->method_begin(),
          mitrEnd = rd->method_end(); mitr != mitrEnd; ++mitr){
      CXXMethodDecl* md = *mitr;
        
      if(md->isUserProvided() &&
         !md->isOverloadedOperator() &&
         md->getAccess() == AS_public &&
         !(isa<CXXConstructorDecl>(md) || isa<CXXDestructorDecl>(md))){
          
        nstr methodName = md->getNameAsString();
          
        if(methodName.empty() ||
           methodName == "handle" ||
           methodName.beginsWith("operator ")){
          continue;
        }
        
        int m = isNCallable(md);
          
        if(m > 0 && m < 3){
          mv.push_back(md);
        }
      }
    }
      
    if(mv.empty()){
      return;
    }
      
    for(size_t i = 0; i < mv.size(); ++i){
      if(i > 0){
        ostr << endl;
      }
        
      CXXMethodDecl* md = mv[i];
        
      bool isVoid = md->getReturnType().getTypePtr()->isVoidType();
        
      for(size_t j = md->getMinRequiredArguments(); 
          j <= md->param_size(); ++j){
        ostr << "    add(\"" << md->getNameAsString() << "\", " << j <<
          ", " << endl;
        
        ostr << "      [](void* o, const nstr& f, neu::nvec& v) -> " <<
        "neu::nvar{" << endl;
        
        ostr << "        ";
          
        if(!isVoid){
          ostr << "return";
        }
        
        ostr << " static_cast<" << fullClassName << "*>(o)->";
        ostr << md->getNameAsString() << "(";
          
        for(size_t k = 0; k < j; ++k){
          if(k > 0){
            ostr << ", ";
          }
            
          ParmVarDecl* p = md->getParamDecl(k);
          
          QualType qt = p->getType();
          const Type* t = qt.getTypePtr();
            
          if(t->isPointerType() &&
             isBaseType(t->getPointeeType(), "neu::NObjectBase")){
              
            nstr cn = t->getPointeeType().getAsString();

            cn.findReplace("<anonymous>::", "");
            cn.findReplace("class ", "");
              
            ostr << "v[" << k << "].ptr<" << cn << ">()";
          }
          else if(t->isReferenceType()){
            nstr qs = qt.getAsString();
            
            if(qs == "class neu::nvar &"){
              ostr << "*";
            }

            ostr << "v[" << k << "]";
          }
          else{
            ostr << "v[" << k << "]";
          }
        }
        ostr << ");" << endl;
          
        if(isVoid){
          ostr << "      return neu::none;" << endl;
        }
        ostr << "    });" << endl;
      }
    }
      
    ostr << "  }" << endl;
      
    ostr << "};" << endl << endl;
      
    ostr << "" << className << "_FuncMap _" << className <<
      "_FuncMap;" << endl << endl;
      
    ostr << "} // end namespace" << endl << endl;
    
    ostr << "neu::NFunc " << fullClassName << 
      "::handle(const neu::nvar& n, uint32_t flags){" << endl;

    if(srd == rd){
      ostr << "  return _" << className <<
        "_FuncMap.map(n) ? : neu::NObject::handle(n);" << endl;
    }
    else{
      ostr << "  return _" << className << "_FuncMap.map(n) ? : ";
      ostr << getQualifiedName(getNextSubClass(rd, "neu::NObject"));
      ostr << "::handle(n, flags);" << endl;
    }
      
    ostr << "}" << endl;
    
    *ostr_ << ostr.str() << endl;
  }

  nstr getQualifiedName(NamedDecl* decl){
    nstr ret = decl->getQualifiedNameAsString();
    ret.findReplace("(anonymous namespace)::", "");
      
    return ret;
  }
  
  // ndm - is there a better way to do this?
  bool isInUnnamedNamespace(NamedDecl* decl){
    nstr ret = decl->getQualifiedNameAsString();
    return ret.find("(anonymous namespace)::") != nstr::npos;
  }
  
  void generateClass(CXXRecordDecl* rd){
    nstr name = rd->getName().str();
    nstr fullName = getQualifiedName(rd);
      
    stringstream ostr;
    
    bool foundDist = false;
    
    for(CXXRecordDecl::method_iterator mitr = rd->method_begin(),
          mitrEnd = rd->method_end(); mitr != mitrEnd; ++mitr){
    
      CXXMethodDecl* md = *mitr;
      
      if(!foundDist){
        QualType qt = md->getReturnType();
        QualType ct = context_->getCanonicalType(qt);
        
        if(qt.getAsString() == "ndist" &&
           ct.getAsString() == "class neu::nvar"){
          foundDist = true;
        }
      }
    }

    ostr << "namespace{" << endl << endl;
      
    ostr << "class " << name << "_Class : public neu::NClass{" << endl;
    ostr << "public: " << endl;
      
    ostr << "  " << name << "_Class() : NClass(\"" << fullName << "\"){";
      
    if(enableMetadata_){
      ostr << endl;
      ostr << "    setMetadata(metadata_());" << endl;
      ostr << "  }";
    }
    else{
      ostr << "}";
    }
      
    ostr << endl;

    if(!rd->isAbstract()){
      ostr << endl;

      ostr << "  neu::NObjectBase* construct(const neu::nvar& f){" << endl;
      
      for(CXXRecordDecl::method_iterator mitr = rd->method_begin(),
            mitrEnd = rd->method_end(); mitr != mitrEnd; ++mitr){
        
        CXXMethodDecl* md = *mitr;
        
        if(md->isUserProvided() && isa<CXXConstructorDecl>(md)){
          int m = isNCallable(md);
          
          if(m != 1 && m != 2){
            continue;
          }
          
          for(size_t k = md->getMinRequiredArguments();
              k <= md->param_size(); ++k){
            
            ostr << "    if(f.size() == " << k << "){" << endl;
            ostr << "      return new " << fullName << "(";
            
            for(size_t i = 0; i < k; ++i){
              if(i > 0){
                ostr << ", ";
              }
              
              ParmVarDecl* p = md->getParamDecl(i);
              
              const Type* t = p->getType().getTypePtr();
              
              if((t->isPointerType() || t->isReferenceType()) &&
                 isBaseType(t->getPointeeType(), "neu::NObjectBase")){
                
                nstr cn = t->getPointeeType().getAsString();
                
                nstr c = cn.substr(0, 6);
                
                assert(c == "class ");
                nstr name = cn.substr(6);
                
                if(!t->isPointerType()){
                  ostr << "*";
                }
                
                ostr << "f[" << i << "].ptr<" << fullName << ">()";
              }
              else{
                ostr << "f[" << i << "]";
              }
            }
            ostr << ");" << endl;
            ostr << "    }" << endl;
          }
        }
      }
      ostr << "    return 0;" << endl;
      ostr << "  }" << endl;
      
      if(foundDist){
        ostr << endl;
        ostr << "  neu::NObject* constructRemote(neu::NBroker* broker){" << endl;
        ostr << "    return new " << fullName << "(broker);" << endl;
        ostr << "  }" << endl;
      }
    }

    if(enableMetadata_){
      ostr << endl;
      generateMetadata(ostr, rd);
    }
      
    ostr << "};" << endl << endl;
      
    ostr << name << "_Class* _" << name << "_Class = new " <<
      name << "_Class;" << endl << endl;

    ostr << "} // end namespace" << endl << endl;

    *ostr_ << ostr.str();
  }
    
  void commentToStr(Comment* c, nstr& str){
    for(Comment::child_iterator itr = c->child_begin(),
          itrEnd = c->child_end(); itr != itrEnd; ++itr){

      Comment* ci = *itr;

      if(TextComment* tc = dyn_cast<TextComment>(ci)){
        nstr cs = tc->getText().str();
        cs.strip();

        str += cs;
      }
      else{
        commentToStr(ci, str);
      }
    }
  }
    
  nstr getDeclComment(Decl* d){
    FullComment* comment =
      context_->getCommentForDecl(d, &ci_->getPreprocessor());
      
    if(comment){
      nstr cs;
      commentToStr(comment, cs);
      return cs;
    }
      
    return "";
  }

  bool hasOuter(CXXRecordDecl* rd){
    for(CXXRecordDecl::field_iterator itr = rd->field_begin(),
          itrEnd = rd->field_end(); itr != itrEnd; ++itr){
      FieldDecl* fd = *itr;
      if(fd->getNameAsString() == "x_"){
        return true;
      }
    }
    
    return false;
  }
  
  nstr cleanType(const nstr& type){
    if(type == "_Bool"){
      return "bool";
    }
  
    return type;
  }

  void generateOuter(CXXRecordDecl* rd){
    if(!hasOuter(rd)){
      return;
    }

    nstr name = rd->getNameAsString();
    nstr fullName = getQualifiedName(rd);

    stringstream ostr;
  
    for(CXXRecordDecl::method_iterator mitr = rd->method_begin(),
          mitrEnd = rd->method_end(); mitr != mitrEnd; ++mitr){
      CXXMethodDecl* md = *mitr;

      nstr methodName = md->getNameAsString();
    
      if(md->isStatic() ||
         md->hasInlineBody() ||
         md->hasBody() ||
         !md->isUserProvided() ||
         md->isPure() ||
         methodName == "handle"){
        continue;
      }
      else if(isa<CXXDestructorDecl>(md)){
        ostr << fullName << "::~" << name << "(){" << endl;
        ostr << "  delete x_;" << endl;
        ostr << "}" << endl;
      }
      else{
        bool isDist = false;

        if(isa<CXXConstructorDecl>(md)){
          ostr << fullName << "::" << name << "(";
        }
        else{
          QualType qrt = md->getReturnType();
          QualType crt = context_->getCanonicalType(qrt);
        
          nstr rts = cleanType(crt.getAsString());
        
          ostr << rts << " " << fullName << "::" << methodName << "(";
        
          if(qrt.getAsString() == "ndist" &&
             crt.getAsString() == "class neu::nvar"){
            
            if(CXXRecordDecl* srd = getFirstSubClass(rd, "neu::NObject")){
              int m = isNCallable(md);
              if(m == 1){
                isDist = true;
              }
            }
          }
        }
    
        stringstream pstr;
        bool first = true;
      
        for(FunctionDecl::param_iterator itr = md->param_begin(),
              itrEnd = md->param_end(); itr != itrEnd; ++itr){
          ParmVarDecl* p = *itr;
          if(first){
            if(isa<CXXConstructorDecl>(md)){
              pstr << ", ";
            }
            first = false;
          }
          else{
            ostr << ", ";
            pstr << ", ";
          }
          QualType ct = context_->getCanonicalType(p->getType());
          nstr ts = ct.getAsString();
          ts = cleanType(ts);
        
          ostr << ts << " " << p->getName().str();
          pstr << p->getName().str();
        }
        ostr << ")";
      
        if(md->getTypeQualifiers() & Qualifiers::Const){
          ostr << " const";
        }
      
        ostr << "{" << endl;
      
        if(isDist){
          ostr << "  if(isRemote()){" << endl;
          ostr << "    return remoteRun(nfunc(\"" << methodName <<
            "\")";
          for(FunctionDecl::param_iterator itr = md->param_begin(),
                itrEnd = md->param_end(); itr != itrEnd; ++itr){
            ParmVarDecl* p = *itr;
            QualType qt = p->getType();
            QualType ct = context_->getCanonicalType(qt);
            nstr ts = ct.getAsString();
          
            ostr << " << ";
          
            if(ts == "const class neu::nvar &" ||
               ts == "class neu::nvar &" ||
               ts == "class neu::nstr &"){
              ostr << "neu::nvar(&" << p->getName().str() << ", nvar::Ptr)";
            }
            else{
              ostr << p->getName().str();
            }
          }
          ostr << ");" << endl;
          ostr << "  }" << endl;
        }
      
        ostr << "  ";
      
        if(isa<CXXConstructorDecl>(md)){
          ostr << "x_ = new class " << name << "_(this" << pstr.str() << 
            ");" << endl;
        }
        else{
          if(!md->getReturnType().getTypePtr()->isVoidType()){
            ostr << "return ";
          }
          ostr << "x_->" << md->getName().str() << "(";
      
          ostr << pstr.str() << ");" << endl;
        }
      
        ostr << "}" << endl << endl;
      }
    }

    *ostr_ << ostr.str();
  }

   
  void generateMetadata(ostream& ostr, CXXRecordDecl* rd){
    nstr className = rd->getNameAsString();
      
    CXXRecordDecl* srd = getNextSubClass(rd, "neu::NObject");
      
    ostr << "  neu::nvar metadata_(){" << endl;
      
    ostr << "    neu::nvar ret;" << endl;
      
    ostr << "    neu::nvar& c = ret(\"" << className << "\");" << endl;
      
    nstr cs = getDeclComment(rd);
    if(!cs.empty()){
      cs.escapeForC();
      ostr << "    c(\"comment\") = \"" << cs << "\";" << endl;
    }
      
    nstr en;
      
    if(srd){
      en = srd->getNameAsString();
    }
    else{
      en = "NObject";
    }
      
    ostr << "    c(\"extends\") = neu::nvar(\"" << en <<
      "\", neu::nvar::Sym);" << endl;

    ostr << "    neu::nvar& m = c(\"methods\");" << endl;
      
    for(CXXRecordDecl::method_iterator mitr = rd->method_begin(),
          mitrEnd = rd->method_end(); mitr != mitrEnd; ++mitr){
      CXXMethodDecl* md = *mitr;
        
      if(md->isUserProvided() &&
         md->getAccess() == AS_public &&
         !md->isOverloadedOperator() &&
         !isa<CXXConstructorDecl>(md) &&
         !isa<CXXDestructorDecl>(md)){
          
        nstr methodName = md->getNameAsString();

        if(methodName.empty() ||
           methodName == "handle" ||
           methodName.beginsWith("operator ")){
          continue;
        }          

        int m = isNCallable(md);
          
        if(m > 0 && m < 3){
          for(size_t k = md->getMinRequiredArguments();
              k <= md->param_size(); ++k){
          
            ostr << "    {" << endl;
              
            nstr methodName = md->getNameAsString();
              
            ostr << "      neu::nvar mk({neu::nvar(\"" << methodName <<
              "\", neu::nvar::Sym), neu::nvar(" << k << ")});" << endl;
              
            ostr << "      neu::nvar& mi = m(mk);" << endl;
              
            nstr cs = getDeclComment(md);
              
            if(!cs.empty()){
              cs.escapeForC();
                
              ostr << "      mi(\"comment\") = \"" << cs << "\";" << endl;
            }
              
            ostr << "      mi(\"const\") = ";
              
            if(md->getTypeQualifiers() & Qualifiers::Const){
              ostr << "true";
            }
            else{
              ostr << "false";
            }
              
            ostr << ";" << endl;
              
            if(!md->getReturnType().getTypePtr()->isVoidType()){
              ostr << "      neu::nvar& ri = mi(\"return\");" << endl;
                
              nstr rawType = md->getReturnType().getAsString();
                
              nvec m;
              if(_pointerRegex.match(rawType, m)){
                ostr << "      ri(\"type\") = \"" << m[1].str() << 
                  "\";" << endl;
              }
                
              ostr << "      ri(\"const\") = ";
                
              if(rawType.find("const") == 0){
                ostr << "true";
              }
              else{
                ostr << "false";
              }
                
              ostr << ";" << endl;
            }
              
            for(size_t i = 0; i < k; ++i){
              ostr << "      {" << endl;
                
              ParmVarDecl* p = md->getParamDecl(i);
              nstr rawType = p->getType().getAsString();
                
              ostr << "        mi.pushBack(neu::nvar(\"" <<
                p->getName().str() << "\", neu::nvar::Sym));" << endl;
              ostr << "        neu::nvar& pi = mi.back();" << endl;

              nvec m;
              if(_pointerRegex.match(rawType, m)){
                ostr << "        pi(\"type\") = \"" << 
                  m[1].str() << "\";" << endl;
              }
                
              ostr << "        pi(\"const\") = ";
                
              if(rawType.find("const") == 0){
                ostr << "true";
              }
              else{
                ostr << "false";
              }
                
              ostr << ";" << endl;
                
              ostr << "      }" << endl;
            }
            ostr << "    }" << endl;
          }
        }
      }
    }
    
    ostr << "    return ret;" << endl;
    
    ostr << "  }" << endl;
  }

private:
  typedef NHashSet<nstr> NTypeSet_;

  NTypeSet_ nTypes_;
  ostream* ostr_;
  CompilerInstance* ci_;
  Sema* sema_;
  ASTContext* context_;
  nvec includes_;
  bool enableHandle_;
  bool enableClass_;
  bool enableMetadata_;
  bool enableOuter_;
  nstr className_;
  bool fullClassName_;
  nstr resourceDir_;
};

void printUsage(){
  cout << NProgram::usage("neu-meta [OPTIONS] <source file>") << endl;
}

int main(int argc, char** argv){
  NProgram::opt("help", "h", false,
                "Display usage.");
  
  NProgram::opt("class", "", "",
                "Class name to generate metadata for. "
                "Defaults to the name of the source file.");
  
  NProgram::opt("handle", "", true,
                "True to generate handler.");

  NProgram::opt("factory", "", true,
                "True to generate class.");
  
  NProgram::opt("metadata", "", true,
                "True to generate class metadata.");
  
  NProgram::opt("outer", "", true,
                "True to generate outer.");
  
  NProgram::opt("include", "I", "",
                "Include paths.", false, true);

  NProgram::opt("resourceDir", "", "",
                "Clang resource directory.", true);
  
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

  const nstr& filePath = args[0];

  nstr className = args["class"];
  if(className.empty()){
    className = NSys::fileName(filePath);
  }

  nstr metaPath = className + "_meta.h";

  ofstream out(metaPath.c_str());
  if(out.fail()){
    cerr << "failed to open output file: " << metaPath << endl;
    program.exit(1);
  }
  out.close();

  stringstream ostr;
  MetaGenerator gen;
  
  gen.setResourceDir(args["resourceDir"]);
  gen.enableHandle(args["handle"]);
  gen.enableClass(args["factory"]);
  gen.enableMetadata(args["metadata"]);
  gen.enableOuter(args["outer"]);

  const nvar& is = args["include"];

  for(const nstr& i : is){
    gen.addInclude(i);
  }
  
  if(!gen.generate(ostr, filePath, className)){
    return 1;
  }

  ofstream finalOut(metaPath.c_str());
  if(finalOut.fail()){
    cerr << "failed to open output file: " << metaPath << endl;
    program.exit(1);
  }
  finalOut << ostr.str();
  finalOut.close();
  
  return 0;
}

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
 
 
Neu, Copyright (c) 2013-2014, Andrometa LLC
All rights reserved.

neu@andrometa.net
http://neu.andrometa.net

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
#include <neu/NMetaGenerator.h>

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
        visitor_(visitor){
        
    }
      
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
    : visitor_(visitor){
        
    }
      
    ASTConsumer* CreateASTConsumer(CompilerInstance& compilerInstance,
                                   StringRef file){
        
      return new Consumer(&compilerInstance, visitor_);
    }
      
  private:
    MetaGenerator* visitor_;
  };
    
  class Database : public CompilationDatabase{
  public:
    Database(const nvec& includes)
    : includes_(includes){
        
    }
      
    CompileCommandVec getCompileCommands(StringRef path) const{
      nstr p = path.str();
        
      CompileCommandVec cv;
        
      CompileCommand c;
      c.Directory = ".";
        
      c.CommandLine = {"clang-tool", "-std=c++11"};

      c.CommandLine.push_back("-DMETA_GUARD");
      c.CommandLine.push_back("-Wno-undefined-internal");
      
#ifdef __APPLE__
      c.CommandLine.push_back("-stdlib=libc++");
        
      c.CommandLine.push_back("-resource-dir");
      c.CommandLine.push_back("/Users/nickm/llvm-3.4/build-release/bin/../"
                              "lib/clang/3.4");
        
      c.CommandLine.push_back("-I/Applications/Xcode.app/Contents/Developer/"
                              "Platforms/MacOSX.platform/Developer/SDKs/"
                              "MacOSX10.9.sdk/usr/include");
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
  };
    
  class Factory : public FrontendActionFactory{
  public:
    Factory(MetaGenerator* visitor)
    : visitor_(visitor){
        
    }
      
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
    
    nstr h;
    if(!NSys::getEnv("NEU_HOME", h)){
      NERROR("NEU_HOME environment variable is undefined");
    }
      
    includes_.push_back(h + "/include");

    nTypeMap_["void"] = true;
    nTypeMap_["bool"] = true;
    nTypeMap_["const bool"] = true;
    nTypeMap_["const bool &"] = true;
    nTypeMap_["_Bool"] = true;
    nTypeMap_["const _Bool"] = true;
    nTypeMap_["const _Bool &"] = true;
    nTypeMap_["unsigned char"] = true;
    nTypeMap_["const unsigned char"] = true;
    nTypeMap_["const unsigned char &"] = true;
    nTypeMap_["signed char"] = true;
    nTypeMap_["const signed char"] = true;
    nTypeMap_["const signed char &"] = true;
    nTypeMap_["char"] = true;
    nTypeMap_["const char"] = true;
    nTypeMap_["const char &"] = true;
    nTypeMap_["short"] = true;
    nTypeMap_["const short"] = true;
    nTypeMap_["const short &"] = true;
    nTypeMap_["unsigned short"] = true;
    nTypeMap_["const unsigned short"] = true;
    nTypeMap_["const unsigned short &"] = true;
    nTypeMap_["int"] = true;
    nTypeMap_["const int"] = true;
    nTypeMap_["const int &"] = true;
    nTypeMap_["unsigned int"] = true;
    nTypeMap_["const unsigned int"] = true;
    nTypeMap_["const unsigned int &"] = true;
    nTypeMap_["long"] = true;
    nTypeMap_["const long"] = true;
    nTypeMap_["const long &"] = true;
    nTypeMap_["long long"] = true;
    nTypeMap_["const long long"] = true;
    nTypeMap_["const long long &"] = true;
    nTypeMap_["unsigned long"] = true;
    nTypeMap_["const unsigned long"] = true;
    nTypeMap_["const unsigned long &"] = true;
    nTypeMap_["unsigned long long"] = true;
    nTypeMap_["const unsigned long long"] = true;
    nTypeMap_["const unsigned long long &"] = true;
    nTypeMap_["float"] = true;
    nTypeMap_["const float"] = true;
    nTypeMap_["const float &"] = true;
    nTypeMap_["double"] = true;
    nTypeMap_["const double"] = true;
    nTypeMap_["const double &"] = true;
    nTypeMap_["class neu::nvar"] = true;
    nTypeMap_["class neu::nvar *"] = true;
    nTypeMap_["const class neu::nvar"] = true;
    nTypeMap_["const class neu::nvar &"] = true;
    nTypeMap_["class neu::nvar &"] = true;
    nTypeMap_["class neu::nstr"] = true;
    nTypeMap_["const class neu::nstr"] = true;
    nTypeMap_["const class neu::nstr &"] = true;
    nTypeMap_["class neu::nstr &"] = true;
    nTypeMap_["class std::basic_string<char>"] = true;
    nTypeMap_["const class std::basic_string<char>"] = true;
    nTypeMap_["const class std::basic_string<char> &"] = true;
      
    nTypeMap_["class neu::NVector<class neu::nvar, "
              "class std::allocator<class neu::nvar> >"] = true;
    nTypeMap_["class neu::NVector<class neu::nvar, "
              "class std::__1::allocator<class neu::nvar> >"] = true;
      
    nTypeMap_["const class neu::NVector<class neu::nvar, "
              "class std::allocator<class neu::nvar> >"] = true;
    nTypeMap_["const class neu::NVector<class neu::nvar, "
              "class std::__1::allocator<class neu::nvar> >"] = true;
      
    nTypeMap_["const class neu::NVector<class neu::nvar, "
              "class std::allocator<class neu::nvar> > &"] = true;
    nTypeMap_["const class neu::NVector<class neu::nvar, "
              "class std::__1::allocator<class neu::nvar> > &"] = true;
      
    nTypeMap_["class neu::NList<class neu::nvar, "
              "class std::allocator<class neu::nvar> >"] = true;
    nTypeMap_["class neu::NList<class neu::nvar, "
              "class std::__1::allocator<class neu::nvar> >"] = true;
      
      
    nTypeMap_["const class neu::NList<class neu::nvar, "
              "class std::allocator<class neu::nvar> >"] = true;
    nTypeMap_["const class neu::NList<class neu::nvar, "
              "class std::__1::allocator<class neu::nvar> >"] = true;
      
    nTypeMap_["const class neu::NList<class neu::nvar, "
              "class std::allocator<class neu::nvar> > &"] = true;
    nTypeMap_["const class neu::NList<class neu::nvar, "
              "class std::__1::allocator<class neu::nvar> > &"] = true;
      
    nTypeMap_["class neu::nrat"] = true;
    nTypeMap_["const class neu::nrat"] = true;
    nTypeMap_["const class neu::nrat &"] = true;
    nTypeMap_["class neu::nreal"] = true;
    nTypeMap_["const class neu::nreal"] = true;
    nTypeMap_["const class neu::nreal &"] = true;
    nTypeMap_["class neu::nstr *"] = true;
  }
    
  ~MetaGenerator(){
      
  }

  bool isNType(const nstr& t){
    return nTypeMap_.hasKey(t);
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
    
  bool generate(ostream& ostr, const nstr& filePath, const nstr& className){
    className_ = className;
    fullClassName_ = className_.find("::") != nstr::npos;

    ostr << "#include <neu/nvar.h>" << endl;
      
    if(enableHandle_){
      ostr << "#include <neu/NFuncMap.h>" << endl;
    }
      
    if(enableClass_){
      ostr << "#include <neu/NClass.h>" << endl;
    }

    ostr_ = &ostr;

    Database db(includes_);

    StringVec files = {filePath};
    ClangTool tool(db, files);
      
    Factory factory(this);

    int status = tool.run(&factory);

    return status == 0;
  }
    
  CXXRecordDecl* getSuperClass(CXXRecordDecl* rd, const string& className){
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
          CXXRecordDecl* s = getSuperClass(srd, className);
          if(s){
            return s;
          }
        }
      }
    }
      
    return 0;
  }
    
  CXXRecordDecl* getFirstSuperClass(CXXRecordDecl* rd,
                                    const string& className){
    if(!rd->hasDefinition()){
      return 0;
    }
      
    for(CXXRecordDecl::base_class_iterator bitr = rd->bases_begin(),
          bitrEnd = rd->bases_end(); bitr != bitrEnd; ++bitr){
        
      CXXBaseSpecifier b = *bitr;
      QualType qt = b.getType();
        
      QualType ct = sema_->Context.getCanonicalType(qt);
      const Type* t = ct.getTypePtr();
        
      if(const RecordType* rt = dyn_cast<RecordType>(t)){
        if(CXXRecordDecl* srd = dyn_cast<CXXRecordDecl>(rt->getDecl())){
          if(getSuperClass(srd, className)){
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
    
    CXXRecordDecl* s = getSuperClass(d, "neu::NObject");
    
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
    if(!isNCompatibleType(fd->getResultType())){
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
        
      bool isVoid = md->getResultType().getTypePtr()->isVoidType();
        
      for(size_t j = md->getMinRequiredArguments(); 
          j <= md->param_size(); ++j){
        ostr << "    add(\"" << md->getNameAsString() << "\", " << j <<
          ", " << endl;
        ostr << "      [](void* o, const neu::nvar& n) -> neu::nvar{" << endl;
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
              
            nstr c = cn.substr(0, 6);
              
            assert(c == "class ");
            nstr name = cn.substr(6);
              
            name.findReplace("<anonymous>::", "");
              
            ostr << "n[" << k << "].ptr<" << name << ">()";
          }
          else if(t->isReferenceType()){
            nstr qs = qt.getAsString();
            
            if(qs == "class neu::nvar &"){
              ostr << "*";
            }

            ostr << "n[" << k << "]";
          }
          else{
            ostr << "n[" << k << "]";
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
      ostr << "  return _" << className << "_FuncMap.map(n) ? : " << endl;
      ostr << getQualifiedName(getFirstSuperClass(rd, "neu::NObject"));
      ostr << "::handle(v, flags);";
    }
      
    ostr << "}" << endl;
    
    *ostr_ << ostr.str() << endl;
  }

  nstr getQualifiedName(NamedDecl* decl){
    PrintingPolicy p(sema_->LangOpts);
    nstr ret = decl->getQualifiedNameAsString(p);
    ret.findReplace("<anonymous namespace>::", "");
      
    return ret;
  }
  
  // ndm - is there a better way to do this?
  bool isInUnnamedNamespace(NamedDecl* decl){
    nstr ret = decl->getQualifiedNameAsString();
    return ret.find("<anonymous namespace>::") != nstr::npos;
  }
  
  void generateClass(CXXRecordDecl* rd){
    nstr name = rd->getName().str();
    nstr fullName = getQualifiedName(rd);
      
    stringstream ostr;
      
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
          QualType qrt = md->getResultType();
          QualType crt = context_->getCanonicalType(qrt);
        
          nstr rts = cleanType(crt.getAsString());
        
          ostr << rts << " " << fullName << "::" << methodName << "(";
        
          if(qrt.getAsString() == "ndist" &&
             crt.getAsString() == "class neu::nvar"){
          
            if(CXXRecordDecl* srd = getFirstSuperClass(rd, "neu::NObject")){
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
          ostr << "    return remoteProcess(nfunc(\"" << methodName <<
            "\")";
          for(FunctionDecl::param_iterator itr = md->param_begin(),
                itrEnd = md->param_end(); itr != itrEnd; ++itr){
            ParmVarDecl* p = *itr;
            QualType qt = p->getType();
            QualType ct = context_->getCanonicalType(qt);
            nstr ts = ct.getAsString();
          
            ostr << " << mnode(";
          
            if(ts == "const class neu::nvar &" ||
               ts == "class neu::nvar &" ||
               ts == "class neu::nstr &"){
              ostr << "&";
            }
          
            ostr << p->getName().str() << ")";
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
          if(!md->getResultType().getTypePtr()->isVoidType()){
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
      
    CXXRecordDecl* srd = getFirstSuperClass(rd, "neu::NObject");
      
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
         !md->isOverloadedOperator()){
          
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
              
            if(!md->getResultType().getTypePtr()->isVoidType()){
              ostr << "      neu::nvar& ri = mi(\"return\");" << endl;
                
              nstr rawType = md->getResultType().getAsString();
                
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
  typedef NMap<nstr, bool> NTypeMap_;

  NTypeMap_ nTypeMap_;
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
};

void printUsage(){
  cout << NProgram::usage("neu-meta [OPTIONS] <source file>") << endl;
}

int main(int argc, char** argv){
  NProgram program(argc, argv);

  program.argDefault("class", "",
                     "Class name to generate metadata for. "
                     "Defaults to the name of the source file.");

  program.argDefault("handle", true,
                     "True to generate handler.");
  
  program.argDefault("create", true,
                     "True to generate class.");
  
  program.argDefault("metadata", true,
                     "True to generate class metadata.");
  
  program.argDefault("outer", true,
                     "True to generate outer.");
  
  const nvar& args = program.args();

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
  
  gen.enableHandle(args["handle"]);
  gen.enableClass(args["create"]);
  gen.enableMetadata(args["metadata"]);
  gen.enableOuter(args["outer"]);
  
  if(!gen.generate(ostr, filePath, className)){
    return 1;
  }

  ofstream out2(metaPath.c_str());
  if(out2.fail()){
    cerr << "failed to open output file: " << metaPath << endl;
    program.exit(1);
  }
  out2 << ostr.str();
  out2.close();
  
  return 0;
}

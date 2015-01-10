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

#include <neu/NJObject.h>

#include <cstring>
#include <jni.h>

#include <neu/NSys.h>
#include <neu/NBasicMutex.h>

using namespace std;
using namespace neu;

namespace{

  typedef typename NJMethodMap::Type Type;
  typedef typename NJMethodMap::TypeVec TypeVec;
  
  typedef NVector<jvalue> ValueVec;
  
  NBasicMutex _mutex;
  
  class Method{
  public:
    nstr name;
    Type returnType;
    TypeVec paramTypes;
    bool isStatic;
    jmethodID methodId;
  };
  
  typedef NHashMap<size_t, Method*> MethodMap_;
  
  class Class{
  public:
    
    Class(const nstr& className)
    : className_(className){}
    
    JNIEnv* env(){
      return env_;
    }
    
    nstr typeStr(Type t){
      switch(t){
        case Type::Void:
          return "V";
        case Type::Bool:
          return "Z";
        case Type::Char:
          return "B";
        case Type::Short:
          return "S";
        case Type::Int:
          return "I";
        case Type::Long:
          return "J";
        case Type::Float:
          return "F";
        case Type::Double:
          return "D";
        case Type::String:
          return "Ljava/lang/String;";
        case Type::Var:
        case Type::VarRef:
          return "Lnet/andrometa/neu/nvar;";
      }
    }
    
    void init(JNIEnv* env){
      env_ = env;
      
      nvarClass_ = env_->FindClass("net/andrometa/neu/nvar");
      assert(nvarClass_ != 0);
      
      nvarCtorMethodId_ =
      env_->GetMethodID(nvarClass_, "<init>", "([B)V");
      
      nvarPackMethodId_ =
      env_->GetMethodID(nvarClass_, "pack", "()[B");
      
      class_ = env->FindClass(className_.c_str());
      
      if(class_ == 0){
        NERROR("invalid class: " + className_);
      }
      
      for(auto& itr : methodMap_){
        Method* m = itr.second;
        
        nstr ts = "(";
        
        for(Type t : m->paramTypes){
          ts += typeStr(t);
        }
        
        ts += ")" + typeStr(m->returnType);
        
        if(m->isStatic){
          m->methodId =
          env->GetStaticMethodID(class_, m->name.c_str(), ts.c_str());
        }
        else{
          m->methodId =
          env->GetMethodID(class_, m->name.c_str(), ts.c_str());
        }
      }
    }
    
    void addMethod(size_t methodId,
                   const nstr& name,
                   Type returnType,
                   const TypeVec& paramTypes,
                   bool isStatic){
      
      if(methodMap_.has(methodId)){
        NERROR("duplicate methodId: " + nvar(methodId));
      }
      
      Method* m = new Method;
      m->name = name;
      m->returnType = returnType;
      m->paramTypes = paramTypes;
      m->isStatic = isStatic;
      
      methodMap_[methodId] = m;
    }
    
    void addCtor(size_t methodId, const TypeVec& paramTypes){
      
      if(methodMap_.has(methodId)){
        NERROR("duplicate methodId: " + nvar(methodId));
      }
      
      Method* m = new Method;
      m->name = "<init>";
      m->returnType = Type::Void;
      m->paramTypes = paramTypes;
      m->isStatic = false;
      
      methodMap_[methodId] = m;
    }
    
    void getArgs(Method* m, const nvec& args, ValueVec& vs){
      TypeVec& ts = m->paramTypes;
      
      size_t n = args.size();
      
      jvalue v;
      
      for(size_t i = 0; i < n; ++i){
        switch(ts[i]){
          case Type::Void:
            NERROR("invalid param type");
          case Type::Bool:
            v.z = args[i].toBool();
            break;
          case Type::Char:
            v.b = args[i].toLong();
            break;
          case Type::Short:
            v.s = args[i].toLong();
            break;
          case Type::Int:
            v.i = args[i].toLong();
            break;
          case Type::Long:
            v.j = args[i].toLong();
            break;
          case Type::Float:
            v.f = args[i].toDouble();
            break;
          case Type::Double:
            v.d = args[i].toDouble();
            break;
          case Type::String:
            v.l = env_->NewStringUTF(args[i].c_str());
            break;
          case Type::Var:
          case Type::VarRef:{
            uint32_t size;
            char* buf = args[i].pack(size, nvar::NO_COMPRESS);

            jbyteArray ba = env_->NewByteArray(size);
            env_->SetByteArrayRegion(ba, 0, size, (jbyte*)buf);
            
            free(buf);

            v.l = env_->NewObject(nvarClass_, nvarCtorMethodId_, ba);
            
            env_->DeleteLocalRef(ba);
            break;
          }
        }
        
        vs.push_back(v);
      }
    }
    
    void freeArgs(Method* m, const nvec& args, ValueVec& vs){
      TypeVec& ts = m->paramTypes;
      
      size_t n = args.size();
      
      for(size_t i = 0; i < n; ++i){
        switch(ts[i]){
          case Type::Void:
          case Type::Bool:
          case Type::Char:
          case Type::Short:
          case Type::Int:
          case Type::Long:
          case Type::Float:
          case Type::Double:
            break;
          case Type::String:
          case Type::Var:
            env_->DeleteLocalRef(vs[i].l);
            break;
          case Type::VarRef:{
            nvar& v = *args[i];
            v = undef;
           
            jbyteArray ba =
            (jbyteArray)env_->CallObjectMethod(vs[i].l, nvarPackMethodId_);
            
            uint32_t size = env_->GetArrayLength(ba);
            
            jboolean isCopy;
            jbyte* elems = env_->GetByteArrayElements(ba, &isCopy);
            
            v.unpack((char*)elems, size);
            
            env_->ReleaseByteArrayElements(ba, elems, JNI_ABORT);
            env_->DeleteLocalRef(ba);
            env_->DeleteLocalRef(vs[i].l);
            break;
          }
        }
      }
    }
    
    jobject construct(size_t methodId, const nvec& args){
      auto itr = methodMap_.find(methodId);
      assert(itr != methodMap_.end());
      
      Method* m = itr->second;
      
      ValueVec vs;
      getArgs(m, args, vs);
      
      jobject obj = env_->NewObject(class_, m->methodId, vs.data());

      freeArgs(m, args, vs);

      return obj;
    }

    nvar staticCall(size_t methodId, const nvec& args){
      auto itr = methodMap_.find(methodId);
      assert(itr != methodMap_.end());
      
      Method* m = itr->second;
      
      ValueVec vs;
      getArgs(m, args, vs);
     
      nvar ret;
      
      switch(m->returnType){
        case Type::Void:
          env_->CallStaticVoidMethodA(class_, m->methodId, vs.data());
          ret = none;
          break;
        case Type::Bool:
          ret = env_->CallStaticBooleanMethodA(class_, m->methodId, vs.data());
          break;
        case Type::Char:
          ret = int64_t(env_->CallStaticByteMethodA(class_, m->methodId, vs.data()));
          break;
        case Type::Short:
          ret = int64_t(env_->CallStaticShortMethodA(class_, m->methodId, vs.data()));
          break;
        case Type::Int:
          ret = int64_t(env_->CallStaticIntMethodA(class_, m->methodId, vs.data()));
          break;
        case Type::Long:
          ret = int64_t(env_->CallStaticLongMethodA(class_, m->methodId, vs.data()));
          break;
        case Type::Float:
          ret = env_->CallStaticFloatMethodA(class_, m->methodId, vs.data());
          break;
        case Type::Double:
          ret = env_->CallStaticDoubleMethodA(class_, m->methodId, vs.data());
          break;
        case Type::String:{
          jstring str = 
            (jstring)env_->CallStaticObjectMethodA(class_, m->methodId, vs.data());
          
          jboolean isCopy;
          const char* buf = env_->GetStringUTFChars(str, &isCopy);
          ret = buf;
          
          env_->ReleaseStringUTFChars(str, buf);
          env_->DeleteLocalRef(str);
          
          break;
        }
        case Type::Var:{
          jobject v = env_->CallStaticObjectMethodA(class_, m->methodId, vs.data());
          
          jbyteArray ba =
          (jbyteArray)env_->CallObjectMethod(v, nvarPackMethodId_);
          
          uint32_t size = env_->GetArrayLength(ba);
          
          jboolean isCopy;
          jbyte* elems = env_->GetByteArrayElements(ba, &isCopy);
          
          ret.unpack((char*)elems, size);
          
          env_->ReleaseByteArrayElements(ba, elems, JNI_ABORT);
          env_->DeleteLocalRef(ba);
          env_->DeleteLocalRef(v);
          
          break;
        }
        case Type::VarRef:
          NERROR("invalid return type");
          break;
      }
      
      freeArgs(m, args, vs);
      
      return ret;
    }
    
    nvar call(jobject obj, size_t methodId, const nvec& args){
      auto itr = methodMap_.find(methodId);
      assert(itr != methodMap_.end());
      
      Method* m = itr->second;
      
      ValueVec vs;
      getArgs(m, args, vs);

      nvar ret;
      
      switch(m->returnType){
        case Type::Void:
          env_->CallVoidMethodA(obj, m->methodId, vs.data());
          ret = none;
          break;
        case Type::Bool:
          ret = env_->CallBooleanMethodA(obj, m->methodId, vs.data());
          break;
        case Type::Char:
          ret = int64_t(env_->CallByteMethodA(obj, m->methodId, vs.data()));
          break;
        case Type::Short:
          ret = int64_t(env_->CallShortMethodA(obj, m->methodId, vs.data()));
          break;
        case Type::Int:
          ret = int64_t(env_->CallIntMethodA(obj, m->methodId, vs.data()));
          break;
        case Type::Long:
          ret = int64_t(env_->CallLongMethodA(obj, m->methodId, vs.data()));
          break;
        case Type::Float:
          ret = env_->CallFloatMethodA(obj, m->methodId, vs.data());
          break;
        case Type::Double:
          ret = env_->CallDoubleMethodA(obj, m->methodId, vs.data());
          break;
        case Type::String:{
          jstring str = (jstring)env_->CallObjectMethodA(obj, m->methodId, vs.data());
          
          jboolean isCopy;
          const char* buf = env_->GetStringUTFChars(str, &isCopy);
          ret = buf;
          
          env_->ReleaseStringUTFChars(str, buf);
          env_->DeleteLocalRef(str);
          
          break;
        }
        case Type::Var:{
          jobject v = env_->CallObjectMethodA(obj, m->methodId, vs.data());
          
          jbyteArray ba =
          (jbyteArray)env_->CallObjectMethod(v, nvarPackMethodId_);
          
          uint32_t size = env_->GetArrayLength(ba);
          
          jboolean isCopy;
          jbyte* elems = env_->GetByteArrayElements(ba, &isCopy);
          
          ret.unpack((char*)elems, size);

          env_->ReleaseByteArrayElements(ba, elems, JNI_ABORT);
          env_->DeleteLocalRef(ba);
          env_->DeleteLocalRef(v);
          
          break;
        }
        case Type::VarRef:
          NERROR("invalid return type");
          break;
      }
      
      freeArgs(m, args, vs);
      
      return ret;
    }
    
  private:
    typedef NHashMap<size_t, Method*> MethodMap_;

    JNIEnv* env_;

    nstr className_;
    jclass class_;

    jclass nvarClass_;
    jmethodID nvarCtorMethodId_;
    jmethodID nvarPackMethodId_;

    MethodMap_ methodMap_;
  };
  
  class Global{
  public:
    Global()
    : env_(0),
    jvm_(0){}
    
    ~Global(){
      if(!jvm_){
        return;
      }
      
      jvm_->DestroyJavaVM();
    }
    
    void addMethod(const nstr& className,
                   size_t methodId,
                   const nstr& name,
                   Type returnType,
                   const TypeVec& paramTypes,
                   bool isStatic){
      auto itr = classMap_.find(className);
      assert(itr != classMap_.end());
      
      itr->second->addMethod(methodId, name, returnType, paramTypes, isStatic);
    }
    
    void addCtor(const nstr& className,
                 size_t methodId,
                 const TypeVec& paramTypes){
      auto itr = classMap_.find(className);
      assert(itr != classMap_.end());
      
      itr->second->addCtor(methodId, paramTypes);
    }
    
    void addClass(const nstr& className){
      if(classMap_.has(className)){
        NERROR("duplicate class: " + className);
      }
      
      classMap_[className] = new Class(className);
    }
    
    void initJVM(){
      if(jvm_){
        return;
      }
      
      nstr classPath;
      if(!NSys::getEnv("CLASSPATH", classPath)){
        classPath = ".";
      }
      
      options_ = "-Djava.class.path=" + classPath;
      
      JavaVMOption options[1];
      options[0].optionString = (char*)options_.c_str();
      
      JavaVMInitArgs args;
      memset(&args, 0, sizeof(args));
      args.version = JNI_VERSION_1_6;
      args.nOptions = 1;
      args.options = options;
      
      if(JNI_CreateJavaVM(&jvm_, (void**)&env_, &args) == JNI_ERR){
        NERROR("failed to initialize JVM");
      }
      
      for(auto& itr : classMap_){
        itr.second->init(env_);
      }
    }
    
     Class* getClass(const nstr& className){
      auto itr = classMap_.find(className);
      assert(itr != classMap_.end());
      return itr->second;
    }
    
    JNIEnv* env(){
      return env_;
    }
    
  private:
    typedef NMap<nstr, Class*> ClassMap_;
    
    JNIEnv* env_;
    JavaVM* jvm_;
    nstr options_;
    
    ClassMap_ classMap_;
  };
  
  Global* _global = 0;
  
} // end namespace

namespace neu{

  class NJObject_{
  public:
    NJObject_(NJObject* o, const nstr& className)
    : o_(o){
      assert(_global);
      _mutex.lock();
      _global->initJVM();
      class_ = _global->getClass(className);
      _mutex.unlock();
    }
    
    ~NJObject_(){
      class_->env()->DeleteLocalRef(obj_);
    }
    
    nvar call(size_t methodId, const nvec& args){
      return class_->call(obj_, methodId, args);
    }
    
    void construct(size_t methodId, const nvec& args){
      obj_ = class_->construct(methodId, args);
    }
    
  private:
    NJObject* o_;
    Class* class_;
    jobject obj_;
  };
  
} // end namespace neu

NJObject::NJObject(const nstr& className){
  x_ = new NJObject_(this, className);
}

NJObject::~NJObject(){
  delete x_;
}

nvar NJObject::call(size_t methodId, const nvec& args){
  return x_->call(methodId, args);
}

void NJObject::construct(size_t methodId, const nvec& args){
  x_->construct(methodId, args);
}

nvar NJObject::staticCall(const nstr& className,
                          size_t methodId,
                          const nvec& args){
  _mutex.lock();
  _global->initJVM();
  Class* c = _global->getClass(className);
  _mutex.unlock();
  return c->staticCall(methodId, args);
}

NJMethodMap::NJMethodMap(const nstr& className)
: className_(className){
  _mutex.lock();
  if(!_global){
    _global = new Global;
  }
  _global->addClass(className);
  _mutex.unlock();
}

void NJMethodMap::add(size_t methodId,
                      const nstr& name,
                      Type returnType,
                      const TypeVec& paramTypes,
                      bool isStatic){
  _mutex.lock();
  _global->addMethod(className_, methodId, name,
                     returnType, paramTypes, isStatic);
  _mutex.unlock();
}

void NJMethodMap::addCtor(size_t methodId, const TypeVec& paramTypes){
  _mutex.lock();
  _global->addCtor(className_, methodId, paramTypes);
  _mutex.unlock();
}

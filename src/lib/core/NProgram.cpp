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

#include <neu/NProgram.h>

#include <sstream>

#include <signal.h>

#include <neu/NRegex.h>
#include <neu/NReadGuard.h>
#include <neu/NWriteGuard.h>
#include <neu/NError.h>
#include <neu/NSys.h>
#include <neu/NThread.h>
#include <neu/NResourceManager.h>
#include <neu/global.h>
#include <neu/NMutex.h>
#include <neu/NMLParser.h>
#include <neu/NBasicMutex.h>

#include <sys/resource.h>
#include <signal.h>
#include <unistd.h>

#ifndef NEU_NO_PRECISE

#include <gmp.h>
#include <mpfr.h>

#endif

using namespace std;
using namespace neu;

namespace{
  
  NBasicMutex _exitMutex;
  NBasicMutex _handleMutex;
  
  static void _handleSignal(int s){
    signal(s, _handleSignal);
    
    switch(s){
      case SIGHUP:
      case SIGINT:
      case SIGQUIT:
      case SIGILL:
      case SIGTRAP:
      case SIGABRT:
      case SIGFPE:
      case SIGBUS:
      case SIGSEGV:
      case SIGSYS:
      case SIGPIPE:
      case SIGALRM:
      case SIGTERM:
      case SIGCONT:
      case SIGXCPU:
      case SIGXFSZ:
      case SIGVTALRM:
      case SIGWINCH:
      case SIGUSR1:
      case SIGUSR2:
        if(_handleMutex.tryLock()){
          _program->handleSignal(s, true);
          _handleMutex.unlock();
        }
        break;
      case SIGURG:
      case SIGCHLD:
      case SIGIO:
      case SIGPROF:
        if(_handleMutex.tryLock()){
          _program->handleSignal(s, false);
          _handleMutex.unlock();
        }
        break;
    }
  }
  
  void _resetSignalHandlers(){
    signal(SIGHUP, _handleSignal);
    signal(SIGINT, _handleSignal);
    signal(SIGQUIT, _handleSignal);
    signal(SIGILL, _handleSignal);
    signal(SIGTRAP, _handleSignal);
    signal(SIGABRT, _handleSignal);
    signal(SIGFPE, _handleSignal);
    signal(SIGBUS, _handleSignal);
    signal(SIGSEGV, _handleSignal);
    signal(SIGSYS, _handleSignal);
    signal(SIGPIPE, _handleSignal);
    signal(SIGALRM, _handleSignal);
    signal(SIGTERM, _handleSignal);
    signal(SIGURG, _handleSignal);
    signal(SIGCONT, _handleSignal);
    signal(SIGCHLD, _handleSignal);
    signal(SIGIO, _handleSignal);
    signal(SIGXCPU, _handleSignal);
    signal(SIGXFSZ, _handleSignal);
    signal(SIGVTALRM, _handleSignal);
    signal(SIGPROF, _handleSignal);
    signal(SIGWINCH, _handleSignal);
    signal(SIGUSR1, _handleSignal);
    signal(SIGUSR2, _handleSignal);
  }
  
  enum BuiltinKey{
    BKEY_improper,
    BKEY_packBlockSize,
    BKEY_tempPath,
    BKEY_timeout,
    BKEY_name,
    BKEY_home,
    BKEY_abort,
    BKEY_mathKernelPath
  };
  
  typedef NMap<nstr, BuiltinKey> BuiltinMap;
  
  BuiltinMap _builtinMap;
  
  class Opt{
  public:
    nstr key;
    nstr alias;
    nvar def;
    bool required;
    nstr description;
    bool multi;
  };
  
  typedef NMap<nstr, Opt*> OptMap;
  OptMap _optMap;
  OptMap _builtinOptMap;
  nvar _args;
  
} // end namespace

namespace neu{
  
  using namespace std;
  
  class NProgram_{
  public:
    NProgram_(NProgram* o, const nvar& args)
    : o_(o){
      init(args);
    }
    
    NProgram_(NProgram* o, int& argc, char** argv, const nvar& args)
    : o_(o){
      init(argc, argv, args);
    }
    
    NProgram_(NProgram* o)
    : o_(o){}
    
    ~NProgram_(){}
    
    void init(int& argc, char** argv, const nvar& args){
      NProgram::argc = argc;
      NProgram::argv = argv;
      
      init_();
      NProgram::parseArgs(argc, argv, _args);
      merge(_args, args);
      
      setBuiltins();
      
      nstr p = _name + "_conf.nml";
      if(NSys::exists(p)){
        nvar c;
        parseConfig(p, c);
        merge(_args, c);
        setBuiltins();
      }
      else{
        p = _home + "/conf/" + _name + "_conf.nml";
        if(NSys::exists(p)){
          nvar c;
          parseConfig(p, c);
          merge(_args, c);
          setBuiltins();
        }
      }
      
      setDefaults();
    }
    
    void init(const nvar& args){
      init_();
      _args = args;
      
      setBuiltins();
      setDefaults();
    }
    
    void merge(nvar& args, const nvar& readArgs){
      if(!readArgs.hasMap()){
        return;
      }
      
      const nmap& rm = readArgs;
      
      for(auto& itr : rm){
        const nvar& k = itr.first;
        
        if(!k.hasString()){
          continue;
        }
        
        if(args.has(k)){
          const nstr& ks = k;
          
          if(ks[0] == '_'){
            auto oitr = _builtinOptMap.find(ks);
            if(oitr == _builtinOptMap.end()){
              continue;
            }
            Opt* opt = oitr->second;
            if(opt->multi){
              args[k].append(itr.second);
            }
          }
          else{
            auto oitr = _optMap.find(ks);
            if(oitr == _optMap.end()){
              continue;
            }
            Opt* opt = oitr->second;
            if(opt->multi){
              args[k].append(itr.second);
            }
          }
        }
        else{
          args(k) = itr.second;
        }
      }
    }

    void parseConfig(const nstr& path, nvar& config){
      stringstream estr;
      NMLParser parser;
      parser.setErrorStream(estr);
      config = parser.parseFile(path);
      
      if(config == none){
        nstr err = estr.str();
        NERROR("error parsing config file: " + path + ": " + err);
      }
      
      if(!config.hasMap()){
        return;
      }
      
      nvec keys;
      config.keys(keys);
      for(const nvar& k : keys){
        if(!k.hasString()){
          continue;
        }
        
        const nstr& ks = k;
        
        nvar& v = config[k];
        
        if(v.isString()){
          NSys::replaceEnvs(v.str());
        }
        
        if(ks[0] == '_'){
          auto itr = _builtinOptMap.find(ks);
          if(itr != _builtinOptMap.end()){
            Opt* opt = itr->second;
            
            if(opt->multi && !v.hasSequence()){
              v = nvec() << move(v);
            }
            
            if(opt->key != ks){
              config.erase(k);
              config(opt->key) = move(v);
            }
          }
        }
        else{
          auto itr = _optMap.find(ks);
          if(itr != _optMap.end()){
            Opt* opt = itr->second;
            
            if(opt->multi && !v.hasSequence()){
              v = nvec() << move(v);
            }
            
            if(opt->key != ks){
              config.erase(k);
              config(opt->key) = move(v);
            }
          }
        }
      }
    }
    
    void builtinOpt(BuiltinKey key,
                    const nstr& name,
                    const nstr& alias,
                    const nvar& def,
                    const nstr& description="",
                    bool multi=false){
      
      nstr k = "_" + name;
      
      Opt* opt = new Opt;
      opt->key = k;
      opt->alias = alias;
      opt->def = def;
      opt->multi = multi;
      opt->required = false;
      opt->description = description;
      
      _builtinOptMap[name] = opt;
      
      if(!alias.empty()){
        _builtinOptMap[alias] = opt;
      }
      
      _builtinMap[k] = key;
    }

    void setBuiltins(){
      if(!_args.hasMap()){
        return;
      }
      
      nmap& m = _args;
      
      for(auto& itr : m){
        const nvar& k = itr.first;
        const nvar& v = itr.second;
        
        if(!k.hasString()){
          continue;
        }
        
        const nstr& ks = k.str();
        
        if(ks[0] != '_'){
          continue;
        }
        
        auto bitr = _builtinMap.find(ks);
        if(bitr == _builtinMap.end()){
          continue;
        }
        
        BuiltinKey key = bitr->second;
        switch(key){
          case BKEY_improper:
            _improper = v;
            break;
          case BKEY_packBlockSize:
            _packBlockSize = v;
            break;
          case BKEY_tempPath:
            _tempPath = v;
            break;
          case BKEY_timeout:
            _timeout = v;
            break;
          case BKEY_name:
            _name = v;
            break;
          case BKEY_home:
            _home = v;
            break;
          case BKEY_abort:
            _abort = v;
            break;
          case BKEY_mathKernelPath:
            _mathKernelPath = v;
            break;
        }
      }
    }
    
    void setDefaults(){
      if(_tempPath.empty()){
        _tempPath = _home + "/scratch";
      }
      
      for(auto& itr : _optMap){
        Opt* o = itr.second;
        
        if(!_args.has(o->key)){
          if(o->required){
            NERROR("missing option: " + o->key);
          }
          else if(o->multi){
            _args(o->key).intoVector();
          }
          else{
            _args(o->key) = o->def;
          }
        }
      }
    }
    
    void init_(){
      if(_program){
        NERROR("NProgram exists");
      }
      
      _program = o_;
      
      _resetSignalHandlers();
      
#ifndef NEU_NO_PRECISE
      size_t precision = 256;
      
      if(precision < MPFR_PREC_MIN || precision > MPFR_PREC_MAX){
        NERROR("invalid mPrecision value");
      }
      
      mpfr_set_default_prec(precision);
#endif
      
      NSys::getEnv("NEU_HOME", _home);

      builtinOpt(BKEY_home, "home", "", _home);
      builtinOpt(BKEY_name, "name", "", _name);
      builtinOpt(BKEY_abort, "abort", "", _abort);
      builtinOpt(BKEY_improper, "improper", "", _improper);
      builtinOpt(BKEY_packBlockSize, "packBlockSize", "", _packBlockSize);
      builtinOpt(BKEY_tempPath, "tempPath", "", _tempPath);
      builtinOpt(BKEY_timeout, "timeout", "", _timeout);
      builtinOpt(BKEY_mathKernelPath, "mathKernelPath", "", _mathKernelPath);
    }
    
  private:
    NProgram* o_;
  };
  
} // end namespace neu

NProgram::NProgram(int& argc, char** argv, const nvar& args){
  x_ = new NProgram_(this, argc, argv, args);
}

NProgram::NProgram(const nvar& args){
  x_ = new NProgram_(this, args);
}

NProgram::NProgram(){
  x_ = new NProgram_(this);
}

NProgram::~NProgram(){
  delete x_;
}

void NProgram::init(int& argc, char** argv, const nvar& args){
  x_->init(argc, argv, args);
}

void NProgram::init(const nvar& args){
  x_->init(args);
}

void NProgram::exit(int status){
  if(_exitMutex.tryLock()){
    if(_program){
      _program->onExit();
    }
    
    _resourceManager->release();
    std::exit(status);
  }
}

NProgram* NProgram::instance(){
  return _program;
}

void NProgram::opt(const nstr& name,
                   const nstr& alias,
                   const nvar& def,
                   const nstr& description,
                   bool required,
                   bool multi){
  if(name.empty()){
    NERROR("empty option name");
  }
  
  if(_optMap.has(name) || (!alias.empty() && _optMap.has(alias))){
    NERROR("duplicate option name: " + name);
  }

  Opt* opt = new Opt;
  opt->key = name;
  opt->alias = alias;
  opt->def = def;
  opt->multi = multi;
  opt->required = required;
  opt->description = description;
  
  _optMap[name] = opt;

  if(!alias.empty()){
    _optMap[alias] = opt;
  }
}

void NProgram::parseArgs(int argc, char** argv, nvar& args){
  _name = NSys::basename(argv[0]);
  
  if(_optMap.empty()){
    nstr lastKey;

    for(size_t i = 1; i < argc; ++i){
      char* arg = argv[i];
      
      if(arg[0] == '-'){
        if(!lastKey.empty()){
          NERROR("expected a value at arg: " + nvar(i));
        }
        
        lastKey = &arg[1];

        if(lastKey.empty()){
          nstr text;
        
          for(size_t j = i; j < args; ++j){
            if(j > 0){
              text += " ";
            }
            text += argv[j];
          }
          
          args("text") = move(text);
          break;
        }
        else if(lastKey[0] == '_'){
          lastKey[0] = '_';
        }
      }
      else if(lastKey.empty()){
        for(size_t j = i; j < argc; ++j){
          args.pushBack(argv[j]);
        }
        break;
      }
      else{
        args(lastKey) = nvar::fromStr(argv[i]);
        lastKey = "";
      }
    }
  }
  else{
    Opt* lastOpt = 0;
    
    for(size_t i = 1; i < argc; ++i){
      char* arg = argv[i];
      
      if(arg[0] == '-'){
        if(lastOpt){
          NERROR("expected a value at arg: " + nvar(i));
        }
        
        nstr key = &arg[1];
        bool negate = false;
        
        if(key.empty()){
          nstr text;
          
          for(size_t j = i; j < args; ++j){
            if(j > 0){
              text += " ";
            }
            text += argv[j];
          }
          
          args("text") = move(text);
          break;
        }
        else if(key[0] == '-'){
          if(key.beginsWith("-no-")){
            key.replace(0, 4, "");
            negate = true;
          }
          else{
            key.replace(0, 1, "");
          }
          
          auto itr = _builtinOptMap.find(key);
          if(itr == _builtinOptMap.end()){
            NERROR("unknown builtin option: " + key);
          }

          lastOpt = itr->second;
        }
        else{
          if(key.beginsWith("no-")){
            key.replace(0, 3, "");
            negate = true;
          }
          
          auto itr = _optMap.find(key);
          if(itr == _optMap.end()){
            NERROR("unknown option: " + key);
          }
          
          lastOpt = itr->second;
        }
        
        const nvar& def = lastOpt->def;
        
        if(def.isBool()){
          if(lastOpt->multi){
            args(lastOpt->key) << !negate;
          }
          else{
            args(lastOpt->key) = !negate;
          }
        
          lastOpt = 0;
        }
      }
      else if(lastOpt){
        const nvar& def = lastOpt->def;
        nvar v;
        
        switch(def.type()){
          case nvar::String:
            v = argv[i];
            break;
          case nvar::False:
          case nvar::True:
            v = nvar::fromStr(argv[i]);
            v = v.toBool();
            break;
          case nvar::Rational:
            v = nvar::fromStr(argv[i]);
            v = v.toRat();
            break;
          case nvar::Integer:
            v = nvar::fromStr(argv[i]);
            v = v.toLong();
            break;
          case nvar::Float:
            v = nvar::fromStr(argv[i]);
            v = v.toDouble();
            break;
          default:
            v = nvar::fromStr(argv[i]);
            break;
        }
        
        if(lastOpt->multi){
          args(lastOpt->key) << move(v);
        }
        else{
          args(lastOpt->key) = move(v);
        }
        
        lastOpt = 0;
      }
      else{
        for(size_t j = i; j < argc; ++j){
          args.pushBack(argv[j]);
        }
        
        break;
      }
    }
  }
}

const nvar& NProgram::args(){
  return _args;
}

nstr NProgram::usage(const nstr& msg){
  stringstream ostr;
  nstr idt = "       ";
  
  ostr << "SYNOPSIS" << endl;
  ostr << idt << msg << endl << endl;
  ostr << "DESCRIPTION" << endl;
  
  NMap<Opt*, bool> m;
  
  for(auto& itr : _optMap){
    Opt* opt = itr.second;

    if(m.has(opt)){
      continue;
    }
    
    m[opt] = true;
    
    ostr << idt << "-" << opt->key;
    
    const nstr& alias = opt->alias;
    
    if(!alias.empty()){
      ostr << ", -" << alias;
    }
    
    if(!opt->required){
      ostr << "=" << opt->def;
    }
    
    ostr << endl;
    
    ostr << idt << idt << opt->description;
    
    ostr << endl << endl;
  }
  
  return ostr.str();
}

nstr NProgram::signalName(int signal){
  switch(signal){
    case SIGHUP:
      return "SIGHUP";
    case SIGINT:
      return "SIGINT";
    case SIGQUIT:
      return "SIGQUIT";
    case SIGILL:
      return "SIGILL";
    case SIGTRAP:
      return "SIGTRAP";
    case SIGABRT:
      return "SIGABRT";
    case SIGFPE:
      return "SIGFPE";
    case SIGBUS:
      return "SIGBUS";
    case SIGSEGV:
      return "SIGSEGV";
    case SIGSYS:
      return "SIGSYS";
    case SIGPIPE:
      return "SIGPIPE";
    case SIGALRM:
      return "SIGALRM";
    case SIGTERM:
      return "SIGTERM";
    case SIGCONT:
      return "SIGCONT";
    case SIGXCPU:
      return "SIGXCPU";
    case SIGXFSZ:
      return "SIGXFSZ";
    case SIGVTALRM:
      return "SIGVTALRM";
    case SIGWINCH:
      return "SIGWINCH";
    case SIGUSR1:
      return "SIGUSR1";
    case SIGUSR2:
      return "SIGUSR2";
    case SIGURG:
      return "SIGURG";
    case SIGCHLD:
      return "SIGCHLD";
    case SIGIO:
      return "SIGIO";
    case SIGPROF:
      return "SIGPROF";
    default:
      NERROR("invalid signal: " + nvar(signal));
  }
}

void NProgram::require(){
  if(!_program){
    NERROR("NProgram has not been initialized");
  }
}

int NProgram::argc = 0;
char** NProgram::argv = 0;

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

#include <neu/NCommand.h>

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <cmath>

#include <neu/NError.h>
#include <neu/NRegex.h>
#include <neu/NResourceManager.h>
#include <neu/global.h>

using namespace std;
using namespace neu;

namespace neu{
  
  class NCommand_{
  public:
    NCommand_(NCommand* o, const nstr& command, int mode)
    : o_(o),
    command_(command),
    closeSignal_(15),
    ifd_(-1),
    ofd_(-1),
    efd_(-1),
    mode_(mode){
      
      int ip[2];
      int op[2];
      int ep[2];
      
      if(mode_ & NCommand::OutputWithError &&
         (mode_ & NCommand::Output || mode_ & NCommand::Error)){
        NERROR("OutputWithError mode cannot be combined with Output or "
               "Error modes");
      }
      
      if(mode_ & NCommand::Input){
        pipe(ip);
      }
      
      if(mode_ & NCommand::Output){
        pipe(op);
      }
      
      if(mode_ & NCommand::Error){
        pipe(ep);
      }
      
      if(mode_ & NCommand::OutputWithError){
        pipe(op);
      }
      
      if(!isPersistent()){
        _resourceManager->add(o_);
      }
      
      pid_ = fork();
      
      if(pid_ < 0){
        NERROR("failed to execute command: " + command);
      }
      
      if(pid_ == 0){
        if(mode_ & NCommand::Input){
          ::close(ip[1]);
          dup2(ip[0], 0);
          ::close(ip[0]);
        }
        
        if(mode_ & NCommand::Output){
          ::close(op[0]);
          dup2(op[1], 1);
          ::close(op[1]);
        }
        
        if(mode_ & NCommand::Error){
          ::close(ep[0]);
          dup2(ep[1], 2);
          ::close(ep[1]);
        }
        
        if(mode_ & NCommand::OutputWithError){
          ::close(op[0]);
          dup2(op[1], 1);
          dup2(op[1], 2);
          ::close(op[1]);
        }
        
        if(execl("/bin/bash", "bash", "-c", command.c_str(), NULL) < 0){
          NERROR("failed to execute command: " + command);
        }
      }
      else{
        if(mode_ & NCommand::Input){
          ::close(ip[0]);
          ifd_ = ip[1];
        }
        
        if(mode_ & NCommand::Output){
          ::close(op[1]);
          ofd_ = op[0];
          fcntl(ofd_, F_SETFL, O_NONBLOCK);
        }
        
        if(mode_ & NCommand::Error){
          ::close(ep[1]);
          efd_ = ep[0];
          fcntl(efd_, F_SETFL, O_NONBLOCK);
        }
        
        if(mode_ & NCommand::OutputWithError){
          ::close(op[1]);
          ofd_ = op[0];
          efd_ = op[0];
          fcntl(ofd_, F_SETFL, O_NONBLOCK);
        }
      }
    }
    
    ~NCommand_(){
      if(!isPersistent()){
        _resourceManager->remove(o_);
        close(true);
      }
    }
    
    bool readOutput(nstr& out, double timeout){
      if(ofd_ < 0){
        NERROR("command was not started with output mode");
      }
      
      char buf[2049];
      
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(ofd_, &fds);
      
      double sec = floor(timeout);
      double fsec = timeout - sec;
      
      timeval tv;
      tv.tv_sec = sec;
      tv.tv_usec = fsec*1e6;
      
      if(select(ofd_ + 1, &fds, NULL, NULL, &tv)){
        for(;;){
          int n = ::read(ofd_, buf, 2048);
          if(n == -1){
            timeval tv2;
            tv2.tv_sec = 0;
            tv2.tv_usec = 50000;
            
            if(!select(ofd_ + 1, &fds, NULL, NULL, &tv2)){
              break;
            }
          }
          else if(n == 0){
            return true;
          }
          
          buf[n] = '\0';
          out += buf;
        }
        return true;
      }
      else{
        return false;
      }
    }
    
    bool matchOutput(const NRegex& regex, nvec& m, double timeout){
      if(ofd_ < 0){
        NERROR("command was not started with output mode");
      }
      
      char buf[2049];
      
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(ofd_, &fds);
      
      nstr out;
      
      for(;;){
        double sec = floor(timeout);
        double fsec = timeout - sec;
        
        timeval tv;
        tv.tv_sec = sec;
        tv.tv_usec = fsec*1e6;
        
        if(select(ofd_ + 1, &fds, NULL, NULL, &tv)){
          int n = ::read(ofd_, buf, 2048);
          if(n > 0){
            buf[n] = '\0';
            out += buf;
            
            if(regex.match(out, m)){
              return true;
            }
          }
        }
        else{
          return false;
        }
      }
    }
    
    bool readError(nstr& err, double timeout){
      if(efd_ < 0){
        NERROR("command was not started with error mode");
      }
      
      char buf[2049];
      
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(efd_, &fds);
      
      double sec = floor(timeout);
      double fsec = timeout - sec;
      
      timeval tv;
      tv.tv_sec = sec;
      tv.tv_usec = fsec*1e6;
      
      if(select(efd_ + 1, &fds, NULL, NULL, &tv)){
        for(;;){
          int n = ::read(efd_, buf, 2048);
          if(n == -1){
            timeval tv2;
            tv2.tv_sec = 0;
            tv2.tv_usec = 50000;
            
            if(!select(ofd_ + 1, &fds, NULL, NULL, &tv2)){
              break;
            }
          }
          else if(n == 0){
            return true;
          }
          
          buf[n] = '\0';
          err += buf;
        }
        return true;
      }
      else{
        return false;
      }
    }
    
    bool matchError(const NRegex& regex, nvec& m, double timeout){
      if(efd_ < 0){
        NERROR("command was not started with output mode");
      }
      
      char buf[2049];
      
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(ofd_, &fds);
      
      nstr err;
      
      for(;;){
        double sec = floor(timeout);
        double fsec = timeout - sec;
        
        timeval tv;
        tv.tv_sec = sec;
        tv.tv_usec = fsec*1e6;
        
        if(select(efd_ + 1, &fds, NULL, NULL, &tv)){
          int n = ::read(efd_, buf, 2048);
          if(n > 0){
            buf[n] = '\0';
            err += buf;
            
            if(regex.match(err, m)){
              return true;
            }
          }
        }
        else{
          return false;
        }
      }
    }
    
    void write(const nstr& in){
      if(ifd_ < 0){
        NERROR("command was not started with input mode");
      }
      
      int n = ::write(ifd_, in.c_str(), in.length());
      if(n < 0){
        NERROR("error while writing");
      }
    }
    
    int await(){
      int status;
      waitpid(pid_, &status, 0);
      return status;
    }
    
    int processId(){
      return pid_;
    }
    
    int status(){
      int status;
      if(waitpid(pid_, &status, WNOHANG) == 0){
        return -1;
      }
      return status;
    }
    
    void close(bool await){
      if(pid_ < 0){
        return;
      }
      
      kill(pid_, closeSignal_);
      
      if(await){
        int status;
        waitpid(pid_, &status, 0);
      }
      
      if(ifd_ >= 0){
        ::close(ifd_);
      }
      
      if(ofd_ >= 0){
        ::close(ofd_);
      }
      
      if(efd_ >= 0){
        ::close(efd_);
      }
      
      pid_ = -1;
    }
    
    void setCloseSignal(size_t signalNum){
      closeSignal_ = signalNum;
    }
    
    void signal(size_t signalNum){
      kill(pid_, signalNum);
    }
    
    bool isPersistent() const{
      return mode_ & NCommand::Persistent;
    }
    
    const nstr& command() const{
      return command_;
    }
    
  private:
    NCommand* o_;
    nstr command_;
    size_t closeSignal_;
    int pid_;
    int ifd_;
    int ofd_;
    int efd_;
    int mode_;
  };
  
} // end namespace neu

NCommand::NCommand(const nstr& command, int mode){
  x_ = new NCommand_(this, command, mode);
}

NCommand::~NCommand(){
  delete x_;
}

bool NCommand::readOutput(nstr& out, double timeout){
  return x_->readOutput(out, timeout);
}

bool NCommand::readError(nstr& err, double timeout){
  return x_->readError(err, timeout);
}

void NCommand::write(const nstr& in){
  x_->write(in);
}

int NCommand::await(){
  return x_->await();
}

int NCommand::processId(){
  return x_->processId();
}

int NCommand::status(){
  return x_->status();
}

void NCommand::signal(size_t signalNum){
  return x_->signal(signalNum);
}

void NCommand::close(bool await){
  x_->close(await);
}

void NCommand::setCloseSignal(size_t signalNum){
  x_->setCloseSignal(signalNum);
}

void NCommand::matchOutput(const NRegex& regex, nvec& m){
  x_->matchOutput(regex, m, 315360000.0);
}

bool NCommand::matchOutput(const NRegex& regex, nvec& m, double timeout){
  return x_->matchOutput(regex, m, timeout);
}

void NCommand::matchError(const NRegex& regex, nvec& m){
  x_->matchError(regex, m, 315360000.0);
}

bool NCommand::matchError(const NRegex& regex, nvec& m, double timeout){
  return x_->matchError(regex, m, timeout);
}

bool NCommand::isPersistent() const{
  return x_->isPersistent();
}

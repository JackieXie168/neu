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

#ifndef NEU_N_PROGRAM_H
#define NEU_N_PROGRAM_H

#include <neu/nvar.h>

#include <ostream>
#include <signal.h>

namespace neu{
  
  class NProgram{
  public:
    NProgram(int& argc, char** argv, const nvar& args=undef);
    
    NProgram(const nvar& args);
    
    NProgram();
    
    virtual ~NProgram();

    static void use();
    
    void init(int& argc, char** argv, const nvar& args=undef);
    
    void init(const nvar& args);
    
    static void opt(const nstr& name,
                    const nstr& alias="",
                    const nvar& def=none,
                    const nstr& description="",
                    bool required=false,
                    bool multi=false);
    
    static void parseArgs(int argc, char** argv, nvar& args);
    
    static char** toArgv(const nvec& argVec);
    
    static const nvar& args();
    
    static nstr usage(const nstr& msg);
    
    virtual void onExit(){}
    
    static void exit(int status);
    
    static NProgram* instance();
    
    static const int SIG_HUP = SIGHUP;
    static const int SIG_INT = SIGINT;
    static const int SIG_QUIT = SIGQUIT;
    static const int SIG_ILL = SIGILL;
    static const int SIG_TRAP = SIGTRAP;
    static const int SIG_ABRT = SIGABRT;
    static const int SIG_FPE = SIGFPE;
    static const int SIG_BUS = SIGBUS;
    static const int SIG_SEGV = SIGSEGV;
    static const int SIG_SYS = SIGSYS;
    static const int SIG_PIPE = SIGPIPE;
    static const int SIG_ALRM = SIGALRM;
    static const int SIG_TERM = SIGTERM;
    static const int SIG_URG = SIGURG;
    static const int SIG_CONT = SIGCONT;
    static const int SIG_CHLD = SIGCHLD;
    static const int SIG_IO = SIGIO;
    static const int SIG_XCPU = SIGXCPU;
    static const int SIG_XFSZ = SIGXFSZ;
    static const int SIG_VTALRM = SIGVTALRM;
    static const int SIG_PROF = SIGPROF;
    static const int SIG_WINCH = SIGWINCH;
    static const int SIG_USR1 = SIGUSR1;
    static const int SIG_USR2 = SIGUSR2;
    
    virtual void handleSignal(int signal, bool fatal){
      if(fatal){
        std::cerr << "NProgram received: " << signalName(signal) << std::endl;
        NProgram::exit(1);
      }
    }
    
    static nstr signalName(int signal);
    
    static int argc;
    static char** argv;

    NProgram& operator=(const NProgram&) = delete;
    
    NProgram(const NProgram&) = delete;

  private:
    class NProgram_* x_;
  };
  
} // end namespace neu

#endif // NEU_N_PROGRAM_H

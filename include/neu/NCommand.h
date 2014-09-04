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

#ifndef NEU_N_COMMAND_H
#define NEU_N_COMMAND_H

#include <neu/nvar.h>

namespace neu{
  
  class NRegex;
  
  class NCommand{
  public:
    
    static const int Input = 0x01;
    static const int Output = 0x02;
    static const int Error = 0x04;
    static const int OutputWithError = 0x08;
    static const int Persistent = 0x10;
    
    NCommand(const nstr& command, int mode=0);
    
    ~NCommand();
    
    bool readOutput(nstr& out, double timeout);
    
    void matchOutput(const NRegex& regex, nvec& m);
    
    bool matchOutput(const NRegex& regex, nvec& m, double timeout);
    
    bool readError(nstr& err, double timeout);
    
    void matchError(const NRegex& regex, nvec& m);
    
    bool matchError(const NRegex& regex, nvec& m, double timeout);
    
    void write(const nstr& in);
    
    int await();
    
    int processId();
    
    int status();
    
    void setCloseSignal(size_t signalNum);
    
    void close(bool await=true);
    
    void signal(size_t signalNum);
    
    bool isPersistent() const;
    
    const nstr& command() const;
    
  private:
    class NCommand_* x_;
  };
  
} // end namespace neu

#endif // NEU_N_COMMAND_H

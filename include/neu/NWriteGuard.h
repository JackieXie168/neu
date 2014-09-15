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

#ifndef NEU_N_WRITE_GUARD_H
#define NEU_N_WRITE_GUARD_H

#include <neu/NRWMutex.h>

namespace neu{
  
  class NWriteGuard{
  public:
    NWriteGuard(NRWMutex& t)
    : t_(t),
    locked_(true){
      t_.writeLock();
    }
    
    ~NWriteGuard(){
      if(locked_){
        t_.unlock();
      }
    }
    
    void release(){
      t_.unlock();
      locked_ = false;
    }
    
    void acquire(){
      t_.writeLock();
      locked_ = true;
    }
    
    bool tryAcquire(){
      locked_ = t_.tryWriteLock();
      return locked_;
    }
    
    NWriteGuard(const NWriteGuard&) = delete;
    
    NWriteGuard& operator=(const NWriteGuard&) = delete;
    
  private:
    NRWMutex& t_;
    bool locked_;
  };
  
} // end namespace neu

#endif // NEU_N_WRITE_GUARD_H

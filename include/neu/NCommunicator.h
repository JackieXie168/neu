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

#ifndef NEU_N_COMMUNICATOR_H
#define NEU_N_COMMUNICATOR_H

#include <neu/nvar.h>

namespace neu{
  
  class NProcTask;
  class NSocket;
  
  class NCommunicator{
  public:

    class Encoder{
    public:
      // if an encrypted buffer is provided, user is responsible for
      // freeing buf, first 4 bytes of output buf are reserved for size
      virtual char* encrypt(char* buf, uint32_t& size){
        return buf;
      }
      
      // if a decrypted buffer is provided, user is responsible for
      // freeing buf
      virtual char* decrypt(char* buf, uint32_t& size){
        return buf;
      }
    };
    
    NCommunicator(NProcTask* task);
    
    virtual ~NCommunicator();

    void setEncoder(Encoder* encoder);
    
    bool connect(const nstr& host, int port);
    
    void setSocket(NSocket* socket);
    
    NProcTask* task();

    void close();
    
    virtual void onClose(bool manual){}
    
    bool isConnected() const;
    
    void send(nvar& msg);
    
    bool receive(nvar& msg);
    
    bool receive(nvar& msg, double timeout);
    
    nvar& session();
    
    NCommunicator(const NCommunicator&) = delete;
    
    NCommunicator& operator=(const NCommunicator&) = delete;
    
  private:
    class NCommunicator_* x_;
  };
  
} // end namespace neu

#endif // NEU_N_COMMUNICATOR_H

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

#include <neu/NPQueue.h>

#include <neu/NVSemaphore.h>
#include <neu/NThread.h>
#include <neu/NBasicMutex.h>
#include <neu/NList.h>
#include <neu/NQueue.h>

using namespace std;
using namespace neu;

namespace{
  
  class Queue{
  public:
    
    void add(size_t index){
      queue_.push_back(index);
    }
    
    bool get(size_t& index){
      mutex_.lock();
      
      if(queue_.empty()){
        mutex_.unlock();
        return false;
      }
      
      index = queue_.front();
      queue_.pop_front();
      mutex_.unlock();
      
      return true;
    }
    
  private:
    typedef NQueue<size_t> Queue_;
    
    Queue_ queue_;
    NBasicMutex mutex_;
  };
  
} // end namespace

namespace neu{
  
class NPQueue_{
public:
  
  NPQueue_(NPQueue* o, size_t threads)
  : o_(o),
    threads_(threads),
    chunk_(-1){

    for(size_t i = 0; i < threads_; ++i){
      Thread_* thread = new Thread_(funcVec_, queue_, chunk_);
      threadVec_.push_back(thread);
      thread->start();
    }
  }
  
  ~NPQueue_(){
    for(size_t i = 0; i < threads_; ++i){
      Thread_* thread = threadVec_[i];
      thread->exit();
      thread->join();
      delete thread;
    }
  }
  
  void add(NPFunc* func){
    chunk_ = -1;
    funcVec_.push_back(func);
  }

  void clear(bool free){
    if(free){
      for(auto& n : funcVec_){
        delete n;
      }
    }
    
    funcVec_.clear();
  }
  
  void start(){
    size_t end = funcVec_.size();

    if(chunk_ < 0){
      resetChunk();
    }

    for(size_t i = 0; i < end; i += chunk_){
      queue_.add(i);
    }

    for(size_t i = 0; i < threads_; ++i){
      threadVec_[i]->begin();
    }
  }
  
  void await(){
    for(size_t i = 0; i < threads_; ++i){
      threadVec_[i]->finish();
    }
  }

  void resetChunk(){
    chunk_ = funcVec_.size() / (threads_ * 2) + 1;
    
    while(chunk_ % 32 != 0){
      ++chunk_;
    }
  }

  bool run(){
    if(funcVec_.empty()){
      return false;
    }

    size_t end = funcVec_.size();

    if(chunk_ < 0){
      resetChunk();
    }

    for(size_t i = 0; i < end; i += chunk_){
      queue_.add(i);
    }

    for(size_t i = 0; i < threads_; ++i){
      threadVec_[i]->begin();
    }

    // run

    for(size_t i = 0; i < threads_; ++i){
      threadVec_[i]->finish();
    }
    
    return true;
  }
  
private:
  typedef NVector<NPFunc*> FuncVec_;

  NPQueue* o_;
  size_t threads_;
  FuncVec_ funcVec_;
  Queue queue_;
  int chunk_;

  class Thread_ : public NThread{
  public:
    Thread_(FuncVec_& funcVec, Queue& queue, int& chunk)
      : funcVec_(funcVec),
        queue_(queue),
        exit_(false),
        chunk_(chunk){
      
    }
    
    void begin(){
      beginSem_.release();
    }

    void finish(){
      finishSem_.acquire();
    }

    void exit(){
      exit_ = true;
      beginSem_.release();
    }
    
    void run(){
      size_t i;
      NPFunc* n;

      for(;;){
        beginSem_.acquire();

        if(exit_){
          return;
        }

        for(;;){
          if(!queue_.get(i)){
            break;
          }

          size_t end = i + chunk_;
          size_t size = funcVec_.size();

          if(end > size){
            end = size;

            while(i < end){
              n = funcVec_[i++];
              n->fp(n);
            }
          }
          else{
            while(i < end){
              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);
              
              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);
              
              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);
              
              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);
              
              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);
              
              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);
              
              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);
              
              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);
              
              n = funcVec_[i++];
              n->fp(n);

              n = funcVec_[i++];
              n->fp(n);
            }
          }
        }
    
        finishSem_.release();
      }
    }
    
  private:
    FuncVec_& funcVec_;
    Queue& queue_;
    NVSemaphore beginSem_;
    NVSemaphore finishSem_;
    bool exit_;
    int& chunk_;
  };
  
  typedef NVector<Thread_*> ThreadVec_;
  
  ThreadVec_ threadVec_;
};

} // end namespace neu

NPQueue::NPQueue(size_t threads){
  x_ = new NPQueue_(this, threads);
}

NPQueue::~NPQueue(){
  delete x_; 
}

void NPQueue::add(NPFunc* func){
  x_->add(func);
}

void NPQueue::clear(bool free){
  x_->clear(free);
}

void NPQueue::start(){
  x_->start();
}

bool NPQueue::run(){
  return x_->run();
}

void NPQueue::await(){
  x_->await();
}

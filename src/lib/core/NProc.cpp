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

Neu can be used freely for commercial purposes. We hope you will find
Neu powerful, useful to make money or otherwise, and fun! If so,
please consider making a donation via PayPal to: neu@andrometa.net

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

#include <neu/NProc.h>

#include <iostream>
#include <queue>

#include <neu/NVSemaphore.h>
#include <neu/NThread.h>
#include <neu/NBasicMutex.h>

using namespace std;
using namespace neu;

namespace neu{
  
  class NProc_{
  public:
    NProc_(NProc* o, NProcTask* task)
    : o_(o),
    task_(task),
    queueCount_(0),
    terminated_(false){
      
    }
    
    NProc_(NProc* o)
    : o_(o),
    task_(0),
    queueCount_(0),
    terminated_(false){
      
    }
    
    ~NProc_(){
      
    }
    
    void signal(NProc_* proc, nvar& v, double priority){
      assert(proc->task_ && "NProc has no task");
      
      nvar r;
      if(proc->o_->handle(v, r)){
        proc->task_->queue(proc->o_, r, priority);
      }
    }
    
    void setTask(NProcTask* task){
      task_ = task;
    }
    
    NProcTask* task(){
      return task_;
    }
    
    void queued(){
      ++queueCount_;
    }
    
    bool dequeued(){
      return --queueCount_ == 0 && terminated_;
    }
    
    NProc* outer(){
      return o_;
    }
    
    bool terminate(){
      terminated_ = true;
      return queueCount_ == 0;
    }
    
    bool terminated(){
      return terminated_;
    }
    
  private:
    NProc* o_;
    NProcTask* task_;
    NVSemaphore finishSem_;
    atomic<uint32_t> queueCount_;
    atomic_bool terminated_;
  };
  
  class NProcTask_{
  public:
    
    class Item{
    public:
      Item(NProc_* np, nvar& r, double p)
      : np(np),
      r(move(r)),
      p(p){

      }
      
      NProc_* np;
      nvar r;
      double p;
    };
    
    class Queue{
    public:
      Queue()
      : active_(true){}
      
      ~Queue(){}
      
      void dealloc(NProcTask* task){
        while(!queue_.empty()){
          Item* item = queue_.top();
          queue_.pop();
          
          task->dealloc(item->r);
          
          if(item->np->dequeued()){
            delete item->np->outer();
          }
          
          delete item;
        }
      }
      
      void put(Item* item){
        mutex_.lock();
        queue_.push(item);
        mutex_.unlock();
        
        sem_.release();
      }
      
      Item* get(){
        sem_.acquire();
        if(active_){
          mutex_.lock();
          Item* item = queue_.top();
          queue_.pop();
          mutex_.unlock();
          return item;
        }
        
        return 0;
      }

      void setActive(bool flag){
        active_ = flag;
        sem_.release();
      }
      
    private:
      struct Compare_{
        bool operator()(const Item* i1, const Item* i2) const{
          return i1->p < i2->p;
        }
      };
      
      typedef priority_queue<Item*, vector<Item*>, Compare_> Queue_;
      
      Queue_ queue_;
      NVSemaphore sem_;
      NBasicMutex mutex_;
      atomic_bool active_;
    };
    
    class Thread : public NThread{
    public:
      Thread(Queue& queue)
      : queue_(queue),
      active_(true){
        
      }
      
      void run(){
        while(active_){
          Item* item = queue_.get();
          if(item){
            NProc_* np = item->np;
            np->outer()->run(item->r);
            
            if(np->dequeued()){
              delete np->outer();
            }
            delete item;
          }
        }
      }
      
      void setActive(bool flag){
        active_ = flag;
      }
      
    private:
      Queue& queue_;
      atomic_bool active_;
    };
    
    NProcTask_(NProcTask* o, size_t threads)
    : o_(o){
      
      for(size_t i = 0; i < threads; ++i){
        Thread* thread = new Thread(q_);
        threadVec_.push_back(thread);
        thread->start();
      }
    }
    
    ~NProcTask_(){
      q_.setActive(false);
      
      for(Thread* t : threadVec_){
        t->setActive(false);
      }
      
      for(Thread* t : threadVec_){
        t->join();
        delete t;
      }
      
      q_.dealloc(o_);
    }
    
    void queue(NProc_* proc, nvar& r, double priority){
      if(proc->terminated()){
        return;
      }
      
      proc->queued();
      Item* item = new Item(proc, r, priority);
      q_.put(item);
    }
    
  private:
    typedef NVector<Thread*> ThreadVec_;
    
    NProcTask* o_;
    Queue q_;
    ThreadVec_ threadVec_;
  };
  
} // end namespace neu

NProc::NProc(NProcTask* task){
  x_ = new NProc_(this, task);
}

NProc::NProc(){
  x_ = new NProc_(this);
}

NProc::~NProc(){
  delete x_;
}

void NProc::signal(NProc* proc, nvar& v, double priority){
  x_->signal(proc->x_, v, priority);
}

void NProc::setTask(NProcTask* task){
  x_->setTask(task);
}

bool NProc::terminate(){
  return x_->terminate();
}

NProcTask* NProc::task(){
  return x_->task();
}

void NProc::queue(nvar& r, double priority){
  x_->task()->queue(this, r, priority);
}

void NProc::queue(){
  x_->task()->queue(this);
}

NProcTask::NProcTask(size_t threads){
  x_ = new NProcTask_(this, threads);
}

NProcTask::~NProcTask(){
  delete x_;
}

void NProcTask::queue(NProc* proc, nvar& r, double priority){
  x_->queue(proc->x_, r, priority);
}

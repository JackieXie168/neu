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

#ifndef NEU_N_V_SEMAPHORE_H
#define NEU_N_V_SEMAPHORE_H

#include <pthread.h>
#include <sys/time.h>
#include <cmath>

namespace neu{
  
  class NVSemaphore{
  public:
    
    NVSemaphore(int count=0)
    : count_(count),
    maxCount_(0),
    disabled_(false){

      pthread_mutex_init(&mutex_, 0);
      pthread_cond_init(&condition_, 0);
    }
    
    NVSemaphore(int count, int maxCount)
    : count_(count),
    maxCount_(maxCount),
    disabled_(false){

      pthread_mutex_init(&mutex_, 0);
      pthread_cond_init(&condition_, 0);
    }
    
    ~NVSemaphore(){
      pthread_cond_destroy(&condition_);
      pthread_mutex_destroy(&mutex_);
    }

    bool acquire(double timeout){
      timeval tv;
      gettimeofday(&tv, 0);
      
      double t = tv.tv_sec + tv.tv_usec/1e6;
      t += timeout;
      
      pthread_mutex_lock(&mutex_);
      
      if(disabled_){
        pthread_mutex_unlock(&mutex_);
        return false;
      }
      
      double sec = std::floor(t);
      double fsec = t - sec;

      timespec ts;
      ts.tv_sec = sec;
      ts.tv_nsec = fsec*1e9;

      while(count_ <= 0){
        if(pthread_cond_timedwait(&condition_, 
                                  &mutex_,
                                  &ts) != 0){
          pthread_mutex_unlock(&mutex_);
          return false;
        }
      }

      if(disabled_){
        pthread_mutex_unlock(&mutex_);
        return false;
      }
      
      --count_;
      pthread_mutex_unlock(&mutex_);
      
      return true;
    }
    
    bool acquire(){
      pthread_mutex_lock(&mutex_);

      if(disabled_){
        pthread_mutex_unlock(&mutex_);
        return false;
      }
      
      while(count_ <= 0){
        pthread_cond_wait(&condition_, &mutex_);
      }

      if(disabled_){
        pthread_mutex_unlock(&mutex_);
        return false;
      }
      
      --count_;
      pthread_mutex_unlock(&mutex_);
      
      return true;
    }
    
    bool tryAcquire(){
      pthread_mutex_lock(&mutex_);

      if(disabled_){
        pthread_mutex_unlock(&mutex_);
        return false;
      }
      
      if(count_ > 0){
        --count_;
        pthread_mutex_unlock(&mutex_);
        return true;
      }

      pthread_mutex_unlock(&mutex_);
      return false;
    }
    
    void release(){
      pthread_mutex_lock(&mutex_);

      if(maxCount_ == 0 || count_ < maxCount_){
        ++count_;
      }

      pthread_cond_signal(&condition_);
      pthread_mutex_unlock(&mutex_);
    }

    void enable(){
      disabled_ = false;
    }
    
    void disable(){
      pthread_mutex_lock(&mutex_);

      disabled_ = true;
      
      pthread_cond_broadcast(&condition_);
      pthread_mutex_unlock(&mutex_);
    }
    
    NVSemaphore& operator=(const NVSemaphore&) = delete;
    
    NVSemaphore(const NVSemaphore&) = delete;

  private:
    pthread_mutex_t mutex_;
    pthread_cond_t condition_;

    int count_;
    int maxCount_;
    bool disabled_;
  };
  
} // end namespace neu

#endif // NEU_N_V_SEMAPHORE_H

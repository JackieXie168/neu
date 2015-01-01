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

#include <neu/NNModule.h>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include <neu/NThread.h>
#include <neu/NBasicMutex.h>
#include <neu/NVSemaphore.h>
#include <neu/NNet.h>
#include <neu/NQueue.h>

using namespace std;
using namespace llvm;
using namespace neu;

namespace{
  
  void allocVector(size_t size, double** start, double** aligned){
    char* s = (char*)malloc(sizeof(double)*size + 32);
    char* a = s;
    while((size_t)a % 32 != 0){
      ++a;
    }
    
    *start = (double*)s;
    *aligned = (double*)a;
  }
  
  bool _initialized = false;
  NBasicMutex _mutex;
  
  class Queue;
  
  class RunLayer{
  public:
    void* inputVecStart;
    void* inputVec;
    void* outputVecStart;
    void* outputVec;
    Function* f;
    void (*fp)(void*, void*, void*, int);
    Queue* queue;
  };
  
  class RunNeuron{
  public:
    ~RunNeuron(){
      free(weightVecStart);
    }
    
    RunLayer* layer;
    void* weightVecStart;
    void* weightVec;
    int outputIndex;
    
    void run(){
      layer->fp(layer->inputVec, weightVec, layer->outputVec, outputIndex);
    }
  };
  
  class Queue{
  public:
    Queue(size_t threads)
    : threads_(threads),
    chunk_(-1){
      
      for(size_t i = 0; i < threads_; ++i){
        Thread_* thread = new Thread_(neuronVec_, *this, chunk_);
        threadVec_.push_back(thread);
        thread->start();
      }
    }
    
    ~Queue(){
      for(size_t i = 0; i < threads_; ++i){
        Thread_* thread = threadVec_[i];
        thread->exit();
        thread->join();
        delete thread;
      }
    }
    
    void addIndex(size_t index){
      queue_.push_back(index);
    }
    
    bool getIndex(size_t& index){
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
    
    void add(RunNeuron* n){
      chunk_ = -1;
      neuronVec_.push_back(n);
    }
    
    void clear(bool free){
      if(free){
        for(auto& n : neuronVec_){
          delete n;
        }
        neuronVec_.clear();
      }
      else{
        neuronVec_.clear();
      }
    }
    
    void start(){
      size_t end = neuronVec_.size();
      
      if(chunk_ < 0){
        resetChunk();
      }
      
      for(size_t i = 0; i < end; i += chunk_){
        addIndex(i);
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
      chunk_ = neuronVec_.size() / (threads_ * 2) + 1;
      
      while(chunk_ % 32 != 0){
        ++chunk_;
      }
    }
    
    bool run(){
      if(neuronVec_.empty()){
        return false;
      }
      
      size_t end = neuronVec_.size();
      
      if(chunk_ < 0){
        resetChunk();
      }
      
      for(size_t i = 0; i < end; i += chunk_){
        addIndex(i);
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
    typedef NVector<RunNeuron*> NeuronVec_;
    typedef NQueue<size_t> Queue_;
    
    size_t threads_;
    NeuronVec_ neuronVec_;
    Queue_ queue_;
    NBasicMutex mutex_;
    int chunk_;
    
    class Thread_ : public NThread{
    public:
      Thread_(NeuronVec_& neuronVec, Queue& queue, int& chunk)
      : neuronVec_(neuronVec),
      queue_(queue),
      exit_(false),
      chunk_(chunk){}
      
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
        
        for(;;){
          beginSem_.acquire();
          
          if(exit_){
            return;
          }
          
          for(;;){
            if(!queue_.getIndex(i)){
              break;
            }
            
            size_t end = i + chunk_;
            size_t size = neuronVec_.size();
            
            if(end > size){
              end = size;
              
              while(i < end){
                neuronVec_[i++]->run();
              }
            }
            else{
              while(i < end){
                // loop unroll if chunk is large enough
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
                neuronVec_[i++]->run();
              }
            }
          }
          finishSem_.release();
        }
      }
      
    private:
      NeuronVec_& neuronVec_;
      Queue& queue_;
      NVSemaphore beginSem_;
      NVSemaphore finishSem_;
      bool exit_;
      int& chunk_;
    };
    
    typedef NVector<Thread_*> ThreadVec_;
    
    ThreadVec_ threadVec_;
  };
  
  class RunNetwork{
  public:
    typedef NVector<RunLayer*> LayerVec_;
    
    LayerVec_ layerVec;
  };
  
} // end namespace

namespace neu{
  
  class NNModule_{
  public:
    typedef vector<Type*> TypeVec;
    
    NNModule_(NNModule* o)
    : o_(o),
    context_(getGlobalContext()),
    module_("NNModule", context_),
    builder_(context_){
      
      _mutex.lock();
      if(!_initialized){
        InitializeNativeTarget();
        _initialized = true;
      }
      _mutex.unlock();
      
      engine_ = EngineBuilder(&module_).setUseMCJIT(true).create();
      
      TypeVec args;
      args.push_back(doubleType());
      createExtern("llvm.exp.f64", doubleType(), args);
    }
    
    ~NNModule_(){
      for(auto itr : networkMap_){
        remove_(itr.second);
      }
    }
    
    Function* createExtern(const nstr& name,
                           Type* rt,
                           const TypeVec& args){
      FunctionType* ft = FunctionType::get(rt, args, false);
      
      Function* f = Function::Create(ft,
                                     Function::ExternalLinkage,
                                     name.c_str(),
                                     &module_);
      
      externMap_[name] = f;
      
      return f;
    }
    
    Function* getExtern(const nstr& name){
      auto itr = externMap_.find(name);
      if(itr == externMap_.end()){
        return 0;
      }
      
      return itr->second;
    }
    
    Value* getActivationOutput(NNet::Neuron* n, Value* v){
      const NNet::Func* activationFunc = &n->activationFunc();
      
      if(const NNet::SigmoidFunc* f =
         dynamic_cast<const NNet::SigmoidFunc*>(activationFunc)){
        
        double a = f->a();
        Function* expFunc = getExtern("llvm.exp.f64");
        assert(expFunc && "failed to get exp function");
        
        Value* neg_av =
        builder_.CreateFMul(getDouble(-a), v, "neg_av");
        
        NVector<Value*> args;
        args.push_back(neg_av);
        
        Value* exp_neg_av =
        builder_.CreateCall(expFunc, args.vector(), "exp_neg_av");
        
        Value* exp_neg_av_1 =
        builder_.CreateFAdd(getDouble(1.0), exp_neg_av, "exp_neg_av_1");
        
        return builder_.CreateFDiv(getDouble(1.0), exp_neg_av_1, "out");
      }
      
      assert(false && "unrecognized activation function");
    }
    
    Type* voidType(){
      return Type::getVoidTy(context_);
    }
    
    Type* int32Type(){
      return Type::getInt32Ty(context_);
    }
    
    Type* doubleType(){
      return Type::getDoubleTy(context_);
    }
    
    VectorType* doubleVecType(size_t length){
      return VectorType::get(doubleType(), length);
    }
    
    size_t vectorLength(VectorType* vt){
      return vt->getNumElements();
    }
    
    size_t vectorLength(Type* t){
      VectorType* vt = dyn_cast<VectorType>(t);
      if(!vt){
        return 0;
      }
      
      return vectorLength(vt);
    }
    
    size_t vectorLength(Value* v){
      return vectorLength(v->getType());
    }
    
    Value* getInt32(int32_t v){
      return ConstantInt::get(context_, APInt(32, v, true));
    }
    
    Value* getDouble(double v){
      return ConstantFP::get(context_, APFloat(v));
    }
    
    PointerType* getPointer(Type* type){
      return PointerType::get(type, 0);
    }
    
    bool compile(const nstr& name,
                 NNet& network,
                 size_t threads){
      
      RunNetwork* runNetwork = new RunNetwork;
      
      NNet::Layer* inputLayer = network.layer(0);
      
      size_t numLayers = network.numLayers();
      
      RunLayer* lastRunLayer = 0;
      
      for(size_t l = 1; l < numLayers; ++l){
        RunLayer* runLayer = new RunLayer;
        runLayer->queue = new Queue(threads);
        
        size_t inputLayerSize = inputLayer->size();
        
        NNet::Layer* layer = network.layer(l);
        
        size_t layerSize  = layer->size();
        
        if(l > 1){
          runLayer->inputVecStart = lastRunLayer->outputVecStart;
          runLayer->inputVec = lastRunLayer->outputVec;
        }
        
        if(l < numLayers - 1){
          double* outputVecPtrStart;
          double* outputVecPtr;
          allocVector(layerSize, &outputVecPtrStart, &outputVecPtr);
          runLayer->outputVecStart = outputVecPtrStart;
          runLayer->outputVec = outputVecPtr;
        }
        
        TypeVec args;
        args.push_back(getPointer(doubleVecType(inputLayerSize)));
        args.push_back(getPointer(doubleVecType(inputLayerSize)));
        args.push_back(getPointer(doubleType()));
        args.push_back(int32Type());
        
        FunctionType* ft = FunctionType::get(voidType(), args, false);
        
        Function* f =
        Function::Create(ft, Function::ExternalLinkage,
                         name.c_str(), &module_);
        
        BasicBlock* entry = BasicBlock::Create(context_, "entry", f);
        
        builder_.SetInsertPoint(entry);
        
        auto aitr = f->arg_begin();
        
        Value* inputVecPtr = aitr;
        inputVecPtr->setName("input_vec_ptr");
        
        ++aitr;
        Value* weightVecPtr = aitr;
        weightVecPtr->setName("weight_vec_ptr");
        
        ++aitr;
        Value* outputVecPtr = aitr;
        outputVecPtr->setName("output_vec_ptr");
        
        ++aitr;
        Value* outputIndex = aitr;
        outputIndex->setName("output_index");
        
        Value* inputVec =
        builder_.CreateLoad(inputVecPtr, "input_vec");
        
        Value* weightVec =
        builder_.CreateLoad(weightVecPtr, "weight_vec");
        
        Value* mulVec =
        builder_.CreateFMul(inputVec, weightVec, "mul_vec");
        
        Value* sumActivation =
        builder_.CreateExtractElement(mulVec, getInt32(0), "sum_elem");
        
        for(size_t i = 1; i < inputLayerSize; ++i){
          Value* elem =
          builder_.CreateExtractElement(mulVec, getInt32(i), "sum_elem");
          
          sumActivation =
          builder_.CreateFAdd(sumActivation, elem, "sum_activation");
        }
        
        Value* output =
        getActivationOutput(layer->neuron(0), sumActivation);
        
        Value* outputElement =
        builder_.CreateGEP(outputVecPtr, outputIndex, "out_elem");
        
        builder_.CreateStore(output, outputElement);
        
        builder_.CreateRetVoid();
        
        runLayer->f = f;
        
        runLayer->fp = (void (*)(void*, void*, void*, int))
        engine_->getPointerToFunction(f);
        
        for(size_t j = 0; j < layerSize; ++j){
          NNet::Neuron* nj = layer->neuron(j);
          
          RunNeuron* runNeuron = new RunNeuron;
          runNeuron->layer = runLayer;
          runNeuron->outputIndex = j;
          
          double* weightVecPtrStart;
          double* weightVecPtr;
          allocVector(inputLayerSize, &weightVecPtrStart, &weightVecPtr);
          runNeuron->weightVecStart = weightVecPtrStart;
          runNeuron->weightVec = weightVecPtr;
          
          for(size_t i = 0; i < inputLayerSize; ++i){
            NNet::Neuron* ni = inputLayer->neuron(i);
            weightVecPtr[i] = nj->weight(ni);
          }
          
          runLayer->queue->add(runNeuron);
        }
        
        runNetwork->layerVec.push_back(runLayer);
        
        inputLayer = layer;
        lastRunLayer = runLayer;
      }
      
      networkMap_.insert(make_pair(name, runNetwork));
      
      return true;
    }
    
    void run(const nstr& name,
             void* inputVec,
             void* outputVec){
      auto itr = networkMap_.find(name);
      assert(itr != networkMap_.end());
      RunNetwork* network = itr->second;
      
      RunLayer* firstLayer = network->layerVec[0];
      firstLayer->inputVec = inputVec;
      
      RunLayer* lastLayer = network->layerVec.back();
      lastLayer->outputVec = outputVec;
      
      size_t numLayers = network->layerVec.size();
      for(size_t i = 0; i < numLayers; ++i){
        RunLayer* layer = network->layerVec[i];
        layer->queue->run();
      }
    }
    
    void remove_(RunNetwork* network){
      size_t numLayers = network->layerVec.size();
      for(size_t i = 0; i < numLayers; ++i){
        RunLayer* layer = network->layerVec[i];
        if(i < numLayers - 1){
          free(layer->outputVecStart);
        }
        
        engine_->freeMachineCodeForFunction(layer->f);
        layer->queue->clear(true);
        
        delete layer;
      }
    }
    
    void remove(const string& name){
      auto itr = networkMap_.find(name);
      if(itr == networkMap_.end()){
        return;
      }
      remove_(itr->second);
      networkMap_.erase(itr);
    }
    
  private:
    typedef NMap<nstr, Function*> FunctionMap_;
    typedef NMap<nstr, RunNetwork*> NetworkMap_;
    
    NNModule* o_;
    
    LLVMContext& context_;
    Module module_;
    IRBuilder<> builder_;
    ExecutionEngine* engine_;
    
    FunctionMap_ functionMap_;
    FunctionMap_ externMap_;
    NetworkMap_ networkMap_;
  };
  
} // end namespace neu

NNModule::NNModule(){
  x_ = new NNModule_(this);
}

NNModule::~NNModule(){
  delete x_;
}

bool NNModule::compile(const nstr& name,
                       NNet& network,
                       size_t threads){
  return x_->compile(name, network, threads);
}

void NNModule::run(const nstr& name,
                   void* inputVec,
                   void* outputVec){
  x_->run(name, inputVec, outputVec);
}

void NNModule::remove(const nstr& name){
  x_->remove(name);
}

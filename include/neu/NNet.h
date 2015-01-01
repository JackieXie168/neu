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

#ifndef NEU_N_NET_H
#define NEU_N_NET_H

#include <neu/NRandom.h>

namespace neu{
  
  class NNet{
  public:
    typedef NVector<double> ValueVec;
    
    class Func{
    public:
      virtual ~Func(){}
      
      virtual double operator()(double value) const = 0;
      
      virtual double deriv(double value) const = 0;
      
      virtual Func* copy() const = 0;
    };
    
    class LinearFunc : public Func{
    public:
      double operator()(double v) const{
        return v;
      }
      
      double deriv(double v) const{
        return 1;
      }
      
      virtual LinearFunc* copy() const{
        return new LinearFunc(*this);
      }
    };
    
    class SigmoidFunc : public Func{
    public:
      SigmoidFunc(double a)
      : a_(a){}
      
      double operator()(double v) const{
        return 1.0 / (1.0 + std::exp(-a_ * v));
      }
      
      double deriv(double v) const{
        double d = 1 + std::exp(-a_ * v);
        
        return (a_ * std::exp(-a_ * v))/(d*d);
      }
      
      virtual SigmoidFunc* copy() const{
        return new SigmoidFunc(*this);
      }
      
      double a() const{
        return a_;
      }
      
    private:
      double a_;
    };
    
    class SignumFunc : public Func{
    public:
      SignumFunc(double threshold)
      : threshold_(threshold){}
      
      double operator()(double v) const{
        return v < threshold_ ? -1.0 : 1.0;
      }
      
      double deriv(double v) const{
        return nvar::nan();
      }
      
      virtual SignumFunc* copy() const{
        return new SignumFunc(*this);
      }
      
    private:
      double threshold_;
    };
    
    static LinearFunc* linearFunc;
    static SigmoidFunc* sigmoidFunc;
    static SignumFunc* signumFunc;
    
    class Neuron{
    public:
      Neuron(Func& activationFunc)
      : activationFunc_(activationFunc.copy()),
      ownsActivationFunc_(true),
      totalInputs_(0),
      inputsReceived_(0),
      activation_(0),
      output_(nvar::nan()){}
      
      Neuron(Func* activationFunc)
      : activationFunc_(activationFunc),
      ownsActivationFunc_(false),
      totalInputs_(0),
      inputsReceived_(0),
      activation_(0),
      output_(nvar::nan()){}
      
      Neuron()
      : activationFunc_(linearFunc),
      ownsActivationFunc_(false),
      totalInputs_(0),
      inputsReceived_(0),
      activation_(0),
      output_(nvar::nan()){
        
        receive(0, 1.0);
      }
      
      ~Neuron(){
        if(ownsActivationFunc_){
          delete activationFunc_;
        }
      }
      
      void input(double value){
        activate_(0, value);
      }
      
      void receive(Neuron* inputNeuron, double weight){
        auto itr = inputMap_.find(inputNeuron);
        assert(itr == inputMap_.end());
        
        inputMap_.insert({inputNeuron, weight});
        ++totalInputs_;
        
        if(inputNeuron){
          itr = inputNeuron->outputMap_.find(this);
          assert(itr == inputNeuron->outputMap_.end());
          
          inputNeuron->outputMap_.insert({this, 0});
        }
      }
      
      double weight(Neuron* inputNeuron) const{
        auto itr = inputMap_.find(inputNeuron);
        assert(itr != inputMap_.end());
        
        return itr->second;
      }
      
      void setWeight(Neuron* inputNeuron, double weight){
        auto itr = inputMap_.find(inputNeuron);
        assert(itr != inputMap_.end());
        
        itr->second = weight;
      }
      
      void adjustWeight(Neuron* inputNeuron, double dw){
        auto itr = inputMap_.find(inputNeuron);
        assert(itr != inputMap_.end());
        
        itr->second += dw;
      }
      
      size_t inputCount() const{
        return inputMap_.size();
      }
      
      double output() const{
        assert(!std::isnan(output_));
        
        return output_;
      }
      
      void reset(){
        output_ = nvar::nan();
      }
      
      double activation() const{
        return activation_;
      }
      
      const Func& activationFunc() const{
        return *activationFunc_;
      }
      
    private:
      typedef NMap<Neuron*, double> NeuronValueMap_;
      
      void activate_(Neuron* inputNeuron, double value){
        auto itr = inputMap_.find(inputNeuron);
        assert(itr != inputMap_.end());
        
        activation_ += value * itr->second;
        
        ++inputsReceived_;
        
        if(inputsReceived_ == totalInputs_){
          output_ = (*activationFunc_)(activation_);
          
          itr = outputMap_.begin();
          auto itrEnd = outputMap_.end();
          
          while(itr != itrEnd){
            itr->first->activate_(this, output_);
            ++itr;
          }
          
          inputsReceived_ = 0;
          activation_ = 0;
        }
      }
      
      Func* activationFunc_;
      bool ownsActivationFunc_;
      NeuronValueMap_ inputMap_;
      NeuronValueMap_ outputMap_;
      uint32_t totalInputs_;
      uint32_t inputsReceived_;
      double activation_;
      double output_;
    };
    
    class Layer{
    public:
      Layer(){}
      
      Layer(size_t size){
        for(size_t i = 0; i < size; ++i){
          addNeuron(new Neuron);
        }
      }
      
      Layer(Func* activationFunc, size_t size){
        for(size_t i = 0; i < size; ++i){
          addNeuron(new Neuron(activationFunc));
        }
      }
      
      Layer(Func& activationFunc, size_t size){
        for(size_t i = 0; i < size; ++i){
          addNeuron(new Neuron(activationFunc));
        }
      }
      
      ~Layer(){
        for(size_t i = 0; i < neuronVec_.size(); ++i){
          delete neuronVec_[i];
        }
      }
      
      void addNeuron(Neuron* neuron){
        neuronVec_.push_back(neuron);
      }
      
      Neuron* neuron(size_t i){
        assert(i < neuronVec_.size());
        
        return neuronVec_[i];
      }
      
      size_t size() const{
        return neuronVec_.size();
      }
      
    private:
      typedef NVector<Neuron*> NeuronVec_;
      
      NeuronVec_ neuronVec_;
    };
    
    NNet()
    : bias_(0){
      random_.timeSeed();
    }
    
    ~NNet(){
      for(size_t i = 0; i < layerVec_.size(); ++i){
        delete layerVec_[i];
      }
    }
    
    void setSeed(int64_t seed){
      random_.setSeed(seed);
    }
    
    void addLayer(Func& activationFunc,
                  size_t size,
                  double wmin,
                  double wmax){
      Layer* l = new Layer(activationFunc, size);
      addLayer(l, wmin, wmax);
    }
    
    void addLayer(Func* activationFunc,
                  size_t size,
                  double wmin,
                  double wmax){
      Layer* l = new Layer(activationFunc, size);
      addLayer(l, wmin, wmax);
    }
    
    void addLayer(Layer* layer){
      addLayer(layer, 0, 0);
    }
    
    void addLayer(Layer* layer, double wmin, double wmax){
      Layer* l;
      
      if(layerVec_.empty()){
        l = 0;
      }
      else{
        l = layerVec_.back();
      }
      
      layerVec_.push_back(layer);
      
      if(l){
        connect_(l, layer, wmin, wmax);
      }
    }
    
    Layer* layer(size_t l){
      assert(l < layerVec_.size());
      
      return layerVec_[l];
    }
    
    Layer* backLayer(size_t l){
      return layer(layerVec_.size() - l - 1);
    }
    
    Neuron* neuron(size_t i, size_t j){
      Layer* l = layer(i);
      return l->neuron(j);
    }
    
    void addBias(double wmin, double wmax){
      assert(!bias_ && "Already added bias neuron");
      
      bias_ = new Neuron;
      for(size_t i = 1; i < layerVec_.size(); ++i){
        Layer* l = layerVec_[i];
        for(size_t j = 0; j < l->size(); ++j){
          Neuron* nj = l->neuron(j);
          nj->receive(bias_, random_.uniform(wmin, wmax));
        }
      }
    }
    
    Neuron* bias(){
      return bias_;
    }
    
    size_t numLayers() const{
      return layerVec_.size();
    }
    
    double error(const ValueVec& desired){
      Layer* l = backLayer(0);
      
      double error = 0;
      double e;
      
      size_t size = l->size();
      for(size_t j = 0; j < size; ++j){
        e = desired[j] - l->neuron(j)->output();
        e *= e;
        error += e;
      }
      
      error /= l->size();
      
      return error;
    }
    
    void backPropagate(const ValueVec& desired, double eta){
      Layer* l2 = backLayer(0);
      
      assert(desired.size() == l2->size());
      
      Layer* l1 = backLayer(1);
      
      ValueVec d2(l2->size(), 0);
      
      size_t size2 = l2->size();
      for(size_t j = 0; j < size2; ++j){
        Neuron* nj = l2->neuron(j);
        double e = desired[j] - nj->output();
        
        const Func& f = nj->activationFunc();
        d2[j] = e * f.deriv(nj->activation());
        
        size_t size1 = l1->size();
        
        for(size_t i = 0; i < size1; ++i){
          Neuron* ni = l1->neuron(i);
          double dw = eta * d2[j] * ni->output();
          nj->adjustWeight(ni, dw);
        }
      }
      
      size_t n = numLayers();
      for(size_t l = 2; l < n; ++l){
        ValueVec d1(l1->size(), 0);
        
        for(size_t j = 0; j < l1->size(); ++j){
          Neuron* nj = l1->neuron(j);
          const Func& f = nj->activationFunc();
          
          d1[j] = f.deriv(nj->activation());
          double s = 0;
          for(size_t k = 0; k < l2->size(); ++k){
            Neuron* nk = l2->neuron(k);
            s += nk->weight(nj) * d2[k];
          }
          
          d1[j] *= s;
        }
        
        l2 = l1;
        l1 = backLayer(l);
        d2 = d1;
      
        size_t size2 = l2->size();
        for(size_t j = 0; j < size2; ++j){
          Neuron* nj = l2->neuron(j);
          
          size_t size1 = l1->size();
          for(size_t i = 0; i < size1; ++i){
            Neuron* ni = l1->neuron(i);
            
            double dw = eta * d2[j] * ni->output();
            nj->adjustWeight(ni, dw);
          }
        }
      }
    }
    
  private:
    void connect_(Layer* l1, Layer* l2, double wmin, double wmax){
      size_t size2 = l2->size();
      for(size_t i = 0; i < size2; ++i){
        Neuron* ni = l2->neuron(i);
        
        size_t size1 = l1->size();
        for(size_t j = 0; j < size1; ++j){
          Neuron* nj = l1->neuron(j);
          ni->receive(nj, random_.uniform(wmin, wmax));
        }
      }
    }
    
    typedef NVector<Layer*> LayerVec_;
    
    LayerVec_ layerVec_;
    Neuron* bias_;
    NRandom random_;
  };
  
} // end namespace neu

#endif // NEU_N_NET_H

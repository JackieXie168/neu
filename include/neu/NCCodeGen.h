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

#ifndef NEU_C_CODE_GEN_H
#define NEU_C_CODE_GEN_H

#include <neu/NObject.h>

namespace neu{

class NConcept;

class NCCodeGen : public NObject{
public:
  NCCodeGen(size_t population);

  ~NCCodeGen();

  NFunc handle(const nvar& n, uint32_t flags);

  void shutdown();

  void addInput(const nstr& name, NConcept* input);

  void addOutput(const nstr& name, NConcept* output);

  void generate();

  void next();

  bool run();

  bool finish(double fitness);

  void reject();

  nvar getSolution();
  
  nvar getFinalSolution();

  nvar getSolution(size_t rank, nvar& solution);

  void setMergeRate(double rate);

  double mergeRate() const;

  void setRestartRate(double rate);

  double restartRate() const;

  void setSelectionPressure(double rate);

  double selectionPressure() const;

  void setUnusedBias(double bias);

  double unusedBias() const;

  void setMinComplexity(size_t c);

  size_t minComplexity() const;

  void setComplexity(size_t c);

  size_t complexity() const;

  void setMaxComplexity(size_t c);

  size_t maxComplexity() const;

  void setBacktrack(bool flag);

  void setFill(bool flag);

  nvar reset();

  nvar getCGG(nvar& cgg) const;

  nvar setResetInterval(size_t interval);

  nvar iteration() const;

  nvar round() const;

  nvar elapsedRoundTime() const;

  nvar elapsedRunTime() const;

  nvar population() const;

  nvar numSolutions() const;

  nvec getStats() const;

  void enableMatchLog();
  
  void enableErrorLog();

  void disableAll();

  void enable(const nstr& concept, const nstr& method, bool flag);

  void enable(const nstr& concept, bool flag);

private:
  class NCCodeGen_* x_;
};

} // end namespace neu

#endif // NEU_C_CODE_GEN_H

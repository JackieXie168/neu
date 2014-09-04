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

#ifndef NEU_NC_PROGRAM_H
#define NEU_NC_PROGRAM_H

#include <iostream>
#include <cmath>

#include <neu/NProgram.h>
#include <neu/NMLGenerator.h>
#include <neu/NCOntology.h>
#include <neu/NCCodeGen.h>

namespace neu{
  
  class NCProgram : public NProgram{
  public:
    NCProgram(int argc, char** argv)
    : codeGen_(0){
      
      opt("help", "h", false,
          "Display usage message");
      
      opt("verbose", "v", false,
          "True to enable verbose mode");
      
      opt("population", "p", 1000,
          "CodeGen population");
      
      opt("fill", "", false,
          "If true, then wait for the population to fill before "
          "starting to merge");
      
      opt("selectionPressure", "s", 0.5,
          "CodeGen selection pressure");
      
      opt("mergeRate", "m", 0.5,
          "CodeGen merge rate");
      
      opt("restartRate", "r", 1.0,
          "Restart rate");
      
      opt("unusedBias", "u", 5.0,
          "Unused variable bias");
      
      opt("minComplexity", "", 0,
          "Minimum solution complexity");
      
      opt("maxComplexity", "", 9999999,
          "Maximum solution complexity");
      
      opt("complexity", "c", 10,
          "Ideal solution complexity");
      
      opt("backtrack", "b", true,
          "True to enable backtracking");
      
      opt("resetInterval", "i", 9999999,
          "Reset interval");
      
      opt("desiredFitness", "f", nvar::max(),
          "Desired Fitness");
      
      opt("matchLog", "", false,
          "True to write a match log to match.log");
      
      opt("errorLog", "", false,
          "True to write an error log to error.log");
      
      opt("trials", "t", 1,
          "Number of trials to perform");
      
      NProgram::init(argc, argv);
    }
    
    ~NCProgram(){
      delete codeGen_;
    }
    
    virtual void setup() = 0;
    
    virtual nvar evaluate() = 0;
    
    void init(){
      const nvar& a = args();
      
      trials_ = a["trials"];
      
      std::cout << "trials: " << trials_ << std::endl;
      
      std::cout << "population: " << a["population"] << std::endl;
      std::cout << "selectionPressure: " << a["selectionPressure"] << std::endl;
      std::cout << "unusedBias: " << a["unusedBias"] << std::endl;
      std::cout << "mergeRate: " << a["mergeRate"] << std::endl;
      std::cout << "restartRate: " << a["restartRate"] << std::endl;
      std::cout << "minComplexity: " << a["minComplexity"] << std::endl;
      std::cout << "maxComplexity: " << a["maxComplexity"] << std::endl;
      std::cout << "complexity: " << a["complexity"] << std::endl;
      std::cout << "backtrack: " << a["backtrack"] << std::endl;
      std::cout << "resetInterval: " << a["resetInterval"] << std::endl;
      std::cout << "desiredFitness: " << a["desiredFitness"] << std::endl;
      
      std::cout << std::endl;
      
      NCOntology* ontology = NCOntology::get();
      codeGen_ = new NCCodeGen(a["population"]);
      
      setup();
      
      codeGen_->setFill(a["fill"]);
      codeGen_->setSelectionPressure(a["selectionPressure"]);
      codeGen_->setMergeRate(a["mergeRate"]);
      codeGen_->setRestartRate(a["restartRate"]);
      codeGen_->setUnusedBias(a["unusedBias"]);
      codeGen_->setResetInterval(a["resetInterval"]);
      codeGen_->setBacktrack(a["backtrack"]);
      codeGen_->setMinComplexity(a["minComplexity"]);
      codeGen_->setMaxComplexity(a["maxComplexity"]);
      codeGen_->setComplexity(a["complexity"]);
      
      if(a["matchLog"]){
        codeGen_->enableMatchLog();
      }
      
      if(a["errorLog"]){
        codeGen_->enableErrorLog();
      }
      
      codeGen_->generate();
    }
    
    void handleSignal(int signal, bool fatal){
      if(fatal){
        std::cout << "shutting down..." << std::endl;
        if(codeGen_){
          codeGen_->shutdown();
        }
        NProgram::exit(0);
      }
    }
    
    bool run(){
      return codeGen_->run();
    }
    
    NCCodeGen* getCodeGen(size_t population){
      const nvar& a = args();
      
      NCOntology* ontology = NCOntology::get();
      
      codeGen_ = new NCCodeGen(a["population"]);
      return codeGen_;
    }
    
    void printStats(nvar& stats){
      nvec s = codeGen_->getStats();
      
      size_t r = codeGen_->round();
      size_t i = codeGen_->iteration();
      
      double runTime = codeGen_->elapsedRunTime();
      double roundTime = codeGen_->elapsedRoundTime();
      
      std::cout << "----- round:            " << r << std::endl;
      std::cout << "----- iterations:       " << i << std::endl;
      std::cout << "----- run time:         " << runTime << std::endl;
      std::cout << "----- round time:       " << roundTime << std::endl;
      std::cout << "----- population size:  " << s[0] << std::endl;
      std::cout << "----- best fitness:     " << s[1] << std::endl;
      std::cout << "----- worst fitness:    " << s[2] << std::endl;
      std::cout << "----- avg. fitness:     " << s[3] << std::endl;
      std::cout << "----- avg. temps:       " << s[4] << std::endl;
      std::cout << "----- avg. norm. size:  " << s[5] << std::endl;
      std::cout << "----- avg. orig. size:  " << s[6] << std::endl;
      
      if(s[4] != 0){
        std::cout << "----- temp util:        " <<
        s[5].toDouble()/s[4] << std::endl;
      }
      
      std::cout << "----- avg. reduction:   " << s[7] << std::endl;
      std::cout << "----- total solutions:  " << s[8] << std::endl;
      std::cout << "----- total misses:     " << s[9] << std::endl;
      
      if(s[8] != 0){
        std::cout << "----- hit rate:         "
        << 1 - s[9].toDouble()/s[8] << std::endl;
      }
      
      std::cout << "----- total accepted:   " << s[10] << std::endl;
      std::cout << "----- total denied:     " << s[11] << std::endl;
      
      if(s[11] != 0){
        std::cout << "----- acceptance rate:  "
        << s[10].toDouble()/s[11] << std::endl;
      }
      
      std::cout << "----- total errors:     " << s[12] << std::endl;
      std::cout << "----- error rate:       " << double(s[12])/s[8] <<
      std::endl;
      
      std::cout << "----- total duplicates: " << s[13] << std::endl;
      
      std::cout << "----- duplication rate: " << double(s[13])/i << std::endl;
      
      stats("roundTime") = roundTime;
      stats("bestFitness") = s[1];
      stats("worstFitness") = s[2];
      stats("avgFitness") = s[3];
      stats("avgTemps") = s[4];
      stats("avgNormSize") = s[5];
      stats("avgOrigSize") = s[6];
      stats("avgReduction") = s[7];
      stats("misses") = s[9];
      stats("hitRate") = 1 - s[9].toDouble()/s[8];
      stats("accepted") = s[10];
      stats("denied") = s[11];
      stats("acceptanceRate") = s[11] != 0 ? s[10].toDouble()/s[11] : nvar(0.0);
      stats("errors") = s[12];
      stats("errorRate") = double(s[12])/s[8];
      stats("duplicates") = s[13];
      stats("duplicationRate") = double(s[13])/i;
    }
    
    void start(){
      const nvar& a = args();
      
      double desiredFitness = a["desiredFitness"];
      bool verbose = a["verbose"];
      
      nvec iterations;
      nvec roundTime;
      nvec bestFitness;
      nvec worstFitness;
      nvec avgFitness;
      nvec avgTemps;
      nvec avgNormSize;
      nvec avgOrigSize;
      nvec avgReduction;
      nvec misses;
      nvec hitRate;
      nvec accepted;
      nvec denied;
      nvec acceptanceRate;
      nvec errors;
      nvec errorRate;
      nvec duplicates;
      nvec duplicationRate;
      
      nvar maxFitness = nvar::negInf();
      
      for(size_t i = 0; i < trials_; ++i){
        for(;;){
          codeGen_->next();
          
          nvar fitness = evaluate();
          if(fitness == undef){
            codeGen_->reject();
            continue;
          }
          
          if(fitness >= desiredFitness){
            nvar stats;
            printStats(stats);
            
            std::cout << std::endl;
            
            nvar f = codeGen_->getFinalSolution();
            
            nstr nml = NMLGenerator::toStr(f);
            
            size_t iteration = codeGen_->iteration();
            size_t round = codeGen_->round();
            
            std::cout << "----- trial:      " << i + 1 << "/" << trials_
            << std::endl;
            std::cout << "----- rounds:     " << round + 1 << std::endl;
            std::cout << "----- iterations: " << iteration << std::endl;
            std::cout << "----- final solution" << std::endl;
            std::cout << nml << std::endl;
            
            if(trials_ > 1){
              iterations.push_back(iteration);
              roundTime.push_back(stats["roundTime"]);
              bestFitness.push_back(stats["bestFitness"]);
              worstFitness.push_back(stats["worstFitness"]);
              avgFitness.push_back(stats["avgFitness"]);
              avgTemps.push_back(stats["avgTemps"]);
              avgNormSize.push_back(stats["avgNormSize"]);
              avgOrigSize.push_back(stats["avgOrigSize"]);
              avgReduction.push_back(stats["avgReduction"]);
              misses.push_back(stats["misses"]);
              hitRate.push_back(stats["hitRate"]);
              accepted.push_back(stats["accepted"]);
              denied.push_back(stats["denied"]);
              acceptanceRate.push_back(stats["acceptanceRate"]);
              errors.push_back(stats["errors"]);
              errorRate.push_back(stats["errorRate"]);
              duplicates.push_back(stats["duplicates"]);
              duplicationRate.push_back(stats["duplicationRate"]);
              codeGen_->reject();
              codeGen_->reset();
            }
            
            break;
          }
          else if(codeGen_->finish(fitness)){
            bool newBest = fitness >= maxFitness;
            
            if(newBest){
              maxFitness = fitness;
            }
            
            if(verbose || newBest){
              if(newBest){
                std::cout << "----- new best fitness: ";
              }
              else{
                std::cout << "----- fitness: ";
              }
              
              std::cout << fitness << std::endl;
              
              nvar f = codeGen_->getFinalSolution();
              
              nstr nml = NMLGenerator::toStr(f);
              
              std::cout << nml << std::endl;
              
              nvar stats;
              printStats(stats);
              
              std::cout << std::endl;
            }
          }
        }
      }
      
      if(trials_ > 1){
        size_t totalIterations = 0;
        double totalRoundTime = 0;
        double totalBestFitness = 0;
        double totalWorstFitness = 0;
        double totalAvgFitness = 0;
        double totalAvgTemps = 0;
        double totalAvgNormSize = 0;
        double totalAvgOrigSize = 0;
        double totalAvgReduction = 0;
        double totalMisses = 0;
        double totalHitRate = 0;
        double totalAccepted = 0;
        double totalDenied = 0;
        double totalAcceptanceRate = 0;
        double totalErrors = 0;
        double totalErrorRate = 0;
        double totalDuplicates = 0;
        double totalDuplicationRate = 0;
        
        for(const nvar& i : iterations){
          totalIterations += i.toLong();
        }
        
        for(const nvar& i : roundTime){
          totalRoundTime += i.toDouble();
        }
        
        for(const nvar& i : bestFitness){
          totalBestFitness += i.toDouble();
        }
        
        for(const nvar& i : worstFitness){
          totalWorstFitness += i.toDouble();
        }
        
        for(const nvar& i : avgFitness){
          totalAvgFitness += i.toDouble();
        }
        
        for(const nvar& i : avgTemps){
          totalAvgTemps += i.toDouble();
        }
        
        for(const nvar& i : avgNormSize){
          totalAvgNormSize += i.toDouble();
        }
        
        for(const nvar& i : avgOrigSize){
          totalAvgOrigSize += i.toDouble();
        }
        
        for(const nvar& i : avgReduction){
          totalAvgReduction += i.toDouble();
        }
        
        for(const nvar& i : misses){
          totalMisses += i.toDouble();
        }
        
        for(const nvar& i : hitRate){
          totalHitRate += i.toDouble();
        }
        
        for(const nvar& i : accepted){
          totalAccepted += i.toDouble();
        }
        
        for(const nvar& i : denied){
          totalDenied += i.toDouble();
        }
        
        for(const nvar& i : acceptanceRate){
          totalAcceptanceRate += i.toDouble();
        }
        
        for(const nvar& i : errors){
          totalErrors += i.toDouble();
        }
        
        for(const nvar& i : errorRate){
          totalErrorRate += i.toDouble();
        }
        
        for(const nvar& i : duplicates){
          totalDuplicates += i.toDouble();
        }
        
        for(const nvar& i : duplicationRate){
          totalDuplicationRate += i.toDouble();
        }
        
        double meanIterations = double(totalIterations)/trials_;
        double meanRoundTime = totalRoundTime/trials_;
        double meanBestFitness = totalBestFitness/trials_;
        double meanWorstFitness = totalWorstFitness/trials_;
        double meanAvgFitness = totalAvgFitness/trials_;
        double meanAvgTemps = totalAvgTemps/trials_;
        double meanAvgNormSize = totalAvgNormSize/trials_;
        double meanAvgOrigSize = totalAvgOrigSize/trials_;
        double meanAvgReduction = totalAvgReduction/trials_;
        double meanMisses = totalMisses/trials_;
        double meanHitRate = totalHitRate/trials_;
        double meanAccepted = totalAccepted/trials_;
        double meanDenied = totalDenied/trials_;
        double meanAcceptanceRate = totalAcceptanceRate/trials_;
        double meanErrors = totalErrors/trials_;
        double meanErrorRate = totalErrorRate/trials_;
        double meanDuplicates = totalDuplicates/trials_;
        double meanDuplicationRate = totalDuplicationRate/trials_;
        
        double totalDeviation = 0;
        for(const nvar& i : iterations){
          double di = i - meanIterations;
          di *= di;
          totalDeviation += di;
        }
        
        double stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "trials: " << trials_ << std::endl;
        std::cout << "mean iterations: " << meanIterations << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : roundTime){
          double di = i - meanRoundTime;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean round time: " << meanRoundTime << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : bestFitness){
          double di = i - meanBestFitness;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean best fitness: " << meanBestFitness << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : worstFitness){
          double di = i - meanWorstFitness;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean worst fitness: " << meanWorstFitness << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : avgFitness){
          double di = i - meanAvgFitness;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean avg. fitness: " << meanAvgFitness << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : avgTemps){
          double di = i - meanAvgTemps;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean avg. temps: " << meanAvgTemps << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : avgNormSize){
          double di = i - meanAvgNormSize;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean avg. norm. size: " << meanAvgNormSize << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : avgOrigSize){
          double di = i - meanAvgOrigSize;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean avg. orig. size: " << meanAvgOrigSize << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : avgReduction){
          double di = i - meanAvgReduction;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean avg. reduction: " << meanAvgReduction << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : misses){
          double di = i - meanMisses;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean misses: " << meanMisses << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : hitRate){
          double di = i - meanHitRate;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean hit rate: " << meanHitRate << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : accepted){
          double di = i - meanAccepted;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean accepted: " << meanAccepted << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : denied){
          double di = i - meanDenied;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean denied: " << meanDenied << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : acceptanceRate){
          double di = i - meanAcceptanceRate;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean acceptance rate: " << meanAcceptanceRate << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : errors){
          double di = i - meanErrors;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean errors: " << meanErrors << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl <<
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : errorRate){
          double di = i - meanErrorRate;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean error rate: " << meanErrorRate << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl << 
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : duplicates){
          double di = i - meanDuplicates;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean duplicates: " << meanDuplicates << std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl << 
        std::endl;
        
        // -------------------------
        
        totalDeviation = 0;
        for(const nvar& i : duplicationRate){
          double di = i - meanDuplicationRate;
          di *= di;
          totalDeviation += di;
        }
        
        stdDeviation = std::sqrt(totalDeviation/(trials_ - 1));
        
        std::cout << "mean duplication rate: " << meanDuplicationRate << 
        std::endl;
        std::cout << "std. deviation: " << stdDeviation << std::endl << 
        std::endl;
      }
      
      codeGen_->shutdown();
    }
    
    void addInput(const nstr& name, NConcept* input){
      codeGen_->addInput(name, input);
    }
    
    void addOutput(const nstr& name, NConcept* output){
      codeGen_->addOutput(name, output);
    }
    
    void disableAll(){
      codeGen_->disableAll();
    }
    
    void enable(const nstr& concept, const nstr& method, bool flag){
      codeGen_->enable(concept, method, flag);
    }
    
    void enable(const nstr& concept, bool flag){
      codeGen_->enable(concept, flag);
    }
    
  private:
    NCCodeGen* codeGen_;
    size_t trials_;
  };
  
} // end namespace neu

#endif // NEU_NC_PROGRAM_H

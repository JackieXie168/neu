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

#ifndef NEU_N_DATABASE_H
#define NEU_N_DATABASE_H

#include <neu/nvar.h>

#include <set>
#include <algorithm>

namespace neu{
  
  class NTable{
  public:
    
    class RowSet{

      void insert(uint64_t rowId){
        set_.insert(rowId);
      }
      
      void unite(const RowSet& s){
        Set_ r;
        
        std::set_union(set_.begin(), set_.end(),
                       s.set_.begin(), s.set_.end(),
                       std::inserter(r, r.begin()));
        
        set_ = std::move(r);
      }
      
      void intersect(const RowSet& s){
        Set_ r;
        
        std::set_intersection(set_.begin(), set_.end(),
                              s.set_.begin(), s.set_.end(),
                              std::inserter(r, r.begin()));
        
        set_ = std::move(r);
      }
      
      void complement(const RowSet& s){
        Set_ r;
        
        std::set_difference(set_.begin(), set_.end(),
                            s.set_.begin(), s.set_.end(),
                            std::inserter(r, r.begin()));
        
        set_ = std::move(r);
      }
      
    private:
      typedef std::set<uint64_t> Set_;
      
      Set_ set_;
    };
    
    typedef uint8_t IndexType;
    
    static const IndexType Integer =    0;
    static const IndexType Float =      1;
    static const IndexType Hash =       2;
    
    void addIndex(const nstr& indexName, IndexType indexType);
    
    uint64_t insert(const nvar& row);
    
    void update(const nvar& row);
    
    bool get(uint64_t rowId, nvar& row);
    
    void find(const nstr& indexName, const nvar& value, nvar& row);
    
    void forward(nvar& row);
    
    void back(nvar& row);
    
    void erase(uint64_t rowId);
    
    void compact();
    
    void dump();
    
    friend class NDatabase_;
    
  private:
    NTable();
    
    class NTable_* x_;
  };
  
  class NDatabase{
  public:
    NDatabase(const nstr& path);
    
    NTable* addTable(const nstr& tableName);
    
    NTable* getTable(const nstr& tableName);
    
    static NDatabase* create(const nstr& path);
    
  private:
    NDatabase();
    
    class NDatabase_* x_;
  };
  
} // end namespace neu
  
#endif // NEU_N_DATABASE_H

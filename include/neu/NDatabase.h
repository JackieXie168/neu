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

Neu can be used freely for commercial purposes. If you find Neu
useful, please consider helping to support our work and the evolution
of Neu by making a PayPal donation to: neu@andrometa.net

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

namespace neu{
  
  class NTable{
  public:

    class Lock{
    public:
      Lock(NTable* table, bool write=false){
        lockMap_.insert({table, write});
        acquire_();
      }
      
      Lock(std::initializer_list<NTable*> tables, bool write=false){
        for(auto& itr : tables){
          lockMap_.insert({itr, write});
        }
        
        acquire_();
      }
      
      Lock(std::initializer_list<NTable*> readTables,
           std::initializer_list<NTable*> writeTables){
        NMap<NTable*, bool> m;

        for(auto& itr : writeTables){
          lockMap_.insert({itr, true});
        }
        
        for(auto& itr : readTables){
          lockMap_.insert({itr, false});
        }
        
        acquire_();
      }

      ~Lock(){
        release();
      }
      
      void release(){
        for(auto& itr : lockMap_){
          NTable* table = itr.first;
          table->unlock_();
        }
        
        lockMap_.clear();
      }
      
    private:
      void acquire_(){
        for(auto& itr : lockMap_){
          NTable* table = itr.first;
          bool write = itr.second;

          if(write){
            table->writeLock_();
          }
          else{
            table->readLock_();
          }
        }
      }

      typedef NMap<NTable*, bool> LockMap_;
      
      LockMap_ lockMap_;
    };
  
    typedef uint64_t RowId;
    
    typedef NHashSet<RowId> RowSet;
    
    enum IndexType{
      UInt32,
      Int32,
      UInt64,
      Int64,
      Float,
      Double,
      Row,
      Hash
    };
    
    typedef std::function<int(const nvar& r)> QueryFunc;
    
    void addIndex(const nstr& indexName,
                  IndexType indexType,
                  bool unique=false);
    
    void addRowIndex(const nstr& indexName,
                     bool unique=false,
                     bool autoErase=false);
    
    RowId insert(nvar& row);
    
    void update(nvar& row);
    
    bool get(RowId rowId, nvar& row);
    
    void erase(RowId rowId);
    
    void query(const nstr& indexName,
               const nvar& start,
               QueryFunc f);
    
    void setQuery(const nstr& indexName,
                  const nvar& start,
                  const nvar& end,
                  RowSet& rs);
    
    void traverseStart(QueryFunc f);
    
    void traverseEnd(QueryFunc f);

    void join(const nstr& indexName, const RowSet& js, RowSet& rs);

    void get(const RowSet& rs, QueryFunc f);
    
    bool get(const nstr& indexName, const nvar& value, nvar& row);
    
    void commit();
    
    void rollback();
    
    void dump();
    
    void readLock_();
    
    void writeLock_();
    
    void unlock_();
    
  private:
    friend class NDatabase_;
    
    NTable(class NDatabase_*, const nstr&, bool);
    
    class NTable_* x_;
  };
  
  class NDatabase{
  public:
    NDatabase(const nstr& path, bool create);
    
    NTable* addTable(const nstr& tableName);
    
    NTable* getTable(const nstr& tableName);

    void eraseTable(const nstr& tableName);
    
    void commit();

    void rollback();
    
    void compact();
    
    void setMemoryLimit(size_t megabytes);
    
  private:
    class NDatabase_* x_;
  };
  
} // end namespace neu
  
#endif // NEU_N_DATABASE_H

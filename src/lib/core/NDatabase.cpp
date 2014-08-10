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

#include <neu/NDatabase.h>

#include <neu/NEncoder.h>

using namespace std;
using namespace neu;

namespace{
  
  static const uint8_t DataIndexType = 255;
  
  static const size_t MAX_CHUNK_SIZE = 65536/4;
  static const size_t MAX_CHUNKS = 1048*4;
  static const size_t MAX_DATA_SIZE = 16777216;
  
  static const size_t SPLIT_CHUNK_SIZE = MAX_CHUNK_SIZE - 1;
  
  static const uint64_t ErasedFlag = 0x1;
  
  template<typename T>
  void max(T& m){
    m = numeric_limits<T>::max();
  }
  
  void max(double& m){
    m = numeric_limits<double>::infinity();
  }
  
  struct Hash{
    bool operator<(const Hash& h2) const{
      return h2.h[0] < h[0] &&
      h2.h[1] < h[1] &&
      h2.h[2] < h[2] &&
      h2.h[3] < h[3];
    }
    
    uint64_t h[4];
  };
  
  void max(Hash& m){
    uint64_t mi = numeric_limits<uint64_t>::max();
    m = {mi, mi, mi, mi};
  }
  
} // end namespace

namespace neu{
  
  class NTable_{
  public:
    
    typedef uint8_t Action;
    
    static const Action None = 0x0;
    static const Action Remap = 0x1;
    static const Action Split = 0x2;
    static const Action RemapSplit = 0x3;
    
    template<class R, class V>
    class Page{
    public:
      class Chunk{
      public:
        
        Chunk(){
          
        }
        
        Action findInsert(const V& value, size_t& index){
          index = 0;
          
          R* record;
          size_t length = chunk_.size();
          size_t start = 0;
          size_t end = length;
          bool after = false;
          
          while(end > start){
            index = start + (end - start)/2;

            record = &chunk_[index];
            
            if(value < record->value){
              end = index;
              after = false;
            }
            else{
              start = index + 1;
              after = true;
            }
          }
          
          index = after ? index + 1 : index;

          Action action;
          
          if(length == SPLIT_CHUNK_SIZE){
            action |= Split;
          }

          if(index == length){
            action |= Remap;
          }

          return action;
        }

        R* get(const V& value){
          R* record = 0;
          
          size_t start = 0;
          size_t end = chunk_.size();
          size_t index = 0;
          
          while(end > start){
            index = start + (end - start)/2;
            
            record = &chunk_[index];

            if(value < record->value){
              end = index;
            }
            else if(value > record->value){
              start = index + 1;
            }
            else{
              return record;
            }
          }

          return record->value == value ? record : 0;
        }
        
        Action insert(const R& record){
          size_t index;
          if(findInsert(record.value, index) == Remap){
            chunk_.push_back(record);
            return Remap;
          }
          else{
            chunk_.insert(chunk_.begin() + index, record);
            return None;
          }
        }

        void push(const R& record){
          chunk_.push_back(record);
        }
        
        size_t size(){
          return chunk_.size();
        }
        
        Chunk* split(){
          Chunk* c = new Chunk;
          
          auto itr = chunk_.begin();
          itr += chunk_.size()/2;
          
          c->chunk_.insert(c->chunk_.begin(), itr, chunk_.end());

          chunk_.erase(itr, chunk_.end());
          
          return c;
        }
        
        const V& maxValue() const{
          return chunk_.back().value;
        }
        
        void dump(){
          for(size_t i = 0; i < chunk_.size(); ++i){
            const R& r = chunk_[i];
            r.dump();
          }
        }
        
      private:
        typedef vector<R> Chunk_;
        
        Chunk_ chunk_;
      };
      
      Page(uint64_t id)
      : id_(id),
      loaded_(true),
      lastChunk_(0){

      }
      
      bool handleFirst(const R& record){
        if(lastChunk_){
          return false;
        }
        
        lastChunk_ = new Chunk;
        lastChunk_->push(record);
        chunkMap_.insert({record.value, lastChunk_});
        return true;
      }
      
      Action insert(const R& record){
        if(!loaded_){
          load();
        }
       
        if(handleFirst(record)){
          return Remap;
        }

        bool last;
        auto itr = chunkMap_.lower_bound(record.value);
        Chunk* chunk = itr->second;
        Action action = chunk->insert(record);
        
        if(chunk->size() >= MAX_CHUNK_SIZE){
          if(chunkMap_.size() >= MAX_CHUNKS){
            if(action == Remap){
              chunkMap_.erase(itr);
              chunkMap_.insert({chunk->maxValue(), chunk});
            }
            return Split;
          }
          else{
            Chunk* c = chunk->split();
            chunkMap_.erase(itr);
            chunkMap_.insert({chunk->maxValue(), chunk});
            chunkMap_.insert({c->maxValue(), c});
            return last ? Remap : None;
          }
        }
        
        if(action == Remap){
          chunkMap_.erase(itr);
          chunkMap_.insert({chunk->maxValue(), chunk});
        }

        return last ? Remap : None;
      }
      
      Action push(const R& record){
        if(handleFirst(record)){
          return Remap;
        }

        Chunk* chunk;
        
        if(chunkMap_.empty()){
          chunk = new Chunk;
          chunk->push(record);
          chunkMap_.insert({record.value, chunk});
          return None;
        }
        
        auto itr = chunkMap_.end();
        --itr;
        
        lastChunk_->push(record);
        
        if(lastChunk_->size() >= MAX_CHUNK_SIZE){
          if(chunkMap_.size() >= MAX_CHUNKS){
            return Split;
          }
          else{
            Chunk* c = chunk->split();
            chunkMap_.insert({c->maxValue(), c});
            return None;
          }
        }
        
        return None;
      }
      
      R* get(const V& value){
        if(!loaded_){
          load();
        }
        
        if(chunkMap_.empty()){
          return 0;
        }
        
        auto itr = chunkMap_.lower_bound(value);
        if(itr == chunkMap_.end()){
          --itr;
        }
        
        return itr->second->get(value);
      }
      
      Page* split(uint64_t id){
        Page* p = new Page(id);
        
        size_t i = 0;
        size_t end = MAX_CHUNKS/2;
        
        while(i < end){
          auto itr = chunkMap_.begin();
          assert(itr != chunkMap_.end());
          
          p->chunkMap_.insert({itr->first, itr->second});
          chunkMap_.erase(itr);
          ++i;
        }
        
        return p;
      }
      
      void load(){
        loaded_ = true;
      }
      
      const V& maxValue(){
        auto itr = chunkMap_.rbegin();
        assert(itr != chunkMap_.rend());
        
        return itr->second->maxValue();
      }
      
      void dump(){
        for(auto& itr : chunkMap_){
          cout << "@@@ CHUNK UPPER: " << itr.first << endl;
          itr.second->dump();
        }
      }
      
    private:
      typedef NMap<V, Chunk*> ChunkMap_;
      
      uint64_t id_;
      bool loaded_;
      ChunkMap_ chunkMap_;
      Chunk* lastChunk_;
    };
    
    class IndexBase{
    public:
      IndexBase(uint8_t type)
      : type_(type){
        
      }
      
      uint8_t type(){
        return type_;
      }
      
      virtual void dump(){}
      
    private:
      uint8_t type_;
    };
    
    template<typename R, typename V>
    class Index : public IndexBase{
    public:
      typedef Page<R, V> IndexPage;
      
      Index(uint8_t type)
      : IndexBase(type),
      nextPageId_(0){
        max(max_);
        
        lastPage_ = new IndexPage(nextPageId_++);
        pageMap_.insert({max_, lastPage_});
      }
      
      virtual ~Index(){
        
      }
      
      void insertRecord(const R& record){
        auto itr = pageMap_.lower_bound(record.value);
        IndexPage* page = itr->second;
        
        Action action = page->insert(record);
        
        if(action == Split){
          IndexPage* p = page->split(nextPageId_++);
          pageMap_.insert({p->maxValue(), p});
        }
        else if(action == Remap && itr.first != max_){
          pageMap_.erase(itr);
          pageMap_.insert({page->maxValue(), page});
        }
      }
      
      void pushRecord(const R& record){
        Action action = lastPage_->push(record);
        
        if(action == Split){
          IndexPage* p = lastPage_->split(nextPageId_++);
          pageMap_.insert({p->maxValue(), p});
        }
      }
      
      R* getRecord(const V& value){
        auto itr = pageMap_.lower_bound(value);
        return itr->second->get(value);
      }
      
      void dump(){
        for(auto& itr : pageMap_){
          cout << "@@@@@@@ PAGE: " << itr.first << endl;
          itr.second->dump();
        }
      }
      
    private:
      typedef NMap<V, IndexPage*> PageMap_;
      
      V max_;
      uint64_t nextPageId_;
      PageMap_ pageMap_;
      IndexPage* lastPage_;
    };

    struct DataRecord{
      uint64_t value;
      uint32_t dataId : 31;
      uint32_t flags : 1;
      uint32_t offset;
      
      void dump() const{
        cout << "value: " << value << "; dataId: " << dataId <<
        "; offset: " << offset << endl;
      }
    };
    
    class DataIndex : public Index<DataRecord, uint64_t>{
    public:
      
      DataIndex()
      : Index(DataIndexType){
        
      }
      
      void insert(uint32_t dataId, uint32_t offset, uint64_t rowId){
        record_.dataId = dataId;
        record_.offset = offset;
        record_.value = rowId;
        record_.flags = 0;
        
        pushRecord(record_);
      }

      bool get(uint64_t rowId, uint32_t& dataId, uint32_t& offset){
        DataRecord* record = getRecord(rowId);
        if(record){
          dataId = record->dataId;
          offset = record->offset;
          return true;
        }
        
        return false;
      }
      
      void erase(uint64_t rowId){
        DataRecord* record = getRecord(rowId);
        
        if(record){
          record->flags = ErasedFlag;
        }
      }
      
    private:
      DataRecord record_;
    };

    struct IntegerRecord{
      int64_t value;
      uint64_t rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
    };
    
    class IntegerIndex : public Index<IntegerRecord, int64_t>{
    public:
      IntegerIndex()
      : Index(NTable::Integer){
        
      }
      
      void insert(uint64_t rowId, int64_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      IntegerRecord record_;
    };

    struct FloatRecord{
      double value;
      uint64_t rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
    };
    
    class FloatIndex : public Index<FloatRecord, double>{
    public:
      FloatIndex()
      : Index(NTable::Float){
        
      }
      
      void insert(uint64_t rowId, double value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      FloatRecord record_;
    };
    
    /*
    class HashIndex : public Index{
    public:
      struct Record{
        Hash value;
        uint64_t rowId;
        
        void dump() const{
          cout << "<<hash>>";
        }
      };
      
      typedef Page<Record, Hash> HashPage;
      
      void insert(uint64_t rowId, const nvar& v){
        nvar h;
        NEncoder::sha256(v.toStr(), h);
        
        Hash value;
        value.h[0] = h[0];
        value.h[1] = h[1];
        value.h[2] = h[2];
        value.h[3] = h[3];
        
        HashPage* page;
        
        bool erase;
        auto itr = findSlot(pageMap_, value);
        if(itr == pageMap_.end()){
          page = new HashPage(nextPageId_++);
          erase = false;
        }
        else{
          page = itr->second;
          erase = true;
        }
        
        record_.rowId = rowId;
        record_.value = value;
        
        if(page->insert(record_)){
          HashPage* p = page->split(nextPageId_++);
          p->insert(record_);
          pageMap_.insert({p->maxValue(), p});
        }
        
        if(erase){
          pageMap_.erase(itr);
        }
        
        pageMap_.insert({page->maxValue(), page});
      }
      
    private:
      typedef NMap<Hash, HashPage*> PageMap_;
      
      uint64_t nextPageId_;
      PageMap_ pageMap_;
      Record record_;
    };
     */
    
    class Data{
    public:
      Data(uint64_t id)
      : data_(0),
      size_(0),
      id_(id){
        
      }
      
      size_t size(){
        return size_;
      }
      
      uint64_t id(){
        return id_;
      }
      
      uint32_t insert(char* buf, uint32_t size){
        uint32_t offset = size_;
        
        if(data_){
          data_ = (char*)realloc(data_, size_ + size + 5);
        }
        else{
          data_ = (char*)malloc(size_ + size + 5);
        }
        
        memcpy(data_ + size_, &size, 4);
        size_ += 4;
        data_[size_++] = 0;
        memcpy(data_ + size_, buf, size);
        size_ += size;
        
        return offset;
      }
      
      void get(uint32_t offset, nvar& v){
        // need to load
        
        uint32_t size;
        memcpy(&size, data_ + offset, 4);
        v.unpack(data_ + offset + 5, size);
      }
      
    private:
      uint32_t size_;
      char* data_;
      uint64_t id_;
    };
    
    NTable_(NTable* o)
    : o_(o),
    nextRowId_(1),
    nextDataId_(0),
    lastData_(0){
      
    }
    
    void setName(const nstr& name){
      name_ = name;
    }
    
    void addIndex(const nstr& indexName, uint8_t indexType){
      auto itr = indexMap_.find(indexName);
      
      if(itr != indexMap_.end()){
        NERROR("index exists: " + indexName);
      }

      IndexBase* index;
      
      switch(indexType){
        case NTable::Integer:
          index = new IntegerIndex;
          break;
        case NTable::Float:
          index = new FloatIndex;
          break;
          /*
        case NTable::Hash:
          index = new HashIndex;
          break;
           */
        case DataIndexType:
          index = new DataIndex;
          break;
        default:
          NERROR("invalid index type");
      }
      
      indexMap_.insert({indexName, index});
    }
    
    uint64_t insert(const nvar& row){
      uint64_t rowId = nextRowId_++;
      
      uint32_t size;
      char* buf = row.pack(size);
      
      const nmap& m = row;
      for(auto& itr : m){
        const nvar& k = itr.first;
        const nvar& v = itr.second;
        
        if(k.isSymbol()){
          auto iitr = indexMap_.find(k);
          if(iitr != indexMap_.end()){
            IndexBase* index = iitr->second;
            
            switch(index->type()){
              case NTable::Integer:{
                IntegerIndex* i = static_cast<IntegerIndex*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::Float:{
                FloatIndex* i = static_cast<FloatIndex*>(index);
                i->insert(rowId, v);
                break;
              }
              default:
                assert(false);
            }
          }
        }
      }
      
      Data* data;
      
      if(lastData_ && lastData_->size() + size <= MAX_DATA_SIZE){
        data = lastData_;
      }
      else{
        data = 0;
        
        for(auto& itr : dataMap_){
          Data* d = itr.second;
          if(d->size() + size <= MAX_DATA_SIZE){
            data = d;
          }
        }
        
        if(!data){
          uint64_t id = nextDataId_++;
          data = new Data(id);
          dataMap_.insert({id, data});
        }
      }
      
      uint32_t offset = data->insert(buf, size);
      free(buf);
      
      dataIndex_.insert(data->id(), offset, rowId);
      lastData_ = data;

      return rowId;
    }
    
    void update(const nvar& row){
      
    }
    
    void find(const nstr& indexName, const nvar& value, nvar& row){
      auto itr = indexMap_.find(indexName);
      
      if(itr == indexMap_.end()){
        NERROR("invalid index: " + indexName);
      }
    }
    
    bool get(uint64_t rowId, nvar& row){
      uint32_t dataId;
      uint32_t offset;
      
      if(!dataIndex_.get(rowId, dataId, offset)){
        return false;
      }
      
      auto itr = dataMap_.find(dataId);
      assert(itr != dataMap_.end());
      
      Data* data = itr->second;
      data->get(offset, row);
      
      return true;
    }
    
    void forward(nvar& row){
      
    }
    
    void back(nvar& row){
      
    }
    
    NTable* outer(){
      return o_;
    }
    
    void erase(uint64_t rowId){
      dataIndex_.erase(rowId);
    }
    
    void compact(){
      NTable::RowSet rs;
    }
    
    void dump(){
      cout << "+++++++++ DUMP TABLE: " << name_ << endl;
      
      for(auto& itr : indexMap_){
        cout << "---- DUMP INDEX: " << itr.first << endl;
        itr.second->dump();
      }
      
      dataIndex_.dump();
    }
    
  private:
    typedef NMap<nstr, IndexBase*> IndexMap_;
    typedef NMap<uint64_t, Data*> DataMap_;
    
    NTable* o_;
    nstr name_;
    IndexMap_ indexMap_;
    uint64_t nextRowId_;
    uint32_t nextDataId_;
    DataMap_ dataMap_;
    Data* lastData_;
    DataIndex dataIndex_;
  };
  
  class NDatabase_{
  public:
    NDatabase_(NDatabase* o, const nstr& path, bool create)
    : o_(o),
    path_(path){
      
    }
    
    NTable* addTable(const nstr& tableName){
      auto itr = tableMap_.find(tableName);
      if(itr != tableMap_.end()){
        NERROR("table exists: " + tableName);
      }
      
      NTable* table = new NTable;
      NTable_* t = table->x_;
      t->setName(tableName);
      
      tableMap_.insert({tableName, t});
      
      return table;
    }
    
    NTable* getTable(const nstr& tableName){
      auto itr = tableMap_.find(tableName);
      if(itr == tableMap_.end()){
        NERROR("invalid table: " + tableName);
      }
      
      return itr->second->outer();
    }
    
  private:
    typedef NMap<nstr, NTable_*> TableMap_;
    
    NDatabase* o_;
    nstr path_;
    TableMap_ tableMap_;
  };
  
} // end namespace neu

NTable::NTable(){
  x_ = new NTable_(this);
}

void NTable::addIndex(const nstr& indexName, IndexType indexType){
  x_->addIndex(indexName, indexType);
}

uint64_t NTable::insert(const nvar& row){
  return x_->insert(row);
}

void NTable::update(const nvar& row){
  x_->update(row);
}

void NTable::find(const nstr& indexName, const nvar& value, nvar& row){
  x_->find(indexName, value, row);
}

bool NTable::get(uint64_t rowId, nvar& row){
  return x_->get(rowId, row);
}

void NTable::forward(nvar& row){
  x_->forward(row);
}

void NTable::back(nvar& row){
  x_->back(row);
}

void NTable::erase(uint64_t rowId){
  x_->erase(rowId);
}

void NTable::compact(){
  x_->compact();
}

void NTable::dump(){
  x_->dump();
}

NDatabase::NDatabase(){}

NDatabase::NDatabase(const nstr& path){
  x_ = new NDatabase_(this, path, false);
}

NTable* NDatabase::addTable(const nstr& tableName){
  return x_->addTable(tableName);
}

NTable* NDatabase::getTable(const nstr& tableName){
  return x_->getTable(tableName);
}

NDatabase* NDatabase::create(const nstr& path){
  NDatabase* db = new NDatabase;
  db->x_ = new NDatabase_(db, path, true);
  
  return db;
}

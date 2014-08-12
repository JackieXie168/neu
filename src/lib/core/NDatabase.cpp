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
#include <neu/NHashMap.h>
#include <neu/NSys.h>

using namespace std;
using namespace neu;

namespace{
  
  static const uint8_t DataIndexType = 255;
  
  static const size_t MAX_CHUNK_SIZE = 65536;
  static const size_t MAX_CHUNKS = 1024;
  static const size_t MAX_DATA_SIZE = 16777216;
  
  static const size_t SPLIT_CHUNK_SIZE = MAX_CHUNK_SIZE - 1;
  
  template<typename T>
  void min(T& m){
    m = numeric_limits<T>::min();
  }
  
  void min(double& m){
    m = -numeric_limits<double>::infinity();
  }
  
  void min(float& m){
    m = -numeric_limits<float>::infinity();
  }
  
  template<typename T>
  void max(T& m){
    m = numeric_limits<T>::max();
  }
  
  void max(double& m){
    m = numeric_limits<double>::infinity();
  }
  
  void max(float& m){
    m = numeric_limits<float>::infinity();
  }
  
  typedef typename NTable::RowId RowId;
  
  typedef typename NTable::RowSet RowSet;
  
  typedef NHashMap<RowId, RowId> UpdateMap;
  
  typedef function<int(RowId, const nvar&)> QueryFunc_;
  
} // end namespace

namespace neu{
  
  class NTable_;
  
  class NDatabase_{
  public:
    NDatabase_(NDatabase* o, const nstr& path, bool create)
    : o_(o),
    path_(path),
    nextRowId_(1){
      if(create){
        if(NSys::exists(path)){
          NERROR("path exists: " + path);
        }
        
        NSys::makeDir(path);
      }
    }
    
    NTable* addTable(const nstr& tableName);
    
    NTable* getTable(const nstr& tableName);

    RowId nextRowId(){
      return nextRowId_++;
    }
    
    void compact();
    
    const nstr& path(){
      return path_;
    }
    
  private:
    typedef NMap<nstr, NTable_*> TableMap_;
    
    NDatabase* o_;
    nstr path_;
    RowId nextRowId_;
    TableMap_ tableMap_;
  };
  
  class NTable_{
  public:
    
    typedef uint8_t Action;
    
    static const Action None = 0x0;
    static const Action Remap = 0x1;
    static const Action Split = 0x2;
    static const Action RemapSplit = 0x3;
    static const Action Append = 0x4;
    
    class IndexBase{
    public:
      IndexBase(uint8_t type)
      : type_(type){
        
      }
      
      uint8_t type(){
        return type_;
      }
      
      void setPath(const nstr& path){
        path_ = path;
      }
      
      const nstr& path(){
        return path_;
      }
      
      virtual void dump(){}
      
    private:
      uint8_t type_;
      nstr path_;
    };
    
    template<class R, class V>
    class Page{
    public:
      typedef function<void(R& r)> TraverseFunc;
      
      class Chunk{
      public:
        
        typedef function<void(R& r)> TraverseFunc;
        
        Action find(const V& value, size_t& index){
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

          Action action = None;
          
          if(length == SPLIT_CHUNK_SIZE){
            action |= Split;
          }

          if(index == length){
            action |= Append;
          }
          else if(index == 0){
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
          Action action = find(record.value, index);

          if(find(record.value, index) & Append){
            chunk_.push_back(record);
          }
          else{
            chunk_.insert(chunk_.begin() + index, record);
          }
          
          return action;
        }

        Action push(const R& record){
          chunk_.push_back(record);
          Action action = Append;
          
          size_t size = chunk_.size();
          
          if(size >= MAX_CHUNK_SIZE){
            action |= Split;
          }
          else if(size == 1){
            action |= Remap;
          }
          
          return action;
        }
        
        size_t size(){
          return chunk_.size();
        }
        
        Chunk* split(){
          Chunk* c = new Chunk;
          
          auto itr = chunk_.begin();
          itr += chunk_.size()/2;
          
          c->chunk_.insert(c->chunk_.end(), itr, chunk_.end());
          chunk_.erase(itr, chunk_.end());
          
          return c;
        }
        
        V min() const{
          assert(!chunk_.empty());
         
          return chunk_.front().value;
        }
        
        void traverse(TraverseFunc f){
          size_t size = chunk_.size();
          for(size_t i = 0; i < size; ++i){
            f(chunk_[i]);
          }
        }
        
        void dump(){
          for(size_t i = 0; i < chunk_.size(); ++i){
            const R& r = chunk_[i];
            r.dump();
          }
        }
        
        int query(const V& start, QueryFunc_ f){
          size_t index;
          find(start, index);
          
          R* r;
          RowId rowId;
          int s;
          size_t end = chunk_.size() - 1;
          
          for(;;){
            r = &chunk_[index];
            rowId = r->rowId;
            
            s = f(rowId, r->exists() ? nvar(r->value) : none);

            if(s < 0){
              if(index == 0){
                return s;
              }
              --index;
            }
            else if(s > 0){
              if(index == end){
                return s;
              }
              ++index;
            }
            else{
              return 0;
            }
          }
        }
        
      private:
        typedef NVector<R> Chunk_;
        
        Chunk_ chunk_;
      };
      
      Page(IndexBase* index, uint64_t id)
      : index_(index),
      id_(id),
      loaded_(true),
      firstChunk_(0){
        path_ = index->path() + "/" + nvar(id);
      }
      
      ~Page(){
        for(auto& itr : chunkMap_){
          delete itr.second;
        }
      }
      
      bool handleFirst(const R& record){
        if(firstChunk_){
          return false;
        }
        
        firstChunk_ = new Chunk;
        firstChunk_->push(record);
        chunkMap_.insert({record.value, firstChunk_});
        return true;
      }
      
      Action insert(const R& record){
        if(!loaded_){
          load();
        }
       
        if(handleFirst(record)){
          return Remap;
        }

        auto itr = findChunk(record.value);
        
        Chunk* chunk = itr->second;
        
        Action action = chunk->insert(record);
        
        if(action & Remap && chunk == firstChunk_){
          chunkMap_.erase(itr);
          chunkMap_.insert({record.value, chunk});
        }
        
        if(action & Split){
          if(chunkMap_.size() >= MAX_CHUNKS){
            return Split;
          }
          else{
            Chunk* c = chunk->split();
            chunkMap_.insert({c->min(), c});
            return None;
          }
        }

        return None;
      }
      
      Action push(const R& record){
        if(!loaded_){
          load();
        }
        
        if(handleFirst(record)){
          return Remap;
        }

        auto itr = chunkMap_.end();
        --itr;
        Chunk* chunk = itr->second;
        
        Action action = chunk->push(record);

        if(action & Split){
          if(chunkMap_.size() >= MAX_CHUNKS){
            return Split;
          }
          else{
            Chunk* c = chunk->split();
            chunkMap_.insert({c->min(), c});
            return None;
          }
        }
        
        return None;
      }
      
      R* get(const V& value){
        if(!loaded_){
          load();
        }
        
        auto itr = findChunk(value);
        if(itr == chunkMap_.end()){
          return 0;
        }
        
        return itr->second->get(value);
      }
      
      Page* split(uint64_t id){
        Page* p = new Page(index_, id);
        
        typename ChunkMap_::iterator itr;

        for(size_t i = 0; i < MAX_CHUNKS/2; ++i){
          itr = chunkMap_.end();
          --itr;
          
          p->chunkMap_.insert({itr->first, itr->second});
          chunkMap_.erase(itr);
          ++i;
        }
        
        return p;
      }
      
      void load(){
        loaded_ = true;
      }
      
      V min(){
        auto itr = chunkMap_.begin();
        assert(itr != chunkMap_.end());
        
        return itr->second->min();
      }
      
      void traverse(TraverseFunc f){
        for(auto& itr : chunkMap_){
          itr.second->traverse(f);
        }
      }
      
      void dump(){
        for(auto& itr : chunkMap_){
          cout << "@@@ CHUNK LOWER: " << itr.first << endl;
          itr.second->dump();
        }
      }
      
      int query(const V& start, QueryFunc_ f){
        auto itr = findChunk(start);
        
        for(;;){
          int s = itr->second->query(start, f);
          
          if(s > 0){
            ++itr;
            if(itr == chunkMap_.end()){
              return s;
            }
          }
          else if(s < 0){
            if(itr == chunkMap_.begin()){
              return s;
            }
            else{
              --itr;
            }
          }
          else{
            return 0;
          }
        }
      }
      
    private:
      typedef NMap<V, Chunk*> ChunkMap_;
      
      IndexBase* index_;
      uint64_t id_;
      bool loaded_;
      ChunkMap_ chunkMap_;
      Chunk* firstChunk_;
      nstr path_;
      
      typename ChunkMap_::iterator findChunk(const V& v){
        auto itr = chunkMap_.upper_bound(v);
        return itr == chunkMap_.begin() ? itr : --itr;
      }
    };
    
    template<typename R, typename V>
    class Index : public IndexBase{
    public:
      typedef Page<R, V> IndexPage;
      
      typedef function<void(R& r)> TraverseFunc;
      
      Index(uint8_t type)
      : IndexBase(type),
      nextPageId_(0){
        min(min_);
        
        firstPage_ = new IndexPage(this, nextPageId_++);
        pageMap_.insert({min_, firstPage_});
      }
      
      virtual ~Index(){
        for(auto& itr : pageMap_){
          delete itr.second;
        }
      }
      
      void insertRecord(const R& record){
        auto itr = findPage(record.value);
        
        IndexPage* page = itr->second;
        Action action = page->insert(record);
        
        if(action & Split){
          IndexPage* p = page->split(nextPageId_++);
          pageMap_.insert({p->min(), p});
        }
        else if(action & Remap && page != firstPage_){
          pageMap_.erase(itr);
          pageMap_.insert({page->min(), page});
        }
      }
      
      void pushRecord(const R& record){
        auto itr = pageMap_.end();
        --itr;
        IndexPage* page = itr->second;
        
        Action action = page->push(record);
        
        if(action & Split){
          IndexPage* p = page->split(nextPageId_++);
          pageMap_.insert({p->min(), p});
        }
        else if(action & Remap && page != firstPage_){
          pageMap_.erase(itr);
          pageMap_.insert({page->min(), page});
        }
      }
      
      R* getRecord(const V& value){
        return findPage(value)->second->get(value);
      }
      
      void traverse(TraverseFunc f){
        for(auto& itr : pageMap_){
          itr.second->traverse(f);
        }
      }

      void compact(Index& ni, const RowSet& rs){
        traverse([&](R& r){
          if(rs.hasKey(r.rowId)){
            return;
          }
          
          ni.pushRecord(r);
        });
      }
      
      void compact(Index& ni, const RowSet& rs, const UpdateMap& um){
        traverse([&](R& r){
          if(rs.hasKey(r.rowId)){
            return;
          }
          
          RowId rowId = r.value;
          if(rs.hasKey(rowId)){
            auto itr = um.find(rowId);
            if(itr != um.end()){
              r.value = itr->second;
            }
            else{
              return;
            }
          }
          
          ni.pushRecord(r);
        });
      }

      void query(const V& start, QueryFunc_ f){
        auto itr = findPage(start);
        
        for(;;){
          int s = itr->second->query(start, f);
          
          if(s > 0){
            ++itr;

            if(itr == pageMap_.end()){
              break;
            }
          }
          else if(s < 0){
            if(itr == pageMap_.begin()){
              break;
            }

            --itr;
          }
          else{
            break;
          }
        }
      }
      
      void dump(){
        for(auto& itr : pageMap_){
          cout << "@@@@@@@ PAGE: " << itr.first << endl;
          itr.second->dump();
        }
      }
      
    private:
      typedef NMap<V, IndexPage*> PageMap_;
      
      V min_;
      uint64_t nextPageId_;
      PageMap_ pageMap_;
      IndexPage* firstPage_;
      
      typename PageMap_::iterator findPage(const V& v){
        auto itr = pageMap_.upper_bound(v);
        return itr == pageMap_.begin() ? itr : --itr;
      }
    };

    class DataRecord{
    public:
      
      void set(RowId rowId, uint32_t dataId, uint32_t offset){
        remap = 0;
        value = rowId;
        rowId = (uint64_t(dataId) << 32) | uint64_t(offset);
      }
      
      void erase(){
        remap = 1;
        rowId = 0;
      }
      
      void update(uint64_t newRowId){
        remap = 1;
        rowId = newRowId;
      }
      
      bool exists(){
        return !remap;
      }
      
      RowId value;
      uint64_t rowId : 63;
      uint64_t remap : 1;
      
      uint32_t offset() const{
        return rowId & 0xffffffff;
      }
      
      uint32_t dataId() const{
        return rowId >> 32;
      }
      
      void dump() const{
        if(remap){
          if(rowId == 0){
            cout << "deleted: " << value << endl;
          }
          else{
            cout << "updated: " << rowId << endl;
          }
        }
        else{
          
        }
        cout << "rowId: " << value << "; dataId: " << dataId() <<
        "; offset: " << offset() << endl;
      }
    };
    
    class DataIndex : public Index<DataRecord, RowId>{
    public:
      
      DataIndex(NDatabase_* d)
      : Index(DataIndexType),
      d_(d){
        
      }
      
      void insert(uint32_t dataId, uint32_t offset, RowId rowId){
        record_.set(rowId, dataId, offset);
        pushRecord(record_);
      }

      bool get(RowId rowId, uint32_t& dataId, uint32_t& offset){
        DataRecord* record = getRecord(rowId);
        if(record){
          dataId = record->dataId();
          offset = record->offset();
          return true;
        }
        
        return false;
      }
      
      bool exists(RowId rowId){
        DataRecord* record = getRecord(rowId);
        if(record){
          return record->exists();
        }
        
        return false;
      }
      
      void erase(RowId rowId){
        DataRecord* record = getRecord(rowId);
        
        if(record){
          record->erase();
        }
      }
      
      RowId update(RowId rowId){
        DataRecord* record = getRecord(rowId);
        if(record){
          RowId newRowId = d_->nextRowId();
          record->update(newRowId);
          return newRowId;
        }
        
        return 0;
      }
      
      void mapCompact(DataIndex& ni, RowSet& rs, UpdateMap& um){
        traverse([&](DataRecord& r){
          if(r.remap){
            rs.insert(r.value);
            if(r.rowId != 0){
              um.insert({r.value, RowId(r.rowId)});
            }
          }
          else{
            ni.pushRecord(r);
          }
        });
      }
      
    private:
      DataRecord record_;
      NDatabase_* d_;
    };

    struct Int64Record{
      int64_t value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class Int64Index : public Index<Int64Record, int64_t>{
    public:
      Int64Index()
      : Index(NTable::Int64){
        
      }
      
      void insert(RowId rowId, int64_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      Int64Record record_;
    };
    
    struct UInt64Record{
      uint64_t value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class UInt64Index : public Index<UInt64Record, uint64_t>{
    public:
      UInt64Index()
      : Index(NTable::UInt64){
        
      }
      
      void insert(RowId rowId, uint64_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      UInt64Record record_;
    };
    
    struct RowRecord{
      RowId value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class RowIndex : public Index<RowRecord, RowId>{
    public:
      RowIndex()
      : Index(NTable::Row){
        
      }
      
      void insert(RowId rowId, uint64_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      RowRecord record_;
    };
    
    struct Int32Record{
      int32_t value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class Int32Index : public Index<Int32Record, int32_t>{
    public:
      Int32Index()
      : Index(NTable::Int32){
        
      }
      
      void insert(RowId rowId, int32_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
      RowId get(int32_t value){
        return 0;
      }
      
    private:
      Int32Record record_;
    };
    
    struct UInt32Record{
      uint32_t value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class UInt32Index : public Index<UInt32Record, uint32_t>{
    public:
      UInt32Index()
      : Index(NTable::UInt32){
        
      }
      
      void insert(RowId rowId, uint32_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      UInt32Record record_;
    };

    struct DoubleRecord{
      double value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class DoubleIndex : public Index<DoubleRecord, double>{
    public:
      DoubleIndex()
      : Index(NTable::Double){
        
      }
      
      void insert(RowId rowId, double value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      DoubleRecord record_;
    };
    
    struct FloatRecord{
      float value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class FloatIndex : public Index<FloatRecord, double>{
    public:
      FloatIndex()
      : Index(NTable::Float){
        
      }
      
      void insert(RowId rowId, float value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      FloatRecord record_;
    };
    
    struct HashRecord{
      uint64_t value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class HashIndex : public Index<HashRecord, uint64_t>{
    public:
      HashIndex()
      : Index(NTable::Hash){
        
      }
      
      void insert(RowId rowId, uint64_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      HashRecord record_;
    };
    
    class Data{
    public:
      Data(uint64_t id)
      : data_(0),
      size_(0),
      id_(id){
        
      }
      
      ~Data(){
        free(data_);
      }
      
      size_t size(){
        return size_;
      }
      
      uint64_t id(){
        return id_;
      }
      
      uint32_t insert(RowId rowId, char* buf, uint32_t size){
        uint32_t offset = size_;
        
        if(data_){
          data_ = (char*)realloc(data_, size_ + size + 12);
        }
        else{
          data_ = (char*)malloc(size_ + size + 12);
        }
        memcpy(data_ + size, &rowId, 8);
        size_ += 8;
        
        memcpy(data_ + size_, &size, 4);
        size_ += 4;
        
        memcpy(data_ + size_, buf, size);
        size_ += size;
        
        return offset;
      }
      
      void get(uint32_t offset, nvar& v){
        RowId rowId;
        memcpy(&rowId, data_ + offset, 8);
        offset += 8;
        
        uint32_t size;
        memcpy(&size, data_ + offset, 4);
        offset += 4;
        v.unpack(data_ + offset, size);
      }
      
      void compact(Data* newData, const RowSet& rs){
        RowId rowId;
        uint32_t size;
        uint32_t offset = 0;
        
        while(offset < size_){
          memcpy(&rowId, data_ + offset, 8);
          offset += 8;

          memcpy(&size, data_ + offset, 4);
          offset += 4;
          
          if(!rs.hasKey(rowId)){
            newData->insert(rowId, data_ + offset, size);
          }
          offset += size;
        }
      }
      
    private:
      uint32_t size_;
      char* data_;
      uint64_t id_;
    };
    
    NTable_(NTable* o, NDatabase_* d)
    : o_(o),
    d_(d),
    nextDataId_(0),
    lastData_(0),
    dataIndex_(new DataIndex(d_)){
      
    }
    
    void setName(const nstr& name){
      name_ = name;
      path_ = d_->path() + "/" + name;
    }
    
    const nstr& name(){
      return name_;
    }
    
    void addIndex(const nstr& indexName, uint8_t indexType){
      auto itr = indexMap_.find(indexName);
      
      if(itr != indexMap_.end()){
        NERROR("index exists: " + indexName);
      }

      IndexBase* index;
      
      switch(indexType){
        case NTable::Int32:
          index = new Int32Index;
          break;
        case NTable::UInt32:
          index = new UInt32Index;
          break;
        case NTable::Int64:
          index = new Int64Index;
          break;
        case NTable::UInt64:
          index = new UInt64Index;
          break;
        case NTable::Float:
          index = new FloatIndex;
          break;
        case NTable::Double:
          index = new DoubleIndex;
          break;
        case NTable::Row:
          index = new RowIndex;
          break;
        case NTable::Hash:
          index = new HashIndex;
          break;
        default:
          NERROR("invalid index type");
      }
      
      index->setPath(path_ + "/" + indexName);
      
      indexMap_.insert({indexName, index});
    }
    
    RowId insert(nvar& row){
      RowId rowId = d_->nextRowId();
      insert_(rowId, row);
      return rowId;
    }
    
    void insert_(RowId rowId, nvar& row){
      const nmap& m = row;
      for(auto& itr : m){
        const nvar& k = itr.first;
        const nvar& v = itr.second;
        
        if(k.isSymbol()){
          auto iitr = indexMap_.find(k);
          if(iitr != indexMap_.end()){
            IndexBase* index = iitr->second;
            
            switch(index->type()){
              case NTable::Int32:{
                Int32Index* i = static_cast<Int32Index*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::UInt32:{
                UInt32Index* i = static_cast<UInt32Index*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::Int64:{
                Int64Index* i = static_cast<Int64Index*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::UInt64:{
                UInt64Index* i = static_cast<UInt64Index*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::Float:{
                FloatIndex* i = static_cast<FloatIndex*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::Double:{
                DoubleIndex* i = static_cast<DoubleIndex*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::Row:{
                RowIndex* i = static_cast<RowIndex*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::Hash:{
                HashIndex* i = static_cast<HashIndex*>(index);
                i->insert(rowId, v.hash());
                break;
              }
              default:
                assert(false);
            }
          }
        }
      }
      
      row("id") = rowId;
      
      uint32_t size;
      char* buf = row.pack(size);
      
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
      
      uint32_t offset = data->insert(rowId, buf, size);
      free(buf);
      
      dataIndex_->insert(data->id(), offset, rowId);
      lastData_ = data;
    }
    
    void update(nvar& row){
      RowId rowId = row["id"];
      RowId newRowId = dataIndex_->update(rowId);
      
      if(newRowId == 0){
        NERROR("invalid row id: " + nvar(rowId));
      }
      
      row["id"] = newRowId;
      insert_(newRowId, row);
    }
    
    bool get(RowId rowId, nvar& row){
      uint32_t dataId;
      uint32_t offset;
      
      if(!dataIndex_->get(rowId, dataId, offset)){
        return false;
      }
      
      auto itr = dataMap_.find(dataId);
      assert(itr != dataMap_.end());
      
      Data* data = itr->second;
      data->get(offset, row);
      
      return true;
    }
    
    bool exists(RowId rowId){
      return dataIndex_->exists(rowId);
    }
    
    NTable* outer(){
      return o_;
    }
    
    void erase(RowId rowId){
      dataIndex_->erase(rowId);
    }
    
    void mapCompact(RowSet& rs, UpdateMap& um){
      DataIndex* newDataIndex = new DataIndex(d_);
      dataIndex_->mapCompact(*newDataIndex, rs, um);
      delete dataIndex_;
      dataIndex_ = newDataIndex;
    }

    void compact(const RowSet& rs, const UpdateMap& um){
      IndexMap_ newIndexMap_;
      
      for(auto& itr : indexMap_){
        IndexBase* oldIndex = itr.second;
        IndexBase* newIndex;
        
        switch(oldIndex->type()){
          case NTable::Int32:{
            Int32Index* ni = new Int32Index;
            Int32Index* oi = static_cast<Int32Index*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::UInt32:{
            UInt32Index* ni = new UInt32Index;
            UInt32Index* oi = static_cast<UInt32Index*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::Int64:{
            Int64Index* ni = new Int64Index;
            Int64Index* oi = static_cast<Int64Index*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::UInt64:{
            UInt64Index* ni = new UInt64Index;
            UInt64Index* oi = static_cast<UInt64Index*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::Float:{
            FloatIndex* ni = new FloatIndex;
            FloatIndex* oi = static_cast<FloatIndex*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::Double:{
            DoubleIndex* ni = new DoubleIndex;
            DoubleIndex* oi = static_cast<DoubleIndex*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::Row:{
            RowIndex* ni = new RowIndex;
            RowIndex* oi = static_cast<RowIndex*>(oldIndex);
            oi->compact(*ni, rs, um);
            newIndex = ni;
            break;
          }
          case NTable::Hash:{
            HashIndex* ni = new HashIndex;
            HashIndex* oi = static_cast<HashIndex*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          default:
            NERROR("invalid index type");
        }

        delete oldIndex;
        newIndexMap_.insert({itr.first, newIndex});
      }
      
      indexMap_ = move(newIndexMap_);
      
      Data* newData;
      Data* data;
      
      for(auto& itr : dataMap_){
        newData = new Data(itr.first);
        data = itr.second;
        data->compact(newData, rs);
        itr.second = newData;
        delete data;
      }
    }
    
    void query_(const nstr& indexName,
                const nvar& start,
                QueryFunc_ f){

      IndexBase* index;
      if(indexName == "__data"){
        index = dataIndex_;
      }
      else{
        auto itr = indexMap_.find(indexName);
        if(itr == indexMap_.end()){
          NERROR("invalid index: " + indexName);
        }
        index = itr->second;
      }
      
      switch(index->type()){
        case NTable::Int32:{
          Int32Index* i = static_cast<Int32Index*>(index);
          i->query(start, f);
          break;
        }
        case NTable::UInt32:{
          UInt32Index* i = static_cast<UInt32Index*>(index);
          i->query(start, f);
          break;
        }
        case NTable::Int64:{
          Int64Index* i = static_cast<Int64Index*>(index);
          i->query(start, f);
          break;
        }
        case NTable::UInt64:{
          UInt64Index* i = static_cast<UInt64Index*>(index);
          i->query(start, f);
          break;
        }
        case NTable::Float:{
          FloatIndex* i = static_cast<FloatIndex*>(index);
          i->query(start, f);
          break;
        }
        case NTable::Double:{
          DoubleIndex* i = static_cast<DoubleIndex*>(index);
          i->query(start, f);
          break;
        }
        case NTable::Row:{
          RowIndex* i = static_cast<RowIndex*>(index);
          i->query(start, f);
          break;
        }
        case NTable::Hash:{
          HashIndex* i = static_cast<HashIndex*>(index);
          i->query(start, f);
          break;
        }
        case DataIndexType:{
          DataIndex* i = static_cast<DataIndex*>(index);
          i->query(start, f);
          break;
        }
        default:
          assert(false);
      }
    }
    
    void query(const nstr& indexName,
               const nvar& start,
               NTable::QueryFunc qf){
      
      auto f = [&](RowId rowId, const nvar& v) -> int{
        nvar row;
        bool success = get(rowId, row);
        assert(success);
        return qf(row);
      };
      
      query_(indexName, start, f);
    }
    
    void indexQuery(const nstr& indexName,
                    const nvar& start,
                    const nvar& end,
                    RowSet& rs){
      
      if(start > end){
        NERROR("invalid start/end");
      }
      
      auto f = [&](RowId rowId, const nvar& v) -> int{
        if(v > end){
          return 0;
        }
        
        if(exists(rowId)){
          rs.insert(rowId);
        }
        
        return 1;
      };
      
      query_(indexName, start, f);
    }
    
    void traverseStart(NTable::QueryFunc qf){
      auto f = [&](RowId rowId, const nvar& v) -> int{
        nvar row;
        bool success = get(rowId, row);
        assert(success);
        return qf(row);
      };
      
      query_("__data", 1, f);
    }
    
    void traverseEnd(NTable::QueryFunc qf){
      auto f = [&](RowId rowId, const nvar& v) -> int{
        nvar row;
        bool success = get(rowId, row);
        assert(success);
        return qf(row);
      };

      RowId m;
      max(m);
      query_("__data", m, f);
    }
    
    void join(const nstr& indexName, const RowSet& js, RowSet& rs){
      for(RowId findRowId : js){
        auto f = [&](RowId rowId, const nvar& v) -> int{
          RowId toRowId = v;
          
          if(toRowId != findRowId){
            return 0;
          }
          
          if(exists(rowId)){
            rs.insert(rowId);
          }
          
          return 1;
        };
        
        query_(indexName, findRowId, f);
      }
    }
    
    void get(const RowSet& rs, NTable::QueryFunc qf){
      for(RowId findRowId : rs){
        auto f = [&](RowId rowId, const nvar& v) -> int{
          if(v.some()){
            RowId toRowId = v;
            
            if(toRowId != findRowId){
              return 0;
            }
            
            nvar row;
            bool success = get(rowId, row);
            assert(success);
            qf(row);
          }
          
          return 0;
        };
        
        query_("__data", findRowId, f);
      }
    }
    
    bool getFirst(const nstr& indexName, const nvar& value, nvar& row){
      bool success = false;
      
      auto f = [&](RowId rowId, const nvar& v) -> int{
        if(v != value){
          return 0;
        }
        
        nvar row;
        success = get(rowId, row);
        assert(success);
        return 0;
      };
      
      query_(indexName, value, f);
      
      return success;
    }
    
    void dump(){
      cout << "+++++++++ DUMP TABLE: " << name_ << endl;
      
      for(auto& itr : indexMap_){
        cout << "---- DUMP INDEX: " << itr.first << endl;
        itr.second->dump();
      }
      
      dataIndex_->dump();
    }
    
  private:
    typedef NMap<nstr, IndexBase*> IndexMap_;
    typedef NMap<uint64_t, Data*> DataMap_;
    
    NTable* o_;
    NDatabase_* d_;
    nstr name_;
    IndexMap_ indexMap_;
    uint32_t nextDataId_;
    DataMap_ dataMap_;
    Data* lastData_;
    DataIndex* dataIndex_;
    nstr path_;
  };
  
  NTable* NDatabase_::addTable(const nstr& tableName){
    auto itr = tableMap_.find(tableName);
    if(itr != tableMap_.end()){
      NERROR("table exists: " + tableName);
    }
    
    NTable* table = new NTable(this);
    NTable_* t = table->x_;
    t->setName(tableName);
    
    tableMap_.insert({tableName, t});
    
    return table;
  }
  
  NTable* NDatabase_::getTable(const nstr& tableName){
    auto itr = tableMap_.find(tableName);
    if(itr == tableMap_.end()){
      NERROR("invalid table: " + tableName);
    }
    
    return itr->second->outer();
  }
  
  void NDatabase_::compact(){
    RowSet rs;
    UpdateMap um;
    
    for(auto& itr : tableMap_){
      itr.second->mapCompact(rs, um);
    }
    
    for(auto& itr : tableMap_){
      itr.second->compact(rs, um);
    }
  }
  
} // end namespace neu

NTable::NTable(NDatabase_* d){
  x_ = new NTable_(this, d);
}

void NTable::addIndex(const nstr& indexName, IndexType indexType){
  x_->addIndex(indexName, indexType);
}

uint64_t NTable::insert(nvar& row){
  return x_->insert(row);
}

void NTable::update(nvar& row){
  x_->update(row);
}

bool NTable::get(RowId rowId, nvar& row){
  return x_->get(rowId, row);
}

void NTable::erase(RowId rowId){
  x_->erase(rowId);
}

void NTable::query(const nstr& indexName,
                   const nvar& start,
                   QueryFunc f){
  x_->query(indexName, start, f);
}

void NTable::indexQuery(const nstr& indexName,
                        const nvar& start,
                        const nvar& end,
                        RowSet& rs){
  x_->indexQuery(indexName, start, end, rs);
}

void NTable::traverseStart(QueryFunc f){
  x_->traverseStart(f);
}

void NTable::traverseEnd(QueryFunc f){
  x_->traverseEnd(f);
}

void NTable::join(const nstr& indexName, const RowSet& js, RowSet& rs){
  x_->join(indexName, js, rs);
}

void NTable::get(const RowSet& rs, QueryFunc f){
  x_->get(rs, f);
}

bool NTable::getFirst(const nstr& indexName, const nvar& value, nvar& row){
  return x_->getFirst(indexName, value, row);
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

void NDatabase::compact(){
  x_->compact();
}

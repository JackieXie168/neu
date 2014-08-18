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

#include <atomic>

#include <neu/NEncoder.h>
#include <neu/NHashMap.h>
#include <neu/NSys.h>
#include <neu/NRWMutex.h>
#include <neu/NReadGuard.h>
#include <neu/NWriteGuard.h>
#include <neu/NRegex.h>

using namespace std;
using namespace neu;

namespace{
  
  static const uint8_t DataIndexType = 255;
  
  static const size_t MAX_CHUNK_SIZE = 2048;
  static const size_t MAX_CHUNKS = 512;
  static const size_t MAX_DATA_SIZE = 16777216;
  static const size_t DEFAULT_MEMORY_LIMIT = 1024;
  
  static const size_t SPLIT_CHUNK_SIZE = MAX_CHUNK_SIZE - 2;
  static const size_t OVER_ALLOC = MAX_DATA_SIZE/16;
  
  static const size_t EXTRA_DATA_BUFFER_SIZE = 8192;
  static const size_t MIN_COMPRESS_SIZE = 1024;
  
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
  
  class Pageable{
  public:
    virtual size_t memoryUsage() = 0;
    
    virtual void store() = 0;
    
    virtual bool locked() = 0;
  };
  
  typedef NMap<uint64_t, pair<Pageable*, size_t>> PMap;
  
  nstr oldPath(const nstr& path){
    nstr f = NSys::basename(path);
    return NSys::stripPath(path) + "/old/" + f;
  }
  
} // end namespace

namespace neu{
  
  class NTable_;
  
  class NDatabase_{
  public:
    NDatabase_(NDatabase* o, const nstr& path, bool create);
    
    ~NDatabase_(){
      commit();
    }
    
    NTable* addTable(const nstr& tableName);
    
    NTable* getTable(const nstr& tableName);

    void eraseTable(const nstr& tableName);
    
    RowId nextRowId(){
      return nextRowId_++;
    }
    
    void compact();
    
    const nstr& path(){
      return path_;
    }

    uint64_t read(){
      return tick_++;
    }
    
    uint64_t write(){
      return tick_++;
    }

    void setMemoryLimit(size_t limit){
      size_t l = 1048576*limit;
      
      if(l <= MAX_DATA_SIZE){
        NERROR("memory limit too low");
      }
      
      memoryLimit_ = l;
    }
    
    size_t memoryUsage(PMap& pm);
    
    void checkMemory();
    
    void commit();
    
    void saveMeta();
    
    void rollback();
    
    void safeRemoveAll(const nstr& dirPath){
      nvec files;
      if(!NSys::dirFiles(dirPath, files)){
        cerr << "Error: removing files in: " << dirPath << endl;
        exit(1);
      }
      
      for(const nstr& p : files){
        nstr fullPath = dirPath + "/" + p;
        safeRemove(fullPath);
      }
    }
    
    void safeRemove(const nstr& path){
      if(!path.beginsWith(path_)){
        cerr << "Error: attempted to remove file outside "
        "database: " << path << endl;
        exit(1);
      }
      
      remove(path.c_str());
    }
    
  private:
    typedef NMap<nstr, NTable_*> TableMap_;
    
    NDatabase* o_;
    nstr path_;
    nstr metaPath_;
    TableMap_ tableMap_;
    size_t memoryLimit_;
    atomic<RowId> nextRowId_;
    atomic<uint64_t> tick_;
  }; // end class NDatabase_
  
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
      
      virtual ~IndexBase(){}
      
      uint8_t type(){
        return type_;
      }
      
      void setUnique(bool flag){
        unique_ = flag;
      }
      
      bool unique(){
        return unique_;
      }
      
      void setAutoErase(bool flag){
        autoErase_ = flag;
      }
      
      bool autoErase(){
        return autoErase_;
      }
      
      void setClean(bool flag){
        clean_ = flag;
      }
      
      virtual size_t memoryUsage(PMap& pm) = 0;
      
      virtual void save(bool manual) = 0;
      
      virtual void saveMeta() = 0;
      
      virtual void dump(){}
      
      virtual const nstr& path() const = 0;

      virtual void setPath(const nstr& path) = 0;
      
      virtual void rollback() = 0;

      virtual void clean() = 0;

      virtual void clear() = 0;
      
      virtual void append(IndexBase* index) = 0;
      
    private:
      uint8_t type_;
    protected:
      bool unique_;
      bool autoErase_;
      bool clean_;
    }; // end class IndexBase
        
    template<class R, class V>
    class Page : public Pageable{
    public:
      typedef function<void(R& r)> TraverseFunc;
      
      class Chunk{
      public:
        
        Chunk(bool unique)
        : unique_(unique){}
        
        Chunk(bool unique, R* data, size_t n)
        : unique_(unique),
        chunk_(data, data + n){}
        
        typedef function<void(R& r)> TraverseFunc;
        
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

          Action action = None;
          
          if(length >= SPLIT_CHUNK_SIZE){
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
        
        size_t find(const V& value){
          size_t index = 0;
          
          R* record;
          size_t length = chunk_.size();
          size_t start = 0;
          size_t end = length;
          
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
              return index;
            }
          }

          return index;
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
          Action action = findInsert(record.value, index);

          if(unique_ && chunk_[index].value == record.value){
            NERROR("non-unique value: " + nvar(record.value));
          }
          
          if(action & Append){
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
          
          if(size >= SPLIT_CHUNK_SIZE){
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
          Chunk* c = new Chunk(unique_);
          
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
        
        int query(const V& start, QueryFunc_ f){
          size_t index = find(start);
          
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
        
        R* data(){
          return chunk_.data();
        }
        
        void dump(){
          for(size_t i = 0; i < chunk_.size(); ++i){
            const R& r = chunk_[i];
            r.dump();
          }
        }
        
      private:
        typedef NVector<R> Chunk_;
        
        Chunk_ chunk_;
        bool unique_;
      };
      
      Page(NDatabase_* d,
           IndexBase* index,
           uint64_t id,
           bool create)
      : index_(index),
      d_(d),
      id_(id),
      needsLoad_(!create),
      existed_(!create),
      current_(!create),
      firstChunk_(0),
      tick_(0),
      memoryUsage_(0),
      locked_(false){
        path_ = index_->path() + "/" + nvar(id);
      }
      
      ~Page(){
        for(auto& itr : chunkMap_){
          delete itr.second;
        }
      }
      
      void read(){
        tick_ = d_->read();
        
        if(needsLoad_){
          load();
        }
      }
      
      uint64_t id(){
        return id_;
      }
      
      void write(){
        tick_ = d_->write();
        
        if(needsLoad_){
          load();
        }

        current_ = false;
      }
      
      size_t memoryUsage(){
        return memoryUsage_;
      }
      
      void setParentPath(const nstr& path){
        path_ = index_->path() + "/" + nvar(id_);
      }
      
      void store(){
        save(false);
        
        for(auto& itr : chunkMap_){
          delete itr.second;
        }
        
        chunkMap_.clear();
        
        needsLoad_ = true;
        memoryUsage_ = 0;
      }
      
      void save(bool manual){
        if(current_){
          return;
        }
        
        if(manual){
          existed_ = true;
        }
        else if(existed_){
          nstr op = oldPath(path_);
          
          if(!NSys::exists(op)){
            NSys::rename(path_, op);
            index_->setClean(false);
          }
        }

        FILE* file = fopen(path_.c_str(), "wb");
        
        if(!file){
          NERROR("failed to create page file: " + path_);
        }

        uint32_t numChunks = chunkMap_.size();
        
        uint32_t n = fwrite(&numChunks, 1, 4, file);
        if(n != 4){
          NERROR("failed to write to page file [1]: " + path_);
        }

        uint32_t chunkSize;
        uint32_t dataSize;
        for(auto& itr : chunkMap_){
          Chunk* chunk = itr.second;

          chunkSize = chunk->size();
          n = fwrite(&chunkSize, 1, 4, file);
          if(n != 4){
            NERROR("failed to write to page file [2]: " + path_);
          }
          
          dataSize = sizeof(R)*chunkSize;
          
          n = fwrite(chunk->data(), 1, dataSize, file);
          if(n != dataSize){
            NERROR("failed to write to page file [3]: " + path_);
          }
        }
        
        fclose(file);
        current_ = true;
      }
      
      void load(){
        tick_ = d_->read();
        
        FILE* file = fopen(path_.c_str(), "rb");
        
        uint32_t numChunks;
        uint32_t n = fread(&numChunks, 1, 4, file);
        if(n != 4){
          NERROR("failed to read page file [1]: " + path_);
        }
        
        uint32_t chunkSize;
        uint32_t dataSize;
        R buf[MAX_CHUNK_SIZE];
        
        firstChunk_ = 0;
        memoryUsage_ = 0;
        
        for(size_t i = 0; i < numChunks; ++i){
          n = fread(&chunkSize, 1, 4, file);
          if(n != 4){
            NERROR("failed to read page file [2]: " + path_);
          }
        
          dataSize = sizeof(R)*chunkSize;
          memoryUsage_ += dataSize;
          
          n = fread(buf, 1, dataSize, file);
          if(n != dataSize){
            NERROR("failed to read page file [3]: " + path_);
          }
          
          Chunk* chunk = new Chunk(index_->unique(), buf, chunkSize);
          if(!firstChunk_){
            firstChunk_ = chunk;
          }
          
          chunkMap_.insert({chunk->min(), chunk});
        }
        fclose(file);
        
        needsLoad_ = false;
        
        locked_ = true;
        d_->checkMemory();
        locked_ = false;
      }
      
      bool handleFirst(const R& record){
        if(firstChunk_){
          return false;
        }
        
        firstChunk_ = new Chunk(index_->unique());
        firstChunk_->push(record);
        chunkMap_.insert({record.value, firstChunk_});
        memoryUsage_ += sizeof(R);
        return true;
      }
      
      Action insert(const R& record){
        write();
        
        if(handleFirst(record)){
          return Remap;
        }

        auto itr = findChunk(record.value);
        
        Chunk* chunk = itr->second;
        
        Action action = chunk->insert(record);
        memoryUsage_ += sizeof(R);
        
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
        write();
        
        if(handleFirst(record)){
          return Remap;
        }

        auto itr = chunkMap_.end();
        --itr;
        Chunk* chunk = itr->second;
        
        Action action = chunk->push(record);
        memoryUsage_ = sizeof(R);
        
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
        read();
        
        auto itr = findChunk(value);
        if(itr == chunkMap_.end()){
          return 0;
        }
        
        return itr->second->get(value);
      }
      
      Page* split(uint64_t id){
        write();
        
        Page* p = new Page(d_, index_, id, true);
        p->write();
        
        typename ChunkMap_::iterator itr;

        size_t count = MAX_CHUNKS/2;
        
        for(size_t i = 0; i < count; ++i){
          itr = chunkMap_.end();
          --itr;
          
          p->chunkMap_.insert({itr->first, itr->second});
          chunkMap_.erase(itr);
        }
        
        size_t half = memoryUsage_/2;
        
        p->memoryUsage_ = half;
        memoryUsage_ = half;
        
        // ndm - needed?
        //locked_ = true;
        //d_->checkMemory();
        //locked_ = false;
        
        return p;
      }
      
      V min(){
        auto itr = chunkMap_.begin();

        if(itr == chunkMap_.end()){
          V m;
          ::min(m);
          return m;
        }
        
        return itr->second->min();
      }
      
      void traverse(TraverseFunc f){
        read();
        
        locked_ = true;
        for(auto& itr : chunkMap_){
          itr.second->traverse(f);
        }
        locked_ = false;
      }
      
      int query(const V& start, QueryFunc_ f){
        read();
        locked_ = true;
        
        auto itr = findChunk(start);
        assert(itr != chunkMap_.end());
        
        for(;;){
          int s = itr->second->query(start, f);
          
          if(s > 0){
            ++itr;
            if(itr == chunkMap_.end()){
              locked_ = false;
              return s;
            }
          }
          else if(s < 0){
            if(itr == chunkMap_.begin()){
              locked_ = false;
              return s;
            }
            else{
              --itr;
            }
          }
          else{
            locked_ = false;
            return 0;
          }
        }
        
        locked_ = false;
        return 0;
      }
      
      uint64_t tick(){
        return tick_;
      }
      
      void dump(){
        read();
        
        for(auto& itr : chunkMap_){
          cout << "@@@ CHUNK LOWER: " << itr.first << endl;
          itr.second->dump();
        }
      }
      
      bool locked(){
        return locked_;
      }
      
    private:
      typedef NMap<V, Chunk*> ChunkMap_;
      
      IndexBase* index_;
      NDatabase_* d_;
      nstr path_;
      uint64_t id_;
      ChunkMap_ chunkMap_;
      Chunk* firstChunk_;
      uint64_t tick_;
      size_t memoryUsage_;
      bool needsLoad_ : 1;
      bool existed_ : 1;
      bool current_ : 1;
      bool locked_ : 1;
      
      typename ChunkMap_::iterator findChunk(const V& v){
        auto itr = chunkMap_.upper_bound(v);
        return itr == chunkMap_.begin() ? itr : --itr;
      }
    }; // end class Page
    
    template<typename R, typename V>
    class Index : public IndexBase{
    public:
      typedef Page<R, V> IndexPage;
      
      typedef function<void(R& r)> TraverseFunc;
      
      Index(NDatabase_* d, uint8_t type, const nstr& path, bool create)
      : IndexBase(type),
      d_(d),
      path_(path),
      firstPage_(0),
      clean_(true){
        min(min_);
        
        metaPath_ = path_ + "/meta.nvar";
        
        if(create){
          nextPageId_ = 0;
          current_ = false;
          metaCurrent_ = false;
          
          firstPage_ = new IndexPage(d_, this, nextPageId_++, true);
          pageMap_.insert({min_, firstPage_});
          
          if(NSys::exists(path_)){
            NERROR("index path exists: " + path_);
          }
          
          if(!NSys::makeDir(path_)){
            NERROR("failed to make directory: " + path_);
          }
          
          metaPath_ = path_ + "/meta.nvar";
          if(NSys::exists(metaPath_)){
            NERROR("index meta path exists: " + metaPath_);
          }
          
          nstr oldPath = path_ + "/old";
          if(NSys::exists(oldPath)){
            NERROR("index old path exists: " + oldPath);
          }
          
          if(!NSys::makeDir(oldPath)){
            NERROR("failed to make directory: " + oldPath);
          }
        }
        else{
          firstPage_ = 0;
          
          current_ = true;
          metaCurrent_ = true;
          
          nvar m;
          m.open(metaPath_);
          
          nget(m, nextPageId_);
          nget(m, unique_);
          nget(m, autoErase_);
          
          const nhmap& pm = m["pageMap"];
          
          for(auto& itr : pm){
            uint64_t pageId = itr.first;
            V min = itr.second;
            IndexPage* page = new IndexPage(d_, this, pageId, false);
            pageMap_.insert({min, page});
          }
        }
      }
      
      virtual ~Index(){
        for(auto& itr : pageMap_){
          delete itr.second;
        }
      }
      
      void clear(){
        d_->safeRemoveAll(path_);
        for(auto& itr : pageMap_){
          delete itr.second;
        }
        
        pageMap_.clear();
        
        firstPage_ = new IndexPage(d_, this, nextPageId_++, true);
        pageMap_.insert({min_, firstPage_});
        
        metaCurrent_ = false;
        saveMeta();
        current_ = true;
        clean_ = true;
      }
      
      void saveMeta(){
        if(metaCurrent_){
          return;
        }
        
        nvar m;
        nput(m, nextPageId_);
        nput(m, unique_);
        nput(m, autoErase_);
        
        nhmap& pm = m("pageMap") = nhmap();
        
        for(auto& itr : pageMap_){
          IndexPage* page = itr.second;
          pm.insert({page->id(), page->min()});
        }
        
        m.save(metaPath_);
        
        metaCurrent_ = true;
      }
      
      size_t memoryUsage(PMap& pm){
        size_t m = 0;
        for(auto& itr : pageMap_){
          IndexPage* p = itr.second;
          
          size_t mi = p->memoryUsage();
          
          if(mi > 0){
            pm.insert({p->tick(), {p, mi}});
            m += mi;
          }
        }
        
        return m;
      }
      
      virtual void append(IndexBase* index){
        Index* i = static_cast<Index*>(index);
        
        TraverseFunc f =
        [&](R& r){
          insertRecord(r);
        };
        
        for(auto& itr : i->pageMap_){
          itr.second->traverse(f);
        }
      }
      
      void rollback(){
        nstr oldPath = path_ + "/old";
        
        nvec oldFiles;
        if(!NSys::dirFiles(oldPath, oldFiles)){
          NERROR("index failed to rollback[1]");
        }
        
        for(const nstr& p : oldFiles){
          nstr fromPath = oldPath + "/" + p;
          nstr toPath = path_ + "/" + p;
          NSys::rename(fromPath, toPath);
        }
        
        nvar m;
        m.open(metaPath_);
        
        const nhmap& pm = m["pageMap"];
        
        nvec newFiles;
        if(!NSys::dirFiles(path_, newFiles)){
          NERROR("index failed to rollback[2]");
        }
        
        NRegex r("\\d+");
        
        for(const nstr& p : oldFiles){
          if(r.match(p)){
            size_t pageId = nvar(p).toLong();
            
            if(!pm.hasKey(pageId)){
              nstr fullPath = path_ + "/" + p;
              d_->safeRemove(fullPath.c_str());
            }
          }
        }
        
        metaCurrent_ = true;
        clean_ = true;
        current_ = true;
      }
      
      void clean(){
        if(clean_){
          return;
        }
        
        nstr oldPath = path_ + "/old";
        
        nvec oldFiles;
        if(!NSys::dirFiles(oldPath, oldFiles)){
          NERROR("index failed to clean[1]");
        }
        
        for(const nstr& p : oldFiles){
          nstr fullPath = oldPath + "/" + p;
          d_->safeRemove(fullPath.c_str());
        }
        
        clean_ = true;
      }
      
      const nstr& path() const{
        return path_;
      }
      
      void setPath(const nstr& path){
        path_ = path;
        metaPath_ = path_ + "/meta.nvar";
        
        for(auto& itr : pageMap_){
          itr.second->setParentPath(path_);
        }
      }
      
      void save(bool manual){
        if(current_){
          return;
        }
        
        for(auto& itr : pageMap_){
          itr.second->save(manual);
        }
        
        current_ = true;
      }
      
      void insertRecord(const R& record){
        current_ = false;
        
        auto itr = findPage(record.value);
        
        IndexPage* page = itr->second;
        Action action = page->insert(record);
        
        if(action & Split){
          IndexPage* p = page->split(nextPageId_++);
          pageMap_.insert({p->min(), p});
          metaCurrent_ = false;
        }
        else if(action & Remap && page != firstPage_){
          pageMap_.erase(itr);
          pageMap_.insert({page->min(), page});
          metaCurrent_ = false;
        }
      }
      
      void pushRecord(const R& record){
        current_ = false;
        
        auto itr = pageMap_.end();
        --itr;
        IndexPage* page = itr->second;
        
        Action action = page->push(record);
        
        if(action & Split){
          IndexPage* p = page->split(nextPageId_++);
          pageMap_.insert({p->min(), p});
          metaCurrent_ = false;
        }
        else if(action & Remap && page != firstPage_){
          pageMap_.erase(itr);
          pageMap_.insert({page->min(), page});
          metaCurrent_ = false;
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
              if(autoErase_){
                return;
              }
              
              r.value = 0;
            }
          }
          
          ni.pushRecord(r);
        });
      }

      void query(const V& start, QueryFunc_ f){
        auto itr = findPage(start);
        assert(itr != pageMap_.end());
        
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

      size_t memoryUsage(){
        size_t m = 0;
        for(auto& itr : pageMap_){
          m += itr.second->memoryUsage();
        }
        
        return m;
      }
      
      void dump(){
        cout << "@@@@ INDEX: " << path_ << endl;
        
        for(auto& itr : pageMap_){
          cout << "@@@@@@@ PAGE: " << itr.first << endl;
          itr.second->dump();
        }
      }
      
    private:
      typedef NMap<V, IndexPage*> PageMap_;
      
      NDatabase_* d_;
      nstr path_;
      nstr metaPath_;
      PageMap_ pageMap_;
      IndexPage* firstPage_;
      V min_;
      uint64_t nextPageId_;
      bool current_;
      bool metaCurrent_;
      bool clean_;
      
      typename PageMap_::iterator findPage(const V& v){
        auto itr = pageMap_.upper_bound(v);
        return itr == pageMap_.begin() ? itr : --itr;
      }
    }; // end class Index

    class DataRecord{
    public:
      RowId value;
      uint64_t rowId : 63;
      uint64_t remap : 1;
      
      void set(RowId id, uint32_t dataId, uint32_t offset){
        remap = 0;
        value = id;
        rowId = uint64_t(dataId) << 32 | uint64_t(offset);
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
          cout << "rowId: " << value << "; dataId: " << dataId() <<
          "; offset: " << offset() << endl;
        }
      }
    };
    
    class DataIndex : public Index<DataRecord, RowId>{
    public:
      
      DataIndex(NDatabase_* d, const nstr& path, bool create)
      : Index(d, DataIndexType, path, create),
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
      
      void mapCompact(NTable_* t,
                      DataIndex* newIndex,
                      RowSet& rs,
                      UpdateMap& um){
        traverse([&](DataRecord& r){
          if(r.remap){
            rs.insert(r.value);
            if(r.rowId != 0){
              um.insert({r.value, RowId(r.rowId)});
            }
          }
          else{
            t->insertNewData_(newIndex, r.dataId(), r.value, r.offset());
          }
        });
      }
      
    private:
      NDatabase_* d_;
      DataRecord record_;
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
      Int64Index(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::Int64, path, create){
        
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
      UInt64Index(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::UInt64, path, create){
        
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
      RowIndex(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::Row, path, create){
        
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
      Int32Index(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::Int32, path, create){
        
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
      UInt32Index(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::UInt32, path, create){
        
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
      DoubleIndex(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::Double, path, create){
        
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
      FloatIndex(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::Float, path, create){
        
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
      HashIndex(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::Hash, path, create){
        
      }
      
      void insert(RowId rowId, uint64_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      HashRecord record_;
    };
    
    class Data : public Pageable{
    public:
      Data(NTable_* table, uint64_t id, size_t size=0)
      : table_(table),
      d_(table->database()),
      data_(0),
      size_(size),
      allocSize_(0),
      existed_(size > 0),
      current_(size > 0),
      id_(id),
      tick_(0),
      locked_(false){
        path_ = table_->path() + "/__data/" + nvar(id);
      }
      
      ~Data(){
        if(data_){
          free(data_);
        }
      }
      
      void erase(){
        d_->safeRemove(path_);
      }
      
      size_t size(){
        return size_;
      }
      
      size_t memoryUsage(){
        return data_ ? size_ : 0;
      }
      
      uint64_t id(){
        return id_;
      }

      uint64_t tick(){
        return tick_;
      }
      
      void read(){
        tick_ = d_->read();
        load();
      }
      
      void write(){
        tick_ = d_->write();
        load();
        current_ = false;
      }
      
      void save(bool manual){
        if(current_){
          return;
        }
        
        if(manual){
          existed_ = true;
        }
        else if(existed_){
          nstr op = oldPath(path_);
          
          if(!NSys::exists(op)){
            NSys::rename(path_, op);
            table_->setDataClean(false);
          }
        }
        
        FILE* file = fopen(path_.c_str(), "wb");
        
        if(!file){
          NERROR("failed to create data file: " + path_);
        }
        
        uint32_t n = fwrite(data_, 1, size_, file);
        if(n != size_){
          NERROR("failed to write data file: " + path_);
        }
        
        fclose(file);
        current_ = true;
      }

      void store(){
        save(false);
        free(data_);
        data_ = 0;
      }
      
      void load(){
        if(data_ || size_ == 0){
          return;
        }
        
        tick_ = d_->read();
        
        FILE* file = fopen(path_.c_str(), "rb");
        
        if(!file){
          NERROR("failed to open data file: " + path_);
        }
        
        allocSize_ = size_ + OVER_ALLOC;
        data_ = (char*)malloc(allocSize_);
        
        uint32_t n = fread(data_, 1, size_, file);
        
        if(n != size_){
          NERROR("failed to read data file : " + path_);
        }
        
        fclose(file);
        
        locked_ = true;
        d_->checkMemory();
        locked_ = false;
      }

      uint32_t insert(RowId rowId, char* buf, uint32_t size){
        write();
        
        uint32_t offset = size_;

        if(data_){
          size_t reqSize = size_ + size + 12;
          if(reqSize > allocSize_){
            allocSize_ += size + 12 + OVER_ALLOC;
            data_ = (char*)realloc(data_, allocSize_);
          }
        }
        else{
          allocSize_ = size + 12 + OVER_ALLOC;
          data_ = (char*)malloc(allocSize_);
        }

        memcpy(data_ + size_, &rowId, 8);
        size_ += 8;
        
        memcpy(data_ + size_, &size, 4);
        size_ += 4;
        
        memcpy(data_ + size_, buf, size);
        size_ += size;
        
        return offset;
      }
      
      void get(uint32_t offset, nvar& v){
        read();
        
        RowId rowId;
        memcpy(&rowId, data_ + offset, 8);
        offset += 8;
        
        uint32_t size;
        memcpy(&size, data_ + offset, 4);
        offset += 4;
        
        v.unpack(data_ + offset, size);
      }
      
      char* getRaw(uint32_t offset, uint32_t& size){
        read();
        
        RowId rowId;
        memcpy(&rowId, data_ + offset, 8);
        offset += 8;
        
        memcpy(&size, data_ + offset, 4);
        offset += 4;

        return data_ + offset;
      }
      
      uint32_t compact(Data* newData, const RowSet& rs){
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
        
        return offset;
      }
      
      void dump(){
        cout << "###### DUMP DATA" << endl;
        
        read();
        locked_ = true;
        
        size_t offset = 0;
        while(offset < size_){
          RowId rowId;
          memcpy(&rowId, data_ + offset, 8);
          offset += 8;
          
          uint32_t size;
          memcpy(&size, data_ + offset, 4);
          offset += 4;
          
          cout << "### ";

          nvar v;
          v.unpack(data_ + offset, size);

          cout << v << endl;
          
          offset += size;
        }
        
        locked_ = false;
      }
      
      void appendToTable(NTable_* table){
        read();
        
        locked_ = true;
        size_t offset = 0;

        while(offset < size_){
          RowId rowId;
          memcpy(&rowId, data_ + offset, 8);
          offset += 8;
          
          uint32_t size;
          memcpy(&size, data_ + offset, 4);
          offset += 4;
          
          table->insertData_(rowId, size, data_ + offset);
          
          offset += size;
        }
        
        locked_ = false;
      }
      
      bool locked(){
        return locked_;
      }
      
    private:
      NTable_* table_;
      NDatabase_* d_;
      nstr path_;
      uint64_t id_;
      char* data_;
      uint32_t size_;
      uint32_t allocSize_;
      size_t tick_;
      bool existed_ : 1;
      bool current_ : 1;
      bool locked_ : 1;
    }; // end class Data
    
    NTable_(NTable* o, NDatabase_* d, const nstr& path, bool create)
    : o_(o),
    d_(d),
    path_(path),
    lastData_(0),
    newLastData_(0),
    dataClean_(true),
    clean_(true){
      
      new_ = path_.endsWith(".new") ? 0 :
      new NTable_(o_, d_, path_ + ".new", create);
      
      if(create){
        current_ = false;
        nextDataId_ = 0;
        
        if(NSys::exists(path_)){
          NERROR("table path exists: " + path_);
        }
        
        if(!NSys::makeDir(path_)){
          NERROR("failed to make directory: " + path_);
        }
        
        metaPath_ = path_ + "/meta.nvar";
        
        if(NSys::exists(metaPath_)){
          NERROR("table meta path exists: " + metaPath_);
        }
        
        dataPath_ = path_ + "/__data";
        
        if(NSys::exists(dataPath_)){
          NERROR("table data path exists: " + dataPath_);
        }
        
        if(!NSys::makeDir(dataPath_)){
          NERROR("failed to make directory: " + dataPath_);
        }
        
        nstr op = dataPath_ + "/old";
        if(NSys::exists(op)){
          NERROR("table old data path exists: " + op);
        }
        
        if(!NSys::makeDir(op)){
          NERROR("failed to make directory: " + op);
        }
        
        dataMetaPath_ = dataPath_ + "/meta.nvar";
        
        if(NSys::exists(dataMetaPath_)){
          NERROR("table data meta path exists: " + dataMetaPath_);
        }

        dataIndex_ = new DataIndex(d_, path_ + "/__data.index", true);
        
        saveMeta();
      }
      else{
        current_ = true;
        
        metaPath_ = path_ + "/meta.nvar";
        
        if(!NSys::exists(metaPath_)){
          NERROR("table meta path not found: " + metaPath_);
        }
        
        dataPath_ = path_ + "/__data";
        
        if(!NSys::exists(dataPath_)){
          NERROR("table data path not found: " + metaPath_);
        }
        
        dataMetaPath_ = dataPath_ + "/meta.nvar";
        if(!NSys::exists(dataMetaPath_)){
          NERROR("table data meta path not found: " + dataMetaPath_);
        }
        
        dataIndex_ = new DataIndex(d_, path_ + "/__data.index", false);
        
        nvar m;
        m.open(metaPath_);
        nget(m, nextDataId_);
        
        m = undef;
        m.open(dataMetaPath_);
        const nhmap& dm = m;
        
        for(auto& itr : dm){
          uint64_t dataId = itr.first;
          size_t size = itr.second;
          
          dataMap_.insert({dataId, new Data(this, dataId, size)});
        }
        
        nvec files;
        if(!NSys::dirFiles(path_, files)){
          NERROR("failed to read table[1]: " + path_);
        }
        
        NRegex r("(.+?)\\.(\\d+)\\.index");
        
        for(const nstr& p : files){
          nvec m;
          if(r.match(p, m)){
            nstr indexName = NSys::fileName(m[1]);
            nstr fullPath = path_ + "/" + p;
            uint8_t type = m[2].toLong();
            
            IndexBase* index;
            
            switch(type){
              case NTable::Int32:
                index = new Int32Index(d_, fullPath, false);
                break;
              case NTable::UInt32:
                index = new UInt32Index(d_, fullPath, false);
                break;
              case NTable::Int64:
                index = new Int64Index(d_, fullPath, false);
                break;
              case NTable::UInt64:
                index = new UInt64Index(d_, fullPath, false);
                break;
              case NTable::Float:
                index = new FloatIndex(d_, fullPath, false);
                break;
              case NTable::Double:
                index = new DoubleIndex(d_, fullPath, false);
                break;
              case NTable::Row:
                index = new RowIndex(d_, fullPath, false);
                break;
              case NTable::Hash:
                index = new HashIndex(d_, fullPath, false);
                break;
              default:
                NERROR("invalid index type: " + fullPath);
            }
            
            indexMap_.insert({indexName, index});
          }
        }
      }
    }
    
    void clear(){
      dataIndex_->clear();
      
      for(auto& itr : indexMap_){
        itr.second->clear();
      }

      for(auto& itr : dataMap_){
        delete itr.second;
      }

      dataMap_.clear();

      d_->safeRemoveAll(path_ + "/__data");
      
      lastData_ = 0;
      memoryUsage_ = 0;

      current_ = false;
      saveMeta();
      dataClean_ = true;
      clean_ = true;
    }
    
    void erase(){
      if(new_){
        new_->erase();
      }
      
      d_->safeRemove(path_);
    }
    
    void setDataClean(bool flag){
      dataClean_ = flag;
    }
    
    void write(){
      current_ = false;
      clean_ = false;
    }
    
    const nstr& path(){
      return path_;
    }
    
    void readLock_(){
      mutex_.readLock();
    }
    
    void writeLock_(){
      mutex_.writeLock();
    }
    
    void unlock_(){
      mutex_.unlock();
    }
    
    void rollback(){
      assert(new_);
      new_->clear();
      
      nstr oldPath = dataPath_ + "/old";
      
      nvec oldFiles;
      if(!NSys::dirFiles(oldPath, oldFiles)){
        NERROR("table failed to rollback[1]");
      }
      
      for(const nstr& p : oldFiles){
        nstr fromPath = oldPath + "/" + p;
        nstr toPath = dataPath_ + "/" + p;
        NSys::rename(fromPath, toPath);
      }
      
      nvar m;
      m.open(dataMetaPath_);
      const nhmap& dm = m;
      
      nvec newFiles;
      if(!NSys::dirFiles(dataPath_, newFiles)){
        NERROR("table failed to rollback[2]");
      }
      
      NRegex r("\\d+");
      
      for(const nstr& p : oldFiles){
        if(r.match(p)){
          size_t dataId = nvar(p).toLong();
          
          if(!dm.hasKey(dataId)){
            nstr fullPath = dataPath_ + "/" + p;
            d_->safeRemove(fullPath);
          }
        }
      }
      
      for(auto& itr : indexMap_){
        itr.second->rollback();
      }
      
      dataClean_ = true;
      clean_ = true;
    }
    
    void clean(){
      if(clean_){
        return;
      }
      
      for(auto& itr : indexMap_){
        itr.second->clean();
      }
      
      if(dataClean_){
        return;
      }
      
      dataIndex_->clean();
      
      nstr oldPath = dataPath_ + "/old";
      
      nvec oldFiles;
      if(!NSys::dirFiles(oldPath, oldFiles)){
        NERROR("index failed to clean[1]");
      }
      
      for(const nstr& p : oldFiles){
        nstr fullPath = oldPath + "/" + p;
        d_->safeRemove(fullPath);
      }
      
      dataClean_ = true;
      clean_ = true;
    }
    
    void addIndex(const nstr& indexName,
                  uint8_t indexType,
                  bool unique,
                  bool autoErase){
      if(new_){
        new_->addIndex(indexName, indexType, unique, autoErase);
      }
      
      write();
      
      auto itr = indexMap_.find(indexName);
      
      if(itr != indexMap_.end()){
        NERROR("index exists: " + indexName);
      }

      IndexBase* index;
      
      nstr path = path_ + "/" + indexName + "." + nvar(indexType) + ".index";
      
      switch(indexType){
        case NTable::Int32:
          index = new Int32Index(d_, path, true);
          break;
        case NTable::UInt32:
          index = new UInt32Index(d_, path, true);
          break;
        case NTable::Int64:
          index = new Int64Index(d_, path, true);
          break;
        case NTable::UInt64:
          index = new UInt64Index(d_, path, true);
          break;
        case NTable::Float:
          index = new FloatIndex(d_, path, true);
          break;
        case NTable::Double:
          index = new DoubleIndex(d_, path, true);
          break;
        case NTable::Row:
          index = new RowIndex(d_, path, true);
          break;
        case NTable::Hash:
          index = new HashIndex(d_, path, true);
          break;
        default:
          NERROR("invalid index type");
      }
      
      index->setUnique(unique);
      index->setAutoErase(autoErase);
      
      indexMap_.insert({indexName, index});
    }
    
    RowId insert(nvar& row){
      assert(new_);
      write();
      
      RowId rowId = d_->nextRowId();
      insert_(rowId, row);
      return rowId;
    }
    
    void insert_(RowId rowId, nvar& row){
      if(new_){
        new_->insert_(rowId, row);
        return;
      }
      
      write();
      
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
      char* buf = row.pack(size, MIN_COMPRESS_SIZE);
      insertData_(rowId, size, buf);
      free(buf);
    }
    
    void insertData_(RowId rowId, uint32_t size, char* buf){
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
          data = new Data(this, id);
          dataMap_.insert({id, data});
        }
      }
      
      uint32_t offset = data->insert(rowId, buf, size);
      dataIndex_->insert(data->id(), offset, rowId);
      lastData_ = data;
    }
    
    void insertNewData_(DataIndex* dataIndex,
                        uint32_t dataId,
                        RowId rowId,
                        uint32_t offset){

      auto itr = dataMap_.find(dataId);
      assert(itr != dataMap_.end());
      Data* oldData = itr->second;

      uint32_t size;
      char* buf = oldData->getRaw(offset, size);
      
      Data* data;
      
      if(newLastData_ && newLastData_->size() + size <= MAX_DATA_SIZE){
        data = newLastData_;
      }
      else{
        data = 0;
        
        for(auto& itr : newDataMap_){
          Data* d = itr.second;
          if(d->size() + size <= MAX_DATA_SIZE){
            data = d;
          }
        }
        
        if(!data){
          uint64_t id = nextDataId_++;
          data = new Data(this, id);
          newDataMap_.insert({id, data});
        }
      }
      
      uint32_t newOffset = data->insert(rowId, buf, size);
      dataIndex->insert(data->id(), newOffset, rowId);
      newLastData_ = data;
    }
    
    void update(nvar& row){
      write();
      
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
    
    bool exists_(RowId rowId){
      return dataIndex_->exists(rowId);
    }
    
    NTable* outer(){
      return o_;
    }
    
    void erase(RowId rowId){
      dataIndex_->erase(rowId);
    }
    
    void mapCompact(RowSet& rs, UpdateMap& um){
      nstr oldPath = dataIndex_->path();
      nstr newPath = oldPath + ".compact";

      newLastData_ = 0;
      DataIndex* newDataIndex = new DataIndex(d_, newPath, true);
      dataIndex_->mapCompact(this, newDataIndex, rs, um);
      delete dataIndex_;

      for(auto& itr : dataMap_){
        itr.second->erase();
        delete itr.second;
      }
      
      dataMap_ = move(newDataMap_);
      
      for(auto& itr : dataMap_){
        itr.second->save(true);
      }
      
      newDataIndex->save(true);
      newDataIndex->saveMeta();
      
      nstr op = oldPath + "/old";
      d_->safeRemoveAll(op);
      d_->safeRemove(op);
      
      d_->safeRemoveAll(oldPath);
      d_->safeRemove(oldPath);
      NSys::rename(newPath, oldPath);
      
      dataIndex_ = newDataIndex;
      dataIndex_->setPath(oldPath);
    }

    void compact(const RowSet& rs, const UpdateMap& um){
      IndexMap_ newIndexMap_;
      
      write();
      
      for(auto& itr : indexMap_){
        IndexBase* oldIndex = itr.second;
        IndexBase* newIndex;
        
        nstr oldPath = oldIndex->path();
        nstr newPath = oldPath + ".compact";
        
        switch(oldIndex->type()){
          case NTable::Int32:{
            Int32Index* ni = new Int32Index(d_, newPath, true);
            Int32Index* oi = static_cast<Int32Index*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::UInt32:{
            UInt32Index* ni = new UInt32Index(d_, newPath, true);
            UInt32Index* oi = static_cast<UInt32Index*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::Int64:{
            Int64Index* ni = new Int64Index(d_, newPath, true);
            Int64Index* oi = static_cast<Int64Index*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::UInt64:{
            UInt64Index* ni = new UInt64Index(d_, newPath, true);
            UInt64Index* oi = static_cast<UInt64Index*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::Float:{
            FloatIndex* ni = new FloatIndex(d_, newPath, true);
            FloatIndex* oi = static_cast<FloatIndex*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::Double:{
            DoubleIndex* ni = new DoubleIndex(d_, newPath, true);
            DoubleIndex* oi = static_cast<DoubleIndex*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::Row:{
            RowIndex* ni = new RowIndex(d_, newPath, true);
            RowIndex* oi = static_cast<RowIndex*>(oldIndex);
            oi->compact(*ni, rs, um);
            newIndex = ni;
            break;
          }
          case NTable::Hash:{
            HashIndex* ni = new HashIndex(d_, newPath, true);
            HashIndex* oi = static_cast<HashIndex*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          default:
            NERROR("invalid index type");
        }
        
        nstr op = oldPath + "/old";
        d_->safeRemoveAll(op);
        d_->safeRemove(op);
        
        d_->safeRemoveAll(oldPath);
        d_->safeRemove(oldPath);
        NSys::rename(newPath, oldPath);
        
        newIndex->setUnique(oldIndex->unique());
        newIndex->setAutoErase(oldIndex->autoErase());
        newIndex->setPath(oldPath);

        newIndex->save(true);
        newIndex->saveMeta();
        
        delete oldIndex;
        newIndexMap_.insert({itr.first, newIndex});
      }
      
      indexMap_ = move(newIndexMap_);
    }
    
    size_t memoryUsage(PMap& pm){
      size_t m = 0;
      for(auto& itr : indexMap_){
        m += itr.second->memoryUsage(pm);
      }
      
      m += dataIndex_->memoryUsage(pm);
      
      size_t mi;
      for(auto& itr : dataMap_){
        Data* data = itr.second;
        
        mi = data->memoryUsage();
        if(mi > 0){
          pm.insert({data->tick(), {data, mi}});
        }
      }
      
      return m;
    }
    
    void commit(){
      assert(new_);
      
      if(current_){
        return;
      }
      
      for(auto& itr : new_->dataMap_){
        Data* data = itr.second;
        data->appendToTable(this);
      }
      
      for(auto& itr : indexMap_){
        IndexBase* index = itr.second;
        auto itr2 = new_->indexMap_.find(itr.first);
        assert(itr2 != new_->indexMap_.end());
        IndexBase* index2 = itr2->second;
        index->append(index2);
      }
      
      new_->clear();
      
      dataIndex_->save(true);
      
      for(auto& itr : indexMap_){
        itr.second->save(true);
      }
      
      for(auto& itr : dataMap_){
        itr.second->save(true);
      }

      clean();
      
      saveMeta();
      current_ = true;
    }
  
    void saveMeta(){
      if(current_){
        return;
      }
      
      nvar m;
      nput(m, nextDataId_);
      m.save(metaPath_);
      
      nvar dm = nhmap();
      
      Data* data;
      for(auto& itr : dataMap_){
        data = itr.second;
        dm(itr.first) = data->size();
      }
      
      dm.save(dataMetaPath_);

      dataIndex_->saveMeta();
      
      for(auto& itr : indexMap_){
        itr.second->saveMeta();
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
    
    void setQuery(const nstr& indexName,
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
        
        if(exists_(rowId)){
          rs.insert(rowId);
        }
        
        return 1;
      };
      
      query_(indexName, start, f);
    }
    
    void traverseStart(NTable::QueryFunc qf){
      auto f = [&](RowId rowId, const nvar& v) -> int{
        nvar row;
        if(get(rowId, row)){
          return qf(row) != 0 ? 1 : 0;
        }

        return 1;
      };
      
      query_("__data", 1, f);
    }
    
    void traverseEnd(NTable::QueryFunc qf){
      auto f = [&](RowId rowId, const nvar& v) -> int{
        nvar row;
        if(get(rowId, row)){
          return qf(row) != 0 ? -1 : 0;
        }
        return -1;
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
          
          if(exists_(rowId)){
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
            bool success = get(findRowId, row);
            assert(success);
            qf(row);
          }
          
          return 0;
        };
        
        query_("__data", findRowId, f);
      }
    }

    bool getHash(const nstr& indexName, const nvar& value, nvar& row){
      size_t h = value.hash();
      
      bool success = false;
      
      auto f = [&](RowId rowId, const nvar& v) -> int{
        size_t hv = v;

        if(hv != h){
          return 0;
        }
        
        nvar r;
        success = get(rowId, r);
        assert(success);

        if(r[indexName] == value){
          row = move(r);
          success = true;
          return 0;
        }
        
        return 1;
      };
      
      query_(indexName, h, f);
      
      return success;
    }
    
    bool getFirst(const nstr& indexName, const nvar& value, nvar& row){
      bool success = false;
      
      auto f = [&](RowId rowId, const nvar& v) -> int{
        if(v != value){
          return 0;
        }
        
        success = get(rowId, row);
        assert(success);
        return 0;
      };
      
      query_(indexName, value, f);
      
      return success;
    }
    
    bool get(const nstr& indexName, const nvar& value, nvar& row){
      auto itr = indexMap_.find(indexName);
      if(itr == indexMap_.end()){
        NERROR("invalid index: " + indexName);
      }
      
      IndexBase* index = itr->second;
      if(index->type() == NTable::Hash){
        return getHash(indexName, value, row);
      }
      else{
        return getFirst(indexName, value, row);
      }
    }
    
    void dump(){
      cout << "+++++++++ DUMP TABLE: " << path_ << endl;
      
      for(auto& itr : indexMap_){
        cout << "---- DUMP INDEX: " << itr.first << endl;
        itr.second->dump();
      }
      
      dataIndex_->dump();
      
      for(auto& itr : dataMap_){
        itr.second->dump();
      }
    }
    
    NDatabase_* database(){
      return d_;
    }
    
  private:
    typedef NMap<nstr, IndexBase*> IndexMap_;
    typedef NMap<uint64_t, Data*> DataMap_;
    
    NTable* o_;
    NDatabase_* d_;
    NTable_* new_;
    nstr path_;
    nstr dataPath_;
    nstr dataMetaPath_;
    nstr metaPath_;
    NRWMutex mutex_;
    DataIndex* dataIndex_;
    DataMap_ dataMap_;
    DataMap_ newDataMap_;
    IndexMap_ indexMap_;
    Data* lastData_;
    Data* newLastData_;
    uint32_t nextDataId_;
    size_t memoryUsage_;
    bool current_;
    bool dataClean_;
    bool clean_;
  }; // end class NTable_
  
  NDatabase_::NDatabase_(NDatabase* o, const nstr& path, bool create)
  : o_(o),
  path_(path),
  metaPath_(path + "/meta.nvar"),
  tick_(0){
    
    if(create){
      if(NSys::exists(path_)){
        NERROR("path exists: " + path_);
      }
      
      if(!NSys::makeDir(path)){
        NERROR("failed to make directory: " + path);
      }
      
      nextRowId_ = 1;
      setMemoryLimit(DEFAULT_MEMORY_LIMIT);
    }
    else{
      nvar m;
      m.open(metaPath_);
      nget(m, memoryLimit_);
      nget(m, nextRowId_);
      
      nvec files;
      if(!NSys::dirFiles(path_, files)){
        NERROR("failed to read database[1]: " + path_);
      }
      
      for(const nstr& p : files){
        if(p.endsWith(".table")){
          nstr tableName = NSys::fileName(p);
          nstr fullPath = path_ + "/" + p;
          
          NTable* table = new NTable(this, fullPath, false);
          
          tableMap_.insert({tableName, table->x_});
        }
      }
    }
  }
  
  void NDatabase_::saveMeta(){
    nvar m = nhmap();
    
    nput(m, memoryLimit_);
    nput(m, nextRowId_);
    
    m.save(metaPath_);
    
    for(auto& itr : tableMap_){
      itr.second->saveMeta();
    }
  }
  
  NTable* NDatabase_::addTable(const nstr& tableName){
    auto itr = tableMap_.find(tableName);
    if(itr != tableMap_.end()){
      NERROR("table exists: " + tableName);
    }
    
    nstr path = path_ + "/" + tableName + ".table";
    
    NTable* table = new NTable(this, path, true);
    NTable_* t = table->x_;
    
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

  void NDatabase_::eraseTable(const nstr& tableName){
    auto itr = tableMap_.find(tableName);
    if(itr == tableMap_.end()){
      NERROR("invalid table: " + tableName);
    }
    
    itr->second->erase();
    
    tableMap_.erase(itr);
    
    saveMeta();
  }
  
  void NDatabase_::compact(){
    RowSet rs;
    UpdateMap um;

    for(auto& itr : tableMap_){
      itr.second->writeLock_();
    }
    
    for(auto& itr : tableMap_){
      itr.second->mapCompact(rs, um);
    }
    
    for(auto& itr : tableMap_){
      itr.second->compact(rs, um);
    }
    
    for(auto& itr : tableMap_){
      itr.second->saveMeta();
    }
    
    for(auto& itr : tableMap_){
      itr.second->unlock_();
    }
    
    saveMeta();
  }

  void NDatabase_::rollback(){
    for(auto& itr : tableMap_){
      NTable_* t = itr.second;
      t->rollback();
    }
  }
  
  size_t NDatabase_::memoryUsage(PMap& pm){
    size_t m = 0;
    for(auto& itr : tableMap_){
      m += itr.second->memoryUsage(pm);
    }
    
    return m;
  }
  
  void NDatabase_::checkMemory(){
    PMap pm;
    
    int64_t m = memoryUsage(pm);
    
    while(m > memoryLimit_){
      auto itr = pm.begin();
      if(itr == pm.end()){
        break;
      }

      Pageable* p = itr->second.first;
      if(p->locked()){
        pm.erase(itr);
        continue;
      }
      
      p->store();
      
      size_t mi = itr->second.second;
      m -= mi;
      
      pm.erase(itr);
    }
  }
  
  void NDatabase_::commit(){
    for(auto& itr : tableMap_){
      itr.second->writeLock_();
    }
    
    for(auto& itr : tableMap_){
      itr.second->commit();
    }
    
    for(auto& itr : tableMap_){
      itr.second->unlock_();
    }
    
    saveMeta();
  }
  
} // end namespace neu

NTable::NTable(NDatabase_* d, const nstr& path, bool create){
  x_ = new NTable_(this, d, path, create);
}

void NTable::addIndex(const nstr& indexName,
                      IndexType indexType,
                      bool unique){
  x_->addIndex(indexName, indexType, unique, false);
}

void NTable::addRowIndex(const nstr& indexName,
                         bool unique,
                         bool autoErase){
  x_->addIndex(indexName, Row, unique, autoErase);
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

void NTable::setQuery(const nstr& indexName,
                      const nvar& start,
                      const nvar& end,
                      RowSet& rs){
  x_->setQuery(indexName, start, end, rs);
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

bool NTable::get(const nstr& indexName, const nvar& value, nvar& row){
  return x_->get(indexName, value, row);
}

void NTable::dump(){
  x_->dump();
}

void NTable::commit(){
  x_->commit();
}

void NTable::rollback(){
  x_->rollback();
}

void NTable::readLock_(){
  x_->readLock_();
}

void NTable::writeLock_(){
  x_->writeLock_();
}

void NTable::unlock_(){
  x_->unlock_();
}

NDatabase::NDatabase(const nstr& path, bool create){
  x_ = new NDatabase_(this, path, create);
}

NTable* NDatabase::addTable(const nstr& tableName){
  return x_->addTable(tableName);
}

NTable* NDatabase::getTable(const nstr& tableName){
  return x_->getTable(tableName);
}

void NDatabase::eraseTable(const nstr& tableName){
  x_->eraseTable(tableName);
}

void NDatabase::compact(){
  x_->compact();
}

void NDatabase::commit(){
  x_->commit();
}

void NDatabase::setMemoryLimit(size_t megabytes){
  x_->setMemoryLimit(megabytes);
}

void NDatabase::rollback(){
  x_->rollback();
}

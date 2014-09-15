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

#ifndef NEU_N_HASH_MAP_H
#define NEU_N_HASH_MAP_H

#include <unordered_map>

#include <neu/NError.h>

namespace neu{
  
  template<class Key,
  class T,
  class Hash = std::hash<Key>,
  class Pred = std::equal_to<Key>,
  class Alloc = std::allocator<std::pair<const Key, T>>>
  class NHashMap{
  public:
    typedef std::unordered_map<Key, T, Hash, Pred, Alloc> HashMap;
    
    typedef Key key_type;
    typedef T mapped_type;
    typedef Hash hasher;
    typedef Pred key_equal;
    typedef Alloc allocator_type;
    typedef std::pair<const key_type, mapped_type> value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef typename HashMap::iterator iterator;
    typedef typename HashMap::const_iterator const_iterator;
    typedef typename HashMap::local_iterator local_iterator;
    typedef typename HashMap::const_local_iterator const_local_iterator;
    typedef typename HashMap::size_type size_type;
    typedef typename HashMap::difference_type difference_type;
    
    NHashMap()
    noexcept(std::is_nothrow_default_constructible<hasher>::value &&
             std::is_nothrow_default_constructible<key_equal>::value &&
             std::is_nothrow_default_constructible<allocator_type>::value){}
    
    explicit NHashMap(size_type n, const hasher& hf = hasher(),
                      const key_equal& eql = key_equal(),
                      const allocator_type& a = allocator_type())
    : m_(n, hf, eql, a){}
    
    template<class InputIterator>
    NHashMap(InputIterator f, InputIterator l,
             size_type n = 0, const hasher& hf = hasher(),
             const key_equal& eql = key_equal(),
             const allocator_type& a = allocator_type())
    : m_(f, l, n, hf, eql, a){}
    
    explicit NHashMap(const allocator_type& at)
    : m_(at){}
    
    NHashMap(const NHashMap& m)
    : m_(m.m_){}
    
    NHashMap(const NHashMap& m, const Alloc& a)
    : m_(m.m_, a){}
    
    NHashMap(const NHashMap&& m)
    noexcept(std::is_nothrow_move_constructible<hasher>::value &&
             std::is_nothrow_move_constructible<key_equal>::value &&
             std::is_nothrow_move_constructible<allocator_type>::value)
    : m_(std::move(m.m_)){}
    
    NHashMap(NHashMap&& m, const Alloc& a)
    : m_(std::move(m.m_), a){}
    
    NHashMap(const HashMap& m)
    : m_(m){}
    
    NHashMap(const HashMap& m, const Alloc& a)
    : m_(m, a){}
    
    NHashMap(const HashMap&& m)
    noexcept(std::is_nothrow_move_constructible<hasher>::value &&
             std::is_nothrow_move_constructible<key_equal>::value &&
             std::is_nothrow_move_constructible<allocator_type>::value)
    : m_(std::move(m)){}
    
    NHashMap(HashMap&& m, const Alloc& a)
    : m_(std::move(m), a){}
    
    NHashMap(std::initializer_list<value_type> il,
             size_type n = 0,
             const hasher& hf = hasher(),
             const key_equal& eql = key_equal(),
             const allocator_type& a = allocator_type())
    : m_(il, n, hf, eql, a){}
    
    ~NHashMap(){}
    
    NHashMap& operator=(const NHashMap& m){
      m_ = m.m_;
      return *this;
    }
    
    NHashMap& operator=(NHashMap&& m)
    noexcept(allocator_type::propagate_on_container_move_assignment::value &&
             std::is_nothrow_move_assignable<allocator_type>::value &&
             std::is_nothrow_move_assignable<hasher>::value &&
             std::is_nothrow_move_assignable<key_equal>::value){
      m_ = std::move(m.m_);
      return *this;
    }
    
    NHashMap& operator=(const HashMap& m){
      m_ = m;
      return *this;
    }
    
    NHashMap& operator=(HashMap&& m)
    noexcept(allocator_type::propagate_on_container_move_assignment::value &&
             std::is_nothrow_move_assignable<allocator_type>::value &&
             std::is_nothrow_move_assignable<hasher>::value &&
             std::is_nothrow_move_assignable<key_equal>::value){
      m_ = std::move(m);
      return *this;
    }
    
    NHashMap& operator=(std::initializer_list<value_type> il){
      m_ = il;
      return *this;
    }
    
    allocator_type get_allocator() const noexcept{
      return m_.get_allocator();
    }
    
    bool empty() const noexcept{
      return m_.empty();
    }
    
    size_type size() const noexcept{
      return m_.size();
    }
    
    size_type max_size() const noexcept{
      return m_.max_size();
    }
    
    iterator begin() noexcept{
      return m_.begin();
    }
    
    iterator end() noexcept{
      return m_.end();
    }
    
    const_iterator begin() const noexcept{
      return m_.begin();
    }
    
    const_iterator end() const noexcept{
      return m_.end();
    }
    
    const_iterator cbegin() const noexcept{
      return m_.cbegin();
    }
    
    const_iterator cend() const noexcept{
      return m_.cend();
    }
    
    template<class... Args>
    std::pair<iterator, bool> emplace(Args&&... args){
      return m_.emplace(std::forward<Args>(args)...);
    }
    
    template<class... Args>
    iterator emplace_hint(const_iterator position, Args&&... args){
      return m_.emplace_hint(position, std::forward<Args>(args)...);
    }
    
    std::pair<iterator, bool> insert(const value_type& obj){
      return m_.insert(obj);
    }
    
    template<class P>
    std::pair<iterator, bool> insert(P&& obj);
    iterator insert(const_iterator hint, const value_type& obj){
      return m_.insert(hint, std::move(obj));
    }
    
    template<class P>
    iterator insert(const_iterator hint, P&& obj){
      return m_.insert(hint, std::move(obj));
    }
    
    template<class InputIterator>
    void insert(InputIterator first, InputIterator last){
      m_.insert(first, last);
    }
    
    void insert(std::initializer_list<value_type> il){
      m_.insert(il);
    }
    
    iterator erase(const_iterator position){
      return m_.erase(position);
    }
    
    size_type erase(const key_type& k){
      return m_.erase(k);
    }
    
    iterator erase(const_iterator first, const_iterator last){
      return m_.erase(first, last);
    }
    
    void clear() noexcept{
      m_.clear();
    }
    
    void swap(NHashMap& m){
      m_.swap(m);
    }
    
    hasher hash_function() const{
      return m_.hash_function();
    }
    
    key_equal key_eq() const{
      return m_.key_eq();
    }
    
    iterator find(const key_type& k){
      return m_.find(k);
    }
    
    const_iterator find(const key_type& k) const{
      return m_.find(k);
    }
    
    size_type count(const key_type& k) const{
      return m_.count(k);
    }
    
    std::pair<iterator, iterator> equal_range(const key_type& k){
      return m_.equal_range(k);
    }
    
    std::pair<const_iterator, const_iterator>
    equal_range(const key_type& k) const{
      return m_.equal_range(k);
    }
    
    mapped_type& operator[](const key_type& k){
      return m_[k];
    }
    
    mapped_type& operator[](key_type&& k){
      return m_[std::move(k)];
    }
    
    mapped_type& at(const key_type& k){
      return m_.at(k);
    }
    
    const mapped_type& at(const key_type& k) const{
      return m_.at(k);
    }
    
    size_type bucket_count() const noexcept{
      return m_.bucket_count();
    }
    
    size_type max_bucket_count() const noexcept{
      return m_.max_bucket_count();
    }
    
    size_type bucket_size(size_type n) const{
      return m_.buckset_size(n);
    }
    
    size_type bucket(const key_type& k) const{
      return m_.bucket(k);
    }
    
    local_iterator begin(size_type n){
      return m_.begin(n);
    }
    
    local_iterator end(size_type n){
      return m_.end(n);
    }
    
    const_local_iterator begin(size_type n) const{
      return m_.begin(n);
    }
    
    const_local_iterator end(size_type n) const{
      return m_.end(n);
    }
    
    const_local_iterator cbegin(size_type n) const{
      return m_.cbegin(n);
    }
    
    const_local_iterator cend(size_type n) const{
      return m_.cend(n);
    }
    
    float load_factor() const noexcept{
      return m_.load_factor();
    }
    
    float max_load_factor() const noexcept{
      return m_.max_load_factor();
    }
    
    void max_load_factor(float z){
      return m_.max_load_factor(z);
    }
    
    void rehash(size_type n){
      m_.rehash(n);
    }
    
    void reserve(size_type n){
      m_.reserve(n);
    }
    
    HashMap& map(){
      return m_;
    }
    
    const HashMap& map() const{
      return m_;
    }
    
    bool has(const key_type& x) const{
      return m_.find(x) != m_.end();
    }
    
    void merge(const NHashMap& m){
      m_.insert(m.begin(), m.end());
    }
    
    void outerMerge(const NHashMap& m){
      NHashMap::const_iterator i = m.begin();
      while(i != m.end()){
        NHashMap::iterator j = m_.find(i->first);
        if(j != m_.end()){
          j->second = i->second;
        }
        else{
          m_.insert(std::pair<Key, T>(i->first, i->second));
        }
        ++i;
      }
    }
    
    const T& get(const key_type& x, const T& def) const{
      auto itr = m_.find(x);
      return itr == m_.end() ? def : itr->second;
    }
    
    T& get(const key_type& x, T& def){
      auto itr = m_.find(x);
      return itr == m_.end() ? def : itr->second;
    }
    
    Key keyForValue(const T& value){
      for(iterator itr = m_.begin(), itrEnd = m_.end();
          itr != itrEnd; ++itr){
        if(itr->second == value){
          return itr->first;
        }
      }
      
      NERROR("failed to find key for value");
    }
    
  private:
    HashMap m_;
  };
  
  template <class Key, class T, class Hash, class Pred, class Alloc>
  void swap(NHashMap<Key, T, Hash, Pred, Alloc>& x,
            NHashMap<Key, T, Hash, Pred, Alloc>& y){
    swap(x.map(), y.map());
  }
  
  template <class Key, class T, class Hash, class Pred, class Alloc>
  bool operator==(const NHashMap<Key, T, Hash, Pred, Alloc>& a,
                  const NHashMap<Key, T, Hash, Pred, Alloc>& b){
    return a.map() == b.map();
  }
  
  template <class Key, class T, class Hash, class Pred, class Alloc>
  bool operator!=(const NHashMap<Key, T, Hash, Pred, Alloc>& a,
                  const NHashMap<Key, T, Hash, Pred, Alloc>& b){
    return a.map() != b.map();
  }
  
  template <class Key, class T, class Hash, class Pred, class Alloc>
  std::ostream& operator<<(std::ostream& ostr,
                           const NHashMap<Key, T, Hash, Pred, Alloc>& m){
    typename NHashMap<Key, T, Hash, Pred, Alloc>::const_iterator itr = m.begin();
    ostr << "[= ";
    bool first = true;
    while(itr != m.end()){
      if(first){
        first = false;
      }
      else{
        ostr << ", ";
      }
      
      ostr << itr->first << ":";
      ostr << itr->second;
      ++itr;
    }
    ostr << "]";
    return ostr;
  }
  
} // end namespace neu

#endif // NEU_N_HASH_MAP_H

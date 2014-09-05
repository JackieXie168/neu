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

#ifndef NEU_N_HASH_SET_H
#define NEU_N_HASH_SET_H

#include <unordered_set>
#include <algorithm>

template<class Value,
class Hash = std::hash<Value>,
class Pred = std::equal_to<Value>,
class Alloc = std::allocator<Value>>
class NHashSet{
public:
  typedef std::unordered_set<Value, Hash, Pred, Alloc> HashSet;
  
  typedef Value key_type;
  typedef key_type value_type;
  typedef Hash hasher;
  typedef Pred key_equal;
  typedef Alloc allocator_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef typename std::allocator_traits<allocator_type>::pointer pointer;
  typedef typename std::allocator_traits<allocator_type>::const_pointer
  const_pointer;
  
  typedef typename std::allocator_traits<allocator_type>::size_type size_type;
  typedef typename std::allocator_traits<allocator_type>::difference_type
  difference_type;
  
  typedef typename HashSet::iterator iterator;
  typedef typename HashSet::const_iterator const_iterator;
  typedef typename HashSet::local_iterator local_iterator;
  typedef typename HashSet::const_local_iterator const_local_iterator;
  
  NHashSet()
  noexcept(std::is_nothrow_default_constructible<hasher>::value &&
           std::is_nothrow_default_constructible<key_equal>::value &&
           std::is_nothrow_default_constructible<allocator_type>::value){}
  
  explicit NHashSet(size_type n, const hasher& hf = hasher(),
                   const key_equal& eql = key_equal(),
                   const allocator_type& a = allocator_type())
  : s_(n, hf, eql, a){}

  template<class InputIterator>
  NHashSet(InputIterator f, InputIterator l,
          size_type n = 0, const hasher& hf = hasher(),
          const key_equal& eql = key_equal(),
          const allocator_type& a = allocator_type())
  : s_(f, l, n, eql, a){}
  
  explicit NHashSet(const allocator_type& at)
  : s_(at){}
  
  NHashSet(const NHashSet& s)
  : s_(s.s_){}

  NHashSet(const HashSet& s)
  : s_(s){}
  
  NHashSet(const NHashSet& s, const Alloc& a)
  : s_(s.s_, a){}

  NHashSet(const HashSet& s, const Alloc& a)
  : s_(s, a){}
  
  NHashSet(NHashSet&& s)
  noexcept(std::is_nothrow_move_constructible<hasher>::value &&
           std::is_nothrow_move_constructible<key_equal>::value &&
           std::is_nothrow_move_constructible<allocator_type>::value)
  : s_(std::move(s.s_)){}

  NHashSet(HashSet&& s)
  noexcept(std::is_nothrow_move_constructible<hasher>::value &&
           std::is_nothrow_move_constructible<key_equal>::value &&
           std::is_nothrow_move_constructible<allocator_type>::value)
  : s_(std::move(s)){}
  
  NHashSet(NHashSet&& s, const Alloc& a)
  : s_(std::move(s.s_), a){}

  NHashSet(HashSet&& s, const Alloc& a)
  : s_(std::move(s), a){}
  
  NHashSet(std::initializer_list<value_type> il,
          size_type n = 0,
          const hasher& hf = hasher(),
          const key_equal& eql = key_equal(),
          const allocator_type& a = allocator_type())
  : s_(il, n, hf, eql, a){}
  
  ~NHashSet(){}

  NHashSet& operator=(const NHashSet& s){
    s_ = s.s_;
    return *this;
  }

  NHashSet& operator=(const HashSet& s){
    s_ = s;
    return *this;
  }
  
  NHashSet& operator=(NHashSet&& s)
  noexcept(allocator_type::propagate_on_container_move_assignment::value &&
           std::is_nothrow_move_assignable<allocator_type>::value &&
           std::is_nothrow_move_assignable<hasher>::value &&
           std::is_nothrow_move_assignable<key_equal>::value){
    s_ = std::move(s.s_);
    return *this;
  }

  NHashSet& operator=(HashSet&& s)
  noexcept(allocator_type::propagate_on_container_move_assignment::value &&
           std::is_nothrow_move_assignable<allocator_type>::value &&
           std::is_nothrow_move_assignable<hasher>::value &&
           std::is_nothrow_move_assignable<key_equal>::value){
    s_ = std::move(s);
    return *this;
  }
  
  NHashSet& operator=(std::initializer_list<value_type> il){
    s_ = il;
    return *this;
  }
  
  allocator_type get_allocator() const noexcept{
    return s_.get_allocator();
  }
  
  bool empty() const noexcept{
    return s_.empty();
  }
  
  size_type size() const noexcept{
    return s_.size();
  }

  size_type max_size() const noexcept{
    return s_.max_size();
  }
  
  iterator begin() noexcept{
    return s_.begin();
  }
  
  iterator end() noexcept{
    return s_.end();
  }

  const_iterator begin() const noexcept{
    return s_.begin();
  }

  const_iterator end() const noexcept{
    return s_.end();
  }

  const_iterator cbegin() const noexcept{
    return s_.cbegin();
  }

  const_iterator cend() const noexcept{
    return s_.cend();
  }
  
  template<class... Args>
  std::pair<iterator, bool> emplace(Args&&... args){
    return s_.emplace(std::forward<Args>(args)...);
  }

  template<class... Args>
  iterator emplace_hint(const_iterator position, Args&&... args){
    return s_.emplace_hint(position, std::forward<Args>(args)...);
  }
  
  std::pair<iterator, bool> insert(const value_type& obj){
    return s_.insert(obj);
  }
  
  std::pair<iterator, bool> insert(value_type&& obj){
    return s_.insert(std::move(obj));
  }

  std::pair<iterator, bool> add(const value_type& obj){
    return s_.insert(obj);
  }
  
  std::pair<iterator, bool> add(value_type&& obj){
    return s_.insert(std::move(obj));
  }
  
  iterator insert(const_iterator hint, const value_type& obj){
    return s_.insert(hint, obj);
  }
  
  iterator insert(const_iterator hint, value_type&& obj){
    return s_.insert(hint, std::move(obj));
  }
  
  template<class InputIterator>
  void insert(InputIterator first, InputIterator last){
    s_.insert(first, last);
  }
  
  void insert(std::initializer_list<value_type> il){
    s_.insert(il);
  }
  
  iterator erase(const_iterator position){
    return s_.erase(position);
  }
  
  size_type erase(const key_type& k){
    return s_.erase(k);
  }
  
  iterator erase(const_iterator first, const_iterator last){
    return s_.erase(first, last);
  }
  
  void clear() noexcept{
    s_.clear();
  }
  
  void swap(NHashSet& s){
    s_.swap(s.s_);
  }

  void swap(HashSet& s){
    s_.swap(s);
  }
  
  hasher hash_function() const{
    return s_.hash_function();
  }

  key_equal key_eq() const{
    return s_.key_eq();
  }
  
  iterator find(const key_type& k){
    return s_.find(k);
  }
  
  const_iterator find(const key_type& k) const{
    return s_.find(k);
  }
  
  size_type count(const key_type& k) const{
    return s_.count(k);
  }
  
  std::pair<iterator, iterator> equal_range(const key_type& k){
    return s_.equal_range(k);
  }
  
  std::pair<const_iterator, const_iterator>
  equal_range(const key_type& k) const{
    return s_.equal_range(k);
  }
  
  size_type bucket_count() const noexcept{
    return s_.bucket_count();
  }
  
  size_type max_bucket_count() const noexcept{
    return s_.max_bucket_count();
  }
  
  size_type bucket_size(size_type n) const{
    return s_.bucket_size(n);
  }
  
  size_type bucket(const key_type& k) const{
    return s_.bucket(k);
  }
  
  local_iterator begin(size_type n){
    return s_.begin(n);
  }
  
  local_iterator end(size_type n){
    return s_.end();
  }
  
  const_local_iterator begin(size_type n) const{
    return s_.begin(n);
  }
  
  const_local_iterator end(size_type n) const{
    return s_.end(n);
  }
  
  const_local_iterator cbegin(size_type n) const{
    return s_.cbegin(n);
  }
  
  const_local_iterator cend(size_type n) const{
    return s_.cend(n);
  }
  
  float load_factor() const noexcept{
    return s_.load_factor();
  }
  
  float max_load_factor() const noexcept{
    return s_.max_load_factor();
  }
  
  void max_load_factor(float z){
    s_.max_load_factor(z);
  }
  
  void rehash(size_type n){
    s_.rehash(n);
  }
  
  void reserve(size_type n){
    s_.reserve(n);
  }
  
  const HashSet& set() const{
    return s_;
  }
  
  HashSet& set(){
    return s_;
  }
  
  void intersect(const NHashSet& s){
    HashSet r;
    std::set_intersection(s_.begin(), s_.end(),
                          s.s_.begin(), s.s_.end(),
                          std::inserter(r, r.begin()));
    
    s_ = std::move(r);
  }
  
  void unite(const NHashSet& s){
    HashSet r;
    
    std::set_union(s_.begin(), s_.end(),
                   s.s_.begin(), s.s_.end(),
                   std::inserter(r, r.begin()));
    
    s_ = std::move(r);
  }
  
  void complement(const NHashSet& s){
    HashSet r;
    
    std::set_difference(s_.begin(), s_.end(),
                        s.s_.begin(), s.s_.end(),
                        std::inserter(r, r.begin()));
    
    s_ = std::move(r);
  }
  
  bool has(const key_type& x) const{
    return s_.find(x) != s_.end();
  }
  
private:
  HashSet s_;
};

template <class Value, class Hash, class Pred, class Alloc>
void swap(NHashSet<Value, Hash, Pred, Alloc>& x,
          NHashSet<Value, Hash, Pred, Alloc>& y)
noexcept(noexcept(x.swap(y))){
  swap(x.set(), y.set());
}

template <class Value, class Hash, class Pred, class Alloc>
bool
operator==(const NHashSet<Value, Hash, Pred, Alloc>& x,
           const NHashSet<Value, Hash, Pred, Alloc>& y){
  return x.set() == y.set();
}

template <class Value, class Hash, class Pred, class Alloc>
bool
operator!=(const NHashSet<Value, Hash, Pred, Alloc>& x,
           const NHashSet<Value, Hash, Pred, Alloc>& y){
  return x.set() != y.set();
}

template <class Value, class Hash, class Pred, class Alloc>
std::ostream& operator<<(std::ostream& ostr,
                         const NHashSet<Value, Hash, Pred, Alloc>& s){
  auto itr = s.begin();
  ostr << "[^ ";
  bool first = true;
  while(itr != s.end()){
    if(first){
      first = false;
    }
    else{
      ostr << ", ";
    }

    ostr << *itr << ":";
    ++itr;
  }
  ostr << "]";
  return ostr;
}

#endif // NEU_N_HASH_SET_H

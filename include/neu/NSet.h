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

#ifndef NEU_N_SET_H
#define NEU_N_SET_H

#include <set>
#include <algorithm>

namespace neu{
  
  template <class Key,
  class Compare = std::less<Key>,
  class Allocator = std::allocator<Key>>
  class NSet{
  public:
    typedef std::set<Key, Compare, Allocator> Set;
    
    typedef Key key_type;
    typedef key_type value_type;
    typedef Compare key_compare;
    typedef key_compare value_compare;
    typedef Allocator allocator_type;
    typedef typename allocator_type::reference reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::difference_type difference_type;
    typedef typename allocator_type::pointer pointer;
    typedef typename allocator_type::const_pointer const_pointer;
    
    typedef typename Set::iterator iterator;
    typedef typename Set::const_iterator const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    
    NSet()
    noexcept(std::is_nothrow_default_constructible<allocator_type>::value &&
             std::is_nothrow_default_constructible<key_compare>::value &&
             std::is_nothrow_copy_constructible<key_compare>::value){}
    
    explicit NSet(const value_compare& comp)
    : s_(comp){}
    
    NSet(const value_compare& comp, const allocator_type& a)
    : s_(comp, a){}
    
    template <class InputIterator>
    NSet(InputIterator first, InputIterator last,
         const value_compare& comp = value_compare())
    : s_(first, last){}
    
    template <class InputIterator>
    NSet(InputIterator first,
         InputIterator last,
         const value_compare& comp,
         const allocator_type& a)
    : s_(first, last, comp, a){}
    
    NSet(const NSet& s)
    : s_(s.s_){}
    
    NSet(const Set& s)
    : s_(s){}
    
    NSet(NSet&& s)
    noexcept(std::is_nothrow_move_constructible<allocator_type>::value &&
             std::is_nothrow_move_constructible<key_compare>::value)
    : s_(std::move(s.s_)){}
    
    NSet(Set&& s)
    noexcept(std::is_nothrow_move_constructible<allocator_type>::value &&
             std::is_nothrow_move_constructible<key_compare>::value)
    : s_(std::move(s)){}
    
    explicit NSet(const allocator_type& a)
    : s_(a){}
    
    NSet(const NSet& s, const allocator_type& a)
    : s_(s.s_, a){}
    
    NSet(NSet&& s, const allocator_type& a)
    : s_(std::move(s.s_), a){}
    
    NSet(std::initializer_list<value_type> il,
         const value_compare& comp = value_compare())
    : s_(il, comp){}
    
    NSet(std::initializer_list<value_type> il, const value_compare& comp,
         const allocator_type& a)
    : s_(il, comp, a){}
    
    ~NSet(){}
    
    NSet& operator=(const NSet& s){
      s_ = s.s_;
      return *this;
    }
    
    NSet& operator=(const Set& s){
      s_ = s;
      return *this;
    }
    
    NSet& operator=(NSet&& s)
    noexcept(allocator_type::propagate_on_container_move_assignment::value &&
             std::is_nothrow_move_assignable<allocator_type>::value &&
             std::is_nothrow_move_assignable<key_compare>::value){
      s_ = std::move(s.s_);
      return *this;
    }
    
    NSet& operator=(Set&& s)
    noexcept(allocator_type::propagate_on_container_move_assignment::value &&
             std::is_nothrow_move_assignable<allocator_type>::value &&
             std::is_nothrow_move_assignable<key_compare>::value){
      s_ = std::move(s);
      return *this;
    }
    
    NSet& operator=(std::initializer_list<value_type> il){
      s_ = il;
      return *this;
    }
    
    iterator begin() noexcept{
      return s_.begin();
    }
    
    const_iterator begin() const noexcept{
      return s_.begin();
    }
    
    iterator end() noexcept{
      return s_.end();
    }
    
    const_iterator end() const noexcept{
      return s_.end();
    }
    
    reverse_iterator rbegin() noexcept{
      return s_.rbegin();
    }
    
    const_reverse_iterator rbegin() const noexcept{
      return s_.rbegin();
    }
    
    reverse_iterator rend() noexcept{
      return s_.rend();
    }
    
    const_reverse_iterator rend() const noexcept{
      return s_.rend();
    }
    
    const_iterator cbegin() const noexcept{
      return s_.cbegin();
    }
    
    const_iterator cend() const noexcept{
      return s_.cend();
    }
    
    const_reverse_iterator crbegin() const noexcept{
      return s_.crbegin();
    }
    
    const_reverse_iterator crend() const noexcept{
      return s_.crend();
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
    
    template <class... Args>
    std::pair<iterator, bool> emplace(Args&&... args){
      return s_.emplace(std::forward<Args>(args)...);
    }
    
    template <class... Args>
    iterator emplace_hint(const_iterator position, Args&&... args){
      return s_.emplace_hint(position, std::forward<Args>(args)...);
    }
    
    std::pair<iterator, bool> insert(const value_type& v){
      return s_.insert(v);
    }
    
    std::pair<iterator, bool> insert(value_type&& v){
      return s_.insert(std::move(v));
    }

    std::pair<iterator, bool> add(const value_type& v){
      return s_.insert(v);
    }
    
    std::pair<iterator, bool> add(value_type&& v){
      return s_.insert(std::move(v));
    }
    
    iterator insert(const_iterator position, const value_type& v){
      return s_.insert(position, v);
    }
    
    iterator insert(const_iterator position, value_type&& v){
      return s_.insert(position, std::move(v));
    }
    
    template <class InputIterator>
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
    
    void swap(NSet& s){
      s_.swap(s);
    }
    
    allocator_type get_allocator() const noexcept{
      return s_.get_allocator();
    }
    
    key_compare key_comp() const{
      return s_.key_comp();
    }
    
    value_compare value_comp() const{
      return s_.value_comp();
    }
    
    iterator find(const key_type& k){
      return s_.find(k);
    }
    
    const_iterator find(const key_type& k) const{
      return s_.find(k);
    }
    
    template<typename K>
    iterator find(const K& x){
      return s_.find(x);
    }
    
    size_type count(const key_type& k) const{
      return s_.count(k);
    }
    
    iterator lower_bound(const key_type& k){
      return s_.lower_bound(k);
    }
    
    const_iterator lower_bound(const key_type& k) const{
      return s_.lower_bound(k);
    }
    
    iterator upper_bound(const key_type& k){
      return s_.upper_bound(k);
    }
    
    const_iterator upper_bound(const key_type& k) const{
      return s_.upper_bound(k);
    }
    
    std::pair<iterator, iterator> equal_range(const key_type& k){
      return s_.equal_range(k);
    }
    
    std::pair<const_iterator, const_iterator>
    equal_range(const key_type& k) const{
      return s_.equal_range(k);
    }
    
    void intersect(const NSet& s){
      Set r;
      std::set_intersection(s_.begin(), s_.end(),
                            s.s_.begin(), s.s_.end(),
                            std::inserter(r, r.begin()));
      
      s_ = std::move(r);
    }
    
    void unite(const NSet& s){
      Set r;
      
      std::set_union(s_.begin(), s_.end(),
                     s.s_.begin(), s.s_.end(),
                     std::inserter(r, r.begin()));
      
      s_ = std::move(r);
    }
    
    void complement(const NSet& s){
      Set r;
      
      std::set_difference(s_.begin(), s_.end(),
                          s.s_.begin(), s.s_.end(),
                          std::inserter(r, r.begin()));
      
      s_ = std::move(r);
    }
    
    Set& set(){
      return s_;
    }
    
    const Set& set() const{
      return s_;
    }
    
    bool has(const key_type& x) const{
      return s_.find(x) != s_.end();
    }
    
  private:
    Set s_;
  };
  
  template<class Key, class Compare, class Allocator>
  bool
  operator==(const NSet<Key, Compare, Allocator>& x,
             const NSet<Key, Compare, Allocator>& y){
    return x.set() == y.set();
  }
  
  template<class Key, class Compare, class Allocator>
  bool
  operator<(const NSet<Key, Compare, Allocator>& x,
            const NSet<Key, Compare, Allocator>& y){
    return x.set() < y.set();
  }
  
  template<class Key, class Compare, class Allocator>
  bool
  operator!=(const NSet<Key, Compare, Allocator>& x,
             const NSet<Key, Compare, Allocator>& y){
    return x.set() != y.set();
  }
  
  template<class Key, class Compare, class Allocator>
  bool
  operator>(const NSet<Key, Compare, Allocator>& x,
            const NSet<Key, Compare, Allocator>& y){
    return x.set() > y.set();
  }
  
  template<class Key, class Compare, class Allocator>
  bool
  operator>=(const NSet<Key, Compare, Allocator>& x,
             const NSet<Key, Compare, Allocator>& y){
    return x.set() >= y.set();
  }
  
  template<class Key, class Compare, class Allocator>
  bool
  operator<=(const NSet<Key, Compare, Allocator>& x,
             const NSet<Key, Compare, Allocator>& y){
    return x.set() <= y.set();
  }
  
  template <class Key, class Compare, class Allocator>
  void
  swap(NSet<Key, Compare, Allocator>& x, NSet<Key, Compare, Allocator>& y)
  noexcept(noexcept(x.swap(y))){
    swap(x.set(), y.set());
  }
  
  template <class Key, class Compare, class Allocator>
  std::ostream& operator<<(std::ostream& ostr,
                           const NSet<Key, Compare, Allocator>& s){
    typename NSet<Key, Compare, Allocator>::const_iterator itr = s.begin();
    ostr << "[% ";
    while(itr != s.end()){
      if(itr != s.begin()){
        ostr << ", ";
      }
      ostr << *itr << ":";
      ++itr;
    }
    ostr << "]";
    return ostr;
  }
  
} // end namespace neu

#endif // NEU_N_Set_H

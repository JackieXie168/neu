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

#include <map>
#include <ostream>

#ifndef NEU_N_MULTIMAP_H
#define NEU_N_MULTIMAP_H

namespace neu{
  
  template<class Key, class T, class Compare = std::less<Key>,
  class Allocator = std::allocator<std::pair<const Key,T> > >
  class NMultimap{
  public:
    typedef std::multimap<Key, T, Compare, Allocator> Map;
    
    typedef typename Map::iterator iterator;
    typedef typename Map::const_iterator const_iterator;
    typedef typename Map::key_type key_type;
    typedef typename Map::allocator_type allocator_type;
    typedef typename Map::value_type value_type;
    typedef typename Map::key_compare key_compare;
    typedef typename Map::reverse_iterator reverse_iterator;
    typedef typename Map::const_reverse_iterator const_reverse_iterator;
    typedef typename Map::value_compare value_compare;
    
    explicit NMultimap(const Compare& comp=Compare(),
                       const Allocator& allocator=Allocator())
    : m_(comp, allocator){}
    
    template<class InputIterator>
    NMultimap(InputIterator first, InputIterator last,
              const Compare& comp=Compare(),
              const Allocator& allocator=Allocator())
    : m_(first, last, comp, allocator){}
    
    NMultimap(const NMultimap& x)
    : m_(x.m_){}
    
    NMultimap(NMultimap&& x)
    : m_(std::move(x.m_)){}
    
    NMultimap(NMultimap&& x, const allocator_type& a)
    : m_(std::move(x.m_), a){}
    
    NMultimap(std::initializer_list<value_type> il,
              const key_compare& comp = key_compare())
    : m_(il, comp){}
    
    NMultimap(std::initializer_list<value_type> il,
              const key_compare& comp,
              const allocator_type& a)
    : m_(il, comp, a){}
    
    ~NMultimap(){}
    
    iterator begin() noexcept{
      return m_.begin();
    }
    
    const_iterator begin() const noexcept{
      return m_.begin();
    }
    
    void clear() noexcept{
      m_.clear();
    }
    
    size_t count(const key_type& x) const{
      return m_.count(x);
    }
    
    bool empty() const noexcept{
      return m_.empty();
    }
    
    iterator end(){
      return m_.end();
    }
    
    const_iterator end() const{
      return m_.end();
    }
    
    std::pair<iterator,iterator>
    equal_range(const key_type& x){
      return m_.equal_range(x);
    }
    
    std::pair<const_iterator,const_iterator>
    equal_range(const key_type& x) const{
      return m_.equal_range(x);
    }
    
    void erase(iterator position){
      m_.erase(position);
    }
    
    size_t erase(const key_type& x){
      return m_.erase(x);
    }
    
    void erase(const_iterator first, const_iterator last){
      m_.erase(first, last);
    }
    
    iterator find(const key_type& x){
      return m_.find(x);
    }
    
    const_iterator find(const key_type& x) const{
      return m_.find(x);
    }
    
    allocator_type get_allocator() const noexcept{
      return m_.get_allocator();
    }
    
    iterator insert(const value_type& x){
      return m_.insert(x);
    }
    
    iterator insert(iterator position, const value_type& x){
      return m_.insert(position, x);
    }
    
    template<class InputIterator>
    void insert(InputIterator first, InputIterator last){
      m_.insert(first, last);
    }
    
    template <class P>
    iterator insert(const_iterator position, P&& p){
      return m_.insert(position, std::move(p));
    }
    
    void insert(std::initializer_list<value_type> il){
      m_.insert(il);
    }
    
    template<class... Args>
    iterator emplace(Args&&... args){
      return m_.emplace(std::forward<Args>(args)...);
    }
    
    template <class... Args>
    iterator emplace_hint(const_iterator position, Args&&... args){
      return m_.emplace_hint(position, std::forward<Args>(args)...);
    }
    
    NMultimap& add(const Key& k, const T& t){
      m_.insert({k, t});
      return *this;
    }
    
    void merge(const NMultimap& m){
      m_.insert(m.begin(), m.end());
    }
    
    key_compare key_comp() const{
      return m_.key_comp();
    }
    
    iterator lower_bound(const key_type& x){
      return m_.lower_bound(x);
    }
    
    const_iterator lower_bound(const key_type& x) const{
      return m_.lower_bound(x);
    }
    
    size_t max_size() const noexcept{
      return m_.max_size();
    }
    
    NMultimap& operator=(const NMultimap& x){
      m_ = x.m_;
      return *this;
    }
    
    NMultimap& operator=(NMultimap&& x){
      m_ = std::move(x.m_);
      return *this;
    }
    
    NMultimap& operator=(std::initializer_list<value_type> il){
      m_ = il;
      return *this;
    }
    
    reverse_iterator rbegin() noexcept{
      return m_.rbegin();
    }
    
    const_reverse_iterator rbegin() const noexcept{
      return m_.rbegin();
    }
    
    reverse_iterator rend() noexcept{
      return m_.rend();
    }
    
    const_reverse_iterator rend() const noexcept{
      return m_.rend();
    }
    
    const_iterator cbegin() const noexcept{
      return m_.cbegin();
    }
    
    const_iterator cend() const noexcept{
      return m_.cend();
    }
    
    const_reverse_iterator crbegin() const noexcept{
      return m_.crbegin();
    }
    
    const_reverse_iterator crend() const noexcept{
      return m_.crend();
    }
    
    size_t size() const noexcept{
      return m_.size();
    }
    
    void swap(NMultimap& m){
      m_.swap(m.m_);
    }
    
    iterator upper_bound(const key_type& x){
      return m_.upper_bound(x);
    }
    
    const_iterator upper_bound(const key_type& x) const{
      return m_.upper_bound(x);
    }
    
    value_compare value_comp() const{
      return m_.value_comp();
    }
    
    const Map& map() const{
      return m_;
    }
    
    Map& map(){
      return m_;
    }
    
  private:
    Map m_;
  };
  
  template <class Key, class T, class Compare, class Allocator>
  bool operator==(const NMultimap<Key,T,Compare,Allocator>& x,
                  const NMultimap<Key,T,Compare,Allocator>& y){
    return x.map() == y.map();
  }
  
  template <class Key, class T, class Compare, class Allocator>
  bool operator<(const NMultimap<Key,T,Compare,Allocator>& x,
                 const NMultimap<Key,T,Compare,Allocator>& y){
    return x.map() < y.map();
  }
  
  template <class Key, class T, class Compare, class Allocator>
  bool operator!=(const NMultimap<Key,T,Compare,Allocator>& x,
                  const NMultimap<Key,T,Compare,Allocator>& y){
    return x.map() != y.map();
  }
  
  template <class Key, class T, class Compare, class Allocator>
  bool operator>(const NMultimap<Key,T,Compare,Allocator>& x,
                 const NMultimap<Key,T,Compare,Allocator>& y){
    return x.map() > y.map();
  }
  
  template <class Key, class T, class Compare, class Allocator>
  bool operator>=(const NMultimap<Key,T,Compare,Allocator>& x,
                  const NMultimap<Key,T,Compare,Allocator>& y){
    return x.map() >= y.map();
  }
  
  template <class Key, class T, class Compare, class Allocator>
  bool operator<=(const NMultimap<Key,T,Compare,Allocator>& x,
                  const NMultimap<Key,T,Compare,Allocator>& y){
    return x.map() <= y.map();
  }
  
  template<typename K, typename T, typename C>
  std::ostream& operator<<(std::ostream& ostr, const NMultimap<K,T,C>& m){
    typename NMultimap<K,T,C>::const_iterator itr = m.begin();
    ostr << "[| ";
    while(itr != m.end()){
      if(itr != m.begin()){
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

#endif // NEU_N_MULTIMAP_H

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

#include <list>
#include <iterator>
#include <sstream>
#include <iostream>

#include <neu/NError.h>

#ifndef NEU_N_LIST_H
#define NEU_N_LIST_H

namespace neu{
  
  template<typename T, class Allocator = std::allocator<T> >
  class NList{
  public:
    typedef std::list<T, Allocator> List;
    
    typedef T value_type;
    typedef typename List::reference reference;
    typedef typename List::const_reference const_reference;
    typedef typename List::iterator iterator;
    typedef typename List::const_iterator const_iterator;
    typedef typename List::allocator_type allocator_type;
    typedef typename List::reverse_iterator reverse_iterator;
    typedef typename List::const_reverse_iterator const_reverse_iterator;
    
    explicit NList(const Allocator& allocator=Allocator())
    : l_(allocator),
    i_(-1){}
    
    explicit NList(size_t n,
                   const T& value=T(),
                   const Allocator& allocator=Allocator())
    : l_(n, value, allocator),
    i_(-1){}
    
    template<class InputIterator>
    NList(InputIterator first,
          InputIterator last,
          const Allocator& allocator=Allocator())
    : l_(first, last, allocator),
    i_(-1){}
    
    NList(const NList& x)
    : l_(x.l_),
    i_(-1){}
    
    NList(NList&& x)
    : l_(std::move(x.l_)){}
    
    NList(NList&& x, const allocator_type& a)
    : l_(std::move(x), a){}
    
    NList(std::initializer_list<value_type> il)
    : l_(il){}
    
    NList(std::initializer_list<value_type> il, const allocator_type& a)
    : l_(il, a){}
    
    ~NList(){}
    
    const List list() const{
      return l_;
    }
    
    List list(){
      return l_;
    }
    
    template<class InputIterator>
    void assign(InputIterator first, InputIterator last){
      l_.assign(first, last);
      i_ = -1;
    }
    
    void assign(size_t n, const T& u){
      l_.assign(n, u);
      i_ = -1;
    }
    
    reference back(){
      return l_.back();
    }
    
    const_reference back() const{
      return l_.back();
    }
    
    iterator begin() noexcept{
      return l_.begin();
    }
    
    const_iterator begin () const noexcept{
      return l_.begin();
    }
    
    void clear() noexcept{
      l_.clear();
      i_ = -1;
    }
    
    bool empty() const noexcept{
      return l_.empty();
    }
    
    iterator end() noexcept{
      return l_.end();
    }
    
    const_iterator end() const noexcept{
      return l_.end();
    }

    iterator erase(size_t index){
      i_ = -1;
      auto itr = l_.begin();
      advance(itr, index);
      return l_.erase(itr);
    }
    
    iterator erase(iterator position){
      i_ = -1;
      return l_.erase(position);
    }
    
    iterator erase(iterator first, iterator last){
      i_ = -1;
      return l_.erase(first, last);
    }
    
    reference front(){
      return l_.front();
    }
    
    const_reference front() const{
      return l_.front();
    }
    
    allocator_type get_allocator() const noexcept{
      return l_.get_allocator();
    }
    
    iterator insert(iterator position, const T& x){
      i_ = -1;
      return l_.insert(position, x);
    }
    
    iterator insert(size_t index, const T& x){
      i_ = -1;
      auto itr = l_.begin();
      advance(itr, index);
      return l_.insert(itr, x);
    }
    
    void insert(iterator position, size_t n, const T& x){
      i_ = -1;
      return l_.insert(position, n, x);
    }
    
    template <class InputIterator>
    void insert(iterator position, InputIterator first, InputIterator last){
      i_ = -1;
      l_.insert(position, first, last);
    }
    
    void append(const NList& l){
      l_.insert(l_.end(), l.begin(), l.end());
    }
    
    template<class S>
    void append(const NList<S>& l){
      l_.insert(l_.end(), l.begin(), l.end());
    }
    
    size_t max_size() const noexcept{
      return l_.max_size();
    }
    
    void merge(NList& x){
      i_ = -1;
      l_.merge(x.l_);
    }
    
    template<class Compare>
    void merge(NList& x, Compare comp){
      i_ = -1;
      l_.merge(x.l_, comp);
    }
    
    NList& operator=(const NList& x){
      i_ = -1;
      l_ = x.l_;
      return *this;
    }
    
    NList& operator=(NList&& x){
      i_ = -1;
      l_ = std::move(x.l_);
      return *this;
    }
    
    NList<T, Allocator>& operator=(std::initializer_list<value_type> il){
      i_ = -1;
      l_ = il;
      return *this;
    }
    
    void pop_back(){
      l_.pop_back();
    }
    
    T popBack(){
      T ret = std::move(l_.back());
      l_.pop_back();
      return ret;
    }
    
    void pop_front(){
      i_ = -1;
      l_.pop_front();
    }
    
    T popFront(){
      i_ = -1;
      T ret = std::move(l_.front());
      l_.pop_front();
      return ret;
    }
    
    void push_back(const T& x){
      l_.push_back(x);
    }
    
    template <class... Args>
    void emplace_back(Args&&... args){
      return l_.emplace_back(std::forward<Args>(args)...);
    }
    
    void push_front(const T& x){
      i_ = -1;
      l_.push_front(x);
    }
    
    template <class... Args>
    void emplace_front(Args&&... args){
      return l_.emplace_front(std::forward<Args>(args)...);
    }
    
    reverse_iterator rbegin() noexcept{
      return l_.rbegin();
    }
    
    const_reverse_iterator rbegin() const noexcept{
      return l_.rbegin();
    }
    
    void remove(const T& value){
      i_ = -1;
      l_.remove(value);
    }
    
    template <class Predicate>
    void remove_if(Predicate pred){
      i_ = -1;
      l_.remove_if(pred);
    }
    
    reverse_iterator rend() noexcept{
      return l_.rend();
    }
    
    const_reverse_iterator rend() const noexcept{
      return l_.rend();
    }
    
    const_iterator cbegin() const noexcept{
      return l_.cbegin();
    }

    const_iterator cend() const noexcept{
      return l_.cend();
    }

    const_reverse_iterator crbegin() const noexcept{
      return l_.crbegin();
    }

    const_reverse_iterator crend() const noexcept{
      return l_.crend();
    }
    
    void resize(size_t sz, T c=T()){
      i_ = -1;
      l_.resize(sz, c);
    }
    
    void reverse() noexcept{
      l_.reverse();
    }
    
    size_t size() const noexcept{
      return l_.size();
    }
    
    void sort(){
      l_.sort();
    }
    
    template<class Compare>
    void sort(Compare comp){
      l_.sort(comp);
    }
    
    void splice(iterator position, NList& x){
      i_ = -1;
      l_.splice(position, x);
    }
    
    void splice(iterator position, NList& x, iterator i){
      i_ = -1;
      l_.splice(position, x, i);
    }
    
    void splice(iterator position,
                NList& x,
                iterator first,
                iterator last){
      i_ = -1;
      l_.splice(position, x, first, last);
    }
    
    void swap(NList& lst){
      i_ = -1;
      l_.swap(lst);
    }
    
    void unique(){
      i_ = -1;
      l_.unique();
    }
    
    template<class BinaryPredicate>
    void unique(BinaryPredicate binary_pred){
      i_ = -1;
      l_.unique(binary_pred);
    }
    
    NList& operator<<(const T& x){
      l_.push_back(x);
      return *this;
    }
    
    reference operator[](size_t i){
      if(i_ < 0){
        itr_ = l_.begin();
        advance(itr_, i);
      }
      else{
        advance(itr_, i - i_);
      }
      
      i_ = i;
      
      return *itr_;
    }
    
    const_reference operator[](size_t i) const{
      NList* l = const_cast<NList*>(this);
      return l->operator[](i);
    }
    
  private:
    List l_;
    iterator itr_;
    int32_t i_;
  };
  
  template<class T, class Allocator>
  bool operator==(const NList<T,Allocator>& x,
                  const NList<T,Allocator>& y){
    return x.list() == y.list();
  }
  
  template<class T, class Allocator>
  bool operator<(const NList<T,Allocator>& x,
                 const NList<T,Allocator>& y){
    return x.list() < y.list();
  }
  
  template<class T, class Allocator>
  bool operator!=(const NList<T,Allocator>& x,
                  const NList<T,Allocator>& y){
    return x.list() != y.list();
  }
  
  template<class T, class Allocator>
  bool operator>(const NList<T,Allocator>& x,
                 const NList<T,Allocator>& y){
    return x.list() > y.list();
  }
  
  template<class T, class Allocator>
  bool operator>=(const NList<T,Allocator>& x,
                  const NList<T,Allocator>& y){
    return x.list() >= y.list();
  }
  
  template <class T, class Allocator>
  bool operator<=(const NList<T,Allocator>& x,
                  const NList<T,Allocator>& y){
    return x.list() <= y.list();
  }
  
  template<typename T>
  std::ostream& operator<<(std::ostream& ostr, const NList<T>& v){
    typename NList<T>::const_iterator itr = v.begin();
    size_t i = 0;
    bool index = v.size() > 10;
    ostr << "`[";
    while(itr != v.end()){
      if(i > 0){
        ostr << ",";
      }
      
      if(index){
        ostr << i << ":";
      }
      ostr << *itr;
      ++i;
      ++itr;
    }
    ostr << "]";
    return ostr;
  }
  
} // end namespace neu

#endif // NEU_N_LIST_H

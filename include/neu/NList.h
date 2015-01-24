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
    : l_(std::move(x.l_)),
    i_(-1){}
    
    NList(NList&& x, const allocator_type& a)
    : l_(std::move(x), a),
    i_(-1){}
    
    NList(std::initializer_list<value_type> il)
    : l_(il),
    i_(-1){}
    
    NList(std::initializer_list<value_type> il, const allocator_type& a)
    : l_(il, a),
    i_(-1){}
    
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
      reset_();
    }
    
    void assign(size_t n, const T& u){
      l_.assign(n, u);
      reset_();
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
      reset_();
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
      reset_();
      auto itr = l_.begin();
      advance(itr, index);
      return l_.erase(itr);
    }
    
    iterator erase(iterator position){
      reset_();
      return l_.erase(position);
    }
    
    iterator erase(iterator first, iterator last){
      reset_();
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
      reset_();
      return l_.insert(position, x);
    }
    
    iterator insert(size_t index, const T& x){
      reset_();
      auto itr = l_.begin();
      advance(itr, index);
      return l_.insert(itr, x);
    }
    
    void insert(iterator position, size_t n, const T& x){
      reset_();
      return l_.insert(position, n, x);
    }
    
    template <class InputIterator>
    void insert(iterator position, InputIterator first, InputIterator last){
      reset_();
      l_.insert(position, first, last);
    }
    
    void append(const NList& l){
      reset_();
      l_.insert(l_.end(), l.begin(), l.end());
    }
    
    template<class S>
    void append(const NList<S>& l){
      reset_();
      l_.insert(l_.end(), l.begin(), l.end());
    }
    
    size_t max_size() const noexcept{
      return l_.max_size();
    }
    
    void merge(NList& x){
      reset_();
      l_.merge(x.l_);
    }
    
    template<class Compare>
    void merge(NList& x, Compare comp){
      reset_();
      l_.merge(x.l_, comp);
    }
    
    NList& operator=(const NList& x){
      reset_();
      l_ = x.l_;
      return *this;
    }
    
    NList& operator=(NList&& x){
      reset_();
      l_ = std::move(x.l_);
      return *this;
    }
    
    NList<T, Allocator>& operator=(std::initializer_list<value_type> il){
      reset_();
      l_ = il;
      return *this;
    }
    
    void pop_back(){
      reset_();
      l_.pop_back();
    }
    
    T popBack(){
      reset_();
      T ret = std::move(l_.back());
      l_.pop_back();
      return ret;
    }
    
    void pop_front(){
      reset_();
      l_.pop_front();
    }
    
    T popFront(){
      reset_();
      T ret = std::move(l_.front());
      l_.pop_front();
      return ret;
    }
    
    void push_back(const T& x){
      reset_();
      l_.push_back(x);
    }
    
    template <class... Args>
    void emplace_back(Args&&... args){
      reset_();
      return l_.emplace_back(std::forward<Args>(args)...);
    }
    
    void push_front(const T& x){
      reset_();
      l_.push_front(x);
    }
    
    template <class... Args>
    void emplace_front(Args&&... args){
      reset_();
      return l_.emplace_front(std::forward<Args>(args)...);
    }
    
    reverse_iterator rbegin() noexcept{
      return l_.rbegin();
    }
    
    const_reverse_iterator rbegin() const noexcept{
      return l_.rbegin();
    }
    
    void remove(const T& value){
      reset_();
      l_.remove(value);
    }
    
    template <class Predicate>
    void remove_if(Predicate pred){
      reset_();
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
      reset_();
      l_.resize(sz, c);
    }
    
    void reverse() noexcept{
      reset_();
      l_.reverse();
    }
    
    size_t size() const noexcept{
      return l_.size();
    }
    
    void sort(){
      reset_();
      l_.sort();
    }
    
    template<class Compare>
    void sort(Compare comp){
      reset_();
      l_.sort(comp);
    }
    
    void splice(iterator position, NList& x){
      reset_();
      l_.splice(position, x);
    }
    
    void splice(iterator position, NList& x, iterator i){
      reset_();
      l_.splice(position, x, i);
    }
    
    void splice(iterator position,
                NList& x,
                iterator first,
                iterator last){
      reset_();
      l_.splice(position, x, first, last);
    }
    
    void swap(NList& lst){
      reset_();
      l_.swap(lst);
    }
    
    void unique(){
      reset_();
      l_.unique();
    }
    
    template<class BinaryPredicate>
    void unique(BinaryPredicate binary_pred){
      reset_();
      l_.unique(binary_pred);
    }
    
    NList& operator<<(const T& x){
      reset_();
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
      return const_cast<NList*>(this)->operator[](i);
    }
    
    void reset_(){
      reset_();
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

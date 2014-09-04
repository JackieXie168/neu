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
    typedef std::list<T, Allocator> List_;
    
    typedef T value_type;
    typedef typename List_::reference reference;
    typedef typename List_::const_reference const_reference;
    typedef typename List_::iterator iterator;
    typedef typename List_::const_iterator const_iterator;
    typedef typename List_::allocator_type allocator_type;
    typedef typename List_::reverse_iterator reverse_iterator;
    typedef typename List_::const_reverse_iterator const_reverse_iterator;
    
    explicit NList(const Allocator& allocator=Allocator())
    : list_(allocator),
    i_(-1){
      
    }
    
    explicit NList(size_t n,
                   const T& value=T(),
                   const Allocator& allocator=Allocator())
    : list_(n, value, allocator),
    i_(-1){
      
    }
    
    template<class InputIterator>
    NList(InputIterator first,
          InputIterator last,
          const Allocator& allocator=Allocator())
    : list_(first, last, allocator),
    i_(-1){
      
    }
    
    NList(const NList<T, Allocator>& x)
    : list_(x.list_),
    i_(-1){
      
    }
    
    NList(NList&& x)
    : list_(std::move(x.list_)){
      
    }
    
    NList(NList&& x, const allocator_type& a)
    : list_(std::move(x), a){
      
    }
    
    NList(std::initializer_list<value_type> il)
    : list_(il){
      
    }
    
    NList(std::initializer_list<value_type> il, const allocator_type& a)
    : list_(il, a){
      
    }
    
    virtual ~NList(){
      
    }
    
    const List_ list() const{
      return list_;
    }
    
    List_ list(){
      return list_;
    }
    
    template<class InputIterator>
    void assign(InputIterator first, InputIterator last){
      list_.assign(first, last);
      i_ = -1;
    }
    
    void assign(size_t n, const T& u){
      list_.assign(n, u);
      i_ = -1;
    }
    
    reference back(){
      return list_.back();
    }
    
    const_reference back() const{
      return list_.back();
    }
    
    iterator begin(){
      return list_.begin();
    }
    
    const_iterator begin () const{
      return list_.begin();
    }
    
    void clear(){
      list_.clear();
      i_ = -1;
    }
    
    bool empty() const{
      return list_.empty();
    }
    
    iterator end(){
      return list_.end();
    }
    
    const_iterator end() const{
      return list_.end();
    }
    
    iterator erase(iterator position){
      i_ = -1;
      return list_.erase(position);
    }
    
    iterator erase(iterator first, iterator last){
      i_ = -1;
      return list_.erase(first, last);
    }
    
    reference front(){
      return list_.front();
    }
    
    const_reference front() const{
      return list_.front();
    }
    
    allocator_type get_allocator() const{
      return list_.get_allocator();
    }
    
    iterator insert(iterator position, const T& x){
      i_ = -1;
      return list_.insert(position, x);
    }
    
    void insert(iterator position, size_t n, const T& x){
      i_ = -1;
      return list_.insert(position, n, x);
    }
    
    template <class InputIterator>
    void insert(iterator position, InputIterator first, InputIterator last){
      i_ = -1;
      list_.insert(position, first, last);
    }
    
    void append(const NList<T> l){
      list_.insert(list_.end(), l.begin(), l.end());
    }
    
    template<class S>
    void append(const NList<S> l){
      list_.insert(list_.end(), l.begin(), l.end());
    }
    
    size_t max_size() const{
      return list_.max_size();
    }
    
    void merge(NList<T, Allocator>& x){
      i_ = -1;
      list_.merge(x.list_);
    }
    
    template<class Compare>
    void merge(NList<T, Allocator>& x, Compare comp){
      i_ = -1;
      list_.merge(x.list_, comp);
    }
    
    NList<T, Allocator>& operator=(const NList<T, Allocator>& x){
      i_ = -1;
      list_ = x.list_;
      return *this;
    }
    
    NList<T, Allocator>& operator=(NList<T, Allocator>&& x){
      i_ = -1;
      list_ = std::move(x.list_);
      return *this;
    }
    
    NList<T, Allocator>& operator=(std::initializer_list<value_type> il){
      i_ = -1;
      list_ = il;
      return *this;
    }
    
    void pop_back(){
      list_.pop_back();
    }
    
    T popBack(){
      T ret = std::move(*(--list_.end()));
      list_.pop_back();
      return ret;
    }
    
    void pop_front(){
      i_ = -1;
      list_.pop_front();
    }
    
    T popFront(){
      i_ = -1;
      T ret = std::move(*list_.begin());
      list_.pop_front();
      return ret;
    }
    
    void push_back(const T& x){
      list_.push_back(x);
    }
    
    template <class... Args>
    void emplace_back(Args&&... args){
      return list_.emplace_back(std::forward<Args>(args)...);
    }
    
    void push_front(const T& x){
      i_ = -1;
      list_.push_front(x);
    }
    
    template <class... Args>
    void emplace_front(Args&&... args){
      return list_.emplace_front(std::forward<Args>(args)...);
    }
    
    reverse_iterator rbegin(){
      return list_.rbegin();
    }
    
    const_reverse_iterator rbegin() const{
      return list_.rbegin();
    }
    
    void remove(const T& value){
      i_ = -1;
      list_.remove(value);
    }
    
    template <class Predicate>
    void remove_if(Predicate pred){
      i_ = -1;
      list_.remove_if(pred);
    }
    
    reverse_iterator rend(){
      return list_.rend();
    }
    
    const_reverse_iterator rend() const{
      return list_.rend();
    }
    
    void resize(size_t sz, T c=T()){
      i_ = -1;
      list_.resize(sz, c);
    }
    
    void reverse(){
      list_.reverse();
    }
    
    size_t size() const{
      return list_.size();
    }
    
    void sort(){
      list_.sort();
    }
    
    template<class Compare>
    void sort(Compare comp){
      list_.sort(comp);
    }
    
    void splice(iterator position, NList<T, Allocator>& x){
      i_ = -1;
      list_.splice(position, x);
    }
    
    void splice(iterator position, NList<T,Allocator>& x, iterator i){
      i_ = -1;
      list_.splice(position, x, i);
    }
    
    void splice(iterator position,
                NList<T,Allocator>& x,
                iterator first,
                iterator last){
      i_ = -1;
      list_.splice(position, x, first, last);
    }
    
    void swap(NList<T, Allocator>& lst){
      i_ = -1;
      list_.swap(lst);
    }
    
    void unique(){
      i_ = -1;
      list_.unique();
    }
    
    template<class BinaryPredicate>
    void unique(BinaryPredicate binary_pred){
      i_ = -1;
      list_.unique(binary_pred);
    }
    
    NList<T,Allocator>& operator<<(const T& x){
      list_.push_back(x);
      return *this;
    }
    
    reference operator[](size_t i){
      if(i_ < 0){
        itr_ = list_.begin();
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
    List_ list_;
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
    ostr << "(";
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
    ostr << ")";
    return ostr;
  }
  
} // end namespace neu

#endif // NEU_N_LIST_H

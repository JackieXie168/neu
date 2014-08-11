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

#include <vector>
#include <ostream>
#include <algorithm>

#ifndef NEU_N_VECTOR_H
#define NEU_N_VECTOR_H

namespace neu{

  template<typename T, class A=std::allocator<T> >
  class NVector{
  public:
    typedef std::vector<T,A> Vector_;
    
    typedef T value_type;
    typedef typename Vector_::iterator iterator;
    typedef typename Vector_::const_iterator const_iterator;
    typedef typename Vector_::reverse_iterator reverse_iterator;
    typedef typename Vector_::const_reverse_iterator const_reverse_iterator;
    typedef typename Vector_::reference reference;
    typedef typename Vector_::const_reference const_reference;
    typedef typename Vector_::allocator_type allocator_type;
    
    explicit NVector(const A& allocator=A())
    : vec_(allocator){
      
    }
    
    explicit NVector(size_t n,
                     const T& value=T(),
                     const A& allocator=A())
    : vec_(n, value, allocator){
      
    }
    
    template<class InputIterator>
    NVector(InputIterator first,
            InputIterator last,
            const A& allocator=A())
    : vec_(first, last, allocator){
      
    }
    
    NVector(const NVector<T,A>& x)
    : vec_(x.vec_){
      
    }
    
    NVector(NVector&& x)
    : vec_(std::move(x.vec_)){
      
    }

    NVector(std::initializer_list<value_type> il)
      : vec_(il){
      
    }

    NVector(std::initializer_list<value_type> il, const allocator_type& a)
      : vec_(il, a){
      
    }

    ~NVector(){
      
    }
    
    const Vector_& vector() const{
      return vec_;
    }
    
    Vector_& vector(){
      return vec_;
    }
    
    template<class InputIterator>
    void assign(InputIterator first, InputIterator last){
      return vec_.assign(first, last);
    }
    
    void assign(size_t n, const T& u){
      vec_.assign(n, u);
    }
    
    const_reference at(size_t n) const{
      return vec_.at(n);
    }
    
    reference at(size_t n){
      return vec_.at(n);
    }
    
    reference back(){
      return vec_.back();
    }
    
    const_reference back() const{
      return vec_.back();
    }
    
    iterator begin(){
      return vec_.begin();
    }
    
    const_iterator begin() const{
      return vec_.begin();
    }
    
    size_t capacity() const{
      return vec_.capacity();
    }
    
    void clear(){
      vec_.clear();
    }
    
    bool empty() const{
      return vec_.empty();
    }
    
    bool isEmpty() const{
      return vec_.empty();
    }
    
    iterator end(){
      return vec_.end();
    }
    
    const_iterator end() const{
      return vec_.end();
    }
    
    iterator erase(iterator position){
      return vec_.erase(position);
    }
    
    iterator erase(iterator first, iterator last){
      return vec_.erase(first, last);
    }

    iterator erase(size_t index){
      iterator itr = vec_.begin();
      itr += index;
      return vec_.erase(itr);
    }

    reference front(){
      return vec_.front();
    }
    
    const_reference front() const{
      return vec_.front();
    }
    
    allocator_type get_allocator() const{
      return vec_.get_allocator();
    }
    
    iterator insert(iterator position, const T& x){
      return vec_.insert(position, x);
    }
    
    void insert(iterator position, size_t n, const T& x){
      vec_.insert(position, n, x);
    }
    
    void append(const NVector<T>& v){
      vec_.insert(vec_.end(), v.begin(), v.end());
    }
        
    template<class InputIterator>
    void insert(iterator position, InputIterator first, InputIterator last){
      vec_.insert(position, first, last);
    }
    
    size_t max_size() const{
      return vec_.max_size();
    }
    
    NVector<T,A>& operator=(const NVector<T, A>& x){
      vec_.operator=(x.vec_);
      return *this;
    }
    
    NVector<T, A>& operator=(NVector<T, A>&& x){
      vec_.operator=(std::move(x.vec_));

      return *this;
    }

    NVector<T, A>& operator=(std::initializer_list<value_type> il){
      vec_.operator=(il);

      return *this;
    }

    reference operator[](size_t n){
      return vec_.operator[](n);
    }
    
    const_reference operator[](size_t n) const{
      return vec_.operator[](n);
    }
    
    const T& uget(size_t n, const T& def) const{
      if(n >= vec_.size()){
        return def;
      }
      return vec_[n];
    }

    value_type* data() noexcept{
      return vec_.data();
    }

    const value_type* data() const noexcept{
      return vec_.data();
    }

    void pop_back(){
      vec_.pop_back();
    }
    
    T popBack(){
      T ret = std::move(*(--vec_.end()));
      vec_.pop_back();
      return ret;
    }
    
    T popFront(){
      T ret = std::move(*vec_.begin());
      vec_.erase(vec_.begin());
      return ret;
    }
    
    void pop_front(){
      vec_.erase(vec_.begin());
    }
    
    void push_back(const T& x){
      vec_.push_back(x);
    }

    template <class... Args>
    void emplace_back(Args&&... args){
      return vec_.emplace_back(std::forward<Args>(args)...);
    } 

    void pushFront(const T& x){
      vec_.insert(vec_.begin(), x);
    }

    reverse_iterator rbegin(){
      return vec_.rbegin();
    }
    
    const_reverse_iterator rbegin() const{
      return vec_.rbegin();
    }
    
    reverse_iterator rend(){
      return vec_.rend();
    }
    
    const_reverse_iterator rend() const{
      return vec_.rend();
    }
    
    void reserve(size_t n){
      vec_.reserve(n);
    }
    
    void resize(size_t sz, T c=T()){
      vec_.resize(sz, c);
    }
    
    size_t size() const{
      return vec_.size();
    }
    
    void swap(NVector<T,A>& vec){
      vec_.swap(vec);
    }
    
    NVector<T,A> operator-() const{
      NVector<T,A> ret(*this);
      std::transform(ret.begin(), ret.end(), ret.begin(), neg_()); 
      return ret;
    }

    NVector<T,A> operator!() const{
      NVector<T,A> ret(*this);
      std::transform(ret.begin(), ret.end(), ret.begin(), not_()); 
      return ret;
    }
    
    NVector<T,A>& operator+=(const T& x){
      std::transform(vec_.begin(), vec_.end(), vec_.begin(), addBy1_(x)); 
      return *this;
    }
    
    NVector<T,A>& operator+=(const NVector<T,A>& v){
      std::transform(vec_.begin(), vec_.end(), v.begin(), vec_.begin(),
		     addBy2_()); 
      return *this;
    }
        
    NVector<T,A> operator+(const T& x) const{
      NVector<T,A> ret = *this;
      ret += x;
      return ret;
    }
    
    NVector<T,A> operator+(const NVector<T,A>& v) const{
      NVector<T,A> ret = *this;
      ret += v;
      return ret;
    }
    
    NVector<T,A>& operator-=(const T& x){
      std::transform(vec_.begin(), vec_.end(), vec_.begin(), subBy1_(x)); 
      return *this;
    }
    
    NVector<T,A>& operator-=(const NVector<T,A>& v){
      std::transform(vec_.begin(), vec_.end(), v.begin(), vec_.begin(),
		     subBy2_()); 
      return *this;
    }
    
    NVector<T,A> operator-(const T& x) const{
      NVector<T,A> ret = *this;
      ret -= x;
      return ret;
    }
    
    NVector<T,A> operator-(const NVector<T,A>& v) const{
      NVector<T,A> ret = *this;
      ret -= v;
      return ret;
    }
    
    NVector<T,A>& operator*=(const T& x){
      std::transform(vec_.begin(), vec_.end(), vec_.begin(), mulBy1_(x)); 
      return *this;
    }
    
    NVector<T,A>& operator*=(const NVector<T,A>& v){
      std::transform(vec_.begin(), vec_.end(), v.begin(), vec_.begin(),
		     mulBy2_()); 
      return *this;
    }
    
    NVector<T,A> operator*(const T& x) const{
      NVector<T,A> ret = *this;
      ret *= x;
      return ret;
    }
    
    NVector<T,A> operator*(const NVector<T,A>& v) const{
      NVector<T,A> ret = *this;
      ret *= v;
      return ret;
    }
    
    NVector<T,A>& operator/=(const T& x){
      std::transform(vec_.begin(), vec_.end(), vec_.begin(), divBy1_(x)); 
      return *this;
    }
    
    NVector<T,A>& operator/=(const NVector<T,A>& v){
      std::transform(vec_.begin(), vec_.end(), v.begin(), vec_.begin(),
		     divBy2_()); 
      return *this;
    }
    
    NVector<T,A> operator/(const T& x) const{
      NVector<T,A> ret = *this;
      ret /= x;
      return ret;
    }
    
    NVector<T,A> operator/(const NVector<T,A>& v) const{
      NVector<T,A> ret = *this;
      ret /= v;
      return ret;
    }

    NVector<T,A>& operator%=(const T& x){
      std::transform(vec_.begin(), vec_.end(), vec_.begin(), modBy1_(x)); 
      return *this;
    }
    
    NVector<T,A>& operator%=(const NVector<T,A>& v){
      std::transform(vec_.begin(), vec_.end(), v.begin(), vec_.begin(),
		     modBy2_()); 
      return *this;
    }

    NVector<T,A> operator%(const T& x) const{
      NVector<T,A> ret = *this;
      ret %= x;
      return ret;
    }
    
    NVector<T,A> operator%(const NVector<T,A>& v) const{
      NVector<T,A> ret = *this;
      ret %= v;
      return ret;
    }

    NVector<T,A>& andBy(const T& x){
      std::transform(vec_.begin(), vec_.end(), vec_.begin(), andBy1_(x)); 
      return *this;
    }
    
    NVector<T,A>& andBy(const NVector<T,A>& v){
      std::transform(vec_.begin(), vec_.end(), v.begin(), vec_.begin(),
		     andBy2_()); 
      return *this;
    }

    NVector<T,A> operator&&(const T& x) const{
      NVector<T,A> ret = *this;
      ret.andBy(x);
      return ret;
    }
    
    NVector<T,A> operator&&(const NVector<T,A>& v) const{
      NVector<T,A> ret = *this;
      ret.andBy(v);
      return ret;
    }    

    NVector<T,A>& orBy(const T& x){
      std::transform(vec_.begin(), vec_.end(), vec_.begin(), orBy1_(x)); 
      return *this;
    }
    
    NVector<T,A>& orBy(const NVector<T,A>& v){
      std::transform(vec_.begin(), vec_.end(), v.begin(), vec_.begin(),
		     orBy2_()); 
      return *this;
    }

    NVector<T,A> operator||(const T& x) const{
      NVector<T,A> ret = *this;
      ret.orBy(x);
      return ret;
    }
    
    NVector<T,A> operator||(const NVector<T,A>& v) const{
      NVector<T,A> ret = *this;
      ret.orBy(v);
      return ret;
    }   

    NVector<T,A>& operator<<(const T& x){
      vec_.push_back(x);
      return *this;
    }
    
  private:
    Vector_ vec_;
    
    class neg_ : public std::unary_function<T,T>{
    public:
      T& operator()(T& x){
        x = -x;
        return x;
      }
    };

    class not_ : public std::unary_function<T,T>{
    public:
      T& operator()(T& x){
        x = !x;
        return x;
      }
    };
    
    class addBy1_ : public std::unary_function<T,T>{
    public:
      addBy1_(const T& v) : v_(v){}
      
      T& operator()(T& x){
        return x += v_;
      }
    private:
      const T& v_;
    };
    
    class addBy2_ : public std::binary_function<T,T,T>{
    public:
      T& operator()(T& x, const T& y){
        return x += y;
      }
    };
    
    class subBy1_ : public std::unary_function<T,T>{
    public:
      subBy1_(const T& v) : v_(v){}
      
      T& operator()(T& x){
        return x -= v_;
      }
    private:
      const T& v_;
    };
    
    class subBy2_ : public std::binary_function<T,T,T>{
    public:
      T& operator()(T& x, const T& y){
        return x -= y;
      }
    };
    
    class mulBy1_ : public std::unary_function<T,T>{
    public:
      mulBy1_(const T& v) : v_(v){}
      
      T& operator()(T& x){
        return x *= v_;
      }
    private:
      const T& v_;
    };
    
    class mulBy2_ : public std::binary_function<T,T,T>{
    public:
      T& operator()(T& x, const T& y){
        return x *= y;
      }
    };
    
    class divBy1_ : public std::unary_function<T,T>{
    public:
      divBy1_(const T& v) : v_(v){}
      
      T& operator()(T& x){
        return x /= v_;
      }
    private:
      const T& v_;
    };
    
    class divBy2_ : public std::binary_function<T,T,T>{
    public:
      T& operator()(T& x, const T& y){
        return x /= y;
      }
    };
    
    class modBy1_ : public std::unary_function<T,T>{
    public:
      modBy1_(const T& v) : v_(v){}
      
      T& operator()(T& x){
        return x %= v_;
      }
    private:
      const T& v_;
    };
    
    class modBy2_ : public std::binary_function<T,T,T>{
    public:
      T& operator()(T& x, const T& y){
        return x %= y;
      }
    };

    class andBy1_ : public std::unary_function<T,T>{
    public:
      andBy1_(const T& v) : v_(v){}
      
      T& operator()(T& x){
        return x = x && v_;
      }
    private:
      const T& v_;
    };
    
    class andBy2_ : public std::binary_function<T,T,T>{
    public:
      T& operator()(T& x, const T& y){
        return x = x && y;
      }
    };

    class orBy1_ : public std::unary_function<T,T>{
    public:
      orBy1_(const T& v) : v_(v){}
      
      T& operator()(T& x){
        return x = x || v_;
      }
    private:
      const T& v_;
    };
    
    class orBy2_ : public std::binary_function<T,T,T>{
    public:
      T& operator()(T& x, const T& y){
        return x = x || y;
      }
    };
    
  };
  
  template<class T, class Allocator>
  bool operator==(const NVector<T,Allocator>& x,
                  const NVector<T,Allocator>& y){
    return x.vector() == y.vector();
  }
  
  template<class T, class Allocator>
  bool operator!=(const NVector<T,Allocator>& x,
                  const NVector<T,Allocator>& y){
    return x.vector() != y.vector();
  }
  
  template<class T, class Allocator>
  bool operator<(const NVector<T,Allocator>& x,
                 const NVector<T,Allocator>& y){
    return x.vector() < y.vector();
  }
  
  template<class T, class Allocator>
  bool operator>(const NVector<T,Allocator>& x,
                 const NVector<T,Allocator>& y){
    return x.vector() > y.vector();
  }
  
  template<class T, class Allocator>
  bool operator>=(const NVector<T,Allocator>& x,
                  const NVector<T,Allocator>& y){
    return x.vector() >= y.vector();
  }
  
  template <class T, class Allocator>
  bool operator<=(const NVector<T,Allocator>& x,
                  const NVector<T,Allocator>& y){
    return x.vector() <= y.vector();
  }
  
  template<typename T>
  std::ostream& operator<<(std::ostream& ostr, const NVector<T>& v){
    typename NVector<T>::const_iterator itr = v.begin();
    size_t i = 0;
    bool index = v.size() > 10;
    ostr << "[";
    while(itr != v.end()){
      if(i > 0){
        ostr << ",";
        if(index){
          ostr << " ";
        }
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

#endif // NEU_N_VECTOR_H

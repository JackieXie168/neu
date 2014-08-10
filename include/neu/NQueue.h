#ifndef NEU_N_QueueH
#define NEU_N_QueueH

#include <deque>

namespace neu{
  
  template <class T, class Allocator=std::allocator<T>>
  class NQueue{
  public:
    typedef std::deque<T, Allocator> Queue;
    
    typedef T value_type;
    typedef Allocator allocator_type;
    
    typedef typename allocator_type::reference reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename Queue::iterator  iterator;
    typedef typename Queue::const_iterator const_iterator;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::difference_type difference_type;
    
    typedef typename allocator_type::pointer pointer;
    typedef typename allocator_type::const_pointer const_pointer;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    
    NQueue()
    noexcept(std::is_nothrow_default_constructible<allocator_type>::value){}
    
    explicit NQueue(const allocator_type& a)
    : q_(a){}
    
    explicit NQueue(size_type n)
    : q_(n){}
    
    NQueue(size_type n, const value_type& v)
    : q_(n, v){}
    
    NQueue(size_type n, const value_type& v, const allocator_type& a)
    : q_(n, v, a){}
    
    template<class InputIterator>
    NQueue(InputIterator f, InputIterator l)
    : q_(f, l){}
    
    template<class InputIterator>
    NQueue(InputIterator f, InputIterator l, const allocator_type& a)
    : q_(f, l, a){}
    
    NQueue(const NQueue& c)
    : q_(c.q_){}
    
    NQueue(const Queue& c)
    : q_(c){}
    
    NQueue(NQueue&& c)
    noexcept(std::is_nothrow_move_constructible<allocator_type>::value)
    : q_(std::move(c.q_)){}
    
    NQueue(Queue&& c)
    noexcept(std::is_nothrow_move_constructible<allocator_type>::value)
    : q_(std::move(c)){}
    
    NQueue(std::initializer_list<value_type> il,
          const Allocator& a = allocator_type())
    : q_(il, a){}
    
    NQueue(const NQueue& c, const allocator_type& a)
    : q_(c.q_, a){}
    
    NQueue(NQueue&& c, const allocator_type& a)
    : q_(std::move(c), a){}
    
    NQueue(const Queue& c, const allocator_type& a)
    : q_(c, a){}
    
    NQueue(Queue&& c, const allocator_type& a)
    : q_(std::move(c), a){}
    
    ~NQueue(){}
    
    NQueue& operator=(const NQueue& c){
      q_ = c.q_;
      return *this;
    }
    
    NQueue& operator=(NQueue&& c)
    noexcept(allocator_type::propagate_on_container_move_assignment::value &&
             std::is_nothrow_move_assignable<allocator_type>::value){
      q_ = std::move(c.q_);
      return *this;
    }
    
    NQueue& operator=(const Queue& c){
      q_ = c;
      return *this;
    }
    
    NQueue& operator=(Queue&& c)
    noexcept(allocator_type::propagate_on_container_move_assignment::value &&
             std::is_nothrow_move_assignable<allocator_type>::value){
      q_ = std::move(c);
      return *this;
    }
    
    NQueue& operator=(std::initializer_list<value_type> il){
      q_ = il;
      return *this;
    }
    
    template <class InputIterator>
    void assign(InputIterator f, InputIterator l){
      q_.assign(f, l);
    }
    
    void assign(size_type n, const value_type& v){
      q_.assign(n, v);
    }
    
    void assign(std::initializer_list<value_type> il){
      q_.assign(il);
    }
    
    allocator_type get_allocator() const noexcept{
      return q_.get_allocator();
    }
    
    iterator begin() noexcept{
      return q_.begin();
    }
    
    const_iterator begin() const noexcept{
      return q_.begin();
    }
    
    iterator end() noexcept{
      return q_.end();
    }
    
    const_iterator end() const noexcept{
      return q_.end();
    }
    
    reverse_iterator rbegin() noexcept{
      return q_.rbegin();
    }
    
    const_reverse_iterator rbegin() const noexcept{
      return q_.rbegin();
    }
    
    reverse_iterator rend() noexcept{
      return q_.rend();
    }
    
    const_reverse_iterator rend() const noexcept{
      return q_.rend();
    }
    
    const_iterator cbegin() const noexcept{
      return q_.cbegin();
    }
    
    const_iterator cend() const noexcept{
      return q_.cend();
    }
    
    const_reverse_iterator crbegin() const noexcept{
      return q_.crbegin();
    }
    
    const_reverse_iterator crend() const noexcept{
      return q_.crend();
    }
    
    size_type size() const noexcept{
      return q_.size();
    }
    
    size_type max_size() const noexcept{
      return q_.max_size();
    }
    
    void resize(size_type n){
      q_.resize();
    }
    
    void resize(size_type n, const value_type& v){
      q_.resize(n, v);
    }
    
    void shrink_to_fit(){
      return q_.shrink_to_fit();
    }
    
    bool empty() const noexcept{
      return q_.empty();
    }
    
    reference operator[](size_type i){
      return q_[i];
    }
    
    const_reference operator[](size_type i) const{
      return q_[i];
    }
    
    reference at(size_type i){
      return q_.at(i);
    }
    
    const_reference at(size_type i) const{
      return q_.at(i);
    }
    
    reference front(){
      return q_.front();
    }
    
    const_reference front() const{
      return q_.front();
    }
    
    reference back(){
      return q_.back();
    }
    
    const_reference back() const{
      return q_.back();
    }
    
    void push_front(const value_type& v){
      q_.push_front(v);
    }
    
    void push_front(value_type&& v){
      q_.push_front(std::move(v));
    }
    
    void push_back(const value_type& v){
      q_.push_back(v);
    }
    
    void push_back(value_type&& v){
      q_.push_back(std::move(v));
    }
    
    NQueue& operator<<(const value_type& v){
      q_.push_back(v);
      return *this;
    }
    
    NQueue& operator<<(value_type&& v){
      q_.push_back(std::move(v));
      return *this;
    }
    
    template<class... Args>
    void emplace_front(Args&&... args){
      return q_.emplace_front(std::forward<Args>(args)...);
    }
    
    template<class... Args>
    void emplace_back(Args&&... args){
      return q_.emplace_back(std::forward<Args>(args)...);
    }
    
    template<class... Args>
    iterator emplace(const_iterator p, Args&&... args){
      return q_.emplace(p, std::forward<Args>(args)...);
    }
    
    iterator insert(const_iterator p, const value_type& v){
      return q_.insert(p, v);
    }
    
    iterator insert(const_iterator p, value_type&& v){
      return q_.insert(p, std::move(v));
    }
    
    iterator insert(const_iterator p, size_type n, const value_type& v){
      return q_.insert(p, n, v);
    }
    
    template<class InputIterator>
    iterator insert(const_iterator p, InputIterator f, InputIterator l){
      return q_.insert(p, f, l);
    }
    
    iterator insert(const_iterator p, std::initializer_list<value_type> il){
      return q_.insert(p, il);
    }
    
    void pop_front(){
      q_.pop_front();
    }

    value_type popFront(){
      T ret = std::move(*(q_.begin()));
      q_.pop_back();
      return ret;
    }
    
    void pop_back(){
      q_.pop_back();
    }
    
    value_type popBack(){
      T ret = std::move(*(--q_.end()));
      q_.pop_back();
      return ret;
    }
    
    iterator erase(const_iterator p){
      return q_.erase(p);
    }
    
    iterator erase(const_iterator f, const_iterator l){
      return q_.erase(f, l);
    }
    
    void swap(NQueue& c)
    noexcept(!allocator_type::propagate_on_container_swap::value ||
             std::__is_nothrow_swappable<allocator_type>::value){
      q_.swap(c.q_);
    }
    
    void clear() noexcept{
      q_.clear();
    }
    
    Queue& queue(){
      return q_;
    }
    
    const Queue& queue() const{
      return q_;
    }
    
  private:
    Queue q_;
  };
  
  template <class T, class Allocator>
  bool operator==(const NQueue<T,Allocator>& x, const NQueue<T,Allocator>& y){
    return x.queue() == y.queue();
  }
  
  template <class T, class Allocator>
  bool operator<(const NQueue<T,Allocator>& x, const NQueue<T,Allocator>& y){
    return x.queue() < y.queue();
  }
  
  template <class T, class Allocator>
  bool operator!=(const NQueue<T,Allocator>& x, const NQueue<T,Allocator>& y){
    return x.queue() != y.queue();
  }
  
  template <class T, class Allocator>
  bool operator>(const NQueue<T,Allocator>& x, const NQueue<T,Allocator>& y){
    return x.queue() > y.queue();
  }
  
  template <class T, class Allocator>
  bool operator>=(const NQueue<T,Allocator>& x, const NQueue<T,Allocator>& y){
    return x.queue() >= y.queue();
  }
  
  template <class T, class Allocator>
  bool operator<=(const NQueue<T,Allocator>& x, const NQueue<T,Allocator>& y){
    return x.queue() <= y.queue();
  }
  
  template <class T, class Allocator>
  void swap(NQueue<T,Allocator>& x, NQueue<T,Allocator>& y)
  noexcept(noexcept(x.swap(y))){
    return swap(x.queue(), y.queue());
  }
  
  template <class T, class Allocator>
  std::ostream& operator<<(std::ostream& ostr,
                           const NQueue<T, Allocator>& q){
    typename NQueue<T, Allocator>::const_iterator itr = q.begin();
    ostr << "[[";
    bool first = true;
    while(itr != q.end()){
      if(first){
        first = false;
      }
      else{
        ostr << ", ";
      }
      
      ostr << *itr;
      ++itr;
    }
    ostr << "]]";
    return ostr;
  }
  
} // end namespace neu

#endif // NEU_N_QueueH

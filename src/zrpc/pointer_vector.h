// Copyright 2011, Nadav Samet.
// All rights reserved.
//
// Author: thesamet@gmail.com <Nadav Samet>

#ifndef ZRPC_POINTER_VECTOR_H
#define ZRPC_POINTER_VECTOR_H

#include <vector>
#include "zrpc/macros.h"

namespace zrpc {

template<typename T>
class PointerVector {
 public:
  typedef T* value;
  typedef T* &reference;
  typedef T* const& const_reference;
  typedef typename std::vector<T*>::const_iterator const_iterator;
  typedef typename std::vector<T*>::iterator iterator;

  PointerVector() {};

  ~PointerVector() { resize(0); }

  inline size_t size() const { return data_.size(); };

  void push_back(T* value) { data_.push_back(value); }

  void resize(size_t sz) {
    for (size_t i = sz; i < size(); ++i) {
      delete data_[i];
    }
    data_.resize(sz);
  }

  inline iterator begin() {
    return data_.begin();
  }

  inline iterator end() {
    return data_.end();
  }

  inline const_iterator begin() const {
    return data_.begin();
  }

  inline const_iterator end() const {
    return data_.end();
  }

  const_reference at(size_t index) const {
    return data_.at(index);
  }

  const_reference at(size_t index) {
    return data_.at(index);
  }

  const_reference operator[](size_t index) const {
    return at(index);
  }

  const_reference operator[](size_t index) {
    return at(index);
  }

  void swap(PointerVector<T>& other) {
    data_.swap(other.data_);
  }

  inline void erase(iterator first, iterator last) {
    for (iterator i = first; i != last; ++i) {
      delete *i;
    }
    data_.erase(first, last);
  }

  inline void erase(size_t index) {
    delete data_[index];
    data_.erase(data_.begin() + index);
  }

 private:
  std::vector<T*> data_;
  DISALLOW_COPY_AND_ASSIGN(PointerVector);
};
}  // namespace
#endif

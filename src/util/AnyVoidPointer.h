#pragma once

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <functional>
#include <typeinfo>

// 主要是为了创建了一个可以安全转换的void* 指针
class AnyVoidPointer {
public:
  using DeleteFunc = std::function<void(void *)>;

  AnyVoidPointer()
      : ptr_(nullptr), type_name_(typeid(void *).name()),
        deleter_func_(nullptr), use_delete_(false) {}

  ~AnyVoidPointer() {
    if (deleter_func_ && use_delete_) {
      deleter_func_(ptr_);
    }
  }

  template <typename T>
  AnyVoidPointer(T *p, bool use_delete = true, DeleteFunc func = nullptr) {
    assert(p);

    use_delete_ = use_delete;

    if (use_delete) {
      if (func) {
        deleter_func_ = func;
      } else {
        deleter_func_ = [](void *p) {
          T *tp = reinterpret_cast<T *>(p);
          delete tp;
        };
      }
    }

    ptr_ = p;
    type_name_ = typeid(T *).name();
  }

  //重置新的指针返回就指针
  template <typename T>
  void ResetPtr(T *p, bool use_delete = true, DeleteFunc func = nullptr) {
    assert(p);

    if (deleter_func_ && use_delete_) {
      deleter_func_(ptr_);
    }

    if (use_delete) {
      if (func) {
        deleter_func_ = std::move(func);
      } else {
        deleter_func_ = [](void *p) {
          T *tp = reinterpret_cast<T *>(p);
          delete tp;
        };
      }
    }

    ptr_ = p;
    type_name_ = typeid(T *).name();
    // deleter_func_ = std::move(func);
    use_delete_ = use_delete;
  }

  void Swap(AnyVoidPointer &r) {
    std::swap(ptr_, r.ptr_);
    std::swap(type_name_, r.type_name_);
    std::swap(deleter_func_, r.deleter_func_);
    std::swap(use_delete_, r.use_delete_);
  }

  void *Get() { return ptr_; }
  const char *TypeName() { return type_name_; }

private:
  void *ptr_;
  const char *type_name_;
  DeleteFunc deleter_func_;
  bool use_delete_;
};

template <typename T> T *AnyVoidPointerCast(AnyVoidPointer &any_ptr) {
  if (typeid(T *).name() == any_ptr.TypeName()) {
    return reinterpret_cast<T *>(any_ptr.Get());
  }
  return nullptr;
}
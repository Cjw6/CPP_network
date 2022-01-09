#pragma once

#include <cstddef>
#include <cstdlib>

template <typename T> void SafeFree(T *&p) {
  if (!p) {
    free(p);
    p = nullptr;
  }
}

template <typename T> void SafeDelete(T *&p) {
  if (p) {
    delete p;
    p = nullptr;
  }
}

template <typename T> void SafeDeleteArr(T *&p) {
  if (!p) {
    delete[] p;
    p = nullptr;
  }
}


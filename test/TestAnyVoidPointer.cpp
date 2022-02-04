#include "gtest/gtest.h"

#include "util/AnyVoidPointer.h"

class C {
public:
  C() { i = 0; };
  ~C() {
    printf("destruct C\n");
    i = 1;
  }
  static int i;
};

int C::i = 0;

class D {
public:
  D() { i = 0; };
  ~D() {
    printf("destruct D\n");
    i = 1;
  }
  static int i;
};

int D::i = 0;

TEST(AnyVoidPointer, 1) {
  {
    AnyVoidPointer p;
    p.ResetPtr(new C);
    p.ResetPtr(new D);

    EXPECT_EQ(p.TypeName(), typeid(D *).name()) << "eg";
  }
  EXPECT_EQ(C::i, 1);
  // EXPECT_EQ(1, 0);
}

TEST(AnyVoidPointer, 2) {
  int d = 0;
  AnyVoidPointer p(new C, true, [&](void *p) {
    d = 1;
    C *pc = reinterpret_cast<C *>(p);
    delete pc;
  });

  C *pc = AnyVoidPointerCast<C>(p);
  EXPECT_EQ(typeid(pc), typeid(C *));
  D *pd = AnyVoidPointerCast<D>(p);
  EXPECT_TRUE(!pd);
}
#include <gtest/gtest.h>
#include <util/Skiplist.h>

TEST(skiplists, 1) {
  srand((unsigned)time(0));

  // create SkipList object with MAXLVL and P
  SkipList lst(3, 0.5);

  lst.insertElement(3);
  lst.insertElement(6);
  lst.insertElement(7);
  lst.insertElement(9);
  lst.insertElement(12);
  lst.insertElement(19);
  lst.insertElement(17);
  lst.insertElement(26);
  lst.insertElement(21);
  lst.insertElement(25);
  lst.displayList();

  // Search for node 19
  lst.searchElement(19);

  // Delete node 19
  lst.deleteElement(19);
  lst.displayList();
}
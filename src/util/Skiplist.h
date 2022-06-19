#ifndef CJ_UTIL_SKIPLIPLIST
#define CJ_UTIL_SKIPLIPLIST

class SkipListNode {
public:
  int key;

  // Array to hold pointers to node of different level
  SkipListNode **forward;
  SkipListNode(int, int);
};

class SkipList {
  // Maximum lel for this skip list
  int MAXLVL;

  // P is the fraction of the nodes with level
  // i pointers also having level i+1 pointers
  float P;

  // current level of skip list
  int level;

  // pointer to header node
  SkipListNode *header;

public:
  SkipList(int, float);
  int randomLevel();
  SkipListNode *createNode(int, int);
  void insertElement(int);
  void deleteElement(int);
  void searchElement(int);
  void displayList();
};

#endif
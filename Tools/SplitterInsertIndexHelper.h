#ifndef SPLITTERINSERTINDEXHELPER_H
#define SPLITTERINSERTINDEXHELPER_H

class SplitterInsertIndexHelper {
 public:
  SplitterInsertIndexHelper(int maxCount);
  ~SplitterInsertIndexHelper();
  int operator()(int sequence);  //  should insert at
  int GetOccupiedCnt() const;

 private:
  bool* m_occupied;
  const int m_maxCount;
};

#endif  // SPLITTERINSERTINDEXHELPER_H

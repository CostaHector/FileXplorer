#ifndef UNDOREDO_H
#define UNDOREDO_H
#include <QStack>
#include "FileOperatorPub.h"

class UndoRedo {
 public:
  static UndoRedo& GetInst();
  bool Do(const FileOperatorType::BATCH_COMMAND_LIST_TYPE& cmd);
  static bool on_Undo();
  static bool on_Redo();

 private:
  UndoRedo() = default;
  inline bool undoAvailable() const { return !mUndoStk.isEmpty(); }
  inline bool redoAvailable() const { return !mRedoStk.isEmpty(); }
  bool Undo();
  bool Redo();
#ifdef RUNNING_UNIT_TESTS
  inline void clear() {
    decltype(mUndoStk) emptyUndoStk;
    mUndoStk.swap(emptyUndoStk);
    decltype(mUndoStk) emptyRedoStk;
    mRedoStk.swap(emptyRedoStk);
  }
#endif
  QStack<FileOperatorType::BATCH_COMMAND_LIST_TYPE> mUndoStk, mRedoStk;
};
#endif  // UNDOREDO_H

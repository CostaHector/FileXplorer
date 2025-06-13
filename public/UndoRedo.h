#ifndef UNDOREDO_H
#define UNDOREDO_H
#include <QStack>
#include "FileOperation/FileOperatorPub.h"

class UndoRedo {
 public:
  bool Do(const FileOperatorType::BATCH_COMMAND_LIST_TYPE& cmd);
  inline bool undoAvailable() const { return !mUndoStk.isEmpty(); }
  inline bool redoAvailable() const { return !mRedoStk.isEmpty(); }
  static bool on_Undo();
  static bool on_Redo();

 private:
  bool Undo();
  bool Redo();
  inline void clear() {
    decltype(mUndoStk) emptyUndoStk;
    mUndoStk.swap(emptyUndoStk);
    decltype(mUndoStk) emptyRedoStk;
    mRedoStk.swap(emptyRedoStk);
  }
  QStack<FileOperatorType::BATCH_COMMAND_LIST_TYPE> mUndoStk, mRedoStk;
};

extern UndoRedo g_undoRedo;
#endif  // UNDOREDO_H

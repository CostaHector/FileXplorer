#ifndef UNDOREDO_H
#define UNDOREDO_H
#include <QPair>
#include <QStack>
#include "FileOperation/FileOperatorPub.h"
#include <utility>


class UndoRedo {
 public:
  bool Do(const FileOperatorType::BATCH_COMMAND_LIST_TYPE& cmd);

  bool Undo();

  bool Redo();

  inline bool undoAvailable() { return !undoList.isEmpty(); }

  inline bool redoAvailable() { return !redoList.isEmpty(); }

  inline void clear() {
    decltype(undoList) emptyUndoList;
    undoList.swap(emptyUndoList);
    decltype(undoList) emptyRedoList;
    redoList.swap(emptyRedoList);
  }

  static bool on_Undo();
  static bool on_Redo();

  QStack<FileOperatorType::BATCH_COMMAND_LIST_TYPE> undoList, redoList;
};

extern UndoRedo g_undoRedo;
#endif  // UNDOREDO_H

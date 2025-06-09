#ifndef UNDOREDO_H
#define UNDOREDO_H
#include <QPair>
#include <QStack>
#include "FileOperation/FileOperatorPub.h"
#include <utility>

struct OperationStream {
  FileOperatorType::BATCH_COMMAND_LIST_TYPE doCmd;
  FileOperatorType::BATCH_COMMAND_LIST_TYPE recoverCmd;
};

class UndoRedo {
 public:
  using UNDO_REDO_RETURN = std::pair<bool, OperationStream>;

  bool Do(const FileOperatorType::BATCH_COMMAND_LIST_TYPE& cmd);

  UNDO_REDO_RETURN Undo();

  UNDO_REDO_RETURN Redo();

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

  QStack<OperationStream> undoList;
  QStack<OperationStream> redoList;
};

extern UndoRedo g_undoRedo;
#endif  // UNDOREDO_H

#include "UndoRedo.h"
UndoRedo g_undoRedo;

bool UndoRedo::on_Undo(){
  if (not g_undoRedo.undoAvailable()) {
    qInfo("[skip] Nothing to undo");
    return true;
  }
  const bool isAllSucceed = g_undoRedo.Undo().first;
  const char* undoMsg = isAllSucceed ? "All undo succeed" : "Some undo failed.";
  qDebug("%s", undoMsg);
  return isAllSucceed;
}

bool UndoRedo::on_Redo() {
  if (not g_undoRedo.redoAvailable()) {
    qInfo("[skip] Nothing to redo");
    return true;
  }
  const bool isAllSucceed = g_undoRedo.Redo().first;
  const char* redoMsg = isAllSucceed ? "All redo succeed" : "Some redo failed.";
  qDebug("%s", redoMsg);
  return isAllSucceed;
}

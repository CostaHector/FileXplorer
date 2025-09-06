#ifndef PATHUNDOREDOER_H
#define PATHUNDOREDOER_H

#include <QStack>

class PathUndoRedoer {
public:
  PathUndoRedoer() = default;

  bool operator()(const QString& pth) {
    undoStack.append(pth);
    return true;
  }

  bool undoAvailable() { return undoStack.size() > 1; }

  QString undo() {
    if (!undoAvailable()) {
      return "";
    }
    redoStack.append(undoStack.pop());
    return undoStack.top();
  }

  bool redoAvailable() { return !redoStack.isEmpty(); }

  QString redo() {
    if (!redoAvailable()) {
      return "";
    }
    const QString& redoPath = redoStack.pop();
    undoStack.append(redoPath);
    return redoPath;
  }

private:
  QStack<QString> undoStack;
  QStack<QString> redoStack;
};

#endif  // PATHUNDOREDOER_H

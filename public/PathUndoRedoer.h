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

  bool undoPathAvailable() { return undoStack.size() > 1; }
  bool redoPathAvailable() { return !redoStack.isEmpty(); }

  QString onUndoPath() {
    if (!undoPathAvailable()) {
      return "";
    }
    redoStack.append(undoStack.pop());
    return undoStack.top();
  }

  QString onRedoPath() {
    if (!redoPathAvailable()) {
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

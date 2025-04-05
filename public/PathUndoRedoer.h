#ifndef PATHUNDOREDOER_H
#define PATHUNDOREDOER_H

#include <QStack>

class PathUndoRedoer {
 public:
  PathUndoRedoer() = default;

  auto operator()(const QString& pth) -> bool {
    undoStack.append(pth);

    return true;
  }

  auto undoAvailable() -> bool { return undoStack.size() > 1; }

  auto undo() -> QString {
    if (not undoAvailable()) {
      return "";
    }
    redoStack.append(undoStack.pop());
    return undoStack.top();
  }

  auto redoAvailable() -> bool { return not redoStack.isEmpty(); }

  auto redo() -> QString {
    if (not redoAvailable()) {
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

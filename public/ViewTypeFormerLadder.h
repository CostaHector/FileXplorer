#ifndef VIEWTYPEFORMERLADDER_H
#define VIEWTYPEFORMERLADDER_H

#include <QStack>
#include "ViewTypeTool.h"

class ViewTypeFormerLadder {
public:
  ViewTypeFormerLadder() {
    operator()(ViewTypeTool::DEFAULT_VIEW_TYPE);
  }

  bool operator()(const ViewTypeTool::ViewType& vt) {
    if (!undoStack.isEmpty() && undoStack.top() == vt) { // same viewType
      qDebug("[Skip] Duplicate ViewType[%s]", ViewTypeTool::c_str(vt));
      return false;
    }
    undoStack.append(vt);
    return true;
  }

  bool undoAvailable() { return undoStack.size() > 1; }

  ViewTypeTool::ViewType undo() {
    if (!undoAvailable()) {
      return ViewTypeTool::DEFAULT_VIEW_TYPE;
    }
    redoStack.append(undoStack.pop());
    return undoStack.top();
  }

  bool redoAvailable() { return !redoStack.isEmpty(); }

  ViewTypeTool::ViewType redo() {
    if (!redoAvailable()) {
      return ViewTypeTool::DEFAULT_VIEW_TYPE;
    }
    const ViewTypeTool::ViewType& redoPath = redoStack.pop();
    undoStack.append(redoPath);
    return redoPath;
  }

private:
  QStack<ViewTypeTool::ViewType> undoStack;
  QStack<ViewTypeTool::ViewType> redoStack;
};

#endif // VIEWTYPEFORMERLADDER_H

#ifndef VIEWTYPEHISTORY_H
#define VIEWTYPEHISTORY_H

#include <QStack>
#include "ViewTypeTool.h"

class ViewTypeHistory {
 public:
  ViewTypeHistory() { operator()(ViewTypeTool::DEFAULT_VIEW_TYPE); }

  bool operator()(const ViewTypeTool::ViewType& vt) {
    if (!undoStack.isEmpty() && undoStack.top() == vt) {  // same viewType
      LOG_D("[Skip] Duplicate ViewType[%s]", ViewTypeTool::c_str(vt));
      return false;
    }
    undoStack.append(vt);
    return true;
  }

  bool undoViewAvailable() { return undoStack.size() > 1; }
  bool redoViewAvailable() { return !redoStack.isEmpty(); }

  ViewTypeTool::ViewType undo() {
    if (!undoViewAvailable()) {
      return ViewTypeTool::DEFAULT_VIEW_TYPE;
    }
    redoStack.append(undoStack.pop());
    return undoStack.top();
  }

  ViewTypeTool::ViewType redo() {
    if (!redoViewAvailable()) {
      return ViewTypeTool::DEFAULT_VIEW_TYPE;
    }
    ViewTypeTool::ViewType redoPath = redoStack.pop();
    undoStack.append(redoPath);
    return redoPath;
  }

 private:
  QStack<ViewTypeTool::ViewType> undoStack;
  QStack<ViewTypeTool::ViewType> redoStack;
};

#endif  // VIEWTYPEHISTORY_H

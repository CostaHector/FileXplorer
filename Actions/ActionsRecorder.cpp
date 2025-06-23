#include "ActionsRecorder.h"
#include <QToolButton>
#include <stack>
#include <utility>
#include <QMenuBar>
using std::pair;
using std::stack;

int ActionsRecorder::FromToolbar(QToolBar* tb) {
  if (tb == nullptr) {
    return 0;
  }
  QMap<QString, QAction*> actionMap;

  stack<pair<QToolBar*, QString>> stack;
  stack.emplace(tb, tb->windowTitle());
  while (!stack.empty()) {
    auto currentPr = stack.top();
    stack.pop();

    QToolBar* currentTb = currentPr.first;
    const QString& currentPath = currentPr.second;

    const QList<QAction*>& actionLst = currentTb->actions();
    for (int i = actionLst.size() - 1; i > -1; --i) {
      QAction* action = actionLst[i];
      if (action->isSeparator()) {  // seperator
        continue;
      }
      auto* widget = currentTb->widgetForAction(action);
      if (widget == nullptr) {
        continue;
      }
      auto* const pType{widget->metaObject()->className()};
      if (strcmp(pType, "QToolBar") == 0) {  // nested toolbar
        auto* nestedTb = qobject_cast<QToolBar*>(widget);
        QString nestedPath = nestedTb->windowTitle() + '/' + currentPath;
        stack.emplace(nestedTb, nestedPath);
        continue;
      }
      if (strcmp(pType, "QToolButton") == 0) {  // QAction or QToolButton
        auto* toolButton = qobject_cast<QToolButton*>(widget);
        QAction* pTemp = action;
        if (toolButton->defaultAction() != nullptr) {
          pTemp = toolButton->defaultAction();
        }
        QString actionKey = pTemp->text() + '/' + currentPath;
        actionMap[actionKey] = action;
        continue;
      }
    }
  }
  mTextToActionMap.insert(actionMap);
  isDirty = true;
  return actionMap.size();
}

int ActionsRecorder::FromMenu(QMenu* menu) {
  if (menu == nullptr) {
    return 0;
  }
  QMap<QString, QAction*> actionMap;

  stack<pair<QMenu*, QString>> menuStack;
  menuStack.emplace(menu, menu->title());
  while (!menuStack.empty()) {
    auto currentPr = menuStack.top();
    QMenu* current = currentPr.first;
    const QString& currentPath = currentPr.second;

    menuStack.pop();
    for (QAction* action : current->actions()) {
      if (action->isSeparator()) {
        continue;
      }

      if (QMenu* subMenu = action->menu()) {
        menuStack.emplace(subMenu, subMenu->title() + '/' + currentPath);
      } else {
        actionMap[action->text() + '/' + currentPath] = action;
      }
    }
  }
  if (actionMap.isEmpty()) {
    return 0;
  }
  mTextToActionMap.insert(actionMap);
  isDirty = true;
  return actionMap.size();
}

int ActionsRecorder::FromMenuBar(QMenuBar* mb) {
  if (mb == nullptr) {
    return 0;
  }
  int cnt = 0;
  QList<QAction*> menuActions = mb->actions();
  for (QAction* action : menuActions) {
    if (QMenu* menu = action->menu()) {
      cnt += FromMenu(menu);
    }
  }
  return cnt;
}

int ActionsRecorder::FromActionGroup(const QActionGroup* actGrp) {
  if (actGrp == nullptr) {
    return 0;
  }
  QMap<QString, QAction*> actionMap;
  for (QAction* act : actGrp->actions()) {
    actionMap[act->text()] = act;
  }
  if (actionMap.isEmpty()) {
    return 0;
  }
  mTextToActionMap.insert(actionMap);
  isDirty = true;
  return mTextToActionMap.size();
}

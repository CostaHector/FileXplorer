#include "ActionsRecorder.h"
#include <QToolButton>
#include <stack>
#include <utility>
#include <QMenuBar>
using std::pair;
using std::stack;

int ActionsRecorder::FromToolButton(QToolButton* toolButton, const QString postPath) {
  if (toolButton == nullptr) {
    return 0;
  }
  QMap<QString, QAction*> actionMap;
  if (toolButton->defaultAction() != nullptr) {
    auto* defaultAction = toolButton->defaultAction();
    const QString rootPath = postPath.isEmpty() ? defaultAction->text() : defaultAction->text() + '/' + postPath;
    actionMap[rootPath] = defaultAction;
  }

  int actionsFromMenuCnt = FromMenu(toolButton->menu(), postPath);

  if (actionMap.isEmpty() && actionsFromMenuCnt == 0) {
    return 0;
  }

  if (!actionMap.isEmpty()) {
    mTextToActionMap.insert(actionMap);
    isDirty = true;
  }

  return actionMap.size() + actionsFromMenuCnt;
}

int ActionsRecorder::FromToolbar(QToolBar* tb) {
  if (tb == nullptr) {
    return 0;
  }
  QMap<QString, QAction*> actionMap;

  int actionsfromToolButtonCnt = 0;

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
        if (toolButton->defaultAction() != nullptr) {  // qtoolbar
          actionsfromToolButtonCnt += FromToolButton(toolButton, currentPath);
        } else {  // plain action
          QString actionKey = action->text() + '/' + currentPath;
          actionMap[actionKey] = action;
        }
        continue;
      }
    }
  }
  if (!actionMap.isEmpty()) {
    mTextToActionMap.insert(actionMap);
    isDirty = true;
  }
  return actionMap.size() + actionsfromToolButtonCnt;
}

int ActionsRecorder::FromMenu(QMenu* menu, const QString postPath) {
  if (menu == nullptr) {
    return 0;
  }
  QMap<QString, QAction*> actionMap;

  const QString rootPath = postPath.isEmpty() ? menu->title() : menu->title() + '/' + postPath;
  stack<pair<QMenu*, QString>> menuStack;
  menuStack.emplace(menu, rootPath);
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

#ifndef ACTIONSRECORDER_H
#define ACTIONSRECORDER_H

#include <QMap>
#include <QAction>
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QActionGroup>
#include <QToolButton>

class ActionsSearcher;
class ActionsRecorder {
 public:
  friend class ActionsSearcher;
  static ActionsRecorder& GetInst() {
    static ActionsRecorder inst;
    return inst;
  }
  const QStringList& GetKeys() const {
    if (isDirty) {
      keys = mTextToActionMap.keys();
      isDirty = false;
    }
    return keys;
  }
  int FromToolButton(QToolButton* toolButton, const QString postPath = "");
  int FromToolbar(QToolBar* tb);
  int FromMenu(QMenu* menu, const QString postPath = "");
  int FromMenuBar(QMenuBar* mb);
  int FromActionGroup(const QActionGroup* actGrp);

 private:
  ActionsRecorder() = default;
  QMap<QString, QAction*> mTextToActionMap;
  mutable QStringList keys;
  mutable bool isDirty{true};
};

#endif  // ACTIONSRECORDER_H

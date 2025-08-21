#ifndef BROWSER_ACTIONS_H
#define BROWSER_ACTIONS_H

#include <QMenu>
#include <QToolBar>
#include <QWidget>
#include <QAction>
#include <QActionGroup>

class BrowserActions:public QObject {
public:
  static BrowserActions& GetInst() {
    static BrowserActions inst;
    return inst;
  }

  QMenu* GetSearchInDBMenu(QWidget* parent);
  QToolBar* GetSearchInDBToolbar(QWidget* parent);

  QAction *SEARCH_CUR_TEXT{nullptr};
  QAction *ADVANCED_TEXT_SEARCH{nullptr};
  QAction *SEARCH_MULTIPLE_TEXTS{nullptr};
  QAction *CLEAR_ALL_SELECTIONS{nullptr};
  QAction *EDITOR_MODE{nullptr};

  QAction* COPY_SELECTED_TEXT{nullptr};
private:
  explicit BrowserActions(QObject *parent=nullptr);
};

#endif  // ACTIONSRECORDER_H

#ifndef RECYCLEBINACTIONS_H
#define RECYCLEBINACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QObject>
#include <QToolBar>

class RecycleBinActions : public QObject {
  Q_OBJECT
 public:
  explicit RecycleBinActions(QObject* parent = nullptr) : QObject(parent) { RECYLE_BIN_WIDGET->setCheckable(true); }

  QToolBar* getRecycleBinToolBar() const {
    auto* m_recycleBinToolBar = new QToolBar("Recycle Bin Tools");
    m_recycleBinToolBar->addAction(SUBMIT_AND_REFRESH);
    m_recycleBinToolBar->addSeparator();
    m_recycleBinToolBar->addAction(EMPTY_RECYCLE_BIN);
    m_recycleBinToolBar->addSeparator();
    m_recycleBinToolBar->addAction(PROPERTY);
    m_recycleBinToolBar->addSeparator();
    m_recycleBinToolBar->addAction(RESTORE_ALL_ITEMS);
    m_recycleBinToolBar->addAction(RESTORE_SELECTED_ITEMS);
    m_recycleBinToolBar->addSeparator();
    m_recycleBinToolBar->addAction(OPEN_DB_IN_APP);
    m_recycleBinToolBar->addSeparator();
    m_recycleBinToolBar->addAction(PLAY_THIS);
    m_recycleBinToolBar->addAction(REVEAL_IN_FILE_EXPLORER);
    m_recycleBinToolBar->addSeparator();
    m_recycleBinToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    return m_recycleBinToolBar;
  }

  QMenu* getRecycleBinMenu() const {
    auto* m_recycleBinMenu = new QMenu("Recycle Bin Menu");
    m_recycleBinMenu->addAction(RESTORE_SELECTED_ITEMS);
    m_recycleBinMenu->addAction(DELETE_SELECTED_ITEMS);
    m_recycleBinMenu->addSeparator();
    m_recycleBinMenu->addAction(PROPERTY);
    m_recycleBinMenu->addSeparator();
    m_recycleBinMenu->addAction(PLAY_THIS);
    m_recycleBinMenu->addAction(REVEAL_IN_FILE_EXPLORER);
    m_recycleBinMenu->addSeparator();
    m_recycleBinMenu->addAction(SUBMIT_AND_REFRESH);
    return m_recycleBinMenu;
  }

  QAction* RESTORE_SELECTED_ITEMS{new QAction{QIcon(":/themes/RESTORE_SELECTED_ITEMS"), "Restore the Selected Items", this}};
  QAction* DELETE_SELECTED_ITEMS{new QAction{QIcon(":/themes/DELETE_SELECTED_ITEMS"), "Delete the Selected Items", this}};
  QAction* PROPERTY{new QAction{QIcon(":/themes/PROPERTIES"), "Recycle Bin Properties", this}};
  QAction* EMPTY_RECYCLE_BIN{new QAction{QIcon(":/themes/EMPTY_RECYCLE_BIN"), "Empty Recycle Bin", this}};
  QAction* RESTORE_ALL_ITEMS{new QAction{QIcon(":/themes/RESTORE_ALL_ITEMS"), "Restore All Items", this}};
  QAction* PLAY_THIS{new QAction{QIcon(":/themes/PLAY_BUTTON_ROUND"), "Play", this}};
  QAction* REVEAL_IN_FILE_EXPLORER{new QAction{QIcon(":/themes/REVEAL_IN_EXPLORER"), "Reveal in file exploerer", this}};
  QAction* OPEN_DB_IN_APP{new QAction{QIcon(":/themes/SQLITE_APP"), "Open database in APP", this}};

  QAction* SUBMIT_AND_REFRESH{new QAction{QIcon(":/themes/REFRESH_THIS_PATH"), "Submit&Refresh", this}};

  QAction* RECYLE_BIN_WIDGET{new QAction{QIcon(":/themes/DIRTY_STATUS"), "Recycle bin", this}};
};
RecycleBinActions& g_recycleBinAg();

#endif  // RECYCLEBINACTIONS_H

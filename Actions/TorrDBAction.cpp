#include "TorrDBAction.h"
#include <QMenu>
#include "PublicVariable.h"

TorrDBAction::TorrDBAction(QObject* parent)
    : QObject{parent},
      OPEN_DB_WITH_LOCAL_APP{new (std::nothrow) QAction(QIcon(":img/SQLITE_APP"), tr("&Open with local app"), this)},
      INIT_DATABASE{new (std::nothrow) QAction(QString("init Database [%1]").arg(SystemPath::TORRENTS_DATABASE), this)},
      INIT_TABLE{new (std::nothrow) QAction(QString("&Create table [%1]").arg(DB_TABLE::TORRENTS), this)},
      INSERT_INTO_TABLE{new (std::nothrow) QAction(QString("&insert into table [%1]").arg(DB_TABLE::TORRENTS), this)},
      DELETE_FROM_TABLE{new (std::nothrow) QAction(tr("&Delete from table"), this)},
      DROP_TABLE{new (std::nothrow) QAction(tr("Drop table(complete table))"), this)},
      SUBMIT{new (std::nothrow) QAction(tr("&Submit"), this)},
      SHOW_TORRENTS_MANAGER{new (std::nothrow) QAction(QIcon(":img/TORRENTS_MANAGER"), tr("&Torrents"), this)} {
  OPEN_DB_WITH_LOCAL_APP->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_O));
  OPEN_DB_WITH_LOCAL_APP->setToolTip(
      QString("<b>%1 (%2)</b><br/> Open *.db file in local app.<br/>DB Browser(sqlite) and set it open db by default required.")
          .arg(OPEN_DB_WITH_LOCAL_APP->text(), OPEN_DB_WITH_LOCAL_APP->shortcut().toString()));

  SUBMIT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  SUBMIT->setShortcutVisibleInContextMenu(true);

  SHOW_TORRENTS_MANAGER->setCheckable(true);
}

QMenuBar* TorrDBAction::GetMenuBar() const {
  auto* fileMenu = new QMenu(tr("&File"));
  fileMenu->addAction(OPEN_DB_WITH_LOCAL_APP);
  fileMenu->setToolTipsVisible(true);

  auto* editMenu = new QMenu(tr("&Edit"));
  editMenu->addActions({INIT_DATABASE, INIT_TABLE, INSERT_INTO_TABLE});
  editMenu->addSeparator();
  editMenu->addActions({DELETE_FROM_TABLE, DROP_TABLE});
  editMenu->addSeparator();
  editMenu->addAction(SUBMIT);
  editMenu->setToolTipsVisible(true);

  auto* m_menuBar = new QMenuBar;
  m_menuBar->addMenu(fileMenu);
  m_menuBar->addMenu(editMenu);
  return m_menuBar;
}

TorrDBAction& g_torrActions() {
  static TorrDBAction torrentsManagerActIns;
  return torrentsManagerActIns;
}

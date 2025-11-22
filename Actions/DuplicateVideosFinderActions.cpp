#include "DuplicateVideosFinderActions.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include <QLineEdit>
#include <QMenu>
#include <QLabel>

DuplicateVideosFinderActions::DuplicateVideosFinderActions(QObject* parent) : QObject{parent} {
  DIFFER_BY_DURATION = new (std::nothrow) QAction{QIcon{":img/VIDEO_DURATION"}, "Duration Tolerance (ms)", this};
  CHECK_NULLPTR_RETURN_VOID(DIFFER_BY_DURATION);
  DIFFER_BY_DURATION->setToolTip("Values within [reference - tolerance/2, reference + tolerance/2) are grouped together\nUnit: milliseconds");
  DIFFER_BY_DURATION->setCheckable(true);

  DIFFER_BY_SIZE = new (std::nothrow) QAction{QIcon{":img/FILE_SIZE"}, "Size Tolerance (B)", this};
  CHECK_NULLPTR_RETURN_VOID(DIFFER_BY_SIZE);
  DIFFER_BY_SIZE->setToolTip("Values within [reference - tolerance/2, reference + tolerance/2) are grouped together\nUnit: bytes");
  DIFFER_BY_SIZE->setCheckable(true);
  DIFFER_BY_SIZE->setChecked(true);

  DIFFER_BY = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(DIFFER_BY);
  DIFFER_BY->addAction(DIFFER_BY_DURATION);
  DIFFER_BY->addAction(DIFFER_BY_SIZE);
  DIFFER_BY->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  OPEN_DATABASE = new (std::nothrow) QAction{QIcon(":img/SQLITE_APP"), "&Open Database", this};
  CHECK_NULLPTR_RETURN_VOID(OPEN_DATABASE);
  OPEN_DATABASE->setToolTip(QString("<b>%1 (%2)</b><br/>Open SQLite database files (*.db)<br/>Requires DB Browser for SQLite to be installed and set "
                                    "as the default application")
                                .arg(OPEN_DATABASE->text(), OPEN_DATABASE->shortcut().toString()));

  RECYCLE_SELECTIONS = new (std::nothrow) QAction{QIcon{":img/MOVE_TO_TRASH_BIN"}, "Recycle", this};
  CHECK_NULLPTR_RETURN_VOID(RECYCLE_SELECTIONS);
  RECYCLE_SELECTIONS->setShortcut(QKeySequence(Qt::Key_Delete));
  RECYCLE_SELECTIONS->setToolTip(QString("(%1) Move selected items to recycle bin").arg(RECYCLE_SELECTIONS->shortcut().toString()));

  CLEAR_ANALYSIS_LIST = new (std::nothrow) QAction{"Clear Analyse", this};
  CHECK_NULLPTR_RETURN_VOID(CLEAR_ANALYSIS_LIST);
  CLEAR_ANALYSIS_LIST->setToolTip("Clear current analyse list");

  ANALYSE_THESE_TABLES = new (std::nothrow) QAction{QIcon(":img/ANALYSE_AI_MEDIA_TABLES"), "Analyse", this};
  CHECK_NULLPTR_RETURN_VOID(ANALYSE_THESE_TABLES);
  ANALYSE_THESE_TABLES->setToolTip("Append records from selected tables to the current analysis list");

  SCAN_A_PATH = new (std::nothrow) QAction{QIcon(":img/LOAD_A_PATH"), "Scan a path", this};
  CHECK_NULLPTR_RETURN_VOID(SCAN_A_PATH);
  SCAN_A_PATH->setToolTip("Scan videos from a directory and populate the table with their information");

  AUDIT_THESE_TABLES = new (std::nothrow) QAction{QIcon{":img/AUDIT_AI_MEDIA_DUP"}, "Audit", this};
  CHECK_NULLPTR_RETURN_VOID(AUDIT_THESE_TABLES);
  AUDIT_THESE_TABLES->setToolTip("Validate table records and update if corresponding files are missing");

  DROP_THESE_TABLES = new (std::nothrow) QAction{QIcon{":img/DROP_TABLE"}, "Drop Tables", this};
  CHECK_NULLPTR_RETURN_VOID(DROP_THESE_TABLES);
  DROP_THESE_TABLES->setToolTip("Delete selected table(s)");

  FORCE_RELOAD_TABLES = new (std::nothrow) QAction{QIcon{":img/REFRESH_THIS_PATH"}, "Force reload", this};
  CHECK_NULLPTR_RETURN_VOID(FORCE_RELOAD_TABLES);
  FORCE_RELOAD_TABLES->setToolTip("Delete selected table(s) and reload them from source");
}

QToolBar* DuplicateVideosFinderActions::GetAiMediaToolBar(QLineEdit* tableNameFilterLE, QLineEdit* sizeDevLE, QLineEdit* durationDevLE, QWidget* parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(tableNameFilterLE);
  CHECK_NULLPTR_RETURN_NULLPTR(sizeDevLE);
  CHECK_NULLPTR_RETURN_NULLPTR(durationDevLE);

  QToolBar* m_tb = new (std::nothrow) QToolBar{"Duplicator finder toolbar", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(m_tb);

  m_tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  m_tb->addAction(DIFFER_BY_DURATION);
  m_tb->addWidget(durationDevLE);
  m_tb->addSeparator();
  m_tb->addAction(DIFFER_BY_SIZE);
  m_tb->addWidget(sizeDevLE);
  m_tb->addSeparator();
  m_tb->addAction(OPEN_DATABASE);
  m_tb->addSeparator();
  m_tb->addAction(RECYCLE_SELECTIONS);
  m_tb->addSeparator();
  m_tb->addWidget(tableNameFilterLE);
  m_tb->addSeparator();
  return m_tb;
}

DuplicateVideosFinderActions& g_dupVidFinderAg() {
  static DuplicateVideosFinderActions ins;
  return ins;
}

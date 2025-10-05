#include "DatabaseSearchToolBar.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "CastPsonFileHelper.h"
#include "TableFields.h"
#include "MemoryKey.h"
#include "MountHelper.h"
#include "MovieDBActions.h"
#include "CastDBActions.h"
#include <QLineEdit>
#include <QCompleter>
#include <QInputDialog>
#include <QLayout>

// -------------------------------- DatabaseSearchToolBar --------------------------------
DatabaseSearchToolBar::DatabaseSearchToolBar(const QString& title, QWidget* parent) : QToolBar{title, parent} {
  CHECK_NULLPTR_RETURN_VOID(parent);
  m_whereCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_whereCB);
  m_whereCB->setEditable(true);
  m_whereCB->setInsertPolicy(QComboBox::InsertPolicy::InsertAtTop);
  m_whereCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  m_whereCB->lineEdit()->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
  m_whereCB->lineEdit()->setClearButtonEnabled(true);
  using namespace MOVIE_TABLE;
  m_whereCB->addItem(QString{R"(`%1` LIKE "%%")"}.arg(ENUM_2_STR(Name)));
  m_whereCB->addItem(QString{R"(INSTR(`%1`, "")>0)"}.arg(ENUM_2_STR(Name)));

  QCompleter* pCompleter = new (std::nothrow) QCompleter{this};
  CHECK_NULLPTR_RETURN_VOID(pCompleter);
  pCompleter->setCaseSensitivity(Qt::CaseSensitive);
  m_whereCB->setCompleter(pCompleter);

  _QUICK_WHERE_CLAUSE_ACT = new (std::nothrow) QAction(QIcon(":img/QUICK_WHERE_FILTERS"), "Where clause", this);
  CHECK_NULLPTR_RETURN_VOID(_QUICK_WHERE_CLAUSE_ACT);
  _QUICK_WHERE_CLAUSE_ACT->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_H));
  _QUICK_WHERE_CLAUSE_ACT->setToolTip(QString{"<b>%1 (%2)</b><br/> Construct where clause quickly for `MOVIE/CAST` table;"}.arg(
      _QUICK_WHERE_CLAUSE_ACT->text(), _QUICK_WHERE_CLAUSE_ACT->shortcut().toString()));

  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  addWidget(m_whereCB);
  addAction(_QUICK_WHERE_CLAUSE_ACT);
  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
  subscribe();
}

void DatabaseSearchToolBar::onQuickWhereClause() {
  if (m_quickWhereClause == nullptr) {
    m_quickWhereClause = new (std::nothrow) QuickWhereClauseDialog{this};
    CHECK_NULLPTR_RETURN_VOID(m_quickWhereClause)
  }
  int dlgRetCode = QDialog::DialogCode::Rejected;
#ifdef RUNNING_UNIT_TESTS
  dlgRetCode = QDialog::DialogCode::Accepted;
#else
  dlgRetCode = m_quickWhereClause->exec();
#endif
  if (dlgRetCode != QDialog::DialogCode::Accepted) {
    LOG_INFO_P("[Skip] User cancel quick where clause", "dialogCode:%d", dlgRetCode);
    return;
  }
  const QString& whereClause{m_quickWhereClause->GetWhereString()};
  LOG_D("QuickWhereClause: [%s]", qPrintable(whereClause));
  SetWhereClause(whereClause);
  emit whereClauseChanged(whereClause);
}

void DatabaseSearchToolBar::subscribe() {
  connect(m_whereCB->lineEdit(), &QLineEdit::returnPressed, this, &MovieDBSearchToolBar::EmitWhereClauseChangedSignal);
  connect(_QUICK_WHERE_CLAUSE_ACT, &QAction::triggered, this, &MovieDBSearchToolBar::onQuickWhereClause);
}

void DatabaseSearchToolBar::EmitWhereClauseChangedSignal() {
  const QString& clause = GetCurrentWhereClause();
  LOG_D("WhereClauseChanged signal[%s]", qPrintable(clause));
  emit whereClauseChanged(clause);
}

// -------------------------------- MovieDBSearchToolBar --------------------------------
MovieDBSearchToolBar::MovieDBSearchToolBar(const QString& title, QWidget* parent)  //
    : DatabaseSearchToolBar{title, parent} {
  CHECK_NULLPTR_RETURN_VOID(parent);

  m_whereCB->addItem(QString{R"(`%1` LIKE "%%" AND `%1` LIKE "%%")"}.arg(ENUM_2_STR(PrePathRight)));
  m_whereCB->addItem(QString{R"(`%1` LIKE "%%")"}.arg(ENUM_2_STR(PrePathRight)));
  m_whereCB->addItem(QString{R"(INSTR(`%1`, "")>0 AND INSTR(`%1`, "")>0)"}.arg(ENUM_2_STR(PrePathRight)));
  m_whereCB->addItem(QString{R"(INSTR(`%1`, "")>0)"}.arg(ENUM_2_STR(PrePathRight)));
  m_whereCB->addItem(QString{R"(`%1` BETWEEN 0 AND 1000000)"}.arg(ENUM_2_STR(Size)));
  m_whereCB->addItem(QString{R"(`%1` = "E:/")"}.arg(ENUM_2_STR(Driver)));
  m_whereCB->addItem(QString{R"(`%1` IN ("Comedy", "Documentary"))"}.arg(ENUM_2_STR(Tags)));
  m_whereCB->addItem(QString{R"(`%1` LIKES "Chris Evans%")"}.arg(ENUM_2_STR(Cast)));  // Don't use leading wildcard!

  m_tablesCB = new (std::nothrow) QComboBox;
  CHECK_NULLPTR_RETURN_VOID(m_tablesCB);
  m_tablesCB->setEditable(false);
  m_tablesCB->setInsertPolicy(QComboBox::InsertPolicy::InsertAtBottom);  // not editable
  m_tablesCB->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
  m_tablesCB->setMaxVisibleItems(15);
  auto* tblAct = insertWidget(_QUICK_WHERE_CLAUSE_ACT, m_tablesCB);
  insertSeparator(tblAct);
  MovieDBSearchToolBar::extraSignalSubscribe();
}

void MovieDBSearchToolBar::extraSignalSubscribe() {
  connect(m_tablesCB, &QComboBox::currentTextChanged, this, &MovieDBSearchToolBar::movieTableChanged);
}

QString MovieDBSearchToolBar::GetMovieTableMountPath() const {
#ifdef RUNNING_UNIT_TESTS
  return MountPathTableNameMapper::toMountPathMock(m_tablesCB->currentText());
#else
  return MountPathTableNameMapper::toMountPath(m_tablesCB->currentText());
#endif
}

QString MovieDBSearchToolBar::AskUserDropWhichTable() {
  const QStringList& candidates = toMovieTableCandidates();
  if (candidates.isEmpty()) {
    LOG_INFO_NP("There is no table exists", "skip drop");
    return "";
  }
  const QString dropWhichTableTitle = "CONFIRM DROP? (NOT RECOVERABLE)";
  const QString dropWhichTableMsgs{QString{"There are %1 table(s) as following:\n%2"}.arg(candidates.size()).arg(candidates.join('\n'))};
  const int defaultDropIndex = m_tablesCB->currentIndex();

  QString drpTbl;
  bool okUserSelect = false;

#ifdef RUNNING_UNIT_TESTS
  std::tie(okUserSelect, drpTbl) = MovieDBSearchToolBarMock::QryDropWhichTableMock();
#else
  drpTbl = QInputDialog::getItem(this, dropWhichTableTitle, dropWhichTableMsgs, candidates, defaultDropIndex,  //
                                 false, &okUserSelect);
#endif
  if (!okUserSelect) {
    LOG_OK_NP("[skip] Drop table", "User cancel");
    return "";
  }
  if (drpTbl.isEmpty()) {
    LOG_ERR_NP("[Abort] Table name is empty, cannot drop", drpTbl);
    return "";
  }
  return drpTbl;
}

void MovieDBSearchToolBar::AddATable(const QString& newTableName) {
  m_tablesCB->addItem(newTableName);
  m_tablesCB->setCurrentIndex(m_tablesCB->count() - 1);  // todo this line mat can be removed
}

void MovieDBSearchToolBar::InitTables(const QStringList& tbls) {
  m_tablesCB->clear();
  for (const QString& tableName : tbls) {  // in underscore
    m_tablesCB->addItem(tableName);
  }
  LOG_D("Tables count:%d", tbls.size());
}

void MovieDBSearchToolBar::InitCurrentIndex() {
  const QString defaultTableName{Configuration().value(MemoryKey::VIDS_LAST_TABLE_NAME.name, MemoryKey::VIDS_LAST_TABLE_NAME.v).toString()};
  const int defaultDisplayIndex = m_tablesCB->findText(defaultTableName, Qt::MatchStartsWith);
  if (defaultDisplayIndex != -1) {
    m_tablesCB->setCurrentIndex(defaultDisplayIndex);
  }
}

QStringList MovieDBSearchToolBar::toMovieTableCandidates() const {
  const int cnt = m_tablesCB->count();
  QStringList ans;
  ans.reserve(cnt);
  for (int index = 0; index < cnt; ++index) {
    ans << m_tablesCB->itemText(index);
  }
  return ans;
}

// -------------------------------- CastDatabaseSearchToolBar --------------------------------
CastDatabaseSearchToolBar::CastDatabaseSearchToolBar(const QString& title, QWidget* parent)  //
    : DatabaseSearchToolBar{title, parent} {
  CHECK_NULLPTR_RETURN_VOID(m_whereCB) {
    using namespace PERFORMER_DB_HEADER_KEY;
    for (const auto& field : CAST_TABLE_HEADERS) {
      m_whereCB->addItem(QString{R"(`%1` LIKE "%%")"}.arg(field));
      m_whereCB->addItem(QString{R"(INSTR(`%1`, "")>0)"}.arg(field));
    }
    m_whereCB->addItem(QString{R"(`%1`="")"}.arg(ENUM_2_STR(Ori)));
  }
  CastDatabaseSearchToolBar::extraSignalSubscribe();
}

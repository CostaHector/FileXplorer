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

// -------------------------------- Guid2RootPathComboxBox --------------------------------
Guid2RootPathComboxBox::Guid2RootPathComboxBox(QWidget* parent) : QComboBox{parent} {
  setEditable(false);
}

void Guid2RootPathComboxBox::AddItem(const QString& guidUnderscore, const QString& rootPath) {
  const int index = count();
  addItem(guidUnderscore + MountHelper::JOINER_STR + rootPath);
  QString toolHint;
  toolHint.reserve(50);
  toolHint += "GUID:<br/>";
  toolHint += "<b>" + guidUnderscore + "</b><br/>";
  toolHint += "Path:<br/>";
  toolHint += "<b>" + rootPath + "</b>";
  setItemData(index, toolHint, Qt::ToolTipRole);
}

QString Guid2RootPathComboxBox::CurrentTableName() const {
  return MountHelper::ChoppedDisplayName(currentText());
}
QString Guid2RootPathComboxBox::CurrentGuid() const {
  return CurrentTableName().replace(MountHelper::TABLE_UNDERSCORE, MountHelper::GUID_HYPEN);
}
QString Guid2RootPathComboxBox::CurrentRootPath() const {
  return currentText().contains(MountHelper::JOINER_STR) ? currentText().mid(MountHelper::ROOTPATH_START) : "";
}
QStringList Guid2RootPathComboxBox::ToQStringList() const {
  const int cnt = count();
  QStringList ans;
  ans.reserve(cnt);
  for (int index = 0; index < cnt; ++index) {
    ans << itemText(index);
  }
  return ans;
}


// -------------------------------- DatabaseSearchToolBar --------------------------------
DatabaseSearchToolBar::DatabaseSearchToolBar(const QString& title, QWidget* parent) :
  QToolBar{title, parent} {
  CHECK_NULLPTR_RETURN_VOID(parent);
  m_whereCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_whereCB);
  m_whereCB->setEditable(true);
  m_whereCB->setInsertPolicy(QComboBox::InsertPolicy::InsertAtTop);
  m_whereCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  m_whereCB->lineEdit()->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
  m_whereCB->lineEdit()->setClearButtonEnabled(true);
  using namespace MOVIE_TABLE;
  m_whereCB->addItem(QString{R"(INSTR(`%1`, "")>0)"}.arg(ENUM_2_STR(Name)));
  m_whereCB->addItem(QString{R"(INSTR(`%1`, "")>0)"}.arg(ENUM_2_STR(Name)));

  QCompleter* pCompleter = new (std::nothrow) QCompleter{this};
  CHECK_NULLPTR_RETURN_VOID(pCompleter);
  pCompleter->setCaseSensitivity(Qt::CaseSensitive);
  m_whereCB->setCompleter(pCompleter);

  _QUICK_WHERE_CLAUSE_ACT = new (std::nothrow) QAction(QIcon(":img/QUICK_WHERE_FILTERS"), "Where clause", this);
  CHECK_NULLPTR_RETURN_VOID(_QUICK_WHERE_CLAUSE_ACT);
  _QUICK_WHERE_CLAUSE_ACT->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_H));
  _QUICK_WHERE_CLAUSE_ACT->setToolTip(QString{"<b>%1 (%2)</b><br/> Construct where clause quickly for `MOVIE/CAST` table;"}
                                          .arg(_QUICK_WHERE_CLAUSE_ACT->text(), _QUICK_WHERE_CLAUSE_ACT->shortcut().toString()));

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
  auto retCode = m_quickWhereClause->exec();
  if (retCode != QDialog::DialogCode::Accepted) {
    LOG_INFO_P("[Skip] User cancel quick where clause", "dialogCode:%d", retCode);
    return;
  }
  const QString& whereClause {m_quickWhereClause->GetWhereString()};
  qDebug("QuickWhereClause: [%s]", qPrintable(whereClause));
  SetWhereClause(whereClause);
  emit whereClauseChanged(whereClause);
}

void DatabaseSearchToolBar::subscribe() {
  connect(m_whereCB->lineEdit(), &QLineEdit::returnPressed, this, MovieDBSearchToolBar::EmitWhereClauseChangedSignal);
  connect(_QUICK_WHERE_CLAUSE_ACT, &QAction::triggered, this, &MovieDBSearchToolBar::onQuickWhereClause);
}

void DatabaseSearchToolBar::EmitWhereClauseChangedSignal() {
  const QString& clause = GetCurrentWhereClause();
  qDebug("WhereClauseChanged signal[%s]", qPrintable(clause));
  emit whereClauseChanged(clause);
}

// -------------------------------- MovieDBSearchToolBar --------------------------------
MovieDBSearchToolBar::MovieDBSearchToolBar(const QString& title, QWidget* parent)  //
  : DatabaseSearchToolBar{title, parent} {
  CHECK_NULLPTR_RETURN_VOID(parent);

  m_whereCB->addItem(QString{R"(INSTR(`%1`, "")>0 AND INSTR(`%1`, "")>0)"}.arg(ENUM_2_STR(PrePathRight)));
  m_whereCB->addItem(QString{R"(INSTR(`%1`, "")>0)"}.arg(ENUM_2_STR(PrePathRight)));
  m_whereCB->addItem(QString{R"(`%1` BETWEEN 0 AND 1000000)"}.arg(ENUM_2_STR(Size)));
  m_whereCB->addItem(QString{R"(`%1` = "E:/")"}.arg(ENUM_2_STR(Driver)));
  m_whereCB->addItem(QString{R"(`%1` IN ("Comedy", "Documentary"))"}.arg(ENUM_2_STR(Tags)));
  m_whereCB->addItem(QString{R"(`%1` LIKES "Chris Evans%")"}.arg(ENUM_2_STR(Cast))); // Don't use leading wildcard!

  m_tablesCB = new (std::nothrow) Guid2RootPathComboxBox;
  CHECK_NULLPTR_RETURN_VOID(m_tablesCB);
  m_tablesCB->setInsertPolicy(QComboBox::InsertPolicy::InsertAtBottom); // not editable
  m_tablesCB->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
  auto* tblAct = insertWidget(_QUICK_WHERE_CLAUSE_ACT, m_tablesCB);
  insertSeparator(tblAct);
  MovieDBSearchToolBar::extraSignalSubscribe();
}

void MovieDBSearchToolBar::extraSignalSubscribe() {
  connect(m_tablesCB, &QComboBox::currentTextChanged, this, &MovieDBSearchToolBar::movieTableChanged);
}

QString MovieDBSearchToolBar::AskUserDropWhichTable() {
  const QStringList& candidates = m_tablesCB->ToQStringList();
  if (candidates.isEmpty()) {
    LOG_INFO_NP("There is no table exists", "skip drop");
    return "";
  }
  const int defaultDropIndex = m_tablesCB->currentIndex();
  const auto msgs {QString{"There are %1 table(s) as following:\n%2"}.arg(candidates.size()).arg(candidates.join('\n'))};
  bool okUserSelect = false;
  const QString& drpTbl = QInputDialog::getItem(this, "CONFIRM DROP? (NOT RECOVERABLE)",  //
                                                msgs,                                     //
                                                candidates,                               //
                                                defaultDropIndex,                         //
                                                false,                                    //
                                                &okUserSelect);
  if (!okUserSelect) {
    LOG_GOOD_NP("[skip] Drop table", "User cancel");
    return "";
  }
  const QString& deleteTbl = MountHelper::ChoppedDisplayName(drpTbl);
  if (deleteTbl.isEmpty()) {
    LOG_BAD_NP("[Abort] Table name is empty, cannot drop", deleteTbl);
    return "";
  }
  return deleteTbl;
}

void MovieDBSearchToolBar::AddATable(const QString& newTableName) {
  m_tablesCB->AddItem(newTableName, MountHelper::GetDisplayNameByGuidTableName(newTableName));
  m_tablesCB->setCurrentIndex(m_tablesCB->count() - 1);
}

void MovieDBSearchToolBar::InitTables(const QStringList& tbls) {
  m_tablesCB->clear();
  const auto& guidTblName2Disp = MountHelper::GetGuidTableName2DisplayName();
  for (const QString& tableName : tbls) {  // in underscore
    m_tablesCB->AddItem(tableName, guidTblName2Disp.value(tableName, "displace name NOT FOUND"));
  }
  qDebug("Tables count:%d", tbls.size());
}

void MovieDBSearchToolBar::InitCurrentIndex() {
  const QString defaultTableName {Configuration().value(MemoryKey::VIDS_LAST_TABLE_NAME.name, MemoryKey::VIDS_LAST_TABLE_NAME.v).toString()};
  const int defaultDisplayIndex = m_tablesCB->findText(defaultTableName, Qt::MatchStartsWith);
  if (defaultDisplayIndex != -1) {
    m_tablesCB->setCurrentIndex(defaultDisplayIndex);
  }
}

// -------------------------------- CastDatabaseSearchToolBar --------------------------------
CastDatabaseSearchToolBar::CastDatabaseSearchToolBar(const QString& title, QWidget* parent)//
  : DatabaseSearchToolBar{title, parent} {
  {
    using namespace PERFORMER_DB_HEADER_KEY;
    for (const auto& field: DB_HEADER) {
      m_whereCB->addItem(QString{R"(INSTR(`%1`, "")>0)"}.arg(field));
    }
    m_whereCB->addItem(QString{R"(`%1`="")"}.arg(ENUM_2_STR(Ori)));
  }
  CastDatabaseSearchToolBar::extraSignalSubscribe();
}

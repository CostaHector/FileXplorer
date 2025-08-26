#include "DatabaseSearchToolBar.h"
#include "Notificator.h"
#include "PublicMacro.h"
#include "PerformerJsonFileHelper.h"
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
  addItem(QIcon(":img/TABLES"), guidUnderscore + MountHelper::JOINER_STR + rootPath);
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

// -------------------------------- MovieDBSearchToolBar --------------------------------
MovieDBSearchToolBar::MovieDBSearchToolBar(const QString& title, QWidget* parent)  //
  : QToolBar{title, parent} {
  CHECK_NULLPTR_RETURN_VOID(parent);
  m_searchCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_searchCB);
  m_searchCB->setEditable(true);
  m_searchCB->setInsertPolicy(QComboBox::InsertPolicy::InsertAtTop);
  m_searchCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  m_searchCB->lineEdit()->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
  m_searchCB->lineEdit()->setClearButtonEnabled(true);
  using namespace MOVIE_TABLE;
  m_searchCB->addItem(QString{R"(INSTR(`%1`, "")>0)"}.arg(ENUM_2_STR(Name)));
  m_searchCB->addItem(QString{R"(INSTR(`%1`, "")>0)"}.arg(ENUM_2_STR(Name)));
  m_searchCB->addItem(QString{R"(INSTR(`%1`, "")>0 AND INSTR(`%1`, "")>0)"}.arg(ENUM_2_STR(PrePathRight)));
  m_searchCB->addItem(QString{R"(INSTR(`%1`, "")>0)"}.arg(ENUM_2_STR(PrePathRight)));
  m_searchCB->addItem(QString{R"(`%1` BETWEEN 0 AND 1000000)"}.arg(ENUM_2_STR(Size)));
  m_searchCB->addItem(QString{R"(`%1` = "E:/")"}.arg(ENUM_2_STR(Driver)));
  m_searchCB->addItem(QString{R"(`%1` IN ("Comedy", "Documentary"))"}.arg(ENUM_2_STR(Tags)));
  m_searchCB->addItem(QString{R"(`%1` LIKES "Chris Evans%")"}.arg(ENUM_2_STR(Cast))); // Don't use leading wildcard!
  QCompleter* pCompleter = new (std::nothrow) QCompleter{this};
  CHECK_NULLPTR_RETURN_VOID(pCompleter);
  pCompleter->setCaseSensitivity(Qt::CaseSensitive);
  m_searchCB->setCompleter(pCompleter);

  m_tablesCB = new (std::nothrow) Guid2RootPathComboxBox;
  CHECK_NULLPTR_RETURN_VOID(m_tablesCB);
  m_tablesCB->setInsertPolicy(QComboBox::InsertPolicy::InsertAtBottom); // not editable
  m_tablesCB->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
  addWidget(m_searchCB);
  addSeparator();
  addWidget(m_tablesCB);
  addSeparator();
  addAction(g_dbAct()._QUICK_WHERE_CLAUSE_MOVIE);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);

  subscribe();
}

void MovieDBSearchToolBar::onQuickWhereClause() {
  if (m_quickWhereClause == nullptr) {
    m_quickWhereClause = new (std::nothrow) QuickWhereClause{this};
    CHECK_NULLPTR_RETURN_VOID(m_quickWhereClause)
  }
  auto retCode = m_quickWhereClause->exec();
  if (retCode != QDialog::DialogCode::Accepted) {
    LOG_INFO("User cancel quick where clause", "skip")
    return;
  }
  const QString& whereClause {m_quickWhereClause->GetWhereString()};
  qDebug("Quick where clause: [%s]", qPrintable(whereClause));
  SetWhereClause(whereClause);
  emit whereClauseChanged(whereClause);
}

void MovieDBSearchToolBar::subscribe() {
  connect(m_searchCB->lineEdit(), &QLineEdit::returnPressed, [this]() { emit whereClauseChanged(m_searchCB->currentText()); });
  connect(m_tablesCB, &QComboBox::currentTextChanged, this, &MovieDBSearchToolBar::movieTableChanged);
  connect(g_dbAct()._QUICK_WHERE_CLAUSE_MOVIE, &QAction::triggered, this, &MovieDBSearchToolBar::onQuickWhereClause);
}

QString MovieDBSearchToolBar::AskUserDropWhichTable() {
  const QStringList& candidates = m_tablesCB->ToQStringList();
  if (candidates.isEmpty()) {
    LOG_INFO("There is no table exists", "skip drop");
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
    LOG_GOOD("[skip] Drop table", "User cancel")
    return "";
  }
  const QString& deleteTbl = MountHelper::ChoppedDisplayName(drpTbl);
  if (deleteTbl.isEmpty()) {
    LOG_BAD("[Abort] Table name is empty, cannot drop", deleteTbl)
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
  : QToolBar{title, parent} {
  CHECK_NULLPTR_RETURN_VOID(parent)
  m_nameClauseCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_nameClauseCB);
  m_nameClauseCB->setEditable(true);
  m_nameClauseCB->setInsertPolicy(QComboBox::InsertPolicy::InsertAtTop);
  m_nameClauseCB->lineEdit()->addAction(QIcon{":img/SEARCH"}, QLineEdit::LeadingPosition);
  m_nameClauseCB->lineEdit()->setClearButtonEnabled(true);
  m_nameClauseCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  {
    using namespace PERFORMER_DB_HEADER_KEY;
    for (const auto& field: DB_HEADER) {
      m_nameClauseCB->addItem(QString{R"(INSTR(`%1`, "")>0)"}.arg(field));
    }
    m_nameClauseCB->addItem(QString{R"(`%1`="")"}.arg(ENUM_2_STR(Ori)));
  }

  addWidget(m_nameClauseCB);
  addSeparator();
  addAction(g_castAct()._QUICK_WHERE_CLAUSE_CAST);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);

  subscribe();
}

void CastDatabaseSearchToolBar::onQuickWhereClause() {
  if (m_quickWhereClause == nullptr) {
    m_quickWhereClause = new (std::nothrow) QuickWhereClause{this};
    CHECK_NULLPTR_RETURN_VOID(m_quickWhereClause)
  }
  auto retCode = m_quickWhereClause->exec();
  if (retCode != QDialog::DialogCode::Accepted) {
    LOG_INFO("User cancel quick where clause", "skip")
    return;
  }
  const QString& whereClause {m_quickWhereClause->GetWhereString()};
  qDebug("Quick where clause: [%s]", qPrintable(whereClause));
  SetWhereClause(whereClause);
  emit whereClauseChanged(whereClause);
}

void CastDatabaseSearchToolBar::subscribe() {
  connect(m_nameClauseCB->lineEdit(), &QLineEdit::returnPressed, this, &CastDatabaseSearchToolBar::Emit);
  connect(g_castAct()._QUICK_WHERE_CLAUSE_CAST, &QAction::triggered, this, &CastDatabaseSearchToolBar::onQuickWhereClause);
}

void CastDatabaseSearchToolBar::Emit() {
  const QString& clause = m_nameClauseCB->currentText();
  qDebug("Cast where[%s]", qPrintable(clause));
  emit whereClauseChanged(clause);
}

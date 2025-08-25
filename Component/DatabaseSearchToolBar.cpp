#include "DatabaseSearchToolBar.h"
#include "PublicMacro.h"
#include "TableFields.h"
#include "MountHelper.h"
#include <QCompleter>

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

MovieDBSearchToolBar::MovieDBSearchToolBar(const QString& title, QWidget* parent)  //
  : QToolBar{title, parent} {
  m_searchCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_searchCB);
  m_searchCB->setEditable(true);
  m_searchCB->setInsertPolicy(QComboBox::InsertPolicy::InsertAtTop);
  m_searchCB->lineEdit()->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
  m_searchCB->lineEdit()->setClearButtonEnabled(true);
  using namespace MOVIE_TABLE;
  m_searchCB->addItem(QString{R"(`%1` LIKE "%")"}.arg(ENUM_2_STR(Name)));
  m_searchCB->addItem(QString{R"(`%1` BETWEEN 0 AND 1000000)"}.arg(ENUM_2_STR(Size)));
  m_searchCB->addItem(QString{R"(`%1` = "E:/")"}.arg(ENUM_2_STR(Driver)));
  m_searchCB->addItem(QString{R"(`%1` IN ("Comedy", "Documentary"))"}.arg(ENUM_2_STR(Tags)));
  m_searchCB->addItem(QString{R"(`%1` LIKES "%Chris Evans%")"}.arg(ENUM_2_STR(Cast)));
  m_searchCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  QCompleter* pCompleter = new (std::nothrow) QCompleter{this};
  CHECK_NULLPTR_RETURN_VOID(pCompleter);
  pCompleter->setCaseSensitivity(Qt::CaseSensitive);
  m_searchCB->setCompleter(pCompleter);

  m_tablesCB = new (std::nothrow) Guid2RootPathComboxBox;
  CHECK_NULLPTR_RETURN_VOID(m_tablesCB);
  m_tablesCB->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);

  addWidget(m_searchCB);
  addWidget(m_tablesCB);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
}

#include "PerformerJsonFileHelper.h"

CastDatabaseSearchToolBar::CastDatabaseSearchToolBar(const QString& title, QWidget* parent)//
  : QToolBar{title, parent} {
  m_nameClauseCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_nameClauseCB);
  m_nameClauseCB->setEditable(true);
  m_nameClauseCB->setInsertPolicy(QComboBox::InsertPolicy::InsertAtTop);
  m_nameClauseCB->lineEdit()->addAction(QIcon{":img/SEARCH"}, QLineEdit::LeadingPosition);
  m_nameClauseCB->lineEdit()->setClearButtonEnabled(true);
  m_nameClauseCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_nameClauseCB->addItem("");
  if (!PERFORMER_DB_HEADER_KEY::DB_HEADER.isEmpty()) {
    m_nameClauseCB->addItem(QString{R"(`%1` LIKE "%")"}.arg(PERFORMER_DB_HEADER_KEY::DB_HEADER.front()));
  }

  m_otherClauseCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_nameClauseCB);
  m_otherClauseCB->setEditable(true);
  m_otherClauseCB->setInsertPolicy(QComboBox::InsertPolicy::InsertAtTop);
  m_otherClauseCB->lineEdit()->addAction(QIcon{":img/FILE_SYSTEM_FILTER"}, QLineEdit::LeadingPosition);
  m_otherClauseCB->lineEdit()->setClearButtonEnabled(true);
  m_otherClauseCB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  m_otherClauseCB->addItem("");
  for (int i = 1; i < PERFORMER_DB_HEADER_KEY::DB_HEADER.size(); ++i) {
    m_otherClauseCB->addItem(QString{R"(`%1` LIKE "%")"}.arg(PERFORMER_DB_HEADER_KEY::DB_HEADER[i]));
  }

  addWidget(m_nameClauseCB);
  addWidget(m_otherClauseCB);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);

  subscribe();
}

void CastDatabaseSearchToolBar::subscribe() {
  connect(m_nameClauseCB->lineEdit(), &QLineEdit::returnPressed, this, &CastDatabaseSearchToolBar::Emit);
  connect(m_otherClauseCB->lineEdit(), &QLineEdit::returnPressed, this, &CastDatabaseSearchToolBar::Emit);
}

void CastDatabaseSearchToolBar::Emit() {
  QStringList fullClauseList;
  fullClauseList.reserve(2);
  const QString& nameClause = m_nameClauseCB->currentText();
  if (!nameClause.isEmpty()) {
    fullClauseList.push_back(nameClause);
  }
  const QString& oriClause = m_otherClauseCB->currentText();
  if (!oriClause.isEmpty()) {
    fullClauseList.push_back(oriClause);
  }
  QString fullWhereClause;
  if (!fullClauseList.isEmpty()) {
    fullWhereClause += '(';
    fullWhereClause += fullClauseList.join(R"() AND ()");
    fullWhereClause += ')';
  }
  qDebug("Cast where[%s]", qPrintable(fullWhereClause));
  emit whereClauseChanged(fullWhereClause);
}

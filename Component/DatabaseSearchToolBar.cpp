#include "DatabaseSearchToolBar.h"
#include "public/PublicMacro.h"
#include "Tools/FileDescriptor/TableFields.h"
#include "Tools/FileDescriptor/MountHelper.h"
#include <QLayout>
#include <QStackedWidget>
#include <QToolButton>

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
  return currentText().mid(MountHelper::ROOTPATH_START);
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

DatabaseSearchToolBar::DatabaseSearchToolBar(const QString& title, QWidget* parent)  //
  : QWidget{parent} {
  m_searchLE = new (std::nothrow) QLineEdit;
  CHECK_NULLPTR_RETURN_VOID(m_searchLE);
  m_searchLE->setClearButtonEnabled(true);
  m_searchLE->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);

  m_searchCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_searchCB);
  m_searchCB->setLineEdit(m_searchLE);
  using namespace MOVIE_TABLE;
  m_searchCB->addItem(QString{R"('%1' LIKE "%")"}.arg(ENUM_2_STR(Name)));
  m_searchCB->addItem(QString{R"('%1' BETWEEN 0 AND 1000000)"}.arg(ENUM_2_STR(Size)));
  m_searchCB->addItem(QString{R"('%1' = "E:/")"}.arg(ENUM_2_STR(Driver)));
  m_searchCB->addItem(QString{R"('%1' IN ("Comedy", "Documentary"))"}.arg(ENUM_2_STR(Tags)));
  m_searchCB->addItem(QString{R"('%1' LIKES "%Chris Evans%")"}.arg(ENUM_2_STR(Cast)));
  m_searchCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  m_tables = new (std::nothrow) Guid2RootPathComboxBox;
  CHECK_NULLPTR_RETURN_VOID(m_tables);
  m_tables->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);

  mLo = new QHBoxLayout;
  CHECK_NULLPTR_RETURN_VOID(mLo);
  mLo->addWidget(m_searchCB, 8);
  mLo->addWidget(m_tables, 2);
  setLayout(mLo);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
}

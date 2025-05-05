#include "DatabaseSearchToolBar.h"
#include "public/PublicMacro.h"
#include "Tools/FileDescriptor/TableFields.h"
#include "Tools/FileDescriptor/MountHelper.h"
#include <QLayout>

Guid2RootPathComboxBox::Guid2RootPathComboxBox(QWidget* parent) : QComboBox{parent} {}

void Guid2RootPathComboxBox::AddItem(const QString& guidUnderscore, const QString& rootPath) {
  addItem(guidUnderscore + MountHelper::JOINER_STR + rootPath);
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
    : QToolBar(title, parent) {
  m_tables = new (std::nothrow) Guid2RootPathComboxBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_tables);
  m_searchLE = new (std::nothrow) QLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_searchLE);
  m_searchCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_searchCB);

  m_tables->setEditable(false);
  m_tables->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);

  m_searchLE->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
  m_searchLE->setClearButtonEnabled(true);
  m_searchCB->setLineEdit(m_searchLE);
  using namespace MOVIE_TABLE;
  m_searchCB->addItem(QString{R"(%1 LIKE "%")"}.arg(ENUM_TO_STRING(Name)));
  m_searchCB->addItem(QString{R"(%1 BETWEEN 0 AND 1000000)"}.arg(ENUM_TO_STRING(Size)));
  m_searchCB->addItem(QString{R"(%1 = "E:/")"}.arg(ENUM_TO_STRING(Driver)));
  m_searchCB->addItem(QString{R"(%1 IN ("Comedy", "Documentary"))"}.arg(ENUM_TO_STRING(Tags)));
  m_searchCB->addItem(QString{R"(%1 LIKES "%Chris Evans%")"}.arg(ENUM_TO_STRING(Cast)));
  m_searchCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  addWidget(m_tables);
  addWidget(m_searchCB);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
}

#ifndef NAVIGATIONANDADDRESSBAR_H
#define NAVIGATIONANDADDRESSBAR_H

#include "AddressELineEdit.h"
#include "PathUndoRedoer.h"
#include "PublicVariable.h"

#include "Component/FileSystemTypeFilter.h"

#include <QHBoxLayout>
#include <QToolBar>

#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>

#include <functional>

class NavigationAndAddressBar : public QToolBar {
  Q_OBJECT
 public:
  explicit NavigationAndAddressBar(const QString& title = tr("Address Toolbar"), QWidget* parent = nullptr);

  void BindFileSystemViewCallback(T_IntoNewPath IntoNewPath,
                                  T_on_searchTextChanged on_searchTextChanged,
                                  T_on_searchEnterKey on_searchEnterKey,
                                  QFileSystemModel* _fsm);

  auto InitEventWhenViewChanged() -> void;

  auto onBackward() -> bool;

  auto onForward() -> bool;

  auto onUpTo() -> bool;

  void onGetFocus() {
    m_searchLE->setFocus();
    m_searchLE->selectAll();
  }

  AddressELineEdit* m_addressLine;
  PathUndoRedoer m_pathRD;
  QLineEdit* m_searchLE;

  FileSystemTypeFilter* m_fsFilter;

 private:
  std::function<bool(QString, bool, bool)> m_IntoNewPath;
  std::function<void(QString)> m_on_searchTextChanged;
  std::function<void(QString)> m_on_searchEnterKey;
};

#endif  // NAVIGATIONANDADDRESSBAR_H

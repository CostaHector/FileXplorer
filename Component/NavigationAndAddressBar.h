#ifndef NAVIGATIONANDADDRESSBAR_H
#define NAVIGATIONANDADDRESSBAR_H

#include "AddressELineEdit.h"
#include "public/PathUndoRedoer.h"
#include "public/PublicVariable.h"

#include "Component/FileSystemTypeFilter.h"

#include <QHBoxLayout>
#include <QToolBar>

#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>

#include "Tools/FolderNxtAndLastIterator.h"

class NavigationAndAddressBar : public QToolBar {
 public:
  explicit NavigationAndAddressBar(const QString& title = "Address Toolbar", QWidget* parent = nullptr);

  void BindFileSystemViewCallback(T_IntoNewPath IntoNewPath,
                                  T_on_searchTextChanged on_searchTextChanged,
                                  T_on_searchEnterKey on_searchEnterKey,
                                  QFileSystemModel* _fsm);

  auto InitEventWhenViewChanged() -> void;

  auto onBackward() -> bool;

  auto onForward() -> bool;

  auto onUpTo() -> bool;

  bool onIteratorToNextFolder();
  bool onIteratorToLastFolder();

  bool onSearchTextChanged();
  bool onSearchTextReturnPressed();

  void onGetFocus() {
    m_searchLE->setFocus();
    m_searchLE->selectAll();
  }

  AddressELineEdit* m_addressLine{nullptr};
  PathUndoRedoer m_pathRD;
  FolderNxtAndLastIterator mFolderNxtLstIt;
  QLineEdit* m_searchLE{nullptr};
  FileSystemTypeFilter* m_fsFilter{nullptr};

 private:
  bool onIteratorToAnotherFolderCore(bool isNext);

  T_IntoNewPath m_IntoNewPath{nullptr};
  T_on_searchTextChanged m_on_searchTextChanged{nullptr};
  T_on_searchEnterKey m_on_searchEnterKey{nullptr};
};

#endif  // NAVIGATIONANDADDRESSBAR_H

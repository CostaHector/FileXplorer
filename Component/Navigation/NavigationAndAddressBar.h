#ifndef NAVIGATIONANDADDRESSBAR_H
#define NAVIGATIONANDADDRESSBAR_H

#include "AddressELineEdit.h"
#include "PathUndoRedoer.h"
#include "PublicVariable.h"
#include "TypeFilterButton.h"
#include "FolderNxtAndLastIterator.h"

#include <QLineEdit>
#include <QToolBar>

class MovieWallServer;
class QFileSystemModel;

class NavigationAndAddressBar : public QToolBar {
  Q_OBJECT
public:
  explicit NavigationAndAddressBar(const QString& title, QWidget* parent);

  void BindFileSystemViewCallback(T_IntoNewPath IntoNewPath,
                                  T_on_searchTextChanged on_searchTextChanged,
                                  T_on_searchEnterKey on_searchEnterKey,
                                  QFileSystemModel* _fsm);

  void InitEventWhenViewChanged();

  bool onBackward();
  bool onForward();
  bool onUpTo();

  bool onIteratorToNextFolder();
  bool onIteratorToLastFolder();

  bool onSearchTextChanged();
  bool onSearchTextReturnPressed();

  void onGetFocus() {
    mFsSearchLE->setFocus();
    mFsSearchLE->selectAll();
  }

  AddressELineEdit* m_addressLine{nullptr};
  QAction* mOpenMovieWall{nullptr};
  PathUndoRedoer m_pathRD;
  FolderNxtAndLastIterator mFolderNxtLstIt;
  TypeFilterButton* m_fsFilterBtn{nullptr};
  QLineEdit* mFsSearchLE{nullptr};

private:
  bool onIteratorToAnotherFolderCore(bool isNext);
  QAction* mIterPrevFolderAct{nullptr}, *mIterNextFolderAct{nullptr};
  T_IntoNewPath m_IntoNewPath{nullptr};
  T_on_searchTextChanged m_on_searchTextChanged{nullptr};
  T_on_searchEnterKey m_on_searchEnterKey{nullptr};

  bool onToggleMovieWall(bool bLauchServer);
  MovieWallServer* mMovieWallServer{nullptr};
};

#endif  // NAVIGATIONANDADDRESSBAR_H

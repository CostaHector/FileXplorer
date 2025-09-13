#ifndef NAVIGATIONANDADDRESSBAR_H
#define NAVIGATIONANDADDRESSBAR_H

#include <QFileSystemModel>
#include "AddressELineEdit.h"
#include "PathUndoRedoer.h"
#include "PublicVariable.h"
#include "TypeFilterButton.h"
#include "FolderNxtAndLastIterator.h"

#include <QLineEdit>
#include <QToolBar>
#include <QToolButton>
#include <QShortcut>

class SplitToolButton : public QToolButton {
  Q_OBJECT
public:
  explicit SplitToolButton(QWidget *parent = nullptr);
  QString GetShortcutString(const QString& topAction, const QString& bottomAction) const;
signals:
  void topHalfClicked();
  void bottomHalfClicked();

protected:
  void mousePressEvent(QMouseEvent *event) override {
    if (event->y() < height() / 2) {
      emit topHalfClicked();
    } else {
      emit bottomHalfClicked();
    }
    QToolButton::mousePressEvent(event);
  }
private:
  QShortcut* topShortcut{nullptr}, *bottomShortcut{nullptr};
};

class NavigationAndAddressBar : public QToolBar {
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
  PathUndoRedoer m_pathRD;
  FolderNxtAndLastIterator mFolderNxtLstIt;
  QLineEdit* mFsSearchLE{nullptr};
  TypeFilterButton* m_fsFilterBtn{nullptr};

private:
  bool onIteratorToAnotherFolderCore(bool isNext);
  SplitToolButton* mLastNextFolderTb{nullptr};
  T_IntoNewPath m_IntoNewPath{nullptr};
  T_on_searchTextChanged m_on_searchTextChanged{nullptr};
  T_on_searchEnterKey m_on_searchEnterKey{nullptr};
};

#endif  // NAVIGATIONANDADDRESSBAR_H

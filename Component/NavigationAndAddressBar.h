#ifndef NAVIGATIONANDADDRESSBAR_H
#define NAVIGATIONANDADDRESSBAR_H

#include "AddressELineEdit.h"
#include "PathUndoRedoer.h"
#include "PublicVariable.h"

#include <QHBoxLayout>
#include <QToolBar>

#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>

#include <functional>

class NavigationAndAddressBar : public QToolBar {
  Q_OBJECT
 public:
  explicit NavigationAndAddressBar(const QString &title=tr("Address Toolbar"), QWidget* parent = nullptr);

  void subscribe(T_IntoNewPath IntoNewPath, T_on_searchTextChanged on_searchTextChanged, T_on_searchEnterKey on_searchEnterKey);

  auto InitEventWhenViewChanged() -> void;

  auto onBackward() -> bool;

  auto onForward() -> bool;
  
  auto onUpTo() -> bool;

  AddressELineEdit* m_addressLine;
  PathUndoRedoer m_pathRD;
  QLineEdit* m_searchLE;

  QAction* _FILE = new QAction(QIcon(":/themes/FILE"), tr("file"));
  QAction* _FOLDER  = new QAction(QIcon(":/themes/FOLDER"), tr("folder"));
  QAction* _HIDDEN = new QAction(QIcon(":/themes/HIDDEN"), tr("hidden"));
  QAction* _DOTDOT = new QAction(QIcon(":/themes/DOT_DOT"), tr("dotdot"));
  QAction* _IMAGES = new QAction(QIcon(":/themes/IMAGE"), tr("image"));
  QAction* _VIDEOS = new QAction(QIcon(":/themes/VIDEO"), tr("video"));
  QAction* _PLAIN_TEXT = new QAction(QIcon(":/themes/PLAIN_TEXT"), tr("plain Text"));
  QAction* _DOCUMENT = new QAction(QIcon(":/themes/DOCUMENT"), tr("document"));
  QAction* _EXE = new QAction(QIcon(":/themes/EXE"), tr("executable"));

  QMenu* m_itemTypeMenu;
  QToolButton* m_fsFilter;

 private:
  std::function<bool(QString, bool, bool)> m_IntoNewPath;
  std::function<void(QString)> m_on_searchTextChanged;
  std::function<void(QString)> m_on_searchEnterKey;
};

#endif  // NAVIGATIONANDADDRESSBAR_H

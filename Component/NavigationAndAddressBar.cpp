#include "NavigationAndAddressBar.h"
#include "Actions/AddressBarActions.h"
#include "Actions/RightClickMenuActions.h"
#include "public/PublicMacro.h"
#include <QHBoxLayout>

NavigationAndAddressBar::NavigationAndAddressBar(const QString& title, QWidget* parent) : QToolBar{title, parent} {
  m_addressLine = new (std::nothrow) AddressELineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_addressLine)
  m_addressLine->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  mFsSearchLE = new (std::nothrow) QLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(mFsSearchLE)
  mFsSearchLE->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
  mFsSearchLE->setClearButtonEnabled(true);
  mFsSearchLE->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
  mFsSearchLE->setToolTip(
      "For FileSystemModel(wildcard) e.g., *target*\n"
      "For SceneModel(plain) e.g., target\n"
      "For JsonModel(Regex) e.g., target1.*?target2");
  mFsSearchLE->setMinimumWidth(40);

  m_fsFilter = new (std::nothrow) FileSystemTypeFilter;
  CHECK_NULLPTR_RETURN_VOID(m_fsFilter)

  addActions(g_addressBarActions().ADDRESS_CONTROLS->actions());
  addSeparator();
  addWidget(m_addressLine);
  addAction(g_rightClickActions()._FORCE_REFRESH_FILESYSTEMMODEL);
  addSeparator();
  addActions(g_addressBarActions()._FOLDER_ITER_CONTROLS->actions());
  addSeparator();
  addWidget(m_fsFilter);
  addSeparator();
  addWidget(mFsSearchLE);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
  InitEventWhenViewChanged();
}

void NavigationAndAddressBar::BindFileSystemViewCallback(T_IntoNewPath IntoNewPath, T_on_searchTextChanged on_searchTextChanged, T_on_searchEnterKey on_searchEnterKey, QFileSystemModel* _fsm) {
  m_IntoNewPath = IntoNewPath;
  m_on_searchTextChanged = on_searchTextChanged;
  m_on_searchEnterKey = on_searchEnterKey;
  m_fsFilter->BindFileSystemModel(_fsm);
}

void NavigationAndAddressBar::InitEventWhenViewChanged() {
  connect(g_addressBarActions()._BACK_TO, &QAction::triggered, this, &NavigationAndAddressBar::onBackward);
  connect(g_addressBarActions()._FORWARD_TO, &QAction::triggered, this, &NavigationAndAddressBar::onForward);
  connect(g_addressBarActions()._UP_TO, &QAction::triggered, this, &NavigationAndAddressBar::onUpTo);

  connect(g_addressBarActions()._LAST_FOLDER, &QAction::triggered, this, &NavigationAndAddressBar::onIteratorToLastFolder);
  connect(g_addressBarActions()._NEXT_FOLDER, &QAction::triggered, this, &NavigationAndAddressBar::onIteratorToNextFolder);

  connect(mFsSearchLE, &QLineEdit::textChanged, this, &NavigationAndAddressBar::onSearchTextChanged);
  connect(mFsSearchLE, &QLineEdit::returnPressed, this, &NavigationAndAddressBar::onSearchTextReturnPressed);
}

bool NavigationAndAddressBar::onBackward() {
  if (!m_pathRD.undoAvailable()) {
    qDebug("[Skip] backward paths pool empty");
    return false;
  }
  bool backwardRes{true};
  const QString& undoPath = m_pathRD.undo();
  if (m_IntoNewPath != nullptr) {
    backwardRes = m_IntoNewPath(undoPath, false, false);
  }
  return backwardRes;
}

bool NavigationAndAddressBar::onForward() {
  if (!m_pathRD.redoAvailable()) {
    qDebug("[Skip] Forward paths pool empty");
    return false;
  }
  bool forwardRes{true};
  const QString& redoPath = m_pathRD.redo();
  if (m_IntoNewPath != nullptr) {
    forwardRes = m_IntoNewPath(redoPath, false, false);
  }
  return forwardRes;
}

bool NavigationAndAddressBar::onUpTo() {
  const QString& upPath = m_addressLine->dirname();
  bool upRes{true};
  if (m_IntoNewPath != nullptr) {
    upRes = m_IntoNewPath(upPath, true, false);
  }
  return upRes;
}

bool NavigationAndAddressBar::onIteratorToAnotherFolderCore(bool isNext) {
  const QFileInfo fi{m_addressLine->pathFromLineEdit()};
  const QString parentPath = fi.absolutePath();
  const QString curDirName = fi.fileName();
  const QString& newDir = isNext ? mFolderNxtLstIt.next(parentPath, curDirName) : mFolderNxtLstIt.last(parentPath, curDirName);
  if (newDir.isEmpty()) {
    qDebug("skip empty dir[%s]", qPrintable(parentPath));
    return false;
  }
  if (curDirName == newDir) {
    qDebug("skip same dir[%s/%s]", qPrintable(parentPath), qPrintable(curDirName));
    return false;
  }
  const QString& newPath = parentPath + '/' + newDir;
  bool intoRes{true};
  if (m_IntoNewPath != nullptr) {
    intoRes = m_IntoNewPath(newPath, true, false);
  } else {
    qWarning("m_IntoNewPath is nullptr");
    return false;
  }
  return intoRes;
}

bool NavigationAndAddressBar::onIteratorToNextFolder() {
  return onIteratorToAnotherFolderCore(true);
}
bool NavigationAndAddressBar::onIteratorToLastFolder() {
  return onIteratorToAnotherFolderCore(false);
}

bool NavigationAndAddressBar::onSearchTextChanged() {
  if (m_on_searchTextChanged != nullptr) {
    m_on_searchTextChanged(mFsSearchLE->text());
  }
  return true;
}
bool NavigationAndAddressBar::onSearchTextReturnPressed() {
  if (m_on_searchEnterKey != nullptr) {
    m_on_searchEnterKey(mFsSearchLE->text());
  }
  return true;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

class IntoNewPathMockClass {
 public:
  bool IntoNewPath(QString a, bool b, bool c) {
    qDebug("IntoNewPath: %s, %d, %d", qPrintable(a), b, c);
    return true;
  }
  bool on_searchTextChanged(QString a) {
    qDebug("on_searchTextChanged: %s, %d", qPrintable(a));
    return true;
  }
  bool on_searchEnterKey(QString a) {
    qDebug("on_searchEnterKey: %s, %d", qPrintable(a));
    return true;
  }
};

int main(int argc, char* argv[]) {
  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;

  QApplication a(argc, argv);

  IntoNewPathMockClass printer;
  auto f1 = std::bind(&IntoNewPathMockClass::IntoNewPath, &printer, _1, _2, _3);
  auto f2 = std::bind(&IntoNewPathMockClass::on_searchTextChanged, &printer, _1);
  auto f3 = std::bind(&IntoNewPathMockClass::on_searchEnterKey, &printer, _1);

  NavigationAndAddressBar lo(f1, f2, f3);

  QWidget w;
  w.setLayour(lo);

  w.show();
  w.winAddress->UpdatePath(QFileInfo(__FILE__).absolutePath());
  return a.exec();
  ;
}
#endif

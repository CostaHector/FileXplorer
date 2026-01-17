#include "NavigationAndAddressBar.h"
#include "AddressBarActions.h"
#include "RightClickMenuActions.h"
#include "PublicMacro.h"
#include <QLayout>
#include <QKeySequence>

SplitToolButton::SplitToolButton(QWidget* parent) : QToolButton{parent} {
  topShortcut = new (std::nothrow) QShortcut{QKeySequence{Qt::ControlModifier | Qt::Key_BracketLeft}, this};
  CHECK_NULLPTR_RETURN_VOID(topShortcut)
  connect(topShortcut, &QShortcut::activated, this, &SplitToolButton::topHalfClicked);

  bottomShortcut = new (std::nothrow) QShortcut{QKeySequence{Qt::ControlModifier | Qt::Key_BracketRight}, this};
  CHECK_NULLPTR_RETURN_VOID(bottomShortcut)
  connect(bottomShortcut, &QShortcut::activated, this, &SplitToolButton::bottomHalfClicked);
}

QString SplitToolButton::GetToolTipString(const QString& topAction, const QString& bottomAction) const {
  QString shortCutString;
  shortCutString.reserve(topAction.size() + bottomAction.size() + 30);
  shortCutString += QString("<b>%1</b><br/>").arg(text());
  shortCutString += topAction;
  shortCutString += "(";
  shortCutString += topShortcut->key().toString();
  shortCutString += ")";
  shortCutString += "<br/>";
  shortCutString += bottomAction;
  shortCutString += "(";
  shortCutString += bottomShortcut->key().toString();
  shortCutString += ")";
  return shortCutString;
}

void SplitToolButton::mousePressEvent(QMouseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  if (event->y() < height() / 2) {
    emit topHalfClicked();
  } else {
    emit bottomHalfClicked();
  }
  event->accept();
}

NavigationAndAddressBar::NavigationAndAddressBar(const QString& title, QWidget* parent)  //
    : QToolBar{title, parent} {
  CHECK_NULLPTR_RETURN_VOID(parent)

  m_addressLine = new (std::nothrow) AddressELineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_addressLine)
  m_addressLine->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  mFsSearchLE = new (std::nothrow) QLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(mFsSearchLE)
  mFsSearchLE->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
  mFsSearchLE->setClearButtonEnabled(true);
  mFsSearchLE->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
  mFsSearchLE->setToolTip(
      QString("<b>%1</b><br/>").arg(tr("Search")) +
      "For FileSystemModel(wildcard) e.g., *target*\n"
      "For SceneModel(plain) e.g., target\n"
      "For JsonModel(Regex) e.g., target1.*?target2");

  m_fsFilterBtn = new (std::nothrow) TypeFilterButton{ModelFilterE::FILE_SYSTEM, this};
  CHECK_NULLPTR_RETURN_VOID(m_fsFilterBtn)

  mLastNextFolderTb = new (std::nothrow) SplitToolButton{this};
  CHECK_NULLPTR_RETURN_VOID(mLastNextFolderTb)
  mLastNextFolderTb->setText(tr("Folder Iterater"));
  mLastNextFolderTb->setIcon(QIcon(":img/NEXT_OR_LAST_FOLDER"));
  const QString& iteratorToolbuttonTooltip = mLastNextFolderTb->GetToolTipString("Last folder", "Next Folder");
  mLastNextFolderTb->setToolTip(iteratorToolbuttonTooltip);

  addActions(g_addressBarActions().ADDRESS_CONTROLS->actions());
  addSeparator();
  addWidget(m_addressLine);
  addAction(g_rightClickActions()._FORCE_REFRESH_FILESYSTEMMODEL);
  addSeparator();
  addWidget(mLastNextFolderTb);
  addSeparator();
  addWidget(m_fsFilterBtn);
  addSeparator();
  addWidget(mFsSearchLE);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
  InitEventWhenViewChanged();
}

void NavigationAndAddressBar::BindFileSystemViewCallback(T_IntoNewPath IntoNewPath,
                                                         T_on_searchTextChanged on_searchTextChanged,
                                                         T_on_searchEnterKey on_searchEnterKey,
                                                         QFileSystemModel* _fsm) {
  m_IntoNewPath = IntoNewPath;
  m_on_searchTextChanged = on_searchTextChanged;
  m_on_searchEnterKey = on_searchEnterKey;

  // initial
  _fsm->setFilter(m_fsFilterBtn->curDirFilters());
  _fsm->setNameFilterDisables(m_fsFilterBtn->curGrayOrHideUnpassItem());

  // subscribe
  connect(m_fsFilterBtn, &TypeFilterButton::filterChanged, _fsm, &QFileSystemModel::setFilter);
  connect(m_fsFilterBtn, &TypeFilterButton::nameFilterDisablesChanged, _fsm, &QFileSystemModel::setNameFilterDisables);
}

void NavigationAndAddressBar::InitEventWhenViewChanged() {
  connect(g_addressBarActions()._BACK_TO, &QAction::triggered, this, &NavigationAndAddressBar::onBackward);
  connect(g_addressBarActions()._FORWARD_TO, &QAction::triggered, this, &NavigationAndAddressBar::onForward);
  connect(g_addressBarActions()._UP_TO, &QAction::triggered, this, &NavigationAndAddressBar::onUpTo);

  connect(mLastNextFolderTb, &SplitToolButton::topHalfClicked, this, &NavigationAndAddressBar::onIteratorToLastFolder);
  connect(mLastNextFolderTb, &SplitToolButton::bottomHalfClicked, this, &NavigationAndAddressBar::onIteratorToNextFolder);

  connect(mFsSearchLE, &QLineEdit::textChanged, this, &NavigationAndAddressBar::onSearchTextChanged);
  connect(mFsSearchLE, &QLineEdit::returnPressed, this, &NavigationAndAddressBar::onSearchTextReturnPressed);
}

bool NavigationAndAddressBar::onBackward() {
  if (!m_pathRD.undoPathAvailable()) {
    LOG_D("[Skip] backward paths pool empty");
    return false;
  }
  bool backwardRes{false};
  const QString& undoPath = m_pathRD.onUndoPath();
  if (m_IntoNewPath != nullptr) {
    backwardRes = m_IntoNewPath(undoPath, false, false);
  }
  return backwardRes;
}

bool NavigationAndAddressBar::onForward() {
  if (!m_pathRD.redoPathAvailable()) {
    LOG_D("[Skip] Forward paths pool empty");
    return false;
  }
  bool forwardRes{false};
  const QString& redoPath = m_pathRD.onRedoPath();
  if (m_IntoNewPath != nullptr) {
    forwardRes = m_IntoNewPath(redoPath, false, false);
  }
  return forwardRes;
}

bool NavigationAndAddressBar::onUpTo() {
  bool isParentSameAsCurrent = false;
  const QString& parentPath = m_addressLine->parentPath(&isParentSameAsCurrent);
  if (isParentSameAsCurrent) {
    return false;
  }

  bool upRes{false};
  if (m_IntoNewPath != nullptr) {
    upRes = m_IntoNewPath(parentPath, true, false);
  }
  return upRes;
}

bool NavigationAndAddressBar::onIteratorToAnotherFolderCore(bool isNext) {
  const QFileInfo fi{m_addressLine->pathFromLineEdit()};
  const QString parentPath = fi.absolutePath();
  const QString curDirName = fi.fileName();
  const QString& newDir = isNext ? mFolderNxtLstIt.next(parentPath, curDirName) : mFolderNxtLstIt.last(parentPath, curDirName);
  if (newDir.isEmpty()) {
    LOG_D("skip empty dir[%s]", qPrintable(parentPath));
    return false;
  }
  if (curDirName == newDir) {
    LOG_D("skip same dir[%s/%s]", qPrintable(parentPath), qPrintable(curDirName));
    return false;
  }
  const QString& newPath = parentPath + '/' + newDir;
  bool intoRes{false};
  if (m_IntoNewPath != nullptr) {
    intoRes = m_IntoNewPath(newPath, true, false);
  } else {
    LOG_W("m_IntoNewPath is nullptr");
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

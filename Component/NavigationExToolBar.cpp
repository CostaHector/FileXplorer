#include "NavigationExToolBar.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include "StyleSheet.h"

#include <QDragEnterEvent>
#include <QToolTip>
#include <QToolButton>
#include <QMenu>

#include <QActionGroup>
#include <QMimeData>
#include <QFileInfo>
#include <QApplication>
#include <QLayout>
#include <QStyle>

T_IntoNewPath NavigationExToolBar::m_IntoNewPath{nullptr};
constexpr char NavigationExToolBar::EXTRA_NAVI_DICT[];
constexpr char NavigationExToolBar::EXTRA_NAVI_DICT_KEY[];
constexpr char NavigationExToolBar::EXTRA_NAVI_DICT_VALUE[];

// toolbar
NavigationExToolBar::NavigationExToolBar(const QString& title, QWidget* parent)  //
    : ReorderableToolBar{title, parent}                                          //
{
  setObjectName(title);

  UNPIN_THIS = new (std::nothrow) QAction{QIcon{":img/UNPIN"}, "Unpin this", this};
  CHECK_NULLPTR_RETURN_VOID(UNPIN_THIS);
  UNPIN_ALL = new (std::nothrow) QAction{QIcon{":img/UNPIN_ALL"}, "Unpin All", this};
  CHECK_NULLPTR_RETURN_VOID(UNPIN_ALL);

  mMenu = new (std::nothrow) QMenu{"pin and unpin", this};
  CHECK_NULLPTR_RETURN_VOID(mMenu);
  mMenu->addAction(UNPIN_THIS);
  mMenu->addSeparator();
  mMenu->addAction(UNPIN_ALL);

  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
  setOrientation(Qt::Vertical);
  setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);

  // setContextMenuPolicy(Qt::CustomContextMenu);
  // connect(this, &QToolBar::customContextMenuRequested, this, &NavigationExToolBar::CustomContextMenuEvent);

  setAcceptDrops(true);

  ReadSettings();
  Subscribe();
}

void NavigationExToolBar::dragEnterEvent(QDragEnterEvent* event) {
  const QMimeData* pMimeData = event->mimeData();
  CHECK_NULLPTR_RETURN_VOID(pMimeData);
  if (pMimeData->hasUrls()) {
    LOG_D("mimeData urls cnt[%d]", pMimeData->urls().size());
    event->accept();
    return;
  }
  ReorderableToolBar::dragEnterEvent(event);
}

void NavigationExToolBar::dropEvent(QDropEvent* event) {
  auto* pMimeData = event->mimeData();
  if (pMimeData->hasUrls()) {
    const auto& urlsLst = pMimeData->urls();
    QMap<QString, QString> folderName2AbsPath;
    foreach (const QUrl& url, urlsLst) {
      const QString pth{url.toLocalFile()};
      const QFileInfo fi{pth};
      if (!fi.isDir()) {
        continue;
      }
      const QString nameShown{fi.isRoot() ? pth : fi.completeBaseName()};
      folderName2AbsPath[nameShown] = fi.absoluteFilePath();
    }
    LOG_D("%d link action(s) dropped here...", folderName2AbsPath.size());
    AppendExtraActions(folderName2AbsPath);
    SaveName2PathLink();
    event->accept();
    return;
  }
  ReorderableToolBar::dropEvent(event);
}

// accept drag movements only if the target supports drops
void NavigationExToolBar::dragMoveEvent(QDragMoveEvent* event) {
  if (event->mimeData()->hasUrls()) {
    event->accept();
    return;
  }
  ReorderableToolBar::dragMoveEvent(event);
}

void NavigationExToolBar::SaveName2PathLink() {
  const QList<QAction*>& actsList = actions();
  Configuration().beginWriteArray(EXTRA_NAVI_DICT, actsList.size());
  int extraIndex = 0;

  foreach (QAction* pAct, actsList) {
    const QWidget* pWid = widgetForAction(pAct);
    if (pWid == nullptr) {
      continue;
    }
    const QToolButton* pToolButton = qobject_cast<const QToolButton*>(pWid);
    if (pToolButton == nullptr) {
      continue;
    }
    const QAction* pPathAct = pToolButton->defaultAction();
    if (pPathAct == nullptr) {
      continue;
    }
    Configuration().setArrayIndex(extraIndex);
    Configuration().setValue(EXTRA_NAVI_DICT_KEY, pPathAct->text());
    Configuration().setValue(EXTRA_NAVI_DICT_VALUE, pPathAct->toolTip());
    ++extraIndex;
  }
  Configuration().endArray();
}

void NavigationExToolBar::ReadSettings() {
  int lnkCnt = Configuration().beginReadArray(EXTRA_NAVI_DICT);
  QMap<QString, QString> folderName2AbsPath;
  for (int extraIndex = 0; extraIndex < lnkCnt; ++extraIndex) {
    Configuration().setArrayIndex(extraIndex);
    folderName2AbsPath[Configuration().value(EXTRA_NAVI_DICT_KEY).toString()]  //
        = Configuration().value(EXTRA_NAVI_DICT_VALUE).toString();
  }
  Configuration().endArray();
  AppendExtraActions(folderName2AbsPath);
}

void NavigationExToolBar::Subscribe() {
  connect(UNPIN_THIS, &QAction::triggered, this, &NavigationExToolBar::UnpinThis);
  connect(UNPIN_ALL, &QAction::triggered, this, &NavigationExToolBar::UnpinAll);
  connect(mCollectPathAgs, &QActionGroup::triggered, this, &NavigationExToolBar::onPathActionTriggered);
}

bool NavigationExToolBar::UnpinThis() {
  if (mRightClickAtAction == nullptr) {
    return false;
  }
  if (!actions().contains(mRightClickAtAction)) {
    return false;
  }
  removeAction(mRightClickAtAction);
  SaveName2PathLink();
  mRightClickAtAction = nullptr;
  return true;
}

int NavigationExToolBar::UnpinAll() {
  QList<QAction*> actLst = actions();
  foreach (QAction* act, actLst) {
    removeAction(act);
  }
  SaveName2PathLink();
  mRightClickAtAction = nullptr;
  return actLst.size();
}

void NavigationExToolBar::contextMenuEvent(QContextMenuEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  event->accept();
  CHECK_NULLPTR_RETURN_VOID(mMenu);
  mRightClickAtAction = actionAt(event->pos());
#ifndef RUNNING_UNIT_TESTS
  mMenu->popup(event->globalPos());
#endif
}

void NavigationExToolBar::AppendExtraActions(const QMap<QString, QString>& folderName2AbsPath) {
  static const QIcon dirIcon = QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirIcon);
  for (auto it = folderName2AbsPath.cbegin(); it != folderName2AbsPath.cend(); ++it) {
    const QString& folderName = it.key();
    const QString& absPath = it.value();
    QAction* pCollectionAct = new (std::nothrow) QAction{dirIcon, folderName, this};
    CHECK_NULLPTR_RETURN_VOID(pCollectionAct)
    pCollectionAct->setToolTip(absPath);
    addAction(pCollectionAct);
  }
  SetLayoutAlightment(layout(), Qt::AlignmentFlag::AlignLeft);
}

bool NavigationExToolBar::onPathActionTriggered(const QAction* pAct) {
  CHECK_NULLPTR_RETURN_FALSE(m_IntoNewPath);
  CHECK_NULLPTR_RETURN_FALSE(pAct);
  return m_IntoNewPath(pAct->toolTip(), true, true);
}

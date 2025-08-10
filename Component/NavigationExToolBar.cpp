#include "NavigationExToolBar.h"
#include "PublicMacro.h"
#include "MemoryKey.h"

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

T_IntoNewPath NavigationExToolBar::m_IntoNewPath;

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

  setContextMenuPolicy(Qt::CustomContextMenu);
  setAcceptDrops(true);

  ReadSettings();
  Subscribe();
}

void NavigationExToolBar::dragEnterEvent(QDragEnterEvent* event) {
  const QMimeData* pMimeData = event->mimeData();
  CHECK_NULLPTR_RETURN_VOID(pMimeData);
  if (pMimeData->hasUrls()) {
    qDebug("mimeData urls cnt[%d]", pMimeData->urls().size());
    event->acceptProposedAction();
  } else if (pMimeData->hasText()) {
    ReorderableToolBar::dragEnterEvent(event);
    return;
  }
  event->accept();
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
    qDebug("%d link action(s) dropped here...", folderName2AbsPath.size());
    AppendExtraActions(folderName2AbsPath);
    SaveName2PathLink();
  } else {
    ReorderableToolBar::dropEvent(event);
    return;
  }
  QToolBar::dropEvent(event);
}

// accept drag movements only if the target supports drops
void NavigationExToolBar::dragMoveEvent(QDragMoveEvent* event) {
  if (event->mimeData()->hasText()) {
    ReorderableToolBar::dragMoveEvent(event);
    return;
  } else if (event->mimeData()->hasUrls()) {
    // no text has url?
    event->acceptProposedAction();
    QToolBar::dragMoveEvent(event);
  }
}

void NavigationExToolBar::SaveName2PathLink() {
  const QList<QAction*>& actsList = actions();
  PreferenceSettings().beginWriteArray("ExtraNavigationDict", actsList.size());
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
    PreferenceSettings().setArrayIndex(extraIndex);
    PreferenceSettings().setValue("folderName", pPathAct->text());
    PreferenceSettings().setValue("absPath", pPathAct->toolTip());
    ++extraIndex;
  }
  PreferenceSettings().endArray();
}

void NavigationExToolBar::ReadSettings() {
  int lnkCnt = PreferenceSettings().beginReadArray("ExtraNavigationDict");
  QMap<QString, QString> folderName2AbsPath;
  for (int extraIndex = 0; extraIndex < lnkCnt; ++extraIndex) {
    PreferenceSettings().setArrayIndex(extraIndex);
    folderName2AbsPath[PreferenceSettings().value("folderName").toString()]  //
        = PreferenceSettings().value("absPath").toString();
  }
  PreferenceSettings().endArray();
  AppendExtraActions(folderName2AbsPath);
}

void NavigationExToolBar::Subscribe() {
  connect(this, &QToolBar::customContextMenuRequested, this, &NavigationExToolBar::CustomContextMenuEvent);
  connect(UNPIN_THIS, &QAction::triggered, this, &NavigationExToolBar::UnpinThis);
  connect(UNPIN_ALL, &QAction::triggered, this, &NavigationExToolBar::UnpinAll);
  connect(mCollectPathAgs, &QActionGroup::triggered, this, &NavigationExToolBar::onPathActionTriggered);
}

void NavigationExToolBar::UnpinThis() {
  QAction* act = actionAt(mRightClickAtPnt);
  if (actions().contains(act)) {
    removeAction(act);
  }
  SaveName2PathLink();
}

void NavigationExToolBar::UnpinAll() {
  foreach (QAction* act, actions()) {
    removeAction(act);
  }
  SaveName2PathLink();
}

void NavigationExToolBar::CustomContextMenuEvent(const QPoint& pnt) {
  CHECK_NULLPTR_RETURN_VOID(mMenu);
  mMenu->popup(mapToGlobal(pnt));
  mRightClickAtPnt = pnt;
}

void NavigationExToolBar::AlighLeft() {
  for (int i = 0; i < layout()->count(); ++i) {
    layout()->itemAt(i)->setAlignment(Qt::AlignmentFlag::AlignLeft);
  }
}

void NavigationExToolBar::AppendExtraActions(const QMap<QString, QString>& folderName2AbsPath) {
  static const auto dirIcon = QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirIcon);
  for (auto it = folderName2AbsPath.cbegin(); it != folderName2AbsPath.cend(); ++it) {
    const QString& folderName = it.key();
    const QString& absPath = it.value();
    QAction* pCollectionAct = new (std::nothrow) QAction{dirIcon, folderName, this};
    CHECK_NULLPTR_RETURN_VOID(pCollectionAct)
    pCollectionAct->setToolTip(absPath);
    addAction(pCollectionAct);
    //    connect(pCollectionAct, &QAction::triggered, [pCollectionAct]() {  //
    //      NavigationExToolBar::onPathActionTriggered(pCollectionAct);      //
    //    });
  }
  AlighLeft();
}

bool NavigationExToolBar::onPathActionTriggered(const QAction* pAct) {
  CHECK_NULLPTR_RETURN_FALSE(m_IntoNewPath);
  CHECK_NULLPTR_RETURN_FALSE(pAct);
  return m_IntoNewPath(pAct->toolTip(), true, true);
}

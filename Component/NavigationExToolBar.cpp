#include "NavigationExToolBar.h"
#include "Actions/ActionWithPath.h"
#include "public/PublicMacro.h"
#include "public/PublicVariable.h"

#include <QMimeData>
#include <QFileInfo>
#include <QApplication>
#include <QLayout>
#include <QMenu>
#include <QStyle>

NavigationExToolBar::NavigationExToolBar(const QString& title, QWidget* parent)  //
    : QToolBar{title, parent} {
  setObjectName(title);

  UNPIN_THIS = new (std::nothrow) QAction{QIcon{":img/UNPIN"}, "Unpin this", this};
  CHECK_NULLPTR_RETURN_VOID(UNPIN_THIS);
  UNPIN_ALL = new (std::nothrow) QAction{"Unpin All", this};
  CHECK_NULLPTR_RETURN_VOID(UNPIN_ALL);

  mMenu = new (std::nothrow) QMenu{this};
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
  qDebug("mimeData urls cnt[%d]", pMimeData->urls().size());
  event->accept();
}

void NavigationExToolBar::dropEvent(QDropEvent* event) {
  if (!event->mimeData()->hasUrls()) {
    return;
  }
  const auto& urlsLst = event->mimeData()->urls();
  QMap<QString, QString> folderName2AbsPath;
  foreach (const QUrl& url, urlsLst) {
    const QString pth{url.toLocalFile()};
    const QFileInfo fi(pth);
    if (!fi.isDir()) {
      continue;
    }
    const QString nameShown{fi.isRoot() ? pth : fi.completeBaseName()};
    folderName2AbsPath[nameShown] = fi.absoluteFilePath();
  }
  qDebug("%d link action(s) dropped here...", folderName2AbsPath.size());
  AppendExtraActions(folderName2AbsPath);
  SaveName2PathLink();
  return QToolBar::dropEvent(event);
}

// accept drag movements only if the target supports drops
void NavigationExToolBar::dragMoveEvent(QDragMoveEvent* event) {
  if (!event->mimeData()->hasUrls()) {
    return;
  }
  return QToolBar::dragMoveEvent(event);
}

void NavigationExToolBar::SaveName2PathLink() {
  const QList<QAction*>& actsList = actions();
  PreferenceSettings().beginWriteArray("ExtraNavigationDict", actsList.size());
  int extraIndex = 0;
  foreach (const QAction* pAct, actsList) {
    PreferenceSettings().setArrayIndex(extraIndex);
    PreferenceSettings().setValue("folderName", pAct->text());
    PreferenceSettings().setValue("absPath", pAct->toolTip());
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
  for (auto it = folderName2AbsPath.cbegin(); it != folderName2AbsPath.cend(); ++it) {
    const QString& folderName = it.key();
    const QString& absPath = it.value();
    QAction* TEMP_ACTIONS = new ActionWithPath{absPath, QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirIcon), folderName, this};
    addAction(TEMP_ACTIONS);
  }
  AlighLeft();
}

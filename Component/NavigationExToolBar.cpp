#include "NavigationExToolBar.h"
#include "public/PublicMacro.h"
#include "public/MemoryKey.h"
#include "View/ViewHelper.h"

#include <QToolTip>
#include <QToolButton>
#include <QMenu>

#include <QMimeData>
#include <QFileInfo>
#include <QApplication>
#include <QLayout>
#include <QStyle>
#include <QDrag>
#include <QPainter>

class DraggableButton : public QToolButton {
 public:
  DraggableButton(QAction* act, QWidget* parent = nullptr);

 protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

 private:
  QPoint mDragStartPosition;
};

// toolbutton
DraggableButton::DraggableButton(QAction* act, QWidget* parent) : QToolButton(parent), mDragStartPosition(QPoint()) {
  setDefaultAction(act);
  setToolButtonStyle(Qt::ToolButtonTextOnly);
  setAcceptDrops(false);
}

void DraggableButton::mousePressEvent(QMouseEvent* event) {
  if (event->button() & Qt::LeftButton) {
    mDragStartPosition = event->pos();
  }
  QToolButton::mousePressEvent(event);
}

void DraggableButton::mouseMoveEvent(QMouseEvent* event) {
  if (!(event->buttons().testFlag(Qt::LeftButton)))
    return;
  if ((event->pos() - mDragStartPosition).manhattanLength() < View::START_DRAG_DIST_MIN) {
    return;
  }
  setEnabled(false);
  QMimeData* pMimeData = new (std::nothrow) QMimeData;
  CHECK_NULLPTR_RETURN_VOID(pMimeData)
  pMimeData->setText(text());
  QDrag* pDrag = new (std::nothrow) QDrag{this};
  CHECK_NULLPTR_RETURN_VOID(pDrag)
  pDrag->setMimeData(pMimeData);

  QPixmap pixmap = grab();
  QPixmap transparent_pixmap(pixmap.size());
  transparent_pixmap.fill(Qt::transparent);
  QPainter painter(&transparent_pixmap);
  painter.setOpacity(0.75);
  painter.drawPixmap(0, 0, pixmap);
  painter.end();

  pDrag->setPixmap(transparent_pixmap);
  pDrag->setHotSpot(event->pos());
  if (pDrag->exec(Qt::MoveAction) == Qt::IgnoreAction) {
    setEnabled(true);
  }
}

T_IntoNewPath NavigationExToolBar::m_IntoNewPath;

// toolbar
NavigationExToolBar::NavigationExToolBar(const QString& title, QWidget* parent)  //
    : QToolBar{title, parent} {
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

void NavigationExToolBar::addDraggableAction(QAction* act) {
  CHECK_NULLPTR_RETURN_VOID(act);
  auto* btn = new (std::nothrow) DraggableButton(act, this);
  CHECK_NULLPTR_RETURN_VOID(btn);
  addWidget(btn);
}

bool NavigationExToolBar::MoveToolbuttonInToolBar(int fromIndex, int destIndex) {
  if (fromIndex == destIndex || fromIndex + 1 == destIndex) {
    qDebug("no need move widget at index[%d] to destination in front of index[%d]", fromIndex, destIndex);
    return false;
  }
  const int N = actions().size();
  if (fromIndex < 0 || fromIndex >= N) {
    qWarning("fromIndex[%d] out of bound[0, %d)", fromIndex, N);
    return false;
  }
  if (destIndex < 0 || destIndex > N) {
    qWarning("destIndex[%d] out of bound[0, %d]", destIndex, N);
    return false;
  }
  const QList<QAction*>& oldActs = actions();
  QWidget* widget = widgetForAction(oldActs[fromIndex]);
  if (widget == nullptr) {
    qWarning("fromIndex[%d] widget is None", fromIndex);
    return false;
  }
  qDebug("move widget(index at %d) in front of %d", fromIndex, destIndex);
  if (destIndex >= N) {
    addWidget(widget);
    const QList<QAction*>& newActs = actions();
    removeAction(newActs[fromIndex]);
  } else {
    insertWidget(oldActs[destIndex], widget);
    const QList<QAction*>& newActs = actions();
    if (fromIndex > destIndex) {
      removeAction(newActs[fromIndex + 1]);
    } else {
      removeAction(newActs[fromIndex]);
    }
  }
  return true;
}

void NavigationExToolBar::dragEnterEvent(QDragEnterEvent* event) {
  const QMimeData* pMimeData = event->mimeData();
  CHECK_NULLPTR_RETURN_VOID(pMimeData);
  if (pMimeData->hasUrls()) {
    qDebug("mimeData urls cnt[%d]", pMimeData->urls().size());
    event->acceptProposedAction();
  } else if (pMimeData->hasText()) {
    qDebug("mimeData texts[%s]", qPrintable(pMimeData->text()));
    event->acceptProposedAction();
  }
  event->accept();
}

int GetStartPos(Qt::Orientation orientation, const QPoint& pos) {
  if (orientation == Qt::Orientation::Vertical) {
    return pos.y();
  }
  return pos.x();
}
bool IsCursorPosLessThenWidgetCenter(Qt::Orientation orientation, const int cursorPos, const QWidget& widget) {
  if (orientation == Qt::Orientation::Vertical) {
    return cursorPos <= widget.y() + widget.height() / 2;
  } else {
    return cursorPos <= widget.x() + widget.width() / 2;
  }
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
    QObject* source = event->source();
    if (source == nullptr) {
      qWarning("source is None, skip");
      return;
    }
    if (source->parent() != this) {
      qWarning("parent is not self, skip");
      return;
    }
    if (!source->isWidgetType()) {
      qWarning("source is not widget, skip");
      return;
    }
    QLayout* layout = this->layout();
    if (layout == nullptr) {
      qWarning("layout is None, skip");
      return;
    }
    //  qDebug("type(source): %s", source->metaObject()->className());
    auto* pSrcWidget = qobject_cast<QWidget*>(source);
    if (pSrcWidget == nullptr) {
      qWarning("pSrcWidget is nullptr, skip");
      return;
    }
    pSrcWidget->setEnabled(true);
    const int fromIndex{layout->indexOf(pSrcWidget)};
    if (fromIndex == -1) {
      qDebug("from widget not find at all");
      return;
    }
    const QPoint& pos{event->pos()};
    static const auto orien = orientation();
    const int fromPos{GetStartPos(orien, pos)};
    int destIndex = 0;
    while (destIndex < layout->count()) {
      QWidget* widget = layout->itemAt(destIndex)->widget();
      if (!widget) {
        ++destIndex;
        continue;
      }
      if (IsCursorPosLessThenWidgetCenter(orien, fromPos, *widget)) {
        break;
      }
      ++destIndex;
    }
    event->acceptProposedAction();
    if (!MoveToolbuttonInToolBar(fromIndex, destIndex)) {
      return;
    }
    emit widgetMoved(fromIndex, destIndex);
  }

  return QToolBar::dropEvent(event);
}

// accept drag movements only if the target supports drops
void NavigationExToolBar::dragMoveEvent(QDragMoveEvent* event) {
  if (!event->mimeData()->hasUrls()) {
    return;
  }
  if (!event->mimeData()->hasText()) {
    return;
  }
  const QPoint& pos{event->pos()};
  static const auto orien{orientation()};
  const int fromPos{GetStartPos(orien, pos)};
  const QPoint glbPos{mapToGlobal(pos)};
  const QString fromBtnText = event->mimeData()->text();
  bool isAppend = true;
  for (int i = 0; i < layout()->count(); ++i) {
    QWidget* widget = layout()->itemAt(i)->widget();
    if (widget == nullptr) {
      continue;
    }
    auto* tb = dynamic_cast<QToolButton*>(widget);
    if (tb == nullptr) {
      continue;
    }
    if (IsCursorPosLessThenWidgetCenter(orien, fromPos, *widget)) {
      isAppend = false;
      QToolTip::showText(glbPos, QString("Move btn[%1] in front of[%2]?").arg(fromBtnText, tb->text()));
      break;
    }
  }
  if (isAppend) {
    QToolTip::showText(glbPos, QString("Append btn[%1] to last one?").arg(fromBtnText));
  }
  event->acceptProposedAction();
  return QToolBar::dragMoveEvent(event);
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
    addDraggableAction(pCollectionAct);
    connect(pCollectionAct, &QAction::triggered, [pCollectionAct]() {  //
      NavigationExToolBar::onPathActionTriggered(pCollectionAct);      //
    });
  }
  AlighLeft();
}

bool NavigationExToolBar::onPathActionTriggered(const QAction* pAct) {
  CHECK_NULLPTR_RETURN_FALSE(m_IntoNewPath);
  CHECK_NULLPTR_RETURN_FALSE(pAct);
  return m_IntoNewPath(pAct->toolTip(), true, true);
}

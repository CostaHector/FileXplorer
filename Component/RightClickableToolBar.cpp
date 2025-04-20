#include "RightClickableToolBar.h"
#include "Actions/ActionWithPath.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include <QHash>

const QHash<QString, Qt::ToolButtonStyle> TOOL_BTN_STYLE_MAP = {{"ToolButtonTextOnly", Qt::ToolButtonStyle::ToolButtonTextOnly},
                                                                {"ToolButtonIconOnly", Qt::ToolButtonStyle::ToolButtonIconOnly},
                                                                {"ToolButtonTextBesideIcon", Qt::ToolButtonStyle::ToolButtonTextBesideIcon}};
const QHash<Qt::ToolButtonStyle, QString> TOOL_BTN_STYLE_REV_MAP = {{Qt::ToolButtonStyle::ToolButtonTextOnly, "ToolButtonTextOnly"},
                                                                    {Qt::ToolButtonStyle::ToolButtonIconOnly, "ToolButtonIconOnly"},
                                                                    {Qt::ToolButtonStyle::ToolButtonTextBesideIcon, "ToolButtonTextBesideIcon"}};

RightClickableToolBar::RightClickableToolBar(const QString& title)
    : QToolBar(title),
      extraAG(new (std::nothrow) QActionGroup(this)),
      rightClickedPos(-1, -1),
      UNPIN{new (std::nothrow) QAction(QIcon(":img/UNPIN"), tr("Unpin"), this)},
      UNPIN_ALL{new (std::nothrow) QAction(tr("Unpin All"), this)},
      SHOW_TOOL_BUTTON_TEXT(new (std::nothrow) QAction(TOOL_BTN_STYLE_REV_MAP[Qt::ToolButtonStyle::ToolButtonTextOnly], this)),
      SHOW_TOOL_BUTTON_ICON(new (std::nothrow) QAction(TOOL_BTN_STYLE_REV_MAP[Qt::ToolButtonStyle::ToolButtonIconOnly], this)),
      SHOW_TOOL_BUTTON_TEXT_BESIDE_ICON(new (std::nothrow) QAction(TOOL_BTN_STYLE_REV_MAP[Qt::ToolButtonStyle::ToolButtonTextBesideIcon], this)),
      textIconActionGroup(new (std::nothrow) QActionGroup(this)),
      menuQWidget(new QMenu(this))

{
  setObjectName(title);

  textIconActionGroup->addAction(SHOW_TOOL_BUTTON_TEXT);
  textIconActionGroup->addAction(SHOW_TOOL_BUTTON_ICON);
  textIconActionGroup->addAction(SHOW_TOOL_BUTTON_TEXT_BESIDE_ICON);
  textIconActionGroup->setExclusive(true);

  const int _style = PreferenceSettings().value(MemoryKey::RIGHT_CLICK_TOOLBUTTON_STYLE.name, MemoryKey::RIGHT_CLICK_TOOLBUTTON_STYLE.v).toInt();
  foreach(QAction* act, textIconActionGroup->actions()) {
    act->setCheckable(true);
    if (int(TOOL_BTN_STYLE_MAP[act->text()]) == _style) {
      act->setChecked(true);
      _switchTextBesideIcon(act);
    }
  }

  menuQWidget->addAction(UNPIN);
  menuQWidget->addSeparator();
  menuQWidget->addActions(textIconActionGroup->actions());
  menuQWidget->addSeparator();
  menuQWidget->addAction(UNPIN_ALL);

  setContextMenuPolicy(Qt::CustomContextMenu);

  setOrientation(Qt::Vertical);
  setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
  setMaximumWidth(100);
  setAcceptDrops(true);

  readSettings();

  subscribe();
}

void RightClickableToolBar::dragEnterEvent(QDragEnterEvent* event) {
  const QMimeData* md = event->mimeData();
  qDebug("mimeData cnt=[%d]", md->urls().size());
  event->accept();
}

void RightClickableToolBar::dropEvent(QDropEvent* event) {
  if (not event->mimeData()->hasUrls()) {
    return;
  }
  auto urlsLst = event->mimeData()->urls();
  QMap<QString, QString> folderName2AbsPath;
  for (const QUrl& url : urlsLst) {
    QString pth(url.toLocalFile());
    QFileInfo fi(pth);
    if (not fi.isDir()) {
      continue;
    }
    QString nameShown;
    if (fi.isRoot()) {
      nameShown = fi.absoluteFilePath();
    } else {
      nameShown = fi.completeBaseName();
    }
    folderName2AbsPath[nameShown] = fi.absoluteFilePath();
  }
  qDebug("drop cnt[%d]", folderName2AbsPath.size());
  AppendExtraActions(folderName2AbsPath);
  _save();
  return QToolBar::dropEvent(event);
}

void RightClickableToolBar::dragMoveEvent(QDragMoveEvent* event) {
  // accept drag movements only if the target supports drops
  if (not event->mimeData()->hasUrls()) {
    return;
  }
  return QToolBar::dragMoveEvent(event);
}

void RightClickableToolBar::_save() {
  const QList<QAction*>& actsList = this->actions();
  PreferenceSettings().beginWriteArray("ExtraNavigationDict", actsList.size());
  int extraIndex = 0;
  for (const QAction* act : actsList) {
    PreferenceSettings().setArrayIndex(extraIndex);
    PreferenceSettings().setValue("folderName", act->text());
    PreferenceSettings().setValue("absPath", act->toolTip());
    ++extraIndex;
  }
  PreferenceSettings().endArray();
  PreferenceSettings().setValue(SHOW_TOOL_BUTTON_TEXT->text(), SHOW_TOOL_BUTTON_TEXT->isChecked());
}

void RightClickableToolBar::readSettings() {
  int size = PreferenceSettings().beginReadArray("ExtraNavigationDict");
  QMap<QString, QString> folderName2AbsPath;
  for (int extraIndex = 0; extraIndex < size; ++extraIndex) {
    PreferenceSettings().setArrayIndex(extraIndex);
    folderName2AbsPath[PreferenceSettings().value("folderName").toString()] = PreferenceSettings().value("absPath").toString();
  }
  PreferenceSettings().endArray();
  AppendExtraActions(folderName2AbsPath);
}

void RightClickableToolBar::_unpin() {
  QAction* act = actionAt(rightClickedPos);
  if (actions().contains(act)) {
    removeAction(act);
  }
  _save();
}

void RightClickableToolBar::_unpinAll() {
  foreach(QAction* act, actions()) {
    removeAction(act);
  }
  _save();
}

void RightClickableToolBar::_switchTextBesideIcon(const QAction* act) {
  const Qt::ToolButtonStyle styleEnum = TOOL_BTN_STYLE_MAP[act->text()];
  setToolButtonStyle(styleEnum);
  PreferenceSettings().setValue(MemoryKey::RIGHT_CLICK_TOOLBUTTON_STYLE.name, styleEnum);
}

void RightClickableToolBar::CustomContextMenuEvent(const QPoint& pnt) {
  menuQWidget->popup(mapToGlobal(pnt));
  rightClickedPos = pnt;
}

void RightClickableToolBar::alighLeft() {
  for (int i = 0; i < layout()->count(); ++i) {
    layout()->itemAt(i)->setAlignment(Qt::AlignmentFlag::AlignLeft);
  }
}

void RightClickableToolBar::AppendExtraActions(const QMap<QString, QString>& folderName2AbsPath) {
  if (folderName2AbsPath.isEmpty()) {
    return;
  }
  for (auto it = folderName2AbsPath.cbegin(); it != folderName2AbsPath.cend(); ++it) {
    const QString& folderName = it.key();
    const QString& absPath = it.value();
    QAction* TEMP_ACTIONS = new ActionWithPath(absPath, QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirIcon), folderName, this);
    addAction(TEMP_ACTIONS);
  }
  alighLeft();
}

bool RightClickableToolBar::subscribe() {
  connect(textIconActionGroup, &QActionGroup::triggered, this, &RightClickableToolBar::_switchTextBesideIcon);
  connect(this, &QToolBar::customContextMenuRequested, this, &RightClickableToolBar::CustomContextMenuEvent);

  connect(UNPIN, &QAction::triggered, this, &RightClickableToolBar::_unpin);
  connect(UNPIN_ALL, &QAction::triggered, this, &RightClickableToolBar::_unpinAll);
  return true;
}

#include "RightClickableToolBar.h"
#include <QHash>

const QHash<QString, Qt::ToolButtonStyle> TOOL_BTN_STYLE_MAP = {{"ToolButtonTextOnly", Qt::ToolButtonStyle::ToolButtonTextOnly},
                                                                {"ToolButtonIconOnly", Qt::ToolButtonStyle::ToolButtonIconOnly},
                                                                {"ToolButtonTextBesideIcon", Qt::ToolButtonStyle::ToolButtonTextBesideIcon}};
const QHash<Qt::ToolButtonStyle, QString> TOOL_BTN_STYLE_REV_MAP = {{Qt::ToolButtonStyle::ToolButtonTextOnly, "ToolButtonTextOnly"},
                                                                    {Qt::ToolButtonStyle::ToolButtonIconOnly, "ToolButtonIconOnly"},
                                                                    {Qt::ToolButtonStyle::ToolButtonTextBesideIcon, "ToolButtonTextBesideIcon"}};

RightClickableToolBar::RightClickableToolBar(const QString& title)
    : QToolBar(title),
      extraAG(new QActionGroup(this)),
      rightClickedPos(-1, -1),
      SHOW_TOOL_BUTTON_TEXT(new QAction(TOOL_BTN_STYLE_REV_MAP[Qt::ToolButtonStyle::ToolButtonTextOnly], this)),
      SHOW_TOOL_BUTTON_ICON(new QAction(TOOL_BTN_STYLE_REV_MAP[Qt::ToolButtonStyle::ToolButtonIconOnly], this)),
      SHOW_TOOL_BUTTON_TEXT_BESIDE_ICON(new QAction(TOOL_BTN_STYLE_REV_MAP[Qt::ToolButtonStyle::ToolButtonTextBesideIcon], this)),
      textIconActionGroup(new QActionGroup(this)),
      menuQWidget(new QMenu(this))

{
  //    qDebug("%s", QString("Show tool button only text [%1]").arg(int(Qt::ToolButtonStyle::ToolButtonTextOnly)).toStdString().c_str());

  setObjectName(title);

  QAction* UNPIN = new QAction("Unpin", this);
  connect(UNPIN, &QAction::triggered, this, &RightClickableToolBar::_unpin);

  QAction* UNPIN_ALL = new QAction("Unpin All", this);
  connect(UNPIN_ALL, &QAction::triggered, this, &RightClickableToolBar::_unpinAll);

  textIconActionGroup->addAction(SHOW_TOOL_BUTTON_TEXT);
  textIconActionGroup->addAction(SHOW_TOOL_BUTTON_ICON);
  textIconActionGroup->addAction(SHOW_TOOL_BUTTON_TEXT_BESIDE_ICON);
  textIconActionGroup->setExclusive(true);

  const int _style = PreferenceSettings().value(MemoryKey::RIGHT_CLICK_TOOLBUTTON_STYLE.name, MemoryKey::RIGHT_CLICK_TOOLBUTTON_STYLE.v).toInt();
  for (QAction* act : textIconActionGroup->actions()) {
    act->setCheckable(true);
    if (int(TOOL_BTN_STYLE_MAP[act->text()]) == _style) {
      act->setChecked(true);
    }
  }

  menuQWidget->addAction(UNPIN);
  menuQWidget->addActions(textIconActionGroup->actions());
  menuQWidget->addAction(UNPIN_ALL);

  setContextMenuPolicy(Qt::CustomContextMenu);

  setOrientation(Qt::Vertical);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
  setMaximumWidth(100);
  setAcceptDrops(true);

  readSettings();
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
  PreferenceSettings().beginWriteArray("ExtraNavigationDict", extraShownText2Path.size());
  int extraIndex = 0;
  for (auto it = extraShownText2Path.begin(); it != extraShownText2Path.cend(); ++it) {
    const QString& folderName = it.key();
    const QString& absPath = it.value();
    PreferenceSettings().setArrayIndex(extraIndex);
    PreferenceSettings().setValue("folderName", folderName);
    PreferenceSettings().setValue("absPath", absPath);
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
    extraShownText2Path.remove(act->text());
  }
  _save();
}

void RightClickableToolBar::_unpinAll() {
  for (QAction* act : actions()) {
    removeAction(act);
  }
  extraShownText2Path.clear();
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
    const QString shownText = it.key();
    const QString path = it.value();
    QAction* TEMP_ACTIONS = new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirIcon), shownText, this);
    extraShownText2Path[shownText] = path;
    addAction(TEMP_ACTIONS);
  }
  alighLeft();
  _save();
}

bool RightClickableToolBar::subscribe(T_IntoNewPath IntoNewPath) {
  connect(this, &QToolBar::actionTriggered, this, [this, IntoNewPath](const QAction* act) {
    if (!IntoNewPath) {
      qDebug("RightClickableToolBar, IntoNewPath is nullptr");
      return;
    }
    IntoNewPath(extraShownText2Path[act->text()], true, true);
  });
  connect(textIconActionGroup, &QActionGroup::triggered, this, &RightClickableToolBar::_switchTextBesideIcon);
  connect(this, &QToolBar::customContextMenuRequested, this, &RightClickableToolBar::CustomContextMenuEvent);
  return true;
}

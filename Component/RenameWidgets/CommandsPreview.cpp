#include "CommandsPreview.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "PublicMacro.h"
#include "FileTool.h"

#include <QResizeEvent>

CommandsPreview::CommandsPreview(const QString& name, QWidget* parent) //
  : QTextBrowser{parent}
  , mName{name} //
{
  COPY_TEXT = new (std::nothrow) QAction{QIcon(":img/COPY_TEXT"), "Copy", this};
  CHECK_NULLPTR_RETURN_VOID(COPY_TEXT);
  COPY_TEXT->setToolTip("Copy all text in this window to system clipboard");

  STAY_ON_TOP = new (std::nothrow) QAction{QIcon(":img/PIN"), "Stay on Top", this};
  CHECK_NULLPTR_RETURN_VOID(STAY_ON_TOP);
  STAY_ON_TOP->setCheckable(true);
  STAY_ON_TOP->setToolTip("Set this window always stay on top");

  mToolBar = new (std::nothrow) QToolBar{"Floating toolbar", this};
  CHECK_NULLPTR_RETURN_VOID(mToolBar);
  mToolBar->addAction(COPY_TEXT);
  mToolBar->addAction(STAY_ON_TOP);
  mToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  mToolBar->setCursor(Qt::PointingHandCursor);

  adjustButtonPosition();
  subscribe();
  ReadSettings();
}

void CommandsPreview::showEvent(QShowEvent* event) {
  QTextBrowser::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void CommandsPreview::closeEvent(QCloseEvent* event) {
  Configuration().setValue(mName + "_GEOMETRY", saveGeometry());
  QTextBrowser::closeEvent(event);
}

void CommandsPreview::ReadSettings() {
  if (Configuration().contains(mName + "_GEOMETRY")) {
    restoreGeometry(Configuration().value(mName + "_GEOMETRY").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  setFont(StyleSheet::TEXT_EDIT_FONT);
  setWindowIcon(QIcon(":img/COMMAND_PREVIEW"));
}

void CommandsPreview::resizeEvent(QResizeEvent* event) {
  QTextBrowser::resizeEvent(event);
  adjustButtonPosition();
}

void CommandsPreview::adjustButtonPosition() {
  static constexpr int margin = 5;
  mToolBar->move(width() - mToolBar->width() - margin, margin);
}

void CommandsPreview::subscribe() {
  connect(COPY_TEXT, &QAction::triggered, this, [this]() { FileTool::CopyTextToSystemClipboard(toPlainText()); });
  connect(STAY_ON_TOP, &QAction::toggled, this, [this](const bool checked) {
    setWindowFlag(Qt::WindowStaysOnTopHint, checked);
    show();
  });
}

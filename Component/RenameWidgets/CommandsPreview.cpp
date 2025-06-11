#include "CommandsPreview.h"
#include "public/PublicVariable.h"
#include "public/StyleSheet.h"

#include <QClipboard>
#include <QApplication>
#include <QResizeEvent>

CommandsPreview::CommandsPreview(QWidget* parent)  //
    : QPlainTextEdit{parent}                       //
{
  mToolButton = new (std::nothrow) QToolButton{this};
  CHECK_NULLPTR_RETURN_VOID(mToolButton);
  COPY_TEXT = new (std::nothrow) QAction{QIcon(":img/COPY_TEXT"), "Copy", mToolButton};
  CHECK_NULLPTR_RETURN_VOID(COPY_TEXT);

  mToolButton->setDefaultAction(COPY_TEXT);
  mToolButton->setCursor(Qt::PointingHandCursor);
  mToolButton->setFixedSize(QSize{48, 48});

  adjustButtonPosition();
  subscribe();
  ReadSettings();
}

void CommandsPreview::showEvent(QShowEvent* event) {
  QPlainTextEdit::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void CommandsPreview::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("COMMANDS_PREVIEW_GEOMETRY", saveGeometry());
  QPlainTextEdit::closeEvent(event);
}

void CommandsPreview::ReadSettings() {
  if (PreferenceSettings().contains("COMMANDS_PREVIEW_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("COMMANDS_PREVIEW_GEOMETRY").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  static const QFont font{"Consolas", 15};
  setFont(font);
  setWindowIcon(QIcon(":img/COMMAND_PREVIEW"));
}

void CommandsPreview::resizeEvent(QResizeEvent* event) {
  QPlainTextEdit::resizeEvent(event);
  adjustButtonPosition();
}

void CommandsPreview::adjustButtonPosition() {
  static constexpr int margin = 5;
  mToolButton->move(width() - mToolButton->width() - margin, margin);
}

void CommandsPreview::subscribe() {
  connect(COPY_TEXT, &QAction::triggered, [this]() {
    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard == nullptr) {
      qWarning("clipboard is nullptr. cannot copy");
      return;
    }
    clipboard->setText(toPlainText());
  });
}

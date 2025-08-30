#include "LogFloatingPreviewer.h"
#include "PublicMacro.h"
#include "LogHandler.h"
#include <QRect>
#include <QKeyEvent>

constexpr int LogFloatingPreviewer::SIZE_WIDTH;
constexpr int LogFloatingPreviewer::SIZE_HEIGHT;

LogFloatingPreviewer::LogFloatingPreviewer(QWidget* parent) : QTextBrowser{parent} {
  hide();
  setReadOnly(false);
  setWindowFlags(Qt::SubWindow | Qt:: WindowStaysOnTopHint | Qt::FramelessWindowHint);

  static const QFont LOGS_FONT{"Consolas", 13};
  setFont(LOGS_FONT);
  setWindowIcon(QIcon(":img/LOG_FILES_PREVIEW"));
  setWindowTitle("Logs");
}

void LogFloatingPreviewer::BindToolButton(QToolButton* tb) {
  CHECK_NULLPTR_RETURN_VOID(tb)
  _logPreviewTb = tb;
  connect(_logPreviewTb, &QToolButton::clicked, this, &LogFloatingPreviewer::onHideShow);
}

void LogFloatingPreviewer::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_F12) {
    event->accept();
    if (_logPreviewTb != nullptr) {
      _logPreviewTb->setChecked(false);
      onHideShow(false);
    }
    return;
  } else if (event->key() == Qt::Key_F5) {
    event->accept();
    UpdateLogsContents(150, false);
    return;
  }
  QTextBrowser::keyPressEvent(event);
}

void LogFloatingPreviewer::resizeEvent(QResizeEvent *event) {
  QTextBrowser::resizeEvent(event);
  MovePosition();
}

void LogFloatingPreviewer::UpdateLogsContents(const int maxLines, bool bMoveToEnd) {
  const QByteArray buffer = LogHandler::GetLastNLinesOfLogs(maxLines);
  setPlainText(QString::fromUtf8(buffer));
  if (bMoveToEnd) {
    moveCursor(QTextCursor::End);
  }
}

void LogFloatingPreviewer::onHideShow(bool checked) {
  if (!checked) {
    hide();
    return;
  }
  MovePosition();
  UpdateLogsContents();
  show();
}

void LogFloatingPreviewer::MovePosition() {
  if (_logPreviewTb == nullptr) {
    return;
  }
  QRect buttonRect = _logPreviewTb->geometry();
  QPoint pointD = _logPreviewTb->mapToGlobal(QPoint(buttonRect.width(), buttonRect.height()));
  QPoint previewTopLeft(pointD.x() - width(), pointD.y());
  move(previewTopLeft);
}

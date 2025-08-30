#include "LogFloatingPreviewer.h"
#include "PublicMacro.h"
#include "LogHandler.h"
#include <QRect>

constexpr int LogFloatingPreviewer::SIZE_WIDTH;
constexpr int LogFloatingPreviewer::SIZE_HEIGHT;

LogFloatingPreviewer::LogFloatingPreviewer(QWidget* parent) : QTextBrowser{parent} {
  setWindowFlags(Qt::SubWindow | Qt:: WindowStaysOnTopHint |Qt::FramelessWindowHint);
  hide();

  setWindowIcon(QIcon(":img/LOG_FILES_PREVIEW"));
  setWindowTitle("Logs");
}

void LogFloatingPreviewer::BindToolButton(QToolButton* tb) {
  CHECK_NULLPTR_RETURN_VOID(tb)
  _logPreviewTb = tb;
  connect(_logPreviewTb, &QToolButton::toggled, this, &LogFloatingPreviewer::onHideShow);
}

void LogFloatingPreviewer::resizeEvent(QResizeEvent *event) {
  QTextBrowser::resizeEvent(event);
  MovePosition();
}

void LogFloatingPreviewer::UpdateLogsContents(const int maxLines) {
  const QByteArray buffer = LogHandler::GetLastNLinesOfLogs(maxLines);
  setPlainText(QString::fromUtf8(buffer));
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

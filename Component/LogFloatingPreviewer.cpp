#include "LogFloatingPreviewer.h"
#include "PublicMacro.h"
#include "Logger.h"
#include <QRect>
#include <QKeyEvent>
#include <QAction>

constexpr int LogFloatingPreviewer::SIZE_WIDTH;
constexpr int LogFloatingPreviewer::SIZE_HEIGHT;

LogFloatingPreviewer::LogFloatingPreviewer(const QString& name, QWidget* parent)//
  : CommandsPreview{name, parent} {
  hide();
  setReadOnly(false);
  setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);

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
  switch (event->key()) {
    case Qt::Key_Escape: {
      if (_logPreviewTb != nullptr) {
        _logPreviewTb->setChecked(false);
        onHideShow(false);
      }
      event->accept();
      return;
    }
    case Qt::Key_F5:{
      UpdateLogsContents(150, false);
      event->accept();
      return;
    }
    default: break;
  }
  CommandsPreview::keyPressEvent(event);
}

void LogFloatingPreviewer::resizeEvent(QResizeEvent *event) {
  CommandsPreview::resizeEvent(event);
  MovePosition();
}

void LogFloatingPreviewer::UpdateLogsContents(const int maxLines, bool bMoveToEnd) {
  const QByteArray buffer = Logger::GetLastNLinesOfLogs(maxLines);
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

#include "MultiCursorEditor.h"
#include "Logger.h"
#include <QPainter>
#include <QTextBlock>

void MultiCursorEditor::mousePressEvent(QMouseEvent *e) {
  if (e == nullptr) {
    return;
  }
  IntoInitStatus(e->pos());
  QPlainTextEdit::mousePressEvent(e);
}

void MultiCursorEditor::mouseMoveEvent(QMouseEvent *e) {
  if (e == nullptr) {
    return;
  }

  if (mStatus == PREPARING) {
    PreparingStatusUpdateEndPostion(e->pos());
    e->accept();
    return;
  }
  QPlainTextEdit::mouseMoveEvent(e);
}

void MultiCursorEditor::mouseReleaseEvent(QMouseEvent *e) {
  if (e == nullptr) {
    return;
  }
  if (mStatus == PREPARING) {
    IntoMultiLineSelecting(e->pos());
    e->accept();
    return;
  }
  QPlainTextEdit::mouseReleaseEvent(e);
}

void MultiCursorEditor::paintEvent(QPaintEvent *event) {
  QPlainTextEdit::paintEvent(event);

  if (cursorVisible) {
    QPainter painter(viewport());
    painter.setPen(QPen(Qt::black, 2));
    for (const QTextCursor &cur : mCursors) {
      QRect rect1 = cursorRect(cur);
      painter.drawLine(rect1.topLeft(), rect1.bottomLeft());
    }
  }
}

void MultiCursorEditor::keyPressEvent(QKeyEvent *event) {
  if (event->modifiers() == Qt::AltModifier) {
    if (mStatus == INIT) {
      IntoMultiSelectStatus();
      event->accept();
      return;
    }
  }

  if (mCursors.isEmpty()) {
    QPlainTextEdit::keyPressEvent(event);
    return;
  }
  switch (event->key()) {
    case Qt::Key_Escape: {
      disableMultiline();
      break;
    }
    case Qt::Key_Backspace:
      // 在两个光标位置同时删除前一个字符
      deleteAtBothCursors();
      break;

    case Qt::Key_Delete:
      // 在两个光标位置同时删除后一个字符
      deleteAtBothCursors(false);
      break;

    default:
      if (!event->text().isEmpty()) {
        insertAtBothCursors(event->text());
        break;
      }
      // 如果是普通字符，在两个位置同时插入
      QPlainTextEdit::keyPressEvent(event);
      break;
  }
}

void MultiCursorEditor::deleteAtBothCursors(bool backward) {
  // 按行号排序，从下往上处理，避免位置变化
  // 保存当前位置
  std::sort(mCursors.begin(), mCursors.end(), [](const QTextCursor &a, const QTextCursor &b) { return a.blockNumber() > b.blockNumber(); });
  for (QTextCursor &cur : mCursors) {
    int pos = cur.position();
    if (backward) { // Backspace
      if (cur.position() > 0) {
        cur.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
        cur.removeSelectedText();
        cur.setPosition(pos - 1);
      }
    } else { // Delete
      if (cur.position() < toPlainText().length()) {
        cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
        cur.removeSelectedText();
        cur.setPosition(pos);
      }
    }
  }

  viewport()->update();
}

void MultiCursorEditor::insertAtBothCursors(const QString &text) {
  // 保存当前位置
  std::sort(mCursors.begin(), mCursors.end(), [](const QTextCursor &a, const QTextCursor &b) { return a.blockNumber() > b.blockNumber(); });
  for (QTextCursor &cur : mCursors) {
    cur.insertText(text);
  }
  viewport()->update();
}

void MultiCursorEditor::onSelectMultiLine() {
  QList<QTextCursor> newCursors;
  QTextDocument *doc = document();
  if (doc == nullptr) {
    return;
  }

  // 计算选择的列范围（以字符为单位）
  QRect selectionRect(GetRect());
  QTextCursor startCursor = cursorForPosition(selectionRect.topLeft());
  QTextCursor endCursor = cursorForPosition(selectionRect.bottomRight());

  int startLine = startCursor.blockNumber();
  int endLine = endCursor.blockNumber();

  // 确保startLine <= endLine
  if (startLine > endLine) {
    qSwap(startLine, endLine);
    qSwap(startCursor, endCursor);
  }

  // 对于每一行，计算列选择
  for (int line = startLine; line <= endLine; line++) {
    QTextBlock block = doc->findBlockByNumber(line);
    if (!block.isValid()) {
      continue;
    }
    QTextCursor cursor(block);
    // 计算该行的列位置
    int startCol = startCursor.positionInBlock();
    QTextCursor selectionCursor = cursor;
    selectionCursor.setPosition(block.position() + startCol);
    newCursors.push_back(selectionCursor);
  }
  mCursors.swap(newCursors);
  if (!mCursors.isEmpty()) {
    enableMultiline();
  }
}

QRect MultiCursorEditor::GetRect() const {
  if (mStatus == INIT) {
    // LOG_D("mStatus: %d not selecting", mStatus);
    return {};
  }
  int x1 = std::min(m_startPos.x(), m_endPos.x());
  int x2 = std::max(m_startPos.x(), m_endPos.x());
  int y1 = std::min(m_startPos.y(), m_endPos.y());
  int y2 = std::max(m_startPos.y(), m_endPos.y());
  return {QPoint(x1, y1), QPoint(x2, y2)};
}

void MultiCursorEditor::IntoMultiSelectStatus() {
  if (m_startPos.isNull()) {
    LOG_D("[invalid] m_startPos is null");
    return;
  }
  mStatus = PREPARING;
  LOG_D("[preparing] now, alt pressed");
}

void MultiCursorEditor::IntoMultiLineSelecting(QPoint endPos) {
  if (m_startPos.isNull()) {
    LOG_D("[invalid] m_startPos is null");
    return;
  }
  if (endPos.isNull()) {
    LOG_D("[invalid] endPos is null");
    return;
  }
  m_endPos = endPos;
  mStatus = MULTI_LINE_SELECTING;
  onSelectMultiLine();
}

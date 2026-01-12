#ifndef MULTICURSOREDITOR_H
#define MULTICURSOREDITOR_H
#include <QPlainTextEdit>
#include <QTimer>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QTextCursor>

class MultiCursorEditor : public QPlainTextEdit {
public:
  enum SelectStatus {
    INIT,
    PREPARING,
    MULTI_LINE_SELECTING,
  };

  MultiCursorEditor(QWidget *parent = nullptr)
    : QPlainTextEdit(parent) {
    connect(&blinkTimer, &QTimer::timeout, this, &MultiCursorEditor::toggleCursors);
  }

protected:
  void mousePressEvent(QMouseEvent *e) override;

  void mouseMoveEvent(QMouseEvent *e) override;

  void mouseReleaseEvent(QMouseEvent *e) override;

  void paintEvent(QPaintEvent *event) override;

  void keyPressEvent(QKeyEvent *event) override;

private slots:
  void toggleCursors() {
    cursorVisible = !cursorVisible;
    viewport()->update();
  }

private:
  void deleteAtBothCursors(bool backward = true);
  void insertAtBothCursors(const QString &text);

  void onSelectMultiLine();

  QRect GetRect() const;

  void IntoInitStatus(QPoint startPos) {
    disableMultiline();
    mStatus = INIT;
    m_startPos = startPos;
    m_endPos = QPoint(); // invalid
  }

  void IntoMultiSelectStatus();

  void PreparingStatusUpdateEndPostion(QPoint endPos) {
    m_endPos = endPos;
  }

  void IntoMultiLineSelecting(QPoint endPos);

  void enableMultiline() {
    blinkTimer.start(800);
    cursorVisible = true;
    viewport()->update();
  }

  void disableMultiline() {
    mCursors.clear();
    blinkTimer.stop();
    cursorVisible = false;
    viewport()->update();
  }

  bool cursorVisible{false};
  QTimer blinkTimer;
  QList<QTextCursor> mCursors;
  SelectStatus mStatus = INIT;
  QPoint m_startPos, m_endPos;
};

#endif // MULTICURSOREDITOR_H

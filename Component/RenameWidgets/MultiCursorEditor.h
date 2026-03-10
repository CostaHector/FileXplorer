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

  explicit MultiCursorEditor(QWidget *parent = nullptr);

protected:
  void mousePressEvent(QMouseEvent *e) override;

  void mouseMoveEvent(QMouseEvent *e) override;

  void mouseReleaseEvent(QMouseEvent *e) override;

  void paintEvent(QPaintEvent *event) override;

  void keyPressEvent(QKeyEvent *event) override;

private slots:
  void toggleCursors();

private:
  void deleteAtBothCursors(bool backward = true);
  void insertAtBothCursors(const QString &text);

  void onSelectMultiLine();

  QRect GetRect() const;

  void IntoInitStatus(QPoint startPos);

  void IntoMultiSelectStatus();

  void PreparingStatusUpdateEndPostion(QPoint endPos);

  void IntoMultiLineSelecting(QPoint endPos);

  void enableMultiline();

  void disableMultiline();

  bool cursorVisible{false};
  QTimer mBlinkTimer;
  QList<QTextCursor> mCursors;
  SelectStatus mStatus = INIT;
  QPoint m_startPos, m_endPos;
};

#endif // MULTICURSOREDITOR_H

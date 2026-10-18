#ifndef SELECTIONFRAME_H
#define SELECTIONFRAME_H

#include <QFrame>
#include <QToolBar>
class QLabel;

class SelectionFrame : public QFrame {
  Q_OBJECT
public:
  explicit SelectionFrame(QWidget* parent = nullptr);
  QLabel* GetFaceRecognizeResultLabel() const { return m_faceRecognizeResult; }
  bool IsAutoFaceRecognize() const { return bAutoFaceRecognize; }

signals:
  void faceRecognizeRequested();
  void saveRequested();
  void closeRequested();

protected:
  // 吃掉鼠标事件，避免点击框内/框外空白时触发新的框选
  void mousePressEvent(QMouseEvent*)   override {}
  void mouseMoveEvent(QMouseEvent*)    override {}
  void mouseReleaseEvent(QMouseEvent*) override {}

private:
  QLabel* m_faceRecognizeResult{nullptr};
  QToolBar* mControlToolBar{nullptr};

  static void setAutoFaceRecognize(bool bAuto) { bAutoFaceRecognize = bAuto; }
  static bool bAutoFaceRecognize;
};

#endif // SELECTIONFRAME_H

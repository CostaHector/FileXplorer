#ifndef SELECTABLELABEL_H
#define SELECTABLELABEL_H

#include <QLabel>

class SelectableLabel : public QLabel {
  Q_OBJECT
public:
  explicit SelectableLabel(QWidget* parent = nullptr) : QLabel{parent} {}

  void setSelectionEnabled(bool on);
  bool isSelectionEnabled() const { return mSelectionEnabled; }
  static bool SaveSelectionRectImage(const QPixmap& selected, const QSize& srcSize, const QString& sourceImageFullPath);

signals:
  // selected: 裁剪出来的图片；srcSize: 在原始 pixmap 上的尺寸
  void selectionFinished(const QPixmap& selected, const QSize& srcSize);

protected:
  void mousePressEvent(QMouseEvent* e) override;

  void mouseMoveEvent(QMouseEvent* e) override;

  void mouseReleaseEvent(QMouseEvent* e) override;

  void paintEvent(QPaintEvent* e) override;

private:
  QRect selectionRect() const { return QRect(mOrigin, mCurrent).normalized(); }
  QRect pixmapRectInLabel() const;

  bool mSelectionEnabled = false;
  bool mSelecting = false;
  QPoint mOrigin, mCurrent;
};

#endif // SELECTABLELABEL_H

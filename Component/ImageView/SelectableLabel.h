#ifndef SELECTABLELABEL_H
#define SELECTABLELABEL_H

#include <QLabel>
#include <QPixmap>
#include <QSize>
#include "SelectionFrame.h"

class SelectableLabel : public QLabel {
  Q_OBJECT
public:
  explicit SelectableLabel(QWidget* parent = nullptr);

  void setSelectionEnabled(bool on);
  bool isSelectionEnabled() const { return mSelectionEnabled; }
  void setBaseImageFullPath(const QString& baseImageFullPath) const { mBaseImageFullPath = baseImageFullPath; }
  static bool SaveRectSelectionImage(const QPixmap& rectSelectedImg, const QString& sourceImageFullPath, bool bSilentSave);

signals:
  void rectSelectionFinished(const QPixmap& rectSelectedImg);

public slots:
  void onFaceRecognizeRequested();
  void onSaveRequested();

protected:
  void mousePressEvent(QMouseEvent* e) override;
  void mouseMoveEvent(QMouseEvent* e) override;
  void mouseReleaseEvent(QMouseEvent* e) override;
  void paintEvent(QPaintEvent* e) override;

private:
  QRect visSelectionRect() const { return QRect{mOrigin, mCurrent}.normalized(); }
  QRect pixmapRectInLabel() const;
  void  cancelSelection();

  bool   mSelectionEnabled = false;
  bool   mIsDragging       = false;      // 仅表示"正在拖动"
  QPoint mOrigin, mCurrent;

  mutable QString mBaseImageFullPath;
  SelectionFrame* mFrame = nullptr;

  void LazyUpdateRectSelectionImage();
  bool mIsNeedUpdateRectSelectedPixmap{false};
  QRect mRelativeRect;
  QPixmap mRectSelectedPixmap;
};
#endif // SELECTABLELABEL_H

#ifndef VOLUMEWIDGET_H
#define VOLUMEWIDGET_H

#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QAction>
#include "ClickableSlider.h"

class VolumeWidget : public QWidget {
  Q_OBJECT
public:
  explicit VolumeWidget(QBoxLayout::Direction direction = QBoxLayout::Direction::LeftToRight, QWidget* parent = nullptr);
  ~VolumeWidget();

  bool isMuted() const { return mMuteAct != nullptr && mMuteAct->isChecked(); }
  int volumeVal() const { return mVolumeSlider == nullptr ? 100 : mVolumeSlider->value(); }

signals:
  void onMutedChanged(bool bMute);
  void onVolumeChanged(int volumeVal);

private:
  void onVolumeValueChange(const int logScaleValue);

  QAction* mMuteAct{nullptr};              // 静音按钮
  QToolButton* mMuteBtn{nullptr};          // 静音按钮
  ClickableSlider* mVolumeSlider{nullptr}; // 音量控制条
  QLabel* mVolumeValLabel{nullptr};        // 音量值
  QBoxLayout* mLayout{nullptr};
};

#endif // VOLUMEWIDGET_H

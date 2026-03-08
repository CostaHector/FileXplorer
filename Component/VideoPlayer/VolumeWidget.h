#ifndef VOLUMEWIDGET_H
#define VOLUMEWIDGET_H

#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QAction>
#include "ClickableSlider.h"

namespace VolumeIterate {
short linearValue2LogValue(short linearValue);
short logValue2LinearValue(short logValue100);
short previousLevelInLinearScale(short target);
short nextLevelInLinearScale(short target);
constexpr short ENLARGE_TIMES = 100;
}  // namespace VolumeIterate


class VolumeWidget : public QWidget {
  Q_OBJECT
public:
  explicit VolumeWidget(QBoxLayout::Direction direction = QBoxLayout::Direction::LeftToRight, QWidget* parent = nullptr);
  ~VolumeWidget();

  bool isMuted() const { return mMuteAct->isChecked(); }
  int volumeVal() const { return mVolumeSlider->value(); }
  short volumeValLogx100Int() const { return VolumeIterate::linearValue2LogValue(volumeVal()); }
  qreal volumeValLog() const { return (qreal)volumeValLogx100Int() / VolumeIterate::ENLARGE_TIMES; }
  bool reqLogVolumeIncrease() {return reqLogVolumeChange(true);}
  bool reqLogVolumeDecrease() {return reqLogVolumeChange(false);}

signals:
  void mutedStateToggled(bool bMute);
  void sliderVolumeChanged(int volumeVal); // pass it to device(log scale)

private:
  void onMouseEventProcessor(const int sliderValue);
  bool reqLogVolumeChange(bool bIncrease);
  QString GetLabelText() const;

  QAction* mMuteAct{nullptr};              // 静音按钮
  QToolButton* mMuteBtn{nullptr};          // 静音按钮
  ClickableSlider* mVolumeSlider{nullptr}; // 音量控制条
  QLabel* mVolumeValLabel{nullptr};        // 音量值
  QBoxLayout* mLayout{nullptr};
  static constexpr int MIN_VOLUME{0}, MAX_VOLUME{100};
};

#endif // VOLUMEWIDGET_H

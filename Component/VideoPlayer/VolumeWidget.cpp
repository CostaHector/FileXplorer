#include "VolumeWidget.h"
#include "MemoryKey.h"
#include "DualIconCheckableAction.h"
#include "NotificatorMacro.h"

VolumeWidget::VolumeWidget(QBoxLayout::Direction direction, QWidget* parent)
  : QWidget{parent} {
  const bool bMuteVolume = Configuration().value(MemoryKey::VIDEO_PLAYER_MUTE.name, MemoryKey::VIDEO_PLAYER_MUTE.v).toBool();
  mMuteAct = DualIconCheckableAction::CreateMuteAction(this, bMuteVolume);

  mMuteBtn = new QToolButton{this};
  mMuteBtn->setDefaultAction(mMuteAct);

  mVolumeSlider = new ClickableSlider{Qt::Orientation::Horizontal, this};
  mVolumeSlider->setRange(0, 100);
  const int logScaleVolume = Configuration().value(MemoryKey::VIDEO_PLAYER_VOLUME.name, MemoryKey::VIDEO_PLAYER_VOLUME.v).toInt();
  mVolumeSlider->setValue(logScaleVolume);

  mVolumeValLabel = new QLabel{QString::number(logScaleVolume), this};

  mLayout = new QBoxLayout{direction, this};
  mLayout->setContentsMargins(0, 0, 0, 0);
  mLayout->setSpacing(0);
  mLayout->addWidget(mMuteBtn);
  mLayout->addWidget(mVolumeSlider);
  mLayout->addWidget(mVolumeValLabel);

  connect(mMuteAct, &QAction::toggled, this, &VolumeWidget::onMutedChanged);
  mVolumeSlider->reg(std::bind(&VolumeWidget::onVolumeValueChange, this, std::placeholders::_1));
  connect(mVolumeSlider, &QSlider::sliderMoved, this, &VolumeWidget::onVolumeValueChange);
}

VolumeWidget::~VolumeWidget() {
  Configuration().setValue(MemoryKey::VIDEO_PLAYER_MUTE.name, isMuted());
  Configuration().setValue(MemoryKey::VIDEO_PLAYER_VOLUME.name, volumeVal());
}

void VolumeWidget::onVolumeValueChange(const int logScaleValue) {
  // Set slider axis is logarithmic scale while videoplayer use linear scale
  // qreal linearVolume = 100 * QAudio::convertVolume(logScaleValue / qreal(100.0), QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale);
  if (logScaleValue > 0 && mMuteAct->isChecked()) {
    mMuteAct->setChecked(false);
    emit mMuteAct->toggled(false);
  } else if (logScaleValue == 0 && !mMuteAct->isChecked()) {
    mMuteAct->setChecked(true);
    emit mMuteAct->toggled(true);
  }
  mVolumeValLabel->setText(QString::number(logScaleValue));
  mVolumeSlider->setValue(logScaleValue);
  emit onVolumeChanged(logScaleValue);
}

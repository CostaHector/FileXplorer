#include "VolumeWidget.h"
#include "MemoryKey.h"
#include "DualIconCheckableAction.h"
#include "NotificatorMacro.h"
#include <cmath>

namespace VolumeIterate {
constexpr const short VOLUME_IN_LOG_SCALE_ARR[]  //
    {
        0,    1,    4,    9,    16,   25,   36,   49,   64,   81,   100,  121,  144,  169,  196,  225,  256,  289,  324,  361,  400,
        441,  484,  529,  576,  625,  676,  729,  784,  841,  900,  961,  1024, 1089, 1156, 1225, 1296, 1369, 1444, 1521, 1600, 1681,
        1764, 1849, 1936, 2025, 2116, 2209, 2304, 2401, 2500, 2601, 2704, 2809, 2916, 3025, 3136, 3249, 3364, 3481, 3600, 3721, 3844,
        3969, 4096, 4225, 4356, 4489, 4624, 4761, 4900, 5041, 5184, 5329, 5476, 5625, 5776, 5929, 6084, 6241, 6400, 6561, 6724, 6889,
        7056, 7225, 7396, 7569, 7744, 7921, 8100, 8281, 8464, 8649, 8836, 9025, 9216, 9409, 9604, 9801, 10000  //
    };
constexpr int VOLUME_IN_LOG_SCALE_ARR_N{sizeof(VOLUME_IN_LOG_SCALE_ARR) / sizeof(VOLUME_IN_LOG_SCALE_ARR[0])};

short linearValue2LogValue(short linearValue) {
  // 线性刻度->返回平方刻度, 扩大100倍以此整数化
  if (linearValue < 0)
    return 0;
  if (linearValue > 100)
    return 10000;
  return linearValue * linearValue;
}
short logValue2LinearValue(short logValue100) {
  // 入参=100*平方刻度(完全平方数) -> 返回扩大了对数刻度
  if (logValue100 < 0)
    return 0;
  if (logValue100 > 10000)
    return 100;
  return (short)std::sqrt(logValue100);
}
// 没有上一级时返回-1
short previousLevelInLinearScale(short target) {
  const short* greateOrEqualToTargetIt = std::lower_bound(VOLUME_IN_LOG_SCALE_ARR, VOLUME_IN_LOG_SCALE_ARR + VOLUME_IN_LOG_SCALE_ARR_N, target);
  // 找>=101场景 || 找>=0场景
  if (greateOrEqualToTargetIt == VOLUME_IN_LOG_SCALE_ARR + VOLUME_IN_LOG_SCALE_ARR_N || greateOrEqualToTargetIt == VOLUME_IN_LOG_SCALE_ARR) {
    return -1;
  }
  return greateOrEqualToTargetIt - VOLUME_IN_LOG_SCALE_ARR - 1;
}
// 没有下一级时返回-1
short nextLevelInLinearScale(short target) {
  const short* greateOrEqualToTargetIt = std::upper_bound(VOLUME_IN_LOG_SCALE_ARR, VOLUME_IN_LOG_SCALE_ARR + VOLUME_IN_LOG_SCALE_ARR_N, target);
  // 找>100场景
  if (greateOrEqualToTargetIt == VOLUME_IN_LOG_SCALE_ARR + VOLUME_IN_LOG_SCALE_ARR_N) {
    return -1;
  }
  return greateOrEqualToTargetIt - VOLUME_IN_LOG_SCALE_ARR;
}
}  // namespace VolumeIterate

constexpr int VolumeWidget::MIN_VOLUME, VolumeWidget::MAX_VOLUME;

VolumeWidget::VolumeWidget(QBoxLayout::Direction direction, QWidget* parent) : QWidget{parent} {
  const bool bMuteVolume = Configuration().value(MemoryKey::VIDEO_PLAYER_MUTE.name, MemoryKey::VIDEO_PLAYER_MUTE.v).toBool();
  mMuteAct = DualIconCheckableAction::CreateMuteAction(this, bMuteVolume);

  mMuteBtn = new QToolButton{this};
  mMuteBtn->setDefaultAction(mMuteAct);

  const int sliderValue = Configuration().value(MemoryKey::VIDEO_PLAYER_VOLUME.name, MemoryKey::VIDEO_PLAYER_VOLUME.v).toInt();
  mVolumeSlider = new ClickableSlider{Qt::Orientation::Horizontal, this};
  mVolumeSlider->setRange(MIN_VOLUME, MAX_VOLUME);
  mVolumeSlider->setValue(sliderValue);

  mVolumeValLabel = new QLabel{GetLabelText(), this};
  mVolumeValLabel->setToolTip("Volume value[log value]");

  mLayout = new QBoxLayout{direction, this};
  mLayout->setContentsMargins(0, 0, 0, 0);
  mLayout->setSpacing(0);
  mLayout->addWidget(mMuteBtn);
  mLayout->addWidget(mVolumeSlider);
  mLayout->addWidget(mVolumeValLabel);

  connect(mMuteAct, &QAction::toggled, this, &VolumeWidget::mutedStateToggled);
  mVolumeSlider->regMouseEventProcessor(std::bind(&VolumeWidget::onMouseEventProcessor, this, std::placeholders::_1));
}

VolumeWidget::~VolumeWidget() {
  Configuration().setValue(MemoryKey::VIDEO_PLAYER_MUTE.name, isMuted());
  Configuration().setValue(MemoryKey::VIDEO_PLAYER_VOLUME.name, volumeVal());
}

void VolumeWidget::onMouseEventProcessor(const int sliderValue) {
  // slider axis is linear scale, convert linear value to log scale, pass log value to QMediaPlayer
  const bool bNeedMuted{sliderValue == 0};
  if (mMuteAct->isChecked() != bNeedMuted) {
    mMuteAct->toggle();
  }
  mVolumeValLabel->setText(GetLabelText());
  emit sliderVolumeChanged((int)volumeValLog());
}

QString VolumeWidget::GetLabelText() const {
  return QString::asprintf("%d[%d]", (int)volumeVal(), (int)volumeValLog());
}

bool VolumeWidget::reqLogVolumeChange(bool bIncrease) {
  const short currentLogscalex100 = volumeValLogx100Int();
  short newLinearScale{-1};
  using namespace VolumeIterate;
  if (bIncrease) {
    newLinearScale = nextLevelInLinearScale(currentLogscalex100);
  } else {
    newLinearScale = previousLevelInLinearScale(currentLogscalex100);
  }
  if (newLinearScale == -1) {
    return false;
  }
  mVolumeSlider->setValue(newLinearScale);
  onMouseEventProcessor(newLinearScale);
  return true;
}

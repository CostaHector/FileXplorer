#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "VolumeWidget.h"
#include "EndToExposePrivateMember.h"

class VolumeWidgetTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {
    Configuration().clear();
    // init status ok[no configuration]
    {
      VolumeWidget volWid;
      QCOMPARE(volWid.isMuted(), false);
      QCOMPARE(volWid.volumeVal(), 100);
      QCOMPARE(volWid.GetLabelText(), "100[100]");  // floor(100*100/100)=100
      QCOMPARE(volWid.mVolumeValLabel->text(), "100[100]");

      // 鼠标事件才可以触发onMouseEventProcessor, 进而更新mVolumeValLabel
      volWid.mVolumeSlider->setValue(88);
      volWid.mMuteAct->setChecked(true);
    }

    // status keep in memory ok
    {
      QVERIFY(Configuration().contains(MemoryKey::VIDEO_PLAYER_MUTE.name));
      QCOMPARE(Configuration().value(MemoryKey::VIDEO_PLAYER_MUTE.name).toBool(), true);
      QVERIFY(Configuration().contains(MemoryKey::VIDEO_PLAYER_VOLUME.name));
      QCOMPARE(Configuration().value(MemoryKey::VIDEO_PLAYER_VOLUME.name).toInt(), 88);
    }

    // init status ok[exist configuration]
    {
      VolumeWidget volWid;
      QCOMPARE(volWid.isMuted(), true);
      QCOMPARE(volWid.volumeVal(), 88);
      QCOMPARE(volWid.mVolumeValLabel->text(), "88[77]");  // floor(88*88/100)=77
    }
  }

  void cleanupTestCase() {  //
    // Configuration().clear();
  }

  void convertBetweenLinearScaleAndLogScale_ok() {
    using namespace VolumeIterate;
    QCOMPARE(linearValue2LogValue(-1), 0);  // out of range
    QCOMPARE(linearValue2LogValue(0), 0);
    QCOMPARE(linearValue2LogValue(1), 1);
    QCOMPARE(linearValue2LogValue(2), 4);
    QCOMPARE(linearValue2LogValue(99), 9801);
    QCOMPARE(linearValue2LogValue(100), 10000);
    QCOMPARE(linearValue2LogValue(101), 10000);  // out of range

    QCOMPARE(logValue2LinearValue(-1), 0);  // out of range
    QCOMPARE(logValue2LinearValue(0), 0);
    QCOMPARE(logValue2LinearValue(1), 1);
    QCOMPARE(logValue2LinearValue(4), 2);
    QCOMPARE(logValue2LinearValue(9801), 99);
    QCOMPARE(logValue2LinearValue(10000), 100);
    QCOMPARE(logValue2LinearValue(10001), 100);  // out of range
  }

  void logScaleLevelIteratePrevious_ok() {
    using namespace VolumeIterate;
    QCOMPARE(previousLevelInLinearScale(0), -1);  // 平方刻度的0的没有上一档, -1
    QCOMPARE(previousLevelInLinearScale(1), 0);   // 平方刻度的1的上一个档是0, 对应的线性值是0
    QCOMPARE(previousLevelInLinearScale(2), 1);   // 平方刻度的2的上一个档是1, 对应的线性值是1

    QCOMPARE(previousLevelInLinearScale(300), 17);  // 平方刻度的300的上一个档是289, 对应的线性值是17
    QCOMPARE(previousLevelInLinearScale(323), 17);  // 平方刻度的323的上一个档是289, 对应的线性值是17
    QCOMPARE(previousLevelInLinearScale(324), 17);  // 平方刻度的324的上一个档是289, 对应的线性值是17
    QCOMPARE(previousLevelInLinearScale(325), 18);  // 平方刻度的325的上一个档是324, 对应的线性值是18

    QCOMPARE(previousLevelInLinearScale(10000), 99);  // 平方刻度的10000的上一个档是9981, 对应的线性值是99
  }

  void logScaleLevelIterateNext_ok() {
    using namespace VolumeIterate;
    QCOMPARE(nextLevelInLinearScale(0), 1);  // 平方刻度的0的下一个档是1, 对应的线性值是1
    QCOMPARE(nextLevelInLinearScale(1), 2);  // 平方刻度的1的下一个档是4, 对应的线性值是2
    QCOMPARE(nextLevelInLinearScale(2), 2);  // 平方刻度的2的下一个档是4, 对应的线性值是2

    QCOMPARE(nextLevelInLinearScale(300), 18);  // 平方刻度的300的下一个档是324, 对应的线性值是18
    QCOMPARE(nextLevelInLinearScale(323), 18);  // 平方刻度的323的下一个档是324, 对应的线性值是18
    QCOMPARE(nextLevelInLinearScale(324), 19);  // 平方刻度的324的下一个档是361, 对应的线性值是19
    QCOMPARE(nextLevelInLinearScale(325), 19);  // 平方刻度的325的下一个档是361, 对应的线性值是19

    QCOMPARE(nextLevelInLinearScale(10000), -1);  // 平方刻度的10000的没有下一档, -1
  }

  void mutedStateToggled_Signal_ok() {
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_MUTE.name, false);
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_VOLUME.name, 77);

    VolumeWidget volWid;
    QSignalSpy mutedStateToggledSpy(&volWid, &VolumeWidget::mutedStateToggled);
    volWid.mMuteAct->setChecked(true);
    QCOMPARE(mutedStateToggledSpy.count(), 1);
    QCOMPARE(mutedStateToggledSpy.takeLast(), (QVariantList{true}));
    // 静音状态<->非静音状态切换 不会修改音量条
    QCOMPARE(volWid.volumeVal(), 77);

    volWid.mMuteAct->toggle();
    QCOMPARE(mutedStateToggledSpy.count(), 1);
    QCOMPARE(mutedStateToggledSpy.takeLast(), (QVariantList{false}));
    QCOMPARE(volWid.volumeVal(), 77);
  }

  void sliderVolumeChanged_signal_ok() {
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_MUTE.name, true);
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_VOLUME.name, 66);

    VolumeWidget volWid;
    QSignalSpy mutedStateToggledSpy(&volWid, &VolumeWidget::mutedStateToggled);
    QSignalSpy sliderVolumeChangedSpy(&volWid, &VolumeWidget::sliderVolumeChanged);

    QVERIFY(volWid.mVolumeSlider->isRegistered());

    // 音量条不变, 不会触发 静音状态切换, 也不会触发sliderVolumeChanged信号
    volWid.mVolumeSlider->mousePressEventCore(66);
    QCOMPARE(mutedStateToggledSpy.count(), 0);
    QCOMPARE(sliderVolumeChangedSpy.count(), 0);
    QCOMPARE(volWid.isMuted(), true);
    QCOMPARE(volWid.volumeVal(), 66);
    QCOMPARE(volWid.mVolumeValLabel->text(), volWid.GetLabelText());

    // 音量条>0则可以解除静音状态
    volWid.mVolumeSlider->mousePressEventCore(55);
    QCOMPARE(mutedStateToggledSpy.count(), 1);
    QCOMPARE(mutedStateToggledSpy.takeLast(), (QVariantList{false}));
    QCOMPARE(volWid.isMuted(), false);
    QCOMPARE(sliderVolumeChangedSpy.count(), 1);
    QCOMPARE(sliderVolumeChangedSpy.takeLast(), (QVariantList{30}));  // 55*55/100=3025
    QCOMPARE(volWid.volumeVal(), 55);
    QCOMPARE(volWid.mVolumeValLabel->text(), volWid.GetLabelText());

    // 音量条=0则可以进入静音状态
    volWid.mVolumeSlider->mousePressEventCore(0);
    volWid.mVolumeSlider->setValue(0);
    QCOMPARE(mutedStateToggledSpy.count(), 1);
    QCOMPARE(mutedStateToggledSpy.takeLast(), (QVariantList{true}));
    QCOMPARE(volWid.isMuted(), true);
    QCOMPARE(sliderVolumeChangedSpy.count(), 1);
    QCOMPARE(sliderVolumeChangedSpy.takeLast(), (QVariantList{0}));  // 0*0/100
    QCOMPARE(volWid.volumeVal(), 0);
    QCOMPARE(volWid.mVolumeValLabel->text(), volWid.GetLabelText());
  }

  void reqLogVolumeDecrease_ok() {
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_MUTE.name, false);
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_VOLUME.name, 3);
    VolumeWidget volWid;
    QSignalSpy sliderVolumeChangedSpy(&volWid, &VolumeWidget::sliderVolumeChanged);

    QCOMPARE(volWid.reqLogVolumeDecrease(), true);
    volWid.reqLogVolumeDecrease();
    volWid.reqLogVolumeDecrease();
    QCOMPARE(volWid.reqLogVolumeDecrease(), false);  // 已经到0了
    QCOMPARE(volWid.reqLogVolumeIncrease(), true);
    QVERIFY(sliderVolumeChangedSpy.count() > 0);
  }

  void reqLogVolumeIncrease_ok() {
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_MUTE.name, false);
    Configuration().setValue(MemoryKey::VIDEO_PLAYER_VOLUME.name, 97);
    VolumeWidget volWid;
    QSignalSpy sliderVolumeChangedSpy(&volWid, &VolumeWidget::sliderVolumeChanged);

    // 100 没有上一级
    QCOMPARE(volWid.reqLogVolumeIncrease(), true);
    volWid.reqLogVolumeIncrease();
    volWid.reqLogVolumeIncrease();
    QCOMPARE(volWid.reqLogVolumeIncrease(), false);  // 已经到100了
    QCOMPARE(volWid.reqLogVolumeDecrease(), true);
    QVERIFY(sliderVolumeChangedSpy.count() > 0);
  }
};

#include "VolumeWidgetTest.moc"
REGISTER_TEST(VolumeWidgetTest, false)

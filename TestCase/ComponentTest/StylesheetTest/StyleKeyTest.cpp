#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "StyleKey.h"

#include "Configuration.h"
#include "PreferenceActions.h"

using namespace StyleKey;

class StyleKeyTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {
    QImage imgValid{Style::DEF_BACKGROUND_IMAGE};
    QCOMPARE(imgValid.isNull(), false);
  }

  void init() {  //
    Configuration().clear();
  }

  void KV_ok() {
    QVERIFY(BACKGROUND_IMAGE.pChecker != nullptr);
    QVERIFY(BACKGROUND_OVERLAY_OPACITY.pChecker != nullptr);
    QVERIFY(STYLE_PRESET.pChecker != nullptr);
    QVERIFY(STYLE_THEME.pChecker != nullptr);

    QVERIFY(BACKGROUND_IMAGE.iconUrlStr() != nullptr);
    QVERIFY(BACKGROUND_IMAGE.toolTipStr() != nullptr);

    QVERIFY(BACKGROUND_IMAGE.isCurValuePassChecker());
    QVERIFY(BACKGROUND_OVERLAY_OPACITY.isCurValuePassChecker());
    QVERIFY(STYLE_PRESET.isCurValuePassChecker());
    QVERIFY(STYLE_THEME.isCurValuePassChecker());

    QCOMPARE(BACKGROUND_IMAGE.toVariant(), Style::DEF_BACKGROUND_IMAGE);
    QCOMPARE(BACKGROUND_OVERLAY_OPACITY.toVariant(), Style::DEF_BACKGROUND_OVERLAY_OPACITY);
    QCOMPARE(STYLE_PRESET.toVariant(), (int)Style::DEFAULT_STYLE_PRESET);
    QCOMPARE(STYLE_THEME.toVariant(), (int)Style::DEFAULT_STYLE_THEME);
  }

  void onBgImgOverlayOpacityChanged_ok() {
    QVERIFY(BACKGROUND_OVERLAY_OPACITY.pChecker(BACKGROUND_OVERLAY_OPACITY.v.data.i));
    QVERIFY(BACKGROUND_OVERLAY_OPACITY.changedCallback != nullptr);
    QVERIFY(BACKGROUND_OVERLAY_OPACITY.v.data.i != 140);
    QVERIFY(GetBgOverlayOpacity() != 140);

    const QVariantList expectReason{(int)(Notifier::ChangedReason::OVERLAY_OPACITY)};

    QSignalSpy notifierSpy{&Notifier::instance(), &Notifier::styleChanged};
    setConfig(BACKGROUND_OVERLAY_OPACITY, 140);  // 同时有 overlay 和 图片, 图片更新
    QCOMPARE(notifierSpy.count(), 1);
    QCOMPARE(notifierSpy.takeLast(), expectReason);
    QCOMPARE(GetBgOverlayOpacity(), 140);
    QImage img140_0 = GetBgImage();

    setConfig(BACKGROUND_OVERLAY_OPACITY, 140);  // unchange, skip emit, 图片不更新
    QCOMPARE(notifierSpy.count(), 0);
    QCOMPARE(GetBgOverlayOpacity(), 140);
    QImage img140_1 = GetBgImage();
    QCOMPARE(img140_1, img140_0);

    setConfig(BACKGROUND_OVERLAY_OPACITY, 0);  // 无需 overlay, 纯图片, 图片更新
    QCOMPARE(GetBgOverlayOpacity(), 0);
    QCOMPARE(notifierSpy.count(), 1);
    QCOMPARE(notifierSpy.takeLast(), expectReason);
    QImage img0 = GetBgImage();
    QVERIFY(img0 != img140_0);

    setConfig(BACKGROUND_OVERLAY_OPACITY, 255);  // 无需 图片, 纯色背景, 图片更新
    QCOMPARE(GetBgOverlayOpacity(), 255);
    QCOMPARE(notifierSpy.count(), 1);
    QCOMPARE(notifierSpy.takeLast(), expectReason);
    QImage img255 = GetBgImage();
    QVERIFY(img255 != img140_0);

    setConfig(BACKGROUND_OVERLAY_OPACITY, Style::DEF_BACKGROUND_OVERLAY_OPACITY);  // 恢复初始值
  }

  void onBgImgPathChanged_ok() {
    const int overlayValue = GetBgOverlayOpacity();
    QVERIFY(overlayValue != 0);
    QVERIFY(overlayValue != 255);

    const QString newBrokenImgFile{__FILE__};

    const QImage brokenImgInvalid{newBrokenImgFile};
    QCOMPARE(brokenImgInvalid.isNull(), true);

    QVERIFY(BACKGROUND_IMAGE.v.data.str != newBrokenImgFile);

    const QVariantList expectReason{(int)(Notifier::ChangedReason::IMAGE_PATH)};

    QSignalSpy notifierSpy{&Notifier::instance(), &Notifier::styleChanged};
    setConfig(BACKGROUND_IMAGE, newBrokenImgFile);  // 设置损坏的图片(非图片)
    QCOMPARE(notifierSpy.count(), 1);
    QCOMPARE(notifierSpy.takeLast(), expectReason);
    const QImage imgBroken = GetBgImage();
    QCOMPARE(imgBroken.isNull(), true);  // file but not not image

    setConfig(BACKGROUND_IMAGE, newBrokenImgFile);  // 再次设置损坏的图片(非图片), 无变更
    QCOMPARE(notifierSpy.count(), 0);
    QCOMPARE(GetBgImage().isNull(), true);  // file but not not image

    setConfig(BACKGROUND_IMAGE, Style::DEF_BACKGROUND_IMAGE);  // 设置正常的图片, ok
    QCOMPARE(notifierSpy.count(), 1);
    QCOMPARE(notifierSpy.takeLast(), expectReason);
    const QImage imgOk = GetBgImage();
    QCOMPARE(imgOk.isNull(), false);  // file but not not image
  }

  void onStylePresetChanged_ok() {
    const auto& inst = g_PreferenceActions();
    QVERIFY(inst.CurStylePreset() != Style::StylePresetE::PRESET_FUSION);

    QSignalSpy notifierSpy{&Notifier::instance(), &Notifier::styleChanged};  // preset 变化时, 不触发
    setConfig(STYLE_PRESET, (int)Style::StylePresetE::PRESET_FUSION);
    QCOMPARE(notifierSpy.count(), 0);
    QCOMPARE(inst.CurStylePreset(), Style::StylePresetE::PRESET_FUSION);

    setConfig(STYLE_PRESET, (int)Style::DEFAULT_STYLE_PRESET);  // 恢复初始值
    QCOMPARE(notifierSpy.count(), 0);
    QCOMPARE(inst.CurStylePreset(), Style::DEFAULT_STYLE_PRESET);
  }

  void onStyleThemeChanged() {
    const auto& inst = g_PreferenceActions();
    QVERIFY(inst.CurStyleTheme() != Style::StyleThemeE::THEME_NONE);

    const QVariantList expectReason{(int)(Notifier::ChangedReason::THEME)};

    QSignalSpy notifierSpy{&Notifier::instance(), &Notifier::styleChanged};  // theme 变化时, 触发

    setConfig(STYLE_THEME, (int)Style::StyleThemeE::THEME_NONE);  // 改到None
    QCOMPARE(notifierSpy.count(), 1);
    QCOMPARE(notifierSpy.takeLast(), expectReason);
    QCOMPARE(inst.CurStyleTheme(), Style::StyleThemeE::THEME_NONE);
    const QImage imgThemeNone = GetBgImage();
    QCOMPARE(imgThemeNone.isNull(), false);

    setConfig(STYLE_THEME, (int)Style::StyleThemeE::THEME_NONE);  // 再次改到None, 没有变更
    QCOMPARE(notifierSpy.count(), 0);
    QCOMPARE(inst.CurStyleTheme(), Style::StyleThemeE::THEME_NONE);

    setConfig(STYLE_THEME, (int)Style::StyleThemeE::THEME_DARK_MOON_FOG);  // 改到 Dark
    QCOMPARE(notifierSpy.count(), 1);
    QCOMPARE(notifierSpy.takeLast(), expectReason);
    QCOMPARE(inst.CurStyleTheme(), Style::StyleThemeE::THEME_DARK_MOON_FOG);
    const QImage imgThemeDark = GetBgImage();
    QCOMPARE(imgThemeDark.isNull(), false);

    setConfig(STYLE_THEME, (int)Style::DEFAULT_STYLE_THEME);  // 恢复初始值 Light
    QCOMPARE(notifierSpy.count(), 1);
    QCOMPARE(notifierSpy.takeLast(), expectReason);
    QCOMPARE(inst.CurStyleTheme(), Style::DEFAULT_STYLE_THEME);
    const QImage imgThemeLight = GetBgImage();
    QCOMPARE(imgThemeLight.isNull(), false);

    QVERIFY(imgThemeLight != imgThemeDark);
  }
};

#include "StyleKeyTest.moc"
REGISTER_TEST(StyleKeyTest, false)

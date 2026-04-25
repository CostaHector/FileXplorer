#include "StyleKey.h"
#include <QPainter>
#include <QImage>
#include <QFile>
#include "Configuration.h"
#include "StyleSheetGetter.h"
#include "PreferenceActions.h"

namespace {

QString& bgImagePathInternal() {
  static QString bgImagePath{[]() -> QString {
    QString pthFromSetting = getConfig(StyleKey::BACKGROUND_IMAGE).toString();
    if (!StyleKey::BACKGROUND_IMAGE.pChecker(pthFromSetting)) {
      pthFromSetting = StyleKey::BACKGROUND_IMAGE.v.data.str;
    }
    return pthFromSetting;
  }()};
  return bgImagePath;
}

// 任何地方失败都直接返回, 不保证图片有效性
QImage BgWithOverlayOpacityEffect() {
  const int bgOverlayValue = StyleKey::GetBgOverlayOpacity();

  static const auto GetBaseColor = [](int bgOverlayValue) -> QColor {
    static const auto& prefActs = g_PreferenceActions();
    const Style::StyleThemeE styleE = prefActs.CurStyleTheme();
    static const auto& styleGetterInst = StyleSheetGetter::GetInst();
    const QString viewPanelBg{styleGetterInst.GetColorValue("Background/View/Panel", styleE)};
    QColor baseColor{viewPanelBg};
    baseColor.setAlpha(bgOverlayValue);
    return baseColor;
  };

  // 255: no image needed
  if (bgOverlayValue == 255) {
    QImage img{1024, 768, QImage::Format_ARGB32};
    const QColor baseColor = GetBaseColor(bgOverlayValue);
    img.fill(baseColor);
    return img;
  }

  const QString imgPath = bgImagePathInternal();
  if (imgPath.isEmpty()) {
    return {};
  }

  QImage img{imgPath};
  if (img.isNull()) {
    return {};
  }

  // 0: no overlay needed,
  if (bgOverlayValue == 0) {
    return img;
  }

  const QColor baseColor = GetBaseColor(bgOverlayValue);
  QPainter painter(&img);
  painter.fillRect(img.rect(), baseColor);
  return img;
}

QImage& bgImageInternal() {
  static QImage img{BgWithOverlayOpacityEffect()};
  return img;
}

int& bgOverlayOpacityInternal() {
  static int bgOverlayOpacity{[]() -> int {
    int opacityFromSetting = getConfig(StyleKey::BACKGROUND_OVERLAY_OPACITY).toInt();
    if (!StyleKey::BACKGROUND_OVERLAY_OPACITY.pChecker(opacityFromSetting)) {
      opacityFromSetting = StyleKey::BACKGROUND_OVERLAY_OPACITY.v.data.i;
    }
    return opacityFromSetting;
  }()};
  return bgOverlayOpacity;
}

void WhenBackGroundImageParms() {
  QImage img = BgWithOverlayOpacityEffect();
  bgImageInternal().swap(img);
}

} // namespace

namespace StyleKey {

const QImage& GetBgImage() {
  return bgImageInternal();
}

int GetBgOverlayOpacity() {
  return bgOverlayOpacityInternal();
}

bool onBgImgPathChanged(const QVariant& newBgImgPath) {
  if (!BACKGROUND_IMAGE.pChecker(newBgImgPath)) {
    return false;
  }
  const QString newPath = newBgImgPath.toString();
  if (newPath == bgImagePathInternal()) {
    return false;
  }
  bgImagePathInternal() = newPath;

  WhenBackGroundImageParms();

  emit Notifier::instance().styleChanged();
  return true;
}

bool onBgImgOverlayOpacityChanged(const QVariant& newBgOverlayOpacity) {
  if (!BACKGROUND_OVERLAY_OPACITY.pChecker(newBgOverlayOpacity)) {
    return false;
  }
  const int newOpacity = newBgOverlayOpacity.toInt();
  if (newOpacity == bgOverlayOpacityInternal()) {
    return false;
  }
  bgOverlayOpacityInternal() = newOpacity;

  WhenBackGroundImageParms();

  emit Notifier::instance().styleChanged();
  return true;
}

bool onStylePresetChanged(const QVariant& newStylePreset) {
  static auto& prefActs = g_PreferenceActions();
  const int newStylePresetInt = newStylePreset.toInt();
  if (newStylePresetInt != (int)prefActs.CurStylePreset()) {
    prefActs.setCheckedStylePreset(newStylePresetInt);
  }
  prefActs.initStylePreset();

  return true;
}

bool onStyleThemeChanged(const QVariant& newStyleTheme) {
  static auto& prefActs = g_PreferenceActions();
  const int newStyleThemeInt = newStyleTheme.toInt();
  if (newStyleThemeInt != (int)prefActs.CurStyleTheme()) {
    prefActs.setCheckedStyleTheme(newStyleThemeInt);
  }
  prefActs.initStyleTheme(true);

  WhenBackGroundImageParms();

  emit Notifier::instance().styleChanged();
  return true;
}

} // namespace StyleKey

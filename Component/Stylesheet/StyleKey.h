#ifndef STYLEKEY_H
#define STYLEKEY_H

#include "KV.h"
#include "StyleEnum.h"
#include <QObject>

namespace StyleKey {
using namespace RawVariant;
using namespace GeneralDataType;
using namespace ValueChecker;

const QImage& GetBgImage();
int GetBgOverlayOpacity();

bool onBgImgPathChanged(const QVariant& newBgImgPath);
bool onBgImgOverlayOpacityChanged(const QVariant& newBgOverlayOpacity);

using namespace Style;

constexpr KV BACKGROUND_IMAGE{"StyleKey/BACKGROUND_IMAGE", Var{Style::DEF_BACKGROUND_IMAGE}, GeneralDataType::Type::IMAGE_PATH_OPTIONAL, GeneralFilePathOptionalChecker, onBgImgPathChanged, ":img/IMAGE", "Background image used in QTableView"};
constexpr KV BACKGROUND_OVERLAY_OPACITY{
    "StyleKey/BACKGROUND_OVERLAY_OPACITY", Var{Style::DEF_BACKGROUND_OVERLAY_OPACITY}, GeneralDataType::Type::RANGE_INT, GeneralIntRangeChecker<0, 255>, onBgImgOverlayOpacityChanged, ":/styles/BACKGROUND_OVERLAY_OPACITY", "Overlay Opacity"};

bool onStylePresetChanged(const QVariant& newStylePreset);
bool onStyleThemeChanged(const QVariant& newStyleTheme);

constexpr KV STYLE_PRESET{"StyleKey/STYLE_PRESET", Var{(int)Style::DEFAULT_STYLE_PRESET}, GeneralDataType::Type::RANGE_INT_STYLE_PRESET, StylePresetEChecker, onStylePresetChanged, ":/styles/STYLE_PRESET", STYLE_PRESET_TOOLTIP};
constexpr KV STYLE_THEME{"StyleKey/STYLE_THEME", Var{(int)Style::DEFAULT_STYLE_THEME}, GeneralDataType::Type::RANGE_INT_STYLE_THEME, StyleThemeEChecker, onStyleThemeChanged, ":/styles/STYLE_THEME", STYLE_THEME_TOOLTIP};

struct Notifier : public QObject {
  Q_OBJECT
 public:
  enum ChangedReason {
    IMAGE_PATH,
    OVERLAY_OPACITY,
    THEME,
  };

  static Notifier& instance() {
    static Notifier inst;
    return inst;
  }
 signals:
  void styleChanged(int changedReason);
};

}  // namespace StyleKey

#endif  // STYLEKEY_H

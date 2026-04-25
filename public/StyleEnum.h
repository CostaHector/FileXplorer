#ifndef STYLEENUM_H
#define STYLEENUM_H

#include <QVariant>

namespace Style {
enum class StylePresetE : int {
  BEGIN = 0,
  PRESET_WINDOWS_VISTA = BEGIN,
  PRESET_WINDOWS,
  PRESET_FUSION,
  PRESET_MACOS,
  BUTT,
};
constexpr StylePresetE DEFAULT_STYLE_PRESET{StylePresetE::PRESET_WINDOWS_VISTA};
constexpr const char* STYLE_PRESET_TOOLTIP{"Toggle predefined styles preset among WindowsVista(0)/Windows(1)/Fusion(2)/macOS(3)"};

inline bool StylePresetEChecker(const QVariant& v) {
  if (!v.isValid()) {
    return false;
  }
  bool bIsInt{false};
  int eValue = v.toInt(&bIsInt);
  if (!bIsInt) {
    return false;
  }
  return (int)StylePresetE::BEGIN <= eValue && eValue < (int)StylePresetE::BUTT;
}

enum class StyleThemeE : int {
  BEGIN = 0,
  THEME_LIGHT = BEGIN,
  THEME_DARK_MOON_FOG,
  THEME_NONE,
  BUTT,
};
constexpr StyleThemeE DEFAULT_STYLE_THEME{StyleThemeE::THEME_LIGHT};
constexpr const char* STYLE_THEME_TOOLTIP{"Toggle color themes among Light(0)/Dark(1)/None(2)"};

inline bool StyleThemeEChecker(const QVariant& v) {
  if (!v.isValid()) {
    return false;
  }
  bool bIsInt{false};
  int eValue = v.toInt(&bIsInt);
  if (!bIsInt) {
    return false;
  }
  return (int)StyleThemeE::BEGIN <= eValue && eValue < (int)StyleThemeE::BUTT;
}

constexpr const char* DEFAULT_FONT_FAMILY {
#ifdef _WIN32
    "Microsoft YaHei UI",
#else
    "Noto Sans",
#endif
};
constexpr const char* DEFAULT_FONT_FAMILY_CODE {
#ifdef _WIN32
    "Consolas",
#else
    "Monospace",
#endif
};

constexpr const char* DEF_BACKGROUND_IMAGE{":styles/BACKGROUND_IMAGE_TABLEVIEW"};
constexpr int DEF_BACKGROUND_OVERLAY_OPACITY{160};
}

#endif // STYLEENUM_H

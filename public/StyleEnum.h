#ifndef STYLEENUM_H
#define STYLEENUM_H

#include <QVariant>

namespace Style {
enum class StyleE : int {
  STYLE_WINDOWS_VISTA = 0,
  STYLE_WINDOWS,
  STYLE_FUSION,
  STYLE_MACOS,
};

enum class StyleSheetE : int {
  STYLESHEET_DEFAULT_NONE = 0,
  STYLESHEET_LIGHT,
  STYLESHEET_DARK_THEME_MOON_FOG,
  BUTT,
};
constexpr StyleSheetE DEFAULT_STYLE_SHEET = StyleSheetE::STYLESHEET_LIGHT;

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

}

#endif // STYLEENUM_H

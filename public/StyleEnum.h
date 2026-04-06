#ifndef STYLEENUM_H
#define STYLEENUM_H

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
}

#endif // STYLEENUM_H

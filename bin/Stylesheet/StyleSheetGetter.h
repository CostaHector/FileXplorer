#ifndef STYLESHEETGETTER_H
#define STYLESHEETGETTER_H

#include <QString>
#include <vector>
#include <memory>
#include <functional>
#include "StyleEnum.h"
#include <QFont>

class StyleSheetTreeModel;

namespace FontCfg {
QFont ReadFont();
QString ReadFontString();
void updateFont(const QFont& newFont);
QString GetFontFamily();

int GetFontSize();
int GetFontSizeTab();
QFont::Weight GetFontWeight();
QFont::Style GetFontStyle();
QString GetFontWeightString();
QString GetFontStyleString();

constexpr const char* mFontFamilyDef{Style::DEFAULT_FONT_FAMILY};
constexpr int mFontSizeDef{14};                                  // 14 px
constexpr int mFontSizeTabDef{mFontSizeDef + 1};                 // 15 px
constexpr QFont::Weight mFontWeightDef{QFont::Weight::Normal};   // normal; bold
constexpr QFont::Style mFontStyleDef{QFont::Style::StyleNormal}; // normal; italic
}

namespace ColorCfg {
QString GetColorBackgroundGeneral(Style::StyleSheetE styleE);
QString GetColorBackgroundAlternateRow(Style::StyleSheetE styleE);
QString GetColorBackgroundHover(Style::StyleSheetE styleE);
QString GetColorBackgroundSelectedInactive(Style::StyleSheetE styleE);
QString GetColorBackgroundSelectedActive(Style::StyleSheetE styleE);
QString GetColorBackgroundMenu(Style::StyleSheetE styleE);
QString GetColorBackgroundMenuChecked(Style::StyleSheetE styleE);
QString GetColorBackgroundMenuSelected(Style::StyleSheetE styleE);
QString GetColorGridLine(Style::StyleSheetE styleE);
QString GetColorBorderGeneral(Style::StyleSheetE styleE);
QString GetColorBorderMenuRight(Style::StyleSheetE styleE);
QString GetColorForegroundGeneral(Style::StyleSheetE styleE);

QString ColorBackgroundGeneralDef(Style::StyleSheetE styleE);
QString ColorBackgroundAlternateRowDef(Style::StyleSheetE styleE);
QString ColorBackgroundHoverDef(Style::StyleSheetE styleE);
QString ColorBackgroundSelectedInactiveDef(Style::StyleSheetE styleE);
QString ColorBackgroundSelectedActiveDef(Style::StyleSheetE styleE);
QString ColorBackgroundMenuDef(Style::StyleSheetE styleE);
QString ColorBackgroundMenuCheckedDef(Style::StyleSheetE styleE);
QString ColorBackgroundMenuSelectedDef(Style::StyleSheetE styleE);
QString ColorGridLineDef(Style::StyleSheetE styleE);
QString ColorBorderGeneralDef(Style::StyleSheetE styleE);
QString ColorBorderMenuRightDef(Style::StyleSheetE styleE);
QString ColorForegroundGeneralDef(Style::StyleSheetE styleE);
}

class StyleSheetGetter {
public:
  friend class StyleSheetTreeModel;
  static const StyleSheetGetter& GetInst();
  StyleSheetGetter() = default;
  virtual ~StyleSheetGetter() = default;

  using DerivedPtr = std::unique_ptr<StyleSheetGetter>;
  static bool Register(DerivedPtr creator);
  QString operator()(Style::StyleSheetE styleE) const;

private:
  virtual QString GetStyleSheet(Style::StyleSheetE styleE) const { return ""; }

  static std::vector<DerivedPtr>& GetRegistry();
};

#endif

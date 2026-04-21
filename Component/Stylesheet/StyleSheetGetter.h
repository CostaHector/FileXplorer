#ifndef STYLESHEETGETTER_H
#define STYLESHEETGETTER_H

#include <QString>
#include <vector>
#include <memory>
#include <QFont>
#include "StyleEnum.h"

class StyleSheetTreeModel;

namespace FontCfg {
bool isCoarseEqual(const QFont& lhs, const QFont& rhs);
QString Font2String(const QFont& font);
QFont ReadGeneralFont();
QString ReadFontGeneralString();
QString GetFontFamily();
QString GetFontFamilyCode();

int GetFontSize();
int GetFontSizeTab();
QFont::Weight GetFontWeight();
QFont::Style GetFontStyle();

QString GetFontWeightString(int weightE);
QString GetFontGeneralWeightString();
QString GetFontStyleString(int styleE);
QString GetFontGeneralStyleString();

constexpr const char* mFontFamilyDef{Style::DEFAULT_FONT_FAMILY};
constexpr const char* mFontFamilyCodeDef{Style::DEFAULT_FONT_FAMILY_CODE};
constexpr int mFontSizeDef{14};                                  // 14 px
constexpr int mFontSizeTabDef{mFontSizeDef + 1};                 // 15 px
constexpr QFont::Weight mFontWeightDef{QFont::Weight::Normal};   // normal; bold
constexpr QFont::Style mFontStyleDef{QFont::Style::StyleNormal}; // normal; italic
}

class StyleSheetGetter {
public:
  friend class StyleSheetTreeModel;
  static const StyleSheetGetter& GetInst();
  StyleSheetGetter();
  void init() const;
  virtual ~StyleSheetGetter() = default;

  using DerivedPtr = std::unique_ptr<StyleSheetGetter>;
  static bool Register(DerivedPtr creator);
  QString operator()(Style::StyleSheetE styleE) const;

  const QVariant& defValue(const QString& key, bool* bKeyExist = nullptr) const;
  const QVariant& curValue(const QString& key, bool* bKeyExist = nullptr) const;
  const Style::CfgDefCur& defCurValue(const QString& key, bool* bKeyExist = nullptr) const;
  void InitColorUnrelatedValue(const QString& keyCore, const QVariant& def) const;
  void InitColorValue(const QString& keyCore, const QVariant& lightDef, const QVariant& darkDef) const;
  QString GetColorValue(const QString& keyCore, Style::StyleSheetE styleE) const;

  int UpdateCurValue(const QVariantHash& cfg) const;
  static void WriteIntoSettingsCore(const StyleSheetGetter& self);
  void WriteIntoSettings() const;

private:
  virtual QString GetStyleSheet(Style::StyleSheetE styleE) const { return ""; }

  mutable QHash<QString, Style::CfgDefCur> mStyleCfg;
  static std::vector<DerivedPtr>& GetRegistry();
};

#endif

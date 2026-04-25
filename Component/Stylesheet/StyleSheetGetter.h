#ifndef STYLESHEETGETTER_H
#define STYLESHEETGETTER_H

#include <QString>
#include <vector>
#include <memory>
#include <QFont>
#include "StyleEnum.h"
#include "StyleItemData.h"

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
constexpr int mFontSizeDef{14};                                   // 14 px
constexpr int mFontSizeTabDef{mFontSizeDef + 1};                  // 15 px
constexpr QFont::Weight mFontWeightDef{QFont::Weight::Normal};    // normal; bold
constexpr QFont::Style mFontStyleDef{QFont::Style::StyleNormal};  // normal; italic
}  // namespace FontCfg

class StyleSheetGetter {
 public:
  friend class StyleSheetTreeModel;
  static const StyleSheetGetter& GetInst();
  StyleSheetGetter();
  void init() const;
  virtual ~StyleSheetGetter() = default;

  using DerivedPtr = std::unique_ptr<StyleSheetGetter>;
  static bool Register(DerivedPtr creator);
  QString operator()(Style::StyleThemeE styleE) const;

  const QVariant& defValue(const QString& key, bool* bKeyExist = nullptr) const;
  const QVariant& curValue(const QString& key, bool* bKeyExist = nullptr) const;
  const StyleItemData& defCurValue(const QString& key, bool* bKeyExist = nullptr) const;

  QString GetColorValue(const QString& keyCore, Style::StyleThemeE styleE) const;
  QString GetNoColorValue(const QString& keyCore) const;
  static QString toRgbaString(const QString& colorStr);

  int UpdateCurValue(const QVariantHash& cfg) const;
  static void WriteIntoSettingsCore(const StyleSheetGetter& self);
  void WriteIntoSettings() const;

  std::unique_ptr<StyleTreeNode> GetModelData() const;

 private:
  virtual QString GetStyleSheet(Style::StyleThemeE styleE) const { return ""; }

  static QHash<QString, StyleItemData> GetPairDict();
  static constexpr const char ROOT_NODE_NAME[] = "StyleSheet";
  static constexpr int ROOT_NODE_NAME_ARR_LEN = sizeof(ROOT_NODE_NAME) / sizeof(ROOT_NODE_NAME[0]);

  mutable QHash<QString, StyleItemData> mStyleCfg;
  static std::vector<DerivedPtr>& GetRegistry();
};

#endif

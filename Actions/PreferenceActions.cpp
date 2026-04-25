#include "PreferenceActions.h"
#include "Configuration.h"
#include "StyleKey.h"
#include "MenuToolButton.h"
#include "StyleSheet.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "StyleSheetGetter.h"
#include <QApplication>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

PreferenceActions::PreferenceActions(QObject* parent) //
  : QObject{parent}
  , mStylePresetIntAction{this}
  , mStyleThemeIntAction{this} {
  PRESET_WINDOWS_VISTA = new (std::nothrow) QAction{QIcon(":/styles/PRESET_WINDOWS_VISTA"), "WindowsVista", this};
  PRESET_WINDOWS_VISTA->setCheckable(true);
  PRESET_WINDOWS_VISTA->setChecked(true);
  PRESET_WINDOWS_VISTA->setToolTip("Change UI Preset to Windows Vista");

  PRESET_WINDOWS = new (std::nothrow) QAction{QIcon(":/styles/PRESET_WINDOWS_TRADITIONAL"), "Windows", this};
  PRESET_WINDOWS->setCheckable(true);
  PRESET_WINDOWS->setToolTip("Change UI Preset to Windows Traditional (win98)");

  PRESET_FUSION = new (std::nothrow) QAction{QIcon(":/styles/PRESET_FUSION"), "Fusion", this};
  PRESET_FUSION->setCheckable(true);
  PRESET_FUSION->setToolTip("Change UI Preset to Fusion");

  PRESET_MACOS = new (std::nothrow) QAction{QIcon(":/styles/PRESET_MACOS"), "MacOS", this};
  PRESET_MACOS->setCheckable(true);
  PRESET_MACOS->setToolTip("Change UI Preset to Macos (not supported in windows)");

  using namespace Style;
  mStylePresetIntAction.init({{PRESET_WINDOWS_VISTA, StylePresetE::PRESET_WINDOWS_VISTA},
                              {PRESET_WINDOWS, StylePresetE::PRESET_WINDOWS},
                              {PRESET_FUSION, StylePresetE::PRESET_FUSION},
                              {PRESET_MACOS, StylePresetE::PRESET_MACOS}}, //
                             StylePresetE::PRESET_WINDOWS_VISTA,
                             QActionGroup::ExclusionPolicy::Exclusive);

  const int styleInt = getConfig(StyleKey::STYLE_PRESET).toInt();
  setCheckedStylePreset(styleInt);

  THEME_NONE = new (std::nothrow) QAction{QIcon(":/styles/THEME_NONE"), tr("None"), this};
  THEME_NONE->setCheckable(true);
  THEME_NONE->setToolTip("Change theme to default(built-in)");

  THEME_LIGHT = new (std::nothrow) QAction{QIcon(":/styles/THEME_LIGHT"), tr("Light"), this};
  THEME_LIGHT->setCheckable(true);
  THEME_LIGHT->setToolTip("Change theme to light");

  THEME_DARK_MOON_FOG = new (std::nothrow) QAction{QIcon(":/styles/THEME_DARK_MOON_FOG"), tr("Dark"), this};
  THEME_DARK_MOON_FOG->setCheckable(true);
  THEME_DARK_MOON_FOG->setToolTip("Change theme to dark");

  mStyleThemeIntAction.init({{THEME_NONE, StyleThemeE::THEME_NONE}, {THEME_LIGHT, StyleThemeE::THEME_LIGHT}, {THEME_DARK_MOON_FOG, StyleThemeE::THEME_DARK_MOON_FOG}}, //
                            Style::DEFAULT_STYLE_THEME,
                            QActionGroup::ExclusionPolicy::Exclusive);
  const int stylesheetInt = getConfig(StyleKey::STYLE_THEME).toInt();
  setCheckedStyleTheme(stylesheetInt);

  STYLESHEET_MGR = new (std::nothrow) QAction{QIcon{":/styles/STYLESHEET_MGR"}, tr("Stylesheet"), this};

  subscribe();
}

void PreferenceActions::setCheckedStylePreset(int stylePreset) {
  mStylePresetIntAction.setCheckedIfActionExist(stylePreset);
}

void PreferenceActions::setCheckedStyleTheme(int styleTheme) {
  mStyleThemeIntAction.setCheckedIfActionExist(styleTheme);
}

void PreferenceActions::initStylePreset() const {
  if (auto* checkedStyleAct = mStylePresetIntAction.getActionGroup()->checkedAction()) {
    const QString& styleName{checkedStyleAct->text().toLower()};
    qApp->setStyle(styleName);
  }
}

void PreferenceActions::ApplyNewStyleSheet(const QString& newStyleSheetStr) {
  qApp->setStyleSheet(newStyleSheetStr);
}

void PreferenceActions::initStyleTheme(bool bThemeChanged) const {
  const Style::StyleThemeE themeEnum{CurStyleTheme()};
  if (bThemeChanged) {
    StyleSheet::setGlobalDarkMode(themeEnum == Style::StyleThemeE::THEME_DARK_MOON_FOG);
  }
  const QString qssContents = StyleSheetGetter::GetInst()(themeEnum);
  ApplyNewStyleSheet(qssContents);
}

bool PreferenceActions::onStylePresetChanged(const QAction* pStyleAct) const {
  CHECK_NULLPTR_RETURN_FALSE(pStyleAct);
  const QString& styleName{pStyleAct->text().toLower()};
  LOG_D("setStyle[%s]", qPrintable(styleName));
  setConfig(StyleKey::STYLE_PRESET, (int) CurStylePreset());
  return true;
}

bool PreferenceActions::onStyleThemeChanged(const QAction* pStyleSheetAct) const {
  CHECK_NULLPTR_RETURN_FALSE(pStyleSheetAct);
  LOG_D("StyleSheet[%s]", qPrintable(pStyleSheetAct->text()));
  setConfig(StyleKey::STYLE_THEME, (int) CurStyleTheme());
  return true;
}

QToolBar* PreferenceActions::GetStyleAndStyleSheetToolbar(QWidget* parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  MenuToolButton* uiStyleToolButton = new (std::nothrow)
      MenuToolButton(mStylePresetIntAction.getActionEnumAscendingList(), QToolButton::InstantPopup, Qt::ToolButtonStyle::ToolButtonTextBesideIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_16, parent);
  CHECK_NULLPTR_RETURN_NULLPTR(uiStyleToolButton);
  uiStyleToolButton->SetCaption(QIcon{":/styles/STYLE_PRESET"}, tr("Style Preset"), Style::STYLE_PRESET_TOOLTIP);

  MenuToolButton* colorThemeToolButton = new (std::nothrow)
      MenuToolButton(mStyleThemeIntAction.getActionEnumAscendingList(), QToolButton::InstantPopup, Qt::ToolButtonStyle::ToolButtonTextBesideIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_16, parent);
  CHECK_NULLPTR_RETURN_NULLPTR(colorThemeToolButton);
  colorThemeToolButton->SetCaption(QIcon{":/styles/STYLE_THEME"}, tr("Color Theme"), Style::STYLE_THEME_TOOLTIP);

  QToolBar* styleAndStylesheetToolbar = new (std::nothrow) QToolBar{"Style/Stylesheet", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(styleAndStylesheetToolbar);
  styleAndStylesheetToolbar->addWidget(uiStyleToolButton);
  styleAndStylesheetToolbar->addWidget(colorThemeToolButton);
  styleAndStylesheetToolbar->addAction(STYLESHEET_MGR);
  styleAndStylesheetToolbar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  styleAndStylesheetToolbar->setOrientation(Qt::Orientation::Vertical);
  styleAndStylesheetToolbar->setIconSize(QSize{IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16});
  return styleAndStylesheetToolbar;
}

void PreferenceActions::subscribe() {
  connect(mStylePresetIntAction.getActionGroup(), &QActionGroup::triggered, this, &PreferenceActions::onStylePresetChanged);
  connect(mStyleThemeIntAction.getActionGroup(), &QActionGroup::triggered, this, &PreferenceActions::onStyleThemeChanged);
}

Style::StylePresetE PreferenceActions::CurStylePreset() const {
  return mStylePresetIntAction.curVal();
}

Style::StyleThemeE PreferenceActions::CurStyleTheme() const {
  return mStyleThemeIntAction.curVal();
}

PreferenceActions& g_PreferenceActions() {
  static PreferenceActions ins;
  return ins;
}

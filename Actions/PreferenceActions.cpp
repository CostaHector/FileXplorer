#include "PreferenceActions.h"
#include "MemoryKey.h"
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
  , //
  mStyleIntAction{this}
  , //
  mStyleSheetIntAction{this} {
  STYLE_WINDOWS_VISTA = new (std::nothrow) QAction{QIcon(":/styles/STYLE_WINDOWS_VISTA"), "WindowsVista", this};
  STYLE_WINDOWS_VISTA->setCheckable(true);
  STYLE_WINDOWS_VISTA->setChecked(true);
  STYLE_WINDOWS_VISTA->setToolTip("Change UI style to Windows Vista");

  STYLE_WINDOWS = new (std::nothrow) QAction{QIcon(":/styles/STYLE_WINDOWS_TRADITIONAL"), "Windows", this};
  STYLE_WINDOWS->setCheckable(true);
  STYLE_WINDOWS->setToolTip("Change UI style to Windows Traditional (win98)");

  STYLE_FUSION = new (std::nothrow) QAction{QIcon(":/styles/STYLE_FUSION"), "Fusion", this};
  STYLE_FUSION->setCheckable(true);
  STYLE_FUSION->setToolTip("Change UI style to Fusion");

  STYLE_MACOS = new (std::nothrow) QAction{QIcon(":/styles/STYLE_MACOS"), "MacOS", this};
  STYLE_MACOS->setCheckable(true);
  STYLE_MACOS->setToolTip("Change UI style to Macos (not supported in windows)");

  using namespace Style;
  mStyleIntAction.init({{STYLE_WINDOWS_VISTA, StyleE::STYLE_WINDOWS_VISTA},
                        {STYLE_WINDOWS, StyleE::STYLE_WINDOWS},
                        {STYLE_FUSION, StyleE::STYLE_FUSION},
                        {STYLE_MACOS, StyleE::STYLE_MACOS}}, //
                       StyleE::STYLE_WINDOWS_VISTA,
                       QActionGroup::ExclusionPolicy::Exclusive);
  int styleInt = Configuration().value("STYLE_NAME", (int) mStyleIntAction.defVal()).toInt();
  mStyleIntAction.setCheckedIfActionExist(styleInt);

  STYLESHEET_NONE = new (std::nothrow) QAction{QIcon(":/styles/STYLESHEET_NONE"), tr("None"), this};
  STYLESHEET_NONE->setCheckable(true);
  STYLESHEET_NONE->setToolTip("Change stylesheet to default");

  STYLESHEET_LIGHT = new (std::nothrow) QAction{QIcon(":/styles/STYLESHEET_LIGHT_THEME_SUN"), tr("Light"), this};
  STYLESHEET_LIGHT->setCheckable(true);
  STYLESHEET_LIGHT->setToolTip("Change stylesheet to light");

  STYLESHEET_DARK_THEME_MOON_FOG = new (std::nothrow) QAction{QIcon(":/styles/STYLESHEET_DARK_THEME_MOON_FOG"), tr("Dark"), this};
  STYLESHEET_DARK_THEME_MOON_FOG->setCheckable(true);
  STYLESHEET_DARK_THEME_MOON_FOG->setToolTip("Change stylesheet to dark");

  mStyleSheetIntAction.init({{STYLESHEET_NONE, StyleSheetE::STYLESHEET_DEFAULT_NONE},
                             {STYLESHEET_LIGHT, StyleSheetE::STYLESHEET_LIGHT},
                             {STYLESHEET_DARK_THEME_MOON_FOG, StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG}}, //
                            Style::DEFAULT_STYLE_SHEET,
                            QActionGroup::ExclusionPolicy::Exclusive);
  const int stylesheetInt = Configuration().value("STYLESHEET_NAME", (int) Style::DEFAULT_STYLE_SHEET).toInt();
  mStyleSheetIntAction.setCheckedIfActionExist(stylesheetInt);

  STYLESHEET_MGR = new (std::nothrow) QAction{QIcon{":/styles/STYLESHEET_MGR"}, tr("Stylesheet"), this};

  subscribe();
}

PreferenceActions::~PreferenceActions() {
  Configuration().setValue("STYLE_NAME", (int) CurStyle());
  Configuration().setValue("STYLESHEET_NAME", (int) CurStyleSheet());
}

void PreferenceActions::initAppStyle() const {
  if (auto* checkedStyleAct = mStyleIntAction.getActionGroup()->checkedAction()) {
    const QString& styleName{checkedStyleAct->text().toLower()};
    qApp->setStyle(styleName);
  }
}

void PreferenceActions::ApplyNewStyleSheet(const QString& newStyleSheetStr) {
  qApp->setStyleSheet(newStyleSheetStr);
}

void PreferenceActions::initStyleSheet(bool bThemeChanged) const {
  const Style::StyleSheetE stylesheetEnum{CurStyleSheet()};
  if (bThemeChanged) {
    StyleSheet::setGlobalDarkMode(stylesheetEnum == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG);
  }
  const QString qssContents = StyleSheetGetter::GetInst()(stylesheetEnum);
  ApplyNewStyleSheet(qssContents);
}

bool PreferenceActions::onAppStyleChanged(const QAction* pStyleAct) const {
  CHECK_NULLPTR_RETURN_FALSE(pStyleAct);
  const QString& styleName{pStyleAct->text().toLower()};
  LOG_D("setStyle[%s]", qPrintable(styleName));
  initAppStyle();
  return true;
}

bool PreferenceActions::onStylesheetChanged(const QAction* pStyleSheetAct) const {
  CHECK_NULLPTR_RETURN_FALSE(pStyleSheetAct);
  LOG_D("StyleSheet[%s]", qPrintable(pStyleSheetAct->text()));
  initStyleSheet(true);
  return true;
}

QToolBar* PreferenceActions::GetStyleAndStyleSheetToolbar(QWidget* parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  MenuToolButton* uiStyleToolButton = new (std::nothrow) MenuToolButton(mStyleIntAction.getActionEnumAscendingList(),
                                                                        QToolButton::InstantPopup,
                                                                        Qt::ToolButtonStyle::ToolButtonTextBesideIcon,
                                                                        IMAGE_SIZE::TABS_ICON_IN_MENU_16,
                                                                        parent);
  CHECK_NULLPTR_RETURN_NULLPTR(uiStyleToolButton);
  uiStyleToolButton->SetCaption(QIcon{":/styles/STYLE_SETTING"},
                                tr("App UI Style"),
                                "Change application style (Windows Vista, Windows, Fusion, macOS)");

  MenuToolButton* colorThemeToolButton = new (std::nothrow) MenuToolButton(mStyleSheetIntAction.getActionEnumAscendingList(),
                                                                           QToolButton::InstantPopup,
                                                                           Qt::ToolButtonStyle::ToolButtonTextBesideIcon,
                                                                           IMAGE_SIZE::TABS_ICON_IN_MENU_16,
                                                                           parent);
  CHECK_NULLPTR_RETURN_NULLPTR(colorThemeToolButton);
  colorThemeToolButton->SetCaption(QIcon{":/styles/STYLESHEET_SETTING"}, tr("Color Theme"), "Toggle between Light and Dark color schemes");

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
  connect(mStyleIntAction.getActionGroup(), &QActionGroup::triggered, this, &PreferenceActions::onAppStyleChanged);
  connect(mStyleSheetIntAction.getActionGroup(), &QActionGroup::triggered, this, &PreferenceActions::onStylesheetChanged);
}

Style::StyleE PreferenceActions::CurStyle() const {
  return mStyleIntAction.curVal();
}

Style::StyleSheetE PreferenceActions::CurStyleSheet() const {
  return mStyleSheetIntAction.curVal();
}

PreferenceActions& g_PreferenceActions() {
  static PreferenceActions ins;
  return ins;
}

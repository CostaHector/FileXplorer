#include "PreferenceActions.h"
#include "MemoryKey.h"
#include "MenuToolButton.h"
#include "StyleSheet.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include <QApplication>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

PreferenceActions::PreferenceActions(QObject* parent)//
  : QObject{parent},//
  mStyleIntAction{this},//
  mStyleSheetIntAction{this} {

  STYLE_WINDOWS_VISTA = new (std::nothrow) QAction{QIcon(":img/STYLE_WINDOWS_VISTA"), "WindowsVista", this};
  STYLE_WINDOWS_VISTA->setCheckable(true);
  STYLE_WINDOWS_VISTA->setChecked(true);
  STYLE_WINDOWS_VISTA->setToolTip("Change UI style to Windows Vista");

  STYLE_WINDOWS = new (std::nothrow) QAction{QIcon(":img/STYLE_WINDOWS_TRADITIONAL"), "Windows", this};
  STYLE_WINDOWS->setCheckable(true);
  STYLE_WINDOWS->setToolTip("Change UI style to Windows Traditional (win98)");

  STYLE_FUSION = new (std::nothrow) QAction{QIcon(":img/STYLE_FUSION"), "Fusion", this};
  STYLE_FUSION->setCheckable(true);
  STYLE_FUSION->setToolTip("Change UI style to Fusion");

  STYLE_MACOS = new (std::nothrow) QAction{QIcon(":img/STYLE_MACOS"), "MacOS", this};
  STYLE_MACOS->setCheckable(true);
  STYLE_MACOS->setToolTip("Change UI style to Macos (not supported in windows)");

  using namespace Style;
  mStyleIntAction.init({{STYLE_WINDOWS_VISTA, StyleE::STYLE_WINDOWS_VISTA},
                        {STYLE_WINDOWS, StyleE::STYLE_WINDOWS},
                        {STYLE_FUSION, StyleE::STYLE_FUSION},
                        {STYLE_MACOS, StyleE::STYLE_MACOS}},//
                       StyleE::STYLE_WINDOWS_VISTA, QActionGroup::ExclusionPolicy::Exclusive);
  int styleInt = Configuration().value("STYLE_NAME", (int)mStyleIntAction.defVal()).toInt();
  auto* checkedStyleAct = mStyleIntAction.setCheckedIfActionExist(styleInt);
  onSetAppStyle(checkedStyleAct);

  STYLESHEET_DEFAULT_LIGHT = new (std::nothrow) QAction{QIcon(":img/STYLESHEET_LIGHT_THEME_SUN"), tr("Light"), this};
  STYLESHEET_DEFAULT_LIGHT->setCheckable(true);
  STYLESHEET_DEFAULT_LIGHT->setChecked(true);
  STYLESHEET_DEFAULT_LIGHT->setToolTip("Change stylesheet to default");

  STYLESHEET_DARK_THEME_MOON_FOG = new (std::nothrow) QAction{QIcon(":img/STYLESHEET_DARK_THEME_MOON_FOG"), tr("Dark"), this};
  STYLESHEET_DARK_THEME_MOON_FOG->setCheckable(true);
  STYLESHEET_DARK_THEME_MOON_FOG->setToolTip("Change stylesheet to dark");

  mStyleSheetIntAction.init({{STYLESHEET_DEFAULT_LIGHT, StyleSheetE::STYLESHEET_DEFAULT_LIGHT},
                             {STYLESHEET_DARK_THEME_MOON_FOG, StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG}},//
                            StyleSheetE::STYLESHEET_DEFAULT_LIGHT, QActionGroup::ExclusionPolicy::Exclusive);
  int stylesheetInt = Configuration().value("STYLESHEET_NAME", (int)mStyleSheetIntAction.defVal()).toInt();
  auto* checkedStyleSheetAct = mStyleSheetIntAction.setCheckedIfActionExist(stylesheetInt);
  onSetStylesheet(checkedStyleSheetAct);

  Subscribe();
}

PreferenceActions::~PreferenceActions() {
  Configuration().setValue("STYLE_NAME", (int)CurStyle());
  Configuration().setValue("STYLESHEET_NAME", (int)CurStyleSheet());
}

bool PreferenceActions::onSetAppStyle(const QAction* pStyleAct) {
  CHECK_NULLPTR_RETURN_FALSE(pStyleAct);
  const QString& styleName = pStyleAct->text().toLower();
  qApp->setStyle(styleName);
  LOG_D("setStyle to %s", qPrintable(styleName));
  return true;
}

bool PreferenceActions::onSetStylesheet(const QAction* pStyleSheetAct) {
  CHECK_NULLPTR_RETURN_FALSE(pStyleSheetAct);
  using namespace Style;
  StyleSheetE stylesheetEnum = mStyleSheetIntAction.act2Enum(pStyleSheetAct);
  static QString theme2QssContent[(int)StyleSheetE::BUTT]{};
  if (theme2QssContent[(int)stylesheetEnum].isEmpty()) {
    QFile qssFile;
    switch (stylesheetEnum) {
      case StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
        qssFile.setFileName(":stylesheet/dark.qss");
        break;
      default: // anothing except dark
        qssFile.setFileName(":stylesheet/default.qss");
        break;
    }
    if (!qssFile.open(QFile::ReadOnly | QFile::Text)) {
      LOG_ERR_NP("Unable to set stylesheet, file[%s] not found", qPrintable(qssFile.fileName()));
      return false;
    }
    QTextStream ts(&qssFile);
    theme2QssContent[(int)stylesheetEnum] = ts.readAll();
    qssFile.close();
  }
  qApp->setStyleSheet(theme2QssContent[(int)stylesheetEnum]);
  LOG_D("Style Changed to %s", qPrintable(pStyleSheetAct->text()));
  return true;
}

QToolBar *PreferenceActions::GetStyleAndStyleSheetToolbar(QWidget *parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  MenuToolButton* uiStyleToolButton = new (std::nothrow) MenuToolButton(
      mStyleIntAction.getActionEnumAscendingList(),
      QToolButton::InstantPopup,
      Qt::ToolButtonStyle::ToolButtonTextBesideIcon,
      IMAGE_SIZE::TABS_ICON_IN_MENU_24,
      parent);
  CHECK_NULLPTR_RETURN_NULLPTR(uiStyleToolButton);
  uiStyleToolButton->SetCaption(QIcon{":img/STYLE_SETTING"}, tr("App UI Style"), "Change application style (Windows Vista, Windows, Fusion, macOS)");
  uiStyleToolButton->UpdateCaptionForInstantPopMode();

  MenuToolButton* colorThemeToolButton = new (std::nothrow) MenuToolButton(
      mStyleSheetIntAction.getActionEnumAscendingList(),
      QToolButton::InstantPopup,
      Qt::ToolButtonStyle::ToolButtonTextBesideIcon,
      IMAGE_SIZE::TABS_ICON_IN_MENU_24,
      parent);
  CHECK_NULLPTR_RETURN_NULLPTR(colorThemeToolButton);
  colorThemeToolButton->SetCaption(QIcon{":img/STYLESHEET_SETTING"}, tr("Color Theme"), "Toggle between Light and Dark color schemes");
  colorThemeToolButton->UpdateCaptionForInstantPopMode();

  QToolBar* styleAndStylesheetToolbar = new (std::nothrow) QToolBar{"Style/Stylesheet", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(styleAndStylesheetToolbar);
  styleAndStylesheetToolbar->addWidget(uiStyleToolButton);
  styleAndStylesheetToolbar->addWidget(colorThemeToolButton);
  styleAndStylesheetToolbar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  styleAndStylesheetToolbar->setOrientation(Qt::Orientation::Vertical);
  styleAndStylesheetToolbar->setStyleSheet("QToolBar { min-width: 128px;max-width: 128px; }");
  return styleAndStylesheetToolbar;
}

void PreferenceActions::Subscribe() {
  connect(mStyleIntAction.getActionGroup(), &QActionGroup::triggered, this, &PreferenceActions::onSetAppStyle);
  connect(mStyleSheetIntAction.getActionGroup(), &QActionGroup::triggered, this, &PreferenceActions::onSetStylesheet);
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

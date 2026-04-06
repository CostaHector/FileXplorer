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
  auto* checkedStyleAct = mStyleIntAction.setCheckedIfActionExist(styleInt);
  onSetAppStyle(checkedStyleAct);

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
  auto* checkedStyleSheetAct = mStyleSheetIntAction.setCheckedIfActionExist(stylesheetInt);
  onSetStylesheet(checkedStyleSheetAct);

  subscribe();
}

PreferenceActions::~PreferenceActions() {
  Configuration().setValue("STYLE_NAME", (int) CurStyle());
  Configuration().setValue("STYLESHEET_NAME", (int) CurStyleSheet());
}

bool PreferenceActions::onSetAppStyle(const QAction* pStyleAct) {
  CHECK_NULLPTR_RETURN_FALSE(pStyleAct);
  const QString& styleName = pStyleAct->text().toLower();
  qApp->setStyle(styleName);
  LOG_D("setStyle to %s", qPrintable(styleName));
  return true;
}

const QString& GetQssFileContents(const Style::StyleSheetE stylesheetEnum) {
  using namespace Style;
  static QString theme2QssContent[(int) StyleSheetE::BUTT]{};
  if (!theme2QssContent[(int) stylesheetEnum].isEmpty()) {
    return theme2QssContent[(int) stylesheetEnum];
  }

  static const QString EMPTY_QSS_CONTENTS;
  QString qssFileName;
  switch (stylesheetEnum) {
    case StyleSheetE::STYLESHEET_LIGHT:
      qssFileName = ":stylesheet/default.qss";
      break;
    case StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG:
      qssFileName = ":stylesheet/dark.qss";
      break;
    case StyleSheetE::STYLESHEET_DEFAULT_NONE: {
      return EMPTY_QSS_CONTENTS;
    }
    default: {
      LOG_W("invalid stylesheetEnum[%d]", (int) stylesheetEnum);
      return EMPTY_QSS_CONTENTS;
    }
  }

  QFile qssFile{qssFileName};
  if (!qssFile.open(QFile::ReadOnly)) {
    LOG_ERR_NP("Unable to set stylesheet, file[%s] not found", qPrintable(qssFile.fileName()));
    return EMPTY_QSS_CONTENTS;
  }
  theme2QssContent[(int) stylesheetEnum] = QString::fromUtf8(qssFile.readAll());
  qssFile.close();
  return theme2QssContent[(int) stylesheetEnum];
}

bool PreferenceActions::onSetStylesheet(const QAction* pStyleSheetAct) {
  CHECK_NULLPTR_RETURN_FALSE(pStyleSheetAct);
  using namespace Style;
  const StyleSheetE stylesheetEnum{mStyleSheetIntAction.act2Enum(pStyleSheetAct)};
  const QString& qssContents{GetQssFileContents(stylesheetEnum)};
  qApp->setStyleSheet(qssContents);
  StyleSheet::setGlobalDarkMode(stylesheetEnum == StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG);
  LOG_D("Style Changed to %s", qPrintable(pStyleSheetAct->text()));
  return true;
}

QToolBar* PreferenceActions::GetStyleAndStyleSheetToolbar(QWidget* parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  MenuToolButton* uiStyleToolButton = new (std::nothrow) MenuToolButton(mStyleIntAction.getActionEnumAscendingList(),
                                                                        QToolButton::InstantPopup,
                                                                        Qt::ToolButtonStyle::ToolButtonTextBesideIcon,
                                                                        IMAGE_SIZE::TABS_ICON_IN_MENU_24,
                                                                        parent);
  CHECK_NULLPTR_RETURN_NULLPTR(uiStyleToolButton);
  uiStyleToolButton->SetCaption(QIcon{":/styles/STYLE_SETTING"},
                                tr("App UI Style"),
                                "Change application style (Windows Vista, Windows, Fusion, macOS)");

  MenuToolButton* colorThemeToolButton = new (std::nothrow) MenuToolButton(mStyleSheetIntAction.getActionEnumAscendingList(),
                                                                           QToolButton::InstantPopup,
                                                                           Qt::ToolButtonStyle::ToolButtonTextBesideIcon,
                                                                           IMAGE_SIZE::TABS_ICON_IN_MENU_24,
                                                                           parent);
  CHECK_NULLPTR_RETURN_NULLPTR(colorThemeToolButton);
  colorThemeToolButton->SetCaption(QIcon{":/styles/STYLESHEET_SETTING"}, tr("Color Theme"), "Toggle between Light and Dark color schemes");

  QToolBar* styleAndStylesheetToolbar = new (std::nothrow) QToolBar{"Style/Stylesheet", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(styleAndStylesheetToolbar);
  styleAndStylesheetToolbar->addWidget(uiStyleToolButton);
  styleAndStylesheetToolbar->addWidget(colorThemeToolButton);
  styleAndStylesheetToolbar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  styleAndStylesheetToolbar->setOrientation(Qt::Orientation::Vertical);
  styleAndStylesheetToolbar->setStyleSheet("QToolBar { min-width: 128px;max-width: 128px; }");
  return styleAndStylesheetToolbar;
}

void PreferenceActions::subscribe() {
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

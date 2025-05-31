#include "PreferenceActions.h"
#include <QApplication>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include "public/PublicVariable.h"

PreferenceActions::PreferenceActions(QObject* parent) : QObject{parent} {
  STYLE_WINDOWS_VISTA = new (std::nothrow) QAction{QIcon(":img/STYLE_WINDOWS_VISTA"), "windowsvista"};
  STYLE_WINDOWS_VISTA->setCheckable(true);
  STYLE_WINDOWS_VISTA->setChecked(true);
  STYLE_WINDOWS_VISTA->setToolTip("Change style to Windows Vista");

  STYLE_WINDOWS = new (std::nothrow) QAction{QIcon(":img/STYLE_WINDOWS_TRADITIONAL"), "windows"};
  STYLE_WINDOWS->setCheckable(true);
  STYLE_WINDOWS->setToolTip("Change style to Windows Traditional (win98)");

  STYLE_FUSION = new (std::nothrow) QAction{QIcon(":img/STYLE_FUSION"), "fusion"};
  STYLE_FUSION->setCheckable(true);
  STYLE_FUSION->setToolTip("Change style to Fusion");

  STYLE_MACOS = new (std::nothrow) QAction{"macos"};
  STYLE_MACOS->setCheckable(true);
  STYLE_MACOS->setToolTip("Change style to Macos (not supported in windows)");

  STYLE_AG = new (std::nothrow) QActionGroup(this);
  STYLE_AG->addAction(STYLE_WINDOWS_VISTA);
  STYLE_AG->addAction(STYLE_WINDOWS);
  STYLE_AG->addAction(STYLE_FUSION);
  STYLE_AG->addAction(STYLE_MACOS);
  STYLE_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  STYLESHEET_DEFAULT_LIGHT = new (std::nothrow) QAction{QIcon(":img/STYLESHEET_LIGHT_THEME_SUN"), "default"};
  STYLESHEET_DEFAULT_LIGHT->setCheckable(true);
  STYLESHEET_DEFAULT_LIGHT->setChecked(true);
  STYLESHEET_DEFAULT_LIGHT->setToolTip("Change stylesheet to default");

  STYLESHEET_DARK_THEME_MOON_FOG = new (std::nothrow) QAction{QIcon(":img/STYLESHEET_DARK_THEME_MOON_FOG"), "dark"};
  STYLESHEET_DARK_THEME_MOON_FOG->setCheckable(true);
  STYLESHEET_DARK_THEME_MOON_FOG->setToolTip("Change stylesheet to dark");

  STYLESHEET = new (std::nothrow) QActionGroup(this);
  STYLESHEET->addAction(STYLESHEET_DEFAULT_LIGHT);
  STYLESHEET->addAction(STYLESHEET_DARK_THEME_MOON_FOG);
  STYLESHEET->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  PREFERENCE_LIST += STYLE_AG->actions();
  PREFERENCE_LIST.push_back(nullptr);
  PREFERENCE_LIST += STYLESHEET->actions();

  subscribe();
  PostActions();
}

bool PreferenceActions::onSetAppStyle(QAction* pAct) {
  if (pAct == nullptr) {
    qWarning("pAct is nullptr");
    return false;
  }
  static const QStringList styles{"windows", "windowsvista", "fusion", "macos"};
  const QString& styleName = pAct->text();
  if (!styles.contains(styleName)) {
    qWarning("styleName[%s] is not supported", qPrintable(styleName));
    return false;
  }
  PreferenceSettings().setValue("STYLE_NAME", styleName);
  qApp->setStyle(styleName);
  qDebug("qApp->setStyle: %s", qPrintable(styleName));
  return true;
}

void setGlobalDarkMode(bool enable);

#if defined(Q_OS_WIN)
#include <windows.h>
#include <dwmapi.h> // make sure "dwm.exe" already running in task manager at first
void setDarkTitleBar(QWidget* widget, bool enable) {
  if (widget == nullptr) {
    return;
  }
  HWND hwnd = reinterpret_cast<HWND>(widget->winId());
  BOOL darkMode = enable;
  // 19: DWMWA_USE_IMMERSIVE_DARK_MODE_OLD <- Windows 10 Build 18368.418（19H1）
  // 20: DWMWA_USE_IMMERSIVE_DARK_MODE
  DwmSetWindowAttribute(hwnd, 19 /*DWMWA_USE_IMMERSIVE_DARK_MODE*/, &darkMode, sizeof(darkMode));
}

void setGlobalDarkMode(bool enable) {
  foreach (QWidget* widget, qApp->topLevelWidgets()) {
    setDarkTitleBar(widget, enable);
  }
}
#else
void setGlobalDarkMode(bool enable) {
  if (enable) {
    qDebug("Not suport dark mode now");
  }
}
#endif

bool PreferenceActions::onSetStylesheet(QAction* pAct) {
  if (pAct == nullptr) {
    qWarning("pAct is nullptr");
    return false;
  }
  const QString& stylesheetName = pAct->text();
  static const QStringList stylesheets{"default", "light", "dark"};
  if (!stylesheets.contains(stylesheetName)) {
    qWarning("stylesheetName[%s] is not supported", qPrintable(stylesheetName));
    return false;
  }
  PreferenceSettings().setValue("STYLESHEET_NAME", stylesheetName);
  QFile qssFile;
  if (stylesheetName == "dark") {
    qssFile.setFileName(":stylesheet/dark.qss");
    setGlobalDarkMode(true);
  } else {  // "default" or any stylesheet except light/dark
    qssFile.setFileName(":stylesheet/default.qss");
    setGlobalDarkMode(false);
  }
  if (!qssFile.open(QFile::ReadOnly | QFile::Text)) {
    qWarning("Unable to set stylesheet, file[%s] not found", qPrintable(qssFile.fileName()));
    return false;
  }
  QTextStream ts(&qssFile);
  qApp->setStyleSheet(ts.readAll());
  qssFile.close();
  qDebug("qApp->setStyleSheet: %s", qPrintable(stylesheetName));
  return true;
}

void PreferenceActions::subscribe() {
  connect(STYLE_AG, &QActionGroup::triggered, this, &PreferenceActions::onSetAppStyle);
  connect(STYLESHEET, &QActionGroup::triggered, this, &PreferenceActions::onSetStylesheet);
}

bool PreferenceActions::PostActions() {
  const QString& styleName = PreferenceSettings().value("STYLE_NAME", STYLE_WINDOWS_VISTA->text()).toString();
  for (auto* pAct : STYLE_AG->actions()) {
    if (pAct == nullptr) {
      qWarning("pAct is nullptr");
      continue;
    }
    if (pAct->text() != styleName) {
      continue;
    }
    pAct->setChecked(true);
    emit pAct->triggered(true);
    qDebug("last time styleName is %s", qPrintable(styleName));
    break;
  }
  const QString& stylesheetName = PreferenceSettings().value("STYLESHEET_NAME", STYLESHEET_DEFAULT_LIGHT->text()).toString();
  for (auto* pAct : STYLESHEET->actions()) {
    if (pAct == nullptr) {
      qWarning("pAct is nullptr");
      continue;
    }
    if (pAct->text() != stylesheetName) {
      continue;
    }
    pAct->setChecked(true);
    emit pAct->triggered(true);
    qDebug("last time stylesheetName is %s", qPrintable(stylesheetName));
    break;
  }
  return true;
}

PreferenceActions& g_PreferenceActions() {
  static PreferenceActions ins;
  return ins;
}

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

  STYLESHEET_DEFAULT = new (std::nothrow) QAction{QIcon(":img/STYLESHEET_DEFAULT"), "default"};
  STYLESHEET_DEFAULT->setCheckable(true);
  STYLESHEET_DEFAULT->setChecked(true);
  STYLESHEET_DEFAULT->setToolTip("Change stylesheet to default");

  STYLESHEET_LIGHT_THEME_SUN = new (std::nothrow) QAction{QIcon(":img/STYLESHEET_LIGHT_THEME_SUN"), "light"};
  STYLESHEET_LIGHT_THEME_SUN->setCheckable(true);
  STYLESHEET_LIGHT_THEME_SUN->setToolTip("Change stylesheet to light");

  STYLESHEET_DARK_THEME_MOON_FOG = new (std::nothrow) QAction{QIcon(":img/STYLESHEET_DARK_THEME_MOON_FOG"), "dark"};
  STYLESHEET_DARK_THEME_MOON_FOG->setCheckable(true);
  STYLESHEET_DARK_THEME_MOON_FOG->setToolTip("Change stylesheet to dark");

  STYLESHEET = new (std::nothrow) QActionGroup(this);
  STYLESHEET->addAction(STYLESHEET_DEFAULT);
  STYLESHEET->addAction(STYLESHEET_LIGHT_THEME_SUN);
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
  if (stylesheetName == "light") {
    qssFile.setFileName(":stylesheet/light.qss");
  } else if (stylesheetName == "dark") {
    qssFile.setFileName(":stylesheet/dark.qss");
  } else {  // "default" or any stylesheet except light/dark
    qDebug("qApp->setStyleSheet: default[%s]", qPrintable(stylesheetName));
    qApp->setStyleSheet(
        "QMainWindow::separator {"
        "    width: 1px;"
        "    height: 0px;"
        "}"
        "QDockWidget::separator {"
        "    width: 0px;"
        "    height: 0px;"
        "}");
    return false;
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
  const QString& stylesheetName = PreferenceSettings().value("STYLESHEET_NAME", STYLESHEET_DEFAULT->text()).toString();
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

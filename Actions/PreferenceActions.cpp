#include "PreferenceActions.h"
#include <QApplication>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

PreferenceActions::PreferenceActions(QObject *parent): QObject{parent} {
  STYLE = new QAction{QIcon(":img/STYLE_APP"), "Style"};

  STYLE_WINDOWS_VISTA=new QAction{QIcon(":img/STYLE_WINDOWS_VISTA"), "windowsvista"};
  STYLE_WINDOWS_VISTA->setCheckable(true);
  STYLE_WINDOWS_VISTA->setChecked(true);
  STYLE_WINDOWS_VISTA->setToolTip("Windows Vista Style");

  STYLE_WINDOWS = new QAction{QIcon(":img/STYLE_WINDOWS_TRADITIONAL"), "windows"};
  STYLE_WINDOWS->setCheckable(true);
  STYLE_WINDOWS->setToolTip("Windows Traditional Style");

  STYLE_FUSION=new QAction{QIcon(":img/STYLE_FUSION"), "fusion"};
  STYLE_FUSION->setCheckable(true);

  STYLE_MACOS=new QAction{"macos"};
  STYLE_MACOS->setCheckable(true);

  STYLE_AG = new QActionGroup(this);
  STYLE_AG->addAction(STYLE_WINDOWS_VISTA);
  STYLE_AG->addAction(STYLE_WINDOWS);
  STYLE_AG->addAction(STYLE_FUSION);
  STYLE_AG->addAction(STYLE_MACOS);
  STYLE_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  STYLESHEET_DEFAULT=new QAction{QIcon(":img/STYLESHEET_DEFAULT"), "default"};
  STYLESHEET_DEFAULT->setCheckable(true);
  STYLESHEET_DEFAULT->setChecked(true);
  STYLESHEET_LIGHT_THEME_SUN=new QAction{QIcon(":img/STYLESHEET_LIGHT_THEME_SUN"), "light"};
  STYLESHEET_LIGHT_THEME_SUN->setCheckable(true);
  STYLESHEET_DARK_THEME_MOON_FOG=new QAction{QIcon(":img/STYLESHEET_DARK_THEME_MOON_FOG"), "dark"};
  STYLESHEET_DARK_THEME_MOON_FOG->setCheckable(true);
  STYLESHEET = new QActionGroup(this);
  STYLESHEET->addAction(STYLESHEET_DEFAULT);
  STYLESHEET->addAction(STYLESHEET_LIGHT_THEME_SUN);
  STYLESHEET->addAction(STYLESHEET_DARK_THEME_MOON_FOG);
  STYLESHEET->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  PREFERENCE_LIST += STYLE_AG->actions();
  PREFERENCE_LIST.push_back(nullptr);
  PREFERENCE_LIST += STYLESHEET->actions();

  subscribe();
}

bool PreferenceActions::onSetAppStyle(QAction* pAct) {
  if (pAct == nullptr) {
    qWarning("pAct is nullptr");
    return false;
  }
  static const QStringList styles {"windows", "windowsvista", "fusion", "macos"};
  const QString& styleName = pAct->text();
  if (!styles.contains(styleName)) {
    qWarning("styleName[%s] is not supported", qPrintable(styleName));
    return false;
  }
  qApp->setStyle(styleName);
  qDebug("qApp->setStyle: %s", qPrintable(styleName));
  return true;
}

bool PreferenceActions::onSetStylesheet(QAction* pAct) {
  if (pAct == nullptr) {
    qWarning("pAct is nullptr");
    return false;
  }
  const QString& stylesheet = pAct->text();
  QFile qssFile;
  if (stylesheet == "light") {
    qssFile.setFileName(":qdarkstyle/light/lightstyle.qss");
  } else if (stylesheet == "dark") {
    qssFile.setFileName(":qdarkstyle/dark/darkstyle.qss");
  } else { // "default" or any stylesheet except light/dark
    qWarning("qApp->setStyleSheet: default, stylesheet name[%s]", qPrintable(stylesheet));
    qApp->setStyleSheet("");
    return false;
  }
  if (!qssFile.open(QFile::ReadOnly | QFile::Text)) {
    qWarning("Unable to set stylesheet, file[%s] not found", qPrintable(qssFile.fileName()));
    return false;
  }
  QTextStream ts(&qssFile);
  qApp->setStyleSheet(ts.readAll());
  qssFile.close();
  qDebug("qApp->setStyleSheet: %s", qPrintable(stylesheet));
  return true;
}

void PreferenceActions::subscribe() {
  connect(STYLE_AG, &QActionGroup::triggered, this, &PreferenceActions::onSetAppStyle);
  connect(STYLESHEET, &QActionGroup::triggered, this, &PreferenceActions::onSetStylesheet);
}

PreferenceActions& g_PreferenceActions() {
  static PreferenceActions ins;
  return ins;
}

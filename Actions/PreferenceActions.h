#ifndef PREFERENCEACTIONS_H
#define PREFERENCEACTIONS_H

#include <QObject>
#include <QAction>
#include <QActionGroup>

class PreferenceActions : public QObject {
public:
  explicit PreferenceActions(QObject *parent = nullptr);

  void subscribe();
  bool onSetAppStyle(QAction* pAct);
  bool onSetStylesheet(QAction* pAct);
  bool PostActions();

  // style: "windows", "windowsvista", "fusion", or "macos"
  QAction *STYLE_WINDOWS_VISTA{nullptr}, *STYLE_WINDOWS{nullptr}, *STYLE_FUSION{nullptr}, *STYLE_MACOS{nullptr};
  QActionGroup* STYLE_AG{nullptr};

  QAction *STYLESHEET_DEFAULT_LIGHT{nullptr}, *STYLESHEET_DARK_THEME_MOON_FOG{nullptr};
  QActionGroup *STYLESHEET{nullptr};

  QList<QAction*> PREFERENCE_LIST;
};

PreferenceActions& g_PreferenceActions();

#endif // PREFERENCEACTIONS_H

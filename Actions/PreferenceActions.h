#ifndef PREFERENCEACTIONS_H
#define PREFERENCEACTIONS_H

#include <QObject>
#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include "EnumIntAction.h"
#include "StyleEnum.h"

extern template struct EnumIntAction<Style::StyleE>;
extern template struct EnumIntAction<Style::StyleSheetE>;

class PreferenceActions : public QObject {
public:
  explicit PreferenceActions(QObject *parent = nullptr);
  ~PreferenceActions();

  void Subscribe();
  bool onSetAppStyle(const QAction* pStyleAct);
  bool onSetStylesheet(const QAction* pStyleSheetAct);

  // style: "windows", "windowsvista", "fusion", or "macos"
  QAction *STYLE_WINDOWS_VISTA{nullptr}, *STYLE_WINDOWS{nullptr}, *STYLE_FUSION{nullptr}, *STYLE_MACOS{nullptr};

  QAction *STYLESHEET_DEFAULT_LIGHT{nullptr}, *STYLESHEET_DARK_THEME_MOON_FOG{nullptr};
  QActionGroup *STYLESHEET{nullptr};

  QToolBar* GetStyleAndStyleSheetToolbar(QWidget* parent);

  Style::StyleE CurStyle() const;
  Style::StyleSheetE CurStyleSheet() const;
private:
  EnumIntAction<Style::StyleE> mStyleIntAction;
  EnumIntAction<Style::StyleSheetE> mStyleSheetIntAction;
};

PreferenceActions& g_PreferenceActions();

#endif // PREFERENCEACTIONS_H

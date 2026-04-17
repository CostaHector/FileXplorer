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
  Q_OBJECT
public:
  explicit PreferenceActions(QObject *parent = nullptr);
  ~PreferenceActions();

  QToolBar* GetStyleAndStyleSheetToolbar(QWidget* parent);
  void initAppStyle() const;
  void initStyleSheet(bool bThemeChanged) const;

  Style::StyleE CurStyle() const;
  Style::StyleSheetE CurStyleSheet() const;
  QAction *STYLESHEET_MGR{nullptr};
  static void ApplyNewStyleSheet(const QString& newStyleSheetStr);

private:
  void subscribe();
  bool onAppStyleChanged(const QAction* pStyleAct) const;
  bool onStylesheetChanged(const QAction* pStyleSheetAct) const;

  // style: "windows", "windowsvista", "fusion", or "macos"
  QAction *STYLE_WINDOWS_VISTA{nullptr}, *STYLE_WINDOWS{nullptr}, *STYLE_FUSION{nullptr}, *STYLE_MACOS{nullptr};
  QAction *STYLESHEET_NONE{nullptr}, *STYLESHEET_LIGHT{nullptr}, *STYLESHEET_DARK_THEME_MOON_FOG{nullptr};

  EnumIntAction<Style::StyleE> mStyleIntAction;
  EnumIntAction<Style::StyleSheetE> mStyleSheetIntAction;
};

PreferenceActions& g_PreferenceActions();

#endif // PREFERENCEACTIONS_H

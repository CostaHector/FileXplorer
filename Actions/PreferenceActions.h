#ifndef PREFERENCEACTIONS_H
#define PREFERENCEACTIONS_H

#include <QObject>
#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include "EnumIntAction.h"
#include "StyleEnum.h"

extern template struct EnumIntAction<Style::StylePresetE>;
extern template struct EnumIntAction<Style::StyleThemeE>;

class PreferenceActions : public QObject {
  Q_OBJECT
public:
  explicit PreferenceActions(QObject *parent = nullptr);

  QToolBar* GetStyleAndStyleSheetToolbar(QWidget* parent);
  void initStylePreset() const;
  void initStyleTheme(bool bThemeChanged) const;

  Style::StylePresetE CurStylePreset() const;
  Style::StyleThemeE CurStyleTheme() const;
  void setCheckedStylePreset(int stylePreset);
  void setCheckedStyleTheme(int styleTheme);

  QAction *STYLESHEET_MGR{nullptr};
  static void ApplyNewStyleSheet(const QString& newStyleSheetStr);

private:
  void subscribe();
  bool onStylePresetChanged(const QAction* pStyleAct) const;
  bool onStyleThemeChanged(const QAction* pStyleSheetAct) const;

  // style: "windows", "windowsvista", "fusion", or "macos"
  QAction *PRESET_WINDOWS_VISTA{nullptr}, *PRESET_WINDOWS{nullptr}, *PRESET_FUSION{nullptr}, *PRESET_MACOS{nullptr};
  QAction *THEME_NONE{nullptr}, *THEME_LIGHT{nullptr}, *THEME_DARK_MOON_FOG{nullptr};

  EnumIntAction<Style::StylePresetE> mStylePresetIntAction;
  EnumIntAction<Style::StyleThemeE> mStyleThemeIntAction;
};

PreferenceActions& g_PreferenceActions();

#endif // PREFERENCEACTIONS_H

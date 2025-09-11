#ifndef MENUTOOLBUTTON_H
#define MENUTOOLBUTTON_H
#include "StyleSheet.h"
#include "MemoryKey.h"
#include <QToolButton>
#include <QString>
#include <QList>
#include <QAction>
#include <QActionGroup>

class MenuToolButton : public QToolButton {
public:
  explicit MenuToolButton(QList<QAction*> dropdownActions,
                          QToolButton::ToolButtonPopupMode popupMode = QToolButton::ToolButtonPopupMode::InstantPopup,
                          const Qt::ToolButtonStyle toolButtonStyle = Qt::ToolButtonStyle::ToolButtonTextUnderIcon,
                          const int iconSize = IMAGE_SIZE::TABS_ICON_IN_MENU_48,
                          QWidget* parent = nullptr);
  void SetCaption(const QIcon& icon = {}, const QString& text = "", const QString& tooltip = "");
  void InitDefaultActionFromQSetting(const KV& kv, bool enablePersistentBehavior);
  void UpdateCaptionForInstantPopMode();
};

#endif  // MENUTOOLBUTTON_H

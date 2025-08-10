#ifndef DROPLISTTOOLBUTTON_H
#define DROPLISTTOOLBUTTON_H
#include "StyleSheet.h"
#include <QToolButton>
#include <QString>
#include <QList>
#include <QAction>
#include <QActionGroup>

class DropdownToolButton : public QToolButton {
 public:
  DropdownToolButton(QList<QAction*> dropdownActions,
                     QToolButton::ToolButtonPopupMode popupMode = QToolButton::ToolButtonPopupMode::InstantPopup,
                     const Qt::ToolButtonStyle toolButtonStyle = Qt::ToolButtonStyle::ToolButtonTextUnderIcon,
                     const int iconSize = IMAGE_SIZE::TABS_ICON_IN_MENU_48,
                     QWidget* parent = nullptr);
  void SetCaption(const QIcon& icon = {}, const QString& text = "", const QString& tooltip = "");
  void MemorizeCurrentAction(const QString& memoryKey);
  bool FindAndSetDefaultAction(const QString& memoryValue);
 public Q_SLOTS:
  void onToolButtonActTriggered(QAction* pAct);
 private:
  QString m_memoryKey;
};

#endif  // DROPLISTTOOLBUTTON_H

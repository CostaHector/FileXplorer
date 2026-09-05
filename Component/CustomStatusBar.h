#ifndef CUSTOMSTATUSBAR_H
#define CUSTOMSTATUSBAR_H

#include <QLabel>
#include <QStatusBar>
#include <QToolBar>
#include "ViewSwitchToolBar.h"

enum class STATUS_ALERT_LEVEL { NORMAL = 0, ABNORMAL = 1 };
class CustomStatusBar : public QStatusBar {
  Q_OBJECT
public:
  enum MSG_INDEX {
    BEGIN,
    ITEMS = BEGIN,
    SELECTED,
    MSG,
    BUTT
  };
  static CustomStatusBar* GInstance(CustomStatusBar* pInitValue=nullptr, bool bReset=false);
  explicit CustomStatusBar(QWidget* parent = nullptr);

  void onPathInfoChanged(const int count, const int index = 0);
  void onMsgChanged(const QString& text = "", const STATUS_ALERT_LEVEL alertLvl = STATUS_ALERT_LEVEL::NORMAL);

  QString GetText() const;
  ViewTypeTool::ViewType _GetCurViewType() const {
    return m_viewSwitcher == nullptr ? ViewTypeTool::DEFAULT_VIEW_TYPE : m_viewSwitcher->GetCurViewType();
  }

signals:
  void _viewTypeChanged(const ViewTypeTool::ViewType viewType);

private:
  QList<QLabel*> mLabelsLst; // total count, selected count, message
  ViewSwitchToolBar* m_viewSwitcher{nullptr};
};
#endif  // CUSTOMSTATUSBAR_H

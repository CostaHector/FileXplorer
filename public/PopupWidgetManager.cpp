#include "PopupWidgetManager.h"
#include "PublicCheckMacro.h"
#include "StyleSheet.h"

#include "DevicesDrivesTV.h"
#include "DuplicatesImagesFinder.h"
#include "DuplicateVideosFinder.h"
#include "TorrentsManagerWidget.h"
#include "ConfigsMgr.h"
#include "Archiver.h"
#include "ResourceMonitorPanel.h"
#include "Logger.h"

#include <QEvent>

template class PopupWidgetManager<QWidget>;
template class PopupWidgetManager<DevicesDrivesTV>;
template class PopupWidgetManager<DuplicateVideosFinder>;
template class PopupWidgetManager<DuplicatesImagesFinder>;
template class PopupWidgetManager<TorrentsManagerWidget>;
template class PopupWidgetManager<ConfigsMgr>;
template class PopupWidgetManager<Archiver>;
#ifdef PASSVAULT_ENABLED
#include "PasswordBook.h"
template class PopupWidgetManager<PasswordBook>;
#endif
template class PopupWidgetManager<ResourceMonitorPanel>;

template<typename WidgetType>
PopupWidgetManager<WidgetType>::PopupWidgetManager(QAction* pAction, QWidget* notNullptrParent)
    : QObject{notNullptrParent} {
  CHECK_NULLPTR_RETURN_VOID(pAction);
  CHECK_NULLPTR_RETURN_VOID(notNullptrParent);
  m_action = pAction;

  if (m_action->isCheckable()) {
    connect(m_action, &QAction::toggled, this, &PopupWidgetManager::onActionToggled);
  } else {
    connect(m_action, &QAction::triggered, this, &PopupWidgetManager::onActionToggled);
  }
}

template<typename WidgetType>
PopupWidgetManager<WidgetType>::~PopupWidgetManager() {
  if (m_widget == nullptr) {
    return;
  }
  m_widget->removeEventFilter(this);
  if (m_widget->parent() != nullptr) {
    return;
  }
  delete m_widget;
  m_widget = nullptr;
}

template<typename WidgetType>
void PopupWidgetManager<WidgetType>::onActionToggled(bool checked) {
  if (!checked) {
    if (m_widget != nullptr && !m_widget->isHidden()) {
      m_widget->hide();
    }
    return;
  }

  if (m_widget == nullptr) {
    createWidget();
  }

  if (m_widget != nullptr) {
    m_widget->show();
    m_widget->activateWindow();
    m_widget->raise();
  }
}

template<typename WidgetType>
bool PopupWidgetManager<WidgetType>::eventFilter(QObject* watched, QEvent* event) {
  if (watched == m_widget && event->type() == QEvent::Close) {
    if (m_action->isCheckable()) {
      QSignalBlocker blocker(m_action);   // 只改勾选状态，不触发二次 hide
      m_action->setChecked(false);
    }
    if (m_onCloseCallback != nullptr) {
      m_onCloseCallback();
    }
    event->accept();
    return false; // 让事件继续传给 widget 自己的 closeEvent
  }
  if (watched == m_widget && event->type() == QEvent::Show) {
    if (m_action->isCheckable()) {
      QSignalBlocker blocker(m_action);   // 只改勾选状态，不触发二次 show
      m_action->setChecked(true);
    }
    event->accept();
    return false;  // 让事件继续传给 widget 自己的 showEvent
  }
  return QObject::eventFilter(watched, event);
}

template<typename WidgetType>
void PopupWidgetManager<WidgetType>::createWidget() {
  QWidget* pParentWidget = static_cast<QWidget*>(parent());
  CHECK_NULLPTR_RETURN_VOID(pParentWidget);
  if (m_widgetCreator != nullptr) {
    m_widget = m_widgetCreator(pParentWidget);
  } else {
    m_widget = new WidgetType(pParentWidget);
  }

  if (m_widget == nullptr) {
    LOG_E("Failed to create widget action[%s]", qPrintable(m_action->text()));
    if (m_action->isCheckable()) {
      QSignalBlocker blocker(m_action);
      m_action->setChecked(false);
    }
    return;
  }

  // window flag must ahead of event filter; setWindowFlags must ahead of setWindowFlag
  m_widget->setWindowFlags(m_widget->windowFlags() | Qt::Window);
  m_widget->setWindowModality(Qt::NonModal);
  // m_widget->setWindowFlag(Qt::WindowDoesNotAcceptFocus, true);

  m_widget->installEventFilter(this);
  m_widget->setAttribute(Qt::WA_DeleteOnClose, false);
}


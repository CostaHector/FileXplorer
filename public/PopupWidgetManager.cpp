#include "PopupWidgetManager.h"
#include "PublicMacro.h"
#include "StyleSheet.h"

#include "DevicesDrivesTV.h"
#include "RedundantImageFinder.h"
#include "DuplicateVideosFinder.h"
#include "TorrentsManagerWidget.h"
#include "ConfigsTable.h"
#include "Archiver.h"
#include "LoginQryWidget.h"
#include "MemoryKey.h"
#include "Logger.h"

#include <QEvent>

template class PopupWidgetManager<QWidget>;
template class PopupWidgetManager<DevicesDrivesTV>;
template class PopupWidgetManager<DuplicateVideosFinder>;
template class PopupWidgetManager<RedundantImageFinder>;
template class PopupWidgetManager<TorrentsManagerWidget>;
template class PopupWidgetManager<ConfigsTable>;
template class PopupWidgetManager<Archiver>;
template class PopupWidgetManager<LoginQryWidget>;

template<typename WidgetType>
PopupWidgetManager<WidgetType>::PopupWidgetManager(QAction* pAction, QWidget* notNullptrParent, const QString& geometryCfgKey)
    : QObject{notNullptrParent}, m_geometryCfgKey{geometryCfgKey} {
  CHECK_NULLPTR_RETURN_VOID(notNullptrParent);
  CHECK_NULLPTR_RETURN_VOID(pAction);
  CHECK_FALSE_RETURN_VOID(!geometryCfgKey.isEmpty());
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
  Configuration().setValue(m_geometryCfgKey, m_widget->saveGeometry());
  if (m_widget->parent() != nullptr) {
    return;
  }
  delete m_widget;
  m_widget = nullptr;
}

template<typename WidgetType>
void PopupWidgetManager<WidgetType>::onActionToggled(bool checked) {
  if (!checked) {
    if (m_widget != nullptr && m_widget->isVisible()) {
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
      m_action->setChecked(false);
    }
    if (m_onCloseCallback != nullptr) {
      m_onCloseCallback();
    }
    event->accept();
    return true;
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
    LOG_E("Failed to create widget of type: %s", qPrintable(m_geometryCfgKey));
    if (m_action->isCheckable()) {
      m_action->setChecked(false);
    }
    return;
  }

  if (Configuration().contains(m_geometryCfgKey)) {
    m_widget->restoreGeometry(Configuration().value(m_geometryCfgKey).toByteArray());
  } else {
    m_widget->setGeometry(DEFAULT_GEOMETRY);
  }

  // window flag must ahead of event filter; setWindowFlags must ahead of setWindowFlag
  m_widget->setWindowFlags(m_widget->windowFlags() | Qt::Window);
  m_widget->setWindowModality(Qt::NonModal);
  // m_widget->setWindowFlag(Qt::WindowDoesNotAcceptFocus, true);

  m_widget->installEventFilter(this);
  m_widget->setAttribute(Qt::WA_DeleteOnClose, false);
}


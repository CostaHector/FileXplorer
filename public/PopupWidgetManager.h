#ifndef POPUPWIDGETMANAGER_H
#define POPUPWIDGETMANAGER_H

#include <QWidget>
#include <QAction>

template<typename WidgetType>
class PopupWidgetManager : public QObject {
public:
  explicit PopupWidgetManager(QAction* action, QWidget* notNullptrParent, const QString& geometryCfgKey);
  ~PopupWidgetManager();

  void setWidgetCreator(std::function<WidgetType*(QWidget*)> creator) { m_widgetCreator = creator; }

  void setOnCloseCallback(std::function<void()> callback) { m_onCloseCallback = callback; }
  bool isVisible() const { return widget() != nullptr && widget()->isVisible(); }
  const QWidget* widget() const { return m_widget; }
  QWidget* widget() { return m_widget; }

private slots:
  void onActionToggled(bool checked);

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  void createWidget();

  QAction* m_action{nullptr};
  WidgetType* m_widget{nullptr};
  const QString m_geometryCfgKey;

  std::function<WidgetType*(QWidget*)> m_widgetCreator{nullptr};
  std::function<void()> m_onCloseCallback{nullptr};
};

class QWidget;
class DevicesDrivesTV;
class DuplicateVideosFinder;
class RedundantImageFinder;
class TorrentsManagerWidget;
class ConfigsTable;

extern template class PopupWidgetManager<QWidget>;
extern template class PopupWidgetManager<DevicesDrivesTV>;
extern template class PopupWidgetManager<DuplicateVideosFinder>;
extern template class PopupWidgetManager<RedundantImageFinder>;
extern template class PopupWidgetManager<TorrentsManagerWidget>;
extern template class PopupWidgetManager<ConfigsTable>;
#endif // POPUPWIDGETMANAGER_H

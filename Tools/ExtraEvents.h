#ifndef EXTRAEVENTS_H
#define EXTRAEVENTS_H
#include <QObject>
#include <QWidget>
#include "PopupWidgetManager.h"

class PasswordBook;
class PasswordBookServer;

class ExtraEvents : public QObject {
public:
  explicit ExtraEvents(QWidget* parent);
  void subscribe();
private:
  bool onTogglePasswordServer(bool bLauchServer);
  PasswordBookServer* mPwdBookServer{nullptr};

  PopupWidgetManager<TorrentsManagerWidget>* mTorrentsManager{nullptr};
  PopupWidgetManager<ConfigsMgr>* m_settingSys{nullptr};
  PopupWidgetManager<PasswordBook>* mPwdBook{nullptr};
  PopupWidgetManager<ResourceMonitorPanel>* m_resMonitor{nullptr};
};

#endif  // EXTRAEVENTS_H

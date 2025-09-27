#ifndef EXTRAEVENTS_H
#define EXTRAEVENTS_H
#include <QObject>
#include <QWidget>
#include "PopupWidgetManager.h"

class TorrentsManagerWidget;
class ConfigsTable;
class ExtraEvents : public QObject {
public:
  explicit ExtraEvents(QWidget* parent);
  void subscribe();

private:
  PopupWidgetManager<TorrentsManagerWidget>* mTorrentsManager{nullptr};
  PopupWidgetManager<ConfigsTable>* m_settingSys{nullptr};
};

#endif  // EXTRAEVENTS_H

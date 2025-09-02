#ifndef EXTRAEVENTS_H
#define EXTRAEVENTS_H
#include <QObject>
#include <QWidget>

class TorrentsManagerWidget;
class ConfigsTable;
class ExtraEvents : public QObject {
public:
  explicit ExtraEvents(QWidget* parent = nullptr);
  void subscribe();
private:
  QWidget* parentWidget{nullptr};

  void on_showTorrentsManager(const bool checked);
  TorrentsManagerWidget* mTorrentsManager{nullptr};

  void on_settings(const bool checked);
  ConfigsTable* m_settingSys{nullptr};
};

#endif  // EXTRAEVENTS_H

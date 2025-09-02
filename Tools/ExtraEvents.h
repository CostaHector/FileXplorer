#ifndef EXTRAEVENTS_H
#define EXTRAEVENTS_H
#include <QObject>
#include <QWidget>

class TorrentsManagerWidget;
class ExtraEvents : public QObject {
public:
  explicit ExtraEvents(QWidget* parent = nullptr);
  void subscribe();
private:
  void on_showTorrentsManager(const bool checked);
  QWidget* parentWidget{nullptr};
  TorrentsManagerWidget* mTorrentsManager{nullptr};
};

#endif  // EXTRAEVENTS_H

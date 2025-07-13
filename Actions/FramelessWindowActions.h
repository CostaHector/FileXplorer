#ifndef FRAMELESSWINDOWACTIONS_H
#define FRAMELESSWINDOWACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

class FramelessWindowActions : public QObject {
 public:
  explicit FramelessWindowActions(QObject* parent = nullptr);
  QAction* _EXPAND_RIBBONS;
  QAction* _FRAMELESS;
  QAction* _MINIMIZE;
  QAction* _MAXIMIZE;
  QAction* _CLOSE;
  QActionGroup* FRAMELESS_MENU_RIBBONS;

  QActionGroup* Get_FRAMELESS_MENU_Actions();
};

FramelessWindowActions& g_framelessWindowAg();
#endif  // FRAMELESSWINDOWACTIONS_H

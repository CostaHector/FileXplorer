#ifndef FRAMELESSWINDOWACTIONS_H
#define FRAMELESSWINDOWACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QObject>

class FramelessWindowActions : public QObject {
 public:
  QAction* _EXPAND_RIBBONS;
  QAction* _FRAMELESS;
  QAction* _MINIMIZE;
  QAction* _MAXIMIZE;
  QAction* _CLOSE;
  QActionGroup* FRAMELESS_MENU_RIBBONS;

  explicit FramelessWindowActions(QObject* parent = nullptr);

  auto Get_FRAMELESS_MENU_Actions() -> QActionGroup*;
};

FramelessWindowActions& g_framelessWindowAg();
#endif  // FRAMELESSWINDOWACTIONS_H

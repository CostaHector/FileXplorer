#ifndef RIGHTCLICKMENUACTIONS_H
#define RIGHTCLICKMENUACTIONS_H

#include <QObject>
#include <QAction>
class RightClickMenuActions : public QObject {
  Q_OBJECT
 public:
  explicit RightClickMenuActions(QObject *parent = nullptr);

  QAction* _CALC_MD5_ACT;
  QAction* _PROPERTIES;

 signals:

};
RightClickMenuActions& g_rightClickActions();
#endif // RIGHTCLICKMENUACTIONS_H

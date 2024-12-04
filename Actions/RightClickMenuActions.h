#ifndef RIGHTCLICKMENUACTIONS_H
#define RIGHTCLICKMENUACTIONS_H

#include <QObject>
#include <QAction>
class RightClickMenuActions : public QObject {
  Q_OBJECT
 public:
  explicit RightClickMenuActions(QObject *parent = nullptr);

  QAction* _CALC_MD5_ACT{nullptr};
  QAction* _PROPERTIES{nullptr};
  QAction* _FORCE_REFRESH_FILESYSTEMMODEL{nullptr};
 signals:

};
RightClickMenuActions& g_rightClickActions();
#endif // RIGHTCLICKMENUACTIONS_H

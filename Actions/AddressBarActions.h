#ifndef ADDRESSBARACTIONS_H
#define ADDRESSBARACTIONS_H

#include <QObject>
#include <QAction>
#include <QActionGroup>

class AddressBarActions : public QObject
{
  Q_OBJECT
 public:
  explicit AddressBarActions(QObject *parent = nullptr);

  QActionGroup* GetAddressBarActions();
  QAction* _BACK_TO;
  QAction* _FORWARD_TO;
  QAction* _UP_TO;

  QActionGroup* ADDRESS_CONTROLS;
};

AddressBarActions& g_addressBarActions();

#endif // ADDRESSBARACTIONS_H

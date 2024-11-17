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
  QAction* _BACK_TO{nullptr};
  QAction* _FORWARD_TO{nullptr};
  QAction* _UP_TO{nullptr};
  QActionGroup* ADDRESS_CONTROLS{nullptr};

  QActionGroup* GetFolderIteratorActions();
  QAction* _LAST_FOLDER{nullptr};
  QAction* _NEXT_FOLDER{nullptr};
  QActionGroup* _FOLDER_ITER_CONTROLS{nullptr};
};

AddressBarActions& g_addressBarActions();

#endif // ADDRESSBARACTIONS_H

#ifndef MULTIPAR2ACTIONS_H
#define MULTIPAR2ACTIONS_H

#include <QObject>
#include <QAction>

class MultiPar2Actions : public QObject {
  Q_OBJECT
public:
  static MultiPar2Actions& GetInst();
  explicit MultiPar2Actions(QObject* parent=nullptr);
  QWidget* GetToolBar(QWidget* parent);

  QAction* _CREATE_PAR2_FILES{nullptr};
  QAction* _VERIFY_IF_NEED_RECOVERY{nullptr};
};

#endif // MULTIPAR2ACTIONS_H

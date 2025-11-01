#ifndef RATEACTIONS_H
#define RATEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QMap>

class RateActions : public QObject {
  Q_OBJECT
public:
  static RateActions& GetInst();
  QActionGroup* RATE_AGS {nullptr};
  QAction* _RATE_RECURSIVELY{nullptr};
  QAction* _RATE_RECURSIVELY_OVERRIDE{nullptr};
  QList<QAction*> GetRateActionsList() const {return RATE_ACTIONS_LIST;}

signals:
  void MovieRateChanged(int newRate);
  void MovieRateRecursivelyChanged(bool bOverrideForce);

private:
  explicit RateActions(QObject* parent = nullptr);

  void subscribe();
  void onRateActionTriggered(QAction* pActTriggered);
  QList<QAction*> RATE_ACTIONS_LIST;
};

#endif // RATEACTIONS_H

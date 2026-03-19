#ifndef RATEACTIONS_H
#define RATEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QMap>
#include <QMenu>

class RateActions : public QObject {
  Q_OBJECT
public:
  enum class RateRequestFrom {FROM_BEGIN=0, FILE_XPLORER=FROM_BEGIN, VIDEO_TABLE_VIEW, FROM_BUTT};
  static RateActions& GetInst(RateRequestFrom reqFrom = RateRequestFrom::FILE_XPLORER);
  explicit RateActions(QObject* parent = nullptr);
  const QActionGroup* GetActionGroup() const {return RATE_AGS;}
  QList<QAction*> GetAdjustRateActions() const { return {_INCREASING_RATING, _DECREASING_RATING}; }
  QList<QAction*> GetAllRateActionsList() const {return ALL_RATE_ACTIONS_LIST;}
  QMenu* GetRateMenu(QWidget* notNullParent) const;
  QWidget* GetRateToolButton(QWidget* notNullParent, bool bShortCutEnabled = false) const;

signals:
  void RateMovieReq(int newRate);
  void RateMovieRecursivelyReq(bool bOverrideForce);
  void AdjustRateMovieReq(int delta = 1);
  void AdjustRateMovieRecursivelyReq(int delta = 1);

public slots:
  int onRateMoviesRecursively(const QString& rootPath, bool bOverrideForce, QWidget* parent) const;

private:
  void subscribe();
  void onRateActionTriggered(QAction* pActTriggered);
  QList<QAction*> ALL_RATE_ACTIONS_LIST;

  QActionGroup* RATE_AGS {nullptr};
  QAction* _RATE_RECURSIVELY{nullptr};
  QAction* _RATE_RECURSIVELY_OVERRIDE{nullptr};

  QAction* _INCREASING_RATING{nullptr}, *_DECREASING_RATING{nullptr};
  QAction* _INCREASING_RATETING_RECURSIVELY{nullptr}, *_DECREASING_RATETING_RECURSIVELY{nullptr};
};

#endif // RATEACTIONS_H

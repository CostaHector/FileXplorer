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

  QList<QAction*> GetRateActionsList() const { return RATE_AGS->actions(); }
  QList<QAction*> GetAdjustRateActions() const {return {_INCREASING_RATING, _DECREASING_RATING};}
  QMenu* GetVideoWidRateMenu(QWidget* notNullParent) const;
  QMenu* GetRibbonRateMenu(QWidget* notNullParent) const;

  static int onRateMoviesRecursively(const QString& rootPath, bool bOverrideForce, QWidget* parent);

signals:
  void RateMovieReq(int newRate);
  void RateMovieRecursivelyReq(bool bOverrideForce);
  void AdjustRateMovieReq(int delta = 1);
  void AdjustRateMovieRecursivelyReq(int delta = 1);

private:
  void subscribe();
  bool onRateActionTriggered(const QAction* pActTriggered);

  QActionGroup* RATE_AGS{nullptr};

  QAction* _RATE_RECURSIVELY{nullptr}, *_RATE_RECURSIVELY_OVERRIDE{nullptr};
  QAction* _INCREASING_RATING{nullptr}, *_DECREASING_RATING{nullptr};
  QAction* _INCREASING_RATING_RECURSIVELY{nullptr}, *_DECREASING_RATING_RECURSIVELY{nullptr};
};

#endif // RATEACTIONS_H

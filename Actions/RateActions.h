#ifndef RATEACTIONS_H
#define RATEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QMap>
#include <QMenu>

class RateActions : public QObject {
  Q_OBJECT
public:
  enum class RateRequestFrom {FROM_BEGIN=0, FILE_XPLORER=FROM_BEGIN, THUMBNAIL_VIEWER, VIDEO_TABLE_VIEW, FROM_BUTT};
  static RateActions& GetInst(RateRequestFrom reqFrom = RateRequestFrom::FILE_XPLORER);
  explicit RateActions(QObject* parent = nullptr);
  const QActionGroup* GetActionGroup() const {return RATE_AGS;}
  QList<QAction*> GetAllRateActionsList() const {return ALL_RATE_ACTIONS_LIST;}
  QMenu* GetRateMenu(QWidget* notNullParent) const;
  QWidget* GetRateToolButton(QWidget* notNullParent) const;

signals:
  void MovieRateChanged(int newRate);
  void MovieRateRecursivelyChanged(bool bOverrideForce);

public slots:
  int onRateMoviesRecursively(const QString& rootPath, bool bOverrideForce, QWidget* parent) const;

private:
  void subscribe();
  void onRateActionTriggered(QAction* pActTriggered);
  QList<QAction*> ALL_RATE_ACTIONS_LIST;

  QActionGroup* RATE_AGS {nullptr};
  QAction* _RATE_RECURSIVELY{nullptr};
  QAction* _RATE_RECURSIVELY_OVERRIDE{nullptr};
};

#endif // RATEACTIONS_H

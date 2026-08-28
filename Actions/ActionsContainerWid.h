#ifndef ACTIONSCONTAINERWID_H
#define ACTIONSCONTAINERWID_H

#include <QScrollArea>
#include <QGridLayout>

class ActionsContainerWid : public QScrollArea {
public:
  explicit ActionsContainerWid(Qt::Orientation arrangeOrientation = Qt::Orientation::Vertical, int eachRankCnt = 1, QWidget* parent = nullptr);
  void AddActions(const QList<QAction*>& acts, Qt::ToolButtonStyle tbs);

private:
  QGridLayout* mGridLo{nullptr};
  Qt::Orientation mArrangeOrientation;
  int mEachRankCnt;
};

#endif // ACTIONSCONTAINERWID_H

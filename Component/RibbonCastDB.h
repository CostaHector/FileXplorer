#ifndef RIBBONCASTDB_H
#define RIBBONCASTDB_H

#include <QToolBar>

class RibbonCastDB : public QToolBar {
public:
  explicit RibbonCastDB(const QString& title, QWidget* parent = nullptr);

  QToolBar* m_BasicTableOp{nullptr};
  QToolBar* m_SyncVidsFromDbOp{nullptr};
};


#endif // RIBBONCASTDB_H

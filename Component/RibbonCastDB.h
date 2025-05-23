#ifndef RIBBONCASTDB_H
#define RIBBONCASTDB_H

#include <QToolBar>

class RibbonCastDB : public QToolBar {
 public:
  explicit RibbonCastDB(const QString& title, QWidget* parent = nullptr);

  QToolBar* m_BasicTableOp{nullptr};
  QToolBar* m_RefreshOp{nullptr};
  QToolBar* m_FileSystemOp{nullptr};
  QToolBar* m_LoadExtentOp{nullptr};
  QToolBar* m_ExportToOp{nullptr};
};


#endif // RIBBONCASTDB_H

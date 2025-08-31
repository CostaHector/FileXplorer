#ifndef RENAMEWIDGET_LONGPATH_H
#define RENAMEWIDGET_LONGPATH_H

#include "AdvanceRenamer.h"
#include "LongPathFinder.h"

class RenameWidget_LongPath : public AdvanceRenamer {
 public:
  explicit RenameWidget_LongPath(QWidget* parent = nullptr);

  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  void InitExtraMemberWidget() override;

  void FilterNames(FileOsWalker& walker) const override;
  QStringList RenameCore(const QStringList& replaceeList) override;

 protected:
  void DropSectionChanged(const QString& newDropSectionStr);

 private:
  QLineEdit* m_dropSectionLE{nullptr};
  QLineEdit* m_maxPathLengthLE{nullptr};
  QToolBar* m_dropControlBar{nullptr};

  LongPathFinder m_lpf;
};

#endif  // RENAMEWIDGET_LONGPATH_H

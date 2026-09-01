#ifndef RENAMEWIDGET_ARRANGESECTION_H
#define RENAMEWIDGET_ARRANGESECTION_H

#include "AdvanceRenamer.h"
#include <QComboBox>
#include <QAction>
#include <QActionGroup>

class RenameWidget_ArrangeSection : public AdvanceRenamer {
  Q_OBJECT
 public:
  using AdvanceRenamer::AdvanceRenamer;
  void initExclusiveSetting() override;

  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  void InitExtraMemberWidget() override;
  QStringList RenameCore(const QStringList& replaceeList) override;

 private:
  QAction* _CHOP_POSTFIX{nullptr};
  QAction* _SWAP_SECTION_AT_2_INDEXES{nullptr};
  QAction* _SECTIONS_USED_TO_JOIN{nullptr};
  QActionGroup* _ARRANGE_SECTION_AG{nullptr};

  QComboBox* m_swap2Index{nullptr};
  QComboBox* m_sectionsUsedToJoin{nullptr};
  QCheckBox* m_recordWasted{nullptr};
};

#endif  // RENAMEWIDGET_ARRANGESECTION_H

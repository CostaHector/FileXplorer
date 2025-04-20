#ifndef RENAMEWIDGET_ARRANGESECTION_H
#define RENAMEWIDGET_ARRANGESECTION_H

#include "AdvanceRenamer.h"
#include <QAction>
#include <QActionGroup>

class ArrangeSectionActions : public QObject {
 public:
  static ArrangeSectionActions& GetInst();
  ArrangeSectionActions(QObject* parent = nullptr);
  QAction* _SWAP_SECTION_AT_2_INDEXES{new (std::nothrow) QAction{"Swap 2 sections:"}};
  QAction* _SECTION_JOIN_WITH_SELECT_INDEXES{new (std::nothrow) QAction{"Join sections with select indexes:", this}};
  QActionGroup* _ARRANGE_SECTION_AG{new (std::nothrow) QActionGroup{this}};
};

class RenameWidget_ArrangeSection : public AdvanceRenamer {
 public:
  explicit RenameWidget_ArrangeSection(QWidget* parent = nullptr);
  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  void InitExtraMemberWidget() override;
  QStringList RenameCore(const QStringList& replaceeList) override;

 private:
  QLineEdit* m_2IndexesInput{new QLineEdit};
  QComboBox* m_indexes{new QComboBox(this)};
  QComboBox* m_sectionsUsedToJoin{new QComboBox(this)};
  QCheckBox* m_strictMode{new QCheckBox{"Strict mode", this}};
};

#endif  // RENAMEWIDGET_ARRANGESECTION_H

#include "TypeFilterButton.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include "Logger.h"
#include <QMenu>
#include <QDirIterator>

TypeFilterButton::TypeFilterButton(ModelFilterE modelE, QWidget* parent)
  : QToolButton{parent},
  mFilterHelper{ModelFilterActions::GetInst(modelE)}
{
  setIcon(QIcon{":img/FILE_SYSTEM_FILTER"});;
  setText("Filter");
  setToolTip("Used in:\n"
             "1. QFileSystemModel->setFilter()\n"
             "2. AdvanceSearchModel->setFilter()");

  auto fileTypeFilterMenu = new (std::nothrow) QMenu{"Filter Menu", this};
  fileTypeFilterMenu->setToolTipsVisible(true);
  fileTypeFilterMenu->addSection("Dir::Filter");
  fileTypeFilterMenu->addActions(mFilterHelper.m_FILTER_FLAG_AGS->actions());
  fileTypeFilterMenu->addSeparator();
  fileTypeFilterMenu->addAction(mFilterHelper.RESET_FILTERS);
  fileTypeFilterMenu->addSeparator();
  fileTypeFilterMenu->addSection("When item don't pass, disable or hide this item");
  fileTypeFilterMenu->addAction(mFilterHelper.GRAY_ENTRIES_DONT_PASS_FILTER);
  if (modelE == ModelFilterE::ADVANCE_SEARCH) {
    fileTypeFilterMenu->addSeparator();
    fileTypeFilterMenu->addSection("When advance search, including subdirectory or not");
    fileTypeFilterMenu->addAction(mFilterHelper.INCLUDING_SUBDIRECTORIES);
    connect(mFilterHelper.INCLUDING_SUBDIRECTORIES,    &QAction::triggered,      this, &TypeFilterButton::EmitIncludingSubdirectoryChanged);
  }

  setMenu(fileTypeFilterMenu);
  setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);

  connect(mFilterHelper.m_FILTER_FLAG_AGS,             &QActionGroup::triggered, this, &TypeFilterButton::EmitFilterChanged);
  connect(mFilterHelper.RESET_FILTERS,                 &QAction::triggered,      this, &TypeFilterButton::EmitFilterChangedWhenResetFilters);
  connect(mFilterHelper.GRAY_ENTRIES_DONT_PASS_FILTER, &QAction::triggered,      this, &TypeFilterButton::EmitNameFilterDisablesChanged);
}

TypeFilterButton::~TypeFilterButton() {
  Configuration().setValue(mFilterHelper.GetMemoryKeyName(MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.name), (int)curDirFilters());
  Configuration().setValue(mFilterHelper.GetMemoryKeyName(MemoryKey::GRAY_ENTRIES_DONT_PASS_FILTER.name), curGrayOrHideUnpassItem());
  if (mFilterHelper.GetModelE() == ModelFilterE::ADVANCE_SEARCH){
    Configuration().setValue(mFilterHelper.GetMemoryKeyName(MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.name), curIteratorFlag() == QDirIterator::IteratorFlag::Subdirectories);
  }
}

void TypeFilterButton::EmitFilterChanged(const QAction* pAct) {
  CHECK_NULLPTR_RETURN_VOID(pAct);
  const auto newDirFilters = mFilterHelper.getCurDirFilters();
  LOG_D("ModelFilterE[%s] triggered on %s. Now DirFilters is set to %d", mFilterHelper.c_str(), qPrintable(pAct->text()), (int)newDirFilters);
  emit filterChanged(newDirFilters);
}

void TypeFilterButton::EmitFilterChangedWhenResetFilters() {
  mFilterHelper.resetFiltersToInitial();
  EmitFilterChanged(mFilterHelper.RESET_FILTERS);
}

void TypeFilterButton::EmitNameFilterDisablesChanged(bool bGrayUnpass) {
  LOG_D("ModelFilterE[%s] now bDisableOrGray unpass item bool[%d]", mFilterHelper.c_str(), bGrayUnpass);
  emit nameFilterDisablesChanged(bGrayUnpass);
}

void TypeFilterButton::EmitIncludingSubdirectoryChanged(const bool bInclude) {
  auto iteratorFlg = curIteratorFlag();;
  LOG_D("ModelFilterE[%s] including subdirectory bool:%d, iteratorFlg:%d", mFilterHelper.c_str(), bInclude, (int)iteratorFlg);
  emit includingSubdirectoryChanged(iteratorFlg);
}

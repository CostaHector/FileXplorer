#include "DuplicateImagesTable.h"
#include "RedundantImageFinderActions.h"
#include "RedunImgFinderKey.h"
#include "PathTool.h"
#include "Configuration.h"
#include "FileTool.h"
#include "UndoRedo.h"
#include "NotificatorMacro.h"

#include <QHeaderView>
#include <QFileInfo>

using namespace DuplicateImageDetectionCriteria;

DuplicateImagesTable::DuplicateImagesTable(QWidget* parent)
  : CustomTableView{"RedundantImageTable", parent} {
  m_imgModel = new (std::nothrow) DuplicateImagesModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_imgModel);

  m_imgProxy = new (std::nothrow) QSortFilterProxyModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_imgProxy);
  m_imgProxy->setSourceModel(m_imgModel);

  setModel(m_imgProxy);
  registerProxyModel(m_imgProxy);

  mSelectSameHashRows = new QAction{tr("Select Same Hash Rows(Ignore first one)"), this};

  QList<QAction*> exclusiveActs{mSelectSameHashRows};
  PushFrontExclusiveActions(exclusiveActs);

  if (QHeaderView* horHeader = verticalHeader()) {
    horHeader->setDefaultSectionSize(DuplicateImagesModel::SMALL_PIXMAP_WIDTH);
    horHeader->setSectionResizeMode(QHeaderView::Fixed);
  }

  subscribe();
}

void DuplicateImagesTable::subscribe() {
  auto& inst = g_redunImgFinderAg();
  connect(&inst, &RedundantImageFinderActions::disableImageDecorationChanged, m_imgModel, &DuplicateImagesModel::onDisableImageDecorationChanged);

  connect(&inst, &RedundantImageFinderActions::findByChanged, this, &DuplicateImagesTable::onFindByChanged);
  connect(inst.INCLUDING_EMPTY_IMAGES, &QAction::toggled, this, &DuplicateImagesTable::onIncludeEmptyImgChanged);
  connect(inst.RECYLE_NOW, &QAction::triggered, this, &DuplicateImagesTable::RecycleSelection);

  connect(this, &QAbstractItemView::doubleClicked, this, &DuplicateImagesTable::onOpenImageDoubleClicked);

  connect(mSelectSameHashRows, &QAction::triggered, this, &DuplicateImagesTable::SelectRowsToDelete);
}

QString DuplicateImagesTable::GetWinTitle() const {
  constexpr const char* TITLE_TEMPLATE{"Redundant Images Finder | Path: %1 | %2 item(s)"};
  return QString(TITLE_TEMPLATE)   //
      .arg(m_imgModel->rootPath()) //
      .arg(m_imgModel->rowCount());
}

bool DuplicateImagesTable::operator()(const QString& folderPath) {
  static const auto& inst = g_redunImgFinderAg();
  const auto findBy = inst.GetCurFindDupBy();
  const bool bIncludeEmptyImg = inst.isIncludeEmptyImage();
  if (m_imgModel->setRootPath(folderPath, findBy, bIncludeEmptyImg) < 0) {
    return false;
  }
  emit windowTitleChanged(GetWinTitle());
  return true;
}

bool DuplicateImagesTable::onFindByChanged(DICriteriaE findBy) {
  if (m_imgModel->setFindBy(findBy) < 0) {
    return false;
  }
  emit windowTitleChanged(GetWinTitle());
  return true;
}

bool DuplicateImagesTable::onIncludeEmptyImgChanged(bool bInclude) {
  if (m_imgModel->setIncludeEmptyImg(bInclude) < 0) {
    return false;
  }
  emit windowTitleChanged(GetWinTitle());
  return true;
}

bool DuplicateImagesTable::onOpenImageDoubleClicked(const QModelIndex& proxyClickedIndex) const {
  if (!proxyClickedIndex.isValid()) {
    return false;
  }
  const QModelIndex srcClickedInd = m_imgProxy->mapToSource(proxyClickedIndex);
  const QString imgPath{m_imgModel->filePath(srcClickedInd)};
  return FileTool::OpenLocalImageFile(imgPath);
}

int DuplicateImagesTable::RecycleSelection() {
  const QModelIndexList& srcIndexes = selectedRowsSource();
  const int SELECTED_CNT = srcIndexes.size();
  if (SELECTED_CNT <= 0) {
    LOG_INFO_NP("Skip recycle", "No row selected");
    return 0;
  }

  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE recycleCmds;
  recycleCmds.reserve(SELECTED_CNT);
  for (const auto& srcInd : srcIndexes) {
    recycleCmds.append(ACMD::GetInstMOVETOTRASH("", m_imgModel->filePath(srcInd)));
  }
  bool isRenameAllSucceed = UndoRedo::GetInst().Do(recycleCmds);
  LOG_OE_P(isRenameAllSucceed, "Recyle redundant images", "selected count: %d", SELECTED_CNT);

  m_imgModel->UpdateDisplayWhenRecycled(srcIndexes);

  return SELECTED_CNT;
}

bool DuplicateImagesTable::SelectRowsToDelete() {
  QItemSelection srcSelection = m_imgModel->GetSameHashRowWithFirstOneIgnored();
  if (srcSelection.isEmpty()) {
    LOG_INFO_NP("Skip selection", "No rows need select for delete");
    return false;
  }
  QItemSelection proSelection = m_imgProxy->mapSelectionFromSource(srcSelection);
  selectionModel()->select(proSelection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  return true;
}

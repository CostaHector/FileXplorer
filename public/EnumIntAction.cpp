#include "EnumIntAction.h"
#include <QListView>
#include "PreviewTypeTool.h"
#include "ViewTypeTool.h"
#include "StyleEnum.h"
#include "SearchTools.h"
#include "BytesRangeTool.h"
#include "DuplicateImagesHelper.h"
#include "FileStructurePolicy.h"
#include "ScenePageNaviHelper.h"
#include <QHeaderView>
#include <QCryptographicHash>

template struct EnumIntAction<PreviewTypeTool::PREVIEW_TYPE_E>;
template struct EnumIntAction<ViewTypeTool::ViewType>;
template struct EnumIntAction<Style::StyleE>;
template struct EnumIntAction<Style::StyleSheetE>;
template struct EnumIntAction<SearchTools::SearchModeE>;
template struct EnumIntAction<BytesRangeTool::BytesRangeE>;
template struct EnumIntAction<QCryptographicHash::Algorithm>;
template struct EnumIntAction<DuplicateImageDetectionCriteria::DICriteriaE>;
template struct EnumIntAction<FileStructurePolicy::FileStuctureModeE>;
template struct EnumIntAction<ScenePageNaviHelper::PageNaviE>;
template struct EnumIntAction<SceneSortOrderHelper::SortDimE>;
template struct EnumIntAction<Qt::TextElideMode>;
template struct EnumIntAction<QHeaderView::ResizeMode>;
template struct EnumIntAction<Qt::ScrollBarPolicy>;

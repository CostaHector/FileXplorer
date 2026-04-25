#include "EnumIntAction.h"
#include <QListView>
#include "PreviewTypeTool.h"
#include "ViewTypeTool.h"
#include "StyleEnum.h"
#include "SearchTools.h"
#include "BytesRangeTool.h"
#include "DuplicateImagesHelper.h"
#include "FileStructurePolicy.h"
#include "VideoPlayTool.h"
#include "FavoriteItemData.h"
#include "SceneInfo.h"
#include "ScenePageNaviHelper.h"
#include <QHeaderView>
#include <QCryptographicHash>
#include <QMediaPlaylist>

template struct EnumIntAction<PreviewTypeTool::PREVIEW_TYPE_E>;
template struct EnumIntAction<ViewTypeTool::ViewType>;
template struct EnumIntAction<Style::StylePresetE>;
template struct EnumIntAction<Style::StyleThemeE>;
template struct EnumIntAction<SearchTools::SearchModeE>;
template struct EnumIntAction<BytesRangeTool::BytesRangeE>;
template struct EnumIntAction<QCryptographicHash::Algorithm>;
template struct EnumIntAction<DuplicateImageDetectionCriteria::DICriteriaE>;
template struct EnumIntAction<FileStructurePolicy::FileStuctureModeE>;
template struct EnumIntAction<ScenePageNaviHelper::PageNaviE>;
template struct EnumIntAction<Qt::TextElideMode>;
template struct EnumIntAction<QHeaderView::ResizeMode>;
template struct EnumIntAction<Qt::ScrollBarPolicy>;
template struct EnumIntAction<QMediaPlaylist::PlaybackMode>;
template struct EnumIntAction<VideoPlayTool::PlaybackTriggerMode>;
template struct EnumIntAction<FavoriteItemData::Role>;
template struct EnumIntAction<SceneInfo::Role>;

#include "EnumIntAction.h"
#include <QListView>
#include "PreviewTypeTool.h"
#include "ViewTypeTool.h"
#include "StyleEnum.h"
#include "SearchTools.h"
#include "BytesRangeTool.h"
#include "RedundantImageTool.h"
#include <QCryptographicHash>

template struct EnumIntAction<QListView::Flow>;
template struct EnumIntAction<PreviewTypeTool::PREVIEW_TYPE_E>;
template struct EnumIntAction<ViewTypeTool::ViewType>;
template struct EnumIntAction<Style::StyleE>;
template struct EnumIntAction<Style::StyleSheetE>;
template struct EnumIntAction<SearchTools::SearchModeE>;
template struct EnumIntAction<BytesRangeTool::BytesRangeE>;
template struct EnumIntAction<QCryptographicHash::Algorithm>;
template struct EnumIntAction<RedundantImageTool::DecideByE>;

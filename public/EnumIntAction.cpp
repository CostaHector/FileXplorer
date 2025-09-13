#include "EnumIntAction.h"
#include <QListView>
#include "PreviewTypeTool.h"
#include "ViewTypeTool.h"
#include "StyleEnum.h"
#include "SearchTools.h"

template struct EnumIntAction<QListView::Flow>;
template struct EnumIntAction<PreviewTypeTool::PREVIEW_TYPE_E>;
template struct EnumIntAction<ViewTypeTool::ViewType>;
template struct EnumIntAction<Style::StyleE>;
template struct EnumIntAction<Style::StyleSheetE>;
template struct EnumIntAction<SearchTools::SearchModeE>;

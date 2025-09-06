#include "EnumIntAction.h"
#include <QListView>
#include "PreviewTypeTool.h"
#include "ViewTypeTool.h"

template struct EnumIntAction<QListView::Flow>;
template struct EnumIntAction<PreviewTypeTool::PREVIEW_TYPE_E>;
template struct EnumIntAction<ViewTypeTool::ViewType>;

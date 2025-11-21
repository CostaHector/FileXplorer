#include "ItemView.h"
#include "PublicMacro.h"
#include "PublicTool.h"
#include "NotificatorMacro.h"
#include "ThumbnailImageViewer.h"

#include <QActionGroup>
#include <QFile>
#include <QFileInfo>

ItemView::ItemView(const QString& itemViewName, QWidget* parent)  //
    : CustomListView{itemViewName, parent} {
  setMovement(QListView::Movement::Free);

  _PLAY_ITEM = new (std::nothrow) QAction{"Play", this};
  m_menu->addAction(_PLAY_ITEM);
  AddItselfAction2Menu();

  subscribe();
}

void ItemView::subscribe() {
  connect(this, &QListView::doubleClicked, this, &ItemView::onCellDoubleClicked);
  connect(_PLAY_ITEM, &QAction::triggered, this, [this]() { onCellDoubleClicked(currentIndex()); });
}

bool ItemView::onCellDoubleClicked(const QModelIndex& clickedIndex) const {
  CHECK_NULLPTR_RETURN_FALSE(mModels);
  if (!clickedIndex.isValid()) {
    return false;
  }
  const QString& path = mModels->filePath(clickedIndex);
  if (!QFile::exists(path)) {
    return false;
  }
#ifdef RUNNING_UNIT_TESTS
  return true;
#endif
  return FileTool::OpenLocalFile(path);
}

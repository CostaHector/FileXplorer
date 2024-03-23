#include "JsonListView.h"
#include <QDesktopServices>
#include <QInputDialog>

#include "Actions/JsonEditorActions.h"
#include "View/ViewHelper.h"
#include "View/ViewStyleSheet.h"

JsonListView::JsonListView(JsonModel* model_, QWidget* parent)
    : QListView{parent}, m_jsonModel{model_}, m_jsonMenu{g_jsonEditorActions().GetJsonToBeEdittedListMenu(this)} {
  setModel(m_jsonModel);
  InitViewSettings();

  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers
  setDragDropMode(QAbstractItemView::NoDragDrop);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);

  subscribe();

  setStyleSheet(ViewStyleSheet::LISTVIEW_STYLESHEET);
}

void JsonListView::subscribe() {
  connect(g_jsonEditorActions()._CLR_TO_BE_EDITED_LIST, &QAction::triggered, m_jsonModel, &JsonModel::clear);

  connect(this, &QListView::doubleClicked, this, [this](const QModelIndex& ind) {
    const QString& jsonPath = m_jsonModel->filePath(currentIndex());
    const auto& jsonUrl = QUrl::fromLocalFile(jsonPath);
    QDesktopServices::openUrl(jsonUrl);
  });

  connect(g_jsonEditorActions()._REVEAL_IN_EXPLORER, &QAction::triggered, this, [this]() {
    const QString& jsonPath = m_jsonModel->filePath(currentIndex());
    const QString& containsPath = QFileInfo(jsonPath).absolutePath();
    const auto& containsUrl = QUrl::fromLocalFile(containsPath);
    QDesktopServices::openUrl(containsUrl);
  });

  connect(g_jsonEditorActions()._REVEAL_IN_EXPLORER, &QAction::triggered, this, [this]() {
    const QString& jsonPath = m_jsonModel->filePath(currentIndex());
    const QString& containsPath = QFileInfo(jsonPath).absolutePath();
    const auto& containsUrl = QUrl::fromLocalFile(containsPath);
    QDesktopServices::openUrl(containsUrl);
  });
}

void JsonListView::contextMenuEvent(QContextMenuEvent* event) {
  m_jsonMenu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
  QListView::contextMenuEvent(event);
}

void JsonListView::onSetPerfCount(const bool checked) {
  const int storedSkipCnt = PreferenceSettings()
                                .value(MemoryKey::COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT.name, MemoryKey::COMPLETE_JSON_FILE_MIN_PERFORMERS_COUNT.v)
                                .toInt();
  bool okClicked = false;
  const int skipCount = QInputDialog::getInt(this, "Auto Skip", "when perforers count >= ?", storedSkipCnt, 0, 5, 1, &okClicked);
  if (not okClicked) {
    qInfo("[User Cancel] Set skip perfs count");
    return;
  }
  if (storedSkipCnt == skipCount) {
    qInfo("skip change remains[%d]", storedSkipCnt);
    return;
  }
  m_jsonModel->SetCompletePerfCount(skipCount);
}

void JsonListView::autoNext() {
  clearSelection();
  for (auto curRow = currentRow() + 1; curRow < count(); ++curRow) {
    if (not m_jsonModel->isPerfComplete(curRow)) {
      setCurrentRow(curRow);
      return;
    }
  }
  if (count() > 0) {  // stop at last line
    setCurrentRow(count() - 1);
  }
}

auto JsonListView::InitViewSettings() -> void {
  setAlternatingRowColors(true);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  this->sizeHintForRow(ViewStyleSheet::ROW_SECTION_HEIGHT);
  View::UpdateItemViewFontSizeCore(this);
}

bool JsonListView::hasLast() const {
  const auto curRow = currentRow();
  return curRow - 1 >= 0;
}

void JsonListView::last() {
  const auto curRow = currentRow() - 1;
  clearSelection();
  setCurrentRow(curRow);
  qDebug("last curRow %d", curRow);
}

bool JsonListView::hasNext() const {
  const auto curRow = currentRow();
  return curRow + 1 < count();
}

void JsonListView::next() {
  const auto curRow = currentRow() + 1;
  clearSelection();
  setCurrentRow(curRow);
  qDebug("next curRow %d", curRow);
}

void JsonListView::onNext() {
  if (g_jsonEditorActions()._AUTO_SKIP->isChecked()) {
    autoNext();
    return;
  }
  if (hasNext()) {
    next();
  }
}

void JsonListView::onLast() {
  if (hasLast()) {
    last();
  }
}

int JsonListView::load(const QString& path) {
  if (not QDir(path).exists()) {
    return 0;
  }
  const int beforeJsonFileCnt = count();
  m_jsonModel->setRootPath(path);
  const int afterJsonFileCnt = count();
  const int deltaFile = afterJsonFileCnt - beforeJsonFileCnt;
  if (deltaFile != 0) {
    setCurrentRow(beforeJsonFileCnt);
  }
  return deltaFile;
}

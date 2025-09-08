#include "JsonTableView.h"
#include "JsonActions.h"
#include "NotificatorMacro.h"
#include "StudiosManager.h"
#include "NameTool.h"
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextCursor>
#include <QPlainTextEdit>
#include <QStyleOptionViewItem>

JsonTableView::JsonTableView(JsonTableModel* jsonModel, JsonProxyModel* jsonProxyModel, QWidget* parent)  //
  : CustomTableView{"JSON_TABLE_VIEW", parent}                                                          //
{
  CHECK_NULLPTR_RETURN_VOID(jsonModel);
  _JsonModel = jsonModel;
  CHECK_NULLPTR_RETURN_VOID(jsonProxyModel);
  _JsonProxyModel = jsonProxyModel;

  _JsonProxyModel->setSourceModel(_JsonModel);
  _JsonProxyModel->setFilterKeyColumn(JSON_KEY_E::Name);

  setModel(_JsonProxyModel);
  setEditTriggers(QAbstractItemView::EditTrigger::EditKeyPressed | QAbstractItemView::EditTrigger::AnyKeyPressed);

  m_DetailEdit = new (std::nothrow) MultiLineEditDelegate;
  CHECK_NULLPTR_RETURN_VOID(jsonModel);
  setItemDelegateForColumn(JSON_KEY_E::Detail, m_DetailEdit);

  InitTableView();
  setWindowTitle("Json Table View");
  setWindowIcon(QIcon(":img/JSON_EDITOR"));

  subscribe();
}

QModelIndex JsonTableView::CurrentIndexSource() const {
  const QModelIndex& proIndex = CustomTableView::currentIndex();
  return _JsonProxyModel->mapToSource(proIndex);
}

QModelIndexList JsonTableView::selectedRowsSource(JSON_KEY_E column) const {
  const QModelIndexList& proIndexes = CustomTableView::selectionModel()->selectedRows(column);
  QModelIndexList srcIndexes;
  srcIndexes.reserve(proIndexes.size());
  for (const auto& proIndex : proIndexes) {
    srcIndexes.append(_JsonProxyModel->mapToSource(proIndex));
  }
  return srcIndexes;
}

int JsonTableView::ReadADirectory(const QString& path) {
  return _JsonModel->setRootPath(path);
}

int JsonTableView::onSaveCurrentChanges() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("[skip] nothing selected", "skip sync name field");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Name);
  const int cnt = _JsonModel->SaveCurrentChanges(indexes);
  if (cnt < 0) {
    LOG_BAD_P("Save failed", "errorCode:%d", cnt);
    return cnt;
  }
  LOG_GOOD_P("Changes have been saved", "%d/%d row(s)", cnt, indexes.size());
  return indexes.size();
}

int JsonTableView::onSyncNameField() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("[skip] nothing selected", "skip sync name field");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Name);
  const int cnt = _JsonModel->SyncFieldNameByJsonBaseName(indexes);

  LOG_GOOD_P("Name field has been sync by json basename", "%d/%d row(s)", cnt, indexes.size());
  return indexes.size();
}

int JsonTableView::onExportCastStudioToDictonary() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("[skip] nothing selected", "skip sync name field");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Name);
  int castCnt{0}, studioCnt{0};
  std::tie(castCnt, studioCnt) = _JsonModel->ExportCastStudioToLocalDictionaryFile(indexes);
  if (castCnt < 0 || studioCnt < 0) {
    LOG_BAD_NP("Export cast/studio to local dictionary file failed", "see details in log");
    return -1;
  }
  const QString affectedRowsMsg = QString{"Increment cast:%1/studio:%2\nby %3 selected row(s)"}.arg(castCnt).arg(studioCnt).arg(indexes.size());
  LOG_GOOD_NP("Export succeed", affectedRowsMsg);
  QMessageBox::information(this, "Export succeed", affectedRowsMsg);
  return castCnt + studioCnt;
}

int JsonTableView::onRenameJsonAndRelated() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("nothing selected", "skip sync name field");
    return 0;
  }
  const QModelIndex& ind = CurrentIndexSource();
  const QString oldJsonBaseName = _JsonModel->fileBaseName(ind);
  bool isInputOk{false};
  const QString& newJsonBaseName = QInputDialog::getItem(this, "Input an new json base name", oldJsonBaseName,  //
                                                         {oldJsonBaseName}, 0, true, &isInputOk);
  if (!isInputOk) {
    LOG_GOOD_NP("[skip] User cancel rename json and related files", "return");
    return 0;
  }
  if (newJsonBaseName.isEmpty()) {
    LOG_BAD_NP("[skip] New json base name can not be empty", "return");
    return 0;
  }
  int cnt = _JsonModel->RenameJsonAndItsRelated(ind, newJsonBaseName);
  const QString msg{QString{"Rename Json\n[%1]\n[%2]\n and it's related file(s). retCode: %3"}.arg(oldJsonBaseName).arg(newJsonBaseName).arg(cnt)};
  if (cnt < JsonPr::E_OK) {
    LOG_BAD_P(msg, "Failed, errorCode:%d", cnt);
    return cnt;
  }
  LOG_GOOD_NP(msg, "All succeed");
  return 0;
}

int JsonTableView::onSetStudio() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("nothing selected", "skip");
    return 0;
  }
  const auto& curInd = CurrentIndexSource();
  const QString& fileBaseName = _JsonModel->fileBaseName(curInd);

  static StudiosManager& sm = StudiosManager::getIns();
  const QString defStudio = sm(fileBaseName);
  int defIndex = m_studioCandidates.indexOf(defStudio);
  if (defIndex == -1) {
    m_studioCandidates.push_back(defStudio);
    defIndex = m_studioCandidates.size() - 1;
  }

  bool isInputOk{false};
  QString hintMsg{"Choose or select studio from drop down list"};
  const QString& studio = QInputDialog::getItem(this, "Input an studio name", hintMsg,  //
                                                m_studioCandidates,                     //
                                                defIndex,                               //
                                                true, &isInputOk);
  if (!isInputOk) {
    LOG_GOOD_NP("[skip] User cancel set studio", "return");
    return 0;
  }
  if (studio.isEmpty()) {
    LOG_BAD_NP("[skip] Studio name can not be empty", "return");
    return 0;
  }

  if (m_studioCandidates.indexOf(studio) == -1) {
    auto insertPos = std::upper_bound(m_studioCandidates.begin(), m_studioCandidates.end(), studio);
    m_studioCandidates.insert(insertPos, studio);
  }

  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Studio);
  const int cnt = _JsonModel->SetStudio(indexes, studio);

  LOG_GOOD_P("studio has been changed", "%d/%d row(s) to %s", cnt, indexes.size(), qPrintable(studio));
  return indexes.size();
}

int JsonTableView::onInitCastAndStudio() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("nothing selected", "skip init cast/studio studio");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Studio);
  const int cnt = _JsonModel->InitCastAndStudio(indexes);

  LOG_GOOD_P("cast/studio has been inited", "%d/%d row(s)", cnt, indexes.size());
  return indexes.size();
}

int JsonTableView::onHintCastAndStudio() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("nothing selected", "skip hint cast/studio");
    return 0;
  }

  QString userSelection;
  EDITOR_WIDGET_TYPE edtWidType{EDITOR_WIDGET_TYPE::BUTT};
  if (!GetSelectedTextInCell(userSelection, edtWidType)) {
    LOG_WARN_NP("Get Selected Text in Selected Cell failed", "see detail in logs");
    return -1;
  }

  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Studio);
  if (indexes.size() > 1 && !userSelection.isEmpty()) {
    LOG_WARN_NP("Dangerous! User select more than 1 line and text selected", "skip");
    return -1;
  }

  const int cnt = _JsonModel->HintCastAndStudio(indexes, userSelection);

  LOG_GOOD_P("cast/studio has been hint", "%d/%d row(s)", cnt, indexes.size());
  return indexes.size();
}

int JsonTableView::onFormatCast() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("[Skip]nothing selected", "skip cast format");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Cast);
  const int cnt = _JsonModel->FormatCast(indexes);

  LOG_GOOD_P("Cast has been format", "%d/%d row(s)", cnt, indexes.size());
  return indexes.size();
}

int JsonTableView::onClearStudio() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("[Skip]nothing selected", "skip clear studio");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Studio);
  const int cnt = _JsonModel->SetStudio(indexes, "");
  LOG_GOOD_P("studio has been cleared", "%d/%d row(s)", cnt, indexes.size());
  return indexes.size();
}

int JsonTableView::onSetCastOrTags(const FIELD_OP_TYPE type, const FIELD_OP_MODE mode) {
  const QString fieldOperation{"Operation:" + FIELF_OP_TYPE_ARR[(int)type] + ' ' + FIELD_OP_MODE_ARR[(int)mode]};
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("nothing selected. skip", fieldOperation);
    return 0;
  }

  QString tagsOrCast;
  if (mode == FIELD_OP_MODE::CLEAR) {
    tagsOrCast = "";
  } else {
    QStringList& candidates = m_candidatesLst[(int)type];
    bool isInputOk{false};
    const QString hintMsg{QString{"Choose or select from drop down list[%1]"}.arg(fieldOperation)};
    tagsOrCast = QInputDialog::getItem(this, fieldOperation, hintMsg,  //
                                       candidates,                     //
                                       candidates.size() - 1,          //
                                       true, &isInputOk);
    if (!isInputOk) {
      LOG_GOOD_NP("[Skip] User cancel", fieldOperation);
      return 0;
    }
    if (tagsOrCast.isEmpty()) {
      LOG_BAD_NP("[Abort] Input can not be empty", fieldOperation);
      return 0;
    }
    candidates.push_back(tagsOrCast);
  }

  JSON_KEY_E fieldColumn{JSON_KEY_E::JSON_KEY_BUTT};
  switch (type) {
    case FIELD_OP_TYPE::CAST:
      fieldColumn = JSON_KEY_E::Cast;
      break;
    case FIELD_OP_TYPE::TAGS:
      fieldColumn = JSON_KEY_E::Tags;
      break;
    default:
      LOG_BAD_P("Field Type invalid", "field: %d", (int)type);
      return -1;
  }

  int cnt{0};
  const QModelIndexList& indexes = selectedRowsSource(fieldColumn);
  switch (mode) {
    case FIELD_OP_MODE::SET:
    case FIELD_OP_MODE::CLEAR:
      cnt = _JsonModel->SetCastOrTags(indexes, fieldColumn, tagsOrCast);
      break;
    case FIELD_OP_MODE::APPEND:
      cnt = _JsonModel->AddCastOrTags(indexes, fieldColumn, tagsOrCast);
      break;
    case FIELD_OP_MODE::REMOVE:
      cnt = _JsonModel->RmvCastOrTags(indexes, fieldColumn, tagsOrCast);
      break;
    default:
      LOG_BAD_P("Field Operation invalid", "mode: %d", (int)mode);
      return -2;
  }
  LOG_GOOD_P("SetCastOrTags", "%d/%d row(s) affected by [%s]", cnt, indexes.size(), qPrintable(fieldOperation));
  return indexes.size();
}

// current index invalid => false
// no selection => false
// no edit mode => true
bool JsonTableView::GetSelectedTextInCell(QString& selectedText, EDITOR_WIDGET_TYPE& edtWidType) const {
  selectedText.clear();
  edtWidType = EDITOR_WIDGET_TYPE::BUTT;
  const QModelIndex& curInd = currentIndex();
  if (!curInd.isValid()) {
    LOG_WARN_NP("Current Index is invalid", "select a line first");
    return false;
  }
  QWidget* editor = indexWidget(curInd);
  if (editor == nullptr) {  // not in edit mode
    LOG_D("Cell not in edit, cannot get selection text in cell");
    return true;
  }
  if (auto lineEdit = qobject_cast<QLineEdit*>(editor)) {
    selectedText = lineEdit->selectedText();
    edtWidType = EDITOR_WIDGET_TYPE::LINE_EDIT;
  } else if (auto textEdit = qobject_cast<QPlainTextEdit*>(editor)) {
    // Unicode U+2029 used for segment splitter
    selectedText = textEdit->textCursor().selectedText().replace(QChar(0x2029), '\n');
    edtWidType = EDITOR_WIDGET_TYPE::PLAIN_TEXT_EDIT;
  } else if (auto richTextEdit = qobject_cast<QTextEdit*>(editor)) {
    selectedText = richTextEdit->textCursor().selectedText();
    edtWidType = EDITOR_WIDGET_TYPE::TEXT_EDIT;
  } else {
    selectedText.clear();
    edtWidType = EDITOR_WIDGET_TYPE::BUTT;
    LOG_WARN_NP("Unsupported editor type, current not QLineEdit/QPlainTextEdit/QTextEdit", "failed");
    return false;
  }
  return true;
}

int JsonTableView::onAppendFromSelection(bool isUpperCaseSentence) {
  EDITOR_WIDGET_TYPE edtWidType{EDITOR_WIDGET_TYPE::BUTT};
  QString userSelection;
  if (!GetSelectedTextInCell(userSelection, edtWidType)) {
    LOG_WARN_NP("Get Selected Text in Selected Cell failed", "see detail in logs");
    return -1;
  }

  if (userSelection.trimmed().isEmpty()) {
    LOG_WARN_NP("User selection text empty", "failed");
    return false;
  }

  const QModelIndex& curInd = currentIndex();
  if (!curInd.isValid()) {
    LOG_WARN_NP("Current Index is invalid", "select a line first");
    return -1;
  }

  const QModelIndex& srcModelInd = _JsonProxyModel->mapToSource(curInd);
  int cnt = _JsonModel->AppendCastFromSentence(srcModelInd, userSelection, isUpperCaseSentence);
  if (cnt < 0) {
    LOG_BAD_NP("Cast append failed", "see detail in logs");
    return -1;
  }
  LOG_GOOD_P("Cast append succeed", "cnt %d", cnt);
  return cnt;
}

int JsonTableView::onSelectionCaseOperation(bool isTitle) {
  const QModelIndex& curInd = currentIndex();

  QString userSelection;
  EDITOR_WIDGET_TYPE edtWidType{EDITOR_WIDGET_TYPE::BUTT};
  if (!GetSelectedTextInCell(userSelection, edtWidType)) {
    LOG_WARN_NP("Index valid or no selection", "No need change case");
    return -1;
  }
  if (userSelection.trimmed().isEmpty()) {
    LOG_WARN_NP("User selection empty", "No need change case");
    return -1;
  }

  QWidget* editor = indexWidget(curInd);
  if (editor == nullptr) {
    LOG_WARN_NP("Cell not in edit", "Cannot change case of selection text");
    return -1;
  }

  QString newText;
  auto* pCaseFunc{isTitle ? NameTool::CapitaliseFirstLetterKeepOther : NameTool::Lower};
  bool ret{false};
  switch (edtWidType) {
    case EDITOR_WIDGET_TYPE::LINE_EDIT: {
      auto* lineEdit = qobject_cast<QLineEdit*>(editor);
      ret = NameTool::ReplaceAndUpdateSelection(*lineEdit, pCaseFunc);
      newText = lineEdit->text();
      break;
    }
    case EDITOR_WIDGET_TYPE::PLAIN_TEXT_EDIT:{
      auto* plainTextEdit = qobject_cast<QPlainTextEdit*>(editor);
      ret = NameTool::ReplaceAndUpdateSelection(*plainTextEdit, pCaseFunc);
      newText = plainTextEdit->toPlainText();
      break;
    }
    case EDITOR_WIDGET_TYPE::TEXT_EDIT:{
      auto* textEdit = qobject_cast<QTextEdit*>(editor);
      ret = NameTool::ReplaceAndUpdateSelection(*textEdit, pCaseFunc);
      newText = textEdit->toPlainText();
      break;
    }
    case EDITOR_WIDGET_TYPE::BUTT: {
      LOG_WARN_NP("Editor type invalid", "Cannot change case of selection text");
      break;
    }
  }
  const QModelIndex& srcModelInd = _JsonProxyModel->mapToSource(curInd);
  _JsonModel->setData(srcModelInd, newText);
  if (!ret) {
    LOG_BAD_NP("Change selection case failed", "see detail in logs");
    return -1;
  }
  LOG_GOOD_NP("Change selection text case succeed", userSelection);
  return 0;
}

void JsonTableView::subscribe() {
  auto& inst = g_JsonActions();

  connect(inst._SAVE_CURRENT_CHANGES, &QAction::triggered, this, &JsonTableView::onSaveCurrentChanges);

  connect(inst._SYNC_NAME_FIELD_BY_FILENAME, &QAction::triggered, this, &JsonTableView::onSyncNameField);
  connect(inst._RELOAD_JSON_FROM_FROM_DISK, &QAction::triggered, _JsonModel, &JsonTableModel::forceReloadPath);
  connect(inst._EXPORT_CAST_STUDIO_TO_DICTION, &QAction::triggered, this, &JsonTableView::onExportCastStudioToDictonary);

  connect(inst._CAPITALIZE_FIRST_LETTER_OF_EACH_WORD, &QAction::triggered, this, [this]() { onSelectionCaseOperation(true); });
  connect(inst._LOWER_ALL_WORDS, &QAction::triggered, this, [this]() { onSelectionCaseOperation(false); });

  connect(inst._RENAME_JSON_AND_RELATED_FILES, &QAction::triggered, this, &JsonTableView::onRenameJsonAndRelated);

  connect(inst._AI_HINT_CAST_STUDIO, &QAction::triggered, this, &JsonTableView::onHintCastAndStudio);
  connect(inst._FORMATTER, &QAction::triggered, this, &JsonTableView::onFormatCast);

  connect(inst._INIT_STUDIO_CAST, &QAction::triggered, this, &JsonTableView::onInitCastAndStudio);
  connect(inst._STUDIO_FIELD_SET, &QAction::triggered, this, &JsonTableView::onSetStudio);

  connect(inst._CLEAR_STUDIO, &QAction::triggered, this, &JsonTableView::onClearStudio);
  connect(inst._CLEAR_CAST, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::CLEAR); });
  connect(inst._CLEAR_TAGS, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::CLEAR); });

  connect(inst._CAST_FIELD_SET, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET); });
  connect(inst._CAST_FIELD_APPEND, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND); });
  connect(inst._CAST_FIELD_RMV, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::REMOVE); });

  connect(inst._TAGS_FIELD_SET, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::SET); });
  connect(inst._TAGS_FIELD_APPEND, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::APPEND); });
  connect(inst._TAGS_FIELD_RMV, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::REMOVE); });

  connect(inst._ADD_SELECTED_CAST_SENTENCE, &QAction::triggered, this, [this]() { onAppendFromSelection(false); });
  connect(inst._EXTRACT_UPPERCASE_CAST, &QAction::triggered, this, [this]() { onAppendFromSelection(true); });
}

// #define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE

#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  JsonTableView jtv;
  jtv.show();
  jtv.ReadADirectory("E:/MenPreview");
  a.exec();
  return 0;
}
#endif

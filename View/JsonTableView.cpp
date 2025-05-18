#include "JsonTableView.h"
#include "Actions/JsonActions.h"
#include "Component/Notificator.h"
#include "Tools/StudiosManager.h"
#include "Tools/NameTool.h"
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>

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
    LOG_INFO("nothing selected", "skip sync name field");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Name);
  const int cnt = _JsonModel->SaveCurrentChanges(indexes);
  if (cnt < 0) {
    LOG_BAD(QString("Save failed, errorCode:%1").arg(cnt), "See detail in logs");
    return cnt;
  }
  const QString affectedRowsMsg{QString{"[Uncommit] %1/%2 row(s) of changes have been saved"}.arg(cnt).arg(indexes.size())};
  LOG_GOOD(affectedRowsMsg, "ok");
  return indexes.size();
}

int JsonTableView::onSyncNameField() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO("nothing selected", "skip sync name field");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Name);
  const int cnt = _JsonModel->SyncFieldNameByJsonBaseName(indexes);
  const QString affectedRowsMsg{QString{"[Uncommit] %1/%2 row(s) name field has been sync by json basename"}.arg(cnt).arg(indexes.size())};
  LOG_GOOD(affectedRowsMsg, "ok");
  return indexes.size();
}

int JsonTableView::onExportCastStudioToDictonary() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO("nothing selected", "skip sync name field");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Name);
  int castCnt{0}, studioCnt{0};
  std::tie(castCnt, studioCnt) = _JsonModel->ExportCastStudioToLocalDictionaryFile(indexes);
  if (castCnt < 0 || studioCnt < 0) {
    LOG_BAD("Export cast/studio to local dictionary file failed", "see details in log");
    return -1;
  }
  const QString affectedRowsMsg = QString{"Increment cast:%1/studio:%2\nby %3 selected row(s)"}.arg(castCnt).arg(studioCnt).arg(indexes.size());
  LOG_GOOD(affectedRowsMsg, "ok");
  QMessageBox::information(this, "Export succeed", affectedRowsMsg);
  return castCnt + studioCnt;
}

int JsonTableView::onRenameJsonAndRelated() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO("nothing selected", "skip sync name field");
    return 0;
  }
  const QModelIndex& ind = CurrentIndexSource();
  const QString oldJsonBaseName = _JsonModel->fileBaseName(ind);
  bool isInputOk{false};
  const QString& newJsonBaseName = QInputDialog::getItem(this, "Input an new json base name", oldJsonBaseName,  //
                                                         {oldJsonBaseName}, 0, true, &isInputOk);
  if (!isInputOk) {
    LOG_GOOD("User cancel rename json and related files", "skip")
    return 0;
  }
  if (newJsonBaseName.isEmpty()) {
    LOG_BAD("New json base name can not be empty", "skip")
    return 0;
  }
  int cnt = _JsonModel->RenameJsonAndItsRelated(ind, newJsonBaseName);
  const QString msg{QString{"Rename Json\n[%1]\n[%2]\n and it's related file(s). retCode: %3"}.arg(oldJsonBaseName).arg(newJsonBaseName).arg(cnt)};
  if (cnt < JsonPr::E_OK) {
    LOG_BAD(msg, "Failed, see in detail in logs");
    return cnt;
  }
  LOG_GOOD(msg, "All succeed");
  return 0;
}

int JsonTableView::onSetStudio() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO("nothing selected", "skip");
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
    LOG_GOOD("User cancel set studio", "skip")
    return 0;
  }
  if (studio.isEmpty()) {
    LOG_BAD("Studio name can not be empty", "skip")
    return 0;
  }

  if (m_studioCandidates.indexOf(studio) == -1) {
    auto insertPos = std::upper_bound(m_studioCandidates.begin(), m_studioCandidates.end(), studio);
    m_studioCandidates.insert(insertPos, studio);
  }

  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Studio);
  const int cnt = _JsonModel->SetStudio(indexes, studio);
  const QString affectedRowsMsg{QString{"[Uncommit] %1/%2 row(s) studio has been changed to %s"}.arg(cnt).arg(indexes.size()).arg(studio)};
  LOG_GOOD(affectedRowsMsg, studio);
  return indexes.size();
}

int JsonTableView::onInitCastAndStudio() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO("nothing selected", "skip init cast/studio studio");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Studio);
  const int cnt = _JsonModel->InitCastAndStudio(indexes);
  const QString affectedRowsMsg{QString{"[Uncommit] %1/%2 row(s) cast/studio has been inited"}.arg(cnt).arg(indexes.size())};
  LOG_GOOD(affectedRowsMsg, "ok");
  return indexes.size();
}

int JsonTableView::onHintCastAndStudio() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO("nothing selected", "skip hint cast/studio");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Studio);
  const int cnt = _JsonModel->HintCastAndStudio(indexes);
  const QString affectedRowsMsg{QString{"[Uncommit] %1/%2 row(s) cast/studio has been hint"}.arg(cnt).arg(indexes.size())};
  LOG_GOOD(affectedRowsMsg, "ok");
  return indexes.size();
}

int JsonTableView::onFormatCast() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO("nothing selected", "skip cast format");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Cast);
  const int cnt = _JsonModel->FormatCast(indexes);
  const QString affectedRowsMsg{QString{"[Uncommit] %1/%2 row(s) cast has been format"}.arg(cnt).arg(indexes.size())};
  LOG_GOOD(affectedRowsMsg, "ok");
  return indexes.size();
}

int JsonTableView::onClearStudio() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO("nothing selected", "skip clear studio");
    return 0;
  }
  const QModelIndexList& indexes = selectedRowsSource(JSON_KEY_E::Studio);
  const int cnt = _JsonModel->SetStudio(indexes, "");
  const QString affectedRowsMsg{QString{"[Uncommit] %1/%2 row(s) studio has been cleared"}.arg(cnt).arg(indexes.size())};
  LOG_GOOD(affectedRowsMsg, "ok");
  return indexes.size();
}

int JsonTableView::onSetCastOrTags(const FIELD_OP_TYPE type, const FIELD_OP_MODE mode) {
  const QString fieldOperation{"Operation:" + FIELF_OP_TYPE_ARR[(int)type] + ' ' + FIELD_OP_MODE_ARR[(int)mode]};
  if (!selectionModel()->hasSelection()) {
    LOG_INFO("nothing selected. skip", fieldOperation);
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
      LOG_GOOD("[Skip] User cancel", fieldOperation)
      return 0;
    }
    if (tagsOrCast.isEmpty()) {
      LOG_BAD("[Abort] Input can not be empty", fieldOperation)
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
      LOG_BAD("Field Type invalid", QString::number((int)type));
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
      LOG_BAD("Field Operation invalid", QString::number((int)mode));
      return -2;
  }
  const QString affectedRowsMsg{QString{"[Uncommit] %1/%2 row(s) affected by [%3]"}.arg(cnt).arg(indexes.size()).arg(fieldOperation)};
  LOG_GOOD(affectedRowsMsg, tagsOrCast);
  return indexes.size();
}

int JsonTableView::onAppendFromSelection(bool isUpperCaseSentence) {
  const QModelIndex& curInd = CurrentIndexSource();
  if (!curInd.isValid()) {
    LOG_WARN("Current Index is invalid", "select a line first");
    return -1;
  }
  QWidget* editor = indexWidget(curInd);
  if (editor == nullptr) {
    LOG_WARN("editor is nullptr", "failed");
    return -1;
  }
  auto lineEdit = qobject_cast<QLineEdit*>(editor);
  if (lineEdit == nullptr) {
    LOG_WARN("lineEdit is nullptr", "failed");
    return -1;
  }
  const QString userSelection{lineEdit->selectedText()};
  if (userSelection.trimmed().isEmpty()) {
    LOG_WARN("User selection text empty", "failed");
    return -1;
  }
  int cnt = _JsonModel->AppendCastFromSentence(curInd, userSelection, isUpperCaseSentence);
  if (cnt < 0) {
    LOG_BAD("append failed", "see detail in logs");
    return -1;
  }
  QString msg{QString{"%1 cast append succeed"}.arg(cnt)};
  LOG_GOOD(msg, userSelection);
  return cnt;
}

int JsonTableView::onSelectionCaseOperation(bool isTitle) {
  const QModelIndex& curInd = CurrentIndexSource();
  if (!curInd.isValid()) {
    LOG_WARN("Current Index is invalid", "select a line first");
    return -1;
  }
  QWidget* editor = indexWidget(curInd);
  if (editor == nullptr) {
    LOG_WARN("editor is nullptr", "failed");
    return -1;
  }
  auto lineEdit = qobject_cast<QLineEdit*>(editor);
  if (lineEdit == nullptr) {
    LOG_WARN("lineEdit is nullptr", "failed");
    return -1;
  }
  const QString userSelection{lineEdit->selectedText()};
  if (userSelection.trimmed().isEmpty()) {
    LOG_WARN("User selection text empty", "failed");
    return -1;
  }
  bool ret = NameTool::ReplaceAndUpdateSelection(*lineEdit, (isTitle ? NameTool::CapitaliseFirstLetterKeepOther : NameTool::Lower));
  if (!ret) {
    LOG_BAD("Change selection case failed", "see detail in logs");
    return -1;
  }
  LOG_GOOD("Change selection text case succeed", userSelection);
  return 0;
}

void JsonTableView::subscribe() {
  auto& inst = g_JsonActions();

  connect(inst._SAVE_CURRENT_CHANGES, &QAction::triggered, this, &JsonTableView::onSaveCurrentChanges);

  connect(inst._SYNC_NAME_FIELD_BY_FILENAME, &QAction::triggered, this, &JsonTableView::onSyncNameField);
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

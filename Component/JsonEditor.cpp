#include "JsonEditor.h"

#include "Actions/JsonEditorActions.h"

#include "Component/NotificatorFrame.h"
#include "Tools/ProductionStudioManager.h"

#include "Tools/JsonFileHelper.h"
#include "Tools/NameTool.h"
#include "Tools/PerformersAkaManager.h"
#include "Tools/PerformersManager.h"
#include "Tools/StringEditHelper.h"

#include <QDesktopServices>
#include <QDirIterator>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenuBar>
#include <QTableWidgetItem>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QToolBar>

const QString JsonEditor::TITLE_TEMPLATE = "Json Editor [Delta:%1/Total:%2]";
const QColor JsonEditor::MEET_CONDITION_COLOR(150, 150, 150);
const QColor JsonEditor::NOT_MEET_CONDITION_COLOR(255, 0, 0);

JsonEditor::JsonEditor(QWidget* parent)
    : QMainWindow{parent},
      freqJsonKeyValue{{JSONKey::Name, new QLineEdit},     {JSONKey::Performers, new QLineEdit}, {JSONKey::ProductionStudio, new QLineEdit},
                       {JSONKey::Uploaded, new QLineEdit}, {JSONKey::Tags, new QLineEdit},       {JSONKey::Rate, new QLineEdit},
                       {JSONKey::Size, new QLineEdit},     {JSONKey::Resolution, new QLineEdit}, {JSONKey::Bitrate, new QLineEdit},
                       {JSONKey::Hot, new QLineEdit},      {JSONKey::Detail, new QTextEdit}},
      m_editorPanel(new QFormLayout),
      m_extraEditorPanel(new QFormLayout),
      m_editorWidget(new QWidget(this)),

      m_jsonModel{new JsonModel{this}},
      m_jsonList(new JsonListView{m_jsonModel, this}),

      m_menuBar(g_jsonEditorActions().GetJsonMenuBar(this)),
      m_editorToolBar(g_jsonEditorActions().GetJsonToolBar(this)),
      m_editorAndListSplitter(new QSplitter(Qt::Orientation::Horizontal, this)) {
  setMenuBar(m_menuBar);
  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_editorToolBar);

  for (const QString& jsonKey : JSONKey::JsonKeyListOrder) {
    m_editorPanel->addRow(jsonKey, freqJsonKeyValue[jsonKey]);
  }
  m_editorPanel->setLabelAlignment(Qt::AlignmentFlag::AlignRight);
  m_editorPanel->setRowWrapPolicy(QFormLayout::RowWrapPolicy::WrapLongRows);
  m_editorPanel->addRow(m_extraEditorPanel);

  m_editorWidget->setLayout(m_editorPanel);

  m_editorAndListSplitter->addWidget(m_editorWidget);
  m_editorAndListSplitter->addWidget(m_jsonList);
  m_editorAndListSplitter->setStyleSheet(QString("QTextEdit {font-size: %1pt} \r\n QLineEdit {font-size: %1pt};").arg(13));
  setCentralWidget(m_editorAndListSplitter);

  subscribe();

  setWindowTitle(TITLE_TEMPLATE);
  setWindowIcon(QIcon(":/themes/SHOW_FOLDER_PREVIEW_JSON_EDITOR"));

  updateWindowsSize();
}

void JsonEditor::refreshEditPanel(const QModelIndex& curIndex) {
  jsonKeySetMet.clear();
  if (not curIndex.isValid()) {
    qWarning("Current index invalid");
    return;
  }
  m_jsonModel->updatePerfCount(curIndex.row());
  const QString& curJsonPath = m_jsonList->filePath(curIndex.row());
  qDebug("refreshEditPanel %s", qPrintable(curJsonPath));
  const auto& jsonDict = JsonFileHelper::MovieJsonLoader(curJsonPath);
  const QList<QPair<QString, QVariant>>& jsonItem = JsonFileHelper::MapToOrderedList(jsonDict);

  for (int r = m_extraEditorPanel->rowCount() - 1; r > -1; --r) {
    m_extraEditorPanel->removeRow(r);
  }
  if (jsonItem.isEmpty()) {
    return;
  }

  for (auto it = jsonItem.cbegin(); it != jsonItem.cend(); ++it) {
    const QString& keyName = it->first;
    const QVariant& v = it->second;
    jsonKeySetMet.insert(keyName);
    QString valueStr = JsonFileHelper::GetJsonValueString(keyName, v);
    if (freqJsonKeyValue.contains(keyName)) {
      if (keyName == JSONKey::Detail) {
        qobject_cast<QTextEdit*>(freqJsonKeyValue[JSONKey::Detail])->setText(valueStr);
      } else {
        qobject_cast<QLineEdit*>(freqJsonKeyValue[keyName])->setText(valueStr);
      }
      continue;
    }
    m_extraEditorPanel->addRow(keyName, new QLineEdit(valueStr));
  }
  if (not jsonKeySetMet.contains(JSONKey::Performers)) {
    jsonKeySetMet.insert(JSONKey::Performers);
    onPerformersHint();
  }
  if (not jsonKeySetMet.contains(JSONKey::ProductionStudio)) {
    jsonKeySetMet.insert(JSONKey::ProductionStudio);
    qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::ProductionStudio])->setText("");
  }
  if (not jsonKeySetMet.contains(JSONKey::Hot)) {
    jsonKeySetMet.insert(JSONKey::Hot);
    qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Hot])->setText("");
  }
  if (not jsonKeySetMet.contains(JSONKey::Rate)) {
    jsonKeySetMet.insert(JSONKey::Rate);
    qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Rate])->setText("-1");
  }
  m_editorPanel->itemAt(0, QFormLayout::ItemRole::FieldRole)->widget()->setFocus();
}

void JsonEditor::subscribe() {
  connect(m_jsonList->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &JsonEditor::refreshEditPanel);

  connect(g_jsonEditorActions()._FORMATTER, &QAction::triggered, this, &JsonEditor::formatter);
  connect(g_jsonEditorActions()._RELOAD_JSON_FROM_FROM_DISK, &QAction::triggered, this, [this]() { refreshEditPanel(m_jsonList->currentIndex()); });

  connect(g_jsonEditorActions()._NEXT_FILE, &QAction::triggered, m_jsonList, &JsonListView::onNext);
  connect(g_jsonEditorActions()._LAST_FILE, &QAction::triggered, m_jsonList, &JsonListView::onLast);
  connect(g_jsonEditorActions()._DONE_AND_NEXT, &QAction::triggered, this, &JsonEditor::onSaveAndNextUnfinishedItem);
  connect(g_jsonEditorActions()._COMPLETE_PERFS_COUNT, &QAction::triggered, m_jsonList, &JsonListView::onSetPerfCount);

  connect(g_jsonEditorActions()._SAVE, &QAction::triggered, this, &JsonEditor::onStageChanges);  // (.json, save former .backup for recover)
  connect(g_jsonEditorActions()._CANCEL, &QAction::triggered, this, &JsonEditor::onResetChanges);
  connect(g_jsonEditorActions()._SUBMIT, &QAction::triggered, this, [this]() {
    qDebug("submit all");  //  (.json and delete .backup)
    const auto ret = QMessageBox::warning(this, "Submit all ?", "Cannot recover", QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                                          QMessageBox::StandardButton::No);
    if (ret == QMessageBox::StandardButton::Yes) {
      onSubmitAllChanges();
    }
  });

  connect(g_jsonEditorActions()._ADD_SELECTED_PERFORMER, &QAction::triggered, this, &JsonEditor::onSelectedTextAppendToPerformers);
  connect(g_jsonEditorActions()._EXTRACT_CAPITALIZED_PERFORMER, &QAction::triggered, this, &JsonEditor::onExtractCapitalizedPerformersHint);

  connect(g_jsonEditorActions()._BROWSE_AND_SELECT_THE_FOLDER, &QAction::triggered, this, [this]() { this->onLoadASelectedPath(); });

  connect(g_jsonEditorActions()._LOWER_ALL_WORDS, &QAction::triggered, this, &JsonEditor::onLowercaseEachWord);
  connect(g_jsonEditorActions()._CAPITALIZE_FIRST_LETTER_OF_EACH_WORD, &QAction::triggered, this, &JsonEditor::onCapitalizeEachWord);
  connect(g_jsonEditorActions()._LEARN_PERFORMERS_FROM_JSON, &QAction::triggered, this, &JsonEditor::onLearnPerfomersFromJsonFile);
  connect(g_jsonEditorActions()._AI_HINT, &QAction::triggered, this, &JsonEditor::onPerformersHint);

  connect(g_jsonEditorActions()._STUDIO_INFORMATION, &QAction::triggered, this, [this]() {
    static auto& psm = ProductionStudioManager::getIns();
    psm.DisplayStatistic(this);
  });
  connect(g_jsonEditorActions()._EDIT_STUDIOS, &QAction::triggered, this, &JsonEditor::onEditStudios);
  connect(g_jsonEditorActions()._RELOAD_STUDIOS, &QAction::triggered, this, []() {
    static auto& psm = ProductionStudioManager::getIns();
    int itemsCntChanged = psm.ForceReloadStdStudioName();
    Notificator::information("Reload standard studio name", QString("delta %1 items").arg(itemsCntChanged));
  });

  connect(g_jsonEditorActions()._PERFORMERS_INFORMATION, &QAction::triggered, this, [this]() {
    static auto& pm = PerformersManager::getIns();
    pm.DisplayStatistic(this);
  });
  connect(g_jsonEditorActions()._EDIT_PERFS, &QAction::triggered, this, &JsonEditor::onEditPerformers);
  connect(g_jsonEditorActions()._RELOAD_PERFS, &QAction::triggered, this, []() {
    static auto& pm = PerformersManager::getIns();
    int itemsCntChanged = pm.ForceReloadPerformers();
    Notificator::information("Reload performers", QString("delta %1 item(s)").arg(itemsCntChanged));
  });

  connect(g_jsonEditorActions()._AKA_PERFORMERS_INFORMATION, &QAction::triggered, this, [this]() {
    static auto& dbTM = PerformersAkaManager::getIns();
    dbTM.DisplayStatistic(this);
  });
  connect(g_jsonEditorActions()._EDIT_PERF_AKA, &QAction::triggered, this, &JsonEditor::onEditAkaPerformer);
  connect(g_jsonEditorActions()._RELOAD_PERF_AKA, &QAction::triggered, this, []() {
    static auto& dbTM = PerformersAkaManager::getIns();
    int itemsCntChanged = dbTM.ForceReloadAkaName();
    Notificator::information("Reload AKA performers", QString("delta %1 item(s)").arg(itemsCntChanged));
  });
}

int JsonEditor::onLoadASelectedPath(const QString& folderPath) {
  QString loadFromPath = folderPath;
  if (folderPath.isEmpty()) {
    const QString& defaultOpenDir =
        PreferenceSettings().value(MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.name, MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.v).toString();
    loadFromPath = QFileDialog::getExistingDirectory(this, "Learn From", defaultOpenDir);
  }
  QFileInfo loadFromFi(loadFromPath);
  if (not loadFromFi.isDir()) {
    QMessageBox::warning(this, "Failed when Load json from a folder", QString("Not a folder:\n%1").arg(folderPath));
    qWarning("Failed when Load json from a folder. Not a folder:\n%s", qPrintable(folderPath));
    return 0;
  }
  PreferenceSettings().setValue(MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.name, loadFromFi.absoluteFilePath());
  return load(loadFromFi.absoluteFilePath());
}

bool JsonEditor::onStageChanges() {
  const auto curRow = m_jsonList->currentRow();
  if (not(0 <= curRow and curRow < m_jsonList->count())) {
    qWarning("try save on out of range[0, %d] row[%d]", m_jsonList->count(), curRow);
    return false;
  }

  QVariantHash dict;
  for (auto r = 0; r != m_extraEditorPanel->rowCount(); ++r) {
    const QString& keyName = qobject_cast<QLabel*>(m_extraEditorPanel->itemAt(r, QFormLayout::ItemRole::LabelRole)->widget())->text();
    const QString& valueStr = qobject_cast<QLineEdit*>(m_extraEditorPanel->itemAt(r, QFormLayout::ItemRole::FieldRole)->widget())->text();
    dict.insert(keyName, valueStr);
  }

  for (const QString& keyName : jsonKeySetMet) {
    if (keyName == JSONKey::Detail) {
      dict.insert(keyName, qobject_cast<QTextEdit*>(freqJsonKeyValue[JSONKey::Detail])->toPlainText());
      continue;
    }
    if (not freqJsonKeyValue.contains(keyName)) {
      continue;
    }

    const QString& valueStr = qobject_cast<QLineEdit*>(freqJsonKeyValue[keyName])->text().trimmed();
    if (keyName == JSONKey::Performers or keyName == JSONKey::Tags) {
      const auto& arr = NameTool()(valueStr);
      dict.insert(keyName, arr);
    } else if (keyName == JSONKey::Hot) {
      const auto& arr = JsonFileHelper::HotSceneString2IntList(valueStr);
      dict.insert(keyName, arr);
    } else if (keyName == JSONKey::Rate) {
      bool isOk = false;
      int rateNumer = valueStr.toInt(&isOk);
      if (not isOk) {
        qDebug("Rate[%s] is not a number, will use default value -1", qPrintable(valueStr));
        rateNumer = -1;
      }
      dict.insert(keyName, rateNumer);
    } else {
      dict.insert(keyName, valueStr);
    }
  }
  if (dict.contains(JSONKey::Performers)) {
    const int newCount = dict[JSONKey::Performers].toStringList().size();
    m_jsonModel->setPerfCount(curRow, newCount);
  }

  const QString& curJsonPath = m_jsonList->filePath(curRow);
  const QString& backupJsonPath = getBackupJsonFile(curJsonPath);
  if (QFile::exists(backupJsonPath)) {
    const auto rmRet = QFile::remove(backupJsonPath);
    if (not rmRet) {
      qDebug("cannot rm json file[%s]", qPrintable(backupJsonPath));
      return false;
    }
  }
  const auto copyRet = QFile::copy(curJsonPath, backupJsonPath);
  qDebug("result:%d, changes->%s, backup: %s", int(copyRet), qPrintable(curJsonPath), qPrintable(backupJsonPath));
  if (not copyRet) {
    qDebug("cannot copy json file[%s]", qPrintable(backupJsonPath));
    return false;
  }
  return JsonFileHelper::MovieJsonDumper(dict, curJsonPath);
}

bool JsonEditor::onSaveAndNextUnfinishedItem() {
  const bool isSavedSucceed = onStageChanges();
  if (not isSavedSucceed) {
    return false;
  }
  m_jsonList->onNext();
  return true;
}

bool JsonEditor::onResetChanges() {
  const auto curRow = m_jsonList->currentRow();
  if (curRow == -1) {
    return true;
  }
  const QString& curJsonPath = m_jsonList->filePath(curRow);
  const QString& backupJsonPath = getBackupJsonFile(curJsonPath);
  if (not QFile::exists(backupJsonPath)) {
    qDebug("cannot reset. backup file[%s] not exist", qPrintable(backupJsonPath));
    return false;
  }
  return QFile::rename(curJsonPath, curJsonPath + "mv") and QFile::rename(backupJsonPath, curJsonPath) and
         QFile::rename(curJsonPath + "mv", backupJsonPath);
}

bool JsonEditor::onSubmitAllChanges() {
  int succeedCnt = 0;
  int failCnt = 0;
  for (auto r = 0; r < m_jsonList->count(); ++r) {
    const QString& curJsonPath = m_jsonList->filePath(r);
    const QString& backupJsonPath = getBackupJsonFile(curJsonPath);
    if (QFile::exists(backupJsonPath)) {
      const auto ret = QFile::remove(backupJsonPath);
      succeedCnt += int(ret);
      failCnt += 1 - int(ret);
    }
  }
  qDebug("%d/%d succeed", succeedCnt, succeedCnt + failCnt);
  return failCnt == 0;
}

auto JsonEditor::onLowercaseEachWord() -> void {
  for (const QString& keyName : jsonKeySetMet) {
    if (keyName == JSONKey::Detail) {
      QTextEdit* detailEditWidget = qobject_cast<QTextEdit*>(freqJsonKeyValue[JSONKey::Detail]);
      StringEditHelper::ReplaceAndUpdateSelection(detailEditWidget, StringEditHelper::lowercaseSentense);
      continue;
    }
    QLineEdit* lineEditWidget = qobject_cast<QLineEdit*>(freqJsonKeyValue[keyName]);
    StringEditHelper::ReplaceAndUpdateSelection(lineEditWidget, StringEditHelper::lowercaseSentense);
  }
}

auto JsonEditor::onCapitalizeEachWord() -> void {
  for (const QString& keyName : jsonKeySetMet) {
    if (keyName == JSONKey::Detail) {
      QTextEdit* detailEditWidget = qobject_cast<QTextEdit*>(freqJsonKeyValue[JSONKey::Detail]);
      StringEditHelper::ReplaceAndUpdateSelection(detailEditWidget, StringEditHelper::capitalizeEachWord);
      continue;
    }
    QLineEdit* lineEditWidget = qobject_cast<QLineEdit*>(freqJsonKeyValue[keyName]);
    StringEditHelper::ReplaceAndUpdateSelection(lineEditWidget, StringEditHelper::capitalizeEachWord);
  }
}

bool JsonEditor::onLearnPerfomersFromJsonFile() {
  const QString& loadFromDefaultPath =
      PreferenceSettings().value(MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.name, MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.v).toString();
  const auto loadFromPath = QFileDialog::getExistingDirectory(this, "Choose location", loadFromDefaultPath);
  QFileInfo loadFromFi(loadFromPath);
  if (not loadFromFi.isDir()) {
    QMessageBox::information(this, "Cancel Learning", QString("Path[%1] not directory").arg(loadFromFi.absoluteFilePath()));
    return false;
  }
  PreferenceSettings().setValue(MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.name, loadFromFi.absoluteFilePath());

  static PerformersManager& pm = PerformersManager::getIns();
  const int newLearnedCnt = pm.LearningFromAPath(loadFromFi.absoluteFilePath());

  static ProductionStudioManager& psm = ProductionStudioManager::getIns();
  const int newLearnedProdStudioCnt = psm.LearningFromAPath(loadFromFi.absoluteFilePath());

  QMessageBox::information(
      this, "Learning succeed",
      QString("New Learned:\n Performers Count:%1\n Production Studios Count:%2").arg(newLearnedCnt).arg(newLearnedProdStudioCnt));

  return newLearnedCnt >= 0 or newLearnedProdStudioCnt >= 0;
}

QStringList JsonEditor::onPerformersHint() {
  static PerformersManager& pm = PerformersManager::getIns();
  static ProductionStudioManager& psm = ProductionStudioManager::getIns();

  QString nameText;
  QString sentence;
  if (jsonKeySetMet.contains(JSONKey::Name)) {
    nameText = qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Name])->text();
    sentence += nameText;
  }
  if (jsonKeySetMet.contains(JSONKey::Detail)) {
    auto* te = qobject_cast<QTextEdit*>(freqJsonKeyValue[JSONKey::Detail]);
    if (te->textCursor().hasSelection()) {
      sentence += " " + te->textCursor().selection().toPlainText();
    }
  }
  if (not jsonKeySetMet.contains(JSONKey::Performers)) {
    jsonKeySetMet.insert(JSONKey::Performers);
  }
  auto* p = qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Performers]);
  const QStringList& newPerfsList = pm(sentence);
  QStringList beforePerfsList;
  if (not p->text().isEmpty()) {
    beforePerfsList = NameTool()(p->text());
  }
  if (beforePerfsList.size() < newPerfsList.size()) {
    p->setText(newPerfsList.join(", "));
  }

  if (not jsonKeySetMet.contains(JSONKey::ProductionStudio)) {
    jsonKeySetMet.insert(JSONKey::ProductionStudio);
  }

  auto* ps = qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::ProductionStudio]);
  const QString newProdStudioName = psm(nameText);
  if (not newProdStudioName.isEmpty()) {
    ps->setText(newProdStudioName);
  }
  return newPerfsList;
}

auto JsonEditor::onExtractCapitalizedPerformersHint() -> bool {
  static PerformersManager& pm = PerformersManager::getIns();

  if (not jsonKeySetMet.contains(JSONKey::Performers)) {
    jsonKeySetMet.insert(JSONKey::Performers);
  }

  QStringList perfs;
  auto* p = qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Performers]);
  if (not p->text().isEmpty()) {
    perfs += NameTool()(p->text());
  }

  if (jsonKeySetMet.contains(JSONKey::Name)) {
    const auto* le = qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Name]);
    if (le->hasSelectedText()) {
      const QString& capitalizedStr = le->selectedText();
      perfs << NameTool().fromArticleCapitalizedNames(capitalizedStr);
    }
  }
  if (jsonKeySetMet.contains(JSONKey::Detail)) {
    const auto* te = qobject_cast<QTextEdit*>(freqJsonKeyValue[JSONKey::Detail]);
    if (te->textCursor().hasSelection()) {
      const QString& capitalizedStr = te->textCursor().selection().toPlainText();
      perfs += NameTool().fromArticleCapitalizedNames(capitalizedStr);
    }
  }
  perfs.removeDuplicates();
  p->setText(perfs.join(", "));
  return true;
}

bool JsonEditor::onSelectedTextAppendToPerformers() {
  static PerformersManager& pm = PerformersManager::getIns();

  if (not jsonKeySetMet.contains(JSONKey::Performers)) {
    jsonKeySetMet.insert(JSONKey::Performers);
  }

  QStringList perfs;
  auto* p = qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Performers]);
  if (not p->text().isEmpty()) {
    perfs += NameTool()(p->text());
  }

  if (jsonKeySetMet.contains(JSONKey::Name)) {
    const auto* le = qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Name]);
    if (le->hasSelectedText()) {
      perfs << le->selectedText();
    }
  }
  if (jsonKeySetMet.contains(JSONKey::Detail)) {
    const auto* te = qobject_cast<QTextEdit*>(freqJsonKeyValue[JSONKey::Detail]);
    if (te->textCursor().hasSelection()) {
      perfs << te->textCursor().selection().toPlainText();
    }
  }
  perfs.removeDuplicates();
  p->setText(perfs.join(", "));
  return true;
}

void JsonEditor::onEditPerformers() {
#ifdef _WIN32
  QString fileAbsPath = PreferenceSettings().value(MemoryKey::WIN32_PERFORMERS_TABLE.name).toString();
#else
  QString fileAbsPath = PreferenceSettings().value(MemoryKey::LINUX_PERFORMERS_TABLE.name).toString();
#endif
  if (not QFile::exists(fileAbsPath)) {
    qDebug("Cannot edit. File[%s] not found", qPrintable(fileAbsPath));
    Notificator::warning("Cannot edit", QString("File[%1] not found").arg(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  Notificator::information("Remember to reload", "don't forget it");
}

void JsonEditor::onEditAkaPerformer() {
#ifdef _WIN32
  QString fileAbsPath = PreferenceSettings().value(MemoryKey::WIN32_AKA_PERFORMERS.name).toString();
#else
  QString fileAbsPath = PreferenceSettings().value(MemoryKey::LINUX_AKA_PERFORMERS.name).toString();
#endif
  if (not QFile::exists(fileAbsPath)) {
    qDebug("Cannot edit. File[%s] not found", qPrintable(fileAbsPath));
    Notificator::warning("Cannot edit", QString("File[%1] not found").arg(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  Notificator::information("Work after reopen", "changes not work now");
}

void JsonEditor::onEditStudios() {
#ifdef _WIN32
  QString fileAbsPath = PreferenceSettings().value(MemoryKey::WIN32_STANDARD_STUDIO_NAME.name).toString();
#else
  QString fileAbsPath = PreferenceSettings().value(MemoryKey::LINUX_STANDARD_STUDIO_NAME.name).toString();
#endif
  if (not QFile::exists(fileAbsPath)) {
    qDebug("Cannot edit. File[%s] not found", qPrintable(fileAbsPath));
    Notificator::warning("Cannot edit", QString("File[%1] not found").arg(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  Notificator::information("Work after rebuild", "changes not work now");
}

bool JsonEditor::formatter() {
  if (jsonKeySetMet.contains(JSONKey::Performers)) {
    auto* lineWidget = qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Performers]);
    const QStringList& stdNameList = NameTool()(lineWidget->text());
    lineWidget->setText(stdNameList.join(", "));
  }
  if (jsonKeySetMet.contains(JSONKey::Tags)) {
    auto* lineWidget = qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Tags]);
    const QStringList& stdTagsList = NameTool()(lineWidget->text());
    lineWidget->setText(stdTagsList.join(", "));
  }
}

int JsonEditor::load(const QString& path) {
  const int deltaFile = m_jsonList->load(path);
  setWindowTitle(TITLE_TEMPLATE.arg(deltaFile).arg(m_jsonList->count()));
  return deltaFile;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  JsonEditor jsonEditor;
  jsonEditor.show();
  jsonEditor.load(PROJECT_PATH + "/bin/JsonExample");
  a.exec();
  return 0;
}
#endif

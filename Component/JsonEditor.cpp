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

JsonEditor::JsonEditor(QWidget* parent)
    : QMainWindow{parent},
      m_stdKeys{{JSONKey::Name, new QLineEdit},     {JSONKey::Performers, new QLineEdit}, {JSONKey::ProductionStudio, new QLineEdit},
                {JSONKey::Uploaded, new QLineEdit}, {JSONKey::Tags, new QLineEdit},       {JSONKey::Rate, new QLineEdit},
                {JSONKey::Size, new QLineEdit},     {JSONKey::Resolution, new QLineEdit}, {JSONKey::Bitrate, new QLineEdit},
                {JSONKey::Hot, new QLineEdit},      {JSONKey::Detail, new QTextEdit}},
      m_jsonFormLo(new QFormLayout),
      m_jsonFormExtraLo(new QFormLayout),
      m_jsonFormWid(new QWidget(this)),

      m_jsonModel{new JsonModel{this}},
      m_jsonList(new JsonListView{m_jsonModel, this}),

      m_menuBar(g_jsonEditorActions().GetJsonMenuBar(this)),
      m_toolBar(g_jsonEditorActions().GetJsonToolBar(this)),
      m_splitter(new QSplitter(Qt::Orientation::Horizontal, this)) {
  setMenuBar(m_menuBar);
  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolBar);

  for (const QString& key : JSONKey::JsonKeyListOrder) {
    m_jsonFormLo->addRow(key, m_stdKeys[key]);
  }
  m_jsonFormLo->setLabelAlignment(Qt::AlignmentFlag::AlignRight);
  m_jsonFormLo->setRowWrapPolicy(QFormLayout::RowWrapPolicy::WrapLongRows);
  m_jsonFormLo->addRow(m_jsonFormExtraLo);

  m_jsonFormWid->setLayout(m_jsonFormLo);

  m_splitter->addWidget(m_jsonFormWid);
  m_splitter->addWidget(m_jsonList);
  m_splitter->setStyleSheet(QString("QTextEdit {font-size: %1pt}"
                                    "QLineEdit {font-size: %1pt};")
                                .arg(13));
  setCentralWidget(m_splitter);

  subscribe();

  setWindowTitle(TITLE_TEMPLATE);
  setWindowIcon(QIcon(":img/JSON_EDITOR"));

  updateWindowsSize();
}

void JsonEditor::refreshEditPanel(const QModelIndex& curIndex) {
  m_keysMet.clear();
  if (not curIndex.isValid()) {
    qWarning("Current index invalid");
    return;
  }
  m_jsonModel->updatePerfCount(curIndex.row());

  for (int r = m_jsonFormExtraLo->rowCount() - 1; r > -1; --r) {
    m_jsonFormExtraLo->removeRow(r);
  }

  const QString& curJsonPath = m_jsonList->filePath(curIndex.row());
  const auto& jsonDict = JsonFileHelper::MovieJsonLoader(curJsonPath);
  const QList<QPair<QString, QVariant>>& jsonList = JsonFileHelper::MapToOrderedList(jsonDict);
  if (jsonList.isEmpty()) {
    return;
  }

  for (auto it = jsonList.cbegin(); it != jsonList.cend(); ++it) {
    const QString& key = it->first;
    const QVariant& var = it->second;
    m_keysMet.insert(key);

    QString valueStr = JsonFileHelper::GetJsonValueString(key, var);
    if (m_stdKeys.contains(key)) {
      if (key == JSONKey::Detail) {
        qobject_cast<QTextEdit*>(m_stdKeys[JSONKey::Detail])->setText(valueStr);
      } else {
        qobject_cast<QLineEdit*>(m_stdKeys[key])->setText(valueStr);
      }
      continue;
    }

    m_jsonFormExtraLo->addRow(key, new QLineEdit(valueStr));
  }
  if (not m_keysMet.contains(JSONKey::Performers)) {
    m_keysMet.insert(JSONKey::Performers);
    onPerformersHint();
  }
  if (not m_keysMet.contains(JSONKey::ProductionStudio)) {
    m_keysMet.insert(JSONKey::ProductionStudio);
    qobject_cast<QLineEdit*>(m_stdKeys[JSONKey::ProductionStudio])->setText("");
  }
  if (not m_keysMet.contains(JSONKey::Hot)) {
    m_keysMet.insert(JSONKey::Hot);
    qobject_cast<QLineEdit*>(m_stdKeys[JSONKey::Hot])->setText("");
  }
  if (not m_keysMet.contains(JSONKey::Rate)) {
    m_keysMet.insert(JSONKey::Rate);
    qobject_cast<QLineEdit*>(m_stdKeys[JSONKey::Rate])->setText("-1");
  }
  m_jsonFormLo->itemAt(0, QFormLayout::ItemRole::FieldRole)->widget()->setFocus();
  qDebug("Refresh json file display [%s]", qPrintable(curJsonPath));
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

  connect(g_jsonEditorActions()._RENAME_THIS_FILE, &QAction::triggered, this, &JsonEditor::onRenameJsonFile);
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
  const int curRow = m_jsonList->currentRow();
  if (not(0 <= curRow and curRow < m_jsonList->count())) {
    qWarning("Skip save row[%d] of out range[0, %d]", curRow, m_jsonList->count());
    return true;
  }

  QVariantHash dict;
  for (auto r = 0; r != m_jsonFormExtraLo->rowCount(); ++r) {
    const QString& keyName = qobject_cast<QLabel*>(m_jsonFormExtraLo->itemAt(r, QFormLayout::ItemRole::LabelRole)->widget())->text();
    const QString& valueStr = qobject_cast<QLineEdit*>(m_jsonFormExtraLo->itemAt(r, QFormLayout::ItemRole::FieldRole)->widget())->text();
    dict.insert(keyName, valueStr);
  }

  for (const QString& keyName : m_keysMet) {
    if (keyName == JSONKey::Detail) {
      dict.insert(keyName, qobject_cast<QTextEdit*>(m_stdKeys[JSONKey::Detail])->toPlainText());
      continue;
    }
    if (not m_stdKeys.contains(keyName)) {
      continue;
    }

    const QString& valueStr = qobject_cast<QLineEdit*>(m_stdKeys[keyName])->text().trimmed();
    if (keyName == JSONKey::Performers or keyName == JSONKey::Tags) {
      const auto& arr = NameTool()(valueStr);
      dict.insert(keyName, arr);
      auto* p = qobject_cast<QLineEdit*>(m_stdKeys[keyName]);
      p->setText(arr.join(", "));
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
  qDebug("result:%d, changed to->%s, backup source: %s", int(copyRet), qPrintable(curJsonPath), qPrintable(backupJsonPath));
  if (not copyRet) {
    qDebug("Copy json file failed[%s]", qPrintable(backupJsonPath));
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
  for (const QString& keyName : m_keysMet) {
    if (keyName == JSONKey::Detail) {
      QTextEdit* detailEditWidget = qobject_cast<QTextEdit*>(m_stdKeys[JSONKey::Detail]);
      StringEditHelper::ReplaceAndUpdateSelection(detailEditWidget, StringEditHelper::lowercaseSentense);
      continue;
    }
    QLineEdit* lineEditWidget = qobject_cast<QLineEdit*>(m_stdKeys[keyName]);
    StringEditHelper::ReplaceAndUpdateSelection(lineEditWidget, StringEditHelper::lowercaseSentense);
  }
}

auto JsonEditor::onCapitalizeEachWord() -> void {
  for (const QString& keyName : m_keysMet) {
    if (keyName == JSONKey::Detail) {
      QTextEdit* detailEditWidget = qobject_cast<QTextEdit*>(m_stdKeys[JSONKey::Detail]);
      StringEditHelper::ReplaceAndUpdateSelection(detailEditWidget, StringEditHelper::capitalizeEachWord);
      continue;
    }
    QLineEdit* lineEditWidget = qobject_cast<QLineEdit*>(m_stdKeys[keyName]);
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
  QString newProdStudioName;
  QString sentence;
  if (m_keysMet.contains(JSONKey::Name)) {
    nameText = qobject_cast<QLineEdit*>(m_stdKeys[JSONKey::Name])->text();
    newProdStudioName = psm.hintStdStudioName(nameText);
    sentence += nameText;
  }

  if (m_keysMet.contains(JSONKey::Detail)) {
    auto* te = qobject_cast<QTextEdit*>(m_stdKeys[JSONKey::Detail]);
    if (te->textCursor().hasSelection()) {
      sentence += " " + te->textCursor().selection().toPlainText();
    }
  }
  const QStringList& newPerfsList = pm(sentence);

  if (not m_keysMet.contains(JSONKey::Performers)) {
    m_keysMet.insert(JSONKey::Performers);
  }

  const QString& perfStr = GetArrLine(JSONKey::Performers);
  if (perfStr.isEmpty()) {
    UpdateDisplayArrLine(JSONKey::Performers, newPerfsList);
  } else {
    const QStringList& beforePerfsList = NameTool()(perfStr);
    if (beforePerfsList.size() < newPerfsList.size()) {
      UpdateDisplayArrLine(JSONKey::Performers, newPerfsList);
    }
  }

  if (not m_keysMet.contains(JSONKey::ProductionStudio)) {
    m_keysMet.insert(JSONKey::ProductionStudio);
  }

  auto* ps = qobject_cast<QLineEdit*>(m_stdKeys[JSONKey::ProductionStudio]);
  if (not newProdStudioName.isEmpty()) {
    ps->setText(newProdStudioName);
  }
  return newPerfsList;
}

auto JsonEditor::onExtractCapitalizedPerformersHint() -> bool {
  if (not m_keysMet.contains(JSONKey::Performers)) {
    m_keysMet.insert(JSONKey::Performers);
  }

  QStringList perfs;
  const QString& names = GetArrLine(JSONKey::Performers);
  if (not names.isEmpty()) {
    perfs += NameTool()(names);
  }

  if (m_keysMet.contains(JSONKey::Name)) {
    const auto* le = qobject_cast<QLineEdit*>(m_stdKeys[JSONKey::Name]);
    if (le->hasSelectedText()) {
      const QString& capitalizedStr = le->selectedText();
      perfs << NameTool().fromArticleCapitalizedNames(capitalizedStr);
    }
  }
  if (m_keysMet.contains(JSONKey::Detail)) {
    const auto* te = qobject_cast<QTextEdit*>(m_stdKeys[JSONKey::Detail]);
    if (te->textCursor().hasSelection()) {
      const QString& capitalizedStr = te->textCursor().selection().toPlainText();
      perfs += NameTool().fromArticleCapitalizedNames(capitalizedStr);
    }
  }
  perfs.removeDuplicates();
  UpdateDisplayArrLine(JSONKey::Performers, perfs);
  return true;
}

bool JsonEditor::onSelectedTextAppendToPerformers() {
  if (not m_keysMet.contains(JSONKey::Performers)) {
    m_keysMet.insert(JSONKey::Performers);
  }

  QStringList perfs;

  const QString& names = GetArrLine(JSONKey::Performers);
  if (not names.isEmpty()) {
    perfs += NameTool()(names);
  }

  if (m_keysMet.contains(JSONKey::Name)) {
    const auto* le = qobject_cast<QLineEdit*>(m_stdKeys[JSONKey::Name]);
    if (le->hasSelectedText()) {
      perfs << le->selectedText();
    }
  }

  if (m_keysMet.contains(JSONKey::Detail)) {
    const auto* te = qobject_cast<QTextEdit*>(m_stdKeys[JSONKey::Detail]);
    if (te->textCursor().hasSelection()) {
      perfs << te->textCursor().selection().toPlainText();
    }
  }

  perfs.removeDuplicates();
  UpdateDisplayArrLine(JSONKey::Performers, perfs);
  return true;
}

bool JsonEditor::onRenameJsonFile() {
  const QModelIndex& ind = m_jsonList->currentIndex();
  if (not ind.isValid()) {
    qWarning() << "Try to rename invalid ind" << ind;
    return true;
  }
  const QFileInfo fi{m_jsonModel->filePath(ind)};
  if (not fi.exists()) {
    qWarning("Skip json file[%s] not exist", qPrintable(fi.absoluteFilePath()));
    return true;
  }
  const QString beforeFileName = fi.fileName();
  QDir dir = fi.absoluteDir();

  bool okClicked = false;
  const QString& newFileName = QInputDialog::getItem(this, "Rename Jsons", fi.absolutePath(), {beforeFileName}, 0, true, &okClicked);
  if (not okClicked or newFileName.isEmpty()) {
    qInfo("Skip User cancel rename");
    return false;
  }
  if (beforeFileName == newFileName) {
    qInfo("Skip name not changed");
    return true;
  }
  const bool renameResult = dir.rename(beforeFileName, newFileName);
  if (not renameResult) {
    qWarning("Rename failed [%s]=>[%s]", qPrintable(beforeFileName), qPrintable(newFileName));
    Notificator::warning(QString("Rename failed[%1]").arg(fi.absolutePath()), beforeFileName + "=>" + newFileName);
    return false;
  }
  m_jsonModel->setData(ind, dir.absoluteFilePath(newFileName), Qt::DisplayRole);
  return true;
}

bool JsonEditor::formatter() {
  NameTool nt;
  if (m_keysMet.contains(JSONKey::Performers)) {
    const QString& nameStr = GetArrLine(JSONKey::Performers);
    UpdateDisplayArrLine(JSONKey::Performers, nt(nameStr));
  }
  if (m_keysMet.contains(JSONKey::Tags)) {
    const QString& tagStr = GetArrLine(JSONKey::Tags);
    UpdateDisplayArrLine(JSONKey::Tags, nt(tagStr));
  }
  return true;
}

int JsonEditor::load(const QString& path) {
  const int deltaFile = m_jsonList->load(path);
  setWindowTitle(TITLE_TEMPLATE.arg(deltaFile).arg(m_jsonList->count()));
  return deltaFile;
}

bool JsonEditor::IsValueOfKeyArr(const QString& key) const {
  if (not(key == JSONKey::Performers or key == JSONKey::Tags)) {
    qWarning("Value of Key[%s] is not registered as array", qPrintable(key));
    return false;
  }
  return true;
}

QString JsonEditor::GetArrLine(const QString& key) const {
  if (not IsValueOfKeyArr(key)) {
    qDebug("Cannot get ArrLine of key[%s]", qPrintable(key));
    return "";
  }
  auto* p = qobject_cast<QLineEdit*>(m_stdKeys[key]);
  return p->text();
}

void JsonEditor::UpdateDisplayArrLine(const QString& key, const QStringList& arr) {
  if (not IsValueOfKeyArr(key)) {
    qDebug("Cannot update ArrLine of key[%s]", qPrintable(key));
    return;
  }
  auto* p = qobject_cast<QLineEdit*>(m_stdKeys[key]);
  p->setText(arr.join(", "));
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

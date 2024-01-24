#include "JsonEditor.h"
#include "Actions/JsonEditorActions.h"
#include "Actions/QuickWhereActions.h"
#include "Component/ProductionStudioManager.h"
#include "Tools/JsonFileHelper.h"
#include "Tools/PerformersStringParser.h"

#include <QDesktopServices>
#include <QDirIterator>
#include <QFileDialog>
#include <QInputDialog>
#include <QTableWidgetItem>
#include <QTextDocumentFragment>
#include <QToolBar>

const QString JsonEditor::TITLE_TEMPLATE = "Json Editor [%1/%2]";
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

      m_jsonList(new QListWidget),
      m_listMenu(getListPanelRightClickMenu()),

      m_editorAndListSplitter(new QSplitter(Qt::Orientation::Horizontal, this)),
      m_editorToolBar(new QToolBar("json editor actions", this)) {
  m_jsonList->setContextMenuPolicy(Qt::CustomContextMenu);
  m_jsonList->setAlternatingRowColors(true);
  m_jsonList->setSortingEnabled(true);
  m_jsonList->setDragDropMode(QAbstractItemView::NoDragDrop);
  m_jsonList->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);

  m_editorToolBar->addActions(g_jsonEditorActions().FILES_ACTIONS->actions());
  m_editorToolBar->addSeparator();
  m_editorToolBar->addActions(g_jsonEditorActions().EDIT_ACTIONS->actions());
  m_editorToolBar->addSeparator();
  m_editorToolBar->addAction(g_jsonEditorActions()._LEARN_PERFORMERS_FROM_JSON);
  m_editorToolBar->addAction(g_jsonEditorActions()._HINT);
  m_editorToolBar->addAction(g_quickWhereAg().OPEN_AKA_TEXT);
  m_editorToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
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

  setCentralWidget(m_editorAndListSplitter);
  m_editorAndListSplitter->setStyleSheet(QString("QTextEdit {font-size: %1pt} \r\n QLineEdit {font-size: %1pt};").arg(13));

  subscribe();

  setWindowTitle(TITLE_TEMPLATE);
  setWindowIcon(QIcon(":/themes/SHOW_FOLDER_PREVIEW_JSON_EDITOR"));

  updateWindowsSize();
}

auto JsonEditor::getListPanelRightClickMenu() -> QMenu* {
  auto* menu = new QMenu("json list right click menu", this);
  menu->addAction(g_jsonEditorActions()._REVEAL_IN_EXPLORER);
  menu->addAction(g_jsonEditorActions()._OPEN_THIS_FILE);
  return menu;
}

bool JsonEditor::hasLast() const {
  const auto curRow = m_jsonList->currentRow();
  return curRow - 1 >= 0;
}

void JsonEditor::last() {
  const auto curRow = m_jsonList->currentRow() - 1;
  m_jsonList->setCurrentRow(curRow);
  qDebug("last curRow %d", curRow);
}

bool JsonEditor::hasNext() const {
  const auto curRow = m_jsonList->currentRow();
  return curRow + 1 < m_jsonList->count();
}

void JsonEditor::next() {
  const auto curRow = m_jsonList->currentRow() + 1;
  m_jsonList->setCurrentRow(curRow);
  qDebug("next curRow %d", curRow);
}

void JsonEditor::onAutoSkipSwitch(const bool checked) {
  if (not checked) {
    return;
  }
  bool okClicked = false;
  const int PAUSE_CNT = PreferenceSettings()
                            .value(MemoryKey::AUTO_SKIP_WHEN_PERFORMERS_CNT_LESS_THAN.name, MemoryKey::AUTO_SKIP_WHEN_PERFORMERS_CNT_LESS_THAN.v)
                            .toInt();
  const QString& pauseCntStr =
      QInputDialog::getItem(this, "Auto Skip when perforers count < ?", MemoryKey::AUTO_SKIP_WHEN_PERFORMERS_CNT_LESS_THAN.name,
                            {QString::number(PAUSE_CNT), "1", "2", "3", "4", "5"}, 0, false, &okClicked);
  if (not okClicked or pauseCntStr.isEmpty()) {
    qDebug("Cancel");
    return;
  }
  PreferenceSettings().setValue(MemoryKey::AUTO_SKIP_WHEN_PERFORMERS_CNT_LESS_THAN.name, pauseCntStr.toInt());
}

void JsonEditor::autoNext() {
  const auto reverseCondition = g_jsonEditorActions()._CONDITION_NOT->isChecked();
  const int PAUSE_CNT = PreferenceSettings()
                            .value(MemoryKey::AUTO_SKIP_WHEN_PERFORMERS_CNT_LESS_THAN.name, MemoryKey::AUTO_SKIP_WHEN_PERFORMERS_CNT_LESS_THAN.v)
                            .toInt();

  for (auto curRow = m_jsonList->currentRow() + 1; curRow < m_jsonList->count(); ++curRow) {
    const QString& curJsonPath = m_jsonList->item(curRow)->text();
    const auto& dict = JsonFileHelper::MovieJsonLoader(curJsonPath);
    if (dict.isEmpty()) {
      continue;
    }
    qDebug("len(performers)=%d, [%s]", dict[DB_HEADER_KEY::Performers].toJsonArray().size(), qPrintable(curJsonPath));
    if ((dict[DB_HEADER_KEY::Performers].toJsonArray().size() < PAUSE_CNT) == not reverseCondition) {
      m_jsonList->item(curRow)->setForeground(NOT_MEET_CONDITION_COLOR);
      m_jsonList->setCurrentRow(curRow);
      return;
    }
    m_jsonList->item(curRow)->setForeground(MEET_CONDITION_COLOR);
  }
  if (m_jsonList->count() > 0) {
    m_jsonList->setCurrentRow(m_jsonList->count() - 1);
  }
}

void JsonEditor::onNext() {
  if (g_jsonEditorActions()._AUTO_SKIP->isChecked()) {
    autoNext();
    return;
  }
  if (hasNext()) {
    next();
  }
}

void JsonEditor::onLast() {
  if (hasLast()) {
    last();
  }
}

void JsonEditor::refreshEditPanel() {
  jsonKeySetMet.clear();
  if (not m_jsonList->currentItem()) {
    qDebug("skip refresh edit panel");
    return;
  }

  const QString& curJsonPath = m_jsonList->currentItem()->text();
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
  connect(m_jsonList, &QListWidget::itemSelectionChanged, this, &JsonEditor::refreshEditPanel);
  connect(m_jsonList, &QListWidget::itemDoubleClicked, this,
          [this](const QListWidgetItem* item) { QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(item->text()).absolutePath())); });

  auto ShowContextMenu = [this](const QPoint pnt) {
    m_listMenu->popup(m_jsonList->mapToGlobal(pnt));  // or QCursor::pos()
  };
  connect(m_jsonList, &QListView::customContextMenuRequested, ShowContextMenu);

  connect(g_jsonEditorActions()._REVEAL_IN_EXPLORER, &QAction::triggered, this, [this]() {
    if (m_jsonList->currentItem())
      QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(m_jsonList->currentItem()->text()).absolutePath()));
  });
  connect(g_jsonEditorActions()._OPEN_THIS_FILE, &QAction::triggered, this, [this]() {
    if (m_jsonList->currentItem())
      QDesktopServices::openUrl(QUrl::fromLocalFile(m_jsonList->currentItem()->text()));
  });

  connect(g_jsonEditorActions()._FORMATTER, &QAction::triggered, this, &JsonEditor::formatter);
  connect(g_jsonEditorActions()._RELOAD_JSON_FROM_FROM_DISK, &QAction::triggered, this, &JsonEditor::refreshEditPanel);

  connect(g_jsonEditorActions()._NEXT_JSON, &QAction::triggered, this, &JsonEditor::onNext);
  connect(g_jsonEditorActions()._LAST_JSON, &QAction::triggered, this, &JsonEditor::onLast);

  connect(g_jsonEditorActions()._AUTO_SKIP, &QAction::triggered, this, &JsonEditor::onAutoSkipSwitch);

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

  connect(g_jsonEditorActions()._SELECT_A_FOLDER_AND_LOAD_JSON, &QAction::triggered, this, [this]() { this->onLoadASelectedPath(); });
  connect(g_jsonEditorActions()._EMPTY_JSONS_LISTWIDGET, &QAction::triggered, this->m_jsonList, &QListWidget::clear);

  connect(g_jsonEditorActions()._LOWER_ALL_WORDS, &QAction::triggered, this, &JsonEditor::onLowercaseEachWord);
  connect(g_jsonEditorActions()._CAPITALIZE_FIRST_LETTER_OF_EACH_WORD, &QAction::triggered, this, &JsonEditor::onCapitalizeEachWord);
  connect(g_jsonEditorActions()._LEARN_PERFORMERS_FROM_JSON, &QAction::triggered, this, &JsonEditor::onLearnPerfomersFromJsonFile);
  connect(g_jsonEditorActions()._HINT, &QAction::triggered, this, &JsonEditor::onPerformersHint);
}

bool JsonEditor::onLoadASelectedPath(const QString& folderPath) {
  QString loadFromPath = folderPath;
  if (folderPath.isEmpty()) {
    const QString& defaultOpenDir =
        PreferenceSettings().value(MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.name, MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.v).toString();
    loadFromPath = QFileDialog::getExistingDirectory(this, "Learn From", defaultOpenDir);
  }
  QFileInfo loadFromFi(loadFromPath);
  if (not loadFromFi.isDir()) {
    QMessageBox::warning(this, "Failed when Load json from a folder", QString("Not a folder:\n%1").arg(folderPath));
    qDebug("Failed when Load json from a folder. Not a folder:\n%s", qPrintable(folderPath));
    return false;
  }
  PreferenceSettings().setValue(MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.name, loadFromFi.absoluteFilePath());
  load(loadFromFi.absoluteFilePath());
}

bool JsonEditor::onStageChanges() {
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
      const auto& arr = JsonFileHelper::PerformersString2StringList(valueStr);
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

  const auto curRow = m_jsonList->currentRow();
  if (curRow == -1) {
    return true;
  }
  const QString& curJsonPath = m_jsonList->item(curRow)->text();
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

bool JsonEditor::onResetChanges() {
  const auto curRow = m_jsonList->currentRow();
  if (curRow == -1) {
    return true;
  }
  const QString& curJsonPath = m_jsonList->item(curRow)->text();
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
    const QString& curJsonPath = m_jsonList->item(r)->text();
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

#include <QTextCursor>
auto JsonEditor::onLowercaseEachWord() -> void {
  static auto lowercaseSentense = [](const QString& sentence) -> QString { return sentence.toLower(); };
  for (const QString& keyName : jsonKeySetMet) {
    if (keyName == JSONKey::Detail) {
      QTextEdit* detailEditWidget = qobject_cast<QTextEdit*>(freqJsonKeyValue[JSONKey::Detail]);
      if (not detailEditWidget->textCursor().hasSelection()) {
        continue;
      }
      const QString& before = detailEditWidget->textCursor().selection().toPlainText();
      detailEditWidget->textCursor().removeSelectedText();
      const QString& after = lowercaseSentense(before);
      detailEditWidget->textCursor().insertText(after);
      continue;
    }
    QLineEdit* lineEditWidget = qobject_cast<QLineEdit*>(freqJsonKeyValue[keyName]);
    if (not lineEditWidget->hasSelectedText()) {
      continue;
    }
    const QString& before = lineEditWidget->selectedText();
    const QString& after = lowercaseSentense(before);
    const QString& beforeFullText = lineEditWidget->text();
    const QString& afterFullText =
        QString("%1%2%3").arg(beforeFullText.left(lineEditWidget->selectionStart()), after, beforeFullText.mid(lineEditWidget->selectionEnd()));
    lineEditWidget->setText(afterFullText);
  }
}

auto JsonEditor::onCapitalizeEachWord() -> void {
  static auto capitalizeEachWord = [](QString sentence) -> QString {
    if (not sentence.isEmpty()) {
      sentence[0] = sentence[0].toTitleCase();
    }
    for (int i = 1; i < sentence.size(); ++i) {
      if (sentence[i - 1] == '.' or sentence[i - 1] == ' ') {
        sentence[i] = sentence[i].toTitleCase();
      }
    }
    return sentence;
  };
  for (const QString& keyName : jsonKeySetMet) {
    if (keyName == JSONKey::Detail) {
      QTextEdit* detailEditWidget = qobject_cast<QTextEdit*>(freqJsonKeyValue[JSONKey::Detail]);
      if (not detailEditWidget->textCursor().hasSelection()) {
        continue;
      }
      const QString& before = detailEditWidget->textCursor().selection().toPlainText();
      detailEditWidget->textCursor().removeSelectedText();
      const QString& after = capitalizeEachWord(before);
      detailEditWidget->textCursor().insertText(after);
      continue;
    }
    QLineEdit* lineEditWidget = qobject_cast<QLineEdit*>(freqJsonKeyValue[keyName]);
    if (not lineEditWidget->hasSelectedText()) {
      continue;
    }
    const QString& before = lineEditWidget->selectedText();
    const QString& after = capitalizeEachWord(before);
    const QString& beforeFullText = lineEditWidget->text();
    const QString& afterFullText =
        QString("%1%2%3").arg(beforeFullText.left(lineEditWidget->selectionStart()), after, beforeFullText.mid(lineEditWidget->selectionEnd()));
    lineEditWidget->setText(afterFullText);
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

  static PerformersStringParser& pm = PerformersStringParser::getIns();
  const int newLearnedCnt = pm.LearningFromAPath(loadFromFi.absoluteFilePath());

  static ProductionStudioManager& psm = ProductionStudioManager::getIns();
  const int newLearnedProdStudioCnt = psm.LearningFromAPath(loadFromFi.absoluteFilePath());

  QMessageBox::information(
      this, "Learning succeed",
      QString("New Learned:\n Performers Count:%1\n Production Studios Count:%2").arg(newLearnedCnt).arg(newLearnedProdStudioCnt));

  return newLearnedCnt >= 0 or newLearnedProdStudioCnt >= 0;
}

QStringList JsonEditor::onPerformersHint() {
  static PerformersStringParser& pm = PerformersStringParser::getIns();
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
    beforePerfsList = p->text().split(JSON_RENAME_REGEX::SEPERATOR_COMP);
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

bool JsonEditor::onSelectedTextAppendToPerformers() {
  static PerformersStringParser& pm = PerformersStringParser::getIns();

  if (not jsonKeySetMet.contains(JSONKey::Performers)) {
    jsonKeySetMet.insert(JSONKey::Performers);
  }

  QStringList perfs;
  auto* p = qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Performers]);
  if (not p->text().isEmpty()) {
    perfs += p->text().split(JSON_RENAME_REGEX::SEPERATOR_COMP);
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

bool JsonEditor::formatter() {
  if (jsonKeySetMet.contains(JSONKey::Performers)) {
    auto* lineWidget = qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Performers]);
    QString valueStr = lineWidget->text();
    valueStr.replace(JSON_RENAME_REGEX::SEPERATOR_COMP, ", ");
    lineWidget->setText(valueStr.trimmed());
  }
  if (jsonKeySetMet.contains(JSONKey::Tags)) {
    auto* lineWidget = qobject_cast<QLineEdit*>(freqJsonKeyValue[JSONKey::Tags]);
    QString valueStr = lineWidget->text();
    valueStr.replace(JSON_RENAME_REGEX::SEPERATOR_COMP, ", ");
    lineWidget->setText(valueStr.trimmed());
  }
}

bool JsonEditor::load(const QString& path) {
  if (not QDir(path).exists()) {
    return false;
  }
  QDirIterator it(path, {"*.json"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    if (m_jsonList->findItems(it.filePath(), Qt::MatchFlag::MatchExactly).isEmpty()) {
      m_jsonList->addItem(it.filePath());
    }
  }
  setWindowTitle(TITLE_TEMPLATE.arg(0).arg(m_jsonList->count()));
  if (m_jsonList->count() == 0) {
    return true;
  }
  m_jsonList->setCurrentRow(0);
  return true;
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  JsonEditor jsonEditor;
  jsonEditor.show();
  jsonEditor.load("../FileExplorerReadOnly/bin/JsonExample");
  a.exec();
  return 0;
}
#endif

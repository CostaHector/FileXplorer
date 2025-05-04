#include "JsonEditor.h"

#include "Actions/JsonEditorActions.h"
#include "Component/Notificator.h"

#include "public/MemoryKey.h"
#include "public/PublicMacro.h"

#include "Tools/FileDescriptor/TableFields.h"
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
      m_jsonFormLo{new QFormLayout},
      m_jsonFormWid{new QWidget(this)},

      m_jsonModel{new JsonModel{this}},
      m_jsonList(new JsonListView{m_jsonModel, this}),

      m_menuBar{g_jsonEditorActions().GetJsonMenuBar(this)},
      m_toolBar{g_jsonEditorActions().GetJsonToolBar(this)},
      m_splitter{new (std::nothrow) QSplitter{Qt::Orientation::Horizontal, this}} {
  setMenuBar(m_menuBar);
  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolBar);

  mName = new LineEditStr{JSON_KEY::NameS, "", this};
  mPerfsCsv = new LineEditCSV{JSON_KEY::PerformersS, "", this};  // comma seperated
  mStudio = new LineEditStr{JSON_KEY::StudioS, "", this};
  mUploaded = new LineEditStr{JSON_KEY::UploadedS, "", this};
  mTagsCsv = new LineEditCSV{JSON_KEY::TagsS, "", this};  // comma seperated
  mRateInt = new LineEditInt{JSON_KEY::RateS, "", this};  // int
  mSize = new LineEditStr{JSON_KEY::SizeS, "", this};
  mResolution = new LineEditStr{JSON_KEY::ResolutionS, "", this};
  mBitrate = new LineEditStr{JSON_KEY::BitrateS, "", this};
  mHot = new LineEditCSV{JSON_KEY::HotS, "", this};              // QList<QVariant>
  mDetail = new TextEditMultiLine{JSON_KEY::DetailS, "", this};  // multi-line

  m_jsonFormLo->addRow(mName->GetFormName(), mName);
  m_jsonFormLo->addRow(mPerfsCsv->GetFormName(), mPerfsCsv);
  m_jsonFormLo->addRow(mStudio->GetFormName(), mStudio);
  m_jsonFormLo->addRow(mUploaded->GetFormName(), mUploaded);
  m_jsonFormLo->addRow(mTagsCsv->GetFormName(), mTagsCsv);
  m_jsonFormLo->addRow(mRateInt->GetFormName(), mRateInt);
  m_jsonFormLo->addRow(mSize->GetFormName(), mSize);
  m_jsonFormLo->addRow(mResolution->GetFormName(), mResolution);
  m_jsonFormLo->addRow(mBitrate->GetFormName(), mBitrate);
  m_jsonFormLo->addRow(mHot->GetFormName(), mHot);
  m_jsonFormLo->addRow(mDetail->GetFormName(), mDetail);

  m_jsonFormLo->setLabelAlignment(Qt::AlignmentFlag::AlignRight);
  m_jsonFormLo->setRowWrapPolicy(QFormLayout::RowWrapPolicy::WrapLongRows);

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
  if (!curIndex.isValid()) {
    qWarning("Current index invalid");
    return;
  }

  const int rowIdx = curIndex.row();
  const QString& jAbsPth = m_jsonList->filePath(rowIdx);
  const QVariantHash& jsonDict = JsonFileHelper::MovieJsonLoader(jAbsPth);
  const int newCnt = jsonDict[JSON_KEY::PerformersS].toStringList().size();
  m_jsonModel->updatePerfCount(rowIdx, newCnt);
  qDebug("Now json[%s] perfs cnt:%d", qPrintable(jAbsPth), newCnt);

  mName->setText(jsonDict[JSON_KEY::NameS].toString());
  mPerfsCsv->ReadFromStringList(jsonDict[JSON_KEY::PerformersS].toStringList());
  mStudio->setText(jsonDict[JSON_KEY::StudioS].toString());
  mUploaded->setText(jsonDict[JSON_KEY::UploadedS].toString());
  mTagsCsv->ReadFromStringList(jsonDict[JSON_KEY::TagsS].toStringList());
  mRateInt->ReadFromInt(jsonDict[JSON_KEY::RateS].toInt());
  mSize->setText(jsonDict[JSON_KEY::SizeS].toString());
  mResolution->setText(jsonDict[JSON_KEY::ResolutionS].toString());
  mBitrate->setText(jsonDict[JSON_KEY::BitrateS].toString());
  mHot->ReadFromVariantList(jsonDict[JSON_KEY::HotS].toList());
  mDetail->setText(jsonDict[JSON_KEY::DetailS].toString());
  // todo: user determine save or not
  m_jsonFormLo->itemAt(0, QFormLayout::ItemRole::FieldRole)->widget()->setFocus();
}

void JsonEditor::subscribe() {
  connect(m_jsonList->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &JsonEditor::refreshEditPanel);

  connect(g_jsonEditorActions()._FORMATTER, &QAction::triggered, this, &JsonEditor::formatter);
  connect(g_jsonEditorActions()._RELOAD_JSON_FROM_FROM_DISK, &QAction::triggered, this, [this]() { refreshEditPanel(m_jsonList->currentIndex()); });

  connect(g_jsonEditorActions()._NEXT_FILE, &QAction::triggered, m_jsonList, &JsonListView::onNext);
  connect(g_jsonEditorActions()._LAST_FILE, &QAction::triggered, m_jsonList, &JsonListView::onLast);
  connect(g_jsonEditorActions()._DONE_AND_NEXT, &QAction::triggered, this, &JsonEditor::onSaveAndNextUnfinishedItem);
  connect(g_jsonEditorActions()._COMPLETE_PERFS_COUNT, &QAction::triggered, m_jsonList, &JsonListView::onSetPerfCount);

  connect(g_jsonEditorActions()._SAVE, &QAction::triggered, this, &JsonEditor::onStageChanges);

  connect(g_jsonEditorActions()._ADD_SELECTED_PERFORMER, &QAction::triggered, this, &JsonEditor::onSelectedTextAppendToPerformers);
  connect(g_jsonEditorActions()._EXTRACT_CAPITALIZED_PERFORMER, &QAction::triggered, this, &JsonEditor::onExtractCapitalizedPerformersHint);

  connect(g_jsonEditorActions()._LOWER_ALL_WORDS, &QAction::triggered, this, &JsonEditor::onLowercaseEachWord);
  connect(g_jsonEditorActions()._CAPITALIZE_FIRST_LETTER_OF_EACH_WORD, &QAction::triggered, this, &JsonEditor::onCapitalizeEachWord);
  connect(g_jsonEditorActions()._LEARN_PERFORMERS_FROM_JSON, &QAction::triggered, this, &JsonEditor::onLearnPerfomersFromJsonFile);
  connect(g_jsonEditorActions()._AI_HINT, &QAction::triggered, this, &JsonEditor::onPerformersHint);

  connect(g_jsonEditorActions()._RENAME_THIS_FILE, &QAction::triggered, this, &JsonEditor::onRenameJsonFile);
}

int JsonEditor::operator()(const QString& folderPath) {
  QString loadFromPath = folderPath;
  if (folderPath.isEmpty()) {
    const QString& defaultOpenDir = PreferenceSettings().value(MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.name, MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.v).toString();
    loadFromPath = QFileDialog::getExistingDirectory(this, "Learn From", defaultOpenDir);
  }
  QFileInfo loadFromFi(loadFromPath);
  if (!loadFromFi.isDir()) {
    QMessageBox::warning(this, "Failed when Load json from a folder", QString("Not a folder:\n%1").arg(folderPath));
    qWarning("Failed when Load json from a folder. Not a folder:\n%s", qPrintable(folderPath));
    return 0;
  }
  PreferenceSettings().setValue(MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.name, loadFromFi.absoluteFilePath());
  return load(loadFromFi.absoluteFilePath());
}

bool JsonEditor::onStageChanges() {
  const int curRow = m_jsonList->currentRow();
  if (curRow < 0 || curRow >= m_jsonList->count()) {
    qWarning("Cannot save, curRow:%d out of bound [0, %d)", curRow, m_jsonList->count());
    return true;
  }

  const QStringList& perfs = mPerfsCsv->GetStringList();
  m_jsonModel->updatePerfCount(curRow, perfs.size());

  QVariantHash dict;
  dict[mName->GetFormName()] = mName->text();
  dict[mPerfsCsv->GetFormName()] = perfs;
  dict[mStudio->GetFormName()] = mStudio->text();
  dict[mUploaded->GetFormName()] = mUploaded->text();
  dict[mTagsCsv->GetFormName()] = mTagsCsv->GetStringList();
  dict[mRateInt->GetFormName()] = mRateInt->GetIntValue();
  dict[mSize->GetFormName()] = mSize->text();
  dict[mResolution->GetFormName()] = mResolution->text();
  dict[mBitrate->GetFormName()] = mBitrate->text();
  dict[mHot->GetFormName()] = mHot->GetVariantList();
  dict[mDetail->GetFormName()] = mDetail->toPlainText();

  const QString& curJsonPath = m_jsonList->filePath(curRow);
  return JsonFileHelper::DumpJsonDict(dict, curJsonPath);
}

bool JsonEditor::onSaveAndNextUnfinishedItem() {
  const bool isSavedSucceed = onStageChanges();
  if (not isSavedSucceed) {
    return false;
  }
  m_jsonList->onNext();
  return true;
}

auto JsonEditor::onLowercaseEachWord() -> void {
  StringEditHelper::ReplaceAndUpdateSelection(mName, StringEditHelper::lowercaseSentense);
  StringEditHelper::ReplaceAndUpdateSelection(mPerfsCsv, StringEditHelper::lowercaseSentense);
  StringEditHelper::ReplaceAndUpdateSelection(mStudio, StringEditHelper::lowercaseSentense);
  StringEditHelper::ReplaceAndUpdateSelection(mUploaded, StringEditHelper::lowercaseSentense);
  StringEditHelper::ReplaceAndUpdateSelection(mTagsCsv, StringEditHelper::lowercaseSentense);
  StringEditHelper::ReplaceAndUpdateSelection(mRateInt, StringEditHelper::lowercaseSentense);
  StringEditHelper::ReplaceAndUpdateSelection(mSize, StringEditHelper::lowercaseSentense);
  StringEditHelper::ReplaceAndUpdateSelection(mResolution, StringEditHelper::lowercaseSentense);
  StringEditHelper::ReplaceAndUpdateSelection(mBitrate, StringEditHelper::lowercaseSentense);
  StringEditHelper::ReplaceAndUpdateSelection(mHot, StringEditHelper::lowercaseSentense);
  StringEditHelper::ReplaceAndUpdateSelection(mDetail, StringEditHelper::lowercaseSentense);
}

auto JsonEditor::onCapitalizeEachWord() -> void {
  StringEditHelper::ReplaceAndUpdateSelection(mName, StringEditHelper::capitalizeEachWord);
  StringEditHelper::ReplaceAndUpdateSelection(mPerfsCsv, StringEditHelper::capitalizeEachWord);
  StringEditHelper::ReplaceAndUpdateSelection(mStudio, StringEditHelper::capitalizeEachWord);
  StringEditHelper::ReplaceAndUpdateSelection(mUploaded, StringEditHelper::capitalizeEachWord);
  StringEditHelper::ReplaceAndUpdateSelection(mTagsCsv, StringEditHelper::capitalizeEachWord);
  StringEditHelper::ReplaceAndUpdateSelection(mRateInt, StringEditHelper::capitalizeEachWord);
  StringEditHelper::ReplaceAndUpdateSelection(mSize, StringEditHelper::capitalizeEachWord);
  StringEditHelper::ReplaceAndUpdateSelection(mResolution, StringEditHelper::capitalizeEachWord);
  StringEditHelper::ReplaceAndUpdateSelection(mBitrate, StringEditHelper::capitalizeEachWord);
  StringEditHelper::ReplaceAndUpdateSelection(mHot, StringEditHelper::capitalizeEachWord);
  StringEditHelper::ReplaceAndUpdateSelection(mDetail, StringEditHelper::capitalizeEachWord);
}

bool JsonEditor::onLearnPerfomersFromJsonFile() {
  const QString& loadFromDefaultPath = PreferenceSettings().value(MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.name, MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.v).toString();
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

  QMessageBox::information(this, "Learning succeed", QString("New Learned:\n Performers Count:%1\n Production Studios Count:%2").arg(newLearnedCnt).arg(newLearnedProdStudioCnt));

  return newLearnedCnt >= 0 or newLearnedProdStudioCnt >= 0;
}

bool JsonEditor::onPerformersHint() {
  const int curRow = m_jsonList->currentRow();
  if (curRow < 0 || curRow >= m_jsonList->count()) {
    qWarning("Cannot Hint, curRow:%d out of bound [0, %d)", curRow, m_jsonList->count());
    return false;
  }

  const QString& nameText = mName->text();
  QString sentence = nameText;

  if (mDetail->textCursor().hasSelection()) {
    sentence += " " + mDetail->textCursor().selection().toPlainText();
  }
  static PerformersManager& pm = PerformersManager::getIns();
  const QStringList& hintPerfsList = pm(sentence);
  const int newPerfsCnt = mPerfsCsv->AppendFromStringList(hintPerfsList);

  static ProductionStudioManager& psm = ProductionStudioManager::getIns();
  const QString& newProdStudioName = psm.hintStdStudioName(nameText);
  if (!newProdStudioName.isEmpty()) {
    mStudio->setText(newProdStudioName);
  }

  m_jsonModel->updatePerfCount(curRow, newPerfsCnt);
  return true;
}

auto JsonEditor::onExtractCapitalizedPerformersHint() -> bool {
  QStringList hintPerfs;
  if (mName->hasSelectedText()) {
    const QString& capitalizedStr = mName->selectedText();
    hintPerfs += NameTool().fromArticleCapitalizedNames(capitalizedStr);
  }
  if (mDetail->textCursor().hasSelection()) {
    const QString& capitalizedStr = mDetail->textCursor().selection().toPlainText();
    hintPerfs += NameTool().fromArticleCapitalizedNames(capitalizedStr);
  }
  mPerfsCsv->AppendFromStringList(hintPerfs);
  return true;
}

bool JsonEditor::onSelectedTextAppendToPerformers() {
  QStringList selectedPerfs;

  if (mName->hasSelectedText()) {
    selectedPerfs += mName->selectedText();
  }

  if (mDetail->textCursor().hasSelection()) {
    selectedPerfs += mDetail->textCursor().selection().toPlainText();
  }

  mPerfsCsv->AppendFromStringList(selectedPerfs);
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
  mPerfsCsv->Format();
  mTagsCsv->Format();
  return true;
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
  jsonEditor.load("./bin/JsonExample");
  a.exec();
  return 0;
}
#endif

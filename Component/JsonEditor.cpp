#include "JsonEditor.h"
#include "Actions/JsonEditorActions.h"
#include "Tools/JsonFileHelper.h"

#include <QDesktopServices>
#include <QDirIterator>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QToolBar>

const QString JsonEditor::TITLE_TEMPLATE = "Json Editor [%1/%2]";
const QMap<QString, QString> JsonEditor::key2ValueType = {{"Performers", "QStringList"}, {"Tags", "QStringList"}, {"Rate", "int"}};

const QColor JsonEditor::MEET_CONDITION_COLOR(150, 150, 150);
const QColor JsonEditor::NOT_MEET_CONDITION_COLOR(255, 0, 0);

JsonEditor::JsonEditor(QWidget* parent)
    : QMainWindow{parent}, jsonListPanel(new QListWidget), editorPanel(new QFormLayout), jsonListPanelMenu(getListPanelRightClickMenu()) {
  QToolBar* tb = new QToolBar("json editor actions", this);
  tb->addActions(g_jsonEditorActions().JSON_EDITOR_ACTIONS->actions());
  tb->addSeparator();
  tb->addAction(g_jsonEditorActions()._REVEAL_IN_EXPLORER);
  tb->addAction(g_jsonEditorActions()._OPEN_THIS_FILE);
  tb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  addToolBar(Qt::ToolBarArea::TopToolBarArea, tb);

  auto* mainLo = new QHBoxLayout;
  mainLo->addLayout(editorPanel);
  mainLo->addWidget(jsonListPanel);

  QWidget* mainWidget = new QWidget;
  mainWidget->setLayout(mainLo);
  setCentralWidget(mainWidget);

  subscribe();
  setWindowTitle(TITLE_TEMPLATE);
  setWindowIcon(QIcon(":/themes/SHOW_FOLDER_PREVIEW_JSON_EDITOR"));

  mainWidget->setStyleSheet(QString("QTextEdit {font-size: %1pt} \r\n QLineEdit {font-size: %1pt};").arg(14));
  jsonListPanel->setContextMenuPolicy(Qt::CustomContextMenu);
}

auto JsonEditor::getListPanelRightClickMenu() -> QMenu* {
  auto* menu = new QMenu("json list right click menu", this);
  menu->addAction(g_jsonEditorActions()._REVEAL_IN_EXPLORER);
  menu->addAction(g_jsonEditorActions()._OPEN_THIS_FILE);
  return menu;
}

bool JsonEditor::hasLast() const {
  const auto curRow = jsonListPanel->currentRow();
  return curRow - 1 >= 0;
}

void JsonEditor::last() {
  const auto curRow = jsonListPanel->currentRow() - 1;
  jsonListPanel->setCurrentRow(curRow);
  qDebug("last curRow %d", curRow);
}

bool JsonEditor::hasNext() const {
  const auto curRow = jsonListPanel->currentRow();
  return curRow + 1 < jsonListPanel->count();
}

void JsonEditor::next() {
  const auto curRow = jsonListPanel->currentRow() + 1;
  jsonListPanel->setCurrentRow(curRow);
  qDebug("next curRow %d", curRow);
}

void JsonEditor::autoNext() {
  const auto reverseCondition = g_jsonEditorActions()._CONDITION_NOT->isChecked();
  for (auto curRow = jsonListPanel->currentRow() + 1; curRow < jsonListPanel->count(); ++curRow) {
    const QString& curJsonPath = jsonListPanel->item(curRow)->text();
    const auto& dict = JsonFileHelper::MovieJsonLoader(curJsonPath);
    if (dict.isEmpty()) {
      continue;
    }
    qDebug("len(performers)=%d, [%s]", dict["Performers"].toArray().size(), curJsonPath.toStdString().c_str());
    if ((dict["Performers"].toArray().size() == 0) == not reverseCondition) {
      jsonListPanel->item(curRow)->setForeground(NOT_MEET_CONDITION_COLOR);
      jsonListPanel->setCurrentRow(curRow);
      return;
    }
    jsonListPanel->item(curRow)->setForeground(MEET_CONDITION_COLOR);
  }
  if (jsonListPanel->count() > 0) {
    jsonListPanel->setCurrentRow(jsonListPanel->count() - 1);
  }
}

void JsonEditor::refreshEditPanel() {
  const auto curRow = jsonListPanel->currentRow();
  const QString& curJsonPath = jsonListPanel->item(curRow)->text();
  qDebug("refreshEditPanel %s", curJsonPath.toStdString().c_str());
  const auto& jsonDict = JsonFileHelper::MovieJsonLoader(curJsonPath);
  const QList<QPair<QString, QJsonValue>>& jsonItem = JsonFileHelper::MapToOrderedList(jsonDict);

  for (int r = editorPanel->rowCount() - 1; r > -1; r--) {
    editorPanel->removeRow(r);
  }
  if (jsonItem.isEmpty()) {
    return;
  }
  for (auto it = jsonItem.cbegin(); it != jsonItem.cend(); ++it) {
    const QString& keyName = it->first;
    const QJsonValue& v = it->second;
    if (not key2ValueType.contains(keyName)) {
      if (not v.isString()) {
        qDebug("%s is not string", keyName.toStdString().c_str());
        continue;
      }
      if (keyName == "Detail") {
        editorPanel->addRow(keyName, new QTextEdit(v.toString()));
        continue;
      }
      editorPanel->addRow(keyName, new QLineEdit(v.toString()));
      continue;
    }
    if (key2ValueType[keyName] == "QStringList") {
      if (not v.isArray()) {
        qDebug("%s is not array", keyName.toStdString().c_str());
        continue;
      }
      QStringList sl;
      for (const auto& element : v.toArray()) {
        sl.append(element.toString());
      }
      editorPanel->addRow(keyName, new QLineEdit(sl.join(", ")));
      continue;
    }
    if (key2ValueType[keyName] == "int") {
      if (not v.isDouble()) {
        qDebug("%s is not double", keyName.toStdString().c_str());
        continue;
      }
      editorPanel->addRow(keyName, new QLineEdit(QString::number(static_cast<int>(v.toDouble()))));
    }
  }
  qobject_cast<QLineEdit*>(editorPanel->itemAt(0, QFormLayout::ItemRole::FieldRole)->widget())->setFocus();
}

void JsonEditor::subscribe() {
  connect(jsonListPanel, &QListWidget::itemSelectionChanged, this, &JsonEditor::refreshEditPanel);
  connect(jsonListPanel, &QListWidget::itemDoubleClicked, this,
          [this](const QListWidgetItem* item) { QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(item->text()).absolutePath())); });

  auto ShowContextMenu = [this](const QPoint pnt) {
    jsonListPanelMenu->popup(jsonListPanel->mapToGlobal(pnt));  // or QCursor::pos()
  };
  connect(jsonListPanel, &QListView::customContextMenuRequested, ShowContextMenu);

  connect(g_jsonEditorActions()._REVEAL_IN_EXPLORER, &QAction::triggered, this,
          [this]() { QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(jsonListPanel->currentItem()->text()).absolutePath())); });
  connect(g_jsonEditorActions()._OPEN_THIS_FILE, &QAction::triggered, this,
          [this]() { QDesktopServices::openUrl(QUrl::fromLocalFile(jsonListPanel->currentItem()->text())); });

  connect(g_jsonEditorActions()._FORMATTER, &QAction::triggered, this, &JsonEditor::formatter);
  connect(g_jsonEditorActions()._RELOAD_FROM_JSON_FILE, &QAction::triggered, this, &JsonEditor::refreshEditPanel);

  connect(g_jsonEditorActions()._NEXT, &QAction::triggered, this, [this]() {
    if (g_jsonEditorActions()._AUTO_SKIP->isChecked()) {
      autoNext();
      return;
    }
    if (hasNext()) {
      next();
    }
  });

  connect(g_jsonEditorActions()._LAST, &QAction::triggered, this, [this]() {
    if (hasLast()) {
      last();
    }
  });

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

  connect(g_jsonEditorActions()._LOAD, &QAction::triggered, this, [this]() {
    const QString& loadFromDefaultPath =
        PreferenceSettings().value(MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.name, MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.v).toString();
    const auto loadFromPath = QFileDialog::getExistingDirectory(this, "Choose location", loadFromDefaultPath);
    QFileInfo loadFromFi(loadFromPath);
    if (not loadFromFi.isDir()) {
      return "";
    }
    PreferenceSettings().setValue(MemoryKey::PATH_JSON_EDITOR_LOAD_FROM.name, loadFromFi.absoluteFilePath());
    load(loadFromFi.absoluteFilePath());
  });
  connect(g_jsonEditorActions()._EMPTY, &QAction::triggered, this->jsonListPanel, &QListWidget::clear);
}

bool JsonEditor::onStageChanges() {
  QVariantMap dict;
  for (auto r = 0; r != editorPanel->rowCount(); ++r) {
    const QString& keyName = qobject_cast<QLabel*>(editorPanel->itemAt(r, QFormLayout::ItemRole::LabelRole)->widget())->text();
    if (not key2ValueType.contains(keyName)) {
      if (keyName == "Detail") {
        const QString& valueStr = qobject_cast<QTextEdit*>(editorPanel->itemAt(r, QFormLayout::ItemRole::FieldRole)->widget())->toHtml();
        dict.insert(keyName, valueStr);
        continue;
      }
      const QString& valueStr = qobject_cast<QLineEdit*>(editorPanel->itemAt(r, QFormLayout::ItemRole::FieldRole)->widget())->text();
      dict.insert(keyName, valueStr);
      continue;
    }
    const QString& valueStr = qobject_cast<QLineEdit*>(editorPanel->itemAt(r, QFormLayout::ItemRole::FieldRole)->widget())->text();
    if (keyName == "Performers" or keyName == "Tags") {
      QJsonArray arr;
      if (not valueStr.isEmpty()) {
        for (const QString& s : valueStr.split(sepComp)) {
          arr << s;
        }
      }
      dict.insert(keyName, arr);
      continue;
    }
    if (keyName == "Rate") {
      bool isOk = false;
      int rateNumer = valueStr.toInt(&isOk);
      if (not isOk) {
        qDebug("Rate %s is not a number", valueStr.toStdString().c_str());
        continue;
      }
      dict.insert(keyName, rateNumer);
      continue;
    }
  }

  const auto curRow = jsonListPanel->currentRow();
  const QString& curJsonPath = jsonListPanel->item(curRow)->text();
  const QString& backupJsonPath = getBackupJsonFile(curJsonPath);
  if (QFile::exists(backupJsonPath)) {
    const auto rmRet = QFile::remove(backupJsonPath);
    if (not rmRet) {
      qDebug("cannot rm json file[%s]", backupJsonPath.toStdString().c_str());
      return false;
    }
  }
  const auto copyRet = QFile::copy(curJsonPath, backupJsonPath);

  qDebug("result:%d, changes->%s, backup: %s", int(copyRet), curJsonPath.toStdString().c_str(), backupJsonPath.toStdString().c_str());
  if (not copyRet) {
    qDebug("cannot copy json file[%s]", backupJsonPath.toStdString().c_str());
    return false;
  }
  return JsonFileHelper::MovieJsonDumper(dict, curJsonPath);
}

bool JsonEditor::onResetChanges() {
  const auto curRow = jsonListPanel->currentRow();
  const QString& curJsonPath = jsonListPanel->item(curRow)->text();
  const QString& backupJsonPath = getBackupJsonFile(curJsonPath);
  if (not QFile::exists(backupJsonPath)) {
    qDebug("cannot reset. backup file[%s] not exist", backupJsonPath.toStdString().c_str());
    return false;
  }
  return QFile::rename(curJsonPath, curJsonPath + "mv") and QFile::rename(backupJsonPath, curJsonPath) and
         QFile::rename(curJsonPath + "mv", backupJsonPath);
}

bool JsonEditor::onSubmitAllChanges() {
  int succeedCnt = 0;
  int failCnt = 0;
  for (auto r = 0; r < jsonListPanel->count(); ++r) {
    const QString& curJsonPath = jsonListPanel->item(r)->text();
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

bool JsonEditor::formatter() {
  for (auto r = 0; r != editorPanel->rowCount(); ++r) {
    const QString& keyName = qobject_cast<QLabel*>(editorPanel->itemAt(r, QFormLayout::ItemRole::LabelRole)->widget())->text();
    if (keyName == "Performers" or keyName == "Tags") {
      auto* lineWidget = qobject_cast<QLineEdit*>(editorPanel->itemAt(r, QFormLayout::ItemRole::FieldRole)->widget());
      QString keyValue = lineWidget->text();
      keyValue.replace(sepComp, ", ");
      lineWidget->setText(keyValue);
    }
  }
}

bool JsonEditor::load(const QString& path) {
  if (not QDir(path).exists()) {
    return false;
  }
  QDirIterator it(path, {"*.json"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    if (jsonListPanel->findItems(it.filePath(), Qt::MatchFlag::MatchExactly).isEmpty()) {
      jsonListPanel->addItem(it.filePath());
    }
  }
  setWindowTitle(TITLE_TEMPLATE.arg(0).arg(jsonListPanel->count()));
  if (jsonListPanel->count() == 0) {
    return true;
  }
  jsonListPanel->setCurrentRow(0);
  return true;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  JsonEditor jsonEditor;
  jsonEditor.show();
  jsonEditor.load("E:/115/test");
  a.exec();
  return 0;
}
#endif

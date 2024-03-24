#ifndef JSONEDITOR_H
#define JSONEDITOR_H

#include "PublicVariable.h"

#include <QFormLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QRegExp>
#include <QSplitter>
#include <QTextEdit>
#include <QWidget>

#include <QVariantMap>

#include "Model/JsonModel.h"
#include "View/JsonListView.h"

class JsonEditor : public QMainWindow {
 public:
  explicit JsonEditor(QWidget* parent = nullptr);

  auto load(const QString& path) -> int;
  auto refreshEditPanel(const QModelIndex& curIndex) -> void;

  auto currentJsonString() const -> QString;

  auto subscribe() -> void;

  auto onLoadASelectedPath(const QString& folderPath = "") -> int;

  auto onStageChanges() -> bool;
  bool onSaveAndNextUnfinishedItem();
  auto onResetChanges() -> bool;
  auto onSubmitAllChanges() -> bool;
  auto onLowercaseEachWord() -> void;
  auto onCapitalizeEachWord() -> void;
  auto onLearnPerfomersFromJsonFile() -> bool;
  auto onPerformersHint() -> QStringList;
  auto onExtractCapitalizedPerformersHint() -> bool;
  auto onSelectedTextAppendToPerformers() -> bool;

  bool onRenameJsonFile();
  void onEditPerformers();
  void onEditAkaPerformer();
  void onEditStudios();

  auto updateWindowsSize() -> void {
    if (PreferenceSettings().contains("JsonEditorGeometry")) {
      restoreGeometry(PreferenceSettings().value("JsonEditorGeometry").toByteArray());
    } else {
      setGeometry(QRect(0, 0, 600, 400));
    }
    m_editorAndListSplitter->restoreState(PreferenceSettings().value("JsonEditorSplitterState", QByteArray()).toByteArray());
  }

  auto closeEvent(QCloseEvent* event) -> void override {
    PreferenceSettings().setValue("JsonEditorGeometry", saveGeometry());
    qDebug("Json Editor geometry was resize to (%d, %d, %d, %d)", geometry().x(), geometry().y(), geometry().width(), geometry().height());
    PreferenceSettings().setValue("JsonEditorSplitterState", m_editorAndListSplitter->saveState());
    QMainWindow::closeEvent(event);
  }

 private:
  static inline auto getBackupJsonFile(const QString& origin) -> QString { return origin + ".bkp"; }

  auto formatter() -> bool;

  QHash<QString, QWidget*> freqJsonKeyValue;

  QFormLayout* m_editorPanel;
  QFormLayout* m_extraEditorPanel;  // no so frequently used key-value pair

  QWidget* m_editorWidget;

  JsonModel* m_jsonModel;
  JsonListView* m_jsonList;

  QMenuBar* m_menuBar;
  QToolBar* m_editorToolBar;

  QSplitter* m_editorAndListSplitter;

  QSet<QString> jsonKeySetMet;

  static const QString TITLE_TEMPLATE;

  static const QColor MEET_CONDITION_COLOR;
  static const QColor NOT_MEET_CONDITION_COLOR;
};

#endif  // JSONEDITOR_H

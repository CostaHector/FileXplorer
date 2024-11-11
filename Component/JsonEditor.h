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

  auto updateWindowsSize() -> void {
    if (PreferenceSettings().contains("JsonEditorGeometry")) {
      restoreGeometry(PreferenceSettings().value("JsonEditorGeometry").toByteArray());
    } else {
      setGeometry(QRect(0, 0, 600, 400));
    }
    m_splitter->restoreState(PreferenceSettings().value("JsonEditorSplitterState", QByteArray()).toByteArray());
  }

  auto closeEvent(QCloseEvent* event) -> void override {
    PreferenceSettings().setValue("JsonEditorGeometry", saveGeometry());
    qDebug("Json Editor geometry was resize to (%d, %d, %d, %d)", geometry().x(), geometry().y(), geometry().width(), geometry().height());
    PreferenceSettings().setValue("JsonEditorSplitterState", m_splitter->saveState());
    QMainWindow::closeEvent(event);
  }

 private:
  static inline auto getBackupJsonFile(const QString& origin) -> QString { return origin + ".bkp"; }
  bool IsValueOfKeyArr(const QString& key) const;
  QString GetArrLine(const QString& key) const;
  void UpdateDisplayArrLine(const QString& key, const QStringList& arr);

  auto formatter() -> bool;

  QHash<QString, QWidget*> m_stdKeys;

  QFormLayout* m_jsonFormLo;
  QFormLayout* m_jsonFormExtraLo;  // no so frequently used key-value pair

  QWidget* m_jsonFormWid;

  JsonModel* m_jsonModel;
  JsonListView* m_jsonList;

  QMenuBar* m_menuBar;
  QToolBar* m_toolBar;

  QSplitter* m_splitter;

  QSet<QString> m_keysMet;

  static const QString TITLE_TEMPLATE;
};

#endif  // JSONEDITOR_H

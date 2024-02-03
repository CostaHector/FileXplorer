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

class JsonEditor : public QMainWindow {
 public:
  explicit JsonEditor(QWidget* parent = nullptr);

  auto getListPanelRightClickMenu() -> QMenu*;

  auto hasLast() const -> bool;
  auto hasNext() const -> bool;

  auto last() -> void;
  auto next() -> void;

  auto onAutoSkipSwitch(const bool checked) -> void;
  void autoNext();

  auto onNext() -> void;
  auto onLast() -> void;

  auto load(const QString& path) -> bool;
  auto refreshEditPanel() -> void;

  auto currentJsonString() const -> QString;

  auto subscribe() -> void;

  auto onLoadASelectedPath(const QString& folderPath = "") -> bool;

  auto onStageChanges() -> bool;
  auto onResetChanges() -> bool;
  auto onSubmitAllChanges() -> bool;
  auto onLowercaseEachWord() -> void;
  auto onCapitalizeEachWord() -> void;
  auto onLearnPerfomersFromJsonFile() -> bool;
  auto onPerformersHint() -> QStringList;
  auto onSelectedTextAppendToPerformers() -> bool;

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

  QListWidget* m_jsonList;
  QMenu* m_listMenu;

  QSplitter* m_editorAndListSplitter;
  QToolBar* m_editorToolBar;

  QMenuBar* m_menuBar;

  QSet<QString> jsonKeySetMet;

  static const QString TITLE_TEMPLATE;

  static const QColor MEET_CONDITION_COLOR;
  static const QColor NOT_MEET_CONDITION_COLOR;
};

#endif  // JSONEDITOR_H

#ifndef JSONEDITOR_H
#define JSONEDITOR_H

#include "public/PublicVariable.h"

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

#include "JsonEditorWidget/LineEditCSV.h"
#include "JsonEditorWidget/LineEditInt.h"
#include "JsonEditorWidget/LineEditStr.h"
#include "JsonEditorWidget/TextEditMultiLine.h"

class JsonEditor : public QMainWindow {
 public:
  explicit JsonEditor(QWidget* parent = nullptr);

  auto load(const QString& path) -> int;
  auto refreshEditPanel(const QModelIndex& curIndex) -> void;

  auto currentJsonString() const -> QString;

  auto subscribe() -> void;

  int operator()(const QString& folderPath = "");

  auto onStageChanges() -> bool;
  bool onSaveAndNextUnfinishedItem();
  auto onLowercaseEachWord() -> void;
  auto onCapitalizeEachWord() -> void;
  auto onLearnPerfomersFromJsonFile() -> bool;
  bool onPerformersHint();
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
  bool formatter();

  LineEditStr* mName{nullptr};
  LineEditCSV* mPerfsCsv{nullptr};  // comma seperated
  LineEditStr* mStudio{nullptr};
  LineEditStr* mUploaded{nullptr};
  LineEditCSV* mTagsCsv{nullptr};  // comma seperated
  LineEditInt* mRateInt{nullptr}; // int
  LineEditStr* mSize{nullptr};
  LineEditStr* mResolution{nullptr};
  LineEditStr* mBitrate{nullptr};
  LineEditCSV* mHot{nullptr};           // QList<QVariant>
  TextEditMultiLine* mDetail{nullptr};  // multi-line

  QFormLayout* m_jsonFormLo;

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

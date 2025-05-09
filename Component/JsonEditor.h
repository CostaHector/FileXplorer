#ifndef JSONEDITOR_H
#define JSONEDITOR_H

#include <QMainWindow>
#include <QFormLayout>
#include <QSplitter>

#include "Model/JsonModel.h"
#include "View/JsonListView.h"

#include "JsonEditorWidget/LineEditCSV.h"
#include "JsonEditorWidget/LineEditInt.h"
#include "JsonEditorWidget/LineEditStr.h"
#include "JsonEditorWidget/TextEditMultiLine.h"

class JsonEditor : public QMainWindow {
 public:
  explicit JsonEditor(QWidget* parent = nullptr);
  void updateWindowsSize();
  void closeEvent(QCloseEvent* event) override;

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

 private:
  bool formatter();

  LineEditStr* mName{nullptr};
  LineEditCSV* mPerfsCsv{nullptr};  // comma seperated
  LineEditStr* mStudio{nullptr};
  LineEditStr* mUploaded{nullptr};
  LineEditCSV* mTagsCsv{nullptr};  // comma seperated
  LineEditInt* mRateInt{nullptr}; // int
  LineEditInt* mSize{nullptr};
  LineEditStr* mResolution{nullptr};
  LineEditStr* mBitrate{nullptr};
  LineEditCSV* mHot{nullptr};           // QList<QVariant>
  TextEditMultiLine* mDetail{nullptr};  // multi-line

  QFormLayout* m_jsonFormLo;

  QWidget* m_jsonFormWid;

  JsonModel* m_jsonModel;
  JsonListView* m_jsonList;

  QToolBar* m_toolBar;

  QSplitter* m_splitter;

  QSet<QString> m_keysMet;

  static const QString TITLE_TEMPLATE;
};

#endif  // JSONEDITOR_H

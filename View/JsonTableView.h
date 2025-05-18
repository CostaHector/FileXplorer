#ifndef JSONVIEW_H
#define JSONVIEW_H

#include "CustomTableView.h"
#include "Model/JsonTableModel.h"
#include "Model/JsonProxyModel.h"
#include "Component/MultiLineEditDelegate.h"

class JsonTableView : public CustomTableView {
 public:
  explicit JsonTableView(JsonTableModel* jsonModel, JsonProxyModel* jsonProxyModel, QWidget* parent = nullptr);
  QModelIndex CurrentIndexSource() const;
  QModelIndexList selectedRowsSource(JSON_KEY_E column = JSON_KEY_E::Name) const;

  int ReadADirectory(const QString& path);
  void subscribe();

  int onSaveCurrentChanges();
  int onSyncNameField();
  int onExportCastStudioToDictonary();
  int onRenameJsonAndRelated();
  int onSetStudio();
  int onInitCastAndStudio();
  int onHintCastAndStudio();
  int onFormatCast();
  int onClearStudio();
  int onSetCastOrTags(const FIELD_OP_TYPE type, const FIELD_OP_MODE mode);
  int onAppendFromSelection(bool isUpperCaseSentence);
  int onSelectionCaseOperation(bool isTitle);

  enum class EDITOR_WIDGET_TYPE {
    LINE_EDIT = 0,
    PLAIN_TEXT_EDIT,
    TEXT_EDIT,
    BUTT,
  };

  bool GetSelectedTextInCell(QString& selectedText, EDITOR_WIDGET_TYPE& edtWidType) const;
 private:
  QStringList m_studioCandidates;
  QStringList m_candidatesLst[(int)FIELD_OP_TYPE::BUTT];
  JsonTableModel* _JsonModel{nullptr};
  JsonProxyModel* _JsonProxyModel{nullptr};
  MultiLineEditDelegate* m_DetailEdit{nullptr};
};

#endif  // JSONVIEW_H

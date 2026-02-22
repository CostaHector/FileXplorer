#ifndef JSONVIEW_H
#define JSONVIEW_H

#include "CustomTableView.h"
#include "JsonTableModel.h"
#include "MultiLineEditDelegate.h"
#include <QSortFilterProxyModel>

class JsonTableView : public CustomTableView {
  Q_OBJECT
 public:
  explicit JsonTableView(JsonTableModel* jsonModel, QSortFilterProxyModel* jsonProxyModel, QWidget* parent = nullptr);
  QModelIndex CurrentIndexSource() const;
  QModelIndexList selectedRowsSource(JSON_KEY_E column = JSON_KEY_E::Name) const;

  int ReadADirectory(const QString& path);
  void subscribe();

  int onFixSelectionRecordContents();
  int onSaveCurrentChanges();
  int onSyncNameField();
  int onExportCastStudioToDictonary();
  int onRenameJsonAndRelated();
  int onSetStudio();
  int onInitCastAndStudio();
  int onHintCastAndStudio();
  int onFormatCast();
  int onUpdateDuration();
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
 signals:
  void currentJsonSelectedChanged(const QString& name, const QString& jsonAbsFilePath, const QStringList& imgPthLst, const QStringList& vidsLst);

 private:
  void onSelectNewJsonLine(const QModelIndex &current);
  QStringList m_studioCandidates;
  QStringList m_candidatesLst[(int)FIELD_OP_TYPE::BUTT];
  JsonTableModel* _JsonModel{nullptr};
  QSortFilterProxyModel* _JsonProxyModel{nullptr};
  MultiLineEditDelegate* m_DetailEdit{nullptr};
  QMenu* m_jsonMenu{nullptr};
#ifdef RUNNING_UNIT_TESTS
  QWidget* pWidgetInCellMock{nullptr};
#endif
};

#endif  // JSONVIEW_H

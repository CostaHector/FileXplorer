#ifndef JSONVIEW_H
#define JSONVIEW_H

#include "CustomTableView.h"
#include "Model/JsonTableModel.h"
class JsonTableView: public CustomTableView
{
 public:
  explicit JsonTableView(JsonTableModel* _jsonModel, QWidget* parent = nullptr);
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
 private:
  QStringList m_studioCandidates;
  QStringList m_candidatesLst[(int)FIELD_OP_TYPE::BUTT];
  JsonTableModel* mJsonModel{nullptr};
};

#endif // JSONVIEW_H

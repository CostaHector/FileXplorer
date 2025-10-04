#ifndef JSONVIEW_H
#define JSONVIEW_H

#include "CustomTableView.h"
#include "JsonTableModel.h"
#include "MultiLineEditDelegate.h"
#include <QSortFilterProxyModel>

namespace JsonTableViewMock {
inline std::pair<bool, QString>& QryNewJsonBaseNameMock() {
  static std::pair<bool, QString> acceptJsonBaseNamePair;
  return acceptJsonBaseNamePair;
}
inline std::pair<bool, QString>& InputStudioNameMock() {
  static std::pair<bool, QString> accept2StudioName;
  return accept2StudioName;
}
inline bool& clearTagsOrCastsMock() {
  static bool bClearTagsOrCasts;
  return bClearTagsOrCasts;
}
inline std::pair<bool, QString>& InputTagsOrCastsMock() {
  static std::pair<bool, QString> accept2TagsOrCasts;
  return accept2TagsOrCasts;
}
inline void clear() {
  QryNewJsonBaseNameMock() = std::pair<bool, QString>(false, "");
  InputStudioNameMock() = std::pair<bool, QString>(false, "");
  clearTagsOrCastsMock() = false;
  InputTagsOrCastsMock() = std::pair<bool, QString>(false, "");
}
}  // namespace JsonTableViewMock

class JsonTableView : public CustomTableView {
 public:
  explicit JsonTableView(JsonTableModel* jsonModel, QSortFilterProxyModel* jsonProxyModel, QWidget* parent = nullptr);
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
  QSortFilterProxyModel* _JsonProxyModel{nullptr};
  MultiLineEditDelegate* m_DetailEdit{nullptr};
#ifdef RUNNING_UNIT_TESTS
  QWidget* pWidgetInCellMock{nullptr};
#endif
};

#endif  // JSONVIEW_H

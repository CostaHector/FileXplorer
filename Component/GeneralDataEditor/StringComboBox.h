#ifndef STRINGCOMBOBOX_H
#define STRINGCOMBOBOX_H

#include "GeneralComboBox.h"

class StringComboBox : public GeneralComboBox {
public:
  explicit StringComboBox(GeneralDataType::Type gDataType, QWidget *parent = nullptr);
  QVariant getSetDataEditRoleValue() const override;

  void updateCurrentTextFromEditRole(const QVariant &editRoleData) override;
  static QString displayTextFromDisplayRole(GeneralDataType::Type gDataType, const QVariant &displayRoleData);

private:
  using T_CANDIDATES_STR_LST = QStringList;
  const T_CANDIDATES_STR_LST *mCandidates{nullptr};

  static const T_CANDIDATES_STR_LST *GetCandidates(GeneralDataType::Type gDataType);
  static QString getDisplayString(const QVariant &variantData, const T_CANDIDATES_STR_LST &pCandidateDisp);
};

#endif // STRINGCOMBOBOX_H

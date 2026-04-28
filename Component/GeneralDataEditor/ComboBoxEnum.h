#ifndef COMBOBOXENUM_H
#define COMBOBOXENUM_H

#include "ComboBoxGeneral.h"

class ComboBoxEnum : public ComboBoxGeneral {
public:
  explicit ComboBoxEnum(GeneralDataType::Type gDataType, QWidget *parent = nullptr);
  QVariant getSetDataEditRoleValue() const override;

  void updateCurrentTextFromEditRole(const QVariant &editRoleData) override;
  static QString displayTextFromDisplayRole(GeneralDataType::Type gDataType, const QVariant &displayRoleData);

private:
  using T_CANDIDATES_STR_TO_ENUM = QMap<QString, int>;
  const T_CANDIDATES_STR_TO_ENUM *mCandidates{nullptr};

  using T_ENUM_TO_CANDIDATES_STR = QMap<int, QString>;
  const T_ENUM_TO_CANDIDATES_STR *mCandidatesDisplay{nullptr};

  using PAIR_TYPE = std::pair<const T_CANDIDATES_STR_TO_ENUM *, const T_ENUM_TO_CANDIDATES_STR *>;
  static PAIR_TYPE GetCandidates(GeneralDataType::Type gDataType);
  static QString getDisplayString(const QVariant &variantData, const T_ENUM_TO_CANDIDATES_STR &pCandidateDisp);
};

#endif // COMBOBOXENUM_H

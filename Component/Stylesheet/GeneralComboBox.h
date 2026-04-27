#ifndef GENERALCOMBOBOX_H
#define GENERALCOMBOBOX_H

#include "GeneralDataType.h"
#include <QComboBox>

class GeneralComboBox : public QComboBox {
public:
  explicit GeneralComboBox(GeneralDataType::Type gDataType, QWidget *parent = nullptr);
  GeneralDataType::Type type() const { return mDataType; }
  virtual QVariant getSetDataVariant() const = 0;
  virtual void updateCurrentDisplayString(const QVariant &editRoleData) { setCurrentText(editRoleData.toString()); }

  static GeneralComboBox *create(int generalDataType, QWidget *parent);

private:
  GeneralDataType::Type mDataType;
};

class EnumComboBox : public GeneralComboBox {
public:
  explicit EnumComboBox(GeneralDataType::Type gDataType, QWidget *parent = nullptr);

  QVariant getSetDataVariant() const override;

  void updateCurrentDisplayString(const QVariant &editRoleData) override;

private:
  using T_CANDIDATES_STR_TO_ENUM = QMap<QString, int>;
  const T_CANDIDATES_STR_TO_ENUM *mCandidates{nullptr};

  using T_ENUM_TO_CANDIDATES_STR = QMap<int, QString>;
  const T_ENUM_TO_CANDIDATES_STR *mCandidatesDisplay{nullptr};

  using PAIR_TYPE = std::pair<const T_CANDIDATES_STR_TO_ENUM *, const T_ENUM_TO_CANDIDATES_STR *>;
  static PAIR_TYPE GetCandidates(GeneralDataType::Type gDataType);
};

class StringComboBox : public GeneralComboBox {
public:
  explicit StringComboBox(GeneralDataType::Type gDataType, QWidget *parent = nullptr);
  QVariant getSetDataVariant() const override;

private:
  static const QStringList CANDIDATES_FONT_FAMILY;
  const QStringList *mCandidates;
};

#endif // GENERALCOMBOBOX_H

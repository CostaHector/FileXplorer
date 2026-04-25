#ifndef CONFIGSMODEL_H
#define CONFIGSMODEL_H

#include <QAbstractTableModel>
#include "MemoryKey.h"
#include "StyleKey.h"

namespace KVTestOnly {
using namespace RawVariant;
using namespace GeneralDataType;
using namespace ValueChecker;

constexpr KV playerFilePath{"playerFilePath", Var{"inexists/player/file/path"}, GeneralDataType::Type::FILE_PATH, ValueChecker::GeneralFilePathChecker};      // invalid
constexpr int FILE_PATH_ROW = 0;

constexpr KV workFolderPath{"workFolderPath", Var{"inexists/work/folder/path"}, GeneralDataType::Type::FOLDER_PATH, ValueChecker::GeneralFolderPathChecker};  // invalid
constexpr int FOLDER_PATH_ROW = 1;

constexpr KV volumeValue{"volumeValue", Var{-5}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>};                                           // invalid
constexpr int VOLUME_VALUE_ROW = 2;

constexpr KV playerMute{"playerMute", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr int PLATER_MUTE_ROW = 3;

constexpr int KV_TEST_COUNT = 4;
}  // namespace KVTestOnly

namespace EditableConfigs {
constexpr const KV* mKVs[]{
#ifdef RUNNING_UNIT_TESTS
    &KVTestOnly::playerFilePath,
    &KVTestOnly::workFolderPath,
    &KVTestOnly::volumeValue,
    &KVTestOnly::playerMute,
#else
    &PathKey::LAST_TIME_COPY_TO,            //
    &BehaviorKey::VIDS_LAST_TABLE_NAME,     //
    &BehaviorKey::WHERE_CLAUSE_HISTORY,     //
    &RedunImgFinderKey::RUND_IMG_PATH,      //
    &StyleKey::STYLE_PRESET,                //
    &StyleKey::STYLE_THEME,                 //
    &StyleKey::BACKGROUND_IMAGE,            //
    &StyleKey::BACKGROUND_OVERLAY_OPACITY,  //
#endif
};
constexpr const int EDTIABLE_ROWS_COUNT = sizeof(mKVs) / sizeof(mKVs[0]);
}

class ConfigsModel : public QAbstractTableModel {
 public:
  enum ColumnE {
    NAME = 0,
    INITIAL_VALUE = 1,
    CURRENT_VALUE = 2,
    VALIDATIDATION_VALUE = 3,
    DATA_TYPE_ROLE = Qt::UserRole + 1,
  };
  using QAbstractTableModel::QAbstractTableModel;

  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return EditableConfigs::EDTIABLE_ROWS_COUNT; }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return COLUMNS_COUNT; }
  int failCount() const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (orientation == Qt::Orientation::Horizontal) {
        return CONFIGS_TABLE_HEADER[section];
      }
      return section + 1;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
  }

  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (index.column() == EDITABLE_COLUMN) {
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEditable;
    }
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }

  bool isPath(const QModelIndex& index) const;
  QString filePath(const QModelIndex& index) const;

  static constexpr int EDITABLE_COLUMN = ColumnE::CURRENT_VALUE;

 private:
  static constexpr const char* CONFIGS_TABLE_HEADER[]{"Name", "Initial value", "Current value", "Validation"};
  static constexpr const int COLUMNS_COUNT = sizeof(CONFIGS_TABLE_HEADER) / sizeof(CONFIGS_TABLE_HEADER[0]);
};

#endif  // CONFIGSMODEL_H

#ifndef CONFIGSMODEL_H
#define CONFIGSMODEL_H

#include <QAbstractTableModel>
#include "MemoryKey.h"
#include "BehaviorKey.h"
#include "PathKey.h"
#include "RedunImgFinderKey.h"
#include "StyleKey.h"
#include <bitset>

namespace KVTestOnly {
using namespace RawVariant;
using namespace GeneralDataType;
using namespace ValueChecker;

constexpr KV PLAYER_FILE_PATH{"PLAYER_FILE_PATH", Var{"inexists/player/file/path"}, GeneralDataType::Type::FILE_PATH, ValueChecker::GeneralFilePathChecker}; // invalid
constexpr int FILE_PATH_ROW = 0;

constexpr KV WORK_FOLDER_PATH{"WORK_FOLDER_PATH", Var{"inexists/work/folder/path"}, GeneralDataType::Type::FOLDER_PATH, ValueChecker::GeneralFolderPathChecker}; // invalid
constexpr int FOLDER_PATH_ROW = 1;

constexpr KV VOLUME_VALUE{"VOLUME_VALUE", Var{-5}, GeneralDataType::Type::RANGE_INT, GeneralIntRangeChecker<0, 100>}; // invalid
constexpr int VOLUME_VALUE_ROW = 2;

constexpr KV PLAYER_MUTE{"PLAYER_MUTE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr int PLATER_MUTE_ROW = 3;

constexpr int KV_TEST_COUNT = 4;
} // namespace KVTestOnly

namespace EditableConfigs {
constexpr const KV* mKVs[]{
#ifdef RUNNING_UNIT_TESTS
    &KVTestOnly::PLAYER_FILE_PATH,
    &KVTestOnly::WORK_FOLDER_PATH,
    &KVTestOnly::VOLUME_VALUE,
    &KVTestOnly::PLAYER_MUTE,
#else
    &BehaviorKey::VIDS_LAST_TABLE_NAME,    //
    &BehaviorKey::WHERE_CLAUSE_HISTORY,    //
    &PathKey::LAST_TIME_COPY_TO,           //
    &RedunImgFinderKey::RUND_IMG_PATH,     //
    &StyleKey::STYLE_PRESET,               //
    &StyleKey::STYLE_THEME,                //
    &StyleKey::BACKGROUND_IMAGE,           //
    &StyleKey::BACKGROUND_OVERLAY_OPACITY, //
#endif
};
constexpr const int EDTIABLE_ROWS_COUNT = sizeof(mKVs) / sizeof(mKVs[0]);
} // namespace EditableConfigs

class ConfigsModel : public QAbstractTableModel {
  Q_OBJECT
public:
  enum ColumnE {
    NAME = 0,
    INITIAL_VALUE = 1,
    CURRENT_VALUE = 2,
    VALIDATIDATION_VALUE = 3,
    DATA_TYPE_ROLE = Qt::UserRole + 1,
  };

  explicit ConfigsModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return EditableConfigs::EDTIABLE_ROWS_COUNT; }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return ConfigsModel::COLUMNS_COUNT; }
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (index.column() != ConfigsModel::EDITABLE_COLUMN) {
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
    }
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEditable;
  }

  bool isPath(const QModelIndex& index) const;
  QString filePath(const QModelIndex& index) const;
  void RecheckAllCfg();
  int failCount() const { return mCfgCheckResult.size() - static_cast<int>(mCfgCheckResult.count()); }

  static constexpr int EDITABLE_COLUMN = ColumnE::CURRENT_VALUE;

signals:
  void failedCountChanged(int newFaileCnt);

private:
  std::bitset<EditableConfigs::EDTIABLE_ROWS_COUNT> mCfgCheckResult;
  static constexpr const char* CONFIGS_TABLE_HEADER[]{"Name", "Initial value", "Current value", "Validation"};
  static constexpr const int COLUMNS_COUNT = sizeof(CONFIGS_TABLE_HEADER) / sizeof(CONFIGS_TABLE_HEADER[0]);
};

#endif // CONFIGSMODEL_H

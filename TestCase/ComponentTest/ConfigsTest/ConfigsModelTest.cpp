#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ConfigsModel.h"
#include "EndToExposePrivateMember.h"

#include "GlbDataProtect.h"

#include "MemoryKey.h"
#include "StyleKey.h"
#include "Configuration.h"

#include <QSignalSpy>

class ConfigsModelTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void init() { //
    Configuration().clear();
  }

  void default_not_crash_ok() {
    // 预期 4-by-4
    ConfigsModel model;
    QCOMPARE(KVTestOnly::KV_TEST_COUNT, 4);
    QCOMPARE(EditableConfigs::EDTIABLE_ROWS_COUNT, 4);
    QCOMPARE(model.rowCount(), 4);

    QCOMPARE(ConfigsModel::COLUMNS_COUNT, 4);
    QCOMPARE(model.columnCount(), (int) ConfigsModel::COLUMNS_COUNT);

    // headerData ok
    QCOMPARE(model.headerData(0, Qt::Orientation::Horizontal, Qt::DisplayRole).toString(), "Name");
    QCOMPARE(model.headerData(1, Qt::Orientation::Horizontal, Qt::DisplayRole).toString(), "Initial value");
    QCOMPARE(model.headerData(2, Qt::Orientation::Horizontal, Qt::DisplayRole).toString(), "Current value");
    QCOMPARE(model.headerData(3, Qt::Orientation::Horizontal, Qt::DisplayRole).toString(), "Validation");

    QCOMPARE(model.headerData(0, Qt::Orientation::Vertical, Qt::TextAlignmentRole).toInt(), Qt::AlignRight);
    QCOMPARE(model.headerData(0, Qt::Orientation::Vertical, Qt::DisplayRole).toInt(), 1);
    QCOMPARE(model.headerData(3, Qt::Orientation::Vertical, Qt::DisplayRole).toInt(), 1 + 3);

    QCOMPARE(model.headerData(0, Qt::Orientation::Horizontal, Qt::ForegroundRole).isValid(), false);
    QCOMPARE(model.headerData(0, Qt::Orientation::Vertical, Qt::ForegroundRole).isValid(), false);

    // data ok
    QCOMPARE(model.data({}).isValid(), false);

    // flags ok
    model.flags({});
    QModelIndex allowedEditColumn = model.index(KVTestOnly::VOLUME_VALUE_ROW, ConfigsModel::EDITABLE_COLUMN);
    QVERIFY(model.flags(allowedEditColumn).testFlag(Qt::ItemIsEditable));
    QModelIndex notAllowedEditColumn = model.index(KVTestOnly::VOLUME_VALUE_ROW, ConfigsModel::ColumnE::NAME);
    QVERIFY(!model.flags(notAllowedEditColumn).testFlag(Qt::ItemIsEditable));

    // setData ok
    // index非法
    QCOMPARE(model.setData({}, 20, Qt::EditRole), false);
    // role 不对
    QCOMPARE(model.setData(allowedEditColumn, 20, Qt::DisplayRole), false);
    // 数值不接受
    QCOMPARE(model.setData(allowedEditColumn, -1, Qt::EditRole), false);
    // 该列不允许编辑
    QCOMPARE(model.setData(notAllowedEditColumn, -1, Qt::EditRole), false);

    // filePath ok
    QModelIndex notPathRelatedRow{model.index(KVTestOnly::PLATER_MUTE_ROW, ConfigsModel::ColumnE::NAME)};
    QCOMPARE(model.filePath({}), "");
    QCOMPARE(model.filePath(notPathRelatedRow), "");

    // failCount ok
    QCOMPARE(model.failCount(), 3);
    // RecheckAllCfg ok;
    QCOMPARE(model.failCount(), 3);
  }

  void RecheckAllCfg_ok() {
    ConfigsModel model;
    model.mCfgCheckResult.reset(); // 全0, 全失败
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.failCount(), 4);
    model.RecheckAllCfg();
    QCOMPARE(model.failCount(), 3);
  }

  void dataRetrieve_ok() {
    ConfigsModel model;
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.failCount(), 3);

    const QModelIndex playerFilePathIndex{model.index(KVTestOnly::FILE_PATH_ROW, ConfigsModel::NAME)};
    const QModelIndex workFolderPathIndex{model.index(KVTestOnly::FOLDER_PATH_ROW, ConfigsModel::NAME)};
    const QModelIndex volumeValueIndex{model.index(KVTestOnly::VOLUME_VALUE_ROW, ConfigsModel::NAME)};
    const QModelIndex playerMuteIndex{model.index(KVTestOnly::PLATER_MUTE_ROW, ConfigsModel::NAME)};

    QVERIFY(model.isPath(playerFilePathIndex));
    QVERIFY(model.isPath(workFolderPathIndex));
    QVERIFY(!model.isPath(volumeValueIndex));
    QVERIFY(!model.isPath(playerMuteIndex));

    QCOMPARE(model.data(playerFilePathIndex).toString(), "PLAYER_FILE_PATH");
    QCOMPARE(model.data(workFolderPathIndex).toString(), "WORK_FOLDER_PATH");
    QCOMPARE(model.data(volumeValueIndex).toString(), "VOLUME_VALUE");
    QCOMPARE(model.data(playerMuteIndex).toString(), "PLAYER_MUTE");

    QCOMPARE(model.data(playerFilePathIndex.siblingAtColumn(ConfigsModel::INITIAL_VALUE)).toString(), "inexists/player/file/path");
    QCOMPARE(model.data(workFolderPathIndex.siblingAtColumn(ConfigsModel::INITIAL_VALUE)).toString(), "inexists/work/folder/path");
    QCOMPARE(model.data(volumeValueIndex.siblingAtColumn(ConfigsModel::INITIAL_VALUE)).toInt(), -5);
    QCOMPARE(model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::INITIAL_VALUE)).toBool(), false);

    // 空配置文件时, 无key, 使用预设值
    QCOMPARE(model.data(playerFilePathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toString(), "inexists/player/file/path");
    QCOMPARE(model.data(workFolderPathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toString(), "inexists/work/folder/path");
    QCOMPARE(model.data(volumeValueIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toInt(), -5);
    QCOMPARE(model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toBool(), false);

    // DecorationRole: 3 failed + 1 pass
    QVERIFY(!model.data(playerFilePathIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE), Qt::DisplayRole).toBool());
    QVERIFY(!model.data(workFolderPathIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE), Qt::DisplayRole).toBool());
    QVERIFY(!model.data(volumeValueIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE), Qt::DisplayRole).toBool());
    QVERIFY(model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE), Qt::DisplayRole).toBool());

    model.data(playerFilePathIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE), Qt::DecorationRole);
    model.data(workFolderPathIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE), Qt::DecorationRole);
    model.data(volumeValueIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE), Qt::DecorationRole);
    model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE), Qt::DecorationRole);

    model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::NAME), Qt::DecorationRole); // not crash down
    model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::NAME), Qt::ToolTipRole);    // not crash down

    QCOMPARE(model.data(playerFilePathIndex.siblingAtColumn(ConfigsModel::NAME), ConfigsModel::ColumnE::DATA_TYPE_ROLE).toInt(), GeneralDataType::Type::FILE_PATH);
    QCOMPARE(model.data(workFolderPathIndex.siblingAtColumn(ConfigsModel::NAME), ConfigsModel::ColumnE::DATA_TYPE_ROLE).toInt(), GeneralDataType::Type::FOLDER_PATH);
    QCOMPARE(model.data(volumeValueIndex.siblingAtColumn(ConfigsModel::NAME), ConfigsModel::ColumnE::DATA_TYPE_ROLE).toInt(), GeneralDataType::Type::RANGE_INT);
    QCOMPARE(model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::NAME), ConfigsModel::ColumnE::DATA_TYPE_ROLE).toInt(), GeneralDataType::Type::PLAIN_BOOL);

    // current value, initial value column contains no decorationRole
    QCOMPARE(model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE), Qt::DecorationRole).isValid(), false);
    QCOMPARE(model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::INITIAL_VALUE), Qt::DecorationRole).isValid(), false);

    // ForegroundRole: not specify
    QCOMPARE(model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::NAME), Qt::ForegroundRole).isValid(), false);

    // 仅CURRENT_VALUE列允许修改的
    QVERIFY(!model.flags(playerFilePathIndex.siblingAtColumn(ConfigsModel::NAME)).testFlag(Qt::ItemFlag::ItemIsEditable));
    QVERIFY(!model.flags(playerFilePathIndex.siblingAtColumn(ConfigsModel::INITIAL_VALUE)).testFlag(Qt::ItemFlag::ItemIsEditable));
    QVERIFY(model.flags(playerFilePathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).testFlag(Qt::ItemFlag::ItemIsEditable));
    QVERIFY(!model.flags(playerFilePathIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE)).testFlag(Qt::ItemFlag::ItemIsEditable));
  }

  void setData_ok() {
    // init() 中清空配置文件

    // 检查总共4条配置, 预设值有3个错误;
    ConfigsModel model;
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.failCount(), 3);

    const QModelIndex playerFilePathIndex{model.index(KVTestOnly::FILE_PATH_ROW, ConfigsModel::CURRENT_VALUE)};
    const QModelIndex workFolderPathIndex{model.index(KVTestOnly::FOLDER_PATH_ROW, ConfigsModel::CURRENT_VALUE)};
    const QModelIndex volumeValueIndex{model.index(KVTestOnly::VOLUME_VALUE_ROW, ConfigsModel::CURRENT_VALUE)};
    const QModelIndex playerMuteIndex{model.index(KVTestOnly::PLATER_MUTE_ROW, ConfigsModel::CURRENT_VALUE)};

    // 修改
    const QFileInfo fi{__FILE__};
    const QString correctPlayerPath{fi.absoluteFilePath()};
    const QString correctWorkFolderPath{fi.absolutePath()};
    const int correctVolumeValue{99};
    const QString incorrectPlayerPath{correctWorkFolderPath};
    const QString incorrectWorkFolderPath{correctPlayerPath};
    const bool newPlayerMute{!KVTestOnly::PLAYER_MUTE.v.data.b};

    QSignalSpy dataChangedSpy{&model, &ConfigsModel::dataChanged};
    QSignalSpy failedCountChangedSpy{&model, &ConfigsModel::failedCountChanged};
    QCOMPARE(model.failCount(), 3);

    // 改到不通过, 不允许修改, 无任何信号, 配置中的值不变
    QVERIFY(!model.setData(playerFilePathIndex, "new inexist/player/file", Qt::EditRole));
    QVERIFY(!model.setData(workFolderPathIndex, "new inexist/folder/file", Qt::EditRole));
    QVERIFY(!model.setData(volumeValueIndex, -999, Qt::EditRole));
    QVERIFY(!model.setData(playerMuteIndex, QVariant{}, Qt::EditRole));
    QCOMPARE(model.data(playerFilePathIndex).toString(), KVTestOnly::PLAYER_FILE_PATH.v.data.str);
    QCOMPARE(model.data(workFolderPathIndex).toString(), KVTestOnly::WORK_FOLDER_PATH.v.data.str);
    QCOMPARE(model.data(volumeValueIndex).toInt(), KVTestOnly::VOLUME_VALUE.v.data.i);
    QCOMPARE(model.data(playerMuteIndex).toBool(), KVTestOnly::PLAYER_MUTE.v.data.b);
    QCOMPARE(dataChangedSpy.count(), 0);
    QCOMPARE(failedCountChangedSpy.count(), 0);
    dataChangedSpy.clear();
    failedCountChangedSpy.clear();
    QCOMPARE(model.failCount(), 3);

    // file 改到通过, 允许修改, VALIDATIDATION_VALUE列信号1次, 当前编辑列信号1次, failedCountChanged 1次
    QCOMPARE(model.setData(playerFilePathIndex, correctPlayerPath, Qt::EditRole), true);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(failedCountChangedSpy.count(), 1);
    QCOMPARE(failedCountChangedSpy.takeLast(), (QVariantList{2}));
    dataChangedSpy.clear();
    failedCountChangedSpy.clear();
    QCOMPARE(model.failCount(), 2);
    QCOMPARE(model.data(playerFilePathIndex).toString(), correctPlayerPath);
    QCOMPARE(model.filePath(playerFilePathIndex), correctPlayerPath);

    // folder 改到通过, 允许修改, VALIDATIDATION_VALUE列信号1次, 当前编辑列信号1次, failedCountChanged 1次
    QCOMPARE(model.setData(workFolderPathIndex, correctWorkFolderPath, Qt::EditRole), true);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(failedCountChangedSpy.count(), 1);
    QCOMPARE(failedCountChangedSpy.takeLast(), (QVariantList{1}));
    dataChangedSpy.clear();
    failedCountChangedSpy.clear();
    QCOMPARE(model.failCount(), 1);
    QCOMPARE(model.data(workFolderPathIndex).toString(), correctWorkFolderPath);
    QCOMPARE(model.filePath(workFolderPathIndex), correctWorkFolderPath);

    // volume 改到通过, 允许修改, VALIDATIDATION_VALUE列信号1次, 当前编辑列信号1次, failedCountChanged 1次
    QCOMPARE(model.setData(volumeValueIndex, correctVolumeValue, Qt::EditRole), true);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(failedCountChangedSpy.count(), 1);
    QCOMPARE(failedCountChangedSpy.takeLast(), (QVariantList{0}));
    dataChangedSpy.clear();
    failedCountChangedSpy.clear();
    QCOMPARE(model.failCount(), 0);
    QCOMPARE(model.data(volumeValueIndex).toInt(), correctVolumeValue);
    QCOMPARE(model.filePath(volumeValueIndex), "");

    // Mute 从通过改到另一个通过, 允许修改, VALIDATIDATION_VALUE列信号0次, 当前编辑列信号1次, failedCountChanged 0次
    QCOMPARE(model.setData(playerMuteIndex, newPlayerMute, Qt::EditRole), true);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(failedCountChangedSpy.count(), 0);
    dataChangedSpy.clear();
    failedCountChangedSpy.clear();
    QCOMPARE(model.failCount(), 0);
    QCOMPARE(model.data(playerMuteIndex).toBool(), newPlayerMute);
    QCOMPARE(model.filePath(playerMuteIndex), "");
  }
};

#include "ConfigsModelTest.moc"
REGISTER_TEST(ConfigsModelTest, false)

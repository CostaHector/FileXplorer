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
  void init() {  //
    Configuration().clear();
  }

  void default_ok() {
    // not crash down
    ConfigsModel model;
    QCOMPARE(KVTestOnly::KV_TEST_COUNT, 4);
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.failCount(), 3);

    QCOMPARE(model.columnCount(), (int)ConfigsModel::COLUMNS_COUNT);
    QCOMPARE(model.headerData(0, Qt::Orientation::Horizontal, Qt::DisplayRole).toString(), "Name");
    QCOMPARE(model.headerData(1, Qt::Orientation::Horizontal, Qt::DisplayRole).toString(), "Initial value");
    QCOMPARE(model.headerData(2, Qt::Orientation::Horizontal, Qt::DisplayRole).toString(), "Current value");
    QCOMPARE(model.headerData(3, Qt::Orientation::Horizontal, Qt::DisplayRole).toString(), "Validation");

    QCOMPARE(model.headerData(0, Qt::Orientation::Vertical, Qt::TextAlignmentRole).toInt(), Qt::AlignRight);
    QCOMPARE(model.headerData(0, Qt::Orientation::Vertical, Qt::DisplayRole).toInt(), 1);
    QCOMPARE(model.headerData(3, Qt::Orientation::Vertical, Qt::DisplayRole).toInt(), 1 + 3);

    QCOMPARE(model.headerData(0, Qt::Orientation::Horizontal, Qt::ForegroundRole).isValid(), false);
    QCOMPARE(model.headerData(0, Qt::Orientation::Vertical, Qt::ForegroundRole).isValid(), false);

    QCOMPARE(model.data({}).isValid(), false);
    model.flags({});

    QModelIndex allowedEditColumn = model.index(KVTestOnly::VOLUME_VALUE_ROW, ConfigsModel::EDITABLE_COLUMN);
    QVERIFY(model.flags(allowedEditColumn).testFlag(Qt::ItemIsEditable));
    QModelIndex notAllowedEditColumn = model.index(KVTestOnly::VOLUME_VALUE_ROW, ConfigsModel::ColumnE::NAME);
    QVERIFY(!model.flags(notAllowedEditColumn).testFlag(Qt::ItemIsEditable));

    // index非法
    QCOMPARE(model.setData({}, 20, Qt::EditRole), false);
    // role 不对
    QCOMPARE(model.setData(allowedEditColumn, 20, Qt::DisplayRole), false);
    // 数值不接受
    QCOMPARE(model.setData(allowedEditColumn, -1, Qt::EditRole), false);
    // 该列不允许编辑
    QCOMPARE(model.setData(notAllowedEditColumn, -1, Qt::EditRole), false);

    QModelIndex notPathRelatedRow{model.index(KVTestOnly::PLATER_MUTE_ROW, ConfigsModel::ColumnE::NAME)};
    QCOMPARE(model.filePath({}), "");
    QCOMPARE(model.filePath(notPathRelatedRow), "");
  }

  void data_set_data_ok() {
    // 配置当前值清空, 检查总共4条配置, 预设值有3个错误;
    ConfigsModel model;
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.failCount(), 3);

    QModelIndex playerFilePathIndex{model.index(0, ConfigsModel::NAME)};
    QModelIndex workFolderPathIndex{model.index(1, ConfigsModel::NAME)};
    QModelIndex volumeValueIndex{model.index(2, ConfigsModel::NAME)};
    QModelIndex playerMuteIndex{model.index(3, ConfigsModel::NAME)};

    QCOMPARE(model.data(playerFilePathIndex).toString(), "playerFilePath");
    QCOMPARE(model.data(workFolderPathIndex).toString(), "workFolderPath");
    QCOMPARE(model.data(volumeValueIndex).toString(), "volumeValue");
    QCOMPARE(model.data(playerMuteIndex).toString(), "playerMute");

    QCOMPARE(model.data(playerFilePathIndex.siblingAtColumn(ConfigsModel::INITIAL_VALUE)).toString(), "inexists/player/file/path");
    QCOMPARE(model.data(workFolderPathIndex.siblingAtColumn(ConfigsModel::INITIAL_VALUE)).toString(), "inexists/work/folder/path");
    QCOMPARE(model.data(volumeValueIndex.siblingAtColumn(ConfigsModel::INITIAL_VALUE)).toInt(), -5);
    QCOMPARE(model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::INITIAL_VALUE)).toBool(), false);

    // 无当前值时, 将使用预设值
    QCOMPARE(model.data(playerFilePathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toString(), "inexists/player/file/path");
    QCOMPARE(model.data(workFolderPathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toString(), "inexists/work/folder/path");
    QCOMPARE(model.data(volumeValueIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toInt(), -5);
    QCOMPARE(model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toBool(), false);

    // DecorationRole: 1 pass, 2 not pass Icon
    QVariant playerFileNotPassIcon = model.data(playerFilePathIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE), Qt::DecorationRole);
    QVariant workFolderNotPassIcon = model.data(workFolderPathIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE), Qt::DecorationRole);
    QVariant volumeValueNotPassIcon = model.data(volumeValueIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE), Qt::DecorationRole);
    QVariant playerMutePassIcon = model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::VALIDATIDATION_VALUE), Qt::DecorationRole);
    QCOMPARE(workFolderNotPassIcon, playerFileNotPassIcon);
    QCOMPARE(volumeValueNotPassIcon, playerFileNotPassIcon);
    QVERIFY(playerMutePassIcon != playerFileNotPassIcon);
    QVERIFY(playerMutePassIcon.canConvert<QIcon>());

    // current value column contains no decorationRole
    QCOMPARE(model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE), Qt::DecorationRole).isValid(), false);

    // ForegroundRole: not specify
    QCOMPARE(model.data(playerMuteIndex.siblingAtColumn(ConfigsModel::NAME), Qt::ForegroundRole).isValid(), false);

    // 仅CURRENT_VALUE列允许修改的
    QCOMPARE(model.flags(playerFilePathIndex.siblingAtColumn(ConfigsModel::NAME)).testFlag(Qt::ItemFlag::ItemIsEditable), false);
    QCOMPARE(model.flags(playerFilePathIndex.siblingAtColumn(ConfigsModel::INITIAL_VALUE)).testFlag(Qt::ItemFlag::ItemIsEditable), false);
    QCOMPARE(model.flags(playerFilePathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).testFlag(Qt::ItemFlag::ItemIsEditable), true);

    // 修改
    const QString correctPlayerPath{__FILE__};
    const QString correctWorkFolderPath{QFileInfo{__FILE__}.absolutePath()};
    const int correctVolumeValue{99};
    const QString incorrectPlayerPath{correctWorkFolderPath};
    const QString incorrectWorkFolderPath{correctPlayerPath};
    const int incorrectVolumeValue{-correctVolumeValue};
    // role不匹配, 列不匹配,
    model.setData(playerFilePathIndex, correctPlayerPath, Qt::DisplayRole);
    model.setData(playerFilePathIndex, correctPlayerPath, Qt::EditRole);

    // 改到不通过, 不允许修改
    QCOMPARE(model.setData(playerFilePathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE), incorrectPlayerPath, Qt::EditRole), false);
    QCOMPARE(model.setData(workFolderPathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE), incorrectWorkFolderPath, Qt::EditRole), false);
    QCOMPARE(model.setData(workFolderPathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE), incorrectVolumeValue, Qt::EditRole), false);
    QVERIFY(model.data(playerFilePathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toString() != incorrectPlayerPath);
    QVERIFY(model.data(workFolderPathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toString() != incorrectWorkFolderPath);
    QVERIFY(model.data(volumeValueIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toInt() != incorrectVolumeValue);

    // 改到通过, 允许修改
    QSignalSpy decorationRoleDataChangedSignal{&model, &ConfigsModel::dataChanged};

    QCOMPARE(model.failCount(), 3);
    QCOMPARE(model.setData(playerFilePathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE), correctPlayerPath, Qt::EditRole), true);
    QCOMPARE(decorationRoleDataChangedSignal.count(), 1);
    QVariantList playerParms = decorationRoleDataChangedSignal.takeLast();
    QCOMPARE(playerParms.size(), 3);

    QCOMPARE(model.failCount(), 2);
    QCOMPARE(model.setData(workFolderPathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE), correctWorkFolderPath, Qt::EditRole), true);
    QCOMPARE(decorationRoleDataChangedSignal.count(), 1);
    decorationRoleDataChangedSignal.takeLast();
    QCOMPARE(model.failCount(), 1);
    QCOMPARE(model.setData(volumeValueIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE), correctVolumeValue, Qt::EditRole), true);
    QCOMPARE(decorationRoleDataChangedSignal.count(), 1);
    decorationRoleDataChangedSignal.takeLast();
    QCOMPARE(model.failCount(), 0);

    QCOMPARE(model.data(playerFilePathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toString(), correctPlayerPath);
    QCOMPARE(model.data(workFolderPathIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toString(), correctWorkFolderPath);
    QCOMPARE(model.data(volumeValueIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE)).toInt(), correctVolumeValue);
    QCOMPARE(model.filePath(playerFilePathIndex), correctPlayerPath);
    QCOMPARE(model.filePath(workFolderPathIndex), correctWorkFolderPath);
  }
};

#include "ConfigsModelTest.moc"
REGISTER_TEST(ConfigsModelTest, false)

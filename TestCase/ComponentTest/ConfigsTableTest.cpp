#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>

#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "MemoryKey.h"
#include "ConfigsTable.h"
#include "EndToExposePrivateMember.h"
#include "FileLeafAction.h"

class ConfigsTableTest : public PlainTestSuite {
  Q_OBJECT
 public:
  ConfigsTableTest() : PlainTestSuite{} { LOG_D("ConfigsTableTest object created\n"); }

 private slots:
  void test_configs_table_fail_count_ok() {
    Configuration().clear();
    static constexpr int USER_CFG_COUNT = 4;
    QCOMPARE(KV::mEditableKVs.size(), USER_CFG_COUNT);

    QList<const KV*> expectsList{&MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE,  //
                                 &MemoryKey::WHERE_CLAUSE_HISTORY,             //
                                 &MemoryKey::VIDS_LAST_TABLE_NAME,             //
                                 &RedunImgFinderKey::RUND_IMG_PATH};
    QCOMPARE(KV::mEditableKVs, expectsList);

    Configuration().setValue(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, "inexist imagehost path");
    Configuration().setValue(MemoryKey::WHERE_CLAUSE_HISTORY.name, "no checker here");
    Configuration().setValue(MemoryKey::VIDS_LAST_TABLE_NAME.name, "no checker here");
    Configuration().setValue(RedunImgFinderKey::RUND_IMG_PATH.name, "inexist redun image path");

    // 2 path not exist
    QCOMPARE(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.checker(                                      //
                 Configuration().value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name).toString()),  //
             false);
    QCOMPARE(RedunImgFinderKey::RUND_IMG_PATH.checker(                                      //
                 Configuration().value(RedunImgFinderKey::RUND_IMG_PATH.name).toString()),  //
             false);

    ConfigsTable cfgTbl;
    QVERIFY(cfgTbl.m_alertsTable != nullptr);
    QVERIFY(cfgTbl.m_alertModel != nullptr);
    cfgTbl.show();
    QCOMPARE(QTest::qWaitForWindowExposed(&cfgTbl), true);

    QCOMPARE(cfgTbl.m_alertModel->rowCount(), USER_CFG_COUNT);
    QCOMPARE(cfgTbl.m_alertModel->failCount(), 2);

    QModelIndex imageHostValueIndex = cfgTbl.m_alertModel->index(0, ConfigsModel::VALUE_COLUMN);
    QModelIndex redunImgValueIndex = cfgTbl.m_alertModel->index(3, ConfigsModel::VALUE_COLUMN);
    // set a file path, not accept
    const QString fileNotFolderPath = __FILE__;
    QCOMPARE(cfgTbl.m_alertModel->setData(imageHostValueIndex, fileNotFolderPath, Qt::EditRole), false);
    QCOMPARE(cfgTbl.m_alertModel->failCount(), 2);

    // set 2 folder path, accept, memory changed
    const QString folderPath = QFileInfo(fileNotFolderPath).absolutePath();
    QCOMPARE(cfgTbl.m_alertModel->setData(imageHostValueIndex, folderPath, Qt::EditRole), true);
    QCOMPARE(cfgTbl.m_alertModel->setData(redunImgValueIndex, folderPath, Qt::EditRole), true);
    QCOMPARE(cfgTbl.m_alertModel->filePath(imageHostValueIndex), folderPath);
    QCOMPARE(cfgTbl.m_alertModel->filePath(redunImgValueIndex), folderPath); //
    QCOMPARE(cfgTbl.m_alertModel->filePath(QModelIndex{}), ""); // invalid index return empty string
    QCOMPARE(Configuration().value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name).toString(), folderPath);
    QCOMPARE(Configuration().value(RedunImgFinderKey::RUND_IMG_PATH.name).toString(), folderPath);
    QCOMPARE(cfgTbl.m_alertModel->failCount(), 0);

    cfgTbl.hide();
    QCOMPARE(g_fileLeafActions()._SETTINGS->isChecked(), false);
    cfgTbl.close();
  }
};

#include "ConfigsTableTest.moc"
REGISTER_TEST(ConfigsTableTest, false)

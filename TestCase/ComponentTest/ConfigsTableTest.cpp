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
#include "StyleSheet.h"
#include "FileTool.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class ConfigsTableTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void default_ok() {
    Configuration().clear();
    static constexpr int USER_CFG_COUNT = 4;
    QCOMPARE(KV::mEditableKVs.size(), USER_CFG_COUNT);

    QList<const KV*> expectsList{&MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE,  //
                                 &MemoryKey::WHERE_CLAUSE_HISTORY,             //
                                 &MemoryKey::VIDS_LAST_TABLE_NAME,             //
                                 &RedunImgFinderKey::RUND_IMG_PATH};
    QCOMPARE(KV::mEditableKVs, expectsList);

    ConfigsTable cfgTbl;
    cfgTbl.showEvent(nullptr);
    QShowEvent defaultShowEvent;
    cfgTbl.showEvent(&defaultShowEvent);
    cfgTbl.hideEvent(nullptr);
    QHideEvent defaultHideEvent;
    cfgTbl.hideEvent(&defaultHideEvent);
    QCOMPARE(g_fileLeafActions()._SETTINGS->isChecked(), false);

    const QString cfgFilePath{Configuration().fileName()};
    bool bCfgExist{QFile::exists(cfgFilePath)};
    if (bCfgExist) {
      MOCKER(FileTool::OpenLocalFile).expects(exactly(1)).with(eq(cfgFilePath)).will(returnValue(true));
      QCOMPARE(cfgTbl.onEditPreferenceSetting(), true);
    } else {
      MOCKER(FileTool::OpenLocalFile).expects(never()).will(returnValue(true));
      QCOMPARE(cfgTbl.onEditPreferenceSetting(), false);
    }

    QCOMPARE(cfgTbl.on_cellDoubleClicked({}), false);
  }

  void configs_table_fail_count_ok() {
    Configuration().clear();
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

    QCOMPARE(cfgTbl.m_alertModel->rowCount(), KV::mEditableKVs.size());
    QCOMPARE(cfgTbl.m_alertModel->failCount(), 2);

    const QString fileNotFolderPath = __FILE__;
    const QString folderPath = QFileInfo(fileNotFolderPath).absolutePath();
    MOCKER(FileTool::OpenLocalFile).expects(exactly(1)).with(eq(folderPath)).will(returnValue(true));

    QModelIndex imageHostValueIndex = cfgTbl.m_alertModel->index(0, ConfigsModel::VALUE_COLUMN);
    QModelIndex redunImgValueIndex = cfgTbl.m_alertModel->index(3, ConfigsModel::VALUE_COLUMN);
    QCOMPARE(cfgTbl.on_cellDoubleClicked(imageHostValueIndex), false);  // PATH_PERFORMER_IMAGEHOST_LOCATE 对应的路径不存在
    QCOMPARE(cfgTbl.on_cellDoubleClicked(redunImgValueIndex), false);   // RUND_IMG_PATH 对应的路径不存在

    // set a file path, not accept
    QCOMPARE(cfgTbl.m_alertModel->setData(imageHostValueIndex, fileNotFolderPath, Qt::EditRole), false);
    QCOMPARE(cfgTbl.m_alertModel->failCount(), 2);

    // set 2 folder path, accept, memory changed
    QCOMPARE(cfgTbl.m_alertModel->setData(imageHostValueIndex, folderPath, Qt::EditRole), true);
    QCOMPARE(cfgTbl.m_alertModel->setData(redunImgValueIndex, folderPath, Qt::EditRole), true);
    QCOMPARE(cfgTbl.m_alertModel->filePath(imageHostValueIndex), folderPath);
    QCOMPARE(cfgTbl.m_alertModel->filePath(redunImgValueIndex), folderPath);
    QCOMPARE(cfgTbl.on_cellDoubleClicked(redunImgValueIndex), true);  // RUND_IMG_PATH 对应的路径存在

    QCOMPARE(cfgTbl.m_alertModel->filePath(QModelIndex{}), "");  // invalid index return empty string
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

#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ConfigsTableView.h"
#include "EndToExposePrivateMember.h"

#include "GlbDataProtect.h"
#include "Configuration.h"
#include "FileTool.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class ConfigsTableViewTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void init() { //
    Configuration().clear();
    GlobalMockObject::reset();
  }
  void cleanup() { GlobalMockObject::verify(); }

  void GetStatistics_ok() {
    using namespace RawVariant;
    using namespace ValueChecker;

    GlbDataProtect<QList<const KV *>> editableKVsBackup{KV::GetEditableKVs()};
    constexpr KV playerFilePath{"playerFilePath", Var{"inexists/player/file/path"}, GeneralDataType::Type::FILE_PATH, ValueChecker::GeneralFilePathChecker};     // invalid
    constexpr KV workFolderPath{"workFolderPath", Var{"inexists/work/folder/path"}, GeneralDataType::Type::FOLDER_PATH, ValueChecker::GeneralFolderPathChecker}; // invalid
    constexpr KV volumeValue{"volumeValue", Var{-5}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>};                                          // invalid
    constexpr KV playerMute{"playerMute", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};                                                    // valid
    QList<const KV *> tempLst{&playerFilePath, &workFolderPath, &volumeValue, &playerMute};
    KV::GetEditableKVs().swap(tempLst);
    QCOMPARE(KV::GetEditableKVs().size(), 4);

    std::pair<int, int> failedTotalPair{3, 4};
    ConfigsTableView alertView{"AlertViewInTest"};
    QCOMPARE(alertView.GetStatistics(), failedTotalPair);

    QCOMPARE(alertView.GetName(), "AlertViewInTest");
    QCOMPARE(alertView.ConfigsTableView::m_defaultShowBackgroundImage, true);

    auto *model = alertView.GetModel();
    QVERIFY(model != nullptr);
    QModelIndex r0 = model->index(0, 0);
    QModelIndex r1 = model->index(0, 0);
    QModelIndex r2 = model->index(0, 0);

    const QFileInfo curFi{__FILE__};
    const QString filePath{curFi.absoluteFilePath()};
    const QString folderPath{curFi.absolutePath()};

    MOCKER(FileTool::OpenLocalFile) //
        .expects(exactly(2))        //
        .will(returnValue(true))    // 1st
        .then(returnValue(false));  // 2nd. return false for distinguish usage only

    QCOMPARE(alertView.selectionModel()->hasSelection(), false);
    emit alertView.doubleClicked({});

    QCOMPARE(alertView.on_cellDoubleClicked({}), false);
    QCOMPARE(alertView.on_cellDoubleClicked(r0), false); // path related, but file path not exist
    QCOMPARE(alertView.on_cellDoubleClicked(r1), false); // path related, but folder path not exist
    QCOMPARE(alertView.on_cellDoubleClicked(r2), false); // not path related

    setConfig(playerFilePath, filePath);
    setConfig(workFolderPath, folderPath);
    QCOMPARE(alertView.on_cellDoubleClicked(r0), true);  // 1st path related, and file path exist
    QCOMPARE(alertView.on_cellDoubleClicked(r1), false); // 2nd path related, and folder path exist
  }
};

#include "ConfigsTableViewTest.moc"
REGISTER_TEST(ConfigsTableViewTest, false)

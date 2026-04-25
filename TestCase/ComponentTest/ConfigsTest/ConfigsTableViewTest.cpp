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
  void cleanup() { //
    GlobalMockObject::verify();
  }

  void GetStatistics_ok() {
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

    setConfig(KVTestOnly::playerFilePath, filePath);
    setConfig(KVTestOnly::workFolderPath, folderPath);
    QCOMPARE(alertView.on_cellDoubleClicked(r0), true);  // 1st path related, and file path exist
    QCOMPARE(alertView.on_cellDoubleClicked(r1), false); // 2nd path related, and folder path exist
  }
};

#include "ConfigsTableViewTest.moc"
REGISTER_TEST(ConfigsTableViewTest, false)

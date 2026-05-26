#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "MultiParView.h"
#include "MultiParDialog.h"
#include "EndToExposePrivateMember.h"

#include "ParVerifyInfomationExamples.h"
#include "MultiParTools.h"
#include "UserInteractiveMock.h"

#include <QMessageBox>
#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class MultiParViewTest : public PlainTestSuite {
  Q_OBJECT
public:
  using PlainTestSuite::PlainTestSuite;

private:
private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void default_ok() {
    using namespace ParVerifyInfomationExamples;
    ParVerifyInfomationList infoList;

    MultiParView view{std::move(infoList), "MultiPar2View"};
    QCOMPARE(view.onRepairBrokenFile(), 0);
    QCOMPARE(view.onFixMisnamedFile(), 0);
    QCOMPARE(view.onProcessOldNewPar2NamesThenReverify(), 0);

    ParVerifyInfomationList emptyInfoList;
    MultiParDialog dlg{std::move(emptyInfoList)};
    dlg.onStartFilter("NothingShouldMatch");
  }

  void repair_reverify_ok() {
    using namespace ParVerifyInfomationExamples;
    ParVerifyInfomation allFilesComplete{GetAllFilesCompleteStruct()};
    ParVerifyInfomation readyToRename{GetReadyToRenameStruct()};
    ParVerifyInfomation readyToRepair{GetReadyToRepairStruct()};
    ParVerifyInfomation needMoreSlicesToRepair{GetNeedMoreSlicesToRepairStruct()};
    ParVerifyInfomation untrustable{GetVerifyUntrustableStruct()};

    ParVerifyInfomationList infoList{allFilesComplete, readyToRename, readyToRepair, untrustable, needMoreSlicesToRepair};
    MOCKER(MultiParTools::IsMultiPar2Available).stubs().will(returnValue(false));

    MultiParView view{std::move(infoList), "MultiPar2View"};
    view.selectAll();
    QCOMPARE(view.onRepairBrokenFile(), -1);
    QCOMPARE(view.onSyncPar2FileName(), 0); // cannot sync at all

    MOCKER((UserInteractiveMock::QUESTION_TYPE)QMessageBox::warning).expects(exactly(2)).will(returnValue(QMessageBox::StandardButton::No)).then(returnValue(QMessageBox::StandardButton::Yes));
    QCOMPARE(view.onFixMisnamedFile(), 0);  // user select no
    QCOMPARE(view.onFixMisnamedFile(), -1); // user select yes

    QCOMPARE(view.mSortFilterProxy->rowCount(), 5);

    QSignalSpy showCliOutputReqSpy{&view, &MultiParView::showCliOutputReq};
    view.onSelectionChange(QModelIndex{});
    QCOMPARE(showCliOutputReqSpy.count(), 0);

    QModelIndexList proIndexes = view.selectionModel()->selectedRows();
    QCOMPARE(proIndexes.size(), 5);
    QModelIndex proInd = proIndexes.front();
    view.onSelectionChange(proInd);
    QCOMPARE(showCliOutputReqSpy.count(), 1);
    showCliOutputReqSpy.clear();

    view.setFilter("MediaFileNeedBackup.mp4");
    QCOMPARE(view.mSortFilterProxy->rowCount(), 1);

    view.setFilter("");
    QCOMPARE(view.mSortFilterProxy->rowCount(), 5);
  }
};

#include "MultiParViewTest.moc"
REGISTER_TEST(MultiParViewTest, false)

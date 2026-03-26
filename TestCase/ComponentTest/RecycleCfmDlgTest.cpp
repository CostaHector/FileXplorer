#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "RecycleCfmDlg.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

std::function<bool(const QString&)> GetCheckFunc(const QString& containsStr, const QString& notContainsStr) {
  return [containsStr, notContainsStr](const QString& text) -> bool {  //
    return (containsStr.isEmpty() || text.contains(containsStr))       //
           && (notContainsStr.isEmpty() || !text.contains(notContainsStr));
  };
}

class RecycleCfmDlgTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() { GlobalMockObject::reset(); }

  void cleanup() { GlobalMockObject::verify(); }

  void recycle_ok() {
    auto recycleTitleChecker = GetCheckFunc("Move to Recycle Bin", "");
    auto deleteTitleChecker = GetCheckFunc("Permanently Delete Files", "");

    auto recycleMsgChecker = GetCheckFunc("", "WARNING: This action cannot be undone!");
    auto deleteMsgChecker = GetCheckFunc("WARNING: This action cannot be undone!", "");

    auto detailMsgChecker = GetCheckFunc("file1", "");

    MOCKER(RecycleCfmDlg::YesOrCancelBox)                                                                                                     //
        .expects(exactly(2))                                                                                                                  //
        .with(QMessageBox::Icon::Question, any(), checkWith(recycleTitleChecker), checkWith(recycleMsgChecker), checkWith(detailMsgChecker))  //
        .will(returnValue(false))                                                                                                             //
        .then(returnValue(true))                                                                                                              //
        .id("0");
    MOCKER(RecycleCfmDlg::YesOrCancelBox)                                                                                                  //
        .expects(exactly(2))                                                                                                               //
        .with(QMessageBox::Icon::Warning, any(), checkWith(deleteTitleChecker), checkWith(deleteMsgChecker), checkWith(detailMsgChecker))  //
        .after("0")                                                                                                                        //
        .will(returnValue(false))                                                                                                          //
        .then(returnValue(true))                                                                                                           //
        .id("1");

    QVERIFY(!RecycleCfmDlg::recycleQuestion("locatedPath", {"file1, file2"}, false));  // recycle, Cancel
    QVERIFY(RecycleCfmDlg::recycleQuestion("locatedPath", {"file1, file2"}, false));   // recycle, Yes

    QVERIFY(!RecycleCfmDlg::recycleQuestion("locatedPath", {"file1, file2"}, true));  // delete, Cancel
    QVERIFY(RecycleCfmDlg::recycleQuestion("locatedPath", {"file1, file2"}, true));   // delete, Yes
  }
};

#include "RecycleCfmDlgTest.moc"
REGISTER_TEST(RecycleCfmDlgTest, false)

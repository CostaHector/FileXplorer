#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pubTestTool/MyTestSuite.h"
#include "Tools/RenameHelper.h"
using namespace RenameHelper;

class RenameHelperTest : public MyTestSuite {
  Q_OBJECT
public:
  RenameHelperTest() : MyTestSuite{false} {}
private slots:
  void test_ReplaceRename_invalid_regex_failed() {
    QStringList replaceeList;
    replaceeList << "AAAAAAA"
                 << "aaaaaaa"
                 << "0000000";
    QString oldString{"[A-Za-z0-9{0,9}"};  // invalid
    QString newString{"X"};
    bool regexEnable{true};
    QStringList ansLst = ReplaceRename(replaceeList, oldString, newString, regexEnable);
    QVERIFY(ansLst.isEmpty());
  }

  void test_ReplaceRename_valid_regex_ok() {
    QStringList replaceeList;
    replaceeList << "AAAAAAA"
                 << "aaaaaaa"
                 << "0000000";
    QStringList expectList;
    expectList << "X"
               << "X"
               << "X";
    QString oldString{"[A-Za-z0-9]{1,7}"};
    QString newString{"X"};
    bool regexEnable{true};
    QStringList ansLst = ReplaceRename(replaceeList, oldString, newString, regexEnable);
    QCOMPARE(ansLst, expectList);
  }

  void test_ReplaceRename_ok() {
    QStringList replaceeList;
    replaceeList << "la - hello world"
                 << "map - nice T-Shirt"
                 << "LA - not bad";
    QStringList expectList;
    expectList << "LosAngle - hello world"
               << "map - nice T-Shirt"
               << "LA - not bad";
    QString oldString{"la"};
    QString newString{"LosAngle"};
    bool regexEnable{true};
    QStringList ansLst = ReplaceRename(replaceeList, oldString, newString, regexEnable);
    QCOMPARE(ansLst, expectList);
  }

  void test_regular_expression_replace() {
    QRegularExpression repRegex("^(.*?)$");
    QVERIFY(repRegex.isValid());
    QString beforeName {"FS 001"};
    beforeName.replace(repRegex, "FusionSphere \\1 front");
    QCOMPARE(beforeName, "FusionSphere FS 001 front");
  }

  void test_ReplaceRename_regex_ok() {
    QStringList replaceeList;
    replaceeList << "FS 01"
                 << "FS 02"
                 << "FS 05";
    QStringList expectList;
    expectList << "FushionSphere FS 01 Back"
               << "FushionSphere FS 02 Back"
               << "FushionSphere FS 05 Back";
    QString oldString{"^(.*?)$"};
    QString newString{"FushionSphere \\1 Back"};
    bool regexEnable{true};
    QStringList ansLst = ReplaceRename(replaceeList, oldString, newString, regexEnable);
    QCOMPARE(ansLst, expectList);
  }

  void test_ReplaceRename_skip_ok() {
    QVERIFY(ReplaceRename({}, "A", "B", false).isEmpty());
    QVERIFY(ReplaceRename({"AAA", "BBB"}, "", "B", false).isEmpty());
    QVERIFY(ReplaceRename({}, "A", "B", false).isEmpty());
  }

  void test_NumerizeReplace_with_extension_startIndx_0_ok() {
    QStringList replaceeList;
    replaceeList << "la - hello world"
                 << "la - hello world 2"
                 << "la - hello world 3"
                 << "la - hello world"
                 << "la - hello world";
    QStringList suffixs;
    suffixs << ".mp4"
            << ".mp4"
            << ".mp4"
            << ".json"
            << ".jpeg";
    QStringList expectList;
    expectList << "LosAngle - hello world (0)"
               << "LosAngle - hello world (1)"
               << "LosAngle - hello world (2)"
               << "LosAngle - hello world"
               << "LosAngle - hello world";
    const QString& baseName{"LosAngle - hello world"};
    const int startInd{0};
    const QString& namePattern{" (%1)"};
    QStringList ansLst = NumerizeReplace(replaceeList, suffixs, baseName, startInd, namePattern);
    QCOMPARE(ansLst, expectList);
  }

  void test_NumerizeReplace_startIndx_99_filled_with_0_ok() {
    QStringList replaceeList;
    replaceeList << "la - hello world 99"
                 << "la - hello world 10"
                 << "la - hello world"
                 << "la - hello world";
    QStringList suffixs;
    suffixs << ".mp4"
            << ".mp4"
            << ".json"
            << ".jpeg";
    QStringList expectList;
    expectList << "LosAngle - hello world 099"
               << "LosAngle - hello world 100"
               << "LosAngle - hello world"
               << "LosAngle - hello world";
    const QString& baseName{"LosAngle - hello world"};
    const int startInd{99};
    const QString& namePattern{" %1"};
    QStringList ansLst = NumerizeReplace(replaceeList, suffixs, baseName, startInd, namePattern);
    QCOMPARE(ansLst, expectList);
  }

  void test_NumerizeReplace_startIndx_1_with_extension() {
    QStringList replaceeList;
    replaceeList << "la - hello world.jpg"
                 << "la - hello world 1.json";
    QStringList suffixs;
    suffixs << ""
            << "";
    QStringList expectList;
    expectList << "LosAngle - hello world 0"
               << "LosAngle - hello world 1";
    const QString& baseName{"LosAngle - hello world"};
    const int startInd{0};
    const QString& namePattern{" %1"};
    QStringList ansLst = NumerizeReplace(replaceeList, suffixs, baseName, startInd, namePattern);
    QCOMPARE(ansLst, expectList);
  }

  void test_InsertRename_push_front_ok() {
    QStringList replaceeList;
    replaceeList << "la - hello world 99"
                 << "la - hello world 10"
                 << "la - hello world"
                 << "la - hello world";
    QStringList expectList;
    expectList << "Moviela - hello world 99"
               << "Moviela - hello world 10"
               << "Moviela - hello world"
               << "Moviela - hello world";
    const QString insertString{"Movie"};
    const int insertAt{0};
    QStringList ansLst = InsertRename(replaceeList, insertString, insertAt);
    QCOMPARE(ansLst, expectList);
  }

  void test_InsertRename_push_back_ok() {
    QStringList replaceeList;
    replaceeList << "la - hello world 99"
                 << "la - hello world 10"
                 << "la - hello world"
                 << "la - hello world";
    QStringList expectList;
    expectList << "la - hello world 99Movie"
               << "la - hello world 10Movie"
               << "la - hello worldMovie"
               << "la - hello worldMovie";
    const QString insertString{"Movie"};
    const int insertAt{512};
    QStringList ansLst = InsertRename(replaceeList, insertString, insertAt);
    QCOMPARE(ansLst, expectList);
  }

  void test_InsertRename_insert_middle_ok() {
    QStringList replaceeList;
    replaceeList << "la - hello world 99"
                 << "la - hello world 10"
                 << "la - hello world"
                 << "la - hello world";
    QStringList expectList;
    expectList << "lMoviea - hello world 99"
               << "lMoviea - hello world 10"
               << "lMoviea - hello world"
               << "lMoviea - hello world";
    const QString insertString{"Movie"};
    const int insertAt{1};
    QStringList ansLst = InsertRename(replaceeList, insertString, insertAt);
    QCOMPARE(ansLst, expectList);
  }

  void test_InsertRename_push_back_after_extension_ok() {
    QStringList replaceeList;
    replaceeList << "la - hello world 99.jpg"
                 << "la - hello world 10.jpg"
                 << "la - hello world.jpg"
                 << "la - hello world.jpg";
    QStringList expectList;
    expectList << "la - hello world 99.jpgMovie"
               << "la - hello world 10.jpgMovie"
               << "la - hello world.jpgMovie"
               << "la - hello world.jpgMovie";
    const QString insertString{"Movie"};
    const int insertAt{512};
    QStringList ansLst = InsertRename(replaceeList, insertString, insertAt);
    QCOMPARE(ansLst, expectList);
  }

  void test_PrependParentFolderNameToFileName_skip() {
    QStringList parentFolders;
    QStringList completeNames;
    QStringList suffixs;
    QCOMPARE(PrependParentFolderNameToFileName(parentFolders, completeNames, suffixs), (QStringList{}));

    // elements count inequal
    parentFolders.push_back("path/to");
    completeNames.push_back("a");
    QCOMPARE(PrependParentFolderNameToFileName(parentFolders, completeNames, suffixs), (QStringList{}));

    // not a file
    suffixs.push_back("");
    QCOMPARE(PrependParentFolderNameToFileName(parentFolders, completeNames, suffixs), completeNames);
  }

  void test_PrependParentFolderNameToFileName_ok() {
    QStringList parentFolders{"Henry Cavill/Superman/post",//
                              "Henry Cavill/Superman/post",//
                              "Chris Evans/Captain/post",//
                              "Chris Evans/Captain/post",//
                             };
    QStringList completeNames{"HC 1", "HC 2", "CE", "CE 2"};
    QStringList suffixs{".jpg", ".jpeg", ".jpeg", ".jpeg"};
    QCOMPARE(PrependParentFolderNameToFileName(parentFolders, completeNames, suffixs), //
             (QStringList{
                "Henry Cavill Superman post HC 1",//
                "Henry Cavill Superman post HC 2",//
                "Chris Evans Captain post CE",//
                "Chris Evans Captain post CE 2",//
              }));
  }
};

RenameHelperTest g_RenameHelperTest;
#include "RenameHelperTest.moc"

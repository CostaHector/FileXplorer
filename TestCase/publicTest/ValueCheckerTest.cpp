#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "ValueChecker.h"

using namespace ValueChecker;

class ValueCheckerTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:

  void GeneralIntChecker_ok() {
    QVERIFY(!GeneralIntChecker(QVariant{}));

    QVERIFY(GeneralIntChecker(INT_MIN));
    QVERIFY(GeneralIntChecker(0));
    QVERIFY(GeneralIntChecker(INT_MAX));

    QVERIFY(!(GeneralIntRangeChecker<0, 10>(11)));
    QVERIFY(!(GeneralIntRangeChecker<0, 10>(-1)));
    QVERIFY((GeneralIntRangeChecker<0, 10>(0)));
    QVERIFY((GeneralIntRangeChecker<0, 10>(9)));
    QVERIFY((GeneralIntRangeChecker<0, 10>(10)));

    QVERIFY(!(GeneralIntRangeChecker<0, 16>(-1)));
    QVERIFY(!(GeneralIntRangeChecker<0, 16>(17)));
    QVERIFY((GeneralIntRangeChecker<0, 16>(0)));
    QVERIFY((GeneralIntRangeChecker<0, 16>(16)));

    QVERIFY(!(GeneralIntRangeChecker<0, 100>(-1)));
    QVERIFY(!(GeneralIntRangeChecker<0, 100>(101)));
    QVERIFY((GeneralIntRangeChecker<0, 100>(0)));
    QVERIFY((GeneralIntRangeChecker<0, 100>(100)));

    QVERIFY(!(GeneralIntRangeChecker<0, 255>(-1)));
    QVERIFY(!(GeneralIntRangeChecker<0, 255>(257)));
    QVERIFY((GeneralIntRangeChecker<0, 255>(0)));
    QVERIFY(!(GeneralIntRangeChecker<0, 255>(256)));

    QVERIFY(!(GeneralIntRangeChecker<0, 500>(-1)));
    QVERIFY(!(GeneralIntRangeChecker<0, 500>(501)));
    QVERIFY((GeneralIntRangeChecker<0, 500>(0)));
    QVERIFY((GeneralIntRangeChecker<0, 500>(500)));

    QVERIFY(!(GeneralIntRangeChecker<0, INT_MAX>(-1)));
    QVERIFY(!(GeneralIntRangeChecker<0, INT_MAX>((long long) INT_MAX + 1)));
    QVERIFY((GeneralIntRangeChecker<0, INT_MAX>(0)));
    QVERIFY((GeneralIntRangeChecker<0, INT_MAX>(INT_MAX)));
  }

  void path_ok() {
    QVERIFY(!GeneralFilePathChecker(QVariant()));
    QVERIFY(!GeneralFolderPathChecker(QVariant()));

    QVERIFY(!GeneralFilePathChecker(""));
    QVERIFY(!GeneralFolderPathChecker(""));

    QVERIFY(!GeneralFilePathChecker("Not/A/Path"));
    QVERIFY(!GeneralFolderPathChecker("Not/A/Folder"));

    const QFileInfo fi{__FILE__};
    const QString filePath = fi.absoluteFilePath();
    const QString folderPath = fi.absolutePath();

    QVERIFY(GeneralFilePathChecker(filePath));
    QVERIFY(GeneralFolderPathChecker(folderPath));

    QVERIFY(!GeneralFilePathChecker(folderPath));
    QVERIFY(!GeneralFolderPathChecker(filePath));
  }

  void GeneralSequenceChecker_ok() {
    QVERIFY(!GeneralSequenceChecker(QVariant{}));
    QVERIFY(!GeneralSequenceChecker(QVariant{""}));

    QVERIFY(!GeneralSequenceChecker(QVariant{"01234567890"})); // 超过10个元素
    QVERIFY(!GeneralSequenceChecker(QVariant{"00000111110"})); // 有重复
    QVERIFY(!GeneralSequenceChecker(QVariant{"01234a56"}));    // 有非数值

    QVERIFY(GeneralSequenceChecker(QVariant{"0"}));
    QVERIFY(GeneralSequenceChecker(QVariant{"049"}));
    QVERIFY(GeneralSequenceChecker(QVariant{"8614"}));
  }

  void other_ok() {
    QVERIFY(!GeneralCharChecker(QVariant{}));

    QVERIFY(!GeneralCStrChecker(QVariant{}));
    QVERIFY(GeneralCStrChecker("ValidCStr"));

    QVERIFY(!GeneralDoubleChecker(QVariant{}));
    QVERIFY(!GeneralDoubleChecker("NotADouble"));
    QVERIFY(GeneralDoubleChecker(3.14159));

    QVERIFY(!GeneralBoolChecker(QVariant{}));
    QVERIFY(GeneralBoolChecker(true));
    QVERIFY(GeneralBoolChecker(false));
  }
};

#include "ValueCheckerTest.moc"
REGISTER_TEST(ValueCheckerTest, false)

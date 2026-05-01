#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "ValueChecker.h"
#include "KV.h"

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


  void toVariant_ok() {
    bool isOk{false};

    KV floatOk{"floatOk", RawVariant::Var{3.14f}, GeneralDataType::Type::PLAIN_FLOAT, nullptr, nullptr, nullptr, nullptr};
    float actualFloatValue = floatOk.toVariant().toFloat(&isOk);
    QVERIFY(isOk);
    double floatDiff{(double)actualFloatValue - 3.14};
    QVERIFY(std::abs(floatDiff) < 1E-6);

    isOk = false;
    KV doubleOk{"doubleOk", RawVariant::Var{3.14}, GeneralDataType::Type::PLAIN_DOUBLE, nullptr, nullptr, nullptr, nullptr};
    double actualDoubleValue = doubleOk.toVariant().toDouble(&isOk);
    QVERIFY(isOk);
    double doubleDiff{actualDoubleValue - 3.14};
    QVERIFY(std::abs(doubleDiff) < 1E-6);

    KV charOk{"charOk", RawVariant::Var{char{'A'}}, GeneralDataType::Type::PLAIN_CHAR, nullptr, nullptr, nullptr, nullptr};
    char actualCharValue = charOk.toVariant().value<char>();
    QCOMPARE(actualCharValue, 'A');

    isOk = false;
    KV longlongOk{"longlongOk", RawVariant::Var{100LL}, GeneralDataType::Type::PLAIN_LONGLONG, nullptr, nullptr, nullptr, nullptr};
    long long actualLongLongValue = longlongOk.toVariant().toLongLong(&isOk);
    QCOMPARE(100, actualLongLongValue);
  }
};

#include "ValueCheckerTest.moc"
REGISTER_TEST(ValueCheckerTest, false)

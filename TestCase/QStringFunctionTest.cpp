#include <QtTest>
#include <QCoreApplication>

// add necessary includes here

class QStringFunctionTest : public QObject
{
    Q_OBJECT

public:
    void toUpper();
    void capitalizer();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_regexReplace();
};

void QStringFunctionTest::initTestCase()
{

}

void QStringFunctionTest::cleanupTestCase()
{

}

void QStringFunctionTest::toUpper()
{
    QString s("abc123ABC!@#");
    QCOMPARE(s.toUpper(), QString("ABC123ABC!@#"));
}

void QStringFunctionTest::capitalizer(){
    QCOMPARE(1, 2);
}

#include <QRegExp>
void QStringFunctionTest::test_regexReplace(){
    QRegExp continousBlank("\\s\\s+");
    QString fileNameWithBlank("01 2  3   4    5     .txt");
    QString expectFileName("01 2 3 4 5 .txt");
    QString afterFileName = fileNameWithBlank.replace(continousBlank, " ");
    QCOMPARE(afterFileName, expectFileName);
}


//QTEST_MAIN(QStringFunctionTest)

#include "QStringFunctionTest.moc"

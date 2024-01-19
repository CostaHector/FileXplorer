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
    void test_folderDirname();
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

void QStringFunctionTest::test_folderDirname() {
#ifdef _WIN32
    QFileInfo cUsers("C:/Users");
    QVERIFY2(not cUsers.isRoot(), "[C:/Users] is not root.");

    QFileInfo cDisk("C:/");
    QVERIFY2(cDisk.isRoot(), "[C:/] is root.");
    QCOMPARE(cDisk.absolutePath(), QString("C:/"));
#else
    QFileInfo linuxHomePath("/home/path");
    QVERIFY2(not linuxHomePath.isRoot(), "[/home/path] is not root.");
    QCOMPARE(linuxHomePath.absolutePath(), QString("/home"));
#endif
}


//QTEST_MAIN(QStringFunctionTest)

// #include "QStringFunctionTest.moc"

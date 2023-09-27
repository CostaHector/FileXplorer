#include <QtTest>
#include <QCoreApplication>

// add necessary includes here


class FileOperationTest : public QObject
{
    Q_OBJECT

public:
    FileOperationTest();
    ~FileOperationTest();


private slots:
    void initTestCase();
    void cleanupTestCase();
    void toUpper();
    void capitalizer();
    void testCurrentFileSplitDirName();
};

FileOperationTest::FileOperationTest()
{

}

FileOperationTest::~FileOperationTest()
{

}

void FileOperationTest::initTestCase()
{

}

void FileOperationTest::cleanupTestCase()
{

}

void FileOperationTest::toUpper()
{
    QString s("abc123ABC!@#");
    QCOMPARE(s.toUpper(), QString("ABC123ABC!@#"));
}

void FileOperationTest::capitalizer(){
    QCOMPARE(1, 1);
}

#include <QFileInfo>
#include "FileOperation/FileOperation.h"

void FileOperationTest::testCurrentFileSplitDirName()
{
    QFileInfo fi(__FILE__);
    const QPair<QString, QString>& dirAndName = FileOperation::SplitDirName(fi.absoluteFilePath());
    QCOMPARE(dirAndName.first, fi.absolutePath());
    QCOMPARE(dirAndName.second, fi.fileName());
}

QTEST_MAIN(FileOperationTest)

#include "FileOperationTest.moc"

#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "MovieDbHelper.h"

#include "SqlRecordTestHelper.h"

class MovieDbHelperTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void GetDetailHtml_ok() {
    QString sampleMd5{"01234567890123456789012345678901"}; // 32 char(s)
    QString prePathLeft{"C:"}, otherPrePathLeft{"D:"};
    QString prePathRight{"/Captain America DVD/"};
    QString name{"file 1.VOB"};
    qint64 sz{1024};
    QSqlRecord record1 = SqlRecordTestHelper::GetAMovieRecordFromDb(sampleMd5, prePathLeft, prePathRight, name, sz);
    QSqlRecord record2 = SqlRecordTestHelper::GetAMovieRecordFromDb(sampleMd5, otherPrePathLeft, prePathRight, name, sz);

    // 相同文件名, 不同文件夹, 标题中可以区分二者
    QString win1TitleStr = MovieDbHelper::GetWindowTitleName(record1);
    QVERIFY(win1TitleStr.contains("file 1.VOB"));
    QString win2TitleStr = MovieDbHelper::GetWindowTitleName(record2);
    QVERIFY(win2TitleStr.contains("file 1.VOB"));
    QVERIFY(win1TitleStr != win2TitleStr);

    QString htmlSource = MovieDbHelper::GetDetailHtml(record1);
    QVERIFY(htmlSource.contains(prePathLeft));
    QVERIFY(htmlSource.contains(prePathRight));
    QVERIFY(htmlSource.contains(name));
    QVERIFY(htmlSource.contains("0'0'1'0"));
    QVERIFY(htmlSource.contains("SampleMD5"));
    QVERIFY(htmlSource.contains(sampleMd5));
    QVERIFY(htmlSource.contains("Detail"));
  }
};

#include "MovieDbHelperTest.moc"
REGISTER_TEST(MovieDbHelperTest, false)
#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "FileDescriptor.h"
#include "EndToExposePrivateMember.h"
#include "MD5Calculator.h"
#include "JsonKey.h"
#include "TDir.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class FileDescriptorTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
 private slots:
  void initTestCase() {
    QStringList casts;
    QString studios;
    QStringList tags;
    QString uploaded;
    int rates{0};
    QString emptyKakaMd5;
    QString validVaraneMd5{"AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD"};
    bool bContentsFixed{false};
    QString details;

    QByteArray kakaJsonBA = JsonKey::ConstructJsonByteArray("Kaka", casts, studios, tags, uploaded, rates,  //
                                                            0, "", "", {}, 0, {}, "",                       //
                                                            bContentsFixed, emptyKakaMd5, details);
    QByteArray varaneJsonBA = JsonKey::ConstructJsonByteArray("Kaka", casts, studios, tags, uploaded, rates,  //
                                                              0, "", "", {}, 0, {}, "",                       //
                                                              bContentsFixed, validVaraneMd5, details);
    QList<FsNodeEntry> nodes{
        {"Cristiano Ronaldo.mp4", false, "abc"},  // not json correspond
        {"Kaka.mp4", false, "cde"},                  // json correspond, but md5 value is empty
        {"Kaka.json", false, kakaJsonBA},         //
        {"Varane.mp4", false, ""},                // json correspond, md5 value valid
        {"Varane.json", false, varaneJsonBA},     //
    };
    QVERIFY(mTDir.IsValid());
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());
  }

  void init() {  //
    GlobalMockObject::reset();
  }
  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void GetFileUniquedId_ok() {
    using namespace FileDescriptor;
    MOCKER(MD5Calculator::GetFileMD5)
        .expects(exactly(2))                                                                                  //
        .will(returnValue(QByteArray{"DDDDDDDDCCCCCCCCBBBBBBBBAAAAAAAA"}))                                    //
        .then(returnValue(QByteArray{"ZZZZZZZZCCCCCCCCBBBBBBBBAAAAAAAA"}));                                   //
    QCOMPARE(GetFileUniquedId(mTDir.itemPath("Cristiano Ronaldo.mp4")), "DDDDDDDDCCCCCCCCBBBBBBBBAAAAAAAA");  // 1st
    QCOMPARE(GetFileUniquedId(mTDir.itemPath("Kaka.mp4")), "ZZZZZZZZCCCCCCCCBBBBBBBBAAAAAAAA");               // 2st
    QCOMPARE(GetFileUniquedId(mTDir.itemPath("Varane.mp4")), "AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD");             // from json
  }

  void GetFileUniquedIds_ok() {
    // not through json at all
    MOCKER(MD5Calculator::GetFileMD5)
        .expects(exactly(3))                                                 //
        .will(returnValue(QByteArray{"DDDDDDDDCCCCCCCCBBBBBBBBAAAAAAAA"}))   //
        .then(returnValue(QByteArray{"ZZZZZZZZCCCCCCCCBBBBBBBBAAAAAAAA"}))   //
        .then(returnValue(QByteArray{"YYYYYYYYCCCCCCCCBBBBBBBBAAAAAAAA"}));  //

    using namespace FileDescriptor;
    const QStringList files{mTDir.itemPath("Cristiano Ronaldo.mp4"), mTDir.itemPath("Kaka.mp4"), mTDir.itemPath("Varane.mp4")};
    QList<QByteArray> md5s = GetFileUniquedIds(files);
    QList<QByteArray> expectMd5s{"DDDDDDDDCCCCCCCCBBBBBBBBAAAAAAAA", "ZZZZZZZZCCCCCCCCBBBBBBBBAAAAAAAA", "YYYYYYYYCCCCCCCCBBBBBBBBAAAAAAAA"};
    QCOMPARE(md5s, expectMd5s);
  }
};

#include "FileDescriptorTest.moc"
REGISTER_TEST(FileDescriptorTest, false)

#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "TagsHelper.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"

#include "FileTool.h"
#include <QSignalSpy>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class TagsHelperTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void initTestCase() {
    TDir mTDir;
    QVERIFY(mTDir.IsValid());
    QCOMPARE(2, mTDir.createEntries(QList<FsNodeEntry>{
      FsNodeEntry{"Documentary.svg", false, "Documentary"},
      FsNodeEntry{"Science.ico", false, "Science"},
    }));
    // check point 1. key is the lowercase of image file BaseName
    const QMap<QString, QString> tagID2ImgFullPath{
      {"documentary", mTDir.itemPath("Documentary.svg")},
      {"science", mTDir.itemPath("Science.ico")},
    };
    QCOMPARE(TagsHelper::GetImgBaseName2FullPath(mTDir.path()), tagID2ImgFullPath);
  }

  void cleanupTestCase() {

  }

  void init() {  //
    GlobalMockObject::reset();
  }
  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void ParseALine_ok() {
    // 1 succeed
    {
      TagDefine validTag;
      QVERIFY(!validTag);
      QVERIFY(validTag = TagDefine::ParseALine("Chris Evens"));
      QCOMPARE(validTag, (TagDefine{"Chris Evens","Chris Evens","", 0}));
      QCOMPARE(validTag.tagID(), "Chris Evens");
      QCOMPARE(validTag.address, nullptr);

      QVERIFY(validTag = TagDefine::ParseALine("Chris Evens,ce"));
      QCOMPARE(validTag, (TagDefine{"Chris Evens","ce","", 0}));
      QCOMPARE(validTag.tagID(), "Chris Evens");
      QCOMPARE(validTag.address, nullptr);

      QVERIFY(validTag = TagDefine::ParseALine("Chris Evens,ce,Captain American,1"));
      QCOMPARE(validTag, (TagDefine{"Chris Evens","ce","Captain American", 1}));
      QCOMPARE(validTag.tagID(), "Chris Evens");
      QCOMPARE(validTag.address, nullptr);
      QCOMPARE(validTag.GetToolTip(), "<b>Chris Evens</b> Captain American<br/>Used count:1");

      const QAction* pAct1 = validTag.InitAction({}, nullptr);
      QCOMPARE(pAct1, validTag.address);
      QVERIFY(pAct1 != nullptr);

      // init again
      const QAction* pAct2 = validTag.InitAction({}, nullptr);
      QCOMPARE(pAct2, pAct1);
    }
    // 2. failed
    {
      TagDefine inValidTag;
      QVERIFY(!inValidTag);
      QVERIFY(!(inValidTag = TagDefine::ParseALine(""))); // empty line
      QVERIFY(!(inValidTag = TagDefine::ParseALine(",ce,Captain American,1"))); // TagID aka textFullName empty
      QVERIFY(!(inValidTag = TagDefine::ParseALine("Chris Evens,ce,Captain American"))); // 3 column failed
      QCOMPARE(inValidTag.tagID(), "");
      QCOMPARE(inValidTag.address, nullptr);
    }

    // ParseAFile ok
    {
      bool bReadInexistCSVFailed = false;
      MOCKER(FileTool::StringTextReader).expects(exactly(1))//
          .with(eq(QString{"inexist csv file path"}), outBoundP(&bReadInexistCSVFailed, sizeof(bReadInexistCSVFailed)), eq(true))
          .will(returnValue(QString{""})) // call1
          .id("ReadInexistCSVFailed"); // don't use ignoreReturnValue();

      bool bReadExistCSVOk = true;
      MOCKER(FileTool::StringTextReader).expects(exactly(2))//
          .with(eq(QString{"exist csv file path"}), outBoundP(&bReadExistCSVOk, sizeof(bReadExistCSVOk)), eq(true))
          .after("ReadInexistCSVFailed")
          .will(returnValue(QString{""})) // call2 return empty contents
          .then(returnValue(QString{
                                    R"(textFullName,textAbbr,toolTip,accessCount

Jane Grey
Hugh Jackman,hj
Chris Evens,ce,Captain American,1
Chris Hemsworth,ch,Thunder Thor,2

)"})) // call3
          .id("bReadExistCSVOk");

      const QList<TagDefine> defaultTags{TagDefine{"Documentary"},
                                         TagDefine{"Superhero"},
                                         TagDefine{"Comedy"},
                                         };
      QCOMPARE(TagDefine::ParseAFile("inexist csv file path"), defaultTags); // call1

      const QList<TagDefine> tagsInEmptyCsvFile;
      QCOMPARE(TagDefine::ParseAFile("exist csv file path"), tagsInEmptyCsvFile); // call2
      const QList<TagDefine> valid4Tags{TagDefine{"Jane Grey"},
                                        TagDefine{"Hugh Jackman", "hj"},
                                        TagDefine{"Chris Evens","ce","Captain American", 1},
                                        TagDefine{"Chris Hemsworth","ch","Thunder Thor", 2},
                                        };
      QCOMPARE(TagDefine::ParseAFile("exist csv file path"), valid4Tags); // call3
    }
  }

  void UpdateTagsActionCheckedStatus_ok_onActionGroupTriggered_ok() {
    const QList<TagDefine> valid4Tags{TagDefine{"Documentary"},
                                      TagDefine{"Comedy"},
                                      TagDefine{"Fiction"},
                                      TagDefine{"Novel"},
                                      };
    constexpr int TAG_COUNT = 4;
    QCOMPARE(valid4Tags.size(), TAG_COUNT);

    const QMap<QString, QString> tagID2ImgFullPath;
    MOCKER(TagsHelper::GetImgBaseName2FullPath)
        .stubs()
        .will(returnValue(tagID2ImgFullPath));
    MOCKER(TagDefine::ParseAFile)
        .stubs()
        .will(returnValue(valid4Tags));

    TagsHelper tagHelper{"tag csv files", nullptr};
    QCOMPARE(tagHelper.mTags.size(), TAG_COUNT);
    QCOMPARE(tagHelper.mTagActionList.size(), TAG_COUNT);

    QVERIFY(tagHelper.mCurrentCheckedActions.isEmpty());
    QCOMPARE(tagHelper.mTagID2Action.size(), TAG_COUNT);

    QVERIFY(tagHelper.mTagsAG != nullptr);
    QCOMPARE(tagHelper.mTagsAG->exclusionPolicy(), QActionGroup::ExclusionPolicy::None);


    QAction* pDocumentary = tagHelper.mTagID2Action["Documentary"];
    QAction* pComedy = tagHelper.mTagID2Action["Comedy"];
    QAction* pFiction = tagHelper.mTagID2Action["Fiction"];
    QAction* pNovel = tagHelper.mTagID2Action["Novel"];
    QVERIFY(pDocumentary != nullptr);
    QVERIFY(pComedy != nullptr);
    QVERIFY(pFiction != nullptr);
    QVERIFY(pNovel != nullptr);
    const auto GetCheckedStates = [pDocumentary, pComedy, pFiction, pNovel]() -> std::array<bool, TAG_COUNT> {
      return {pDocumentary->isChecked(),
              pComedy->isChecked(),
              pFiction->isChecked(),
              pNovel->isChecked(),
              };
    };
    QCOMPARE(GetCheckedStates(), (std::array<bool, TAG_COUNT>{false, false, false, false}));
    QCOMPARE(tagHelper.mCurrentCheckedActions.size(), 0);

    // UpdateTagsActionCheckedStatus_ok
    {
      QCOMPARE(tagHelper.UpdateTagsActionCheckedStatus(QStringList{"Documentary", "Comedy"}), std::make_pair(0, 2));
      QCOMPARE(GetCheckedStates(), (std::array<bool, TAG_COUNT>{true, true, false, false}));
      QCOMPARE(tagHelper.mCurrentCheckedActions.size(), 2);

      QCOMPARE(tagHelper.UpdateTagsActionCheckedStatus(QStringList{"Documentary", "Comedy"}), std::make_pair(0, 0));
      QCOMPARE(GetCheckedStates(), (std::array<bool, TAG_COUNT>{true, true, false, false}));
      QCOMPARE(tagHelper.mCurrentCheckedActions.size(), 2);

      QCOMPARE(tagHelper.UpdateTagsActionCheckedStatus(QStringList{"Documentary", "Comedy", "Fiction", "Novel"}), std::make_pair(0, 2));
      QCOMPARE(GetCheckedStates(), (std::array<bool, TAG_COUNT>{true, true, true, true}));
      QCOMPARE(tagHelper.mCurrentCheckedActions.size(), 4);

      QCOMPARE(tagHelper.UpdateTagsActionCheckedStatus(QStringList{"Documentary"}), std::make_pair(3, 0));
      QCOMPARE(GetCheckedStates(), (std::array<bool, TAG_COUNT>{true, false, false, false}));
      QCOMPARE(tagHelper.mCurrentCheckedActions.size(), 1);

      QCOMPARE(tagHelper.UpdateTagsActionCheckedStatus(QStringList{}), std::make_pair(1, 0));
      QCOMPARE(GetCheckedStates(), (std::array<bool, TAG_COUNT>{false, false, false, false}));
      QCOMPARE(tagHelper.mCurrentCheckedActions.size(), 0);
    }

    QCOMPARE(GetCheckedStates(), (std::array<bool, TAG_COUNT>{false, false, false, false}));
    QCOMPARE(tagHelper.mCurrentCheckedActions.size(), 0);

    // onActionGroupTriggered_ok
    {
      QSignalSpy reqAddRmvTagsSpy{&tagHelper, &TagsHelper::reqAddRmvTags};

      QVERIFY(!tagHelper.onActionGroupTriggered(nullptr)); // nullptr. ignore
      QCOMPARE(reqAddRmvTagsSpy.count(), 0);

      QAction noPropertyAct;
      QVERIFY(!tagHelper.onActionGroupTriggered(&noPropertyAct)); // no TagID property. ignore
      QCOMPARE(reqAddRmvTagsSpy.count(), 0);

      // check pDocumentary
      QVERIFY(!pDocumentary->isChecked());
      pDocumentary->setChecked(true);
      QVERIFY(tagHelper.onActionGroupTriggered(pDocumentary));
      QCOMPARE(reqAddRmvTagsSpy.count(), 1);
      QCOMPARE(reqAddRmvTagsSpy.takeLast(), (QVariantList{"Documentary", true}));
      QCOMPARE(GetCheckedStates(), (std::array<bool, TAG_COUNT>{true, false, false, false}));
      QCOMPARE(tagHelper.mCurrentCheckedActions.size(), 1);

      // check pComedy
      QVERIFY(!pComedy->isChecked());
      pComedy->setChecked(true);
      QVERIFY(tagHelper.onActionGroupTriggered(pComedy));
      QCOMPARE(reqAddRmvTagsSpy.count(), 1);
      QCOMPARE(reqAddRmvTagsSpy.takeLast(), (QVariantList{"Comedy", true}));
      QCOMPARE(GetCheckedStates(), (std::array<bool, TAG_COUNT>{true, true, false, false}));
      QCOMPARE(tagHelper.mCurrentCheckedActions.size(), 2);

      // uncheck pDocumentary
      QVERIFY(pDocumentary->isChecked());
      pDocumentary->setChecked(false);
      QVERIFY(tagHelper.onActionGroupTriggered(pDocumentary));
      QCOMPARE(reqAddRmvTagsSpy.count(), 1);
      QCOMPARE(reqAddRmvTagsSpy.takeLast(), (QVariantList{"Documentary", false}));
      QCOMPARE(GetCheckedStates(), (std::array<bool, TAG_COUNT>{false, true, false, false}));
      QCOMPARE(tagHelper.mCurrentCheckedActions.size(), 1);

      // uncheck pComedy
      QVERIFY(pComedy->isChecked());
      pComedy->setChecked(false);
      QVERIFY(tagHelper.onActionGroupTriggered(pComedy));
      QCOMPARE(reqAddRmvTagsSpy.count(), 1);
      QCOMPARE(reqAddRmvTagsSpy.takeLast(), (QVariantList{"Comedy", false}));
      QCOMPARE(GetCheckedStates(), (std::array<bool, TAG_COUNT>{false, false, false, false}));
      QCOMPARE(tagHelper.mCurrentCheckedActions.size(), 0);
    }

    QCOMPARE(GetCheckedStates(), (std::array<bool, TAG_COUNT>{false, false, false, false}));
    QCOMPARE(tagHelper.mCurrentCheckedActions.size(), 0);
  }
};

#include "TagsHelperTest.moc"
REGISTER_TEST(TagsHelperTest, false)
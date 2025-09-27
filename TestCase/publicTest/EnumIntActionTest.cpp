#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "EnumIntAction.h"
#include "EndToExposePrivateMember.h"

#include <QListView>
#include "PreviewTypeTool.h"
#include "ViewTypeTool.h"
#include "StyleEnum.h"
#include "SearchTools.h"
#include "BytesRangeTool.h"
#include "DuplicateImagesHelper.h"
#include "FileStructurePolicy.h"
#include <QCryptographicHash>

extern template struct EnumIntAction<QListView::Flow>;
extern template struct EnumIntAction<PreviewTypeTool::PREVIEW_TYPE_E>;
extern template struct EnumIntAction<ViewTypeTool::ViewType>;
extern template struct EnumIntAction<Style::StyleE>;
extern template struct EnumIntAction<Style::StyleSheetE>;
extern template struct EnumIntAction<SearchTools::SearchModeE>;
extern template struct EnumIntAction<BytesRangeTool::BytesRangeE>;
extern template struct EnumIntAction<QCryptographicHash::Algorithm>;
extern template struct EnumIntAction<DuplicateImageDetectionCriteria::DICriteriaE>;
extern template struct EnumIntAction<FileStructurePolicy::FileStuctureModeE>;

class EnumIntActionTest : public PlainTestSuite {
  Q_OBJECT
public:
  template<typename ENUM_T>
  void EnumIntActionChecker(ENUM_T firstEnum, ENUM_T secondEnum, ENUM_T whenInvalidEnum, QActionGroup::ExclusionPolicy exclusivePlcy) {
    QAction pAct1{"ascii b large"};
    pAct1.setCheckable(true);
    QAction pAct2{"ascii a less"};
    QVERIFY(pAct1.text() > pAct2.text());
    pAct2.setCheckable(true);
    QVERIFY(firstEnum < secondEnum);
    if (exclusivePlcy == QActionGroup::ExclusionPolicy::Exclusive) {
      pAct2.setChecked(true);
    }

    EnumIntAction<ENUM_T> anyEnumIntAction;
    QCOMPARE(anyEnumIntAction, false);
    anyEnumIntAction.init({{&pAct1, firstEnum}, {&pAct2, secondEnum}}, whenInvalidEnum, exclusivePlcy);
    QCOMPARE(anyEnumIntAction, true);

    QCOMPARE(anyEnumIntAction.defVal(), whenInvalidEnum);
    if (exclusivePlcy == QActionGroup::ExclusionPolicy::Exclusive) {
      QCOMPARE(anyEnumIntAction.curVal(), secondEnum);
    } else {
      QCOMPARE(anyEnumIntAction.curVal(), whenInvalidEnum);
    }

    auto* pActGrp = anyEnumIntAction.getActionGroup();
    QVERIFY(pActGrp != nullptr);
    // it's element sequence depend on init input parament container type and enum value. here we don't expect its order
    QCOMPARE(pActGrp->actions().size(), 2);
    QCOMPARE(anyEnumIntAction.getActionEnumAscendingList(), (QList<QAction*>{&pAct1, &pAct2}));
    QCOMPARE(anyEnumIntAction.getActionNameAscendingList(), (QList<QAction*>{&pAct2, &pAct1}));

    QCOMPARE(anyEnumIntAction.act2Enum(&pAct1), firstEnum);
    QCOMPARE(anyEnumIntAction.act2Enum((const QAction*) &pAct1), firstEnum);
    QCOMPARE(anyEnumIntAction.intVal2Enum((int) firstEnum), firstEnum);

    QCOMPARE(anyEnumIntAction.act2Enum(&pAct2), secondEnum);
    QCOMPARE(anyEnumIntAction.act2Enum((const QAction*) &pAct2), secondEnum);
    QCOMPARE(anyEnumIntAction.intVal2Enum((int) secondEnum), secondEnum);

    QCOMPARE(anyEnumIntAction.setCheckedIfActionExist((int) secondEnum), &pAct2);
    QCOMPARE(anyEnumIntAction.setCheckedIfActionExist(secondEnum), &pAct2);
    QCOMPARE(anyEnumIntAction.curVal(), secondEnum);
  }

private slots:
  void enum_in_action_exclusive() {
    EnumIntActionChecker<QListView::Flow>(QListView::Flow::LeftToRight,              //
                                          QListView::Flow::TopToBottom,              //
                                          QListView::Flow::LeftToRight,              //
                                          QActionGroup::ExclusionPolicy::Exclusive); //

    EnumIntActionChecker<ViewTypeTool::ViewType>( //
        ViewTypeTool::ViewType::TABLE,            //
        ViewTypeTool::ViewType::SEARCH,           //
        ViewTypeTool::ViewType::TABLE,            //
        QActionGroup::ExclusionPolicy::Exclusive);

    EnumIntActionChecker<Style::StyleE>(    //
        Style::StyleE::STYLE_WINDOWS_VISTA, //
        Style::StyleE::STYLE_FUSION,        //
        Style::StyleE::STYLE_WINDOWS_VISTA, //
        QActionGroup::ExclusionPolicy::Exclusive);

    EnumIntActionChecker<Style::StyleSheetE>(               //
        Style::StyleSheetE::STYLESHEET_DEFAULT_LIGHT,       //
        Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG, //
        Style::StyleSheetE::STYLESHEET_DEFAULT_LIGHT,       //
        QActionGroup::ExclusionPolicy::Exclusive);

    EnumIntActionChecker<SearchTools::SearchModeE>( //
        SearchTools::SearchModeE::REGEX,            //
        SearchTools::SearchModeE::FILE_CONTENTS,    //
        SearchTools::SearchModeE::REGEX,            //
        QActionGroup::ExclusionPolicy::Exclusive);

    EnumIntActionChecker<BytesRangeTool::BytesRangeE>( //
        BytesRangeTool::BytesRangeE::FIRST_16,         //
        BytesRangeTool::BytesRangeE::ENTIRE_FILE,      //
        BytesRangeTool::BytesRangeE::ENTIRE_FILE,      //
        QActionGroup::ExclusionPolicy::Exclusive);

    EnumIntActionChecker<QCryptographicHash::Algorithm>( //
        QCryptographicHash::Algorithm::Md5,              //
        QCryptographicHash::Algorithm::Sha256,           //
        QCryptographicHash::Algorithm::Md5,              //
        QActionGroup::ExclusionPolicy::Exclusive);

    EnumIntActionChecker<DuplicateImageDetectionCriteria::DICriteriaE>( //
        DuplicateImageDetectionCriteria::DICriteriaE::LIBRARY,          //
        DuplicateImageDetectionCriteria::DICriteriaE::MD5,              //
        DuplicateImageDetectionCriteria::DICriteriaE::LIBRARY,          //
        QActionGroup::ExclusionPolicy::Exclusive);

    EnumIntActionChecker<FileStructurePolicy::FileStuctureModeE>( //
        FileStructurePolicy::FileStuctureModeE::PRESERVE,          //
        FileStructurePolicy::FileStuctureModeE::FLATTEN,              //
        FileStructurePolicy::FileStuctureModeE::QUERY,          //
        QActionGroup::ExclusionPolicy::Exclusive);
  }

  void enum_in_action_exclusive_optional() {
    EnumIntActionChecker<PreviewTypeTool::PREVIEW_TYPE_E>( //
        PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY,         //
        PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD, //
        PreviewTypeTool::PREVIEW_TYPE_E::NONE,             //
        QActionGroup::ExclusionPolicy::ExclusiveOptional); //
  }
};

#include "EnumIntActionTest.moc"
REGISTER_TEST(EnumIntActionTest, false)

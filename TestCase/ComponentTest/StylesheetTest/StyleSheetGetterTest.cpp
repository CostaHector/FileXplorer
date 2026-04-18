#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "StyleSheetGetter.h"
#include "EndToExposePrivateMember.h"
#include "PreferenceActions.h"

#include "MemoryKey.h"
#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class StyleSheetGetterTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() { Configuration().clear(); }
  void cleanupTestCase() { Configuration().clear(); }

  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void FontCfg_ok() {
    MOCKER(PreferenceActions::ApplyNewStyleSheet).expects(exactly(1));

    using namespace FontCfg;
    QFont fromEmptySettingFont = ReadFont();

    QFont newFont{"Noto Sans", 18, QFont::Weight::Bold, true};  // bold and italic
    QVERIFY(newFont != fromEmptySettingFont);
    updateFont(newFont);  // 1st time call ApplyNewStyleSheet

    QFont fromSaved = ReadFont();
    QCOMPARE(fromSaved.family(), newFont.family());
    QCOMPARE(fromSaved.styleName(), newFont.styleName());
    QCOMPARE(fromSaved.pointSize(), newFont.pointSize());
    QCOMPARE(fromSaved.style(), newFont.style());
    QCOMPARE(GetFontWeightString(), "bold");
    QCOMPARE(GetFontStyleString(), "italic");

    // when settings is wrong, fall back to default value
    Configuration().setValue("StyleSheet/Font/Weight/General", 9999);  // invalid weight
    QCOMPARE(GetFontWeight(), QFont::Weight::Normal);

    Configuration().setValue("StyleSheet/Font/Style/General", 9999);  // invalid style
    QCOMPARE(GetFontStyle(), QFont::Style::StyleNormal);

    QCOMPARE(StyleSheetGetter::GetInst().GetStyleSheet(Style::StyleSheetE::STYLESHEET_LIGHT), "");
    QCOMPARE(StyleSheetGetter::GetInst().GetStyleSheet(Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG), "");
  }
};

#include "StyleSheetGetterTest.moc"
REGISTER_TEST(StyleSheetGetterTest, false)

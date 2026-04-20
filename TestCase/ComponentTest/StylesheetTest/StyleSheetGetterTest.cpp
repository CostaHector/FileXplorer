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

  void updateFont_will_writeSettings() {
    MOCKER(PreferenceActions::ApplyNewStyleSheet).expects(exactly(1));

    auto& inst = StyleSheetGetter::GetInst();
    QCOMPARE(inst.mStyleCfg.isEmpty(), false);
    // base class return empty
    QCOMPARE(inst.GetStyleSheet(Style::StyleSheetE::STYLESHEET_DEFAULT_NONE), "");
    QCOMPARE(inst.GetStyleSheet(Style::StyleSheetE::STYLESHEET_LIGHT), "");
    QCOMPARE(inst.GetStyleSheet(Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG), "");

    QFont fromEmptySettingFont = FontCfg::ReadFont();
    QFont newFont{"Noto Sans", 18, QFont::Weight::Bold, true}; // bold and italic
    QVERIFY(newFont != fromEmptySettingFont);

    int changedCnt = inst.updateGeneralFont(newFont); // 1st time call ApplyNewStyleSheet
    QVERIFY(changedCnt > 0);

    { // check if correct
      using namespace FontCfg;
      QFont fromSaved = ReadFont();
      QCOMPARE(fromSaved.family(), newFont.family());
      QCOMPARE(fromSaved.styleName(), newFont.styleName());
      QCOMPARE(fromSaved.pointSize(), newFont.pointSize());
      QCOMPARE(fromSaved.style(), newFont.style());

      QCOMPARE(GetFontWeightString(), "bold");
      QCOMPARE(GetFontStyleString(), "italic");

      QCOMPARE(GetFontFamily().isEmpty(), false);
      QCOMPARE(GetFontFamilyCode().isEmpty(), false);
    }

    // will also write into QSettings
    QCOMPARE(Configuration().contains("StyleSheet/Font/Family/General"), true);
    QCOMPARE(Configuration().contains("StyleSheet/Font/Size/General"), true);
    QCOMPARE(Configuration().contains("StyleSheet/Font/Weight/General"), true);
    QCOMPARE(Configuration().contains("StyleSheet/Font/Style/General"), true);
  }

  void WriteIntoSettings_ok() {
    Configuration().clear();
    const QSettings& settings = Configuration();

    auto& inst = StyleSheetGetter::GetInst();
    inst.WriteIntoSettings();

    QStringList expectKeys = inst.mStyleCfg.keys();
    std::sort(expectKeys.begin(), expectKeys.end());

    QStringList actualKeys = settings.allKeys();
    std::sort(actualKeys.begin(), actualKeys.end());

    QCOMPARE(actualKeys, expectKeys);
  }

  void UpdateCurValue_ok() {
    const auto& inst = StyleSheetGetter::GetInst();
    QCOMPARE(inst.UpdateCurValue({}), 0);

    // 从不新增键
    const QVariantHash totallyNotExistKey{
        {"totally/not/exist/key1", 1},
        {"totally/not/exist/key2", 2},
    };
    QCOMPARE(inst.UpdateCurValue(totallyNotExistKey), 0);

    // 只能修改键
    int updateCnt = inst.UpdateCurValue(QVariantHash{
        {"StyleSheet/Font/Weight/General", 9999}, // invalid weight
        {"StyleSheet/Font/Style/General", 9999},  // invalid style
    });
    QCOMPARE(updateCnt, 2);

    // when settings is wrong, fall back to default value
    QCOMPARE(FontCfg::GetFontWeight(), QFont::Weight::Normal);
    QCOMPARE(FontCfg::GetFontStyle(), QFont::Style::StyleNormal);
  }
};

#include "StyleSheetGetterTest.moc"
REGISTER_TEST(StyleSheetGetterTest, false)

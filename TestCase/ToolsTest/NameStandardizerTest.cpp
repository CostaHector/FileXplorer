#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "OnScopeExit.h"
#include "PlainTestSuite.h"
#include "NameStandardizer.h"

#include "BeginToExposePrivateMember.h"
#include "StudiosManager.h"
#include "EndToExposePrivateMember.h"

class NameStandardizerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  NameStandardizer ns;
 private slots:
  void test_remove_nbspInHtml() {
    QCOMPARE(ns("A B"), "A B");
  }
  void test_removeSpecialChar() {
    QCOMPARE(ns("A#’“”\"/:*?<>|B"), "A '''' B");
  }
  void test_RemoveContinuousSpace() {
    QCOMPARE(ns("A B"), "A B");
    QCOMPARE(ns("A       B"), "A B");
  }
  void test_RemoveLeadingAndTrailingSpace() {
    QCOMPARE(ns("      A"), "A");
    QCOMPARE(ns("A      "), "A");
    QCOMPARE(ns("      A           "), "A");
  }
  void test_RemoveSpaceBeforeDot() {
    QCOMPARE(ns("A .mp4"), "A.mp4");
  }
  void test_SpaceBeforeOrAfterComma();
  void test_SpaceBeforeOrAfterExclamationMark();
  void test_SpaceBeforeOrAfterHypen();
  void test_RemoveFixedLeading() {
    QCOMPARE(ns("[GT]A"), "A");
    QCOMPARE(ns("[FL] A"), "A");
    QCOMPARE(ns("[FFL] A"), "A");
    QCOMPARE(ns("[A"), "A");
    QCOMPARE(ns("[FFL] [MMM] Movie Name"), "MMM - Movie Name");
    QCOMPARE(ns("[FFL][MMM] Movie Name"), "MMM - Movie Name");
  }
  void test_RemoveMultiHypen();
  void test_RemoveBacket();
  void test_BasicFunc() {
    static auto& psm = StudiosManager::getInst();
    STUDIO_MGR_DATA_T tempStudios;
    tempStudios["marvelfilms"] = "MarvelFilms";
    tempStudios["marvel films"] = "MarvelFilms";
    tempStudios["realmadrid"] = "ReadMadrid";
    tempStudios["real madrid"] = "ReadMadrid";
    psm.ProStudioMap().swap(tempStudios);
    ON_SCOPE_EXIT {
      psm.ProStudioMap().swap(tempStudios);
    };
    QCOMPARE(ns("marvelfilms-1.mp4"), "MarvelFilms - 1.mp4");
    QCOMPARE(ns("marvel films-1.mp4"), "MarvelFilms - 1.mp4");
    QCOMPARE(ns("[marvel films]1.mp4"), "MarvelFilms - 1.mp4");
    QCOMPARE(ns("[marvel films-]-1.mp4"), "MarvelFilms - 1.mp4");
    QCOMPARE(ns("(marvel films)-1.mp4"), "MarvelFilms - 1.mp4");
    QCOMPARE(ns("(marvel films) 1.mp4"), "MarvelFilms - 1.mp4");
    QCOMPARE(ns("real madrid - 1.mp4"), "ReadMadrid - 1.mp4");
    QCOMPARE(ns("[real madrid]1.mp4"), "ReadMadrid - 1.mp4");
  }
};

void NameStandardizerTest::test_SpaceBeforeOrAfterComma() {
  QCOMPARE(ns("A, B"), "A, B");
  QCOMPARE(ns("A , B"), "A, B");
  QCOMPARE(ns("A ,B"), "A, B");
  QCOMPARE(ns("A,  B"), "A, B");
}
void NameStandardizerTest::test_SpaceBeforeOrAfterExclamationMark() {
  QCOMPARE(ns("A! B"), "A! B");
  QCOMPARE(ns("A ! B"), "A! B");
  QCOMPARE(ns("A !B"), "A! B");
  QCOMPARE(ns("A!  B"), "A! B");
}
void NameStandardizerTest::test_SpaceBeforeOrAfterHypen() {
  QCOMPARE(ns("A - B"), "A - B");
  QCOMPARE(ns("A  - B"), "A - B");
  QCOMPARE(ns("A -  B"), "A - B");
}

void NameStandardizerTest::test_RemoveMultiHypen() {
  // "---" -> " - ";
  QCOMPARE(ns("A---C"), "A - C");
  // "- " -> " - ";
  QCOMPARE(ns("A -C"), "A - C");
  // " -" -> " - ";
  QCOMPARE(ns("A- C"), "A - C");
  // " – " -> " - ";
  QCOMPARE(ns("A – C"), "A - C");
  QCOMPARE(ns("A- - -C"), "A - C");
  // "- ." -> " - ";
  QCOMPARE(ns("A- .C"), "A.C");
  QCOMPARE(ns("A - .C"), "A.C");
  QCOMPARE(ns("A - "), "A");
}
void NameStandardizerTest::test_RemoveBacket() {
  // leading open brackets {[( delete;
  QCOMPARE(ns("C [A"), "C - A");
  QCOMPARE(ns("C (A"), "C - A");
  QCOMPARE(ns("C {A"), "C - A");
  // close bracket to " - ";
  QCOMPARE(ns("C A]E"), "C A - E");
  QCOMPARE(ns("C A)E"), "C A - E");
  QCOMPARE(ns("C A}E"), "C A - E");
}

#include "NameStandardizerTest.moc"
REGISTER_TEST(NameStandardizerTest, false)

#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "OnScopeExit.h"
#include "MyTestSuite.h"
#include "NameStandardizer.h"
#include "StudiosManager.h"

class MovieNameStandardizerTest : public MyTestSuite {
  Q_OBJECT
 public:
  MovieNameStandardizerTest() : MyTestSuite{false} { }
  NameStandardizer ns;
 private slots:
  void test_removeSpecialChar();
  void test_RemoveContinuousSpace();
  void test_RemoveLeadingAndTrailingSpace();
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
  void test_RemoveSpaceBeforeDot();
  void test_BasicFunc() {
    static auto& psm = StudiosManager::getIns();
    decltype(psm.m_prodStudioMap) tempStudios;
    tempStudios["marvelfilms"] = "MarvelFilms";
    tempStudios["marvel films"] = "MarvelFilms";
    tempStudios["realmadrid"] = "ReadMadrid";
    tempStudios["real madrid"] = "ReadMadrid";
    psm.m_prodStudioMap.swap(tempStudios);
    ON_SCOPE_EXIT {
      psm.m_prodStudioMap.swap(tempStudios);
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

void MovieNameStandardizerTest::test_removeSpecialChar() {
  QCOMPARE(ns("A#’“”\"/:*?<>|B"), "A '''' B");
}
void MovieNameStandardizerTest::test_RemoveContinuousSpace() {
  QCOMPARE(ns("A B"), "A B");
  QCOMPARE(ns("A       B"), "A B");
}
void MovieNameStandardizerTest::test_RemoveLeadingAndTrailingSpace() {
  QCOMPARE(ns("      A"), "A");
  QCOMPARE(ns("A      "), "A");
  QCOMPARE(ns("      A           "), "A");
}
void MovieNameStandardizerTest::test_RemoveSpaceBeforeDot() {
  QCOMPARE(ns("A .mp4"), "A.mp4");
}
void MovieNameStandardizerTest::test_SpaceBeforeOrAfterComma() {
  QCOMPARE(ns("A, B"), "A, B");
  QCOMPARE(ns("A , B"), "A, B");
  QCOMPARE(ns("A ,B"), "A, B");
  QCOMPARE(ns("A,  B"), "A, B");
}
void MovieNameStandardizerTest::test_SpaceBeforeOrAfterExclamationMark() {
  QCOMPARE(ns("A! B"), "A! B");
  QCOMPARE(ns("A ! B"), "A! B");
  QCOMPARE(ns("A !B"), "A! B");
  QCOMPARE(ns("A!  B"), "A! B");
}
void MovieNameStandardizerTest::test_SpaceBeforeOrAfterHypen() {
  QCOMPARE(ns("A - B"), "A - B");
  QCOMPARE(ns("A  - B"), "A - B");
  QCOMPARE(ns("A -  B"), "A - B");
}

void MovieNameStandardizerTest::test_RemoveMultiHypen() {
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
void MovieNameStandardizerTest::test_RemoveBacket() {
  // leading open brackets {[( delete;
  QCOMPARE(ns("C [A"), "C - A");
  QCOMPARE(ns("C (A"), "C - A");
  QCOMPARE(ns("C {A"), "C - A");
  // close bracket to " - ";
  QCOMPARE(ns("C A]E"), "C A - E");
  QCOMPARE(ns("C A)E"), "C A - E");
  QCOMPARE(ns("C A}E"), "C A - E");
}

#include "MovieNameStandardizerTest.moc"
MovieNameStandardizerTest g_MovieNameStandardizerTest;

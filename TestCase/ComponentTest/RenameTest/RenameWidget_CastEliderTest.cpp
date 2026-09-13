#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RenameWidget_CastElider.h"
#include "EndToExposePrivateMember.h"

#include "Configuration.h"
#include "RenamerKey.h"

class RenameWidget_CastEliderTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void cleanupTestCase() { Configuration().clear(); }

  void init_ok() {
    Configuration().clear();

    {
      RenameWidget_CastElider ce;
      ce.init();
      QCOMPARE(ce.m_JsonBaseNameMaxLength, RenamerKey::JSON_BASENAME_DEFAULT_MAX_LENGTH.v.data.i);

      QVERIFY(ce.m_recursiveCB != nullptr);
      QVERIFY(!ce.m_recursiveCB->isEnabled());
      QVERIFY(!ce.m_recursiveCB->isChecked());

      // fixed: independent.
      QVERIFY(ce.m_nameExtIndependent != nullptr);
      QVERIFY(!ce.m_nameExtIndependent->isEnabled());
      QVERIFY(ce.m_nameExtIndependent->isChecked());

      QVERIFY(ce.m_maxNameLength != nullptr);
      QCOMPARE(ce.m_maxNameLength->text(), QString::number(RenamerKey::JSON_BASENAME_DEFAULT_MAX_LENGTH.v.data.i));

      // out of range will not crash
      QCOMPARE(ce.preAt(-1), "");
      QCOMPARE(ce.fileNameAt(-1), "");
      QCOMPARE(ce.relativePathAt(-1), "");

      QCOMPARE(ce.preAt(1), "");
      QCOMPARE(ce.fileNameAt(1), "");
      QCOMPARE(ce.relativePathAt(1), "");

      ce.m_maxNameLength->setText("999999999");
      QVERIFY(!ce.onJsonBaseNameMaxLengthChanged("999999999"));

      ce.m_maxNameLength->setText("notANumber");
      QVERIFY(!ce.onJsonBaseNameMaxLengthChanged("notANumber"));

      ce.m_maxNameLength->setText("201");
      QVERIFY(ce.onJsonBaseNameMaxLengthChanged("201")); // changed valid
      QCOMPARE(ce.m_JsonBaseNameMaxLength, 201);

      QVERIFY(!ce.onJsonBaseNameMaxLengthChanged("201")); // unchange
    }
    // save in destructor
    QCOMPARE(getConfig(RenamerKey::JSON_BASENAME_DEFAULT_MAX_LENGTH).toInt(), 201);
  }

  void onJsonBaseNameMaxLengthChanged_ok() {
    Configuration().clear();
    const QString currentPath{
#ifdef _WIN32
          "C:/home/random_path"
#else
          "/home/random_path"
#endif
    };
    QStringList relNames
        {
         // change {Chris Hemsworth, Natalie Portman}
         "pre/underpre/Marvel Studios - Thor.json",
         "pre/underpre/Marvel Studios - Thor_tn.jpg",
         // change {Chris Evans, Sebastian Stan}
         "pre/Marvel Studios - Captain America The First Avenger.json",
         "pre/Marvel Studios - Captain America The First Avenger 0.jpg",
         "pre/Marvel Studios - Captain America The First Avenger 1.png",
         "pre/Marvel Studios - Captain America The First Avenger.mp4",
         // change {Gwyneth Paltrow, Robert Downey Jr}
         "Marvel Studios - Iron Man.json",
         "Marvel Studios - Iron Man 0.jpg",
         "Marvel Studios - Iron Man 1.png",
         "Marvel Studios - Iron Man.mp4",
         // unchange {Robert Downey Jr, Chris Hemsworth, Chris Evans}
         "Marvel Studios - Avengers Infinity War (2018) - Robert Downey Jr, Chris Hemsworth, Chris Evans.json",
         };
    QMap<QString, QString> relatedFile2Json
        {
        {"pre/underpre/Marvel Studios - Thor.json", "pre/underpre/Marvel Studios - Thor.json"},
        {"pre/underpre/Marvel Studios - Thor_tn.jpg", "pre/underpre/Marvel Studios - Thor.json"},
        //
        {"pre/Marvel Studios - Captain America The First Avenger.json", "pre/Marvel Studios - Captain America The First Avenger.json"},
        {"pre/Marvel Studios - Captain America The First Avenger 0.jpg", "pre/Marvel Studios - Captain America The First Avenger.json"},
        {"pre/Marvel Studios - Captain America The First Avenger 1.png", "pre/Marvel Studios - Captain America The First Avenger.json"},
        {"pre/Marvel Studios - Captain America The First Avenger.mp4", "pre/Marvel Studios - Captain America The First Avenger.json"},
        //
        {"Marvel Studios - Iron Man.json", "Marvel Studios - Iron Man.json"},
        {"Marvel Studios - Iron Man 0.jpg", "Marvel Studios - Iron Man.json"},
        {"Marvel Studios - Iron Man 1.png", "Marvel Studios - Iron Man.json"},
        {"Marvel Studios - Iron Man.mp4", "Marvel Studios - Iron Man.json"},
        //
        {"Marvel Studios - Avengers Infinity War (2018) - Robert Downey Jr, Chris Hemsworth, Chris Evans.json", "Marvel Studios - Avengers Infinity War (2018) - Robert Downey Jr, Chris Hemsworth, Chris Evans.json"}
        };
    QMap<QString, QStringList> relativeJson2Cast
        {
         {"pre/underpre/Marvel Studios - Thor.json", QStringList{"Chris Hemsworth", "Natalie Portman"}},
         {"pre/Marvel Studios - Captain America The First Avenger.json", QStringList{"Chris Evans", "Sebastian Stan"}},
         {"Marvel Studios - Iron Man.json", QStringList{"Gwyneth Paltrow", "Robert Downey Jr"}},
         {"Marvel Studios - Avengers Infinity War (2018) - Robert Downey Jr, Chris Hemsworth, Chris Evans.json", QStringList{"Robert Downey Jr", "Chris Hemsworth", "Chris Evans"}},
         };

    // precondition. m_JsonBaseNameMaxLength long enough for all cast
    QVERIFY(sizeof("Marvel Studios - Avengers Infinity War (2018) - Robert Downey Jr, Chris Hemsworth, Chris Evans") < 200);
    QVERIFY(RenamerKey::JSON_BASENAME_DEFAULT_MAX_LENGTH.v.data.i > 200);

    RenameWidget_CastElider ce;
    ce.init();
    ce.initCoreName2CastListMap(relativeJson2Cast);
    ce.initRelatedFile2Json(relatedFile2Json);
    ce.InitTextEditContent(currentPath, relNames);
    QCOMPARE(ce.m_JsonBaseNameMaxLength, RenamerKey::JSON_BASENAME_DEFAULT_MAX_LENGTH.v.data.i);

    // batch count: 2 4 4 1
    // preAt ok
    {
      QCOMPARE(ce.preAt(0), "pre/underpre");
      QCOMPARE(ce.preAt(1), "pre/underpre");

      QCOMPARE(ce.preAt(2), "pre");
      QCOMPARE(ce.preAt(3), "pre");
      QCOMPARE(ce.preAt(4), "pre");
      QCOMPARE(ce.preAt(5), "pre");

      QCOMPARE(ce.preAt(6), "");
      QCOMPARE(ce.preAt(7), "");
      QCOMPARE(ce.preAt(8), "");
      QCOMPARE(ce.preAt(9), "");

      QCOMPARE(ce.preAt(10), "");
    }
    // fileNameAt ok
    {
      QCOMPARE(ce.fileNameAt(0), "Marvel Studios - Thor.json");
      QCOMPARE(ce.fileNameAt(1), "Marvel Studios - Thor_tn.jpg");

      QCOMPARE(ce.fileNameAt(2), "Marvel Studios - Captain America The First Avenger.json");
      QCOMPARE(ce.fileNameAt(3), "Marvel Studios - Captain America The First Avenger 0.jpg");
      QCOMPARE(ce.fileNameAt(4), "Marvel Studios - Captain America The First Avenger 1.png");
      QCOMPARE(ce.fileNameAt(5), "Marvel Studios - Captain America The First Avenger.mp4");

      QCOMPARE(ce.fileNameAt(6), "Marvel Studios - Iron Man.json");
      QCOMPARE(ce.fileNameAt(7), "Marvel Studios - Iron Man 0.jpg");
      QCOMPARE(ce.fileNameAt(8), "Marvel Studios - Iron Man 1.png");
      QCOMPARE(ce.fileNameAt(9), "Marvel Studios - Iron Man.mp4");

      QCOMPARE(ce.fileNameAt(10), "Marvel Studios - Avengers Infinity War (2018) - Robert Downey Jr, Chris Hemsworth, Chris Evans.json");
    }
    // relativePathAt ok
    {
      QCOMPARE(ce.relativePathAt(0), "pre/underpre/Marvel Studios - Thor.json");
      QCOMPARE(ce.relativePathAt(1), "pre/underpre/Marvel Studios - Thor_tn.jpg");

      QCOMPARE(ce.relativePathAt(2), "pre/Marvel Studios - Captain America The First Avenger.json");
      QCOMPARE(ce.relativePathAt(3), "pre/Marvel Studios - Captain America The First Avenger 0.jpg");
      QCOMPARE(ce.relativePathAt(4), "pre/Marvel Studios - Captain America The First Avenger 1.png");
      QCOMPARE(ce.relativePathAt(5), "pre/Marvel Studios - Captain America The First Avenger.mp4");

      QCOMPARE(ce.relativePathAt(6), "Marvel Studios - Iron Man.json");
      QCOMPARE(ce.relativePathAt(7), "Marvel Studios - Iron Man 0.jpg");
      QCOMPARE(ce.relativePathAt(8), "Marvel Studios - Iron Man 1.png");
      QCOMPARE(ce.relativePathAt(9), "Marvel Studios - Iron Man.mp4");

      QCOMPARE(ce.relativePathAt(10), "Marvel Studios - Avengers Infinity War (2018) - Robert Downey Jr, Chris Hemsworth, Chris Evans.json");
    }

    // init calue ok
    const QString& sNewName = ce.m_nBaseTE->toPlainText();
    const QString& sNewExt = ce.m_nExtTE->toPlainText();
    QCOMPARE(sNewExt,
             ".json\n.jpg\n"
             ".json\n.jpg\n.png\n.mp4\n"
             ".json\n.jpg\n.png\n.mp4\n"
             ".json");
    QCOMPARE(sNewName,
             "Marvel Studios - Thor - Chris Hemsworth, Natalie Portman\n"
             "Marvel Studios - Thor - Chris Hemsworth, Natalie Portman_tn\n"
             "Marvel Studios - Captain America The First Avenger - Chris Evans, Sebastian Stan\n"
             "Marvel Studios - Captain America The First Avenger - Chris Evans, Sebastian Stan 0\n"
             "Marvel Studios - Captain America The First Avenger - Chris Evans, Sebastian Stan 1\n"
             "Marvel Studios - Captain America The First Avenger - Chris Evans, Sebastian Stan\n"
             "Marvel Studios - Iron Man - Gwyneth Paltrow, Robert Downey Jr\n"
             "Marvel Studios - Iron Man - Gwyneth Paltrow, Robert Downey Jr 0\n"
             "Marvel Studios - Iron Man - Gwyneth Paltrow, Robert Downey Jr 1\n"
             "Marvel Studios - Iron Man - Gwyneth Paltrow, Robert Downey Jr\n"
             "Marvel Studios - Avengers Infinity War (2018) - Robert Downey Jr, Chris Hemsworth, Chris Evans");
  }
};

#include "RenameWidget_CastEliderTest.moc"
REGISTER_TEST(RenameWidget_CastEliderTest, false)

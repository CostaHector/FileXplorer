#include <QtTest>
#include <QCoreApplication>

// add necessary includes here

class PropertiesQString : public QObject
{
    Q_OBJECT

public:
    PropertiesQString();
    ~PropertiesQString();

    void toUpper();
    void capitalizer();

private slots:
    void initTestCase();
    void cleanupTestCase();
};

PropertiesQString::PropertiesQString()
{

}

PropertiesQString::~PropertiesQString()
{

}

void PropertiesQString::initTestCase()
{

}

void PropertiesQString::cleanupTestCase()
{

}

void PropertiesQString::toUpper()
{
    QString s("abc123ABC!@#");
    QCOMPARE(s.toUpper(), QString("ABC123ABC!@#"));
}

void PropertiesQString::capitalizer(){
    QCOMPARE(1, 2);
}

//QTEST_MAIN(PropertiesQString)

#include "SomeTestTemplate.moc"

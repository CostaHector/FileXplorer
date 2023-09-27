#ifndef SOMETESTTEMPLATE_H
#define SOMETESTTEMPLATE_H

#include <QtTest>

#include <QCoreApplication>

#include <QObject>

class SomeTestTemplate : public QObject
{
    Q_OBJECT
public:
    SomeTestTemplate();
    ~SomeTestTemplate();

    void addTest();
    void minusTest();

signals:

private slots:
    void initTestCase();
    void cleanupTestCase();
};

#endif // SOMETESTTEMPLATE_H

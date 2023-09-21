#ifndef ADDRESSELINEEDIT_H
#define ADDRESSELINEEDIT_H

#include <QWidget>
#include <QToolBar>
#include <QComboBox>

#include <QLineEdit>
#include <QAction>

#include <QFileInfo>
#include <QHBoxLayout>

class FocusEventWatch : public QObject
{
    Q_OBJECT
public:
    explicit FocusEventWatch(QObject *parent = nullptr);

    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void focusChanged(bool hasFocus);

private:
    bool mouseButtonPressedBefore = false;
};



class AddressELineEdit : public QToolBar
{
    Q_OBJECT
public:
    explicit AddressELineEdit(QWidget *parent = nullptr);

    auto text()->QString{
        return pathLineEdit->text();
    }

    static inline auto PathProcess(const QString&path) -> QString;
    auto onFocusChange(bool hasFocus) -> void;

    virtual void mousePressEvent(QMouseEvent *event) override;

    auto clickMode()->void;
    auto inputMode()->void;
    auto subscribe()->void;
    auto onPathActionTriggered(const QAction* clkAct)->void;
    auto onReturnPressed(const QString& path)->bool;

    inline auto dirname() -> QString{
        QFileInfo fi(text());
        if (fi.isRoot()){
            return "";
        }
        return fi.absolutePath();
    }


signals:
    void intoAPath_active(const QString&);

public slots:
    void pathChangeTo(const QString& newPath);

private:
    QActionGroup* pathActionsGroup;

    QLineEdit* pathLineEdit;

    QComboBox* pathComboBox;
    FocusEventWatch* pathComboBoxFocusWatcher;

    QAction* addressCBActH;
};

constexpr int CONTROL_HEIGHT = 28;


class TestAddressELineEdit: public QWidget{
    Q_OBJECT
public:
    explicit TestAddressELineEdit(QWidget* parent=nullptr): QWidget(parent){
        AddressELineEdit* add = new AddressELineEdit;

        QLineEdit *searchLe = new QLineEdit("Search here");
        searchLe->setClearButtonEnabled(true);

        searchLe->addAction(QIcon(":/themes/SEARCH"), QLineEdit::LeadingPosition);
        searchLe->setPlaceholderText("Search...");

        QHBoxLayout* lo = new QHBoxLayout;
        lo->addWidget(add);
        lo->addWidget(searchLe);
        setLayout(lo);

        add->setMinimumWidth(400);
        searchLe->setMinimumWidth(100);

        add->pathChangeTo(QFileInfo(__FILE__).absolutePath());
    }
signals:
};

#endif // ADDRESSELINEEDIT_H

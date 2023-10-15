#include "Toaster.h"
#include <QLineEdit>
#include <QPushButton>

class ToasterTest: public QWidget{
public:
    QPushButton* desktopToast;
    QPushButton* desktopToastWarning;
    explicit ToasterTest(QWidget* parent=nullptr): QWidget(parent),
        desktopToast(new QPushButton("Normal")),
        desktopToastWarning(new QPushButton("Warning")){

        QLineEdit* textEdit = new QLineEdit("Ciao!");
        QHBoxLayout* toasterLayout = new QHBoxLayout;
        toasterLayout->addWidget(textEdit);
        toasterLayout->addWidget(desktopToast);
        toasterLayout->addWidget(desktopToastWarning);
        connect(desktopToast, &QPushButton::clicked, this, [this]()->void{
            new Toaster(this, desktopToast->text(), true);
        });
        connect(desktopToastWarning, &QPushButton::clicked, this, [this]()->void{
            new Toaster(this, desktopToastWarning->text(), false);
        });
        setLayout(toasterLayout);
    }
};


//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    ToasterTest wid;
    wid.show();
    return a.exec();
}
#endif

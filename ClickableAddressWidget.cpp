#include "ClickableAddressWidget.h"
#include<QDir>
#include<QFileInfo>
#include<QList>
#include<QHBoxLayout>
#include<Qt>
#include<QDesktopServices>

ClickableAddressWidget::ClickableAddressWidget(QWidget *parent) :
    QWidget(parent),
    addressTB(new QToolBar),addressCB(new QComboBox),addressLE(new QLineEdit)
{
//    addressCB->setStyleSheet("border: 2px solid black;"
//                             "border-radius: 3px;"
//                             "padding: 1px 2px 1px 2px;"
//                             "min- 9em;");
    addressTB->setStyleSheet("QToolBar{"
                             "border-left: 1px solid gray;"
                             "border-right: 1px solid gray;"
                             "border-top: 1px solid gray;"
                             "border-bottom: 1px solid gray;"
                             "};");
    addressCB->setFixedHeight(CONTROL_HEIGHT);
    addressTB->setFixedHeight(CONTROL_HEIGHT);

    addressLE->setFixedHeight(CONTROL_HEIGHT);

#ifdef _WIN32
    const QFileInfoList& drives = QDir::drives();
    for (const auto& d : drives) {
        addressCB->addItem(d.filePath());
    }
#endif
    addressCB->setLineEdit(addressLE);

    QHBoxLayout* lo = new QHBoxLayout;
    lo->addWidget(addressTB);
    lo->addWidget(addressCB);

    lo->setSpacing(0);
    lo->setContentsMargins(0, 0, 0, 0);

    setLayout(lo);
    DispMode();
    subscribe();
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

inline auto ClickableAddressWidget::PathProcess(const QString&path) -> QString{
    if (path.size()>2 and path[path.size()-2]!=':' and path[path.size()-1]=='/')
    {
        return path.left(path.size()-1);
        // drive letter will be kept while trailing path seperator will be trunc
        // i.e., "XX:/" -> "XX:/" and "XX/" - >"XX"
    }
    return path;
}


auto ClickableAddressWidget::DispMode()->void{
    if (not addressTB->isVisible()){
        addressTB->setVisible(true);
    }
    addressCB->setVisible(false);
}

auto ClickableAddressWidget::EditMode()->void{
    addressTB->setVisible(false);
    if (not addressCB->isVisible()){
        addressCB->setVisible(true);
    }
}

void ClickableAddressWidget::focusOutEvent(QFocusEvent *event){
    if(addressCB->hasFocus()){
        qDebug("Drop down menu");
        return;
    }

    qDebug("focusOutEvent");
    DispMode();
}

void ClickableAddressWidget::keyPressEvent(QKeyEvent *event){
    // if event.key() == Qt.Key_Escape:
    //    self.DispMode()
    if (event->key() == Qt::Key_Enter or event->key() == Qt::Key_Return){
        auto ret = whenEnterAPath(addressLE->text());
    }
}

void ClickableAddressWidget::mousePressEvent(QMouseEvent *event){
    addressLE->setText(m_pth);
    EditMode();
}


auto ClickableAddressWidget::onActClicked(const QAction* clkAct)->void{
    QString fullPth;
    for(const auto* act: addressTB->actions()){
        if (act->text().isEmpty()){
            break;
        }
        fullPth += (act->text() + '/');
        if (act == clkAct){
            break;
        }
    }
    fullPth = ClickableAddressWidget::PathProcess(fullPth);
    qDebug("fullPth[%s]", fullPth.toStdString().c_str());
    whenEnterAPath(fullPth);
}

auto ClickableAddressWidget::whenEnterAPath(const QString& path)->bool{
    QString pth = QDir::fromNativeSeparators(path);
    QFileInfo fi(pth);
    if ((not QDir(pth).exists()) and (not fi.exists())){
        qDebug("Inexist path [%s]", pth.toStdString().c_str());
        return false;
    }

    DispMode();
    if (fi.isDir()){
        emit intoAPath_active(pth);
        UpdatePath(pth);
        return true;
    }

    else if (fi.isFile()){
        auto openRet = QDesktopServices::openUrl(pth);
        qDebug("Opening [%s]", pth.toStdString().c_str());
        return true;
    }
    return true;
}


auto ClickableAddressWidget::subscribe()->void{
    connect(addressTB, &QToolBar::actionTriggered, this, &ClickableAddressWidget::onActClicked);
    connect(addressCB, &QComboBox::currentTextChanged, this, [](const QString& s)->void{
        qDebug("current select text: %s, %d", "", s.size());
    });
}

auto ClickableAddressWidget::UpdatePath(QString pth)->void{
    addressTB->clear();
    m_pth = ClickableAddressWidget::PathProcess(pth);
    for(const QString& pt: m_pth.split('/')){
        addressTB->addAction(new QAction(pt, addressTB));
    }

    if (not addressCB->hasFocus()){  // in disp mode
        addressCB->insertItem(0, m_pth);
    }else{  // in edit mode, when return pressed it would auto append one text to the back
        addressCB->insertItem(0, m_pth);
        addressCB->removeItem(addressCB->count() - 1);
    }
}


//#define __NAME__EQ__MAIN__
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    ClickableAddressWidget* add = new ClickableAddressWidget;
    QLineEdit *searchLe = new QLineEdit("Search here");

    searchLe->setClearButtonEnabled(true);
    searchLe->addAction(QIcon(":/themes/SEARCH"), QLineEdit::LeadingPosition);
    searchLe->setPlaceholderText("Search...");


    QHBoxLayout* lo = new QHBoxLayout;
    lo->addWidget(add);
    lo->addWidget(searchLe);


    add->UpdatePath(QFileInfo(__FILE__).absolutePath());

    QWidget wid;
    wid.setLayout(lo);
    wid.show();
    return a.exec();

}
#endif

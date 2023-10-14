#ifndef TOASTER_H
#define TOASTER_H

#include <QDialog>
#include <QLabel>
#include <QToolButton>
#include <QStyle>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QRect>
#include <QTimer>
#include <QPoint>
#include <QPropertyAnimation>
#include <QWidget>
#include <QPainterPath>
class Toaster: public QDialog {
public:
    static constexpr int FADING_DUR = 1000; // million second
    static constexpr int EMERGING_DUR = 0;
    QPropertyAnimation* opacityAni;
    QTimer* timer;

    explicit Toaster(QWidget* parent, const QString& text,
                     const bool isNormal = true, const int margin = 10,
                     const int timeout = 5000, const bool closable = true):
        QDialog(parent),
        opacityAni(new QPropertyAnimation(this, "windowOpacity")),
        timer(new QTimer){

        setWindowFlag(Qt::FramelessWindowHint, true);
        setWindowFlag(Qt::BypassWindowManagerHint, true);
        // alternatively:
        // setAutoFillBackground(True)
        setAttribute(Qt::WA_ShowWithoutActivating);

        QLabel* titleLable = new QLabel;
        if (isNormal){
            titleLable->setPixmap(QPixmap(":/themes/SAVED").scaled(24, 24, Qt::KeepAspectRatio));
        }
        else{
            titleLable->setPixmap(QPixmap(":/themes/NOT_SAVED").scaled(24, 24, Qt::KeepAspectRatio));
        }
        QLabel* label = new QLabel(text);
        label->setStyleSheet("color: rgb(255, 255, 255);");

        QToolButton* closeButton = new QToolButton;
        closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
        closeButton->setAutoRaise(true);
        if (not closable){
            closeButton->hide();
        }else{
            connect(closeButton, &QToolButton::clicked, this, &QDialog::close);
        }

        QHBoxLayout* lo = new QHBoxLayout;
        lo->addWidget(titleLable);
        lo->addWidget(label);
        lo->addWidget(closeButton);
        setLayout(lo);
        setStyleSheet("QDialog {"
                      "   border: 1px solid black;"
                      "   border-radius: 0px;"
                      "   color: rgb(255, 255, 255);"
                      "   background-color: rgb(57, 66, 81);"
                      "}");
        show();
        QRect parentRect = QApplication::primaryScreen()->availableGeometry();
        QRect geo = geometry();
        geo.moveTopRight(parentRect.topRight() + QPoint(-margin, margin));
        setGeometry(geo);

        //raise the widget and adjust its size to the minimum
        raise();
        adjustSize();
        //now the widget should have the correct size hints, let's move it to the right place

        timer->setSingleShot(true);
        timer->setInterval(timeout);
        connect(timer, &QTimer::timeout, this, &Toaster::hide);

        QPropertyAnimation* opacityAni = new QPropertyAnimation(this, "windowOpacity");
        opacityAni->setStartValue(0);
        opacityAni->setEndValue(1);
        opacityAni->setDuration(EMERGING_DUR);

        connect(opacityAni, &QPropertyAnimation::finished, this, &Toaster::checkClosed);
        timer->start();
        opacityAni->start();
    }

    void checkClosed() {
        // if we have been fading out, we're closing the notification
        if (opacityAni->direction() == QPropertyAnimation::Backward){
            close();
        }
    }

    void restore(){
        //this is a "helper function", that can be called from mouseEnterEvent
        //and when the parent widget is resized. We will not close the
        //notification if the mouse is in or the parent is resized
        timer->stop();
        //also, stop the animation if it's fading out...
        opacityAni->stop();
        //...and restore the opacity
        setWindowOpacity(1);
    }

    void hide() {
        // start hiding
        qDebug("Fading...");
        opacityAni->setDuration(FADING_DUR);
        opacityAni->setStartValue(1);
        opacityAni->setEndValue(0);
        opacityAni->start();
    }

    void enterEvent(QEvent* event) override{
        restore();
    }

    void leaveEvent(QEvent* event) override{
        timer->start();
    }

    void closeEvent(QCloseEvent* event) override {
        // we don't need the notification anymore, delete it!
        deleteLater();
    }

    void resizeEvent(QResizeEvent* event) override{
        QDialog::resizeEvent(event);
        // if you don't set a stylesheet, you don't need any of the following!
        clearMask();
        QPainterPath path;
        path.addRoundedRect(rect(), 8, 8);
        setMask(QRegion(path.toFillPolygon(QTransform()).toPolygon()));
    }
};

#endif // TOASTER_H

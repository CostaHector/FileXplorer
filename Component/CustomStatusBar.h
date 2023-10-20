#ifndef CUSTOMSTATUSBAR_H
#define CUSTOMSTATUSBAR_H

#include <QStatusBar>
#include <QToolBar>
#include <QProgressBar>
#include <QLabel>

enum class STATUS_STR_TYPE{
    NORMAL = 0,
    ABNORMAL = 1
};

class CustomStatusBar: public QStatusBar {
public:
    static constexpr int STATUS_BAR_ICON_SIZE = 16;
    QToolBar* tb;
    QProgressBar* process;
    QList<QLabel*> labelLst;

    explicit CustomStatusBar(QWidget* parent=nullptr): QStatusBar(parent),
    tb(new QToolBar("triple view switch")),
    process(new QProgressBar),
    labelLst{new QLabel("items"), new QLabel("selected"), new QLabel("")}{

        process->setRange(0, 100);
        process->setValue(0);

//        tb->addActions(g_viewActions.TRIPLE_VIEW.actions());
//        tb->addAction(g_dbAct.DB_VIEW_CLOSE_SHOW);
        tb->setIconSize(QSize(CustomStatusBar::STATUS_BAR_ICON_SIZE, CustomStatusBar::STATUS_BAR_ICON_SIZE));

        // [QSizeGrip, Here is 1st~(n-1)th Widget, QHBoxLayout, here is nth widget];
        addPermanentWidget(labelLst[0]);  // start=1, dev=0
        addPermanentWidget(labelLst[1]);  // start=1, dev=1
        addPermanentWidget(labelLst[2], 1);  // 1+3
        addPermanentWidget(process);
        addPermanentWidget(tb);  // -1
        setContentsMargins(0,0,0,0);
    }
    auto pathInfo(const int count, const int index = 0) -> void {
        if (index == 0) {
            labelLst[0]->setText(QString("%1 item(s)").arg(count));
        }else if (index == 1) {
            labelLst[1]->setText(QString("%1 selected").arg(count));
        }
    }
    auto msg(const QString& msg = "", const STATUS_STR_TYPE statusStrType = STATUS_STR_TYPE::NORMAL) -> void {
        if (statusStrType == STATUS_STR_TYPE::ABNORMAL) {  // abnormal
            labelLst[2]->setStyleSheet("QLabel{color:red;font-weight:bold;}");
        }else{  // normal;
            labelLst[2]->setStyleSheet("");
        }
        qDebug(msg.toStdString().c_str());
        labelLst[2]->setText(msg);
    }
};
#endif // CUSTOMSTATUSBAR_H

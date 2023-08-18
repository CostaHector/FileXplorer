#ifndef NAVIGATIONANDADDRESSBAR_H
#define NAVIGATIONANDADDRESSBAR_H

#include <QWidget>
#include <QHBoxLayout>

#include <functional>
#include "PathUndoRedoer.h"
#include <QPushButton>
#include <QLineEdit>
#include "ClickableAddressWidget.h"
#include "PublicVariable.h"

class NavigationAndAddressBar : public QHBoxLayout
{
    Q_OBJECT
public:
    explicit NavigationAndAddressBar(T_IntoNewPath IntoNewPath = nullptr,
                                     T_on_searchTextChanged on_searchTextChanged  = nullptr,
                                     T_on_searchEnterKey on_searchEnterKe  = nullptr);

    auto UpdateEventWhenViewChanged() -> void {
        backToBtn->disconnect(SIGNAL(&QPushButton::clicked), this);
        forwardToBtn->disconnect(SIGNAL(&QPushButton::clicked), this);
        upToBtn->disconnect(SIGNAL(&QPushButton::clicked), this);
        searchLE->disconnect(SIGNAL(&QLineEdit::textChanged), this);
        searchLE->disconnect(SIGNAL(&QLineEdit::returnPressed), this);
        winAddress->disconnect(SIGNAL(&ClickableAddressWidget::intoAPath_active), this);
    }


    auto InitEventWhenViewChanged() -> void{
        connect(backToBtn, &QPushButton::clicked, this, [this]()->void{onBackward();});
        connect(forwardToBtn, &QPushButton::clicked, this, [this]()->void{onForward();});
        connect(upToBtn, &QPushButton::clicked, this, [this]()->void{onBackspaceEvent();});
        connect(searchLE, &QLineEdit::textChanged, this, [this]()->void{
            m_on_searchTextChanged(searchLE->text());
        });
        connect(searchLE, &QLineEdit::returnPressed, this, [this]()->void{
            m_on_searchEnterKey(searchLE->text());
        });
        connect(winAddress, &ClickableAddressWidget::intoAPath_active, [this](const QString& pth)->void{
            m_IntoNewPath(pth, true, false);
        });

    }

    auto onBackward() -> bool{
        if (pathRD.undoAvailable()){
            return m_IntoNewPath(pathRD.undo(), false, false);
        }
        qDebug("[Skip] backward paths pool empty");
        return true;
    }

    auto onForward() -> bool{
        if (pathRD.redoAvailable()){
            return m_IntoNewPath(pathRD.redo(), false, false);
        }
        qDebug("[Skip] Forward paths pool empty");
        return true;
    }


    auto onBackspaceEvent() -> bool{
        return m_IntoNewPath(winAddress->dirname(), true, false);
    }

    ClickableAddressWidget* winAddress;
    PathUndoRedoer pathRD;
    QPushButton* backToBtn;
    QPushButton* forwardToBtn;

signals:

private:
    std::function<bool(QString, bool, bool)> m_IntoNewPath;
    std::function<void(QString)> m_on_searchTextChanged;
    std::function<void(QString)> m_on_searchEnterKey;

    QPushButton* upToBtn;
    QLineEdit* searchLE;

};

#endif // NAVIGATIONANDADDRESSBAR_H

#ifndef CLICKABLEADDRESSWIDGET_H
#define CLICKABLEADDRESSWIDGET_H

#include <QWidget>
#include <QToolBar>
#include <QComboBox>

#include <QLineEdit>
#include <QAction>

#include <QFileInfo>
class ClickableAddressWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClickableAddressWidget(QWidget *parent = nullptr);

    static inline auto PathProcess(const QString&path) -> QString;

    virtual void focusOutEvent(QFocusEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

    auto DispMode()->void;
    auto EditMode()->void;
    auto subscribe()->void;
    auto onActClicked(const QAction* clkAct)->void;
    auto whenEnterAPath(const QString& path)->bool;

    inline auto dirname() -> QString{
        QFileInfo fi(m_pth);
        if (fi.isRoot()){
            return "";
        }
        return fi.absolutePath();
    }


signals:
    void intoAPath_active(const QString&);

public slots:
    void UpdatePath(QString path);

private:
    QString m_pth;
    QToolBar* addressTB;
    QComboBox* addressCB;
    QLineEdit* addressLE;
};

constexpr int CONTROL_HEIGHT = 28;
#endif // CLICKABLEADDRESSWIDGET_H

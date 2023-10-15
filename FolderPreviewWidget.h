#ifndef FOLDERPREVIEWWIDGET_H
#define FOLDERPREVIEWWIDGET_H

#include <QWidget>
#include <QString>

#include "FolderPreviewComponent/FolderListView.h"

class FolderPreviewWidget : public QWidget
{
Q_OBJECT
public:
    explicit FolderPreviewWidget(QWidget* parent=nullptr);
    bool operator()(const QString& path);
    void subscribe();

    void CustomContextMenuEvent(const QPoint& pnt);
    QSize sizeHint() const override;
signals:
    void showANewPath(const QString&);

protected:
    FolderListView* m_vidsPreview;
    FolderListView* m_imgsPreview;
    FolderListView* m_othersPreview;

    QMenu* m_folderPreviewMenu;
};

#endif // FOLDERPREVIEWWIDGET_H

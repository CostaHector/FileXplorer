#ifndef OTHERITEMLISTPREVIEW_H
#define OTHERITEMLISTPREVIEW_H

#include "FolderListView.h"

class OtherItemsListPreview : public FolderListView
{
public:
    OtherItemsListPreview();
    auto InitViewSettings()->void override;
};

#endif // OTHERITEMLISTPREVIEW_H

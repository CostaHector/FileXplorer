#ifndef ITEMSPILECATEGORY_H
#define ITEMSPILECATEGORY_H
#include <QString>
#include <QRegularExpression>

namespace ItemsPileCategory {
enum SCENE_COMPONENT_TYPE : char { IMG = 0, VID = 1, JSON = 2, OTHER = 3 };
const QMap<QString, SCENE_COMPONENT_TYPE> DOT_EXT_2_TYPE{{".jpg", IMG},  {".jpeg", IMG}, {".png", IMG}, {".webp", IMG},
                                                         {".jfif", IMG}, {".webp", IMG}, {".mp4", VID}, {".mkv", VID},
                                                         {".avi", VID},  {".wmv", VID},  {".ts", VID},  {".json", JSON}};
const QRegularExpression IMG_PILE_NAME_PATTERN{"^(.*?)( | - )?(\\d{1,3})?$"};
const QRegularExpression VID_PILE_NAME_PATTERN{"^(.*?)$",
                                               QRegularExpression::PatternOption::CaseInsensitiveOption};
}  // namespace ItemsPileCategory

#endif  // ITEMSPILECATEGORY_H

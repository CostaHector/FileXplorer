#ifndef ITEMSPILECATEGORY_H
#define ITEMSPILECATEGORY_H
#include <QString>
#include <QRegularExpression>

namespace ItemsPileCategory {
enum SCENE_COMPONENT_TYPE : char { IMG = 0, VID = 1, JSON = 2, OTHER = 3 };

using T_DOT_EXT_2_TYPE = QHash<QString, SCENE_COMPONENT_TYPE>;
const T_DOT_EXT_2_TYPE& GetTypeFromDotExtension();

const QRegularExpression IMG_PILE_NAME_PATTERN{"^(.*?)( | - )(\\d{1,3})?$"};
}  // namespace ItemsPileCategory

#endif  // ITEMSPILECATEGORY_H

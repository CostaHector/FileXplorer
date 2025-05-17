#include <QVariant>
#include <QVector>
#include <QString>

#include "Tools/Json/DataFormatter.h"
#include "public/PublicMacro.h"

// 定义主宏（放在头文件）
#define ALL_FIELD_ITEMS                                                                                                      \
  ONE_FIELD_ITEM(name, 0, QString, QString{}, DataFormatter::formatDefault, DataFormatter::writeQString)                     \
  ONE_FIELD_ITEM(gen, 1, Gender, Gender::male, DataFormatter::formatGender, DataFormatter::writeGender)                      \
  ONE_FIELD_ITEM(phoneNumber, 2, QString, QString{}, DataFormatter::formatPhoneNumber, DataFormatter::writePhoneNumber)      \
  ONE_FIELD_ITEM(age, 3, int, 0, DataFormatter::formatDefault, DataFormatter::writeInt)                                      \
  ONE_FIELD_ITEM(height, 4, float, 0.f, DataFormatter::formatFloat2Prec, DataFormatter::writeFloat)                               \
  ONE_FIELD_ITEM(weight, 5, float, 0.f, DataFormatter::formatFloat2Prec, DataFormatter::writeFloat)                               \
  ONE_FIELD_ITEM(rateAnnual, 6, QList<char>, QList<char>{}, DataFormatter::formatRateAnnual, DataFormatter::writeRateAnnual) \
  ONE_FIELD_ITEM(hobbies, 7, QStringList, QStringList{}, DataFormatter::formatHobbies, DataFormatter::writeHobbies)          \
  ONE_FIELD_ITEM(isOd, 8, bool, true, DataFormatter::formatBool, DataFormatter::writeBool)

// 生成枚举类型
enum Key_E : int {
  BEGIN = 0,  // 后面1个元素值必须=BEGIN的值
#define ONE_FIELD_ITEM(fieldEnum, fieldEnumVal, Type, defaultValue, formatter, cacheWriter) fieldEnum = fieldEnumVal,
  ALL_FIELD_ITEMS
#undef ONE_FIELD_ITEM
      BUTT
};

// 生成默认值数组
namespace DEF_VAL {
#define ONE_FIELD_ITEM(fieldEnum, fieldEnumVal, Type, defaultValue, formatter, cacheWriter) const Type fieldEnum{defaultValue};
ALL_FIELD_ITEMS
#undef ONE_FIELD_ITEM
}  // namespace DEF_VAL

// 生成字符串映射数组
const std::string MODEL_HORINDEX_2_FIELDNAME[BUTT] = {
#define ONE_FIELD_ITEM(fieldEnum, fieldEnumVal, Type, defaultValue, formatter, cacheWriter) ENUM_2_STR(fieldEnum),
    ALL_FIELD_ITEMS
#undef ONE_FIELD_ITEM
};

struct PersonalInfo {
#define ONE_FIELD_ITEM(fieldEnum, fieldEnumVal, Type, defaultValue, formatter, cacheWriter) Type fieldEnum{defaultValue};
  ALL_FIELD_ITEMS
#undef ONE_FIELD_ITEM
};

#include <QAbstractTableModel>
class JsonTableModel : public QAbstractTableModel {
 public:
  bool setData(const QModelIndex& ind, const QVariant& value, int role = Qt::EditRole) override {
    if (!ind.isValid() || role != Qt::EditRole) {
      return false;
    }
    const int r = ind.row();
    auto& info = m_data[r];
    switch (ind.column()) {
#define ONE_FIELD_ITEM(fieldEnum, fieldEnumVal, Type, defaultValue, format, cacheWriter) \
  case fieldEnum: {                                                                      \
    if (!cacheWriter(info.fieldEnum, value)) {                                           \
      return false;                                                                      \
    }                                                                                    \
    break;                                                                               \
  }
      ALL_FIELD_ITEMS
#undef ONE_FIELD_ITEM
      default:
        return false;
    }
    return true;
  }
  QVariant data(const QModelIndex& ind, int role = Qt::DisplayRole) const override {
    if (role == Qt::DisplayRole) {
      const int r = ind.row();
      const auto& info = m_data[r];
      switch (ind.column()) {
#define ONE_FIELD_ITEM(fieldEnum, fieldEnumVal, Type, defaultValue, format, cacheWriter) \
  case fieldEnum:                                                                        \
    return format(info.fieldEnum);  //
        // #define ONE_FIELD_ITEM(fieldEnum, fieldEnumVal, Type, defaultValue, format, cacheWriter) case fieldEnum: return [](&info) -> QVariant { return format(info.fieldEnum); }();
        ALL_FIELD_ITEMS
#undef JSON_FIELD_ITEM
        default:
          return {};
      }
    }
    return {};
  }

 private:
  QVector<PersonalInfo> m_data;
};

#ifndef MULTIPARKEY_H
#define MULTIPARKEY_H

// enum name, enum value, member variable type, member variable name
#define MULTI_PAR_KEY_MAPPING \
  MULTI_PAR_KEY_ITEM(SOURCE_FILES, 0, QStringList{}, srcFileNames) \
  MULTI_PAR_KEY_ITEM(PAR2_FILE, 1, QString{}, par2FileName) \
  MULTI_PAR_KEY_ITEM(VOL_PAR2_FILES, 2, QStringList{}, volPar2FileNameList) \
  MULTI_PAR_KEY_ITEM(STATUS, 3, QByteArray{}, statusByteArray) \
  MULTI_PAR_KEY_ITEM(COMPLETE_CNT, 4, (int) -1, completeCount) \
  MULTI_PAR_KEY_ITEM(MISNAMED_CNT, 5, (int) -1, misnamedCount) \
  MULTI_PAR_KEY_ITEM(DAMAGED_CNT, 6, (int) -1, damagedCount) \
  MULTI_PAR_KEY_ITEM(MISSING_CNT, 7, (int) -1, missingCount) \
  MULTI_PAR_KEY_ITEM(PRE_PATH, 8, QString{}, prePath) \

namespace MultiParKey {

enum ColumnE {
#define MULTI_PAR_KEY_ITEM(enumName, enumValue, memberVariableDefValue, memberVariableName) enumName = enumValue,
  MULTI_PAR_KEY_MAPPING
#undef MULTI_PAR_KEY_ITEM
  COLUMNS_CNT_BUTT,
};

constexpr const char* TABLE_HOR_HEADERS[COLUMNS_CNT_BUTT]{
#define MULTI_PAR_KEY_ITEM(enumName, enumValue, memberVariableDefValue, memberVariableName) #enumName,
    MULTI_PAR_KEY_MAPPING
#undef MULTI_PAR_KEY_ITEM
};

inline bool isPar2Column(int column) {
  return column == ColumnE::PAR2_FILE || column == ColumnE::VOL_PAR2_FILES;
}

constexpr int RATE_OF_REDUNDANCY_MIN = 1, RATE_OF_REDUNDANCY_MAX = 100;
inline bool isRateOfRedundancyValid(int rateOfRedundancy) {
  return RATE_OF_REDUNDANCY_MIN <= rateOfRedundancy && rateOfRedundancy <= RATE_OF_REDUNDANCY_MAX;
}
}

#endif // MULTIPARKEY_H

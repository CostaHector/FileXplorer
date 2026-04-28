#ifndef GENERALDATATYPE_H
#define GENERALDATATYPE_H

namespace GeneralDataType {

enum Type {
  ERROR_TYPE = 0,
  GROUP,
  PLAIN_STR,
  MULTI_LINE_STR,
  PLAIN_BOOL,
  PLAIN_INT,
  RANGE_INT,
  PLAIN_LONGLONG,
  PLAIN_FLOAT,
  PLAIN_DOUBLE,

  COMBOBOX_NEED_BEGIN,

  COMBOBOX_NEED_STRING_BEGIN = COMBOBOX_NEED_BEGIN,
  FONT_FAMILY = COMBOBOX_NEED_BEGIN,
  COMBOBOX_NEED_STRING_END,

  COMBOBOX_NEED_STRING_TO_ENUM_BEGIN = COMBOBOX_NEED_STRING_END,
  FONT_WEIGHT = COMBOBOX_NEED_STRING_END,
  FONT_STYLE,
  RANGE_INT_STYLE_PRESET,
  RANGE_INT_STYLE_THEME,
  COMBOBOX_NEED_STRING_TO_ENUM_END,
  COMBOBOX_NEED_END = COMBOBOX_NEED_STRING_TO_ENUM_END,

  LINEDIT_WITH_ACTION_BEGIN = COMBOBOX_NEED_STRING_TO_ENUM_END,
  COLOR = LINEDIT_WITH_ACTION_BEGIN,

  PATH_RELATED_BEGIN = COLOR,
  FILE_PATH,           // resource url or local path
  IMAGE_PATH_OPTIONAL, // can be empty
  FOLDER_PATH,
  PATH_RELATED_END,

  LINEDIT_WITH_ACTION_END = PATH_RELATED_END,
};

inline constexpr bool isLineEditWithActionNeededInEditor(int dataType) {
  return GeneralDataType::Type::LINEDIT_WITH_ACTION_BEGIN <= dataType && dataType < GeneralDataType::Type::LINEDIT_WITH_ACTION_END;
}

inline constexpr bool isPathRelatedType(int dataType) {
  return GeneralDataType::Type::PATH_RELATED_BEGIN <= dataType && dataType < GeneralDataType::Type::PATH_RELATED_END;
}

inline constexpr bool isComboBoxNeededInEditor(int dataType) {
  return GeneralDataType::Type::COMBOBOX_NEED_BEGIN <= dataType && dataType < GeneralDataType::Type::COMBOBOX_NEED_END;
}

inline constexpr bool isComboBoxNeededInStringEditor(int dataType) {
  return GeneralDataType::Type::COMBOBOX_NEED_STRING_BEGIN <= dataType && dataType < GeneralDataType::Type::COMBOBOX_NEED_STRING_END;
}

inline constexpr bool isComboBoxNeededInStringToEnumEditor(int dataType) {
  return GeneralDataType::Type::COMBOBOX_NEED_STRING_TO_ENUM_BEGIN <= dataType && dataType < GeneralDataType::Type::COMBOBOX_NEED_STRING_TO_ENUM_END;
}

inline constexpr bool isPlainTextEditNeededInEditor(int dataType) {
  return dataType == MULTI_LINE_STR;
}

} // namespace GeneralDataType

#endif // GENERALDATATYPE_H

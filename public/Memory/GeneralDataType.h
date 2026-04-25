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
  PLAIN_FLOAT,
  PLAIN_DOUBLE,
  RANGE_INT,
  FONT_FAMILY,
  FONT_WEIGHT,
  FONT_STYLE,
  COLOR,
  FILE_PATH,           // resource url or local path
  IMAGE_PATH_OPTIONAL, // can be empty
  FOLDER_PATH,
};

inline constexpr bool isPathRelatedType(int dataType) {
  return dataType == GeneralDataType::Type::FILE_PATH || dataType == GeneralDataType::Type::IMAGE_PATH_OPTIONAL || dataType == GeneralDataType::Type::FOLDER_PATH;
}

inline constexpr bool isComboBoxNeededInEditor(int dataType) {
  return dataType == GeneralDataType::Type::FONT_FAMILY || //
         dataType == GeneralDataType::Type::FONT_WEIGHT || //
         dataType == GeneralDataType::Type::FONT_STYLE;
}

inline constexpr bool isPlainTextEditNeededInEditor(int dataType) {
  return dataType == MULTI_LINE_STR;
}

} // namespace GeneralDataType

#endif // GENERALDATATYPE_H

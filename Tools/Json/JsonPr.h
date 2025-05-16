#ifndef JSON_DICT_H
#define JSON_DICT_H

#include <QVariantHash>
#include "JsonKey.h"
#include "SortedUniqStrLst.h"

using namespace JsonKey;
struct JsonPr {
  static JsonPr fromJsonFile(const QString& jsonAbsFile);

  explicit JsonPr(const QString& jsonAbsFile);
  explicit JsonPr(const QString& filePrePath, const QString& fileName, const QJsonObject& json);
  bool operator==(const JsonPr& rhs) const;

  bool Reload();
  bool WriteIntoFiles() const;
  inline QString GetAbsPath() const { return m_Prepath + '/' + jsonFileName; }

// Json Key and Value
#define JSON_KEY_ITEM(enu, enumVal, defValue, enhanceDefVal, format, writer, initer, jsonWriter) decltype(enhanceDefVal) m_##enu = enhanceDefVal;
  JSON_MODEL_FIELD_MAPPING
#undef JSON_KEY_ITEM

  QString jsonFileName;
  mutable QString hintCast;
  mutable QString hintStudio;

  enum OP_RET {
    E_JSON_NOT_EXIST = 0,          //
    E_JSON_FILE_RENAME_FAILED,     //
    E_RELATED_FILE_RENAME_FAILED,  //
    E_OK,                          //
  };
  int Rename(const QString& newJsonNameUserInput, bool alsoRenameRelatedFiles = true);
  bool SyncNameValueFromFileBaseName();
  bool ConstructCastStudioValue();  // contruct cast/studio
  bool ClearCastStudioValue();      // clear cast/studio
  bool SetStudio(const QString& studio);
  bool SetCastOrTags(const QString& val, FIELD_OP_TYPE fieldType, FIELD_OP_MODE fieldMode);
  void HintForCastStudio(const QString& selectedText) const;
  void AcceptCastHint();
  void AcceptStudioHint();

  void RejectCastHint();
  void RejectStudioHint();
};

#endif

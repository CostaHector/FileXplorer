#ifndef KV_H
#define KV_H

#include "GeneralDataType.h"
#include "RawVariant.h"
#include "ValueChecker.h"
#include <QString>
#include <QVariant>

using ConfigurationChangedCallback = bool (*)(const QVariant& v);

struct KV {
public:
  KV() = delete;
  constexpr KV(const char* _name,                                                          //
               RawVariant::Var _v,                                                         //
               GeneralDataType::Type _gType,                                               //
               ValueChecker::RawVariantChecker _pChecker = ValueChecker::GeneralNoChecker, //
               ConfigurationChangedCallback _changedCallback = nullptr,                    //
               const char* _iconUrl = "",                                                  //
               const char* _toolTip = "")                                                  //
    : name{_name}                                                                          //
    , v{_v}                                                                                //
    , generalDataType{_gType}                                                              //
    , pChecker{_pChecker}                                                                  //
    , changedCallback{_changedCallback}                                                    //
    , iconUrl{_iconUrl}                                                                    //
    , toolTip{_toolTip}                                                                    //
  {}

  constexpr GeneralDataType::Type dataType() const { return generalDataType; }
  constexpr const char* iconUrlStr() const { return iconUrl; }
  constexpr const char* toolTipStr() const { return toolTip; }
  constexpr bool isPath() const { return GeneralDataType::isPathRelatedType(dataType()); }
  QVariant toVariant() const;
  static QList<const KV*>& GetEditableKVs();

  const char* name;
  RawVariant::Var v;
  GeneralDataType::Type generalDataType;
  ValueChecker::RawVariantChecker pChecker;
  ConfigurationChangedCallback changedCallback;
  const char* iconUrl;
  const char* toolTip;
};

#endif // KV_H

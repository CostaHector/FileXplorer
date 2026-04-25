#include "KV.h"

QList<const KV*>& KV::GetEditableKVs() {
  static QList<const KV*> editableKVs;
  return editableKVs;
}

QVariant KV::toVariant() const {
  switch (v.type) {
    case RawVariant::VarType::Empty:
      return {};
    case RawVariant::VarType::Int:
      return QVariant::fromValue<int>(v.data.i);
    case RawVariant::VarType::Bool:
      return QVariant::fromValue<bool>(v.data.b);
    case RawVariant::VarType::Float:
      return QVariant::fromValue<float>(v.data.f);
    case RawVariant::VarType::Double:
      return QVariant::fromValue<double>(v.data.d);
    case RawVariant::VarType::Char:
      return QVariant::fromValue<char>(v.data.c);
    case RawVariant::VarType::LongLong:
      return QVariant::fromValue<long long>(v.data.ll);
    case RawVariant::VarType::CStrLit:
      return QVariant::fromValue<QString>(QString{v.data.str});
    default:
      return {};
  }
}

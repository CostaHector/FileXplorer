#ifndef ONSCOPEEXIT_H
#define ONSCOPEEXIT_H

#define CONCAT_VARIABLE_NAME_AND_LINE_NO_CORE(name, lineNum) name##lineNum // ## require literal direcyly
#define CONCAT_VARIABLE_NAME_AND_LINE_NO(exitor_guard_, lineNumberInt) CONCAT_VARIABLE_NAME_AND_LINE_NO_CORE(exitor_guard_, lineNumberInt)  // let __LINE__ became a number

template <typename Func>
class OnScopeExitClass {
 public:
  OnScopeExitClass() = default;
  ~OnScopeExitClass() {
     m_func();
  }
  Func m_func;
};

enum class PLACEHOLDER_E {};

template <typename Func>
OnScopeExitClass<Func> operator+(PLACEHOLDER_E, Func func) {
  return OnScopeExitClass<Func>{func};
}

#define OnScopeExit auto CONCAT_VARIABLE_NAME_AND_LINE_NO(exitor, __LINE__) = PLACEHOLDER_E{} + [&]()

#endif  // ONSCOPEEXIT_H

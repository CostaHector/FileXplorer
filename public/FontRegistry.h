#ifndef FONTREGISTRY_H
#define FONTREGISTRY_H

#include <unordered_set>
#include <QFont>

// SetFontableObject can be QAction, QWidget, and so on(contains memeber at least font(), setFont(), connect)
template <typename SetFontableObject>
struct FontRegistry {
  using TWidgetSet = std::unordered_set<SetFontableObject*>;
  static TWidgetSet* GetWidgetsSet(bool useMonospaceFont);

  struct AutoManager {
   public:
    explicit AutoManager(SetFontableObject* _widget, bool _useMonospaceFont = false, bool _isAlwaysAlive = true);
    ~AutoManager();

   private:
    SetFontableObject* widget{nullptr};
    bool useMonospaceFont{false};
    bool isAlwaysAlive{true};
  };

  static bool registerWidgetForFont(SetFontableObject* pWid, bool useMonospaceFont = false, bool isAlwaysAlive = true);

  static bool unregisterWidgetForFont(SetFontableObject* pWid, bool useMonospaceFont = false);

  static int updateRegisteredWidgetsFont(const QFont& newFont, bool useMonospaceFont = false);
};


#endif  // FONTREGISTRY_H

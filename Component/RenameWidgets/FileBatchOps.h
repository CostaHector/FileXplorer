#ifndef FILEBATCHOPS_H
#define FILEBATCHOPS_H

#include <QStringList>

class AdvanceRenamer;

namespace FileBatchOps {
enum class OpStatus {
  PARTIAL_FAILED = -1,
  SKIP = 0,
  ALL_SUCCEED = 1,
};

struct OpOutcome {
  OpOutcome() = default;
  OpOutcome(const OpStatus resE, int cnt): result{resE}, count{cnt}{}
  OpOutcome(const OpOutcome& rhs) : OpOutcome{rhs.result, rhs.count} {}
  bool operator==(const OpOutcome& rhs) const { return result == rhs.result && count == rhs.count; }
  OpStatus result{OpStatus::SKIP};
  int count{0};
};

/* 统一模式获取
 jsonNames or videoName format can only be "coreName.ext". All file with name format "^coreName.ext" or "^coreName - \d{1,3}.ext", or "^coreName \d{1,3}.ext" will filtered out for rename
 imgNames format can be "coreName.ext" or "coreName - \d{1,3}.ext" or "coreName \d{1,3}.ext". All file with name format "^coreName" will filtered out for rename
*/
QStringList ExpandRelatedFiles(const QString& path, const QStringList& relPatternNames, QMap<QString, QString>* pFile2Pattern=nullptr);

// RenameWidget_CastElider 需要根据对应关系给旧名后添加演员表
using BeforeInitTextContentFn = std::function<void(AdvanceRenamer&)>;
void prepareRenameDialog(AdvanceRenamer &widget, const QString &workPath, const QStringList &names, const BeforeInitTextContentFn &beforeInitTextFunc = {});
OpStatus execRenameDialog(AdvanceRenamer &widget);

// ---- 重命名核心入口
// 只处理 selectedNames, 不获取相关项目, 再交给 GUI
OpStatus RenameSelected(AdvanceRenamer& widget, const QString& workPath, const QStringList& selectedNames);
// 处理 selectedJsonOrVideoNames 和相关项目
OpOutcome RenameSelectedWithRelated(AdvanceRenamer& widget, const QString& workPath, const QStringList& selectedJsonOrVideoNames);

// ---- 模式指定要重命名的文件
int RenameWithPatterns(AdvanceRenamer& renamer, const QString& path, const QStringList& jsonOrVidNames, const char* logTag);
int RenameWithPatternInsert(const QString& path, const QStringList& jsonOrVidNames);     // jsonNames can also be videoNames
int RenameWithPatternReplace(const QString& path, const QStringList& jsonOrVidNames);    // jsonNames can also be videoNames
int RenameWithPatternNumerizer(const QString& path, const QStringList& jsonOrVidNames);  // jsonNames can also be videoNames

// ---- 直接指定要重命名的文件
OpStatus RenameSelectedInsert(const QString& workPath, const QStringList& selectedNames);
OpStatus RenameSelectedReplace(const QString& workPath, const QStringList& selectedNames);
OpStatus RenameSelectedNumerizer(const QString& workPath, const QStringList& selectedNames);

// ---- 回收相关核心入口
OpStatus MoveToTrashSelected(const QString& workPath, const QStringList& names);
OpStatus MoveToTrashSelectedWithVaryRootPaths(const QStringList& workPaths, const QStringList &rel2names);
OpOutcome MoveToTrashSelectedWithRelated(const QString& workPath, const QStringList& selectedJsonOrVideoNames);

// ---- 彻底删除相关核心入口
OpStatus DeletePermanentlySelected(const QString& workPath, const QStringList& rel2names);
OpStatus DeletePermanentlySelectedWithVaryRootPaths(const QStringList& workPaths, const QStringList &relNames);
}  // namespace BatchRenameBy

#endif  // FILEBATCHOPS_H

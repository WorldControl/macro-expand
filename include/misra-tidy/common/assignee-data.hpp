
#ifndef TIDY_UTILS_COMMON_ASSIGNEE_DATA_HPP
#define TIDY_UTILS_COMMON_ASSIGNEE_DATA_HPP

// LLVM includes
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>

// Standard includes
#include <string>

namespace tidy {
/// Stores information about the assignee of a call expression.
///
/// In `int x = f(5);`, the variable `x` is the *assignee*.
class AssigneeData {
 public:
  // <<=/>>= should be the longest compound assignment.
  using OperatorString = llvm::SmallString<3>;

  class Builder;

  /// Stores information about the type of the assignee.
  struct Type {
    /// Constructor.
    explicit Type(std::string name_ = std::string(),
                  bool isDefaultConstructible_ = true) noexcept;

    /// A string representation of the type.
    std::string name;

    /// Whether the type is default constructible or not.
    bool isDefaultConstructible;
  };

  /// Returns true if the assignee's type is default-constructible or if there
  /// is no type
  /// information at all. There is no type information if the call is as
  /// assignment rather than a
  /// construction. If this function returns false, it is not safe to expand a
  /// function with at
  /// least one return statement that is not the last statement of the function.
  /// For example, in:
  ///
  /// ```.cpp
  /// int& f(int& x) {
  ///  if (x > 10) {
  ///    x += 1;
  ///    return x;
  ///  }
  ///  return x;
  /// }
  /// ```
  ///
  /// It would not be safe to expand a call expression like this:
  ///
  /// ```.cpp
  /// int x = 12;
  /// int& y = f(x);
  /// ```
  ///
  /// As this would translate to an expansion where the reference can not be
  /// bound immediately (or
  /// the type default-constructed):
  ///
  /// ```.cpp
  /// int& y; // invalid
  /// if (x > 10) {
  ///   x += 1;
  ///   y = x;
  /// }
  /// y = x;
  /// ```
  ///
  /// If only the last return statement is given, this expression is valid,
  /// however.
  bool isDefaultConstructible() const noexcept;

  /// Converts the `AssigneeData` to a string like `"int x ="`. If `withType` is
  /// true, the type (like `int` in the example) will be included (this if for
  /// *declarations*). Else, the type is omitted (for *assignments*). Note that
  /// the assignee is not always assigned with `=`, but sometimes also any other
  /// operator like `+=` or `<<=` (compound operators).
  std::string toAssignment(bool withType = true) const;

  /// Converts the `AssigneeData` to a string like `int x;` that would declare
  /// the assignee (i.e. the type and name, followed by a semicolon).
  std::string toDeclaration() const;

  /// The operator used in the assignment (`=`, `+=`, `<<=` etc.).
  OperatorString op;

  /// The name of the variable being assigned to.
  std::string name;

  /// If the assignment is a variable declaration and not just an assignment,
  /// the type of the variable being declared. The type is further split into
  /// the name (string representation) of the type and a boolean flag indicating
  /// whethert the type is default constructible.
  llvm::Optional<Type> type;
};

/// Helper class to build an `AssigneeData` structure.
class AssigneeData::Builder {
 public:
  /// Constructs a new builder with a temporary `AssigneeData` instance.
  explicit Builder(AssigneeData&& assignee = AssigneeData());
  Builder(const Builder&) = delete;
  Builder& operator=(const Builder&) = delete;

  /// Sets the name of the assignee.
  Builder& name(const llvm::StringRef& name);

  /// Sets the type information of the assignee.
  Builder& type(const llvm::StringRef& name,
                bool isDefaultConstructible = true);

  /// Sets the operator information of the assignee.
  Builder& op(const llvm::StringRef& op);

  /// Finishes building the `AssigneeData` and returns it.
  AssigneeData build();

 private:
  /// The `AssigneeData` being built.
  AssigneeData _assignee;
};

}  // namespace tidy

#endif  // TIDY_UTILS_COMMON_ASSIGNEE_DATA_HPP

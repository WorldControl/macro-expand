
#ifndef TIDY_UTILS_COMMON_LOCATION_HPP
#define TIDY_UTILS_COMMON_LOCATION_HPP

// Project includes
#include "misra-tidy/common/offset.hpp"

// Third party includes
#include <third-party/json.hpp>

// LLVM includes
#include <llvm/ADT/StringRef.h>

// Standard includes
#include <string>

namespace clang {
class SourceLocation;
class SourceManager;
}

namespace tidy {

/// An easier-to-use representation of a source location.
///
/// `clang::SourceLocation`s are meant to be stored as efficiently as possible
/// in the AST to keep it small. `SourceLocation` are really just IDs or indices
/// into a "location-table", so this table must be consulted through the source
/// manager to go from a `SourceLocation` to the filename, line and/or column
/// that the `SourceLocation` represents. Meanwhile, this `Location` class is
/// much less space efficient but stores all important information inside (like
/// a "fat" `SourceLocation`). This is useful since we need such `Locations` a
/// lot when doing our processing as well as for final output to stdout.
struct Location {
  /// Constructs a `Location` from a `clang::SourceLocation` using the source
  /// manager.
  Location(const clang::SourceLocation& location,
           const clang::SourceManager& sourceManager);

  /// Constructs a `Location` from a filename and `(line, column)` pair.
  Location(const llvm::StringRef& filename_, unsigned line, unsigned column);

  /// Compares if 2 locations are the same
  bool operator==(const Location& other) const {
      return std::tie(filename, offset.line, offset.column) == 
             std::tie(other.filename, other.offset.line, other.offset.column);
  }

  /// Converts the `Location` to JSON.
  nlohmann::json toJson() const;

  /// The name of the file this location is from.
  std::string filename;

  /// The offset into the file (a `(line, column)` pair).
  Offset offset;
};
}  // namespace tidy

namespace std {

    template <>
    struct hash<const tidy::Location>
    {
        std::size_t operator()(const tidy::Location& k) const
        {
            using std::size_t;
            using std::hash;
            using std::string;

            // Compute individual hash values for first,
            // second and third and combine them using XOR
            // and bit shifting:

            return ((hash<string>()(k.filename)
                ^ (hash<unsigned>()(k.offset.column) << 1)) >> 1)
                ^ (hash<unsigned>()(k.offset.line) << 1);
        }
    };

} //namespace std

#endif  // TIDY_UTILS_COMMON_LOCATION_HPP

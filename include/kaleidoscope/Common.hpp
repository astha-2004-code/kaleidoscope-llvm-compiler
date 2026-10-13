#ifndef KALEIDOSCOPE_COMMON_HPP
#define KALEIDOSCOPE_COMMON_HPP

#include <cstdint>
#include <string>
#include <string_view>
#include <ostream>

namespace kaleidoscope {

/// Represents a precise source location in an input buffer or file.
struct SourceLocation {
    uint32_t line{1};
    uint32_t column{1};

    constexpr SourceLocation() = default;
    constexpr SourceLocation(uint32_t l, uint32_t c) : line(l), column(c) {}

    bool isValid() const { return line > 0 && column > 0; }

    bool operator==(const SourceLocation &other) const {
        return line == other.line && column == other.column;
    }
    bool operator!=(const SourceLocation &other) const {
        return !(*this == other);
    }
};

/// Represents a span between two source locations.
struct SourceRange {
    SourceLocation begin;
    SourceLocation end;

    SourceRange() = default;
    SourceRange(SourceLocation b, SourceLocation e) : begin(b), end(e) {}
    explicit SourceRange(SourceLocation loc) : begin(loc), end(loc) {}
};

inline std::ostream &operator<<(std::ostream &os, const SourceLocation &loc) {
    return os << loc.line << ":" << loc.column;
}

} // namespace kaleidoscope

#endif // KALEIDOSCOPE_COMMON_HPP

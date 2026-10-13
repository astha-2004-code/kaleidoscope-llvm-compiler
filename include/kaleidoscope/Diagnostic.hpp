#ifndef KALEIDOSCOPE_DIAGNOSTIC_HPP
#define KALEIDOSCOPE_DIAGNOSTIC_HPP

#include "Common.hpp"
#include <string>
#include <vector>
#include <iostream>

namespace kaleidoscope {

enum class DiagnosticSeverity {
    Note,
    Warning,
    Error,
    Fatal
};

struct Diagnostic {
    DiagnosticSeverity severity;
    SourceLocation location;
    std::string message;

    Diagnostic(DiagnosticSeverity sev, SourceLocation loc, std::string msg)
        : severity(sev), location(loc), message(std::move(msg)) {}
};

class DiagnosticEngine {
public:
    explicit DiagnosticEngine(std::ostream &outStream = std::cerr);

    void report(DiagnosticSeverity severity, SourceLocation loc, const std::string &message);
    void error(SourceLocation loc, const std::string &message);
    void warning(SourceLocation loc, const std::string &message);
    void note(SourceLocation loc, const std::string &message);

    void setSourceText(std::string text);

    bool hasErrors() const { return errorCount_ > 0; }
    size_t getErrorCount() const { return errorCount_; }
    size_t getWarningCount() const { return warningCount_; }

    const std::vector<Diagnostic> &getDiagnostics() const { return diagnostics_; }
    void clear();

private:
    void formatAndEmit(const Diagnostic &diag);
    std::string getLineSnippet(uint32_t lineNumber) const;

    std::ostream &outStream_;
    std::string sourceText_;
    std::vector<std::string> sourceLines_;
    std::vector<Diagnostic> diagnostics_;
    size_t errorCount_{0};
    size_t warningCount_{0};
};

} // namespace kaleidoscope

#endif // KALEIDOSCOPE_DIAGNOSTIC_HPP

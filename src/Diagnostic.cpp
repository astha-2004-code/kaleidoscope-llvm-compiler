#include "kaleidoscope/Diagnostic.hpp"
#include <sstream>

namespace kaleidoscope {

DiagnosticEngine::DiagnosticEngine(std::ostream &outStream)
    : outStream_(outStream) {}

void DiagnosticEngine::setSourceText(std::string text) {
    sourceText_ = std::move(text);
    sourceLines_.clear();

    std::istringstream stream(sourceText_);
    std::string line;
    while (std::getline(stream, line)) {
        sourceLines_.push_back(line);
    }
}

std::string DiagnosticEngine::getLineSnippet(uint32_t lineNumber) const {
    if (lineNumber > 0 && lineNumber <= sourceLines_.size()) {
        return sourceLines_[lineNumber - 1];
    }
    return "";
}

void DiagnosticEngine::report(DiagnosticSeverity severity, SourceLocation loc, const std::string &message) {
    if (severity == DiagnosticSeverity::Error || severity == DiagnosticSeverity::Fatal) {
        ++errorCount_;
    } else if (severity == DiagnosticSeverity::Warning) {
        ++warningCount_;
    }

    Diagnostic diag(severity, loc, message);
    diagnostics_.push_back(diag);
    formatAndEmit(diag);
}

void DiagnosticEngine::error(SourceLocation loc, const std::string &message) {
    report(DiagnosticSeverity::Error, loc, message);
}

void DiagnosticEngine::warning(SourceLocation loc, const std::string &message) {
    report(DiagnosticSeverity::Warning, loc, message);
}

void DiagnosticEngine::note(SourceLocation loc, const std::string &message) {
    report(DiagnosticSeverity::Note, loc, message);
}

void DiagnosticEngine::formatAndEmit(const Diagnostic &diag) {
    const char *tag = "note";
    switch (diag.severity) {
        case DiagnosticSeverity::Note:    tag = "note"; break;
        case DiagnosticSeverity::Warning: tag = "warning"; break;
        case DiagnosticSeverity::Error:   tag = "error"; break;
        case DiagnosticSeverity::Fatal:   tag = "fatal error"; break;
    }

    outStream_ << diag.location.line << ":" << diag.location.column << ": "
               << tag << ": " << diag.message << "\n";

    std::string snippet = getLineSnippet(diag.location.line);
    if (!snippet.empty()) {
        outStream_ << "    " << snippet << "\n";
        outStream_ << "    ";
        for (uint32_t i = 1; i < diag.location.column; ++i) {
            outStream_ << (snippet[i - 1] == '\t' ? '\t' : ' ');
        }
        outStream_ << "^\n";
    }
}

void DiagnosticEngine::clear() {
    diagnostics_.clear();
    errorCount_ = 0;
    warningCount_ = 0;
}

} // namespace kaleidoscope

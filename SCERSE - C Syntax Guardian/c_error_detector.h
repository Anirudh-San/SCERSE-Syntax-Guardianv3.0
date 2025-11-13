#ifndef C_ERROR_DETECTOR_H
#define C_ERROR_DETECTOR_H

#include <string>
#include <vector>

struct AnalysisResult {
    std::vector<std::string> lexicalErrors;
    std::vector<std::pair<std::string, std::string>> syntaxErrors;
    int totalErrors;
};

class CErrorDetectorEngine {
public:
    CErrorDetectorEngine();
    ~CErrorDetectorEngine();
    
    AnalysisResult analyzeCode(const std::string& sourceCode);
    AnalysisResult analyzeFile(const std::string& filename);
};

#endif // C_ERROR_DETECTOR_H
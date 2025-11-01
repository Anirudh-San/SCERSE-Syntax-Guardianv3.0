/*
 * Modularized C Error Detector
 * - Supports stdio.h functions
 * - Handles #include statements
 * - Qt 6.9.3 compatible architecture
 * - Clean API for GUI integration
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include <sstream>

using namespace std;

// ============================================================================
// FORWARD DECLARATIONS & TYPES
// ============================================================================

struct AnalysisResult {
    vector<string> lexicalErrors;
    vector<pair<string, string>> syntaxErrors; // (error, suggestion)
    int totalErrors;
};

// ============================================================================
// ERROR SUGGESTION ENGINE MODULE
// ============================================================================

struct ErrorSuggestion {
    string error;
    string suggestion;
    string example;
    ErrorSuggestion(string e = "", string s = "", string ex = "") 
        : error(e), suggestion(s), example(ex) {}
};

class SuggestionEngine {
private:
    vector<ErrorSuggestion> suggestions;

public:
    SuggestionEngine() {
        suggestions.push_back(ErrorSuggestion(
            "Expected ';'",
            "Add a semicolon at the end of the statement",
            "int x = 5;  // <- Correct"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Expected '('",
            "Control structures need parentheses around condition",
            "if (x > 5) { } while (y < 10) { }"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Expected ')'",
            "Close the opening parenthesis - check for balanced parentheses",
            "function(arg1, arg2);  // <- All parentheses balanced"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Expected '}'",
            "Close the opening brace - check for balanced braces",
            "void func() { int x = 5; }  // <- Each { has matching }"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Undeclared variable",
            "Declare variable before using: type varname;",
            "int x; x = 5;  // <- Declare BEFORE using"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Redeclaration",
            "Variable already exists in this scope. Use different name",
            "int x = 5; int y = 10;  // <- Use different name"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Invalid numeric literal",
            "Check for multiple decimals or invalid characters",
            "float x = 3.14;  // <- Correct"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Unterminated string",
            "String literals must have opening AND closing quotes",
            "char* s = \"hello\";  // <- Both sides have quotes"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Unterminated character",
            "Character literals must have opening AND closing quotes",
            "char c = 'A';  // <- Both sides have quotes"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Multi-character constant",
            "Character literals can only contain ONE character",
            "char c = 'A';  // <- Correct"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Invalid character",
            "Remove invalid characters. Common: @, #, $ in wrong context",
            "int x = 5 + 10;  // <- Correct"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Unexpected token",
            "This token not expected in position. Check grammar",
            "int x = 5 * 10;  // <- Correct"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Expected type",
            "Type specifier needed: int, float, char, void, double",
            "int x;  float y;  char z;  // <- All have types"
        ));
        suggestions.push_back(ErrorSuggestion(
            "Missing #endif",
            "Preprocessor conditional #if must have matching #endif",
            "#if defined(DEBUG)\\n// code\\n#endif  // <- Proper pairing"
        ));
    }

    string getSuggestion(const string& errorMsg) {
        for (const auto& s : suggestions) {
            if (errorMsg.find(s.error) != string::npos) {
                return "SUGGESTION: " + s.suggestion + " | EXAMPLE: " + s.example;
            }
        }
        return "";
    }

    void addCustomSuggestion(const string& e, const string& s, const string& ex) {
        suggestions.push_back(ErrorSuggestion(e, s, ex));
    }
};

// ============================================================================
// STANDARD LIBRARY MODULE (stdio, stdlib, etc.)
// ============================================================================

class StandardLibrary {
private:
    unordered_set<string> stdioFunctions;
    unordered_set<string> stdlibFunctions;
    unordered_set<string> stringFunctions;
    unordered_set<string> mathFunctions;
    unordered_map<string, string> functionSignatures;

public:
    StandardLibrary() {
        // stdio.h functions
        stdioFunctions = {
            "printf", "scanf", "fprintf", "fscanf", "sprintf", "sscanf",
            "fopen", "fclose", "fread", "fwrite", "fgets", "fputs",
            "getchar", "putchar", "gets", "puts", "perror"
        };
        
        // stdlib.h functions
        stdlibFunctions = {
            "malloc", "calloc", "realloc", "free", "exit", "abort",
            "atoi", "atof", "atol", "rand", "srand", "qsort"
        };
        
        // string.h functions
        stringFunctions = {
            "strcpy", "strncpy", "strlen", "strcmp", "strcat",
            "strchr", "strstr", "memset", "memcpy", "memmove"
        };
        
        // math.h functions
        mathFunctions = {
            "sin", "cos", "tan", "sqrt", "pow", "abs", "floor", "ceil"
        };
        
        // Function signatures for type checking
        functionSignatures["printf"] = "int printf(const char* format, ...)";
        functionSignatures["scanf"] = "int scanf(const char* format, ...)";
        functionSignatures["malloc"] = "void* malloc(size_t size)";
        functionSignatures["free"] = "void free(void* ptr)";
        functionSignatures["strlen"] = "size_t strlen(const char* s)";
    }

    bool isStdioFunction(const string& name) const { return stdioFunctions.count(name) > 0; }
    bool isStdlibFunction(const string& name) const { return stdlibFunctions.count(name) > 0; }
    bool isStringFunction(const string& name) const { return stringFunctions.count(name) > 0; }
    bool isMathFunction(const string& name) const { return mathFunctions.count(name) > 0; }
    bool isStandardFunction(const string& name) const {
        return isStdioFunction(name) || isStdlibFunction(name) || 
               isStringFunction(name) || isMathFunction(name);
    }

    string getFunctionSignature(const string& name) const {
        auto it = functionSignatures.find(name);
        return it != functionSignatures.end() ? it->second : "";
    }
};

// ============================================================================
// PREPROCESSOR MODULE (handles #include, #define, etc.)
// ============================================================================

class PreprocessorHandler {
private:
    vector<string> errors;
    unordered_set<string> includedHeaders;

public:
    void processInclude(const string& line, int lineNum) {
        if (line.find("#include") != 0) return;
        
        size_t start = line.find_first_of("<\"");
        size_t end = line.find_last_of(">\"");
        
        if (start == string::npos || end == string::npos || start >= end) {
            errors.push_back("Line " + to_string(lineNum) + " - Invalid #include syntax");
            return;
        }
        
        string headerName = line.substr(start + 1, end - start - 1);
        includedHeaders.insert(headerName);
    }

    void processPreprocessor(const string& line, int lineNum) {
        if (line.find("#if") == 0 || line.find("#ifdef") == 0 || line.find("#ifndef") == 0) {
            // Track conditional compilation
        }
        if (line.find("#include") == 0) {
            processInclude(line, lineNum);
        }
    }

    bool isHeaderIncluded(const string& header) const {
        return includedHeaders.count(header) > 0;
    }

    vector<string> getErrors() const { return errors; }
};

// ============================================================================
// TOKEN DEFINITIONS
// ============================================================================

enum class TokenType {
    TOK_EOF, TOK_IDENTIFIER, TOK_NUMBER, TOK_STRING, TOK_CHAR,
    KW_INT, KW_FLOAT, KW_CHAR, KW_VOID, KW_DOUBLE,
    KW_IF, KW_ELSE, KW_WHILE, KW_FOR, KW_DO,
    KW_RETURN, KW_BREAK, KW_CONTINUE,
    KW_SWITCH, KW_CASE, KW_DEFAULT,
    KW_STRUCT, KW_TYPEDEF, KW_SIZEOF,
    KW_CONST, KW_STATIC, KW_EXTERN,
    OP_PLUS, OP_MINUS, OP_STAR, OP_SLASH, OP_PERCENT,
    OP_ASSIGN, OP_EQ, OP_NE, OP_LT, OP_GT, OP_LE, OP_GE,
    OP_AND, OP_OR, OP_NOT, OP_INC, OP_DEC,
    OP_PLUSEQ, OP_MINUSEQ,
    OP_BITAND, OP_BITOR, OP_BITXOR, OP_BITNOT,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    SEMICOLON, COMMA, DOT, ARROW, COLON, QUESTION,
    PREPROCESSOR, TOK_ERROR, TOK_UNKNOWN
};

struct Token {
    TokenType type;
    string value;
    int line;
    int column;
    Token(TokenType t = TokenType::TOK_UNKNOWN, string v = "", int l = 1, int c = 1)
        : type(t), value(v), line(l), column(c) {}
};

// ============================================================================
// LEXER MODULE
// ============================================================================

class Lexer {
private:
    string input;
    size_t pos;
    int line, column;
    vector<string> errors;
    PreprocessorHandler preprocessor;
    
    unordered_map<string, TokenType> keywords = {
        {"int", TokenType::KW_INT}, {"float", TokenType::KW_FLOAT},
        {"char", TokenType::KW_CHAR}, {"void", TokenType::KW_VOID},
        {"double", TokenType::KW_DOUBLE}, {"if", TokenType::KW_IF},
        {"else", TokenType::KW_ELSE}, {"while", TokenType::KW_WHILE},
        {"for", TokenType::KW_FOR}, {"do", TokenType::KW_DO},
        {"return", TokenType::KW_RETURN}, {"break", TokenType::KW_BREAK},
        {"continue", TokenType::KW_CONTINUE}, {"switch", TokenType::KW_SWITCH},
        {"case", TokenType::KW_CASE}, {"default", TokenType::KW_DEFAULT},
        {"struct", TokenType::KW_STRUCT}, {"typedef", TokenType::KW_TYPEDEF},
        {"sizeof", TokenType::KW_SIZEOF}, {"const", TokenType::KW_CONST},
        {"static", TokenType::KW_STATIC}, {"extern", TokenType::KW_EXTERN}
    };

    char currentChar() { return pos >= input.length() ? '\0' : input[pos]; }
    char peekChar(int offset = 1) { return pos + offset >= input.length() ? '\0' : input[pos + offset]; }
    
    void advance() {
        if (pos < input.length()) {
            if (input[pos] == '\n') { line++; column = 1; }
            else { column++; }
            pos++;
        }
    }

    void skipWhitespace() { while (isspace(currentChar())) advance(); }
    
    void skipComment() {
        if (currentChar() == '/' && peekChar() == '/') {
            while (currentChar() != '\n' && currentChar() != '\0') advance();
        } else if (currentChar() == '/' && peekChar() == '*') {
            int cLine = line, cCol = column;
            advance(); advance();
            while (true) {
                if (currentChar() == '\0') {
                    errors.push_back("Line " + to_string(cLine) + ":" + to_string(cCol) + " - Unterminated comment");
                    break;
                }
                if (currentChar() == '*' && peekChar() == '/') { advance(); advance(); break; }
                advance();
            }
        }
    }

    Token lexNumber() {
        int sL = line, sC = column;
        string num;
        bool hasDec = false;
        while (isdigit(currentChar()) || currentChar() == '.') {
            if (currentChar() == '.') {
                if (hasDec) errors.push_back("Line " + to_string(line) + ":" + to_string(column) + " - Invalid numeric literal: multiple decimal points");
                hasDec = true;
            }
            num += currentChar();
            advance();
        }
        if (isalpha(currentChar())) {
            errors.push_back("Line " + to_string(line) + ":" + to_string(column) + " - Invalid numeric literal");
            while (isalnum(currentChar())) { num += currentChar(); advance(); }
            return Token(TokenType::TOK_ERROR, num, sL, sC);
        }
        return Token(TokenType::TOK_NUMBER, num, sL, sC);
    }

    Token lexIdentifier() {
        int sL = line, sC = column;
        string id;
        while (isalnum(currentChar()) || currentChar() == '_') { id += currentChar(); advance(); }
        if (keywords.find(id) != keywords.end()) return Token(keywords[id], id, sL, sC);
        return Token(TokenType::TOK_IDENTIFIER, id, sL, sC);
    }

    Token lexString() {
    int sL = line, sC = column;
    string result = "";
    char quote = currentChar();  // Remember opening quote (")
    
    result += quote;  // Include opening quote
    advance();
    
    bool closed = false;
    
    // Read until closing quote or EOF
    while (currentChar() != '\0') {
        char c = currentChar();
        
        // Found closing quote
        if (c == quote) {
            result += quote;  // Include closing quote
            advance();
            closed = true;
            break;
        }
        
        // Newline in string = unterminated error
        if (c == '\n') {
            errors.push_back("Line " + to_string(sL) + ":" + to_string(sC) + 
                           " - Unterminated string literal (newline in string)");
            break;
        }
        
        // Escape sequence handling
        if (c == '\\') {
            result += c;  // Add backslash
            advance();
            
            if (currentChar() != '\0') {
                // Add the escaped character (n, t, ", \, %, d, x, etc.)
                result += currentChar();
                advance();
            }
        }
        else {
            // Regular character - just add it
            // This includes: %, digits, letters, spaces, etc.
            result += c;
            advance();
        }
    }
    
    if (!closed) {
        errors.push_back("Line " + to_string(sL) + ":" + to_string(sC) + 
                       " - Unterminated string literal (EOF reached)");
        return Token(TokenType::TOK_ERROR, result, sL, sC);
    }
    
    return Token(TokenType::TOK_STRING, result, sL, sC);
}

    Token lexChar() {
    int sL = line, sC = column;
    string result = "";
    char quote = currentChar();  // Remember opening quote (')
    
    result += quote;  // Include opening quote
    advance();
    
    bool closed = false;
    int charCount = 0;
    
    // Read until closing quote or EOF
    while (currentChar() != '\0') {
        char c = currentChar();
        
        // Found closing quote
        if (c == quote) {
            result += quote;  // Include closing quote
            advance();
            closed = true;
            break;
        }
        
        // Newline in char = unterminated error
        if (c == '\n') {
            errors.push_back("Line " + to_string(sL) + ":" + to_string(sC) + 
                           " - Unterminated character literal (newline in char)");
            break;
        }
        
        // Escape sequence handling
        if (c == '\\') {
            result += c;  // Add backslash
            advance();
            
            if (currentChar() != '\0') {
                // Add the escaped character
                result += currentChar();
                advance();
                charCount += 2;  // Escape sequence counts as 1 char
            }
        }
        else {
            // Regular character
            result += c;
            advance();
            charCount++;
        }
    }
    
    if (!closed) {
        errors.push_back("Line " + to_string(sL) + ":" + to_string(sC) + 
                       " - Unterminated character literal (EOF reached)");
        return Token(TokenType::TOK_ERROR, result, sL, sC);
    }
    
    // Validate character count
    if (charCount == 0 || charCount > 1) {
        if (charCount == 0) {
            errors.push_back("Line " + to_string(sL) + ":" + to_string(sC) + 
                           " - Empty character literal");
        } else {
            errors.push_back("Line " + to_string(sL) + ":" + to_string(sC) + 
                           " - Multi-character constant");
        }
    }
    
    return Token(TokenType::TOK_CHAR, result, sL, sC);
}


public:
    Lexer(const string& src) : input(src), pos(0), line(1), column(1) {}
    
    Token getNextToken() {
        skipWhitespace();
        while ((currentChar() == '/' && (peekChar() == '/' || peekChar() == '*'))) {
            skipComment();
            skipWhitespace();
        }
        
        int sL = line, sC = column;
        char c = currentChar();
        
        if (c == '\0') return Token(TokenType::TOK_EOF, "", line, column);
        
        // Preprocessor directives
        if (c == '#') {
            string prep;
            while (currentChar() != '\n' && currentChar() != '\0') {
                prep += currentChar();
                advance();
            }
            preprocessor.processPreprocessor(prep, sL);
            return Token(TokenType::PREPROCESSOR, prep, sL, sC);
        }
        
        if (isdigit(c)) return lexNumber();
        if (isalpha(c) || c == '_') return lexIdentifier();
        if (c == '"') return lexString();
        if (c == '\'') return lexChar();

        switch (c) {
            case '(': advance(); return Token(TokenType::LPAREN, "(", sL, sC);
            case ')': advance(); return Token(TokenType::RPAREN, ")", sL, sC);
            case '{': advance(); return Token(TokenType::LBRACE, "{", sL, sC);
            case '}': advance(); return Token(TokenType::RBRACE, "}", sL, sC);
            case '[': advance(); return Token(TokenType::LBRACKET, "[", sL, sC);
            case ']': advance(); return Token(TokenType::RBRACKET, "]", sL, sC);
            case ';': advance(); return Token(TokenType::SEMICOLON, ";", sL, sC);
            case ',': advance(); return Token(TokenType::COMMA, ",", sL, sC);
            case '.': advance(); return Token(TokenType::DOT, ".", sL, sC);
            case ':': advance(); return Token(TokenType::COLON, ":", sL, sC);
            case '?': advance(); return Token(TokenType::QUESTION, "?", sL, sC);
            case '+':
                advance();
                if (currentChar() == '+') { advance(); return Token(TokenType::OP_INC, "++", sL, sC); }
                if (currentChar() == '=') { advance(); return Token(TokenType::OP_PLUSEQ, "+=", sL, sC); }
                return Token(TokenType::OP_PLUS, "+", sL, sC);
            case '-':
                advance();
                if (currentChar() == '-') { advance(); return Token(TokenType::OP_DEC, "--", sL, sC); }
                if (currentChar() == '=') { advance(); return Token(TokenType::OP_MINUSEQ, "-=", sL, sC); }
                if (currentChar() == '>') { advance(); return Token(TokenType::ARROW, "->", sL, sC); }
                return Token(TokenType::OP_MINUS, "-", sL, sC);
            case '*': advance(); return Token(TokenType::OP_STAR, "*", sL, sC);
            case '/': advance(); return Token(TokenType::OP_SLASH, "/", sL, sC);
            case '%': advance(); return Token(TokenType::OP_PERCENT, "%", sL, sC);
            case '=':
                advance();
                if (currentChar() == '=') { advance(); return Token(TokenType::OP_EQ, "==", sL, sC); }
                return Token(TokenType::OP_ASSIGN, "=", sL, sC);
            case '!':
                advance();
                if (currentChar() == '=') { advance(); return Token(TokenType::OP_NE, "!=", sL, sC); }
                return Token(TokenType::OP_NOT, "!", sL, sC);
            case '<':
                advance();
                if (currentChar() == '=') { advance(); return Token(TokenType::OP_LE, "<=", sL, sC); }
                return Token(TokenType::OP_LT, "<", sL, sC);
            case '>':
                advance();
                if (currentChar() == '=') { advance(); return Token(TokenType::OP_GE, ">=", sL, sC); }
                return Token(TokenType::OP_GT, ">", sL, sC);
            case '&':
                advance();
                if (currentChar() == '&') { advance(); return Token(TokenType::OP_AND, "&&", sL, sC); }
                return Token(TokenType::OP_BITAND, "&", sL, sC);
            case '|':
                advance();
                if (currentChar() == '|') { advance(); return Token(TokenType::OP_OR, "||", sL, sC); }
                return Token(TokenType::OP_BITOR, "|", sL, sC);
            case '^': advance(); return Token(TokenType::OP_BITXOR, "^", sL, sC);
            case '~': advance(); return Token(TokenType::OP_BITNOT, "~", sL, sC);
            default:
                errors.push_back("Line " + to_string(line) + ":" + to_string(column) + " - Invalid character: '" + string(1, c) + "'");
                advance();
                return Token(TokenType::TOK_ERROR, string(1, c), sL, sC);
        }
    }

    vector<string> getErrors() const { return errors; }
    
    vector<Token> tokenizeAll() {
        vector<Token> tokens;
        Token tok;
        do {
            tok = getNextToken();
            tokens.push_back(tok);
        } while (tok.type != TokenType::TOK_EOF);
        return tokens;
    }
};

// ============================================================================
// SYMBOL TABLE MODULE
// ============================================================================

struct VarInfo {
    string name, type;
    VarInfo(string n = "", string t = "") : name(n), type(t) {}
};

class SymbolTable {
    vector<unordered_map<string, VarInfo>> scopes;
    StandardLibrary stdLib;
    
public:
    SymbolTable() { pushScope(); }
    void pushScope() { scopes.push_back({}); }
    void popScope() { if (!scopes.empty()) scopes.pop_back(); }
    
    bool declare(const string& n, const string& t) {
        auto& c = scopes.back();
        if (c.count(n)) return false;
        c[n] = VarInfo(n, t);
        return true;
    }
    
    bool exists(const string& n) const {
    // CHECK STANDARD LIBRARY FIRST
    if (stdLib.isStdioFunction(n) || 
        stdLib.isStdlibFunction(n) || 
        stdLib.isStringFunction(n) || 
        stdLib.isMathFunction(n)) {
        return true;
    }
    
    // Then check user variables
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->count(n)) return true;
    }
    return false;
}

};

// ============================================================================
// PARSER MODULE
// ============================================================================

class Parser {
private:
    vector<Token> tokens;
    size_t index;
    SymbolTable sym;
    vector<pair<string, string>> errors;
    SuggestionEngine suggestionEngine;
    StandardLibrary stdLib;
    size_t lastIndex;

    Token curr() const { return index < tokens.size() ? tokens[index] : Token(TokenType::TOK_EOF, ""); }
    Token peek(int offset = 1) const { return index + offset < tokens.size() ? tokens[index + offset] : Token(TokenType::TOK_EOF, ""); }
    
    void advance() { if (index < tokens.size()) index++; }

    void forceAdvance() {
        if (index == lastIndex) {
            Token bad = curr();
            string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Skipping invalid token '" + bad.value + "'";
            string sug = suggestionEngine.getSuggestion(errMsg);
            errors.push_back({errMsg, sug});
            advance();
        }
        lastIndex = index;
    }

    void expect(TokenType type, const string& desc) {
        Token bad = curr();
        if (bad.type != type) {
            string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Expected '" + desc + "' but got '" + bad.value + "'";
            string sug = suggestionEngine.getSuggestion(errMsg);
            errors.push_back({errMsg, sug});
        } else {
            advance();
        }
    }

    bool isTypeToken(const Token& t) {
        return t.type == TokenType::KW_INT || t.type == TokenType::KW_FLOAT ||
               t.type == TokenType::KW_CHAR || t.type == TokenType::KW_VOID ||
               t.type == TokenType::KW_DOUBLE;
    }

    bool isComparisonOp(const Token& t) {
        return t.type == TokenType::OP_LT || t.type == TokenType::OP_GT ||
               t.type == TokenType::OP_LE || t.type == TokenType::OP_GE ||
               t.type == TokenType::OP_EQ || t.type == TokenType::OP_NE;
    }

    bool isOp(const Token& t) {
        return t.type == TokenType::OP_PLUS || t.type == TokenType::OP_MINUS ||
               t.type == TokenType::OP_STAR || t.type == TokenType::OP_SLASH ||
               isComparisonOp(t) || t.type == TokenType::OP_ASSIGN;
    }

    void parseDeclOrFunc() {
        string typeName = curr().value;
        advance();
        if (curr().type != TokenType::TOK_IDENTIFIER) {
            Token bad = curr();
            string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Expected identifier";
            string sug = suggestionEngine.getSuggestion(errMsg);
            errors.push_back({errMsg, sug});
            advance();
            return;
        }
        Token nameTok = curr();
        string ident = nameTok.value;
        advance();
        if (curr().type == TokenType::LPAREN) parseFunction(typeName, ident, nameTok);
        else parseVarDecl(typeName, ident, nameTok);
    }

    void parseVarDecl(const string& type, const string& ident, const Token& nameTok) {
        if (!sym.declare(ident, type)) {
            string errMsg = "Line " + to_string(nameTok.line) + ":" + to_string(nameTok.column) + " - Redeclaration of '" + ident + "'";
            string sug = suggestionEngine.getSuggestion(errMsg);
            errors.push_back({errMsg, sug});
        }
        if (curr().type == TokenType::OP_ASSIGN) { advance(); parseExpression(); }
        while (curr().type == TokenType::COMMA) {
            advance();
            if (curr().type == TokenType::TOK_IDENTIFIER) {
                Token t = curr();
                if (!sym.declare(t.value, type)) {
                    string errMsg = "Line " + to_string(t.line) + ":" + to_string(t.column) + " - Redeclaration";
                    string sug = suggestionEngine.getSuggestion(errMsg);
                    errors.push_back({errMsg, sug});
                }
                advance();
                if (curr().type == TokenType::OP_ASSIGN) { advance(); parseExpression(); }
            }
        }
        expect(TokenType::SEMICOLON, ";");
    }

   void parseFunction(const std::string& type, const std::string& ident, const Token& /*nameTok*/) {
    // Function body without using nameTok
        advance();
        sym.declare(ident, type);
        sym.pushScope();
        if (curr().type != TokenType::RPAREN) {
            while (true) {
                if (!isTypeToken(curr())) {
                    Token bad = curr();
                    string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Expected type";
                    string sug = suggestionEngine.getSuggestion(errMsg);
                    errors.push_back({errMsg, sug});
                    break;
                }
                string pType = curr().value; advance();
                if (curr().type != TokenType::TOK_IDENTIFIER) {
                    Token bad = curr();
                    string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Expected parameter name";
                    string sug = suggestionEngine.getSuggestion(errMsg);
                    errors.push_back({errMsg, sug});
                    break;
                }
                sym.declare(curr().value, pType); advance();
                if (curr().type == TokenType::COMMA) advance();
                else break;
            }
        }
        expect(TokenType::RPAREN, ")");
        if (curr().type == TokenType::SEMICOLON) { advance(); sym.popScope(); return; }
        expect(TokenType::LBRACE, "{");
        parseBlock();
        sym.popScope();
    }

    void parseBlock() {
        int maxIter = 10000;
        int iter = 0;
        while (curr().type != TokenType::RBRACE && curr().type != TokenType::TOK_EOF && iter++ < maxIter) {
            lastIndex = index;
            if (isTypeToken(curr())) parseDeclOrFunc();
            else parseStatement();
            forceAdvance();
        }
        if (iter >= maxIter) {
            Token bad = curr();
            string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Parser stuck, aborting block";
            errors.push_back({errMsg, ""});
        }
        expect(TokenType::RBRACE, "}");
    }

    void parseStatement() {
        if (curr().type == TokenType::PREPROCESSOR) { advance(); }
        else if (curr().type == TokenType::LBRACE) { advance(); sym.pushScope(); parseBlock(); sym.popScope(); }
        else if (curr().type == TokenType::KW_IF) {
            advance(); expect(TokenType::LPAREN, "("); parseExpression(); expect(TokenType::RPAREN, ")");
            parseStatement();
            if (curr().type == TokenType::KW_ELSE) { advance(); parseStatement(); }
        }
        else if (curr().type == TokenType::KW_WHILE || curr().type == TokenType::KW_FOR) {
            advance(); expect(TokenType::LPAREN, "("); parseExpression(); expect(TokenType::RPAREN, ")");
            parseStatement();
        }
        else if (curr().type == TokenType::KW_RETURN) {
            advance();
            if (curr().type != TokenType::SEMICOLON) parseExpression();
            expect(TokenType::SEMICOLON, ";");
        }
        else if (curr().type == TokenType::SEMICOLON) { advance(); }
        else { parseExprOrAssignment(); expect(TokenType::SEMICOLON, ";"); }
    }

    void parseExprOrAssignment() {
        if (curr().type == TokenType::TOK_IDENTIFIER && peek().type == TokenType::OP_ASSIGN) {
            Token id = curr();
            if (!sym.exists(id.value)) {
                string errMsg = "Line " + to_string(id.line) + ":" + to_string(id.column) + " - Undeclared variable '" + id.value + "'";
                string sug = suggestionEngine.getSuggestion(errMsg);
                errors.push_back({errMsg, sug});
            }
            advance(); advance(); parseExpression();
        } else { parseExpression(); }
    }

    void parseExpression() {
        parsePrimary();
        while (isOp(curr())) { advance(); parsePrimary(); }
    }

    void parsePrimary() {
    if (curr().type == TokenType::TOK_IDENTIFIER) {
        Token id = curr();
        if (!sym.exists(id.value)) {
            string errMsg = "Line " + to_string(id.line) + ":" + to_string(id.column) + 
                           " - Undeclared variable '" + id.value + "'";
            string sug = suggestionEngine.getSuggestion(errMsg);
            errors.push_back({errMsg, sug});
        }
        advance();
        
        // ADD THIS: Handle function calls
        if (curr().type == TokenType::LPAREN) {
            advance();  // skip (
            if (curr().type != TokenType::RPAREN) {
                // Parse arguments separated by commas
                while (true) {
                    parseExpression();
                    if (curr().type == TokenType::COMMA) {
                        advance();
                    } else {
                        break;
                    }
                }
            }
            expect(TokenType::RPAREN, ")");
        }
    }
    else if (curr().type == TokenType::TOK_NUMBER || 
             curr().type == TokenType::TOK_STRING || 
             curr().type == TokenType::TOK_CHAR) { 
        advance(); 
    }
    else if (curr().type == TokenType::LPAREN) { 
        advance(); 
        parseExpression(); 
        expect(TokenType::RPAREN, ")"); 
    }
    else if (curr().type != TokenType::SEMICOLON && 
             curr().type != TokenType::RPAREN && 
             curr().type != TokenType::RBRACE && 
             curr().type != TokenType::COMMA && 
             curr().type != TokenType::TOK_EOF) {
        Token bad = curr();
        string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + 
                       " - Unexpected token '" + bad.value + "'";
        string sug = suggestionEngine.getSuggestion(errMsg);
        errors.push_back({errMsg, sug});
        advance();
    }
}


public:
    Parser(const vector<Token>& toks) : tokens(toks), index(0), lastIndex(0) {}

    void parseProgram() {
        int maxIter = 10000;
        int iter = 0;
        while (curr().type != TokenType::TOK_EOF && iter++ < maxIter) {
            lastIndex = index;
            if (curr().type == TokenType::PREPROCESSOR) { advance(); }
            else if (isTypeToken(curr())) parseDeclOrFunc();
            else if (curr().type == TokenType::TOK_ERROR) advance();
            else {
                Token bad = curr();
                string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Unexpected at file scope";
                string sug = suggestionEngine.getSuggestion(errMsg);
                errors.push_back({errMsg, sug});
                advance();
            }
            forceAdvance();
        }
        if (iter >= maxIter) errors.push_back({"Parser stuck - aborting", ""});
    }

    vector<pair<string, string>> getErrorsWithSuggestions() const { return errors; }
};

// ============================================================================
// ANALYSIS ENGINE (Qt-ready public API)
// ============================================================================

class CErrorDetectorEngine {
private:
    Lexer* lexer;
    Parser* parser;
    
public:
    CErrorDetectorEngine() : lexer(nullptr), parser(nullptr) {}
    ~CErrorDetectorEngine() {
        if (lexer) delete lexer;
        if (parser) delete parser;
    }

    AnalysisResult analyzeCode(const string& sourceCode) {
        AnalysisResult result;
        
        lexer = new Lexer(sourceCode);
        vector<Token> tokens = lexer->tokenizeAll();
        result.lexicalErrors = lexer->getErrors();
        
        parser = new Parser(tokens);
        parser->parseProgram();
        vector<pair<string, string>> syntaxErrors = parser->getErrorsWithSuggestions();
        result.syntaxErrors = syntaxErrors;
        
        result.totalErrors = result.lexicalErrors.size() + result.syntaxErrors.size();
        
        return result;
    }

    AnalysisResult analyzeFile(const string& filename) {
        ifstream f(filename);
        if (!f.is_open()) {
            AnalysisResult result;
            result.lexicalErrors.push_back("ERROR: Could not open file '" + filename + "'");
            result.totalErrors = 1;
            return result;
        }
        string code((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
        return analyzeCode(code);
    }
};

// ============================================================================
// MAIN INTERFACE (for testing without Qt)
// ============================================================================

/*int main() {
    cout << "\n" << string(70, '=') << endl;
    cout << "  C ERROR DETECTOR WITH SUGGESTION ENGINE (Qt-Ready)" << endl;
    cout << string(70, '=') << "\n" << endl;

    CErrorDetectorEngine engine;
    
    cout << "Enter the path to a C source file: ";
    string filename;
    getline(cin, filename);

    if (filename.empty()) {
        cout << "No file specified.\n";
        return 1;
    }

    AnalysisResult result = engine.analyzeFile(filename);
    
    if (!result.lexicalErrors.empty()) {
        cout << "\nLEXICAL ERRORS (" << result.lexicalErrors.size() << "):\n";
        cout << string(70, '-') << "\n";
        for (const auto& e : result.lexicalErrors) {
            cout << "  " << e << "\n";
        }
    }

    if (!result.syntaxErrors.empty()) {
        cout << "\nSYNTAX/SEMANTIC ERRORS (" << result.syntaxErrors.size() << "):\n";
        cout << string(70, '-') << "\n";
        for (size_t i = 0; i < result.syntaxErrors.size(); ++i) {
            cout << "[" << (i+1) << "] " << result.syntaxErrors[i].first << "\n";
            if (!result.syntaxErrors[i].second.empty()) {
                cout << "    " << result.syntaxErrors[i].second << "\n";
            }
            cout << "\n";
        }
    }

    cout << string(70, '=') << "\n";
    if (result.totalErrors == 0) {
        cout << "SUCCESS: No errors detected!\n";
    } else {
        cout << "TOTAL ERRORS: " << result.totalErrors << "\n";
    }
    cout << string(70, '=') << "\n\n";

    return 0;
}*/

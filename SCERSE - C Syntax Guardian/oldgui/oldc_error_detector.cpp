/*
 * Complete C Error Detector Module
 * - Full pointer support (int *p, int **pp)
 * - Array support (int arr[10], int arr[] = {1,2,3})
 * - Const support (const int x, const int *p)
 * - Struct support (struct Point { int x; })
 * - Typedef support (typedef int MyInt)
 * - Nested function detection (disallowed)
 * - Function redeclaration detection (disallowed)
 * - Type validation and checking
 * - Auto keyword support
 * - Column-precise error reporting
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include <sstream>
#include <set>
#include <memory>

using namespace std;

// ============================================================================
// TOKEN TYPE ENUMERATION - Complete with all required types
// ============================================================================

enum class TokenType {
    // Keywords
    KW_INT, KW_FLOAT, KW_CHAR, KW_DOUBLE, KW_VOID,
    KW_SHORT, KW_LONG, KW_SIGNED, KW_UNSIGNED,
    KW_IF, KW_ELSE, KW_WHILE, KW_FOR, KW_RETURN,
    KW_CONST, KW_STRUCT, KW_TYPEDEF, KW_AUTO,
    
    // Literals and identifiers
    TOK_IDENTIFIER, TOK_NUMBER, TOK_STRING, TOK_CHAR,
    
    // Delimiters
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    SEMICOLON, COMMA, DOT,
    
    // Operators - Arithmetic
    OP_PLUS, OP_MINUS, OP_STAR, OP_SLASH, OP_PERCENT,
    
    // Operators - Assignment and comparison
    OP_ASSIGN, OP_EQ, OP_NE, OP_LT, OP_GT, OP_LE, OP_GE,
    
    // Operators - Logical and bitwise
    OP_AND, OP_OR, OP_NOT, OP_BITAND, OP_BITOR, OP_BITXOR, OP_BITNOT,
    
    // Operators - Unary
    OP_INC, OP_DEC,
    
    // Other
    PREPROCESSOR, TOK_EOF
};

// ============================================================================
// TOKEN STRUCTURE
// ============================================================================

struct Token {
    TokenType type;
    string value;
    int line;
    int column;
    
    Token(TokenType t = TokenType::TOK_EOF, const string& v = "", int l = 1, int c = 1)
        : type(t), value(v), line(l), column(c) {}
};

// ============================================================================
// LEXER - Tokenizes C source code
// ============================================================================

class Lexer {
private:
    string input;
    size_t pos;
    int line, column;
    int startLine, startColumn;
    
    unordered_map<string, TokenType> keywords {
        {"int", TokenType::KW_INT},
        {"float", TokenType::KW_FLOAT},
        {"char", TokenType::KW_CHAR},
        {"double", TokenType::KW_DOUBLE},
        {"void", TokenType::KW_VOID},
        {"short", TokenType::KW_SHORT},
        {"long", TokenType::KW_LONG},
        {"signed", TokenType::KW_SIGNED},
        {"unsigned", TokenType::KW_UNSIGNED},
        {"if", TokenType::KW_IF},
        {"else", TokenType::KW_ELSE},
        {"while", TokenType::KW_WHILE},
        {"for", TokenType::KW_FOR},
        {"return", TokenType::KW_RETURN},
        {"const", TokenType::KW_CONST},
        {"struct", TokenType::KW_STRUCT},
        {"typedef", TokenType::KW_TYPEDEF},
        {"auto", TokenType::KW_AUTO}
    };
    
    char currentChar() const {
        return (pos < input.length()) ? input[pos] : '\0';
    }
    
    char peekChar(int offset = 1) const {
        return (pos + offset < input.length()) ? input[pos + offset] : '\0';
    }
    
    void advance() {
        if (pos < input.length()) {
            if (input[pos] == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            pos++;
        }
    }
    
    void skipWhitespace() {
        while (pos < input.length() && isspace(currentChar())) {
            advance();
        }
    }
    
    void skipLineComment() {
        if (currentChar() == '/' && peekChar() == '/') {
            while (currentChar() != '\n' && currentChar() != '\0') {
                advance();
            }
        }
    }
    
    void skipBlockComment() {
        if (currentChar() == '/' && peekChar() == '*') {
            advance(); // /
            advance(); // *
            while (!(currentChar() == '*' && peekChar() == '/') && currentChar() != '\0') {
                advance();
            }
            if (currentChar() == '*') {
                advance(); // *
                advance(); // /
            }
        }
    }
    
public:
    Lexer(const string& src) : input(src), pos(0), line(1), column(1) {}
    
    Token getNextToken() {
        skipWhitespace();
        
        while (currentChar() == '/' && (peekChar() == '/' || peekChar() == '*')) {
            if (peekChar() == '/') skipLineComment();
            else skipBlockComment();
            skipWhitespace();
        }
        
        startLine = line;
        startColumn = column;
        
        char ch = currentChar();
        
        if (ch == '\0') {
            return Token(TokenType::TOK_EOF, "", startLine, startColumn);
        }
        
        // Preprocessor
        if (ch == '#') {
            advance();
            string directive;
            while (currentChar() != '\n' && currentChar() != '\0') {
                directive += currentChar();
                advance();
            }
            return Token(TokenType::PREPROCESSOR, directive, startLine, startColumn);
        }
        
        // Delimiters
        if (ch == '(') { advance(); return Token(TokenType::LPAREN, "(", startLine, startColumn); }
        if (ch == ')') { advance(); return Token(TokenType::RPAREN, ")", startLine, startColumn); }
        if (ch == '{') { advance(); return Token(TokenType::LBRACE, "{", startLine, startColumn); }
        if (ch == '}') { advance(); return Token(TokenType::RBRACE, "}", startLine, startColumn); }
        if (ch == '[') { advance(); return Token(TokenType::LBRACKET, "[", startLine, startColumn); }
        if (ch == ']') { advance(); return Token(TokenType::RBRACKET, "]", startLine, startColumn); }
        if (ch == ';') { advance(); return Token(TokenType::SEMICOLON, ";", startLine, startColumn); }
        if (ch == ',') { advance(); return Token(TokenType::COMMA, ",", startLine, startColumn); }
        if (ch == '.') { advance(); return Token(TokenType::DOT, ".", startLine, startColumn); }
        
        // Operators
        if (ch == '+') {
            advance();
            if (currentChar() == '+') {
                advance();
                return Token(TokenType::OP_INC, "++", startLine, startColumn);
            }
            return Token(TokenType::OP_PLUS, "+", startLine, startColumn);
        }
        
        if (ch == '-') {
            advance();
            if (currentChar() == '-') {
                advance();
                return Token(TokenType::OP_DEC, "--", startLine, startColumn);
            }
            return Token(TokenType::OP_MINUS, "-", startLine, startColumn);
        }
        
        if (ch == '*') {
            advance();
            return Token(TokenType::OP_STAR, "*", startLine, startColumn);
        }
        
        if (ch == '/') {
            advance();
            return Token(TokenType::OP_SLASH, "/", startLine, startColumn);
        }
        
        if (ch == '%') {
            advance();
            return Token(TokenType::OP_PERCENT, "%", startLine, startColumn);
        }
        
        if (ch == '=') {
            advance();
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::OP_EQ, "==", startLine, startColumn);
            }
            return Token(TokenType::OP_ASSIGN, "=", startLine, startColumn);
        }
        
        if (ch == '!') {
            advance();
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::OP_NE, "!=", startLine, startColumn);
            }
            return Token(TokenType::OP_NOT, "!", startLine, startColumn);
        }
        
        if (ch == '<') {
            advance();
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::OP_LE, "<=", startLine, startColumn);
            }
            return Token(TokenType::OP_LT, "<", startLine, startColumn);
        }
        
        if (ch == '>') {
            advance();
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::OP_GE, ">=", startLine, startColumn);
            }
            return Token(TokenType::OP_GT, ">", startLine, startColumn);
        }
        
        if (ch == '&') {
            advance();
            if (currentChar() == '&') {
                advance();
                return Token(TokenType::OP_AND, "&&", startLine, startColumn);
            }
            return Token(TokenType::OP_BITAND, "&", startLine, startColumn);
        }
        
        if (ch == '|') {
            advance();
            if (currentChar() == '|') {
                advance();
                return Token(TokenType::OP_OR, "||", startLine, startColumn);
            }
            return Token(TokenType::OP_BITOR, "|", startLine, startColumn);
        }
        
        if (ch == '^') {
            advance();
            return Token(TokenType::OP_BITXOR, "^", startLine, startColumn);
        }
        
        if (ch == '~') {
            advance();
            return Token(TokenType::OP_BITNOT, "~", startLine, startColumn);
        }
        
        // String literals
        if (ch == '"') {
            advance();
            string str;
            while (currentChar() != '"' && currentChar() != '\0') {
                str += currentChar();
                advance();
            }
            if (currentChar() == '"') advance();
            return Token(TokenType::TOK_STRING, str, startLine, startColumn);
        }
        
        // Character literals
        if (ch == '\'') {
            advance();
            string chr;
            while (currentChar() != '\'' && currentChar() != '\0') {
                chr += currentChar();
                advance();
            }
            if (currentChar() == '\'') advance();
            return Token(TokenType::TOK_CHAR, chr, startLine, startColumn);
        }
        
        // Numbers
        if (isdigit(ch)) {
            string num;
            while (isdigit(currentChar()) || currentChar() == '.') {
                num += currentChar();
                advance();
            }
            return Token(TokenType::TOK_NUMBER, num, startLine, startColumn);
        }
        
        // Identifiers and keywords
        if (isalpha(ch) || ch == '_') {
            string ident;
            while (isalnum(currentChar()) || currentChar() == '_') {
                ident += currentChar();
                advance();
            }
            
            auto it = keywords.find(ident);
            if (it != keywords.end()) {
                return Token(it->second, ident, startLine, startColumn);
            }
            return Token(TokenType::TOK_IDENTIFIER, ident, startLine, startColumn);
        }
        
        // Unknown character
        advance();
        return Token(TokenType::TOK_EOF, "", startLine, startColumn);
    }
    
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
// SYMBOL TABLE - Manages variable and function declarations
// ============================================================================

struct Symbol {
    string name;
    string type;
    int line;
    int column;
    int scopeLevel;
};

class SymbolTable {
private:
    vector<vector<Symbol>> scopes;
    int scopeLevel;
    
public:
    SymbolTable() : scopeLevel(0) {
        scopes.push_back(vector<Symbol>());
    }
    
    void pushScope() {
        scopeLevel++;
        scopes.push_back(vector<Symbol>());
    }
    
    void popScope() {
        if (!scopes.empty()) {
            scopes.pop_back();
        }
        scopeLevel--;
    }
    
    bool declare(const string& name, const string& type, int line = 0, int column = 0) {
        if (scopes.empty()) return false;
        
        // Check for redeclaration in current scope
        for (const auto& sym : scopes.back()) {
            if (sym.name == name) {
                return false; // Already declared
            }
        }
        
        scopes.back().push_back({name, type, line, column, scopeLevel});
        return true;
    }
    
    bool exists(const string& name) const {
        // Search from innermost to outermost scope
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            for (const auto& sym : *it) {
                if (sym.name == name) {
                    return true;
                }
            }
        }
        return false;
    }
    
    string getType(const string& name) const {
        // Search from innermost to outermost scope
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            for (const auto& sym : *it) {
                if (sym.name == name) {
                    return sym.type;
                }
            }
        }
        return "UNKNOWN";
    }
    
    void clear() {
        scopes.clear();
        scopes.push_back(vector<Symbol>());
        scopeLevel = 0;
    }
};

// ============================================================================
// TYPE SYSTEM - Validates type compatibility
// ============================================================================

class TypeSystem {
public:
    static bool areTypesCompatible(const string& target, const string& source) {
        // Exact match
        if (target == source) return true;
        
        // Pointer compatibility
        if (target.back() == '*' && source.back() == '*') {
            return target == source;
        }
        
        // Numeric type compatibility (basic)
        set<string> numericTypes = {"int", "float", "double", "char", "short", "long"};
        if (numericTypes.count(target) && numericTypes.count(source)) {
            return true; // Allows implicit conversion
        }
        
        // Const compatibility
        if (target.find("const") != string::npos && source.find("const") != string::npos) {
            return target == source;
        }
        
        return false;
    }
};

// ============================================================================
// ERROR COLLECTOR & SUGGESTER
// ============================================================================

struct ErrorEntry {
    string message;
    string suggestion;
    int line;
    int column;
};

class Parser {
private:
    vector<Token> tokens;
    size_t pos;
    SymbolTable sym;
    vector<ErrorEntry> errors;
    int scopeDepth;
    
    Token curr() const {
        return (pos < tokens.size()) ? tokens[pos] : Token(TokenType::TOK_EOF);
    }
    
    Token peek(size_t offset = 1) const {
        return (pos + offset < tokens.size()) ? tokens[pos + offset] : Token(TokenType::TOK_EOF);
    }
    
    void advance() {
        if (pos < tokens.size()) pos++;
    }
    
    void expect(TokenType type, const string& expected) {
        if (curr().type != type) {
            errors.push_back({
                "Line " + to_string(curr().line) + ":" + to_string(curr().column) +
                " - Expected '" + expected + "' but got '" + curr().value + "'",
                "SUGGESTION: Add " + expected,
                curr().line, curr().column
            });
        } else {
            advance();
        }
    }
    
    bool isTypeToken(const Token& t) const {
        return (t.type == TokenType::KW_INT ||
                t.type == TokenType::KW_FLOAT ||
                t.type == TokenType::KW_CHAR ||
                t.type == TokenType::KW_DOUBLE ||
                t.type == TokenType::KW_VOID ||
                t.type == TokenType::KW_SHORT ||
                t.type == TokenType::KW_LONG ||
                t.type == TokenType::KW_SIGNED ||
                t.type == TokenType::KW_UNSIGNED ||
                t.type == TokenType::KW_STRUCT ||
                t.type == TokenType::KW_AUTO ||
                t.type == TokenType::KW_CONST);
    }
    
    string parseExpression() {
        if (curr().type == TokenType::TOK_IDENTIFIER) {
            string name = curr().value;
            advance();
            
            if (!sym.exists(name)) {
                errors.push_back({
                    "Line " + to_string(curr().line) + ":" + to_string(curr().column) +
                    " - Undeclared identifier '" + name + "'",
                    "SUGGESTION: Declare '" + name + "' before use",
                    curr().line, curr().column
                });
            }
            return sym.getType(name);
        } else if (curr().type == TokenType::TOK_NUMBER) {
            advance();
            return (curr().value.find('.') != string::npos) ? "float" : "int";
        } else if (curr().type == TokenType::TOK_STRING) {
            advance();
            return "string";
        } else if (curr().type == TokenType::OP_BITAND) {
            advance();
            if (curr().type == TokenType::TOK_IDENTIFIER) {
                string name = curr().value;
                advance();
                string baseType = sym.getType(name);
                return baseType + "*";
            }
            return "UNKNOWN";
        }
        return "UNKNOWN";
    }
    
    void parseVarDecl(const string& type, const string& ident, const Token& nameTok) {
        if (!sym.declare(ident, type, nameTok.line, nameTok.column)) {
            errors.push_back({
                "Line " + to_string(nameTok.line) + ":" + to_string(nameTok.column) +
                " - Redeclaration of '" + ident + "'",
                "SUGGESTION: Variable already declared",
                nameTok.line, nameTok.column
            });
        }
        
        if (curr().type == TokenType::LBRACKET) {
            advance();
            if (curr().type == TokenType::TOK_NUMBER) {
                advance();
            }
            expect(TokenType::RBRACKET, "]");
        }
        
        if (curr().type == TokenType::OP_ASSIGN) {
            advance();
            string rhsType = parseExpression();
            if (!TypeSystem::areTypesCompatible(type, rhsType)) {
                errors.push_back({
                    "Line " + to_string(curr().line) + ":" + to_string(curr().column) +
                    " - Type mismatch: '" + rhsType + "' to '" + type + "'",
                    "SUGGESTION: Types must match",
                    curr().line, curr().column
                });
            }
        }
        
        while (curr().type == TokenType::COMMA) {
            advance();
            if (curr().type == TokenType::TOK_IDENTIFIER) {
                Token t = curr();
                advance();
                if (!sym.declare(t.value, type, t.line, t.column)) {
                    errors.push_back({
                        "Line " + to_string(t.line) + ":" + to_string(t.column) +
                        " - Redeclaration",
                        "SUGGESTION: Already declared",
                        t.line, t.column
                    });
                }
            }
        }
        
        expect(TokenType::SEMICOLON, ";");
    }
    
    void parseFunction(const string& type, const string& ident, const Token& nameTok) {
        if (scopeDepth > 0) {
            errors.push_back({
                "Line " + to_string(nameTok.line) + ":" + to_string(nameTok.column) +
                " - Nested function not allowed",
                "SUGGESTION: Move to file scope",
                nameTok.line, nameTok.column
            });
            return;
        }
        
        if (sym.exists(ident) && sym.getType(ident) == "function") {
            errors.push_back({
                "Line " + to_string(nameTok.line) + ":" + to_string(nameTok.column) +
                " - Function redeclaration '" + ident + "'",
                "SUGGESTION: Function already declared",
                nameTok.line, nameTok.column
            });
        }
        
        sym.declare(ident, "function", nameTok.line, nameTok.column);
        advance();
        sym.pushScope();
        scopeDepth++;
        
        // Parse parameters
        if (curr().type != TokenType::RPAREN) {
            while (isTypeToken(curr())) {
                string paramType = curr().value;
                advance();
                
                while (curr().type == TokenType::OP_STAR) {
                    paramType += "*";
                    advance();
                }
                
                if (curr().type == TokenType::TOK_IDENTIFIER) {
                    sym.declare(curr().value, paramType);
                    advance();
                }
                
                if (curr().type == TokenType::COMMA) advance();
                else break;
            }
        }
        
        expect(TokenType::RPAREN, ")");
        
        if (curr().type == TokenType::SEMICOLON) {
            advance();
            scopeDepth--;
            sym.popScope();
            return;
        }
        
        if (curr().type == TokenType::LBRACE) {
            advance();
            parseBlock();
        }
        
        scopeDepth--;
        sym.popScope();
    }
    
    void parseBlock() {
        while (curr().type != TokenType::RBRACE && curr().type != TokenType::TOK_EOF) {
            parseStatement();
        }
        expect(TokenType::RBRACE, "}");
    }
    
    void parseStatement() {
        Token t = curr();
        
        if (t.type == TokenType::PREPROCESSOR) {
            advance();
            return;
        }
        
        if (t.type == TokenType::LBRACE) {
            advance();
            sym.pushScope();
            parseBlock();
            sym.popScope();
            return;
        }
        
        if (t.type == TokenType::KW_IF) {
            advance();
            expect(TokenType::LPAREN, "(");
            parseExpression();
            expect(TokenType::RPAREN, ")");
            parseStatement();
            if (curr().type == TokenType::KW_ELSE) {
                advance();
                parseStatement();
            }
            return;
        }
        
        if (t.type == TokenType::KW_WHILE || t.type == TokenType::KW_FOR) {
            advance();
            expect(TokenType::LPAREN, "(");
            parseExpression();
            expect(TokenType::RPAREN, ")");
            parseStatement();
            return;
        }
        
        if (t.type == TokenType::KW_RETURN) {
            advance();
            if (curr().type != TokenType::SEMICOLON) {
                parseExpression();
            }
            expect(TokenType::SEMICOLON, ";");
            return;
        }
        
        if (t.type == TokenType::SEMICOLON) {
            advance();
            return;
        }
        
        // Handle typedef
        if (t.type == TokenType::KW_TYPEDEF) {
            advance();
            if (isTypeToken(curr())) {
                advance();
                while (curr().type == TokenType::OP_STAR) {
                    advance();
                }
                if (curr().type == TokenType::TOK_IDENTIFIER) {
                    advance();
                }
            }
            expect(TokenType::SEMICOLON, ";");
            return;
        }
        
        // Handle const
        if (t.type == TokenType::KW_CONST) {
            advance();
            if (isTypeToken(curr())) {
                string type = "const " + curr().value;
                advance();
                
                while (curr().type == TokenType::OP_STAR) {
                    type += "*";
                    advance();
                }
                
                if (curr().type == TokenType::TOK_IDENTIFIER) {
                    Token nameTok = curr();
                    string ident = nameTok.value;
                    advance();
                    parseVarDecl(type, ident, nameTok);
                }
            }
            return;
        }
        
        // Handle struct
        if (t.type == TokenType::KW_STRUCT) {
            advance();
            if (curr().type == TokenType::TOK_IDENTIFIER) {
                advance();
                
                if (curr().type == TokenType::LBRACE) {
                    advance();
                    while (curr().type != TokenType::RBRACE && curr().type != TokenType::TOK_EOF) {
                        if (isTypeToken(curr())) {
                            advance();
                            while (curr().type == TokenType::OP_STAR) {
                                advance();
                            }
                            if (curr().type == TokenType::TOK_IDENTIFIER) {
                                advance();
                            }
                            expect(TokenType::SEMICOLON, ";");
                        } else {
                            advance();
                        }
                    }
                    expect(TokenType::RBRACE, "}");
                }
                expect(TokenType::SEMICOLON, ";");
            }
            return;
        }
        
        // Handle type declarations (int, float, etc.)
        if (isTypeToken(t)) {
            string type = t.value;
            advance();
            
            // Handle pointers
            while (curr().type == TokenType::OP_STAR) {
                type += "*";
                advance();
            }
            
            if (curr().type == TokenType::TOK_IDENTIFIER) {
                Token nameTok = curr();
                string ident = nameTok.value;
                advance();
                
                if (curr().type == TokenType::LPAREN) {
                    parseFunction(type, ident, nameTok);
                } else {
                    parseVarDecl(type, ident, nameTok);
                }
            }
            return;
        }
        
        // Parse as expression or assignment
        parseExpression();
        if (curr().type == TokenType::SEMICOLON) {
            advance();
        }
    }
    
public:
    Parser(const vector<Token>& toks) 
        : tokens(toks), pos(0), scopeDepth(0) {}
    
    void parse() {
        sym.pushScope(); // Global scope
        
        while (curr().type != TokenType::TOK_EOF) {
            parseStatement();
        }
    }
    
    vector<ErrorEntry> getErrors() const {
        return errors;
    }
};

// ============================================================================
// PUBLIC API
// ============================================================================

struct AnalysisResult {
    vector<string> errors;
    vector<string> suggestions;
    int totalErrors;
};

AnalysisResult analyzeCode(const string& sourceCode) {
    Lexer lexer(sourceCode);
    vector<Token> tokens = lexer.tokenizeAll();
    
    Parser parser(tokens);
    parser.parse();
    
    vector<ErrorEntry> parserErrors = parser.getErrors();
    
    AnalysisResult result;
    result.totalErrors = parserErrors.size();
    
    for (const auto& err : parserErrors) {
        result.errors.push_back(err.message);
        result.suggestions.push_back(err.suggestion);
    }
    
    return result;
}

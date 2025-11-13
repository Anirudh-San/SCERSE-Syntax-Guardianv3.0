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
#include <set>

using namespace std;

// ============================================================================
// FORWARD DECLARATIONS & TYPES
// ============================================================================

struct AnalysisResult
{
    vector<string> lexicalErrors;
    vector<pair<string, string>> syntaxErrors; // (error, suggestion)
    int totalErrors;
};

// ============================================================================
// ERROR SUGGESTION ENGINE MODULE
// ============================================================================

struct ErrorSuggestion
{
    string error;
    string suggestion;
    string example;
    ErrorSuggestion(string e = "", string s = "", string ex = "")
        : error(e), suggestion(s), example(ex) {}
};

class SuggestionEngine
{
private:
    vector<ErrorSuggestion> suggestions;

public:
    SuggestionEngine()
    {
        suggestions.push_back(ErrorSuggestion(
            "Expected ';'",
            "Add a semicolon at the end of the statement",
            "int x = 5;  // <- Correct"));
        suggestions.push_back(ErrorSuggestion(
            "Expected '('",
            "Control structures need parentheses around condition",
            "if (x > 5) { } while (y < 10) { }"));
        suggestions.push_back(ErrorSuggestion(
            "Expected ')'",
            "Close the opening parenthesis - check for balanced parentheses",
            "function(arg1, arg2);  // <- All parentheses balanced"));
        suggestions.push_back(ErrorSuggestion(
            "Expected '}'",
            "Close the opening brace - check for balanced braces",
            "void func() { int x = 5; }  // <- Each { has matching }"));
        suggestions.push_back(ErrorSuggestion(
            "Undeclared variable",
            "Declare variable before using: type varname;",
            "int x; x = 5;  // <- Declare BEFORE using"));
        suggestions.push_back(ErrorSuggestion(
            "Redeclaration",
            "Variable already exists in this scope. Use different name",
            "int x = 5; int y = 10;  // <- Use different name"));
        suggestions.push_back(ErrorSuggestion(
            "Invalid numeric literal",
            "Check for multiple decimals or invalid characters",
            "float x = 3.14;  // <- Correct"));
        suggestions.push_back(ErrorSuggestion(
            "Unterminated string",
            "String literals must have opening AND closing quotes",
            "char* s = \"hello\";  // <- Both sides have quotes"));
        suggestions.push_back(ErrorSuggestion(
            "Unterminated character",
            "Character literals must have opening AND closing quotes",
            "char c = 'A';  // <- Both sides have quotes"));
        suggestions.push_back(ErrorSuggestion(
            "Multi-character constant",
            "Character literals can only contain ONE character",
            "char c = 'A';  // <- Correct"));
        suggestions.push_back(ErrorSuggestion(
            "Invalid character",
            "Remove invalid characters. Common: @, #, $ in wrong context",
            "int x = 5 + 10;  // <- Correct"));
        suggestions.push_back(ErrorSuggestion(
            "Unexpected token",
            "This token not expected in position. Check grammar",
            "int x = 5 * 10;  // <- Correct"));
        suggestions.push_back(ErrorSuggestion(
            "Expected type",
            "Type specifier needed: int, float, char, void, double",
            "int x;  float y;  char z;  // <- All have types"));
        suggestions.push_back(ErrorSuggestion(
            "Missing #endif",
            "Preprocessor conditional #if must have matching #endif",
            "#if defined(DEBUG)\\n// code\\n#endif  // <- Proper pairing"));
    }

    string getSuggestion(const string &errorMsg)
    {
        for (const auto &s : suggestions)
        {
            if (errorMsg.find(s.error) != string::npos)
            {
                return "SUGGESTION: " + s.suggestion + " | EXAMPLE: " + s.example;
            }
        }
        return "";
    }

    void addCustomSuggestion(const string &e, const string &s, const string &ex)
    {
        suggestions.push_back(ErrorSuggestion(e, s, ex));
    }
};

// ============================================================================
// STANDARD LIBRARY MODULE (stdio, stdlib, etc.)
// ============================================================================

class StandardLibrary
{
private:
    unordered_set<string> stdioFunctions;
    unordered_set<string> stdlibFunctions;
    unordered_set<string> stringFunctions;
    unordered_set<string> mathFunctions;
    unordered_map<string, string> functionSignatures;

public:
    StandardLibrary()
    {
        // stdio.h functions
        stdioFunctions = {
            "printf", "scanf", "fprintf", "fscanf", "sprintf", "sscanf",
            "fopen", "fclose", "fread", "fwrite", "fgets", "fputs",
            "getchar", "putchar", "gets", "puts", "perror"};

        // stdlib.h functions
        stdlibFunctions = {
            "malloc", "calloc", "realloc", "free", "exit", "abort",
            "atoi", "atof", "atol", "rand", "srand", "qsort"};

        // string.h functions
        stringFunctions = {
            "strcpy", "strncpy", "strlen", "strcmp", "strcat",
            "strchr", "strstr", "memset", "memcpy", "memmove"};

        // math.h functions
        mathFunctions = {
            "sin", "cos", "tan", "sqrt", "pow", "abs", "floor", "ceil"};

        // Function signatures for type checking
        functionSignatures["printf"] = "int printf(const char* format, ...)";
        functionSignatures["scanf"] = "int scanf(const char* format, ...)";
        functionSignatures["malloc"] = "void* malloc(size_t size)";
        functionSignatures["free"] = "void free(void* ptr)";
        functionSignatures["strlen"] = "size_t strlen(const char* s)";
    }

    bool isStdioFunction(const string &name) const { return stdioFunctions.count(name) > 0; }
    bool isStdlibFunction(const string &name) const { return stdlibFunctions.count(name) > 0; }
    bool isStringFunction(const string &name) const { return stringFunctions.count(name) > 0; }
    bool isMathFunction(const string &name) const { return mathFunctions.count(name) > 0; }
    bool isStandardFunction(const string &name) const
    {
        return isStdioFunction(name) || isStdlibFunction(name) ||
               isStringFunction(name) || isMathFunction(name);
    }

    string getFunctionSignature(const string &name) const
    {
        auto it = functionSignatures.find(name);
        return it != functionSignatures.end() ? it->second : "";
    }
};

// ============================================================================
// PREPROCESSOR MODULE (handles #include, #define, etc.)
// ============================================================================

class PreprocessorHandler
{
private:
    vector<string> errors;
    unordered_set<string> includedHeaders;

public:
    void processInclude(const string &line, int lineNum)
    {
        if (line.find("#include") != 0)
            return;

        size_t start = line.find_first_of("<\"");
        size_t end = line.find_last_of(">\"");

        if (start == string::npos || end == string::npos || start >= end)
        {
            errors.push_back("Line " + to_string(lineNum) + " - Invalid #include syntax");
            return;
        }

        string headerName = line.substr(start + 1, end - start - 1);
        includedHeaders.insert(headerName);
    }

    void processPreprocessor(const string &line, int lineNum)
    {
        if (line.find("#if") == 0 || line.find("#ifdef") == 0 || line.find("#ifndef") == 0)
        {
            // Track conditional compilation
        }
        if (line.find("#include") == 0)
        {
            processInclude(line, lineNum);
        }
    }

    bool isHeaderIncluded(const string &header) const
    {
        return includedHeaders.count(header) > 0;
    }

    vector<string> getErrors() const { return errors; }
};

// ============================================================================
// TOKEN DEFINITIONS
// ============================================================================

enum class TokenType
{
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,
    TOK_CHAR,
    KW_INT,
    KW_FLOAT,
    KW_CHAR,
    KW_VOID,
    KW_DOUBLE,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_FOR,
    KW_DO,
    KW_RETURN,
    KW_BREAK,
    KW_CONTINUE,
    KW_SWITCH,
    KW_CASE,
    KW_DEFAULT,
    KW_TYPEDEF,
    KW_SIZEOF,
    KW_CONST,
    KW_STATIC,
    KW_EXTERN,
    KW_STRUCT, // ADD: struct keyword
    KW_AUTO,   // ADD: auto keyword
    OP_PLUS,
    OP_MINUS,
    OP_STAR,
    OP_SLASH,
    OP_PERCENT,
    OP_ASSIGN,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_INC,
    OP_DEC,
    OP_PLUSEQ,
    OP_MINUSEQ,
    OP_BITAND,
    OP_BITOR,
    OP_BITXOR,
    OP_BITNOT,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    SEMICOLON,
    COMMA,
    DOT,
    ARROW,
    COLON,
    QUESTION,
    PREPROCESSOR,
    TOK_ERROR,
    TOK_UNKNOWN
};

struct Token
{
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

class Lexer
{
private:
    string input;
    size_t pos;
    int line, column;
    vector<string> errors;
    PreprocessorHandler preprocessor;

    unordered_map<string, TokenType> keywords = {
        {"int", TokenType::KW_INT},
        {"float", TokenType::KW_FLOAT},
        {"char", TokenType::KW_CHAR},
        {"void", TokenType::KW_VOID},
        {"double", TokenType::KW_DOUBLE},
        {"if", TokenType::KW_IF},
        {"else", TokenType::KW_ELSE},
        {"while", TokenType::KW_WHILE},
        {"for", TokenType::KW_FOR},
        {"do", TokenType::KW_DO},
        {"return", TokenType::KW_RETURN},
        {"break", TokenType::KW_BREAK},
        {"continue", TokenType::KW_CONTINUE},
        {"switch", TokenType::KW_SWITCH},
        {"case", TokenType::KW_CASE},
        {"default", TokenType::KW_DEFAULT},
        {"struct", TokenType::KW_STRUCT},
        {"typedef", TokenType::KW_TYPEDEF},
        {"sizeof", TokenType::KW_SIZEOF},
        {"const", TokenType::KW_CONST},
        {"static", TokenType::KW_STATIC},
        {"extern", TokenType::KW_EXTERN},
        {"auto", TokenType::KW_AUTO}};

    char currentChar() { return pos >= input.length() ? '\0' : input[pos]; }
    char peekChar(int offset = 1) { return pos + offset >= input.length() ? '\0' : input[pos + offset]; }

    void advance()
    {
        if (pos < input.length())
        {
            if (input[pos] == '\n')
            {
                line++;
                column = 1;
            }
            else
            {
                column++;
            }
            pos++;
        }
    }

    void skipWhitespace()
    {
        while (isspace(currentChar()))
            advance();
    }

    void skipComment()
    {
        if (currentChar() == '/' && peekChar() == '/')
        {
            while (currentChar() != '\n' && currentChar() != '\0')
                advance();
        }
        else if (currentChar() == '/' && peekChar() == '*')
        {
            int cLine = line, cCol = column;
            advance();
            advance();
            while (true)
            {
                if (currentChar() == '\0')
                {
                    errors.push_back("Line " + to_string(cLine) + ":" + to_string(cCol) + " - Unterminated comment");
                    break;
                }
                if (currentChar() == '*' && peekChar() == '/')
                {
                    advance();
                    advance();
                    break;
                }
                advance();
            }
        }
    }

    Token lexNumber()
    {
        int sL = line, sC = column;
        string num;
        bool hasDec = false;
        while (isdigit(currentChar()) || currentChar() == '.')
        {
            if (currentChar() == '.')
            {
                if (hasDec)
                    errors.push_back("Line " + to_string(line) + ":" + to_string(column) + " - Invalid numeric literal: multiple decimal points");
                hasDec = true;
            }
            num += currentChar();
            advance();
        }
        if (isalpha(currentChar()))
        {
            errors.push_back("Line " + to_string(line) + ":" + to_string(column) + " - Invalid numeric literal");
            while (isalnum(currentChar()))
            {
                num += currentChar();
                advance();
            }
            return Token(TokenType::TOK_ERROR, num, sL, sC);
        }
        return Token(TokenType::TOK_NUMBER, num, sL, sC);
    }

    Token lexIdentifier()
    {
        int sL = line, sC = column;
        string id;
        while (isalnum(currentChar()) || currentChar() == '_')
        {
            id += currentChar();
            advance();
        }
        if (keywords.find(id) != keywords.end())
            return Token(keywords[id], id, sL, sC);
        return Token(TokenType::TOK_IDENTIFIER, id, sL, sC);
    }

    Token lexString()
    {
        int sL = line, sC = column;
        string result = "";
        char quote = currentChar(); // Remember opening quote (")

        result += quote; // Include opening quote
        advance();

        bool closed = false;

        // Read until closing quote or EOF
        while (currentChar() != '\0')
        {
            char c = currentChar();

            // Found closing quote
            if (c == quote)
            {
                result += quote; // Include closing quote
                advance();
                closed = true;
                break;
            }

            // Newline in string = unterminated error
            if (c == '\n')
            {
                errors.push_back("Line " + to_string(sL) + ":" + to_string(sC) +
                                 " - Unterminated string literal (newline in string)");
                break;
            }

            // Escape sequence handling
            if (c == '\\')
            {
                result += c; // Add backslash
                advance();

                if (currentChar() != '\0')
                {
                    // Add the escaped character (n, t, ", \, %, d, x, etc.)
                    result += currentChar();
                    advance();
                }
            }
            else
            {
                // Regular character - just add it
                // This includes: %, digits, letters, spaces, etc.
                result += c;
                advance();
            }
        }

        if (!closed)
        {
            errors.push_back("Line " + to_string(sL) + ":" + to_string(sC) +
                             " - Unterminated string literal (EOF reached)");
            return Token(TokenType::TOK_ERROR, result, sL, sC);
        }

        return Token(TokenType::TOK_STRING, result, sL, sC);
    }

    Token lexChar()
    {
        int sL = line, sC = column;
        string result = "";
        char quote = currentChar(); // Remember opening quote (')

        result += quote; // Include opening quote
        advance();

        bool closed = false;
        int charCount = 0;

        // Read until closing quote or EOF
        while (currentChar() != '\0')
        {
            char c = currentChar();

            // Found closing quote
            if (c == quote)
            {
                result += quote; // Include closing quote
                advance();
                closed = true;
                break;
            }

            // Newline in char = unterminated error
            if (c == '\n')
            {
                errors.push_back("Line " + to_string(sL) + ":" + to_string(sC) +
                                 " - Unterminated character literal (newline in char)");
                break;
            }

            // Escape sequence handling
            if (c == '\\')
            {
                result += c; // Add backslash
                advance();

                if (currentChar() != '\0')
                {
                    // Add the escaped character
                    result += currentChar();
                    advance();
                    charCount += 2; // Escape sequence counts as 1 char
                }
            }
            else
            {
                // Regular character
                result += c;
                advance();
                charCount++;
            }
        }

        if (!closed)
        {
            errors.push_back("Line " + to_string(sL) + ":" + to_string(sC) +
                             " - Unterminated character literal (EOF reached)");
            return Token(TokenType::TOK_ERROR, result, sL, sC);
        }

        // Validate character count
        if (charCount == 0 || charCount > 1)
        {
            if (charCount == 0)
            {
                errors.push_back("Line " + to_string(sL) + ":" + to_string(sC) +
                                 " - Empty character literal");
            }
            else
            {
                errors.push_back("Line " + to_string(sL) + ":" + to_string(sC) +
                                 " - Multi-character constant");
            }
        }

        return Token(TokenType::TOK_CHAR, result, sL, sC);
    }

public:
    Lexer(const string &src) : input(src), pos(0), line(1), column(1) {}

    Token getNextToken()
    {
        skipWhitespace();
        while ((currentChar() == '/' && (peekChar() == '/' || peekChar() == '*')))
        {
            skipComment();
            skipWhitespace();
        }

        int sL = line, sC = column;
        char c = currentChar();

        if (c == '\0')
            return Token(TokenType::TOK_EOF, "", line, column);

        // Preprocessor directives
        if (c == '#')
        {
            string prep;
            while (currentChar() != '\n' && currentChar() != '\0')
            {
                prep += currentChar();
                advance();
            }
            preprocessor.processPreprocessor(prep, sL);
            return Token(TokenType::PREPROCESSOR, prep, sL, sC);
        }

        if (isdigit(c))
            return lexNumber();
        if (isalpha(c) || c == '_')
            return lexIdentifier();
        if (c == '"')
            return lexString();
        if (c == '\'')
            return lexChar();

        switch (c)
        {
        case '(':
            advance();
            return Token(TokenType::LPAREN, "(", sL, sC);
        case ')':
            advance();
            return Token(TokenType::RPAREN, ")", sL, sC);
        case '{':
            advance();
            return Token(TokenType::LBRACE, "{", sL, sC);
        case '}':
            advance();
            return Token(TokenType::RBRACE, "}", sL, sC);
        case '[':
            advance();
            return Token(TokenType::LBRACKET, "[", sL, sC);
        case ']':
            advance();
            return Token(TokenType::RBRACKET, "]", sL, sC);
        case ';':
            advance();
            return Token(TokenType::SEMICOLON, ";", sL, sC);
        case ',':
            advance();
            return Token(TokenType::COMMA, ",", sL, sC);
        case '.':
            advance();
            return Token(TokenType::DOT, ".", sL, sC);
        case ':':
            advance();
            return Token(TokenType::COLON, ":", sL, sC);
        case '?':
            advance();
            return Token(TokenType::QUESTION, "?", sL, sC);
        case '+':
            advance();
            if (currentChar() == '+')
            {
                advance();
                return Token(TokenType::OP_INC, "++", sL, sC);
            }
            if (currentChar() == '=')
            {
                advance();
                return Token(TokenType::OP_PLUSEQ, "+=", sL, sC);
            }
            return Token(TokenType::OP_PLUS, "+", sL, sC);
        case '-':
            advance();
            if (currentChar() == '-')
            {
                advance();
                return Token(TokenType::OP_DEC, "--", sL, sC);
            }
            if (currentChar() == '=')
            {
                advance();
                return Token(TokenType::OP_MINUSEQ, "-=", sL, sC);
            }
            if (currentChar() == '>')
            {
                advance();
                return Token(TokenType::ARROW, "->", sL, sC);
            }
            return Token(TokenType::OP_MINUS, "-", sL, sC);
        case '*':
            advance();
            return Token(TokenType::OP_STAR, "*", sL, sC);
        case '/':
            advance();
            return Token(TokenType::OP_SLASH, "/", sL, sC);
        case '%':
            advance();
            return Token(TokenType::OP_PERCENT, "%", sL, sC);
        case '=':
            advance();
            if (currentChar() == '=')
            {
                advance();
                return Token(TokenType::OP_EQ, "==", sL, sC);
            }
            return Token(TokenType::OP_ASSIGN, "=", sL, sC);
        case '!':
            advance();
            if (currentChar() == '=')
            {
                advance();
                return Token(TokenType::OP_NE, "!=", sL, sC);
            }
            return Token(TokenType::OP_NOT, "!", sL, sC);
        case '<':
            advance();
            if (currentChar() == '=')
            {
                advance();
                return Token(TokenType::OP_LE, "<=", sL, sC);
            }
            return Token(TokenType::OP_LT, "<", sL, sC);
        case '>':
            advance();
            if (currentChar() == '=')
            {
                advance();
                return Token(TokenType::OP_GE, ">=", sL, sC);
            }
            return Token(TokenType::OP_GT, ">", sL, sC);
        case '&':
            advance();
            if (currentChar() == '&')
            {
                advance();
                return Token(TokenType::OP_AND, "&&", sL, sC);
            }
            return Token(TokenType::OP_BITAND, "&", sL, sC);
        case '|':
            advance();
            if (currentChar() == '|')
            {
                advance();
                return Token(TokenType::OP_OR, "||", sL, sC);
            }
            return Token(TokenType::OP_BITOR, "|", sL, sC);
        case '^':
            advance();
            return Token(TokenType::OP_BITXOR, "^", sL, sC);
        case '~':
            advance();
            return Token(TokenType::OP_BITNOT, "~", sL, sC);
        default:
            errors.push_back("Line " + to_string(line) + ":" + to_string(column) + " - Invalid character: '" + string(1, c) + "'");
            advance();
            return Token(TokenType::TOK_ERROR, string(1, c), sL, sC);
        }
    }

    vector<string> getErrors() const { return errors; }

    vector<Token> tokenizeAll()
    {
        vector<Token> tokens;
        Token tok;
        do
        {
            tok = getNextToken();
            tokens.push_back(tok);
        } while (tok.type != TokenType::TOK_EOF);
        return tokens;
    }
};

// ============================================================================
// SYMBOL TABLE MODULE
// ============================================================================

struct VarInfo
{
    string name, type;
    int line, column; // Track where variable was declared
    VarInfo(string n = "", string t = "", int l = 0, int c = 0)
        : name(n), type(t), line(l), column(c) {}
};

class TypeSystem
{
public:
    // Check if two types are compatible
    static bool isInteger(const string &t)
    {
        return t == "int" || t == "long" || t == "short";
    }

    static bool isFloat(const string &t)
    {
        return t == "float" || t == "double";
    }

    static bool isChar(const string &t)
    {
        return t == "char";
    }

    static bool isString(const string &t)
    {
        return t == "string";
    }

    static bool isPointer(const string &t)
    {
        return t.size() > 1 && t.back() == '*';
    }

    static bool isVoid(const string &t)
    {
        return t == "void";
    }

    static bool isStruct(const string &t)
    {
        return t.rfind("struct ", 0) == 0;
    }

    static string basePointerType(const string &t)
    {
        return t.substr(0, t.size() - 1);
    }

    static bool areTypesCompatible(const string &lhs, const string &rhs)
    {
        // ===============================
        // EXACT MATCH
        // ===============================
        if (lhs == rhs)
            return true;

        // ===============================
        // VOID TYPE RULES
        // ===============================
        if (isVoid(lhs) || isVoid(rhs))
            return false; // void cannot be assigned or stored

        // ===============================
        // NULL → pointer allowed
        // ===============================
        if (isPointer(lhs) && rhs == "0")
            return true;

        // ===============================
        // POINTERS
        // ===============================
        if (isPointer(lhs) || isPointer(rhs))
        {
            // pointer = pointer
            if (isPointer(lhs) && isPointer(rhs))
            {
                string a = basePointerType(lhs);
                string b = basePointerType(rhs);

                // e.g., int* = int*
                if (a == b)
                    return true;

                // void* is compatible with any pointer
                if (a == "void" || b == "void")
                    return true;

                // otherwise: incompatible pointer types
                return false;
            }

            // pointer = non-pointer → INVALID
            return false;
        }

        // ===============================
        // STRUCTS
        // ===============================
        if (isStruct(lhs) || isStruct(rhs))
            return lhs == rhs; // structs are compatible only if same type

        // ===============================
        // STRING TYPE (C string literal)
        // ===============================
        if (isString(lhs) || isString(rhs))
        {
            // only string = string allowed
            return lhs == rhs;
        }

        // ===============================
        // CHAR TYPE RULES
        // ===============================
        if (isChar(lhs))
        {
            // char = char OK
            if (isChar(rhs))
                return true;

            // char = int → DANGEROUS (narrowing)
            if (isInteger(rhs))
                return false;

            // char = float → BAD
            if (isFloat(rhs))
                return false;

            return false;
        }

        if (isChar(rhs))
        {
            // int = char → allowed (widening)
            if (isInteger(lhs))
                return true;

            // float = char → allowed but questionable
            if (isFloat(lhs))
                return true;

            // char = float already covered
            return false;
        }

        // ===============================
        // INTEGER TYPE RULES
        // ===============================
        if (isInteger(lhs) && isInteger(rhs))
            return true; // safe

        // int = float → NARROWING (disallowed)
        if (isInteger(lhs) && isFloat(rhs))
            return false;

        // float = int → widening (allowed)
        if (isFloat(lhs) && isInteger(rhs))
            return true;

        // ===============================
        // FLOAT TYPE RULES
        // ===============================
        if (isFloat(lhs) && isFloat(rhs))
            return true;

        // float = char → allowed (widening)
        if (isFloat(lhs) && isChar(rhs))
            return true;

        // char = float → disallowed (already covered)

        // ===============================
        // FALLBACK: INCOMPATIBLE
        // ===============================
        return false;
    }

    // Get type of binary operation result
    static string getOperationResultType(const string &lhs, const string &rhs, const string &op)
    {
        set<string> numericTypes = {"int", "float", "double", "char"};

        // Arithmetic operations
        if (op == "+" || op == "-" || op == "*" || op == "/")
        {
            if (numericTypes.count(lhs) && numericTypes.count(rhs))
            {
                // If either is float/double, result is float/double
                if (lhs == "float" || rhs == "float")
                    return "float";
                if (lhs == "double" || rhs == "double")
                    return "double";
                return "int";
            }
            // String concatenation with +
            if ((lhs == "string" || lhs.find("*char") != string::npos) && op == "+")
            {
                return "string";
            }
            return "INVALID"; // Type error
        }

        // Comparison operations return int (bool)
        if (op == "==" || op == "!=" || op == "<" || op == ">" ||
            op == "<=" || op == ">=")
        {
            if (areTypesCompatible(lhs, rhs))
                return "int";
            return "INVALID";
        }

        // Logical operations
        if (op == "&&" || op == "||")
        {
            return "int";
        }

        // Bitwise operations
        if (op == "&" || op == "|" || op == "^" || op == "<<" || op == ">>")
        {
            if (numericTypes.count(lhs) && numericTypes.count(rhs))
            {
                return "int";
            }
            return "INVALID";
        }

        return "UNKNOWN";
    }

    static bool isNumericType(const string &type)
    {
        set<string> numeric = {"int", "float", "double", "char"};
        return numeric.count(type) > 0;
    }

    static bool isPointerType(const string &type)
    {
        return type.find("*") != string::npos;
    }
};

class SymbolTable
{
    vector<unordered_map<string, VarInfo>> scopes;
    StandardLibrary stdLib;

public:
    SymbolTable() { pushScope(); }
    void pushScope() { scopes.push_back({}); }
    void popScope()
    {
        if (!scopes.empty())
            scopes.pop_back();
    }

    bool declare(const string &n, const string &t, int line = 0, int col = 0)
    {
        auto &c = scopes.back();
        if (c.count(n))
            return false;
        c[n] = VarInfo(n, t, line, col);
        return true;
    }

    // Get the type of a variable
    string getType(const string &n) const
    {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
        {
            if (it->count(n))
                return it->at(n).type;
        }
        // Check if it's a standard library function
        if (stdLib.isStdioFunction(n))
            return "function";
        if (stdLib.isStdlibFunction(n))
            return "function";
        if (stdLib.isStringFunction(n))
            return "function";
        if (stdLib.isMathFunction(n))
            return "function";
        return "UNKNOWN";
    }

    bool exists(const string &n) const
    {
        if (stdLib.isStdioFunction(n) ||
            stdLib.isStdlibFunction(n) ||
            stdLib.isStringFunction(n) ||
            stdLib.isMathFunction(n))
        {
            return true;
        }
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
        {
            if (it->count(n))
                return true;
        }
        return false;
    }
};

// ============================================================================
// PARSER MODULE
// ============================================================================

class Parser
{
private:
    vector<Token> tokens;
    size_t index;
    SymbolTable sym;
    vector<pair<string, string>> errors;
    SuggestionEngine suggestionEngine;
    StandardLibrary stdLib;
    size_t lastIndex;
    TypeSystem typeChecker;
    int scopeDepth = 0; // Track current scope depth

    Token curr() const { return index < tokens.size() ? tokens[index] : Token(TokenType::TOK_EOF, ""); }
    Token peek(int offset = 1) const { return index + offset < tokens.size() ? tokens[index + offset] : Token(TokenType::TOK_EOF, ""); }

    void advance()
    {
        if (index < tokens.size())
            index++;
    }

    void forceAdvance()
    {
        if (index == lastIndex)
        {
            Token bad = curr();
            string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Skipping invalid token '" + bad.value + "'";
            string sug = suggestionEngine.getSuggestion(errMsg);
            errors.push_back({errMsg, sug});
            advance();
        }
        lastIndex = index;
    }

    void expect(TokenType type, const string &desc)
    {
        Token bad = curr();
        if (bad.type != type)
        {
            string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Expected '" + desc + "' but got '" + bad.value + "'";
            string sug = suggestionEngine.getSuggestion(errMsg);
            errors.push_back({errMsg, sug});
        }
        else
        {
            advance();
        }
    }

    bool isTypeToken(const Token &t) const
    {
        if (t.type == TokenType::KW_INT || t.type == TokenType::KW_FLOAT ||
            t.type == TokenType::KW_CHAR || t.type == TokenType::KW_DOUBLE ||
            t.type == TokenType::KW_VOID || t.type == TokenType::KW_STRUCT ||
            t.type == TokenType::KW_AUTO)
            return true;

        // NEW: typedef names act like types
        if (t.type == TokenType::TOK_IDENTIFIER)
        {
            string ty = sym.getType(t.value);
            if (!ty.empty() && ty.rfind("typedef:", 0) == 0)
                return true;
        }
        return false;
    }

    bool isComparisonOp(const Token &t)
    {
        return t.type == TokenType::OP_LT || t.type == TokenType::OP_GT ||
               t.type == TokenType::OP_LE || t.type == TokenType::OP_GE ||
               t.type == TokenType::OP_EQ || t.type == TokenType::OP_NE;
    }

    bool isOp(const Token &t)
    {
        return t.type == TokenType::OP_PLUS || t.type == TokenType::OP_MINUS ||
               t.type == TokenType::OP_STAR || t.type == TokenType::OP_SLASH ||
               isComparisonOp(t) || t.type == TokenType::OP_ASSIGN;
    }

    bool isValidUnaryOp(TokenType type)
    {
        return type == TokenType::OP_MINUS || type == TokenType::OP_PLUS ||
               type == TokenType::OP_NOT || type == TokenType::OP_BITNOT ||
               type == TokenType::OP_INC || type == TokenType::OP_DEC;
    }

    bool isValidBinaryOp(TokenType type)
    {
        return type == TokenType::OP_PLUS || type == TokenType::OP_MINUS ||
               type == TokenType::OP_STAR || type == TokenType::OP_SLASH ||
               type == TokenType::OP_PERCENT || type == TokenType::OP_ASSIGN ||
               type == TokenType::OP_EQ || type == TokenType::OP_NE ||
               type == TokenType::OP_LT || type == TokenType::OP_GT ||
               type == TokenType::OP_LE || type == TokenType::OP_GE ||
               type == TokenType::OP_AND || type == TokenType::OP_OR ||
               type == TokenType::OP_BITAND || type == TokenType::OP_BITOR ||
               type == TokenType::OP_BITXOR;
    }
    // Helper: Get expression type and detect type errors
    string getExpressionType(const Token &t)
    {
        if (t.type == TokenType::TOK_NUMBER)
            return "int"; // Could be float if has .
        if (t.type == TokenType::TOK_STRING)
            return "string";
        if (t.type == TokenType::TOK_CHAR)
            return "char";
        if (t.type == TokenType::TOK_IDENTIFIER)
            return sym.getType(t.value);
        return "UNKNOWN";
    }

    string parseExpressionWithType()
    {
        string type = parsePrimaryWithType();
        while (isOp(curr()))
        {
            Token op = curr();
            advance();
            string rhsType = parsePrimaryWithType();

            // Check type compatibility
            if (type != "UNKNOWN" && rhsType != "UNKNOWN")
            {
                string resultType = TypeSystem::getOperationResultType(type, rhsType, op.value);

                if (resultType == "INVALID")
                {
                    string errMsg = "Line " + to_string(op.line) + ":" + to_string(op.column) +
                                    " - Type error: cannot apply '" + op.value + "' to '" + type +
                                    "' and '" + rhsType + "'";
                    string sug = "SUGGESTION: Ensure both operands are compatible types";
                    errors.push_back({errMsg, sug});
                }
                type = (resultType == "INVALID" || resultType == "UNKNOWN") ? type : resultType;
            }
        }
        return type;
    }

    void parseDeclOrFunc()
    {
        // start type
        string typeName = curr().value;
        advance();

        // SPECIAL: struct <Tag> as a type name or a definition
        if (typeName == "struct")
        {
            if (curr().type != TokenType::TOK_IDENTIFIER)
            {
                Token bad = curr();
                string err = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Expected struct name";
                errors.push_back({err, "SUGGESTION: struct <name> { ... } ; or struct <name> var;"});
                advance();
                return;
            }
            string tag = curr().value;
            Token tagTok = curr();
            advance();

            // struct <Tag> { ... }  → hand to parseStruct()
            if (curr().type == TokenType::LBRACE)
            {
                // step back one token to let parseStruct() consume as if KW_STRUCT was just read
                index -= 2; // back to 'struct'
                return parseStruct();
            }

            // Otherwise it's a type name: "struct Tag"
            typeName = "struct " + tag;
        }

        // function vs variable follows the *real* identifier
        while (curr().type == TokenType::OP_STAR)
        {
            typeName += "*";
            advance();
        }

        if (curr().type != TokenType::TOK_IDENTIFIER)
        {
            Token bad = curr();
            string err = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Expected identifier";
            errors.push_back({err, suggestionEngine.getSuggestion(err)});
            advance();
            return;
        }

        Token nameTok = curr();
        string ident = nameTok.value;
        advance();

        if (curr().type == TokenType::LPAREN)
            parseFunction(typeName, ident, nameTok);
        else
            parseVarDecl(typeName, ident, nameTok);
    }

    void parseVarDecl(const string &type, const string &ident, const Token &nameTok)
    {
        string declaredType = type; // Already includes pointers from parseStatement

        // Handle leading pointer tokens before variable name (int *p)
        while (curr().type == TokenType::OP_STAR)
        {
            declaredType += "*";
            advance();
        }

        bool isArray = false;
        string arraySize = "";

        // Check for array declarator: int arr[10]
        if (curr().type == TokenType::LBRACKET)
        {
            isArray = true;
            advance();

            if (curr().type == TokenType::TOK_NUMBER)
            {
                arraySize = curr().value;
                advance();
            }

            expect(TokenType::RBRACKET, "]");
            declaredType += "[]";
        }

        if (!sym.declare(ident, declaredType, nameTok.line, nameTok.column))
        {
            string errMsg = "Line " + to_string(nameTok.line) + ":" + to_string(nameTok.column) +
                            " - Redeclaration of '" + ident + "'";
            string sug = suggestionEngine.getSuggestion(errMsg);
            errors.push_back({errMsg, sug});
        }

        if (curr().type == TokenType::OP_ASSIGN)
        {
            Token assignTok = curr();
            advance();

            if (isArray && curr().type == TokenType::LBRACE)
            {
                advance();
                int elementCount = 0;
                if (curr().type != TokenType::RBRACE)
                {
                    while (true)
                    {
                        parseExpression();
                        elementCount++;
                        if (curr().type == TokenType::COMMA)
                            advance();
                        else
                            break;
                    }
                }
                expect(TokenType::RBRACE, "}");
                if (!arraySize.empty() && stoi(arraySize) < elementCount)
                {
                    string errMsg = "Line " + to_string(assignTok.line) + ":" + to_string(assignTok.column) +
                                    " - Array size mismatch: declared " + arraySize +
                                    " but initialized with " + to_string(elementCount) + " elements";
                    string sug = "SUGGESTION: Increase array size or reduce initializer elements";
                    errors.push_back({errMsg, sug});
                }
            }
            else
            {
                string rhsType = parseExpressionWithFullType();

                if (rhsType != "UNKNOWN" && !TypeSystem::areTypesCompatible(declaredType, rhsType))
                {
                    string errMsg = "Warning: Line " + to_string(assignTok.line) + ":" + to_string(assignTok.column) +
                                    " - Type mismatch: assigning '" + rhsType + "' to '" + declaredType + "'";
                    string sug = "SUGGESTION: Types must be compatible";
                    errors.push_back({errMsg, sug});
                }
            }
        }

        // Handle multiple declarations: int *p, **q, arr[5];
        // REPLACE the "while (curr().type == TokenType::COMMA) { ... }" in parseVarDecl with:
        while (curr().type == TokenType::COMMA)
        {
            advance();

            // collect any pointer stars BEFORE the next identifier
            string nextDeclaredType = type; // base type, e.g., "int"
            while (curr().type == TokenType::OP_STAR)
            {
                nextDeclaredType += "*";
                advance();
            }

            if (curr().type != TokenType::TOK_IDENTIFIER)
            {
                Token bad = curr();
                string err = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Expected identifier";
                errors.push_back({err, "SUGGESTION: Provide a variable name after ',' (example: int *a, **b;)"});
                break;
            }

            Token t = curr(); // the identifier
            advance();

            // optional array declarator: int *a, **b[10]
            if (curr().type == TokenType::LBRACKET)
            {
                advance();
                if (curr().type == TokenType::TOK_NUMBER)
                    advance();
                expect(TokenType::RBRACKET, "]");
                nextDeclaredType += "[]";
            }

            if (!sym.declare(t.value, nextDeclaredType, t.line, t.column))
            {
                string err = "Line " + to_string(t.line) + ":" + to_string(t.column) +
                             " - Redeclaration of '" + t.value + "'";
                errors.push_back({err, suggestionEngine.getSuggestion(err)});
            }

            if (curr().type == TokenType::OP_ASSIGN)
            {
                advance();
                string rhsType = parseExpressionWithFullType();

                if (rhsType != "UNKNOWN" && !TypeSystem::areTypesCompatible(nextDeclaredType, rhsType))
                {
                    errors.push_back({"Warning: Line " + to_string(t.line) + ":" + to_string(t.column) + " - Type mismatch",
                                      "SUGGESTION: Types must match"});
                }
            }
        }

        expect(TokenType::SEMICOLON, ";");
    }

    void parseFunction(const std::string & /*type*/, const std::string &ident, const Token &nameTok)
    {
        // Reject nested functions
        if (scopeDepth > 0)
        {
            string errMsg = "Line " + to_string(nameTok.line) + ":" + to_string(nameTok.column) +
                            " - Invalid: nested function declaration '" + ident + "'";
            string sug = "SUGGESTION: C does not support nested functions. Move to file scope";
            errors.push_back({errMsg, sug});

            while (curr().type != TokenType::LBRACE && curr().type != TokenType::SEMICOLON &&
                   curr().type != TokenType::TOK_EOF)
            {
                advance();
            }
            if (curr().type == TokenType::SEMICOLON)
            {
                advance();
            }
            else if (curr().type == TokenType::LBRACE)
            {
                int braceCount = 1;
                advance();
                while (braceCount > 0 && curr().type != TokenType::TOK_EOF)
                {
                    if (curr().type == TokenType::LBRACE)
                        braceCount++;
                    if (curr().type == TokenType::RBRACE)
                        braceCount--;
                    advance();
                }
            }
            return;
        }

        // Detect function redeclaration
        if (sym.exists(ident) && sym.getType(ident) == "function")
        {
            string errMsg = "Line " + to_string(nameTok.line) + ":" + to_string(nameTok.column) +
                            " - Redeclaration of function '" + ident + "'";
            string sug = "SUGGESTION: Function '" + ident + "' is already declared";
            errors.push_back({errMsg, sug});
        }

        sym.declare(ident, "function");
        advance();
        sym.pushScope();
        scopeDepth++;

        // Parse parameters
        if (curr().type != TokenType::RPAREN)
        {
            while (true)
            {
                if (!isTypeToken(curr()))
                {
                    Token bad = curr();
                    string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                                    " - Expected type";
                    string sug = suggestionEngine.getSuggestion(errMsg);
                    errors.push_back({errMsg, sug});
                    break;
                }

                string pType = curr().value;
                advance();

                while (curr().type == TokenType::OP_STAR)
                {
                    pType += "*";
                    advance();
                }

                if (curr().type != TokenType::TOK_IDENTIFIER)
                {
                    Token bad = curr();
                    string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                                    " - Expected parameter name";
                    string sug = suggestionEngine.getSuggestion(errMsg);
                    errors.push_back({errMsg, sug});
                    break;
                }

                sym.declare(curr().value, pType);
                advance();

                if (curr().type == TokenType::COMMA)
                    advance();
                else
                    break;
            }
        }

        expect(TokenType::RPAREN, ")");

        if (curr().type == TokenType::SEMICOLON)
        {
            advance();
            scopeDepth--;
            sym.popScope();
            return;
        }

        expect(TokenType::LBRACE, "{");
        parseBlock();
        scopeDepth--;
        sym.popScope();
    }

    void parseBlock()
    {
        int maxIter = 10000;
        int iter = 0;
        while (curr().type != TokenType::RBRACE && curr().type != TokenType::TOK_EOF && iter++ < maxIter)
        {
            lastIndex = index;
            if (isTypeToken(curr()))
                parseDeclOrFunc();
            else
                parseStatement();
            forceAdvance();
        }
        if (iter >= maxIter)
        {
            Token bad = curr();
            string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Parser stuck, aborting block";
            errors.push_back({errMsg, ""});
        }
        expect(TokenType::RBRACE, "}");
    }
    // NEW: factor typedef into a reusable routine
    void parseTypedef()
    {
        advance(); // consumed KW_TYPEDEF

        if (!isTypeToken(curr()))
        {
            Token bad = curr();
            string err = "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                         " - Expected type after typedef";
            errors.push_back({err, "SUGGESTION: typedef <existing_type> <new_name>;"});
            return;
        }

        string baseType = curr().value;
        advance();

        // typedef struct Point ...
        if (baseType == "struct")
        {
            if (curr().type == TokenType::TOK_IDENTIFIER)
            {
                baseType += " " + curr().value;
                advance();
            }
        }

        while (curr().type == TokenType::OP_STAR)
        {
            baseType += "*";
            advance();
        }

        if (curr().type != TokenType::TOK_IDENTIFIER)
        {
            Token bad = curr();
            string err = "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                         " - Expected new type name in typedef";
            errors.push_back({err, "SUGGESTION: Provide name for the new type"});
            return;
        }

        string newTypeName = curr().value;
        advance();
        expect(TokenType::SEMICOLON, ";");

        // Mark clearly as a typedef so we can recognize it as a type later
        sym.declare(newTypeName, "typedef:" + baseType);
    }

    // NEW: factor struct into a reusable routine
    void parseStruct()
    {
        advance(); // consumed KW_STRUCT

        if (curr().type != TokenType::TOK_IDENTIFIER)
        {
            Token bad = curr();
            string err = "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                         " - Expected struct name";
            errors.push_back({err, "SUGGESTION: struct <name> { ... } or struct <name> <variable>;"});
            return;
        }

        string structName = curr().value;
        advance();

        // struct <name> { ... } [opt var] ;
        if (curr().type == TokenType::LBRACE)
        {
            advance();
            while (curr().type != TokenType::RBRACE && curr().type != TokenType::TOK_EOF)
            {
                if (isTypeToken(curr()))
                {
                    string memberType = curr().value;
                    advance();
                    while (curr().type == TokenType::OP_STAR)
                    {
                        memberType += "*";
                        advance();
                    }
                    if (curr().type == TokenType::TOK_IDENTIFIER)
                    {
                        advance();
                        expect(TokenType::SEMICOLON, ";");
                    }
                    else
                    {
                        Token bad = curr();
                        string err = "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                                     " - Expected member name in struct";
                        errors.push_back({err, ""});
                        advance();
                    }
                }
                else
                {
                    Token bad = curr();
                    string err = "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                                 " - Expected type in struct member";
                    errors.push_back({err, ""});
                    advance();
                }
            }
            expect(TokenType::RBRACE, "}");

            // Either just a definition ...
            if (curr().type == TokenType::SEMICOLON)
            {
                advance();
            }
            // ... or an inline variable: struct N { ... } v;
            else if (curr().type == TokenType::TOK_IDENTIFIER || curr().type == TokenType::OP_STAR)
            {
                // allow pointer before name: struct N {..} *p;
                string varType = "struct " + structName;
                while (curr().type == TokenType::OP_STAR)
                {
                    varType += "*";
                    advance();
                }
                Token nameTok = curr();
                if (nameTok.type == TokenType::TOK_IDENTIFIER)
                {
                    advance();
                    parseVarDecl(varType, nameTok.value, nameTok);
                    return;
                }
                else
                {
                    string err = "Line " + to_string(curr().line) + ":" + to_string(curr().column) +
                                 " - Expected variable name after struct definition";
                    errors.push_back({err, "SUGGESTION: struct " + structName + " { ... } var;"});
                }
            }
            else
            {
                string err = "Line " + to_string(curr().line) + ":" + to_string(curr().column) +
                             " - Expected ';' or variable after struct definition";
                errors.push_back({err, "SUGGESTION: struct " + structName + " { ... }; or struct " + structName + " var;"});
            }

            sym.declare(structName, "struct_type");
            return;
        }

        // struct <name> <declarator>...;
        if (curr().type == TokenType::OP_STAR || curr().type == TokenType::TOK_IDENTIFIER)
        {
            // allow pointer stars before identifier: struct Point *p;
            string varType = "struct " + structName;
            while (curr().type == TokenType::OP_STAR)
            {
                varType += "*";
                advance();
            }
            if (curr().type == TokenType::TOK_IDENTIFIER)
            {
                Token nameTok = curr();
                advance();
                parseVarDecl(varType, nameTok.value, nameTok);
                return;
            }
        }

        // forward decl: struct Point;
        if (curr().type == TokenType::SEMICOLON)
        {
            advance();
            sym.declare(structName, "struct_forward");
            return;
        }

        Token bad = curr();
        string err = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Invalid struct syntax";
        errors.push_back({err, "SUGGESTION: struct <name> { ... }; or struct <name> <var>;"});
    }

    // Handles "if (...)" statement logic
    void parseIf()
    {
        Token ifTok = curr();
        advance(); // KW_IF
        expect(TokenType::LPAREN, "(");
        parseExpression();
        expect(TokenType::RPAREN, ")");
        if (curr().type == TokenType::SEMICOLON)
        {
            string errMsg = "Line " + to_string(ifTok.line) + ":" + to_string(ifTok.column) +
                            " - Missing statement after 'if' condition";
            string sug = "SUGGESTION: Add a statement or block after if";
            errors.push_back({errMsg, sug});
            advance();
        }
        else if (curr().type == TokenType::RBRACE || curr().type == TokenType::TOK_EOF)
        {
            string errMsg = "Line " + to_string(ifTok.line) + ":" + to_string(ifTok.column) +
                            " - Missing statement after 'if' condition";
            string sug = "SUGGESTION: Add a statement or block after if";
            errors.push_back({errMsg, sug});
        }
        else
        {
            parseStatement();
        }
        if (curr().type == TokenType::KW_ELSE)
        {
            advance();
            parseStatement();
        }
    }

    // Handles "while (...)" and "for (...)" statement logic
    void parseLoop()
    {
        Token loopTok = curr();
        advance(); // KW_WHILE or KW_FOR
        expect(TokenType::LPAREN, "(");
        parseExpression();
        expect(TokenType::RPAREN, ")");
        if (curr().type == TokenType::SEMICOLON)
        {
            string errMsg = "Line " + to_string(loopTok.line) + ":" + to_string(loopTok.column) +
                            " - Missing statement after loop condition";
            string sug = "SUGGESTION: Add a statement or block";
            errors.push_back({errMsg, sug});
            advance();
        }
        else
        {
            parseStatement();
        }
    }

    void parseStatement()
    {
        Token t = curr();

        // Handle preprocessor
        if (t.type == TokenType::PREPROCESSOR)
        {
            advance();
            return;
        }

        // Handle block
        if (t.type == TokenType::LBRACE)
        {
            advance();
            sym.pushScope();
            parseBlock();
            sym.popScope();
            return;
        }

        // Handle control flow
        if (t.type == TokenType::KW_IF)
        {
            parseIf();
            return;
        }

        if (t.type == TokenType::KW_WHILE || t.type == TokenType::KW_FOR)
        {
            parseLoop();
            return;
        }

        if (t.type == TokenType::KW_RETURN)
        {
            advance();
            if (curr().type != TokenType::SEMICOLON)
                parseExpressionWithFullType();
            expect(TokenType::SEMICOLON, ";");
            return;
        }

        if (t.type == TokenType::SEMICOLON)
        {
            advance();
            return;
        }

        // ============================================================================
        // Handle typedef: typedef int MyInt; typedef struct Point MyPoint;
        // ============================================================================
        // Inside parseStatement()
        if (t.type == TokenType::KW_TYPEDEF)
        {
            parseTypedef();
            return;
        }
        // ============================================================================
        // Handle const keyword at statement start: const int x = 5;
        // ============================================================================
        if (t.type == TokenType::KW_CONST)
        {
            advance();

            if (!isTypeToken(curr()))
            {
                Token bad = curr();
                string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                                " - Expected type after const";
                string sug = "SUGGESTION: const <type> <variable_name>;";
                errors.push_back({errMsg, sug});
                return;
            }

            string type = "const " + curr().value;
            advance();

            // Handle pointers: const int *p;
            while (curr().type == TokenType::OP_STAR)
            {
                type += "*";
                advance();
            }

            if (curr().type != TokenType::TOK_IDENTIFIER)
            {
                Token bad = curr();
                string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                                " - Expected identifier after const type";
                string sug = "SUGGESTION: Provide variable name";
                errors.push_back({errMsg, sug});
                return;
            }

            if (t.type == TokenType::OP_PLUS)
            {
                // Unary + at statement level should only be allowed if numeric
                Token opTok = curr();
                advance();

                if (curr().type == TokenType::TOK_IDENTIFIER)
                {
                    string id = curr().value;
                    string idType = sym.getType(id);

                    if (idType == "function" ||
                        idType == "" ||
                        idType == "string" ||
                        idType == "char" ||
                        idType == "void")
                    {
                        errors.push_back({"Line " + to_string(opTok.line) + ":" + to_string(opTok.column) +
                                              " - Invalid use of unary '+' at statement level",
                                          "SUGGESTION: Statements cannot start with unary '+' unless it's a numeric expression"});
                        return;
                    }
                }

                // If numeric, parse normally
                parsePrimaryWithType();
                expect(TokenType::SEMICOLON, ";");
                return;
            }

            Token nameTok = curr();
            string ident = nameTok.value;
            advance();

            parseVarDecl(type, ident, nameTok);
            return;
        }

        // ============================================================================
        // Handle struct declarations
        // ============================================================================
        // Inside parseStatement() in struct section
        if (t.type == TokenType::KW_STRUCT)
        {
            parseStruct();
            return;
        }
        // ============================================================================
        // Handle regular type declarations (int, float, etc.)
        // ============================================================================
        if (isTypeToken(t))
        {
            string type = t.value;
            advance();

            // Skip const if it's after type (shouldn't happen with our grammar)
            if (curr().type == TokenType::KW_CONST)
            {
                type += " const";
                advance();
            }

            // Handle pointers: int *p;
            while (curr().type == TokenType::OP_STAR)
            {
                type += "*";
                advance();
            }

            if (curr().type != TokenType::TOK_IDENTIFIER)
            {
                Token bad = curr();
                string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                                " - Expected identifier after type '" + type + "'";
                string sug = "SUGGESTION: Provide variable or function name";
                errors.push_back({errMsg, sug});
                return;
            }

            Token nameTok = curr();
            string ident = nameTok.value;
            advance();

            // Function or variable?
            if (curr().type == TokenType::LPAREN)
            {
                parseFunction(type, ident, nameTok);
            }
            else
            {
                parseVarDecl(type, ident, nameTok);
            }
            return;
        }
        // ===============================================
        // NEW: Reject invalid statement starters
        // ===============================================
        Token st = curr();

        bool isValidStart =
            st.type == TokenType::TOK_IDENTIFIER ||
            st.type == TokenType::TOK_NUMBER ||
            st.type == TokenType::TOK_STRING ||
            st.type == TokenType::TOK_CHAR ||
            st.type == TokenType::LPAREN ||
            st.type == TokenType::OP_MINUS ||
            st.type == TokenType::OP_PLUS ||
            st.type == TokenType::OP_NOT ||
            st.type == TokenType::OP_BITNOT ||
            st.type == TokenType::OP_INC ||
            st.type == TokenType::OP_DEC;

        // Extra rule: *, /, % cannot start a statement in C
        if (st.type == TokenType::OP_STAR ||
            st.type == TokenType::OP_SLASH ||
            st.type == TokenType::OP_PERCENT)
        {
            string errMsg =
                "Line " + to_string(st.line) + ":" + to_string(st.column) +
                " - Invalid statement start: '" + st.value + "'";
            errors.push_back({errMsg, "SUGGESTION: Statements cannot begin with '" + st.value + "'"});
            advance();
            return;
        }

        // ADD THIS: forbid starting with unary operators if the next token is a FUNCTION name
        if (st.type == TokenType::OP_MINUS ||
            st.type == TokenType::OP_PLUS ||
            st.type == TokenType::OP_NOT ||
            st.type == TokenType::OP_BITNOT)
        {
            Token nxt = peek();

            if (nxt.type == TokenType::TOK_IDENTIFIER)
            {
                string nxtType = sym.getType(nxt.value);

                if (nxtType == "function")
                {
                    errors.push_back({"Line " + to_string(st.line) + ":" + to_string(st.column) +
                                          " - Invalid unary operator '" + st.value + "' applied to function '" + nxt.value + "'",
                                      "SUGGESTION: Unary operators only apply to numeric expressions"});
                    advance();
                    return;
                }
            }
        }

        // Handle other statement types...
        parseExprOrAssignment();
        expect(TokenType::SEMICOLON, ";");
    }

    void parseExprOrAssignment()
    {
        if (curr().type == TokenType::TOK_IDENTIFIER && peek().type == TokenType::OP_ASSIGN)
        {
            Token id = curr();
            string varType = sym.getType(id.value);

            // CHECK: LHS variable must be declared
            if (!sym.exists(id.value))
            {
                string errMsg = "Line " + to_string(id.line) + ":" + to_string(id.column) +
                                " - Undeclared variable '" + id.value + "'";
                string sug = suggestionEngine.getSuggestion(errMsg);
                errors.push_back({errMsg, sug});
            }

            advance();
            Token assignTok = curr();
            advance();

            // Parse RHS - this will check y and all identifiers in expression
            string rhsType = parseExpressionWithFullType();

            // TYPE CHECK on assignment to existing variable
            if (varType != "UNKNOWN" && rhsType != "UNKNOWN" &&
                !TypeSystem::areTypesCompatible(varType, rhsType))
            {
                string errMsg = "Warning: Line " + to_string(assignTok.line) + ":" + to_string(assignTok.column) +
                                " - Type error: assigning '" + rhsType + "' to '" + varType + "'";
                string sug = "SUGGESTION: Types must match. " + varType + " expected, " + rhsType + " provided";
                errors.push_back({errMsg, sug});
            }
        }
        else
        {
            parseExpression();
        }
    }

    string parseExpression()
    {
        string type = parsePrimaryWithType();

        while (isOp(curr()))
        {
            // ===============================
            // INVALID OPERATOR SEQUENCE CHECK
            // ===============================
            if ((curr().type == TokenType::OP_ASSIGN || curr().type == TokenType::OP_EQ) &&
                (peek().type == TokenType::OP_EQ))
            {
                // This is the pattern: ! = =  (user typed !==)
                Token bad = curr();
                string err = "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                             " - Invalid operator sequence: '!==' is not valid in C";
                errors.push_back({err, "SUGGESTION: Use '!=' for inequality"});

                // Skip the extra =
                advance();
            }

            Token op = curr();
            advance();

            // ERROR: Missing RHS operand
            if (curr().type == TokenType::SEMICOLON ||
                curr().type == TokenType::RPAREN ||
                curr().type == TokenType::RBRACE ||
                curr().type == TokenType::COMMA)
            {
                string errMsg = "Line " + to_string(op.line) + ":" + to_string(op.column) +
                                " - Incomplete expression: missing operand after '" + op.value + "'";
                string sug = "SUGGESTION: Complete the expression. Example: x + y";
                errors.push_back({errMsg, sug});
                return type;
            }

            string rhsType = parsePrimaryWithType();

            // TYPE CHECKING
            if (type != "UNKNOWN" && rhsType != "UNKNOWN")
            {
                string resultType = TypeSystem::getOperationResultType(type, rhsType, op.value);

                if (resultType == "INVALID")
                {
                    string errMsg = "Line " + to_string(op.line) + ":" + to_string(op.column) +
                                    " - Type error: cannot apply '" + op.value + "' to '" + type +
                                    "' and '" + rhsType + "'";
                    string sug = "SUGGESTION: Ensure both operands are compatible types";
                    errors.push_back({errMsg, sug});
                }
                type = (resultType == "INVALID" || resultType == "UNKNOWN") ? type : resultType;
            }
        }
        return type;
    }

    void parsePrimary()
    {
        Token t = curr();

        if (t.type == TokenType::TOK_IDENTIFIER)
        {
            // CHECK 1: Is this identifier declared or a standard library function?
            if (!sym.exists(t.value))
            {
                string errMsg = "Line " + to_string(t.line) + ":" + to_string(t.column) +
                                " - Undeclared identifier '" + t.value + "'";
                string sug = suggestionEngine.getSuggestion(errMsg);
                errors.push_back({errMsg, sug});
            }

            advance();

            // CHECK 2: If it's a function call, check all arguments
            if (curr().type == TokenType::LPAREN)
            {
                advance(); // skip (

                if (curr().type != TokenType::RPAREN)
                {
                    while (true)
                    {
                        // RECURSIVELY check arguments - will find undeclared vars in args
                        parseExpression();

                        if (curr().type == TokenType::COMMA)
                        {
                            advance();
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                expect(TokenType::RPAREN, ")");
            }
        }
        // CHECK 3: Number literal - report if it's a bare statement like "2;"
        else if (t.type == TokenType::TOK_NUMBER)
        {
            // Numbers are OK in expressions, but bare "2;" is suspicious
            // Let the caller decide if this is an error (bare statement)
            advance();
        }
        // CHECK 4: String literal
        else if (t.type == TokenType::TOK_STRING)
        {
            advance();
        }
        // CHECK 5: Character literal
        else if (t.type == TokenType::TOK_CHAR)
        {
            advance();
        }
        // CHECK 6: Parenthesized expression
        else if (t.type == TokenType::LPAREN)
        {
            advance();
            parseExpression(); // RECURSIVE - will check nested identifiers
            expect(TokenType::RPAREN, ")");
        }
        // CHECK 7: Unary operators
        else if (t.type == TokenType::OP_MINUS || t.type == TokenType::OP_PLUS ||
                 t.type == TokenType::OP_NOT || t.type == TokenType::OP_BITNOT)
        {
            advance();
            parsePrimary(); // Check the operand
        }
        // CHECK 8: Increment/Decrement
        else if (t.type == TokenType::OP_INC || t.type == TokenType::OP_DEC)
        {
            advance();
            parsePrimary();
        }
        // ERROR: Unexpected token
        else if (t.type != TokenType::SEMICOLON &&
                 t.type != TokenType::RPAREN &&
                 t.type != TokenType::RBRACE &&
                 t.type != TokenType::COMMA &&
                 t.type != TokenType::TOK_EOF)
        {
            string errMsg = "Line " + to_string(t.line) + ":" + to_string(t.column) +
                            " - Unexpected token '" + t.value + "'";
            string sug = suggestionEngine.getSuggestion(errMsg);
            errors.push_back({errMsg, sug});
            advance();
        }
    }

    string parsePrimaryWithType()
    {
        Token t = curr();

        // ===============================
        // IDENTIFIER (variable or function)
        // ===============================
        if (t.type == TokenType::TOK_IDENTIFIER)
        {
            string type = sym.getType(t.value);
            Token idTok = t;

            if (!sym.exists(t.value))
            {
                string errMsg = "Line " + to_string(t.line) + ":" + to_string(t.column) +
                                " - Undeclared identifier '" + t.value + "'";
                errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
            }

            advance(); // consume identifier

            // ------------------------------------
            // FUNCTION CALL: identifier '(' ... ')'
            // ------------------------------------
            if (curr().type == TokenType::LPAREN)
            {
                advance(); // consume '('

                vector<string> argTypes;

                // Parse arguments if any
                if (curr().type != TokenType::RPAREN)
                {
                    while (true)
                    {
                        argTypes.push_back(parseExpressionWithFullType());

                        if (curr().type == TokenType::COMMA)
                            advance();
                        else
                            break;
                    }
                }

                expect(TokenType::RPAREN, ")");

                // ------------------------------------
                // ARGUMENT COUNT CHECK (NEW)
                // ------------------------------------
                string signature = stdLib.getFunctionSignature(idTok.value);

                if (!signature.empty())
                {
                    size_t pos1 = signature.find("(");
                    size_t pos2 = signature.find(")");

                    if (pos1 != string::npos && pos2 != string::npos)
                    {
                        string params = signature.substr(pos1 + 1, pos2 - pos1 - 1);

                        // Validate fixed-arity functions
                        if (params != "..." && params != "void" && !params.empty())
                        {
                            int expected = count(params.begin(), params.end(), ',') + 1;
                            int provided = argTypes.size();

                            if (expected != provided)
                            {
                                errors.push_back({"Line " + to_string(idTok.line) + ":" + to_string(idTok.column) +
                                                      " - Function call argument count mismatch for '" + idTok.value + "'",
                                                  "SUGGESTION: Expected " + to_string(expected) +
                                                      " argument(s), but got " + to_string(provided)});
                            }
                        }
                    }
                }

                // We don't track true return types yet → treat as int
                return "int";
            }

            // ------------------------------------
            // POSTFIX INC/DEC
            // ------------------------------------
            if (curr().type == TokenType::OP_INC || curr().type == TokenType::OP_DEC)
            {
                Token opTok = curr();

                // Validate lvalue
                if (type == "function" || !isModifiableLvalue(idTok, type))
                {
                    string errMsg = "Line " + to_string(opTok.line) + ":" + to_string(opTok.column) +
                                    " - Invalid: cannot apply '" + opTok.value +
                                    "' to '" + idTok.value + "' (not a modifiable lvalue)";
                    errors.push_back({errMsg, "SUGGESTION: ++/-- require a modifiable variable"});
                }

                advance();
            }

            return type;
        }

        // ===============================
        // NUMBER LITERAL
        // ===============================
        else if (t.type == TokenType::TOK_NUMBER)
        {
            advance();
            if (t.value.find('.') != string::npos)
                return "float";
            return "int";
        }

        // ===============================
        // STRING LITERAL
        // ===============================
        else if (t.type == TokenType::TOK_STRING)
        {
            advance();
            return "string";
        }

        // ===============================
        // CHAR LITERAL
        // ===============================
        else if (t.type == TokenType::TOK_CHAR)
        {
            advance();
            return "char";
        }

        // ===============================
        // PARENTHESIZED EXPRESSION
        // ===============================
        else if (t.type == TokenType::LPAREN)
        {
            advance();
            string type = parseExpressionWithType();
            expect(TokenType::RPAREN, ")");
            return type;
        }

        // ===============================
        // ADDRESS-OF (&)
        // ===============================
        else if (t.type == TokenType::OP_BITAND)
        {
            Token opTok = curr();
            advance();

            if (curr().type != TokenType::TOK_IDENTIFIER)
            {
                string errMsg = "Line " + to_string(opTok.line) + ":" + to_string(opTok.column) +
                                " - Address-of operator & requires a variable";
                errors.push_back({errMsg, "SUGGESTION: Use & with a variable. Example: &x"});
                return "UNKNOWN";
            }

            Token idTok = curr();
            string varType = sym.getType(idTok.value);

            if (!sym.exists(idTok.value))
            {
                string errMsg = "Line " + to_string(idTok.line) + ":" + to_string(idTok.column) +
                                " - Undeclared variable '" + idTok.value + "'";
                errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
            }

            advance();
            return varType + "*";
        }

        // ===============================
        // PREFIX UNARY OPS
        // ===============================
        else if (t.type == TokenType::OP_PLUS ||
                 t.type == TokenType::OP_MINUS ||
                 t.type == TokenType::OP_NOT ||
                 t.type == TokenType::OP_BITNOT)
        {
            Token opTok = curr();
            advance();

            // Detect nonsense like +*- or -/ or !~+
            if (curr().type == TokenType::OP_PLUS ||
                curr().type == TokenType::OP_MINUS ||
                curr().type == TokenType::OP_STAR ||
                curr().type == TokenType::OP_SLASH ||
                curr().type == TokenType::OP_PERCENT ||
                curr().type == TokenType::OP_BITAND ||
                curr().type == TokenType::OP_BITOR ||
                curr().type == TokenType::OP_BITXOR)
            {
                string err =
                    "Line " + to_string(opTok.line) + ":" + to_string(opTok.column) +
                    " - Invalid operator sequence starting at '" + opTok.value + "'";
                errors.push_back({err,
                                  "SUGGESTION: Unary operators must be followed by a valid expression"});

                advance(); // skip garbage
                return "UNKNOWN";
            }

            // ERROR: operand missing
            if (curr().type == TokenType::SEMICOLON ||
                curr().type == TokenType::RPAREN ||
                curr().type == TokenType::RBRACE ||
                curr().type == TokenType::COMMA)
            {
                errors.push_back({"Line " + to_string(opTok.line) + ":" + to_string(opTok.column) +
                                      " - Incomplete unary expression after '" + opTok.value + "'",
                                  "SUGGESTION: Provide a valid operand"});
                return "UNKNOWN";
            }

            // ===================================================
            // NEW: block unary +, -, !, ~ on FUNCTION identifiers
            // ===================================================
            if (curr().type == TokenType::TOK_IDENTIFIER)
            {
                string id = curr().value;
                string idType = sym.getType(id);

                if (idType == "function")
                {
                    errors.push_back({"Line " + to_string(opTok.line) + ":" + to_string(opTok.column) +
                                          " - Unary operator '" + opTok.value +
                                          "' cannot be applied to function '" + id + "'",
                                      "SUGGESTION: Functions must be called normally, e.g. " + id + "();"});

                    advance();
                    return "UNKNOWN";
                }
            }

            return parsePrimaryWithType();
        }

        // ===============================
        // BAD BINARY OP AT START
        // ===============================
        else if (t.type == TokenType::OP_EQ || t.type == TokenType::OP_NE ||
                 t.type == TokenType::OP_AND || t.type == TokenType::OP_OR)
        {
            Token opTok = curr();
            string errMsg = "Line " + to_string(opTok.line) + ":" + to_string(opTok.column) +
                            " - Invalid: operator '" + opTok.value + "' cannot start an expression";
            errors.push_back({errMsg, "SUGGESTION: Add a left operand"});
            advance();
            return "UNKNOWN";
        }
        // ==========================================================
        // STEP 5: Detect two consecutive primaries without an operator
        // ==========================================================
        if (curr().type == TokenType::TOK_IDENTIFIER ||
            curr().type == TokenType::TOK_NUMBER ||
            curr().type == TokenType::TOK_STRING ||
            curr().type == TokenType::TOK_CHAR)
        {
            Token bad = curr();
            string err =
                "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                " - Unexpected token '" + bad.value + "' after expression";
            errors.push_back({err,
                              "SUGGESTION: Missing operator between expressions"});

            advance(); // Prevent infinite loop
        }

        // ===============================
        // FALLBACK
        // ===============================
        return "UNKNOWN";
    }

    bool isModifiableLvalue(const Token &token, const string &symType)
    {
        // A modifiable lvalue for ++/-- must be:
        // 1. An identifier (not a literal)
        // 2. A VARIABLE type, not a function
        // 3. NOT a standard library function
        // 4. NOT a user-defined function (symType would be "function")
        // 5. A complete object type (not incomplete/function type)

        if (token.type != TokenType::TOK_IDENTIFIER)
        {
            return false; // Literals are not lvalues
        }

        // REJECT: User-defined functions
        if (symType == "function")
        {
            return false; // Cannot apply ++/-- to functions
        }

        // REJECT: Standard library functions
        if (stdLib.isStdioFunction(token.value) ||
            stdLib.isStdlibFunction(token.value) ||
            stdLib.isStringFunction(token.value) ||
            stdLib.isMathFunction(token.value))
        {
            return false; // Standard library functions are not modifiable lvalues
        }

        // REJECT: Undeclared or unknown type
        if (symType == "UNKNOWN" || symType == "" || (symType == "int" && !sym.exists(token.value)))
        {
            return false; // Not a declared variable
        }

        // ACCEPT: It's a declared variable with a concrete type
        return true;
    }

    string parseExpressionWithFullType()
    {
        string lhs = parsePrimaryWithType();

        while (isValidBinaryOp(curr().type))
        {
            // ===============================
            // INVALID OPERATOR SEQUENCE CHECK
            // ===============================
            if ((curr().type == TokenType::OP_ASSIGN || curr().type == TokenType::OP_EQ) &&
                (peek().type == TokenType::OP_EQ))
            {
                // This is the pattern: ! = =  (user typed !==)
                Token bad = curr();
                string err = "Line " + to_string(bad.line) + ":" + to_string(bad.column) +
                             " - Invalid operator sequence: '!==' is not valid in C";
                errors.push_back({err, "SUGGESTION: Use '!=' for inequality"});

                // Skip the extra =
                advance();
            }

            Token op = curr();
            advance();

            string rhs = parsePrimaryWithType();

            // If any side is UNKNOWN → carry on but do not error
            if (lhs != "UNKNOWN" && rhs != "UNKNOWN")
            {
                string result = TypeSystem::getOperationResultType(lhs, rhs, op.value);

                if (result == "INVALID")
                {
                    string err =
                        "Line " + to_string(op.line) + ":" + to_string(op.column) +
                        " - Type mismatch: cannot apply operator '" + op.value +
                        "' between '" + lhs + "' and '" + rhs + "'";
                    errors.push_back({err,
                                      "SUGGESTION: Convert operands or use compatible types."});
                }
                else
                {
                    lhs = result;
                }
            }
        }

        return lhs;
    }

public:
    Parser(const vector<Token> &toks) : tokens(toks), index(0), lastIndex(0) {}

    void parseProgram()
    {
        int maxIter = 10000;
        int iter = 0;
        while (curr().type != TokenType::TOK_EOF && iter++ < maxIter)
        {
            lastIndex = index;
            if (curr().type == TokenType::PREPROCESSOR)
            {
                advance();
            }
            if (curr().type == TokenType::KW_TYPEDEF)
            {
                parseTypedef();
                forceAdvance();
                continue;
            }
            if (curr().type == TokenType::KW_STRUCT)
            {
                parseStruct();
                forceAdvance();
                continue;
            }
            else if (isTypeToken(curr()))
                parseDeclOrFunc();
            else if (curr().type == TokenType::TOK_ERROR)
                advance();
            else
            {
                Token bad = curr();
                string errMsg = "Line " + to_string(bad.line) + ":" + to_string(bad.column) + " - Unexpected at file scope";
                string sug = suggestionEngine.getSuggestion(errMsg);
                errors.push_back({errMsg, sug});
                advance();
            }
            forceAdvance();
        }
        if (iter >= maxIter)
            errors.push_back({"Parser stuck - aborting", ""});
    }

    vector<pair<string, string>> getErrorsWithSuggestions() const { return errors; }
};

// ============================================================================
// ANALYSIS ENGINE (Qt-ready public API)
// ============================================================================

class CErrorDetectorEngine
{
private:
    Lexer *lexer;
    Parser *parser;

public:
    CErrorDetectorEngine() : lexer(nullptr), parser(nullptr) {}
    ~CErrorDetectorEngine()
    {
        if (lexer)
            delete lexer;
        if (parser)
            delete parser;
    }

    AnalysisResult analyzeCode(const string &sourceCode)
    {
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

    AnalysisResult analyzeFile(const string &filename)
    {
        ifstream f(filename);
        if (!f.is_open())
        {
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
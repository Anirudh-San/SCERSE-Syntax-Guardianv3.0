/*
 * Modularized C Error Detector
 * - Supports stdio.h functions and common C syntax
 * - Detects:
 *   - Missing semicolons
 *   - Unbalanced parentheses and braces
 *   - Undeclared variables and functions
 *   - Redeclarations
 *   - Misplaced or missing return statements
 *   - Suspicious pointer declarations and assignments
 *   - Type mismatches (assignments & expressions)
 *
 * NOTE:
 *  - This is a lightweight static checker, not a full C compiler.
 *  - It uses a custom tokenizer and parser tailored to common
 *    beginner-level C syntax.
 */

#include <bits/stdc++.h>
using namespace std;

// =============================================================
// TOKENIZER
// =============================================================

enum class TokenType
{
    TOK_INT,
    TOK_FLOAT,
    TOK_DOUBLE,
    TOK_CHAR,
    TOK_LONG,
    TOK_SHORT,
    TOK_VOID,
    TOK_SIGNED,
    TOK_UNSIGNED,
    TOK_STRUCT,
    TOK_TYPEDEF,
    TOK_RETURN,
    TOK_IF,
    TOK_ELSE,
    TOK_FOR,
    TOK_WHILE,
    TOK_DO,
    TOK_BREAK,
    TOK_CONTINUE,
    TOK_SWITCH,
    TOK_CASE,
    TOK_DEFAULT,
    TOK_INCLUDE,
    TOK_DEFINE,
    TOK_SIZEOF,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_CHAR_LITERAL,
    TOK_STRING_LITERAL,

    OP_PLUS,
    OP_MINUS,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_ASSIGN,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LTE,
    OP_GTE,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_BITWISE_AND,
    OP_BITWISE_OR,
    OP_BITWISE_XOR,
    OP_BITWISE_NOT,
    OP_SHIFT_LEFT,
    OP_SHIFT_RIGHT,
    OP_INCREMENT,
    OP_DECREMENT,
    OP_PLUS_ASSIGN,
    OP_MINUS_ASSIGN,
    OP_MUL_ASSIGN,
    OP_DIV_ASSIGN,
    OP_MOD_ASSIGN,
    OP_ARROW,

    COMMA,
    SEMICOLON,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    DOT,
    HASH,
    COLON,
    QUESTION,
    COMMENT,
    END_OF_FILE,
    UNKNOWN
};

struct Token
{
    TokenType type;
    string value;
    int line;
    int column;

    Token(TokenType t = TokenType::UNKNOWN,
          string v = "", int l = 0, int c = 0)
        : type(t), value(std::move(v)), line(l), column(c) {}
};

class Tokenizer
{
    string source;
    size_t pos;
    int line;
    int column;

public:
    explicit Tokenizer(string src)
        : source(std::move(src)), pos(0), line(1), column(1) {}

    bool isAtEnd() const
    {
        return pos >= source.size();
    }

    char peekChar() const
    {
        if (isAtEnd())
            return '\0';
        return source[pos];
    }

    char peekNextChar() const
    {
        if (pos + 1 >= source.size())
            return '\0';
        return source[pos + 1];
    }

    char advanceChar()
    {
        if (isAtEnd())
            return '\0';
        char c = source[pos++];
        if (c == '\n')
        {
            line++;
            column = 1;
        }
        else
        {
            column++;
        }
        return c;
    }

    static bool isAlpha(char c)
    {
        return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
    }

    static bool isAlnum(char c)
    {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    }

    static bool isDigit(char c)
    {
        return std::isdigit(static_cast<unsigned char>(c));
    }

    Token makeToken(TokenType t, const string &val, int l, int c) const
    {
        return Token(t, val, l, c);
    }

    Token readIdentifierOrKeyword(int startLine, int startCol)
    {
        string value;
        value.push_back(source[pos - 1]);
        while (!isAtEnd() && isAlnum(peekChar()))
            value.push_back(advanceChar());

        if (value == "int")
            return makeToken(TokenType::TOK_INT, value, startLine, startCol);
        if (value == "float")
            return makeToken(TokenType::TOK_FLOAT, value, startLine, startCol);
        if (value == "double")
            return makeToken(TokenType::TOK_DOUBLE, value, startLine, startCol);
        if (value == "char")
            return makeToken(TokenType::TOK_CHAR, value, startLine, startCol);
        if (value == "long")
            return makeToken(TokenType::TOK_LONG, value, startLine, startCol);
        if (value == "short")
            return makeToken(TokenType::TOK_SHORT, value, startLine, startCol);
        if (value == "void")
            return makeToken(TokenType::TOK_VOID, value, startLine, startCol);
        if (value == "signed")
            return makeToken(TokenType::TOK_SIGNED, value, startLine, startCol);
        if (value == "unsigned")
            return makeToken(TokenType::TOK_UNSIGNED, value, startLine, startCol);
        if (value == "struct")
            return makeToken(TokenType::TOK_STRUCT, value, startLine, startCol);
        if (value == "typedef")
            return makeToken(TokenType::TOK_TYPEDEF, value, startLine, startCol);
        if (value == "return")
            return makeToken(TokenType::TOK_RETURN, value, startLine, startCol);
        if (value == "if")
            return makeToken(TokenType::TOK_IF, value, startLine, startCol);
        if (value == "else")
            return makeToken(TokenType::TOK_ELSE, value, startLine, startCol);
        if (value == "for")
            return makeToken(TokenType::TOK_FOR, value, startLine, startCol);
        if (value == "while")
            return makeToken(TokenType::TOK_WHILE, value, startLine, startCol);
        if (value == "do")
            return makeToken(TokenType::TOK_DO, value, startLine, startCol);
        if (value == "break")
            return makeToken(TokenType::TOK_BREAK, value, startLine, startCol);
        if (value == "continue")
            return makeToken(TokenType::TOK_CONTINUE, value, startLine, startCol);
        if (value == "switch")
            return makeToken(TokenType::TOK_SWITCH, value, startLine, startCol);
        if (value == "case")
            return makeToken(TokenType::TOK_CASE, value, startLine, startCol);
        if (value == "default")
            return makeToken(TokenType::TOK_DEFAULT, value, startLine, startCol);
        if (value == "include")
            return makeToken(TokenType::TOK_INCLUDE, value, startLine, startCol);
        if (value == "define")
            return makeToken(TokenType::TOK_DEFINE, value, startLine, startCol);
        if (value == "sizeof")
            return makeToken(TokenType::TOK_SIZEOF, value, startLine, startCol);

        return makeToken(TokenType::TOK_IDENTIFIER, value, startLine, startCol);
    }

    Token readNumber(int startLine, int startCol, char first)
    {
        string value;
        bool isFloat = false;
        value.push_back(first);

        while (!isAtEnd() && isDigit(peekChar()))
            value.push_back(advanceChar());

        if (!isAtEnd() && peekChar() == '.')
        {
            isFloat = true;
            value.push_back(advanceChar());
            while (!isAtEnd() && isDigit(peekChar()))
                value.push_back(advanceChar());
        }

        if (!isAtEnd() && (peekChar() == 'e' || peekChar() == 'E'))
        {
            isFloat = true;
            value.push_back(advanceChar());
            if (!isAtEnd() && (peekChar() == '+' || peekChar() == '-'))
                value.push_back(advanceChar());
            while (!isAtEnd() && isDigit(peekChar()))
                value.push_back(advanceChar());
        }

        return makeToken(TokenType::TOK_NUMBER, value, startLine, startCol);
    }

    Token readStringLiteral(int startLine, int startCol)
    {
        string value;
        while (!isAtEnd())
        {
            char c = advanceChar();
            if (c == '"')
                break;
            if (c == '\\' && !isAtEnd())
            {
                char next = advanceChar();
                value.push_back('\\');
                value.push_back(next);
            }
            else
            {
                value.push_back(c);
            }
        }
        return makeToken(TokenType::TOK_STRING_LITERAL, value, startLine, startCol);
    }

    Token readCharLiteral(int startLine, int startCol)
    {
        string value;
        if (isAtEnd())
            return makeToken(TokenType::TOK_CHAR_LITERAL, value, startLine, startCol);

        char c = advanceChar();
        if (c == '\\' && !isAtEnd())
        {
            char next = advanceChar();
            value.push_back('\\');
            value.push_back(next);
        }
        else
        {
            value.push_back(c);
        }

        if (!isAtEnd() && peekChar() == '\'')
            advanceChar();

        return makeToken(TokenType::TOK_CHAR_LITERAL, value, startLine, startCol);
    }

    Token skipLineComment(int startLine, int startCol)
    {
        while (!isAtEnd() && peekChar() != '\n')
            advanceChar();
        return makeToken(TokenType::COMMENT, "//", startLine, startCol);
    }

    Token skipBlockComment(int startLine, int startCol)
    {
        while (!isAtEnd())
        {
            char c = advanceChar();
            if (c == '*' && !isAtEnd() && peekChar() == '/')
            {
                advanceChar();
                break;
            }
        }
        return makeToken(TokenType::COMMENT, "/* */", startLine, startCol);
    }

    Token nextToken()
    {
        while (!isAtEnd())
        {
            int startLine = line;
            int startCol = column;
            char c = advanceChar();

            if (isspace(static_cast<unsigned char>(c)))
                continue;

            if (isAlpha(c))
                return readIdentifierOrKeyword(startLine, startCol);

            if (isDigit(c))
                return readNumber(startLine, startCol, c);

            switch (c)
            {
            case '"':
                return readStringLiteral(startLine, startCol);

            case '\'':
                return readCharLiteral(startLine, startCol);

            case '+':
                if (!isAtEnd() && peekChar() == '+')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_INCREMENT, "++",
                                     startLine, startCol);
                }
                else if (!isAtEnd() && peekChar() == '=')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_PLUS_ASSIGN, "+=",
                                     startLine, startCol);
                }
                return makeToken(TokenType::OP_PLUS, "+", startLine, startCol);

            case '-':
                if (!isAtEnd() && peekChar() == '-')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_DECREMENT, "--",
                                     startLine, startCol);
                }
                else if (!isAtEnd() && peekChar() == '=')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_MINUS_ASSIGN, "-=",
                                     startLine, startCol);
                }
                else if (!isAtEnd() && peekChar() == '>')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_ARROW, "->",
                                     startLine, startCol);
                }
                return makeToken(TokenType::OP_MINUS, "-", startLine, startCol);

            case '*':
                if (!isAtEnd() && peekChar() == '=')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_MUL_ASSIGN, "*=",
                                     startLine, startCol);
                }
                return makeToken(TokenType::OP_MUL, "*", startLine, startCol);

            case '/':
                if (!isAtEnd() && peekChar() == '/')
                {
                    advanceChar();
                    return skipLineComment(startLine, startCol);
                }
                else if (!isAtEnd() && peekChar() == '*')
                {
                    advanceChar();
                    return skipBlockComment(startLine, startCol);
                }
                else if (!isAtEnd() && peekChar() == '=')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_DIV_ASSIGN, "/=",
                                     startLine, startCol);
                }
                return makeToken(TokenType::OP_DIV, "/", startLine, startCol);

            case '%':
                if (!isAtEnd() && peekChar() == '=')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_MOD_ASSIGN, "%=",
                                     startLine, startCol);
                }
                return makeToken(TokenType::OP_MOD, "%", startLine, startCol);

            case '=':
                if (!isAtEnd() && peekChar() == '=')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_EQ, "==",
                                     startLine, startCol);
                }
                return makeToken(TokenType::OP_ASSIGN, "=", startLine, startCol);

            case '!':
                if (!isAtEnd() && peekChar() == '=')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_NEQ, "!=",
                                     startLine, startCol);
                }
                return makeToken(TokenType::OP_NOT, "!", startLine, startCol);

            case '<':
                if (!isAtEnd() && peekChar() == '<')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_SHIFT_LEFT, "<<",
                                     startLine, startCol);
                }
                else if (!isAtEnd() && peekChar() == '=')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_LTE, "<=",
                                     startLine, startCol);
                }
                return makeToken(TokenType::OP_LT, "<", startLine, startCol);

            case '>':
                if (!isAtEnd() && peekChar() == '>')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_SHIFT_RIGHT, ">>",
                                     startLine, startCol);
                }
                else if (!isAtEnd() && peekChar() == '=')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_GTE, ">=",
                                     startLine, startCol);
                }
                return makeToken(TokenType::OP_GT, ">", startLine, startCol);

            case '&':
                if (!isAtEnd() && peekChar() == '&')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_AND, "&&",
                                     startLine, startCol);
                }
                return makeToken(TokenType::OP_BITWISE_AND, "&",
                                 startLine, startCol);

            case '|':
                if (!isAtEnd() && peekChar() == '|')
                {
                    advanceChar();
                    return makeToken(TokenType::OP_OR, "||",
                                     startLine, startCol);
                }
                return makeToken(TokenType::OP_BITWISE_OR, "|",
                                 startLine, startCol);

            case '^':
                return makeToken(TokenType::OP_BITWISE_XOR, "^",
                                 startLine, startCol);

            case '~':
                return makeToken(TokenType::OP_BITWISE_NOT, "~",
                                 startLine, startCol);

            case '(':
                return makeToken(TokenType::LPAREN, "(", startLine, startCol);

            case ')':
                return makeToken(TokenType::RPAREN, ")", startLine, startCol);

            case '{':
                return makeToken(TokenType::LBRACE, "{", startLine, startCol);

            case '}':
                return makeToken(TokenType::RBRACE, "}", startLine, startCol);

            case '[':
                return makeToken(TokenType::LBRACKET, "[", startLine, startCol);

            case ']':
                return makeToken(TokenType::RBRACKET, "]", startLine, startCol);

            case ',':
                return makeToken(TokenType::COMMA, ",", startLine, startCol);

            case ';':
                return makeToken(TokenType::SEMICOLON, ";", startLine, startCol);

            case '.':
                return makeToken(TokenType::DOT, ".", startLine, startCol);

            case '#':
                return makeToken(TokenType::HASH, "#", startLine, startCol);

            case ':':
                return makeToken(TokenType::COLON, ":", startLine, startCol);

            case '?':
                return makeToken(TokenType::QUESTION, "?", startLine, startCol);

            default:
                return makeToken(TokenType::UNKNOWN, string(1, c),
                                 startLine, startCol);
            }
        }

        return makeToken(TokenType::END_OF_FILE, "", line, column);
    }

    vector<Token> tokenizeAll()
    {
        vector<Token> tokens;
        for (;;)
        {
            Token t = nextToken();
            if (t.type == TokenType::COMMENT)
                continue;
            tokens.push_back(t);
            if (t.type == TokenType::END_OF_FILE)
                break;
        }
        return tokens;
    }
};

// =============================================================
// ERROR HANDLING
// =============================================================

struct ErrorInfo
{
    string message;
    string suggestion;
};

class SuggestionEngine
{
public:
    string getSuggestion(const string &errorMessage)
    {
        if (errorMessage.find("Expected ';'") != string::npos)
            return "Ensure each statement ends with a semicolon. Example: int x = 5;";

        if (errorMessage.find("Unbalanced '('") != string::npos ||
            errorMessage.find("Unbalanced ')')") != string::npos)
            return "Check that each '(' has a matching ')'.";

        if (errorMessage.find("Unbalanced '{'") != string::npos ||
            errorMessage.find("Unbalanced '}'") != string::npos)
            return "Check that each '{' has a matching '}'.";

        if (errorMessage.find("Undeclared variable") != string::npos)
            return "Declare the variable before use. Example: int x;";

        if (errorMessage.find("Redeclaration") != string::npos)
            return "Remove the duplicate declaration or rename one of the variables.";

        if (errorMessage.find("Type mismatch") != string::npos)
            return "Ensure the types on both sides of the assignment or operation are compatible.";

        if (errorMessage.find("Expected identifier") != string::npos)
            return "Provide a valid variable or function name.";

        if (errorMessage.find("Pointer assignment") != string::npos)
            return "Check that pointer variables are assigned compatible pointer or address types.";

        if (errorMessage.find("Condition in if/while") != string::npos)
            return "Use an integer (or boolean-like) expression in conditions.";

        return "Review the syntax near the reported line and column.";
    }
};

// =============================================================
// STANDARD LIBRARY KNOWLEDGE BASE
// =============================================================

// Basic database of standard C library functions we want to recognize.
class StandardLibrary
{
    unordered_map<string, string> stdioFunctions;
    unordered_map<string, string> stdlibFunctions;
    unordered_map<string, string> stringFunctions;
    unordered_map<string, string> mathFunctions;

public:
    StandardLibrary()
    {
        stdioFunctions["printf"] = "int";
        stdioFunctions["scanf"] = "int";
        stdioFunctions["fprintf"] = "int";
        stdioFunctions["fscanf"] = "int";
        stdioFunctions["sprintf"] = "int";
        stdioFunctions["sscanf"] = "int";
        stdioFunctions["puts"] = "int";
        stdioFunctions["gets"] = "char*";
        stdioFunctions["fgets"] = "char*";
        stdioFunctions["fputs"] = "int";

        stdlibFunctions["malloc"] = "void*";
        stdlibFunctions["calloc"] = "void*";
        stdlibFunctions["realloc"] = "void*";
        stdlibFunctions["free"] = "void";
        stdlibFunctions["exit"] = "void";
        stdlibFunctions["atoi"] = "int";
        stdlibFunctions["atof"] = "double";
        stdlibFunctions["rand"] = "int";
        stdlibFunctions["srand"] = "void";

        stringFunctions["strlen"] = "size_t";
        stringFunctions["strcpy"] = "char*";
        stringFunctions["strncpy"] = "char*";
        stringFunctions["strcat"] = "char*";
        stringFunctions["strcmp"] = "int";
        stringFunctions["strncmp"] = "int";
        stringFunctions["strchr"] = "char*";
        stringFunctions["strstr"] = "char*";

        mathFunctions["sin"] = "double";
        mathFunctions["cos"] = "double";
        mathFunctions["tan"] = "double";
        mathFunctions["sqrt"] = "double";
        mathFunctions["pow"] = "double";
        mathFunctions["fabs"] = "double";
    }

    bool isStdioFunction(const string &name) const
    {
        return stdioFunctions.find(name) != stdioFunctions.end();
    }

    bool isStdlibFunction(const string &name) const
    {
        return stdlibFunctions.find(name) != stdlibFunctions.end();
    }

    bool isStringFunction(const string &name) const
    {
        return stringFunctions.find(name) != stringFunctions.end();
    }

    bool isMathFunction(const string &name) const
    {
        return mathFunctions.find(name) != mathFunctions.end();
    }

    string getReturnType(const string &name) const
    {
        auto it = stdioFunctions.find(name);
        if (it != stdioFunctions.end())
            return it->second;
        it = stdlibFunctions.find(name);
        if (it != stdlibFunctions.end())
            return it->second;
        it = stringFunctions.find(name);
        if (it != stringFunctions.end())
            return it->second;
        it = mathFunctions.find(name);
        if (it != mathFunctions.end())
            return it->second;
        return "UNKNOWN";
    }

    bool isKnownFunction(const string &name) const
    {
        return isStdioFunction(name) ||
               isStdlibFunction(name) ||
               isStringFunction(name) ||
               isMathFunction(name);
    }
};

// =============================================================
// TYPE SYSTEM
// =============================================================

struct VarInfo
{
    string name, type;
    int line, column;
    VarInfo(string n = "", string t = "", int l = 0, int c = 0)
        : name(std::move(n)), type(std::move(t)), line(l), column(c) {}
};

class TypeSystem
{
public:
    static bool areTypesCompatible(const string &lhs, const string &rhs)
    {
        if (lhs == rhs)
            return true;

        set<string> numericTypes = {"int", "float", "double", "char"};
        if (numericTypes.count(lhs) && numericTypes.count(rhs))
            return true;

        if (lhs.find("*") != string::npos && rhs.find("*") != string::npos)
            return true;
        if (lhs == "void*" && rhs.find("*") != string::npos)
            return true;

        return false;
    }

    static string getOperationResultType(const string &lhs, const string &rhs, const string &op)
    {
        set<string> numericTypes = {"int", "float", "double", "char"};

        if (op == "+" || op == "-" || op == "*" || op == "/")
        {
            if (numericTypes.count(lhs) && numericTypes.count(rhs))
            {
                if (lhs == "double" || rhs == "double")
                    return "double";
                if (lhs == "float" || rhs == "float")
                    return "float";
                return "int";
            }
            if ((lhs == "string" || lhs.find("*char") != string::npos) && op == "+")
                return "string";
            return "INVALID";
        }

        if (op == "==" || op == "!=" || op == "<" || op == ">" ||
            op == "<=" || op == ">=")
        {
            if (areTypesCompatible(lhs, rhs))
                return "int";
            return "INVALID";
        }

        if (op == "&&" || op == "||")
            return "int";

        if (op == "&" || op == "|" || op == "^" || op == "<<" || op == ">>")
        {
            if (numericTypes.count(lhs) && numericTypes.count(rhs))
                return "int";
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

// =============================================================
// SYMBOL TABLE
// =============================================================

class SymbolTable
{
    unordered_map<string, VarInfo> table;
    vector<unordered_map<string, VarInfo>> scopeStack;
    StandardLibrary stdLib;

public:
    SymbolTable()
    {
        for (const auto &name : {"printf", "scanf", "malloc", "free", "strlen", "sqrt"})
        {
            VarInfo v(name, "function", 0, 0);
            table[name] = v;
        }
    }

    void enterScope()
    {
        scopeStack.push_back({});
    }

    void exitScope()
    {
        if (!scopeStack.empty())
            scopeStack.pop_back();
    }

    bool declare(const string &name, const string &type,
                 int line, int column)
    {
        if (scopeStack.empty())
        {
            if (table.count(name))
                return false;
            table[name] = VarInfo(name, type, line, column);
            return true;
        }

        auto &currentScope = scopeStack.back();
        if (currentScope.count(name))
            return false;

        currentScope[name] = VarInfo(name, type, line, column);
        return true;
    }

    bool exists(const string &name) const
    {
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it)
        {
            if (it->count(name))
                return true;
        }
        if (table.count(name))
            return true;
        if (stdLib.isStdioFunction(name) ||
            stdLib.isStdlibFunction(name) ||
            stdLib.isStringFunction(name) ||
            stdLib.isMathFunction(name))
        {
            return true;
        }
        return false;
    }

    string getType(const string &name) const
    {
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it)
        {
            auto found = it->find(name);
            if (found != it->end())
                return found->second.type;
        }

        auto it2 = table.find(name);
        if (it2 != table.end())
            return it2->second.type;

        if (stdLib.isKnownFunction(name))
            return stdLib.getReturnType(name);

        return "UNKNOWN";
    }
};

// =============================================================
// PARSER / ANALYZER
// =============================================================

class Parser
{
    vector<Token> tokens;
    size_t index;
    SymbolTable sym;
    SuggestionEngine suggestionEngine;
    vector<ErrorInfo> errors;
    int parenBalance;
    int braceBalance;

public:
    explicit Parser(vector<Token> toks)
        : tokens(std::move(toks)), index(0), parenBalance(0), braceBalance(0)
    {
    }

    Token &curr()
    {
        static Token eofToken(TokenType::END_OF_FILE, "", 0, 0);
        if (index >= tokens.size())
            return eofToken;
        return tokens[index];
    }

    Token &peek(int offset = 1)
    {
        static Token eofToken(TokenType::END_OF_FILE, "", 0, 0);
        if (index + offset >= tokens.size())
            return eofToken;
        return tokens[index + offset];
    }

    void advance()
    {
        if (index < tokens.size())
            index++;
    }

    bool match(TokenType t)
    {
        if (curr().type == t)
        {
            advance();
            return true;
        }
        return false;
    }

    void expect(TokenType t, const string &msg)
    {
        if (curr().type != t)
        {
            Token &tk = curr();
            string errMsg = "Line " + to_string(tk.line) + ":" + to_string(tk.column) +
                            " - Expected '" + msg + "' but got '" + tk.value + "'";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
        }
        else
        {
            advance();
        }
    }

    bool isTypeToken(const Token &t)
    {
        return t.type == TokenType::TOK_INT || t.type == TokenType::TOK_FLOAT ||
               t.type == TokenType::TOK_DOUBLE || t.type == TokenType::TOK_CHAR ||
               t.type == TokenType::TOK_LONG || t.type == TokenType::TOK_SHORT ||
               t.type == TokenType::TOK_VOID || t.type == TokenType::TOK_SIGNED ||
               t.type == TokenType::TOK_UNSIGNED;
    }

    string tokenTypeToString(const Token &t)
    {
        switch (t.type)
        {
        case TokenType::TOK_INT:
            return "int";
        case TokenType::TOK_FLOAT:
            return "float";
        case TokenType::TOK_DOUBLE:
            return "double";
        case TokenType::TOK_CHAR:
            return "char";
        case TokenType::TOK_VOID:
            return "void";
        case TokenType::TOK_LONG:
            return "long";
        case TokenType::TOK_SHORT:
            return "short";
        case TokenType::TOK_SIGNED:
            return "signed";
        case TokenType::TOK_UNSIGNED:
            return "unsigned";
        default:
            return "UNKNOWN";
        }
    }

    void parse()
    {
        while (curr().type != TokenType::END_OF_FILE)
        {
            parseTopLevel();
        }

        if (parenBalance != 0)
        {
            string errMsg = "Unbalanced '(' and ')' in code.";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
        }

        if (braceBalance != 0)
        {
            string errMsg = "Unbalanced '{' and '}' in code.";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
        }
    }

    void parseTopLevel()
    {
        if (curr().type == TokenType::HASH)
        {
            parsePreprocessor();
            return;
        }

        if (curr().type == TokenType::TOK_STRUCT)
        {
            parseStruct();
            return;
        }

        if (curr().type == TokenType::TOK_TYPEDEF)
        {
            parseTypedef();
            return;
        }

        if (isTypeToken(curr()))
        {
            Token typeToken = curr();
            string baseType = tokenTypeToString(typeToken);
            advance();
            parseFunctionOrVarDecl(baseType);
            return;
        }

        advance();
    }

    void parsePreprocessor()
    {
        if (match(TokenType::HASH))
        {
            if (curr().type == TokenType::TOK_INCLUDE)
            {
                while (curr().type != TokenType::END_OF_FILE &&
                       curr().type != TokenType::SEMICOLON &&
                       curr().type != TokenType::RPAREN &&
                       curr().type != TokenType::LBRACE &&
                       curr().type != TokenType::RBRACE)
                {
                    advance();
                }
            }
            else if (curr().type == TokenType::TOK_DEFINE)
            {
                while (curr().type != TokenType::END_OF_FILE &&
                       curr().type != TokenType::SEMICOLON &&
                       curr().type != TokenType::LBRACE &&
                       curr().type != TokenType::RBRACE)
                {
                    advance();
                }
            }
        }
    }

    void parseStruct()
    {
        advance();

        if (curr().type == TokenType::TOK_IDENTIFIER)
        {
            string structName = curr().value;
            int line = curr().line;
            int col = curr().column;
            advance();

            if (match(TokenType::LBRACE))
            {
                braceBalance++;
                while (curr().type != TokenType::RBRACE &&
                       curr().type != TokenType::END_OF_FILE)
                {
                    if (isTypeToken(curr()))
                    {
                        Token fieldTypeTok = curr();
                        string fieldType = tokenTypeToString(fieldTypeTok);
                        advance();

                        if (curr().type == TokenType::TOK_IDENTIFIER)
                        {
                            int fLine = curr().line, fCol = curr().column;
                            string fieldName = curr().value;
                            advance();

                            while (curr().type == TokenType::COMMA)
                            {
                                advance();
                                if (curr().type == TokenType::TOK_IDENTIFIER)
                                {
                                    fieldName = curr().value;
                                    fLine = curr().line;
                                    fCol = curr().column;
                                    advance();
                                }
                                else
                                {
                                    string errMsg = "Line " + to_string(curr().line) + ":" +
                                                    to_string(curr().column) +
                                                    " - Expected identifier in struct field list";
                                    errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
                                    break;
                                }
                            }

                            expect(TokenType::SEMICOLON, ";");
                        }
                        else
                        {
                            string errMsg = "Line " + to_string(curr().line) + ":" +
                                            to_string(curr().column) +
                                            " - Expected struct field name";
                            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
                            break;
                        }
                    }
                    else
                    {
                        advance();
                    }
                }
                if (curr().type == TokenType::RBRACE)
                {
                    braceBalance--;
                    advance();
                }
                else
                {
                    string errMsg = "Line " + to_string(curr().line) + ":" +
                                    to_string(curr().column) +
                                    " - Expected '}' to close struct definition";
                    errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
                }

                if (curr().type == TokenType::TOK_IDENTIFIER)
                {
                    string varName = curr().value;
                    int vLine = curr().line, vCol = curr().column;
                    advance();

                    if (curr().type == TokenType::LBRACKET)
                    {
                        advance();
                        if (curr().type == TokenType::TOK_NUMBER)
                            advance();
                        expect(TokenType::RBRACKET, "]");
                    }

                    string declaredType = "struct " + structName;
                    if (!sym.declare(varName, declaredType, vLine, vCol))
                    {
                        string errMsg = "Line " + to_string(vLine) + ":" +
                                        to_string(vCol) +
                                        " - Redeclaration of '" + varName + "'";
                        errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
                    }

                    if (curr().type == TokenType::OP_ASSIGN)
                    {
                        advance();
                        parseExpression();
                    }

                    if (curr().type == TokenType::SEMICOLON)
                        advance();
                    else
                    {
                        string errMsg = "Line " + to_string(curr().line) + ":" +
                                        to_string(curr().column) +
                                        " - Expected ';' after struct variable declaration";
                        errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
                    }
                }
            }
            else
            {
                string errMsg = "Line " + to_string(line) + ":" + to_string(col) +
                                " - Expected '{' after struct declaration";
                errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
            }
        }
        else
        {
            string errMsg = "Line " + to_string(curr().line) + ":" +
                            to_string(curr().column) +
                            " - Expected identifier after 'struct'";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
        }
    }

    void parseTypedef()
    {
        advance();

        bool isStructTypedef = false;
        string baseType;

        if (curr().type == TokenType::TOK_STRUCT)
        {
            isStructTypedef = true;
            advance();

            if (curr().type == TokenType::TOK_IDENTIFIER)
            {
                baseType = "struct " + curr().value;
                advance();
            }
            else
            {
                string errMsg = "Line " + to_string(curr().line) + ":" +
                                to_string(curr().column) +
                                " - Expected struct name in typedef";
                errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
                return;
            }
        }
        else if (isTypeToken(curr()))
        {
            baseType = tokenTypeToString(curr());
            advance();
        }
        else
        {
            string errMsg = "Line " + to_string(curr().line) + ":" +
                            to_string(curr().column) +
                            " - Expected type in typedef";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
            return;
        }

        string finalType = baseType;
        while (curr().type == TokenType::OP_STAR)
        {
            finalType += "*";
            advance();
        }

        if (curr().type != TokenType::TOK_IDENTIFIER)
        {
            string errMsg = "Line " + to_string(curr().line) + ":" +
                            to_string(curr().column) +
                            " - Expected typedef alias name";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
            return;
        }

        string aliasName = curr().value;
        int line = curr().line, col = curr().column;
        advance();

        if (!sym.declare(aliasName, finalType, line, col))
        {
            string errMsg = "Line " + to_string(line) + ":" + to_string(col) +
                            " - Redeclaration of typedef '" + aliasName + "'";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
        }

        while (curr().type == TokenType::COMMA)
        {
            advance();

            string nextType = baseType;
            while (curr().type == TokenType::OP_STAR)
            {
                nextType += "*";
                advance();
            }

            if (curr().type != TokenType::TOK_IDENTIFIER)
            {
                string errMsg = "Line " + to_string(curr().line) + ":" +
                                to_string(curr().column) +
                                " - Expected typedef alias name";
                errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
                break;
            }

            string alias2 = curr().value;
            int line2 = curr().line, col2 = curr().column;
            advance();

            if (!sym.declare(alias2, nextType, line2, col2))
            {
                string errMsg = "Line " + to_string(line2) + ":" +
                                to_string(col2) +
                                " - Redeclaration of typedef '" + alias2 + "'";
                errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
            }
        }

        expect(TokenType::SEMICOLON, ";");
    }

    void parseFunctionOrVarDecl(const string &baseType)
    {
        string declaredType = baseType;

        while (curr().type == TokenType::OP_STAR)
        {
            declaredType += "*";
            advance();
        }

        if (curr().type != TokenType::TOK_IDENTIFIER)
        {
            Token &t = curr();
            string errMsg = "Line " + to_string(t.line) + ":" + to_string(t.column) +
                            " - Expected identifier after type '" + baseType + "'";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
            while (curr().type != TokenType::SEMICOLON &&
                   curr().type != TokenType::LBRACE &&
                   curr().type != TokenType::END_OF_FILE)
            {
                advance();
            }
            if (curr().type == TokenType::LBRACE)
            {
                braceBalance++;
                advance();
                parseBlock();
                if (curr().type == TokenType::RBRACE)
                {
                    braceBalance--;
                    advance();
                }
            }
            else if (curr().type == TokenType::SEMICOLON)
            {
                advance();
            }
            return;
        }

        Token identTok = curr();
        string name = identTok.value;
        advance();

        if (curr().type == TokenType::LPAREN)
        {
            parseFunction(declaredType, name, identTok);
        }
        else
        {
            parseVarDecl(declaredType, identTok);
        }
    }

    void parseFunction(const string &returnType, const string &funcName,
                       const Token &funcToken)
    {
        if (!sym.declare(funcName, returnType, funcToken.line, funcToken.column))
        {
            string errMsg = "Line " + to_string(funcToken.line) + ":" +
                            to_string(funcToken.column) +
                            " - Redeclaration of function '" + funcName + "'";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
        }

        expect(TokenType::LPAREN, "(");

        sym.enterScope();

        while (curr().type != TokenType::RPAREN &&
               curr().type != TokenType::END_OF_FILE)
        {
            if (isTypeToken(curr()))
            {
                string paramType = tokenTypeToString(curr());
                advance();

                while (curr().type == TokenType::OP_STAR)
                {
                    paramType += "*";
                    advance();
                }

                if (curr().type == TokenType::TOK_IDENTIFIER)
                {
                    string paramName = curr().value;
                    int pLine = curr().line, pCol = curr().column;
                    advance();
                    if (!sym.declare(paramName, paramType, pLine, pCol))
                    {
                        string errMsg = "Line " + to_string(pLine) + ":" +
                                        to_string(pCol) +
                                        " - Redeclaration of parameter '" + paramName + "'";
                        errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
                    }
                }

                if (curr().type == TokenType::COMMA)
                    advance();
                else
                    break;
            }
            else
            {
                break;
            }
        }

        expect(TokenType::RPAREN, ")");

        if (curr().type == TokenType::LBRACE)
        {
            braceBalance++;
            advance();
            parseBlockWithReturnCheck(returnType);
            if (curr().type == TokenType::RBRACE)
            {
                braceBalance--;
                advance();
            }
            else
            {
                string errMsg = "Line " + to_string(curr().line) + ":" +
                                to_string(curr().column) +
                                " - Expected '}' at end of function '" + funcName + "'";
                errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
            }
        }
        else
        {
            string errMsg = "Line " + to_string(curr().line) + ":" +
                            to_string(curr().column) +
                            " - Expected '{' to start body of function '" +
                            funcName + "'";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
        }

        sym.exitScope();
    }

    void parseBlock()
    {
        sym.enterScope();

        while (curr().type != TokenType::RBRACE &&
               curr().type != TokenType::END_OF_FILE)
        {
            if (isTypeToken(curr()))
            {
                Token typeTok = curr();
                string baseType = tokenTypeToString(typeTok);
                advance();
                parseFunctionOrVarDecl(baseType);
            }
            else if (curr().type == TokenType::TOK_RETURN)
            {
                advance();
                parseExpression();
                expect(TokenType::SEMICOLON, ";");
            }
            else
            {
                parseStatement();
            }
        }

        sym.exitScope();
    }

    void parseBlockWithReturnCheck(const string &returnType)
    {
        sym.enterScope();
        bool hasReturn = false;

        while (curr().type != TokenType::RBRACE &&
               curr().type != TokenType::END_OF_FILE)
        {
            if (isTypeToken(curr()))
            {
                Token typeTok = curr();
                string baseType = tokenTypeToString(typeTok);
                advance();
                parseFunctionOrVarDecl(baseType);
            }
            else if (curr().type == TokenType::TOK_RETURN)
            {
                hasReturn = true;
                Token retTok = curr();
                advance();
                string exprType = parseExpression();
                if (returnType != "void" && exprType != "UNKNOWN" &&
                    !TypeSystem::areTypesCompatible(returnType, exprType))
                {
                    string errMsg = "Line " + to_string(retTok.line) + ":" +
                                    to_string(retTok.column) +
                                    " - Return type mismatch: function returns '" +
                                    returnType + "' but return expression is '" +
                                    exprType + "'";
                    string sug = "SUGGESTION: Ensure the expression type matches the function's return type";
                    errors.push_back({errMsg, sug});
                }
                expect(TokenType::SEMICOLON, ";");
            }
            else
            {
                parseStatement();
            }
        }

        if (returnType != "void" && !hasReturn)
        {
            string errMsg = "Missing return statement in non-void function";
            string sug = "SUGGESTION: Add 'return <value>;' before the end of the function";
            errors.push_back({errMsg, sug});
        }

        sym.exitScope();
    }

    void parseVarDecl(const string &type, const Token &firstIdent)
    {
        string declaredType = type;
        string varName = firstIdent.value;
        int vLine = firstIdent.line;
        int vCol = firstIdent.column;

        if (curr().type == TokenType::LBRACKET)
        {
            advance();
            if (curr().type == TokenType::TOK_NUMBER)
                advance();
            expect(TokenType::RBRACKET, "]");
            declaredType += "[]";
        }

        if (!sym.declare(varName, declaredType, vLine, vCol))
        {
            string errMsg = "Line " + to_string(vLine) + ":" + to_string(vCol) +
                            " - Redeclaration of '" + varName + "'";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
        }

        if (curr().type == TokenType::OP_ASSIGN)
        {
            Token assignTok = curr();
            advance();

            if (curr().type == TokenType::LBRACE)
            {
                advance();
                while (curr().type != TokenType::RBRACE &&
                       curr().type != TokenType::END_OF_FILE)
                {
                    parseExpression();
                    if (curr().type == TokenType::COMMA)
                        advance();
                    else
                        break;
                }
                expect(TokenType::RBRACE, "}");
            }
            else
            {
                string rhsType = parseExpression();
                if (rhsType != "UNKNOWN" &&
                    !TypeSystem::areTypesCompatible(declaredType, rhsType))
                {
                    string errMsg = "Line " + to_string(assignTok.line) + ":" +
                                    to_string(assignTok.column) +
                                    " - Type mismatch: assigning '" +
                                    rhsType + "' to '" + declaredType + "'";
                    string sug = "SUGGESTION: Types must be compatible or cast the value explicitly";
                    errors.push_back({errMsg, sug});
                }
            }
        }

        while (curr().type == TokenType::COMMA)
        {
            advance();

            string nextDeclaredType = type;

            while (curr().type == TokenType::OP_STAR)
            {
                nextDeclaredType += "*";
                advance();
            }

            if (curr().type != TokenType::TOK_IDENTIFIER)
            {
                Token bad = curr();
                string err = "Line " + to_string(bad.line) + ":" +
                             to_string(bad.column) +
                             " - Expected identifier";
                errors.push_back({err, "SUGGESTION: Provide a variable name after ',' (example: int *a, **b;)"});
                break;
            }

            Token t = curr();
            advance();

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
                string err = "Line " + to_string(t.line) + ":" +
                             to_string(t.column) +
                             " - Redeclaration of '" + t.value + "'";
                errors.push_back({err, suggestionEngine.getSuggestion(err)});
            }

            if (curr().type == TokenType::OP_ASSIGN)
            {
                Token assignTok = curr();
                advance();
                string rhsType = parseExpression();
                if (rhsType != "UNKNOWN" &&
                    !TypeSystem::areTypesCompatible(nextDeclaredType, rhsType))
                {
                    string errMsg = "Line " + to_string(assignTok.line) + ":" +
                                    to_string(assignTok.column) +
                                    " - Type mismatch: assigning '" + rhsType +
                                    "' to '" + nextDeclaredType + "'";
                    string sug = "SUGGESTION: Ensure initializer type matches the declared variable type";
                    errors.push_back({errMsg, sug});
                }
            }
        }

        if (curr().type == TokenType::SEMICOLON)
        {
            advance();
        }
        else
        {
            Token &t = curr();
            string errMsg = "Line " + to_string(t.line) + ":" + to_string(t.column) +
                            " - Missing ';' after variable declaration";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
        }
    }

    void parseStatement()
    {
        Token &t = curr();
        switch (t.type)
        {
        case TokenType::TOK_IF:
            parseIfStatement();
            break;
        case TokenType::TOK_FOR:
            parseForStatement();
            break;
        case TokenType::TOK_WHILE:
            parseWhileStatement();
            break;
        case TokenType::TOK_DO:
            parseDoWhileStatement();
            break;
        case TokenType::TOK_SWITCH:
            parseSwitchStatement();
            break;
        case TokenType::LBRACE:
            braceBalance++;
            advance();
            parseBlock();
            if (curr().type == TokenType::RBRACE)
            {
                braceBalance--;
                advance();
            }
            else
            {
                string errMsg = "Line " + to_string(curr().line) + ":" +
                                to_string(curr().column) +
                                " - Expected '}' to close block";
                errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
            }
            break;
        case TokenType::TOK_RETURN:
        {
            Token retTok = curr();
            advance();
            parseExpression();
            if (curr().type == TokenType::SEMICOLON)
            {
                advance();
            }
            else
            {
                string errMsg = "Line " + to_string(curr().line) + ":" +
                                to_string(curr().column) +
                                " - Missing ';' after return statement";
                errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
            }
        }
        break;
        default:
            parseExprOrAssignment();
            if (curr().type == TokenType::SEMICOLON)
            {
                advance();
            }
            else if (curr().type != TokenType::RBRACE &&
                     curr().type != TokenType::END_OF_FILE)
            {
                string errMsg = "Line " + to_string(curr().line) + ":" +
                                to_string(curr().column) +
                                " - Missing ';' after statement";
                errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
            }
            break;
        }
    }

    void parseIfStatement()
    {
        advance();
        expect(TokenType::LPAREN, "(");
        string condType = parseExpression();
        if (condType != "UNKNOWN" &&
            !TypeSystem::isNumericType(condType))
        {
            string errMsg = "Line " + to_string(curr().line) + ":" +
                            to_string(curr().column) +
                            " - Condition in if must be numeric/bool-like, got '" +
                            condType + "'";
            string sug = "SUGGESTION: Use an integer or comparison expression in if condition";
            errors.push_back({errMsg, sug});
        }
        expect(TokenType::RPAREN, ")");
        parseStatement();
        if (curr().type == TokenType::TOK_ELSE)
        {
            advance();
            parseStatement();
        }
    }

    void parseForStatement()
    {
        advance();
        expect(TokenType::LPAREN, "(");
        if (curr().type != TokenType::SEMICOLON)
        {
            if (isTypeToken(curr()))
            {
                Token typeTok = curr();
                string baseType = tokenTypeToString(typeTok);
                advance();
                if (curr().type == TokenType::TOK_IDENTIFIER)
                    parseVarDecl(baseType, curr());
            }
            else
                parseExprOrAssignment();
        }
        expect(TokenType::SEMICOLON, ";");

        if (curr().type != TokenType::SEMICOLON)
        {
            string condType = parseExpression();
            if (condType != "UNKNOWN" &&
                !TypeSystem::isNumericType(condType))
            {
                string errMsg = "Line " + to_string(curr().line) + ":" +
                                to_string(curr().column) +
                                " - Condition in for must be numeric/bool-like, got '" +
                                condType + "'";
                string sug = "SUGGESTION: Use an integer or comparison expression in for condition";
                errors.push_back({errMsg, sug});
            }
        }
        expect(TokenType::SEMICOLON, ";");

        if (curr().type != TokenType::RPAREN)
        {
            parseExprOrAssignment();
        }
        expect(TokenType::RPAREN, ")");

        parseStatement();
    }

    void parseWhileStatement()
    {
        advance();
        expect(TokenType::LPAREN, "(");
        string condType = parseExpression();
        if (condType != "UNKNOWN" &&
            !TypeSystem::isNumericType(condType))
        {
            string errMsg = "Line " + to_string(curr().line) + ":" +
                            to_string(curr().column) +
                            " - Condition in while must be numeric/bool-like, got '" +
                            condType + "'";
            string sug = "SUGGESTION: Use an integer or comparison expression in while condition";
            errors.push_back({errMsg, sug});
        }
        expect(TokenType::RPAREN, ")");
        parseStatement();
    }

    void parseDoWhileStatement()
    {
        advance();
        parseStatement();
        if (curr().type == TokenType::TOK_WHILE)
        {
            advance();
            expect(TokenType::LPAREN, "(");
            string condType = parseExpression();
            if (condType != "UNKNOWN" &&
                !TypeSystem::isNumericType(condType))
            {
                string errMsg = "Line " + to_string(curr().line) + ":" +
                                to_string(curr().column) +
                                " - Condition in do-while must be numeric/bool-like, got '" +
                                condType + "'";
                string sug = "SUGGESTION: Use an integer or comparison expression in do-while condition";
                errors.push_back({errMsg, sug});
            }
            expect(TokenType::RPAREN, ")");
            expect(TokenType::SEMICOLON, ";");
        }
        else
        {
            string errMsg = "Line " + to_string(curr().line) + ":" +
                            to_string(curr().column) +
                            " - Expected 'while' after 'do' block";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
        }
    }

    void parseSwitchStatement()
    {
        advance();
        expect(TokenType::LPAREN, "(");
        parseExpression();
        expect(TokenType::RPAREN, ")");
        expect(TokenType::LBRACE, "{");
        braceBalance++;
        while (curr().type != TokenType::RBRACE &&
               curr().type != TokenType::END_OF_FILE)
        {
            if (curr().type == TokenType::TOK_CASE ||
                curr().type == TokenType::TOK_DEFAULT)
            {
                advance();
                if (curr().type != TokenType::COLON)
                {
                    parseExpression();
                }
                expect(TokenType::COLON, ":");
            }
            else
            {
                parseStatement();
            }
        }
        if (curr().type == TokenType::RBRACE)
        {
            braceBalance--;
            advance();
        }
        else
        {
            string errMsg = "Line " + to_string(curr().line) + ":" +
                            to_string(curr().column) +
                            " - Expected '}' to close switch";
            errors.push_back({errMsg, suggestionEngine.getSuggestion(errMsg)});
        }
    }

    bool isOp(const Token &t)
    {
        switch (t.type)
        {
        case TokenType::OP_PLUS:
        case TokenType::OP_MINUS:
        case TokenType::OP_MUL:
        case TokenType::OP_DIV:
        case TokenType::OP_MOD:
        case TokenType::OP_EQ:
        case TokenType::OP_NEQ:
        case TokenType::OP_LT:
        case TokenType::OP_GT:
        case TokenType::OP_LTE:
        case TokenType::OP_GTE:
        case TokenType::OP_AND:
        case TokenType::OP_OR:
        case TokenType::OP_BITWISE_AND:
        case TokenType::OP_BITWISE_OR:
        case TokenType::OP_BITWISE_XOR:
        case TokenType::OP_SHIFT_LEFT:
        case TokenType::OP_SHIFT_RIGHT:
            return true;
        default:
            return false;
        }
    }

    string parsePrimaryWithType()
    {
        Token t = curr();
        if (t.type == TokenType::TOK_IDENTIFIER)
        {
            if (!sym.exists(t.value))
            {
                string errMsg = "Line " + to_string(t.line) + ":" +
                                to_string(t.column) +
                                " - Undeclared identifier '" + t.value + "'";
                string sug = suggestionEngine.getSuggestion(errMsg);
                errors.push_back({errMsg, sug});
            }

            string idName = t.value;
            string idType = sym.getType(idName);
            advance();

            if (curr().type == TokenType::LPAREN)
            {
                advance();
                while (curr().type != TokenType::RPAREN &&
                       curr().type != TokenType::END_OF_FILE)
                {
                    parseExpression();
                    if (curr().type == TokenType::COMMA)
                        advance();
                    else
                        break;
                }
                expect(TokenType::RPAREN, ")");
                return idType;
            }

            return idType;
        }
        else if (t.type == TokenType::TOK_NUMBER)
        {
            string val = t.value;
            advance();
            if (val.find('.') != string::npos ||
                val.find('e') != string::npos ||
                val.find('E') != string::npos)
                return "double";
            return "int";
        }
        else if (t.type == TokenType::TOK_CHAR_LITERAL)
        {
            advance();
            return "char";
        }
        else if (t.type == TokenType::TOK_STRING_LITERAL)
        {
            advance();
            return "char*";
        }
        else if (t.type == TokenType::LPAREN)
        {
            parenBalance++;
            advance();
            string insideType = parseExpression();
            expect(TokenType::RPAREN, ")");
            parenBalance--;
            return insideType;
        }
        else
        {
            advance();
            return "UNKNOWN";
        }
    }

    string parseExpression()
    {
        string type = parsePrimaryWithType();

        while (isOp(curr()))
        {
            Token op = curr();
            advance();

            if (curr().type == TokenType::SEMICOLON ||
                curr().type == TokenType::RPAREN ||
                curr().type == TokenType::RBRACE ||
                curr().type == TokenType::COMMA)
            {
                string errMsg = "Line " + to_string(op.line) + ":" +
                                to_string(op.column) +
                                " - Incomplete expression: missing operand after '" +
                                op.value + "'";
                string sug = "SUGGESTION: Complete the expression. Example: x + y";
                errors.push_back({errMsg, sug});
                return type;
            }

            string rhsType = parsePrimaryWithType();

            if (type != "UNKNOWN" && rhsType != "UNKNOWN")
            {
                string resultType = TypeSystem::getOperationResultType(type, rhsType, op.value);

                if (resultType == "INVALID")
                {
                    string errMsg = "Line " + to_string(op.line) + ":" +
                                    to_string(op.column) +
                                    " - Type error: cannot apply '" +
                                    op.value + "' to '" + type +
                                    "' and '" + rhsType + "'";
                    string sug = "SUGGESTION: Ensure both operands are compatible types";
                    errors.push_back({errMsg, sug});
                }
                else if (resultType != "UNKNOWN")
                {
                    type = resultType;
                }
            }
        }
        return type;
    }

    void parseExprOrAssignment()
    {
        if (curr().type == TokenType::TOK_IDENTIFIER &&
            peek().type == TokenType::OP_ASSIGN)
        {
            Token id = curr();
            string varType = sym.getType(id.value);

            if (!sym.exists(id.value))
            {
                string errMsg = "Line " + to_string(id.line) + ":" +
                                to_string(id.column) +
                                " - Undeclared variable '" + id.value + "'";
                string sug = suggestionEngine.getSuggestion(errMsg);
                errors.push_back({errMsg, sug});
            }

            advance();
            Token assignTok = curr();
            advance();

            string rhsType = parseExpression();

            if (varType != "UNKNOWN" && rhsType != "UNKNOWN" &&
                !TypeSystem::areTypesCompatible(varType, rhsType))
            {
                string errMsg = "Line " + to_string(assignTok.line) + ":" +
                                to_string(assignTok.column) +
                                " - Type error: assigning '" + rhsType +
                                "' to '" + varType + "'";
                string sug = "SUGGESTION: Types must match. " + varType +
                             " expected, " + rhsType + " provided";
                errors.push_back({errMsg, sug});
            }
        }
        else
        {
            parseExpression();
        }
    }

    const vector<ErrorInfo> &getErrors() const
    {
        return errors;
    }
};

// =============================================================
// DRIVER
// =============================================================

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string line, source;
    while (getline(cin, line))
    {
        source += line + "\n";
    }

    Tokenizer tokenizer(source);
    vector<Token> tokens = tokenizer.tokenizeAll();

    Parser parser(tokens);
    parser.parse();
    const vector<ErrorInfo> &errs = parser.getErrors();

    if (errs.empty())
    {
        cout << "No errors detected.\n";
    }
    else
    {
        for (const auto &e : errs)
        {
            cout << e.message << "\n";
            if (!e.suggestion.empty())
                cout << "→ " << e.suggestion << "\n";
        }
    }

    return 0;
}

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

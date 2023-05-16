#include <iostream>
#include <fstream>
#include <ctype.h>
#include <cstring>
#include <vector>

using namespace std;

enum LexType {
    LEX_PROGRAM,
    LEX_INT, LEX_STRING, LEX_BOOLEAN,
    LEX_FALSE, LEX_TRUE,
    LEX_AND, LEX_OR, LEX_NOT, LEX_IF, LEX_ELSE,
    LEX_WHILE, LEX_BREAK,
    LEX_READ, LEX_WRITE,
    LEX_LCURLY,LEX_RCURLY, LEX_SEMICOLON,
    LEX_COMMA, LEX_COLON, LEX_ASSIGN,
    LEX_LPAREN, LEX_RPAREN,
    LEX_EQ, LEX_LESS, LEX_GREATER,
    LEX_LEQ, LEX_NEQ, LEX_GEQ,
    LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_SLASH,
    LEX_NUM, LEX_ID, LEX_STRLITERAL, LEX_NULL
};

class Lexeme {
    LexType type;
    int value;

public:
    Lexeme(LexType t = LEX_NULL, int v = 0) : type(t), value(v) {}

    LexType getType() const {
        return type;
    }

    int getValue() const {
        return value;
    }
};

const char* keywords[] = {"program", "int", "string", "boolean", "false", "true",
                          "and", "or", "not", "if", "else", "while", "break", "read", "write", NULL};

const char* operators[] = {"{", "}", ";", ",", ":", "=", "(", ")", "==", "<", ">",
                           "<=", "!=", ">=", "+",  "-", "*", "/", NULL};

int getLexIndex(string str, const char** arr) {
    int i = 0;
    while ((arr[i] != NULL) && (string(arr[i]) != str)) {
        i++;
    }
    if (arr[i] == NULL) {
        return -1;
    }
    return i;
}

vector<string> identifiers;
vector<string> constants;

Lexeme getLexeme() {
    enum State {H, IDENT, NUMB, STRING, COM, COMP, SYMB};
    char c;
    int n;
    string buf;
    State currentState = H;

    do {
        cin.get(c);

        switch (currentState) {
            case H:
                if (cin.eof()) {
                    return Lexeme();
                }

                if (c == ' ' || c == '\n' || c == '\t');
                else if (isalpha(c)) {
                    buf.push_back(c);
                    currentState = IDENT;
                }
                else if (isdigit(c)) {
                    n = c - '0';
                    currentState = NUMB;
                }
                else if (c == '+' || c == '-' || c == '*' || c == '{' || c == '}' ||
                         c == ';' || c == ',' || c == ':' || c == '(' || c == ')') {
                    currentState = SYMB;
                    cin.unget();
                }
                else if (c == '/') {
                    if (cin.peek() == '*') {
                        cin.get();
                        currentState = COM;
                    }
                    else {
                        cin.unget();
                        currentState = SYMB;
                    }
                }
                else
                if (c == '=' || c == '>' || c == '<' || c == '!') {
                    if (cin.peek() == '=') {
                        currentState = COMP;
                    }
                    else {
                        currentState = SYMB;
                    }
                    cin.unget();
                }
                else if (c == '"') {
                    currentState = STRING;
                }
                else {
                    throw -11111;
                }
                break;

            case IDENT:
                if (isdigit(c) || isalpha(c)) {
                    buf.push_back(c);
                }
                else {
                    cin.unget();
                    int index = getLexIndex(buf, keywords);
                    if (index == -1) {
                        identifiers.push_back(buf);
                        return Lexeme(LEX_ID, identifiers.size() - 1);
                    }
                    else {
                        return Lexeme((LexType)index, index);
                    }
                }
                break;

            case NUMB:
                if (isdigit(c)) {
                    n = n * 10 + (c - '0');
                }
                else {
                    cin.unget();
                    return Lexeme(LEX_NUM, n);
                }
                break;

            case COM:
                if (cin.eof()) {
                    throw -2;
                }
                if (c == '*') {
                    if (cin.peek() == '/') {
                        cin.get(c);
                        currentState = H;
                    }
                    else {
                        cin.unget();
                    }
                }
                break;

            case STRING:
                if (cin.eof()) {
                    throw -3;
                }
                if (c != '"') {
                    buf.push_back(c);
                } else {
                    constants.push_back(buf);
                    return Lexeme(LEX_STRLITERAL, constants.size() - 1);
                }
                break;

            case COMP: {
                string s1 = "";
                s1.push_back(c);
                s1.push_back(cin.get());
                int index1 = getLexIndex(s1, operators);
                return Lexeme((LexType) (index1 + 15), index1);
                break;
            }
            case SYMB:
                string ss = "";
                ss.push_back(c);
                int index2 = getLexIndex(ss, operators);
                return Lexeme((LexType)(index2 + 15), index2);
                break;
        }
    } while (true);
}

int main() {
    Lexeme lex;
    while (true) {
        lex = getLexeme();
        if (lex.getType() == LEX_NULL) {
            break;
        }
        else {
            cout << lex.getType() << " " << lex.getValue() << endl;
        }
    }
    return 0;
}

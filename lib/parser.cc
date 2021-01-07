#include <iostream>
#include <stack>
#include <cstdlib>

#include "lib/parser.h"

namespace linlib {


enum TK
{
    /* in this enum, operators appear sorted by operator precedence descending */
    
    /* litterals */
    TK_NUM = 0,

    /* operators */
    TK_LPAR,
    TK_MUL,
    TK_PLUS,
};

class Token
{
    public:
    const TK _tk;
    const int _pos;
    int _len;

    Token(TK tk, unsigned pos = -1) : _tk(tk), _pos(pos), _len(0) {}
};

void reduce(const std::string& expr, std::stack<Token>& stk, EventHandler& handler)
{
    std::cout << "reduce" << std::endl;

    while(true)
    {
        Token top = stk.top();
        stk.pop();
        
        switch(top._tk)
        {
            case TK_LPAR:   return;
            case TK_NUM:
                std::cout << "TK_NUM " << expr.substr(top._pos, top._len) << std::endl;
                break;
            case TK_PLUS:
                std::cout << "TK_PLUS" << std::endl;
                break;
            case TK_MUL:
                std::cout << "TK_MUL" << std::endl;
                break;
        }
    }
}

char Parser::next()
{
    while(std::isspace(static_cast<unsigned char>(*_rest)))
      ++_rest;

    return *_rest;
}

bool Parser::expect(char c)
{
    if (next() != c)
    {
        _handler.bad_token_error();
        return false;
    }

    ++_rest;
    return true;
}

bool Parser::read_number()
{
    const char* start = _rest;
    char* end;

    double result = std::strtod(start, &end);
    
    /* TODO should check errno */
    if (end == start)
    {
        _handler.bad_token_error();
        return false;
    }

    _rest = end;
    _handler.handle_literal(result);
    return true;
}

/**
    term := number
            | '+' term
            | '(' expr ')'
*/
bool Parser::read_term()
{
    char n = next();

    switch(n)
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return read_number();
        case '+':
            ++_rest;
            return read_term();
        case '(':
            ++_rest;
            return read_expr() && expect(')');

        default:
            _handler.bad_token_error();
            return false;
    }
}

/**
    expr := term
            | term '+' expr
            | term '*' expr
*/
bool Parser::read_expr()
{
    return read_term();
/*
    char n = next();
    switch(n)
    {
        case '+':
            break;

        default:
            _handler.bad_token_error();
    }
*/
}

bool Parser::parse()
{
    return read_expr();
}

} /* namespace */

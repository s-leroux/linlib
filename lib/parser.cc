#include <iostream>
#include <cstdio>
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

void error_helper(const char* msg, const char* stmt, unsigned pos)
{
    std::fprintf(stderr,
      "%s:\n"
      "%s\n"
      "%*c\n",
      msg, stmt, pos+1, '^');
}

void EventHandler::bad_token_error(const char* stmt, unsigned pos)
{
    error_helper("Bad token error", stmt, pos);
}

void EventHandler::bad_literal_error(const char* stmt, unsigned pos)
{
    error_helper("Bad literal error", stmt, pos);
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
        _handler.bad_token_error(_start, _rest-_start);
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
        _handler.bad_token_error(_start, _rest-_start);
        return false;
    }

    _rest = end;
    return _handler.handle_literal(result);
}

bool Parser::read_identifier()
{
    const char* curr = _rest;

    if (*curr != '_' && !std::isalpha((unsigned int)*curr))
        return false;

    for(;;)
    {
        ++curr;
        if (*curr != '_' && !std::isalnum((unsigned int)*curr))
        {
            _rest = curr;
            return true;
        }

    }
}


/**
    call := identifier '(' expr ')'
*/
bool Parser::read_call()
{
    next(); // skip potential white spaces -- it does not cost much

    const char*   id_start = _rest;
    if (!read_identifier())
        return false;

    const char* id_end = _rest;
    return expect('(') && read_expr() && expect(')') && _handler.handle_call(id_start, id_end-id_start);
}

/**
    term := number
            | '+' term
            | '(' expr ')'
            | call
*/
bool Parser::read_term()
{
    char n = next();

    if (n == '(')
    {
        ++_rest;
        return read_expr() && expect(')');
    }
    else if (n == '+')
    {
        ++_rest;
        return read_term();
    }
    else if ((n & 0x80)==0)
    {
        if (std::isdigit(n))
            return read_number();
        else
            return read_call();
    }

    _handler.bad_token_error(_start, _rest-_start);
    return false;
}

/**
    prod := term [ '*' term ]*
*/
bool Parser::read_prod()
{
    if (!read_term())
        return false;

    while(true)
    {
        char n = next();

        if (n=='*')
        {
            ++_rest;
            if (read_term() and _handler.handle_product())
                continue;
        }
        else if (n=='/')
        {
            ++_rest;
            if (read_term() and _handler.handle_division())
                continue;
        }
        else
            return true;

        // otherwise
        return false;
    }
}

/**
    sum := prod [ '+'|'-' prod ]*
*/
bool Parser::read_sum()
{
    if (!read_prod())
        return false;

    while(true)
    {
        char n = next();

        if (n=='+')
        {
            ++_rest;
            if (read_prod() and _handler.handle_sum())
                continue;
        }
        else if (n=='-')
        {
            ++_rest;
            if (read_prod() and _handler.handle_difference())
                continue;
        }
        else
            return true;

        // otherwise
        return false;
    }
}

/**
    expr := sum
*/
bool Parser::read_expr()
{
    return read_sum();
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

    return read_expr() && expect('\00');
}

} /* namespace */

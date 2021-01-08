#include <iostream>
#include <cstdio>
#include <stack>
#include <cstdlib>

#include "lib/parser.h"

namespace linlib {

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

void EventHandler::syntax_error(const char* stmt, unsigned pos)
{
    error_helper("Syntax error", stmt, pos);
}

char Parser::next()
{
    while(std::isspace(static_cast<unsigned char>(*_rest)))
      ++_rest;

    return *_rest;
}

bool Parser::bad_token_error()
{
    _handler.bad_token_error(_start, _rest-_start);
    _state = BAD_TOKEN_ERROR;

    return false;
}

bool Parser::syntax_error()
{
    _handler.syntax_error(_start, _rest-_start);
    _state = SYNTAX_ERROR;

    return false;
}

bool Parser::expect(char c)
{
    if (next() != c)
        return syntax_error();

    ++_rest;
    return true;
}

bool Parser::read_number()
{
    const char* start = _rest;
    char* end;

    double result = std::strtod(start, &end);

    /* TODO should check errno to return overflow error */
    if (*end & 0x80)
        return bad_token_error();
    else if (end == start)
        return syntax_error();

    _rest = end;
    return _handler.handle_literal(result);
}

bool Parser::read_identifier()
{
    const char* curr = _rest;

    if (*curr & 0x80)
        return bad_token_error();

    else if (*curr != '_' && !std::isalpha((unsigned int)*curr))
        return syntax_error();

    for(;;)
    {
        ++curr;
        if (*curr & 0x80)
            return bad_token_error();
        else if (*curr != '_' && !std::isalnum((unsigned int)*curr))
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

    // n is outside the 0-127 range
    return bad_token_error();
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
    class Monitor
    {
        State& _state;
        State  _end_state = INTERNAL_ERROR;

        public:
        Monitor(State &state)
          : _state(state), _end_state(INTERNAL_ERROR)
        {
            _state = RUNNING;
        }

        ~Monitor()
        {
            if (_state == RUNNING)
                _state = _end_state;
        }

        inline bool done(void) { _end_state = OK; return true; }
    };

    Monitor   monitor{_state};

    return read_expr() && expect('\00') && monitor.done();
}

//========================================================================
//  Diagnostic
//========================================================================
std::string   Parser::where() const
{
    const std::size_t LINE_LEN = 76;
    char    line1[LINE_LEN];
    char    line2[LINE_LEN];

    std::snprintf(line1, LINE_LEN, "%s\n", _start);
    std::snprintf(line2, LINE_LEN, "%*c\n", (int)(_rest-_start+1), '^');

    return std::string(line1) + line2;
}

std::string   Parser::what() const
{
    static const char*  tbl[] = {
        "",
        "running",
        "internal error",
        "bad token error",
        "syntax error",
    };

    return tbl[_state];
}

std::string   Parser::message() const
{
    return what() + ":\n" + where();
}

} /* namespace */

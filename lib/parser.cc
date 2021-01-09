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

bool Parser::bad_token_error()
{
    _handler.bad_token_error(_start, static_cast<std::size_t>(_lookahead.start-_start));
    _state = BAD_TOKEN_ERROR;

    return false;
}

bool Parser::syntax_error()
{
    _handler.syntax_error(_start, static_cast<std::size_t>(_lookahead.start-_start));
    _state = SYNTAX_ERROR;

    return false;
}

bool Parser::next()
{
    _lookahead = _tokenizer.next();
    if (_lookahead.id == Token::BAD_TOKEN)
        return bad_token_error();

    return true;
}

bool Parser::expect(char tk)
{
    if (_lookahead.id != static_cast<Token::Id>(tk))
        return syntax_error();

    return next();
}

bool Parser::read_number()
{
    char *end;
    double result = std::strtod(_lookahead.start, &end);

    /* TODO should check errno to return overflow error */
    if (static_cast<std::size_t>(end-_lookahead.start) != _lookahead.length)
        return syntax_error(); // XXX Should return an internal_error instead

    return next() && _handler.handle_literal(result);
}


/**
    call := identifier '(' expr ')'
*/
bool Parser::read_call()
{
    const Token head = _lookahead;
    // XXX Check if lookhead.id is really a SYMBOL

    return next() && expect('(') && read_expr() && expect(')') && _handler.handle_call(head.start, head.length);
}

/**
    term := number
            | '+' term
            | '(' expr ')'
            | call
*/
bool Parser::read_term()
{
    if (_lookahead.id == '(')
    {
        return next() && read_expr() && expect(')');
    }
    else if (_lookahead.id == '+')
    {
        return next() && read_term();
    }
    else if (_lookahead.id == Token::NUMBER)
    {
        return read_number();
    }
    else if (_lookahead.id == Token::SYMBOL)
    {
        return read_call();
    }

    return syntax_error();
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
        if (_lookahead.id == '*')
        {
            if (next() && read_term() && _handler.handle_product())
                continue;
        }
        else if (_lookahead.id=='/')
        {
            if (next() && read_term() && _handler.handle_division())
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
        if (_lookahead.id=='+')
        {
            if (next() && read_prod() && _handler.handle_sum())
                continue;
        }
        else if (_lookahead.id=='-')
        {
            if (next() && read_prod() && _handler.handle_difference())
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

    return next() && read_expr() && expect('\00') && monitor.done();
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
    std::snprintf(line2, LINE_LEN, "%*c\n", static_cast<int>(_lookahead.start - _start), '^');

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

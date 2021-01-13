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

//========================================================================
//  Engine
//========================================================================
class ParserEngine
{
    private:
    Parser::State&          _state;
    const char*             _start;
    Tokenizer               _tokenizer;
    Token&                  _lookahead;

    EventHandler&           _handler;

    /**
        Report a bad token to the event handler.
        Change the state of the parser.

        A bad token is an unexpected character in the input stream. Only
        characters in the 7-bits ASCII range are allowed in an expression.
    */
    bool  bad_token_error()
    {
        _handler.bad_token_error(_start, static_cast<std::size_t>(_lookahead.start-_start));
        _state = Parser::BAD_TOKEN_ERROR;

        return false;
    }

    /**
        Report a syntax error to the event handler.
        Change the state of the parser.

        Syntax errors occur when an expression does not follow the
        grammar rules for the language.
    */
    bool  syntax_error()
    {
        _handler.syntax_error(_start, static_cast<std::size_t>(_lookahead.start-_start));
        _state = Parser::SYNTAX_ERROR;

        return false;
    }

    /**
        Read the next token from the input stream.

        Raise a bad_token_error is the next token was not recognized
        by the tokenizer.
    */
    bool   next()
    {
        _lookahead = _tokenizer.next();
        if (_lookahead.id == Token::BAD_TOKEN)
            return bad_token_error();

        return true;
    }

    /**
      Consume the given token if found in the stream, otherwise
      report an error.
    */
    bool expect(Token::Id id)
    {
        if (_lookahead.id != id)
            return syntax_error();

        return next();
    }

    /**
        Parse a number.

        Raise a sytax_error if the token can't be converted to
        a double.
    */
    bool read_number()
    {
        char *end;
        double result = std::strtod(_lookahead.start, &end);

        /* TODO should check errno to return overflow error */
        if (static_cast<std::size_t>(end-_lookahead.start) != _lookahead.length)
            return syntax_error(); // XXX Should return an internal_error instead

        return next() && _handler.handle_literal(result);
    }



    /**
        call := SYMBOL '(' expr ')'
                | SYMBOL
    */
    bool read_call()
    {
        const Token symbol = _lookahead;
        // XXX Check if lookhead.id is really a SYMBOL

        if (!next())
            return false;

        if (_lookahead.id == Token::LPAR)
            return next() && read_expr() && expect(Token::RPAR) && _handler.call(symbol.start, symbol.length);
        else
            return _handler.load(symbol.start, symbol.length);
    }

    /**
        term := NUMBER
                | '+' term
                | '-' term
                | '(' expr ')'
                | call'
    */
    bool read_term()
    {
        if (_lookahead.id == '(')
        {
            return next() && read_expr() && expect(Token::RPAR);
        }
        else if (_lookahead.id == '+')
        {
            return next() && read_term();
        }
        else if (_lookahead.id == '-')
        {
            return next() && read_term() && _handler.unary_op(UnaryOpCode::NEG);
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
    bool read_pow()
    {
        if (!read_term())
            return false;

        while(true)
        {
            if (_lookahead.id == Token::POW)
            {
                if (next() && read_term() && _handler.binary_op(BinaryOpCode::POW))
                    continue;
            }
            else
                return true;

            // otherwise
            return false;
        }
    }

    /**
        prod := pow [ '*' pow ]*
    */
    bool read_prod()
    {
        if (!read_pow())
            return false;

        while(true)
        {
            if (_lookahead.id == '*')
            {
                if (next() && read_pow() && _handler.binary_op(BinaryOpCode::MUL))
                    continue;
            }
            else if (_lookahead.id == '/')
            {
                if (next() && read_pow() && _handler.binary_op(BinaryOpCode::DIV))
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
    bool read_sum()
    {
        if (!read_prod())
            return false;

        while(true)
        {
            if (_lookahead.id=='+')
            {
                if (next() && read_prod() && _handler.binary_op(BinaryOpCode::ADD))
                    continue;
            }
            else if (_lookahead.id=='-')
            {
                if (next() && read_prod() && _handler.binary_op(BinaryOpCode::SUB))
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
    bool read_expr()
    {
        return read_sum();
    }

    public:
    ParserEngine(Parser::State& state, const char* start, Token& lookahead, EventHandler& eh)
      : _state(state),
        _start(start),
        _tokenizer(start),
        _lookahead(lookahead),
        _handler(eh)
    {
    }

    bool  parse()
    {
        class Monitor
        {
            Parser::State& _state;
            Parser::State  _end_state = Parser::INTERNAL_ERROR;

            public:
            Monitor(Parser::State &state)
              : _state(state), _end_state(Parser::INTERNAL_ERROR)
            {
                _state = Parser::RUNNING;
            }

            ~Monitor()
            {
                if (_state == Parser::RUNNING)
                    _state = _end_state;
            }

            inline bool done(void)
            {
                _end_state = Parser::OK;
                return true;
            }
        };

        Monitor   monitor{_state};
        return next() && read_expr() && expect(Token::END) && monitor.done();
    }

}; // class ParserEngine


//========================================================================
//  Public interface
//========================================================================
bool Parser::parse()
{
    Tokenizer tokenizer{_start};

    ParserEngine  engine{_state, _start, _lookahead, _handler};
    return engine.parse();
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

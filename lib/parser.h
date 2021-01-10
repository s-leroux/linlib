#if !defined LINLIB_PARSER_H
#define LINLIB_PARSER_H

#include <stack>

#include "lib/tokenizer.h"

namespace linlib {

class EventHandler
{
    protected:
    virtual void error(const char* str) const { throw str; }

    public:
    virtual ~EventHandler(void) {}

    virtual bool handle_literal(double value) = 0;
    virtual bool handle_call(const char *identifier, std::size_t len) = 0;
    virtual bool handle_product() = 0;
    virtual bool handle_division() = 0;
    virtual bool handle_sum() = 0;
    virtual bool handle_difference() = 0;
    virtual bool handle_pow() = 0;

    virtual void bad_token_error(const char* stmt, unsigned pos);
    virtual void syntax_error(const char* stmt, unsigned pos);
};

void parse(const std::string& expr, EventHandler& handler);

class Parser
{
    public:
    enum State
    {
        OK,
        RUNNING,
        INTERNAL_ERROR,
        BAD_TOKEN_ERROR,
        SYNTAX_ERROR,
    };

    private:
    State                   _state;
    const char*             _start;
    Tokenizer               _tokenizer;
    Token                   _lookahead = { Token::END, "", 0 };

    EventHandler&           _handler;

    /**
        Report a bad token to the event handler.
        Change the state of the parser.

        A bad token is an unexpected character in the input stream. Only
        characters in the 7-bits ASCII range are allowed in an expression.
    */
    bool  bad_token_error();

    /**
        Report a syntax error to the event handler.
        Change the state of the parser.

        Syntax errors occurs when a construct does not follow the
        grammar rules for expressions.
    */
    bool  syntax_error();

    public:
    Parser(const char* expr, EventHandler& handler)
      : _state(OK),
        _start(expr),
        _tokenizer(expr),
        _handler(handler)
    {
    }

    bool   next();

    /**
      Consume the given token if found in the stream, otherwise
      report an error.
    */
    bool expect(char tk);

    bool read_identifier();
    bool read_number();

    bool read_sum();
    bool read_prod();
    bool read_pow();

    bool read_call();
    bool read_term();
    bool read_expr();

    /**
        Parse the expression. Stops at the first error and return false.
        Return true is the expression was successfully parsed.

        The parser is a stateful object. The parse method is _not_
        reentrant.
    */
    bool parse();

    /*
        Diagnostic info about the lattest `parse()` invocation.
     */

    /**
        Parser curent state
    */
    inline State state(void) const { return _state; }

    /**
        Return a string-representation of state(). For human consumption only.
     */
    std::string  what() const;

    /**
        Return a formatted string identifying the last parsed code
        fragment. For human consumption only.
     */
    std::string  where() const;

    /**
        Return a formatted string describing the current state of the parser.
        The message is garanteed to contain both what() and where(), but
        the order is unspecified. The message may also contain additional
        informations. For human consumption only.
     */
    std::string  message() const;
};

} /* namespace */

#endif

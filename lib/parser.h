#if !defined LINLIB_PARSER_H
#define LINLIB_PARSER_H

#include <stack>

#include "lib/tokenizer.h"

namespace linlib {

enum struct BinaryOpCode
{
    ADD,
    SUB,
    MUL,
    DIV,

    POW,
};

enum struct UnaryOpCode
{
    NEG,
};

class EventHandler
{
    public:

    protected:
    virtual void error(const char* str) const { throw str; }

    public:
    virtual ~EventHandler(void) {}

    virtual bool number(double value) = 0;
    virtual bool call(const char *identifier, std::size_t len) = 0;
    virtual bool load(const char *identifier, std::size_t len) = 0;
    virtual bool binary_op(BinaryOpCode opcode) = 0;
    virtual bool unary_op(UnaryOpCode opcode) = 0;

    virtual void bad_token_error(const char* stmt, unsigned pos);
    virtual void syntax_error(const char* stmt, unsigned pos);
};

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
    Token                   _lookahead;

    EventHandler&           _handler;

    public:
    Parser(const char* expr, EventHandler& handler)
      : _state(OK),
        _start(expr),
        _lookahead{ Token::END, "", 0 },
        _handler(handler)
    {
    }

    bool   next();

    /**
      Consume the given token if found in the stream, otherwise
      report an error.
    */
    bool expect(Token::Id id);

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

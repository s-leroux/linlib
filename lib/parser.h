#if !defined LINLIB_PARSER_H
#define LINLIB_PARSER_H

#include <stack>

namespace linlib {

class EventHandler
{
    public:
    virtual ~EventHandler(void) {}

    virtual void handle_literal(double value) = 0;

    virtual void bad_token_error() { throw "Bad token error"; }
    virtual void bad_literal_error() { throw "Bad literal error"; }
};

class SampleEventHandler : public EventHandler
{
    public:
    virtual void handle_literal(double value) {};
};

/**
    Parse an expression.

    This is an event-based parser that will call the relevant
    enter_* method of the handler.
*/
void parse(const std::string& expr, EventHandler& handler);

class Parser
{
    private:
    const char*             _rest;

    EventHandler&           _handler;
    std::stack<void (Parser::*)()>  _prod;

    public:
    Parser(const std::string& expr, EventHandler& handler) : _rest(expr.c_str()), _handler(handler) {}

    /**
      Advance to the next character of the input, skipping white spaces
    */
    char next();

    void read_number();

    void read_expr();

    void parse();


};

} /* namespace */

#endif

#if !defined LINLIB_PARSER_H
#define LINLIB_PARSER_H

#include <stack>

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

    virtual void bad_token_error(const char* stmt, unsigned pos);
    virtual void bad_literal_error(const char* stmt, unsigned pos);
};

void parse(const std::string& expr, EventHandler& handler);

class Parser
{
    private:
    const char*             _start;
    const char*             _rest;

    EventHandler&           _handler;

    public:
    Parser(const char* expr, EventHandler& handler) : _start(expr), _rest(expr), _handler(handler) {}

    /**
      Advance to the next character of the input, skipping white spaces
    */
    char next();

    /**
      Consume the given character if found in the stream, otherwise
      report an error.
    */
    bool expect(char c);

    bool read_identifier();
    bool read_number();

    bool read_sum();
    bool read_prod();

    bool read_call();
    bool read_term();
    bool read_expr();

    bool parse();


};

} /* namespace */

#endif

#if !defined LINLIB_TOKENIZER_H
#define LINLIB_TOKENIZER_H

namespace linlib
{

struct Token
{
    enum Id
    {
        BAD_TOKEN       = -1,
        END             = 0,

        PLUS            = '+',
        MINUS           = '-',
        SLASH           = '/',
        TIMES           = '*',

        LPAR            = '(',
        RPAR            = ')',

        SYMBOL          = 256,
        NUMBER,
    };

    /*const*/ Id          id;

    const char* /*const*/ start;
    /*const*/ std::size_t length;

    inline operator bool() const { return id != END; }
};

class Tokenizer
{
    const char* _rest;

    Token bad_token();
    Token symbol();
    Token number();
    Token operator1();

    public:
    Tokenizer(const char* expr) : _rest(expr) {}

    Token next();
};

} // namespace

#endif

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

        LPAR            = '(',
        RPAR            = ')',

        PLUS            = '+',
        MINUS           = '-',
        SLASH           = '/',
        TIMES           = '*',

        POW             = 0x0100,

        SYMBOL          = 0x0200,
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

    Token token1(Token::Id id);
    Token token2(Token::Id id);

    public:
    Tokenizer(const char* expr) : _rest(expr) {}

    Token next();
};

} // namespace

#endif

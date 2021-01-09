#include <cctype>

#include "lib/tokenizer.h"

namespace linlib
{

static int isalpha(char c)
{
    return (c ^ 0x80) && std::isalpha(static_cast<unsigned char>(c));
}

static int isalnum(char c)
{
    return (c ^ 0x80) && std::isalnum(static_cast<unsigned char>(c));
}

/**
    Emit a token matching a 1-character wide operator
*/
Token   Tokenizer::operator1()
{

    const char *curr = _rest++;

    // the _trick_ here is the Token::Id enum uses character code for 1-character
    // wide tokens.
    return { (Token::Id)*curr, curr, 1 };
}

Token   Tokenizer::bad_token()
{
    const char *start = _rest;

    while(*++_rest & 0x80)
    {
        // nothing
    }

    return { Token::BAD_TOKEN, start, static_cast<std::size_t>(_rest-start) };
}

Token   Tokenizer::number()
{
    const char *start = _rest;
    const char *curr = _rest;

    while(isdigit(*curr))
        ++curr;

    if (*curr=='.')
        ++curr;

    while(isdigit(*curr))
        ++curr;


    if (*curr=='e' || *curr=='E')
    {
        ++curr;
        if (*curr=='+' || *curr=='-')
            ++curr;

        while(isdigit(*curr))
            ++curr;
    }

    _rest = curr;

    return { Token::NUMBER, start, static_cast<std::size_t>(curr-start) };
}

Token   Tokenizer::symbol()
{
    const char *start = _rest;
    const char *curr = _rest;

    while(*curr == '_' || isalnum(*curr))
        ++curr;

    _rest = curr;

    return { Token::SYMBOL, start, static_cast<std::size_t>(curr-start) };
}

Token   Tokenizer::next()
{
    while(std::isspace(static_cast<unsigned char>(*_rest)))
      ++_rest;

    if (!*_rest)
        return { Token::END, _rest, 0 };
    else if (*_rest == '_' || isalpha(*_rest))
        return symbol();
    else if (*_rest == '.' || isdigit(*_rest))
        return number();
    else if (*_rest == '+' || *_rest == '-' || *_rest == '/' || *_rest == '*')
        return operator1();
    else
        return bad_token();


}

} // namespace

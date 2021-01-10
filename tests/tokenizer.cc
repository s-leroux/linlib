/*
 *
 *  Tests for the tokenizer
 *
 */
#include <iostream>
#include <cstdio>
#include <cstring>

#include "gtest/gtest.h"
#include "lib/tokenizer.h"


// ========================================================================
//  Helpers
// ========================================================================
void test_tokens(const char* testcase, std::initializer_list<linlib::Token::Id> ids)
{
    linlib::Tokenizer tokenizer{testcase};

    for(auto id : ids)
    {
        auto token = tokenizer.next();

        fprintf(stderr, "%d/%d %.*s\n", token.id, id, (int)token.length, token.start);
        EXPECT_EQ(token.id, id) << testcase;
    }
}

// ========================================================================
//  Tokenize!
// ========================================================================
TEST(Parser, reject_multibyte_chars) {
    using Token = linlib::Token;

    test_tokens(" xxøxx 4", { Token::SYMBOL, Token::BAD_TOKEN, Token::SYMBOL, Token::NUMBER, Token::END });
}

TEST(Parser, numbers) {
    using Token = linlib::Token;

    const char* testcases[] = {
        "0123456789",
        "1e+4",
        "1E+4",
        ".1E+4",
        "1E-4",
        "1e-4",
    };

    for(auto testcase : testcases)
        test_tokens(testcase, {
            Token::NUMBER,
            Token::END,
        });
}

TEST(Parser, core_tokens) {
    using Token = linlib::Token;

    test_tokens(" xxxxx 4", { Token::SYMBOL, Token::NUMBER, Token::END });
}

TEST(Parser, operators) {
    using Token = linlib::Token;

    test_tokens("1+2 *-3/5**", {
        Token::NUMBER, Token::PLUS,
        Token::NUMBER, Token::TIMES, Token::MINUS,
        Token::NUMBER, Token::SLASH,
        Token::NUMBER, Token::POW,
        Token::END,
    });
}

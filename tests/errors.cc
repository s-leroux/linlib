/*
 *
 *  Tests for expected failures and parsing error
 *
 */
#include <iostream>
#include <cstring>

#include "gtest/gtest.h"
#include "lib/parser.h"


// ========================================================================
//  Helpers
// ========================================================================
struct NEH : public linlib::EventHandler
{
    enum
    {
        NOTHING     = 0x0000,
        CALL        = 0x0001,
        LITERAL     = 0x0002,
        PRODUCT     = 0x0004,
        DIVISION    = 0x0008,
        SUM         = 0x0010,
        DIFFERENCE  = 0x0020,
        POW         = 0x0040,
    };

    int   mode;

    NEH(int mode) : mode(mode) {}

    bool handle_call(const char *identifier, std::size_t len)
    {
        return !(mode & CALL);
    }

    bool handle_literal(double v)
    {
        return !(mode & LITERAL);
    }

    bool handle_operator(linlib::OpCode opcode)
    {
        switch(opcode)
        {
            case linlib::OpCode::ADD:
                return !(mode & SUM);
            case linlib::OpCode::SUB:
                return !(mode & DIFFERENCE);
            case linlib::OpCode::MUL:
                return !(mode & PRODUCT);
            case linlib::OpCode::DIV:
                return !(mode & DIVISION);
            case linlib::OpCode::POW:
                return !(mode & POW);
        };

        return false;
    }
};

void test_no_error(int mode, const char* testcase)
{
    NEH eh{mode};

    linlib::Parser  parser{testcase, eh};

    ASSERT_TRUE(parser.parse());
    EXPECT_EQ(parser.state(), linlib::Parser::OK);
}

void test_internal_error(int mode, const char* testcase)
{
    NEH eh{mode};

    linlib::Parser  parser{testcase, eh};

    ASSERT_FALSE(parser.parse()) << testcase << " (" << mode << ")";
    EXPECT_EQ(parser.state(), linlib::Parser::INTERNAL_ERROR);
}

void test_bad_token_error(const char* testcase)
{
    NEH eh{NEH::NOTHING};

    linlib::Parser  parser{testcase, eh};

    ASSERT_FALSE(parser.parse()) << testcase;
    EXPECT_EQ(parser.state(), linlib::Parser::BAD_TOKEN_ERROR);
}

void test_syntax_error(const char* testcase)
{
    NEH eh{NEH::NOTHING};

    linlib::Parser  parser{testcase, eh};

    ASSERT_FALSE(parser.parse()) << testcase;
    EXPECT_EQ(parser.state(), linlib::Parser::SYNTAX_ERROR);
}

void test_error_message(const char* testcase, std::initializer_list<const char*> keywords)
{
    NEH eh{NEH::NOTHING};

    linlib::Parser  parser{testcase, eh};

    ASSERT_FALSE(parser.parse()) << testcase;

    std::string what = parser.what();
    std::string where = parser.where();
    std::string message = parser.message();

    EXPECT_TRUE(std::strstr(message.c_str(), what.c_str()));
    EXPECT_TRUE(std::strstr(message.c_str(), where.c_str()));

    for(const char *keyword : keywords)
    {
        EXPECT_TRUE(std::strstr(what.c_str(), keyword));
    }
}

// ========================================================================
//  Errors in the event handler
// ========================================================================
TEST(Parser, internal_error_1) {
    test_internal_error(NEH::LITERAL, " xxxxx(4)");
    test_internal_error(NEH::CALL, " xxxxx(4)");
    test_no_error(NEH::NOTHING, " xxxxx(4)");
}

TEST(Parser, internal_error_2) {
    test_internal_error(NEH::PRODUCT, "1+2*3/4-5");
    test_internal_error(NEH::DIVISION, "1+2*3/4-5");
    test_internal_error(NEH::SUM, "1+2*3/4-5");
    test_internal_error(NEH::DIFFERENCE, "1+2*3/4-5");
    test_no_error(NEH::NOTHING, "1+2*3/4-5");
}

// ========================================================================
//  Parser detected errors
// ========================================================================
TEST(Parser, bad_token) {
    test_bad_token_error(" côs(4)");
    test_bad_token_error(" 1+ø");
    test_bad_token_error(" 10ø");
    test_bad_token_error("2\\4");
}

TEST(Parser, syntax_error) {
    test_syntax_error("2*/4");
    test_syntax_error("cos(2)(3)");
    test_syntax_error("cos(2))");
    test_syntax_error("*2");
    test_syntax_error("2* *4"); // do not mistaken '* *' for '**'
}

// ========================================================================
//  Error messages
// ========================================================================
TEST(Parser, bad_token_message) {
    test_error_message(" côs(4)", { "token", "error" });
    test_error_message("2*/4", { "syntax", "error" });
}


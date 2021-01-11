#include <array>
#include <cmath>
#include <algorithm>

#include "gtest/gtest.h"
#include "lib/parser.h"

// ========================================================================
//  Helpers
// ========================================================================
struct EH : public linlib::EventHandler
{
    static const std::size_t STACK_SIZE = 128;
    typedef std::array<double, STACK_SIZE>  Stack;

    Stack           _stack;
    Stack::iterator _sp = _stack.begin();

    struct {
        const char* name;
        double (*call)(double);
    } fcts[1] = {
        { "sqrt", std::sqrt },
    };

    bool push(double v)
    {
        *_sp++ = v;
        return true;
    }

    double pop()
    {
        return *--_sp;
    }

    bool handle_call(const char *identifier, std::size_t len)
    {
        std::string fname(identifier, len);
        auto fct = std::find_if(std::begin(fcts), std::end(fcts),
            [&fname](auto &v) { return v.name == fname; }
        );
        if (fct == std::end(fcts))
            return false;

        push(fct->call(pop()));

        return true;
    }

    bool handle_literal(double v)
    {
        push(v);
        return true;
    }

    bool handle_product()
    {
        push(pop() * pop());
        return true;
    }

    bool handle_unary_operator(linlib::UnaryOpCode opcode)
    {
        double x = pop();

        switch(opcode)
        {
            case linlib::UnaryOpCode::NEG:
                return push(-x);
        };

        return false;
    }

    bool handle_binary_operator(linlib::BinaryOpCode opcode)
    {
        double b = pop(),
               a = pop();

        switch(opcode)
        {
            case linlib::BinaryOpCode::ADD:
                return push(a+b);
            case linlib::BinaryOpCode::SUB:
                return push(a-b);
            case linlib::BinaryOpCode::MUL:
                return push(a*b);
            case linlib::BinaryOpCode::DIV:
                return push(a/b);
            case linlib::BinaryOpCode::POW:
                return push(std::pow(a,b));
        };

        return false;
    }
};

void test(const char* testcase, double expected)
{
    EH eh;
    linlib::Parser  parser{testcase, eh};

    ASSERT_TRUE(parser.parse()) << testcase << " == " << expected;
    EXPECT_EQ(eh.pop(), expected) << testcase << " == " << expected;
}



// ========================================================================
//  Tests
// ========================================================================
TEST(Parser, should_pass) {
    test(" 1 ", 1.0);
    test(" 1 + 2 ", 3.0);
    test(" 1 + 2*3 ", 7.0);
    test(" 1 * 2-3 ", -1.0);
    test(" 1 * 2/4 ", 0.5);
    test(" sqrt(4)", 2);
    test(" 1+2**3", 9);
}

TEST(Parser, should_fail) {
    const char* testcase = " xxxxx(4)";

    EH eh;
    linlib::Parser  parser{testcase, eh};

    ASSERT_FALSE(parser.parse());
    EXPECT_EQ(parser.state(), linlib::Parser::INTERNAL_ERROR);
}

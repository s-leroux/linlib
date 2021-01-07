#include "gtest/gtest.h"
#include "lib/parser.h"


struct EH : public linlib::EventHandler
{
    std::string  _stack;

    template<class T>
    void push(const T& v) { _stack = _stack + std::to_string(v) + ";"; }

    void push(const char* v) { _stack = _stack + v + ";"; }

    bool handle_literal(double v)
    {
      push(v);
      return true;
    }

    bool handle_product()
    {
      push("MUL");
      return true;
    }

    bool handle_sum()
    {
      push("ADD");
      return true;
    }
};

TEST(Parser, parse_number) {
  EH eh;
  linlib::Parser  parser{" 123  ", eh};

  ASSERT_TRUE(parser.parse());
  EXPECT_EQ(eh._stack, "123.000000;");
}

TEST(Parser, parse_number_with_unary_plus) {
  EH eh;
  linlib::Parser  parser{"  +123  ", eh};

  ASSERT_TRUE(parser.parse());
  EXPECT_EQ(eh._stack, "123.000000;");
}

TEST(Parser, parse_product)
{
  const char*  testcases[] = {
    "12*3",
    "12 * 3",
    "+ 12 * + 3",
  };

  for(auto testcase : testcases)
  {
      EH eh;
      linlib::Parser  parser{testcase, eh};

      ASSERT_TRUE(parser.parse());

      EXPECT_EQ(eh._stack,
        "12.000000;"
        "3.000000;"
        "MUL;"
      );
  }
}

TEST(Parser, parse_sum)
{
  const char*  testcases[] = {
    "12+3",
    "12 + 3",
    "+ 12 + + 3",
  };

  for(auto testcase : testcases)
  {
      EH eh;
      linlib::Parser  parser{testcase, eh};

      ASSERT_TRUE(parser.parse());

      EXPECT_EQ(eh._stack,
        "12.000000;"
        "3.000000;"
        "ADD;"
      );
  }
}

TEST(Parser, parse_parenthesis_1) {
  EH eh;
  linlib::Parser  parser{" ( 123 ) ", eh};

  ASSERT_TRUE(parser.parse());
  EXPECT_EQ(eh._stack, "123.000000;");
}

TEST(Parser, parse_parenthesis_2) {
  EH eh;
  linlib::Parser  parser{" ( ( 123 ) ) ", eh};

  ASSERT_TRUE(parser.parse());
  EXPECT_EQ(eh._stack, "123.000000;");
}

TEST(Parser, parse_parenthesis_3) {
  EH eh;
  linlib::Parser  parser{" ( + ( 123 ) ) ", eh};

  ASSERT_TRUE(parser.parse());
  EXPECT_EQ(eh._stack, "123.000000;");
}


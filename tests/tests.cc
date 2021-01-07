#include "gtest/gtest.h"
#include "lib/parser.h"

// ========================================================================
//  Helpers
// ========================================================================
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

template<std::size_t N>
void test(const char* (&testcases)[N], const char* expected)
{
  for(auto testcase : testcases)
  {
      EH eh;
      linlib::Parser  parser{testcase, eh};

      ASSERT_TRUE(parser.parse());

      EXPECT_EQ(eh._stack, expected);
  }

}



// ========================================================================
//  Atoms
// ========================================================================
TEST(Parser, parse_number) {
  const char*  testcases[] = {
        "123",
        " 123  ",
        " +123  ",
        " + 123  ",
  };

  test(testcases,
    "123.000000;"
  );
}

// ========================================================================
//  Binary operators
// ========================================================================
TEST(Parser, parse_product)
{
  const char*  testcases[] = {
    "12*3",
    "12 * 3",
    "+ 12 * + 3",
  };

  test(testcases,
    "12.000000;"
    "3.000000;"
    "MUL;"
  );
}

TEST(Parser, parse_sum)
{
  const char*  testcases[] = {
    "12+3",
    "12 + 3",
    "+ 12 + + 3",
  };

  test(testcases,
    "12.000000;"
    "3.000000;"
    "ADD;"
  );
}


// ========================================================================
//  Parenthesis
// ========================================================================
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


// ========================================================================
//  Associativity
// ========================================================================
TEST(Parser, assoc_1)
{
  const char*  testcases[] = {
    "12 + 3 + 4 + 5",
    "(12 + 3) + 4 + 5",
    "((12 + 3) + 4 )+ 5",
  };

  test(testcases,
        "12.000000;"
        "3.000000;"
        "ADD;"
        "4.000000;"
        "ADD;"
        "5.000000;"
        "ADD;"
  );
}

// ========================================================================
//  Operator precedence
// ========================================================================
TEST(Parser, precedence_1)
{
  const char*  testcases[] = {
    "12 + 3 * 4",
    "12 + ( 3 * 4 )",
  };

  test(testcases,
        "12.000000;"
        "3.000000;"
        "4.000000;"
        "MUL;"
        "ADD;"
  );
}

TEST(Parser, precedence_2)
{
  const char*  testcases[] = {
    "12 * 3 + 4",
    "(12 *  3) + 4",
  };

  test(testcases,
        "12.000000;"
        "3.000000;"
        "MUL;"
        "4.000000;"
        "ADD;"
  );
}

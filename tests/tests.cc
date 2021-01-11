#include "gtest/gtest.h"
#include "lib/parser.h"

// ========================================================================
//  Helpers
// ========================================================================
struct EH : public linlib::EventHandler
{
    std::string  _stack;

    template<class T>
    bool push(const T& v) { _stack = _stack + std::to_string(v) + ";"; return true; }
    bool push(const char* v) { _stack = _stack + v + ";"; return true; }

    template<std::size_t N>
    bool push(const char (&v)[N]) { _stack = _stack + v + ";"; return true; }

    bool handle_call(const char *identifier, std::size_t len)
    {
      char buffer[32];

      std::snprintf(buffer, sizeof(buffer)/sizeof(char), "CALL(%.*s)", (int)len, identifier);
      push(buffer);
      return true;
    }

    bool handle_literal(double v)
    {
      push(v);
      return true;
    }

    bool handle_unary_operator(linlib::UnaryOpCode opcode)
    {
      switch(opcode)
      {
          case linlib::UnaryOpCode::NEG:
              return push("NEG");
      };

      return false;
    }

    bool handle_binary_operator(linlib::BinaryOpCode opcode)
    {
      switch(opcode)
      {
          case linlib::BinaryOpCode::ADD:
              return push("ADD");
          case linlib::BinaryOpCode::SUB:
              return push("SUB");
          case linlib::BinaryOpCode::MUL:
              return push("MUL");
          case linlib::BinaryOpCode::DIV:
              return push("DIV");
          case linlib::BinaryOpCode::POW:
              return push("POW");

      };

      return false;
    }
};

template<std::size_t N>
void test(const char* (&testcases)[N], const char* expected)
{
  for(auto testcase : testcases)
  {
      EH eh;
      linlib::Parser  parser{testcase, eh};

      ASSERT_TRUE(parser.parse()) << testcase;

      EXPECT_EQ(eh._stack, expected) << testcase;
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

TEST(Parser, parse_division)
{
  const char*  testcases[] = {
    "12/3",
    "12 / 3",
    "+ 12 / + 3",
  };

  test(testcases,
    "12.000000;"
    "3.000000;"
    "DIV;"
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

TEST(Parser, parse_difference)
{
  const char*  testcases[] = {
    "12-3",
    "12 - 3",
    "+ 12 - + 3",
  };

  test(testcases,
    "12.000000;"
    "3.000000;"
    "SUB;"
  );
}


TEST(Parser, parse_pow)
{
  const char*  testcases[] = {
    "12**3",
    "12 ** 3",
    "+ 12 ** + 3",
  };

  test(testcases,
    "12.000000;"
    "3.000000;"
    "POW;"
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

TEST(Parser, assoc_2)
{
  const char*  testcases[] = {
    "12 - 3 + 4 - 5",
    "(12 - 3) + 4 - 5",
    "((12 - 3) + 4 )- 5",
  };

  test(testcases,
        "12.000000;"
        "3.000000;"
        "SUB;"
        "4.000000;"
        "ADD;"
        "5.000000;"
        "SUB;"
  );
}

TEST(Parser, assoc_3)
{
  const char*  testcases[] = {
    "12 * 3 * 4 * 5",
    "(12 * 3) * 4 * 5",
    "((12 * 3) * 4 )* 5",
  };

  test(testcases,
        "12.000000;"
        "3.000000;"
        "MUL;"
        "4.000000;"
        "MUL;"
        "5.000000;"
        "MUL;"
  );
}

TEST(Parser, assoc_4)
{
  const char*  testcases[] = {
    "12 / 3 * 4 / 5",
    "(12 / 3) * 4 / 5",
    "((12 / 3) * 4 )/ 5",
  };

  test(testcases,
        "12.000000;"
        "3.000000;"
        "DIV;"
        "4.000000;"
        "MUL;"
        "5.000000;"
        "DIV;"
  );
}

// ========================================================================
//  Operator precedence
// ========================================================================
TEST(Parser, precedence_1)
{
  const char*  testcases[] = {
    "12 + 3 ** 4 * 5",
    "12 + ( 3 ** 4 * 5)",
    "12 + ( ( 3 ** 4 ) * 5)",
  };

  test(testcases,
        "12.000000;"
        "3.000000;"
        "4.000000;"
        "POW;"
        "5.000000;"
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

// ========================================================================
//  Function call
// ========================================================================
TEST(Parser, funcall)
{
  const char*  testcases[] = {
    "cos(1+2)",
  };

  test(testcases,
        "1.000000;"
        "2.000000;"
        "ADD;"
        "CALL(cos);"
  );
}


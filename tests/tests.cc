#include "gtest/gtest.h"
#include "lib/parser.h"


struct EH : public linlib::EventHandler
{
    double _v;

    void handle_literal(double v) { _v = v; }
};

TEST(Parser, parse_number) {
  EH eh;
  linlib::Parser  parser{"123 456", eh};

  parser.parse();
  EXPECT_EQ(eh._v, 123.0);

  parser.parse();
  EXPECT_EQ(eh._v, 456.0);
}


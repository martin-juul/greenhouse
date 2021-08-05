#pragma once
#include <string>
#include <vector>

struct Row {
  std::string temperature;
  std::string dewity;
  std::string humidity;
};

class Database {
  public:
  Database();
  std::vector<Row> all();
  Row latest();
  void append(Row r);
  void flush();
};

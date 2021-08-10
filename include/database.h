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
  /** Get all entries
   *
   * @retval std::vector<Row>
   */
  std::vector<Row> all();
  /** Get the latest entry
   *
   * @retval Row
   */
  Row latest();
  /** Append a new row
   *
   * @param Row
   *
   * @retval void
   */
  void append(Row r);
  /** Reset the database
   *
   * @retval void
   */
  void flush();
};

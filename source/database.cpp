#include "database.h"
#include <vector>
#include <cstring>
#include <string>

std::vector<Row> store;

Database::Database() {
  store.begin();
}

std::vector<Row> Database::all() {
  return store;
}

Row Database::latest() {
  return store.front();
}

void Database::append(Row r) {
  printf("[database::append]: temp: %s | dew: %s | hum: %s", r.temperature.c_str(), r.dewity.c_str(), r.humidity.c_str());

  store.push_back(r);
}

void Database::flush() {
  store.empty();
}
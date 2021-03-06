﻿// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "tagged_sqlite.h"

int main() {
  sqlite3 *sqldb;
  sqlite3_open(":memory:", &sqldb);

  skydown::prepared_statement<
      "CREATE TABLE customers(id INTEGER NOT NULL PRIMARY KEY, name TEXT);"  //
      >{sqldb}
      .execute();

  skydown::prepared_statement<
      "CREATE TABLE orders(id INTEGER NOT NULL PRIMARY KEY, item TEXT, "
      "customerid INTEGER, price REAL);"  //
      >{sqldb}
      .execute();

  skydown::prepared_statement<
      "INSERT INTO customers(id, name) VALUES( ?id:int, "
      "?name:string);"  //
      >
      insert_customer{sqldb};

  using skydown::bind;
  insert_customer.execute(bind<"id">(1), bind<"name">("John"));

  skydown::prepared_statement<
      "INSERT INTO orders(item , customerid , price ) "
      "VALUES (?item:string,?customerid:int , ?price:double );"  //
      >
      insert_order{sqldb};

  insert_order.execute(bind<"item">("Phone"), bind<"price">(1444.44),
                       bind<"customerid">(1));
  insert_order.execute(bind<"item">("Laptop"), bind<"price">(1300.44),
                       bind<"customerid">(1));
  insert_order.execute(bind<"customerid">(1), bind<"price">(2000),
                       bind<"item">("MacBook"));

  using skydown::field;

  skydown::prepared_statement<
      "SELECT  orders.id:int, name:string,  item:string?, "
      "price:double "
      "FROM orders JOIN customers ON customers.id = customerid where price > "
      "?price:double;"  //
      >
      select_orders{sqldb};

  for (;;) {
    std::cout << "Enter min price.\n";
    double min_price = 0;
    std::cin >> min_price;

    for (auto &row : select_orders.execute_rows(bind<"price">(min_price))) {
      // Access the fields using `field`. We will get a compiler error if we try
      // to access a field that is not part of the select statement.
      std::cout << field<"orders.id">(row) << " ";
      std::cout << field<"price">(row) << " ";
      std::cout << field<"name">(row) << " ";
      std::cout << field<"item">(row).value() << "\n";
    }
  }
}

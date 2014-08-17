#include <iostream>

#include <neu/nvar.h>
#include <neu/NProgram.h>
#include <neu/NDatabase.h>
#include <neu/NRandom.h>
#include <neu/NSys.h>

using namespace std;
using namespace neu;

static const size_t NUM_ROWS = 10000000;
static const size_t COMMIT_INTERVAL = 1000000;

int main(int argc, char** argv){
  NProgram program(argc, argv);

  // does the database already exist?
  bool create = !NSys::exists("testdb");

  // if not, create it in the current directory
  NDatabase db("testdb", create);

  // limit pageable memory use to 1024 MB
  // larger sizes make reads and writes faster.
  // the memory limit can be changed at any time
  db.setMemoryLimit(1024);

  // test with lower values to see how reads/writes perform in the
  // presence of more page swapping
  //db.setMemoryLimit(128);

  if(create){
    // create a new table named "users"
    NTable* table = db.addTable("users");

    // get a write lock on the table using NTable::Lock. you can
    // acquire multiple read and write locks in an order consistent
    // with how other locks are obtained by others clients of the
    // database to help prevent deadlocking, i.e: table locks will
    // always be acquired in the same relative order. the lock
    // autoreleaes its locks on the underlying tables when the
    // NTable::Lock is destroyed
    NTable::Lock lock(table, true);

    // add indexable fields for fast queries
    table->addIndex("rank", NTable::Double);
    table->addIndex("norm", NTable::Double);

    // the hash index is the catch-all, it can hash any nvar
    // but a hash index will not be sorted by its nvar value in
    // a meaningful way that supports ranged based queries
    table->addIndex("name", NTable::Hash);

    NRandom rng;

    double t1 = NSys::now();
    
    nvar row;

    // add some rows to the table
    for(size_t i = 1; i < NUM_ROWS; ++i){
      row("rank") = rng.uniform(0, 100);
      row("norm") = rng.uniform(0, 100);
      row("name") = "neu" + nvar(i);

      // extra unindexed payload data - can be any type of potentially
      // deep and nested nvar
      if(i % 1000 == 0){
        nvar p;
        p("f1") = "t1";
        p("v") = nvec() << (i + 1) << 2 < 3;
        row("payload1") = p;
      }

      // note, any additional payload fields can be added on a per-row
      // basis

      table->insert(row);
      
      if(i % COMMIT_INTERVAL == COMMIT_INTERVAL - 1){
        table->commit();
        cout << "commiting..." << endl;
      }
    }

    double dt = NSys::now() - t1;

    cout << "inserted " << NUM_ROWS << " rows in: " << dt << endl;

    // add some more rows
    for(size_t i = 1; i < 10000; ++i){
      row("rank") = rng.uniform(0, 100);
      row("norm") = rng.uniform(0, 100);
    }

    // this time we will not commit the rows just added, instead
    // rollback
    table->rollback();

    // make sure everything is committed - when the NDatbase instance
    // is destroyed it will autocommit anyways
    db.commit();

    cout << "run command again to query" << endl;
    
    return 0;
  }

  // we opened the database, now perform some queries on it
  NTable* table = db.getTable("users");

  // get a read lock on the table
  NTable::Lock lock(table);
  
  nvec rows;

  double t1 = NSys::now();

  // a query func is a lambda which receives a const nvar& r for the
  // row - it returns 0 to stop querying, 1 to move forward, -1 to
  // return backwards
  NTable::QueryFunc f =
    [&](const nvar& r){
    if(r["norm"] < 50){
      rows << r;
    }
    return 1;
  };

  // find all rows whose rank >= 99.9 and norm < 50
  table->query("rank", 99.9, f);

  double dt = NSys::now() - t1;

  cout << "rows is: " << rows << endl;

  cout << "queries took: " << dt << endl;

  return 0;
}

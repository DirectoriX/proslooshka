#include <algorithm>
#include <climits>
#include <iostream>
#include <random>
#include <sqlite3.h>
#include <string>
#include <vector>

using namespace std;

const bool debug = true; // FIXME
const bool printOut = true; // WARNING
const int maxWeeks = 1;

// Куча настроек категорий
//const int cat0Calls = 6;
//const int cat0Count = 15;
//const int cat1Calls = 2;
//const int cat1Count = 5;
//const int cat2Calls = 1;
//const int cat2Count = 2;
const int cat0Calls = 8;
const int cat0Count = 40;
const int cat1Calls = 6;
const int cat1Count = 50;
const int cat2Calls = 2;
const int cat2Count = 10;

// Считаем суммарное положенное количество прослушек
const int totalCalls = cat0Calls * cat0Count + cat1Calls * cat1Count + cat2Calls * cat2Count;

int id = 0;
char *zErrMsg = nullptr;
int rc;

void check()
{
  if (rc != SQLITE_OK) // Проверяем на ошибки
    {
      cout << "SQL error: " << zErrMsg << endl;
      sqlite3_free(zErrMsg);
    }
}

int zeroCallback(void *NotUsed, int argc, char **argv, char **azColName)
{
  int i;

  for (i = 0; i < argc; i++)
    {
      //      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      cout << argv[i] << " ";
    }

  cout << endl;
  return 0;
}

int rowidCallback(void *rowid, int argc, char **argv, char **azColName)
{
  if (printOut)
    { zeroCallback(rowid, argc, argv, azColName); }

  *static_cast<int *>(rowid) = std::stoi(argv[0]);
  return 0;
}

int countCallback(void *count, int argc, char **argv, char **azColName)
{
  if (debug && printOut)
    { zeroCallback(count, argc, argv, azColName); }

  *static_cast<int *>(count) = std::stoi(argv[0]);
  return 0;
}

void createTable(sqlite3 *db)
{
  string req;
  req = "DROP TABLE IF EXISTS pocyks; CREATE TABLE pocyks (cat INT, name TEXT, callsRemain INT, mul FLOAT, active INT);";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
  check();
}

void insertPocyk(sqlite3 *db, int cat, int number, int pocyks)
{
  string req;
  req = "INSERT INTO pocyks (cat, name, callsRemain, mul, active) VALUES (";
  req += std::to_string(cat);
  req += ", 'Поцык №";
  req += std::to_string(number);
  req += "', ";
  req += std::to_string(pocyks);
  req += ", 1, 1);";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(),
                    zeroCallback, nullptr, &zErrMsg);
  check();
}

void call(sqlite3 *db)
{
  // Получаем поцыка
  int rowid;
  string req;
  req = "SELECT rowid, cat, name FROM pocyks WHERE active = 1 ORDER BY callsRemain*mul DESC LIMIT 1;";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), rowidCallback, &rowid, &zErrMsg);
  check();
  req = "UPDATE pocyks SET callsRemain = callsRemain-1, mul=mul/2 WHERE rowid=";
  req += std::to_string(rowid);
  req += ";";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
  check();
  //  req = "UPDATE pocyks SET mul = mul+1.0+1.0*random()/9223372036854775807 WHERE active=1 AND rowid!="; //TODO
  req = "UPDATE pocyks SET mul = mul+1.0+1.0*random()/9223372036854775807 WHERE rowid!=";
  req += std::to_string(rowid);
  req += ";";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
  check();
}

void resetCalls(sqlite3 *db)
{
  string req;
  req = "UPDATE pocyks SET callsRemain = ";
  req += std::to_string(cat0Calls);
  req += " WHERE cat=0;";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
  check();
  req = "UPDATE pocyks SET callsRemain = ";
  req += std::to_string(cat1Calls);
  req += " WHERE cat=1;";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
  check();
  req = "UPDATE pocyks SET callsRemain = ";
  req += std::to_string(cat2Calls);
  req += " WHERE cat=2;";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
  check();
}

void addCallsDno(sqlite3 *db)
{
  string req;
  req = "UPDATE pocyks SET callsRemain = ";
  req += std::to_string(cat0Calls);
  req += " WHERE cat=0;";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
  check();
}

bool callsRemain(sqlite3 *db)
{
  int count;
  string req;
  req = "SELECT COUNT(*) FROM pocyks WHERE callsRemain>0 AND active=1;";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), countCallback, &count, &zErrMsg);
  check();
  return count > 0;
}

void toggleActivityRandom(sqlite3 *db)
{
  string req;
  req = "UPDATE pocyks SET active = 1-active WHERE rowid=(SELECT rowid FROM pocyks ORDER BY random() LIMIT 1);";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
  check();
}

sqlite3 *connect(bool inMemory)
{
  sqlite3 *db;

  if (inMemory)
    {
      rc = sqlite3_open(":memory:", &db);
    }
  else
    {
      rc = sqlite3_open("aba", &db);
    }

  if (rc != 0) // Проверяем на ошибки
    {
      cout << "Can't open database: " << sqlite3_errmsg(db) << endl;
      sqlite3_close(db);
      return (nullptr);
    }

  string req;
  req = "PRAGMA journal_mode = OFF; PRAGMA synchronous = 0;";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
  check();
  return db;
}

int main()
{
  srand(time(nullptr)); // Инициализация рандома
  sqlite3 *db = connect(false);
  createTable(db); // Создаём таблицу
  cout << endl << "add pocyks" << endl << endl;
  //  Добавляем поцыков в таблицу в три захода
  {
    for (int i = 0; i < cat0Count; i++)
      {
        insertPocyk(db, 0, i, cat0Calls);
      }

    for (int i = 0; i < cat1Count; i++)
      {
        insertPocyk(db, 1, i, cat1Calls);
      }

    for (int i = 0; i < cat2Count; i++)
      {
        insertPocyk(db, 2, i, cat2Calls);
      }
  }

  for (int week = 0; week < maxWeeks; week++) // Много недель подряд...
    {
      cout << "----- Week " << week << " -----" << endl << endl;

      for (int i = 0; i < totalCalls; i++) // ... столько раз, сколько положено...
        {
          if (1.0 * rand() / RAND_MAX < 0.33) // С какой-то вероятностью поцык может уйти в отпуск или вернуться
            {
              toggleActivityRandom(db);
            }

          if (!callsRemain(db)) // Если больше нет доступных для прослушивания поцыков
            {
              addCallsDno(db); // Накидываем прослушек доньям
            }

          // printOut = true;
          call(db);
          //   printOut = false;
        }

      // Неделя закончилась - всем сбрасываем количество прослушиваний
      resetCalls(db);
      cout << endl << endl; // Разделяем недели пустой строкой
    }

  // Закрываем базу нафиг, хоть она и в памяти
  sqlite3_close(db);
  return 0;
}


#include <algorithm>
#include <array>
#include <climits>
#include <iostream>
#include <random>
#include <sqlite3.h>
#include <string>
#include <vector>

using namespace std;

const bool debug = true; // FIXME
const bool printOut = true; // WARNING
const int maxWeeks = 0;

// Куча настроек категорий
const int cat0Calls = 8;
const int cat0Count = 40;
const int cat1Calls = 6;
const int cat1Count = 50;
const int cat2Calls = 2;
const int cat2Count = 10;
// Считаем суммарное положенное количество прослушек
const int totalCalls = cat0Calls * cat0Count + cat1Calls * cat1Count + cat2Calls * cat2Count;

const int servicesCount = 8;
const vector<string> serviceNames = {"AO", "OdinSO", "ETP", "ABC", "OFD", "KEK", "ABA", "XYZ"};
const vector<int> serviceCalls = {600, 500, 300, 100, 200, 700, 400, 800};

// Считаем суммарное количество звонков за прошлую неделю
int totalCalls2 = 0;

const int OKKCount = 10;

struct Service
{
  int id = -1;
  int remainingOKKCount = 0;
  int calls = 0;
};

struct OKK
{
  int id = 0;
  string name;
  int calls = 0;
  vector<int> s = vector<int>(servicesCount);
};

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
      cout << (argv[i] != nullptr ? argv[i] : "NULL") << " ";
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

int servicesCallback(void *service, int argc, char **argv, char **azColName)
{
  if (debug && printOut)
    { zeroCallback(service, argc, argv, azColName); }

  auto *svc = static_cast<Service *>(service);
  svc->remainingOKKCount++;
  return 0;
}

int OKKCallback(void *OKKs, int argc, char **argv, char **azColName)
{
  if (printOut)
    { zeroCallback(OKKs, argc, argv, azColName); }

  if (argc >= 3)
    {
      auto *ops = static_cast<vector<OKK> *>(OKKs);
      OKK op;
      op.id = std::stoi(argv[0]);
      op.name = argv[1];

      for (int i = 2; i < argc; i++)
        {
          op.s[i - 2] = std::stoi(argv[i] != nullptr ? argv[i] : "-1");
        }

      ops->push_back(op);
      return 0;
    }

  return 1;
}

void createTablePocyks(sqlite3 *db)
{
  string req;
  req = "DROP TABLE IF EXISTS pocyks; CREATE TABLE pocyks (cat INT, name TEXT, callsRemain INT, mul FLOAT, active INT);";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
  check();
}

void createTableOKKs(sqlite3 *db)
{
  string req;
  req = "DROP TABLE IF EXISTS okks; CREATE TABLE okks (name TEXT";

  for (int i = 0; i < servicesCount; i++)
    {
      req += ", ";
      req += serviceNames[i];
      req += " INT";
    }

  req += ");";

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

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
  check();
}

void insertOKK(sqlite3 *db, const string &name, int code)
{
  string req;
  req = "INSERT INTO okks (name";

  for (int i = 0; i < servicesCount; i++)
    {
      req += ", ";
      req += serviceNames[i];
    }

  req += ") VALUES ('";
  req += name;
  req += "'";

  for (int i = 0; i < servicesCount; i++)
    {
      req += ", ";
      req += ((code + i) % 2 == 0 || (code + i + 4) % 5 > 2) ? "0" : "NULL";
    }

  req += ");";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
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

bool servicesComparsionOKKs(const Service &a, const Service &b)
{
  if (a.remainingOKKCount == b.remainingOKKCount)
    { return a.calls > b.calls; }

  return a.remainingOKKCount > b.remainingOKKCount;
}

void setCalls(OKK &op, int id, int calls)
{
  op.s[id] = calls;
  op.calls = 0;

  for (int i = 0; i < op.s.size(); i++)
    {
      if (op.s[i] > 0)
        {
          op.calls += op.s[i];
        }
    }
}

int serviceID;

bool serviceSearch(const Service &s)
{
  return s.id == serviceID;
}

void planOKK(sqlite3 *db)
{
  float k = 1.0 * totalCalls / totalCalls2;
  vector<int> calls;
  calls.reserve(servicesCount);

  for (int i = 0; i < servicesCount; i++)
    {
      calls.push_back(round(serviceCalls[i] * k));
    }

  int callsOKK = totalCalls / OKKCount;
  int dopCallsOKK = totalCalls - callsOKK * OKKCount;
  vector<Service> svcs = vector<Service>(servicesCount);

  for (int i = 0; i < svcs.size(); i++)
    {
      svcs[i].calls = calls[i];
      svcs[i].id = i;
    }

  string req;
  string req1 = "SELECT rowid FROM okks WHERE ";
  string req2 = " NOT NULL;";

  for (auto &svc : svcs)
    {
      req = req1 + serviceNames[svc.id];
      req += req2;

      if (debug)
        { cout << req << endl; }

      rc = sqlite3_exec(db, req.data(), servicesCallback, &svc, &zErrMsg);
      check();
    }

  for (int svcI = 0; svcI < servicesCount - 1; svcI++) // FIXME
    {
      std::sort(svcs.begin(), svcs.end(), servicesComparsionOKKs);
      Service s = svcs.back();
      svcs.pop_back();

      if (s.remainingOKKCount == 0)
        {
          break;
        }

      int calls = s.calls / s.remainingOKKCount;
      int dopCalls = s.calls - calls * s.remainingOKKCount;
      req = "SELECT rowid, * FROM okks WHERE ";
      req += serviceNames[s.id];
      req += " = 0;";

      if (debug)
        { cout << req << endl; }

      vector<OKK> OKKs = vector<OKK>();
      rc = sqlite3_exec(db, req.data(), OKKCallback, &OKKs, &zErrMsg);
      check();

      for (auto &OKK : OKKs)
        {
          OKK.calls = 0;

          for (int i = 0; i < OKK.s.size(); i++)
            {
              if (OKK.s[i] > 0)
                {
                  OKK.calls += OKK.s[i];
                }
            }
        }

      for (auto &OKK : OKKs)
        {
          if (callsOKK - OKK.calls <= calls)
            {
              setCalls(OKK, s.id, callsOKK - OKK.calls);
              serviceID = s.id;
              s.calls -= OKK.s[s.id];
              s.remainingOKKCount--;
            }
        }

      calls = s.calls / s.remainingOKKCount;
      dopCalls = s.calls - calls * s.remainingOKKCount;

      for (auto &OKK : OKKs)
        {
          if (OKK.calls < callsOKK)
            {
              setCalls(OKK, s.id, calls + (dopCalls-- > 0 ? 1 : 0));
            }
        }

      // Проверка на несвободных операторов
      for (auto &OKK : OKKs)
        {
          int freedom = 0;

          for (int i = 0; i < servicesCount; i++)
            {
              if (OKK.s[i] == 0)
                {
                  freedom++;
                }
            }

          if (freedom == 1) // Дополнить оставшуюся службу!
            {
              for (int i = 0; i < servicesCount; i++)
                {
                  if (OKK.s[i] == 0)
                    {
                      serviceID = i;
                      auto it = std::find_if(svcs.begin(), svcs.end(), serviceSearch);
                      setCalls(OKK, i, std::min(callsOKK + (dopCallsOKK-- > 0 ? 1 : 0) - OKK.calls, it->calls));
                      it->calls -= OKK.s[i];
                      it->remainingOKKCount--;
                      break;
                    }
                }
            }

          req = "UPDATE okks SET ";

          for (int i = 0; i < servicesCount; i++)
            {
              req += serviceNames[i];
              req += " = ";
              req += OKK.s[i] < 0 ? "NULL" : std::to_string(OKK.s[i]);
              req += ", ";
            }

          req += "rowid = rowid WHERE rowid = ";
          req += std::to_string(OKK.id);
          req += ";";

          if (debug)
            { cout << req << endl; }

          rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
          check();
        }
    }
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

  for (int i = 0; i < servicesCount; i++)
    {
      totalCalls2 += serviceCalls[i];
    }

  sqlite3 *db = connect(false);
  cout << endl << endl;
  createTablePocyks(db); // Создаём таблицу поцыков
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
  cout << endl << endl;
  createTableOKKs(db); // Создаём таблицу ОКК
  cout << endl << "add OKKs" << endl << endl;
  //  Добавляем OKK в таблицу в три захода
  {
    string OKK = "OKK ";
    string name;

    for (int i = 0; i < OKKCount; i++)
      {
        name = OKK + std::to_string(i);
        insertOKK(db, name, i);
      }
  }
  cout << endl << "planirovanie OKK" << endl << endl;
  planOKK(db);

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

          call(db);
        }

      // Неделя закончилась - всем сбрасываем количество прослушиваний
      resetCalls(db);
      cout << endl << endl; // Разделяем недели пустой строкой
    }

  // Закрываем базу нафиг, хоть она и в памяти
  sqlite3_close(db);
  return 0;
}


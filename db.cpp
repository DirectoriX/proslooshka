#include <algorithm>
#include <cmath>
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>

using namespace std;

const bool debug = true; // FIXME
const bool printOut = true; // WARNING

// Куча настроек категорий
const int cat0Calls = 8;
const int cat0Count = 40;
const int cat1Calls = 6;
const int cat1Count = 50;
const int cat2Calls = 2;
const int cat2Count = 10;
// Считаем суммарное положенное количество прослушек
const int totalCalls = cat0Calls * cat0Count + cat1Calls * cat1Count + cat2Calls * cat2Count;

// Параметры служб: количество, названия, каоличество звноков в прошлом спринте
const int servicesCount = 8;
const vector<string> serviceNames = {"AO", "OdinSO", "ETP", "ABC", "OFD", "KEK", "ABA", "XYZ"};
const vector<int> serviceCalls = {600, 500, 300, 100, 200, 700, 400, 800};

// Считаем суммарное количество звонков за прошлую неделю
int totalCalls2 = 0;

// Количество ОККшников (внезапно)
const int OKKCount = 10;

// Описание службы
struct Service
{
  int id = -1;
  int remainingOKKCount = 0;
  int calls = 0;
};

// Описнаие ОККшника
struct OKK
{
  int id = 0;
  string name;
  int calls = 0;
  vector<int> s = vector<int>(servicesCount);
};

// Пара вспомогательных переменных
char *zErrMsg = nullptr;
int rc;
string req;

// Проверяем запрос к БД на ошибки
void check()
{
  if (rc != SQLITE_OK) // Проверяем на ошибки
    {
      cout << "SQL error: " << zErrMsg << endl;
      sqlite3_free(zErrMsg);
    }
}

// Печатает результат на экран. SELECTы всякие
int zeroCallback(void *NotUsed, int argc, char **argv, char **azColName)
{
  int i;

  for (i = 0; i < argc; i++)
    {
      cout << (argv[i] != nullptr ? argv[i] : "NULL") << " ";
    }

  cout << endl;
  return 0;
}

// Заполняет службы количеством ОККшников
int servicesCallback(void *service, int argc, char **argv, char **azColName)
{
  if (debug && printOut)
    { zeroCallback(service, argc, argv, azColName); }

  auto *svc = static_cast<Service *>(service);
  svc->remainingOKKCount++;
  return 0;
}

// Заполняет информацию об ОККШнике - имя, ID, службы
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

// Создаёт таблицу ОККшников
void createTableOKKs(sqlite3 *db)
{
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

// Добавляет ОККшника
// Много циклов потому что много служб
void insertOKK(sqlite3 *db, const string &name, int code)
{
  req = "INSERT INTO okks (name";

  // Формируем список названий
  for (int i = 0; i < servicesCount; i++)
    {
      req += ", ";
      req += serviceNames[i];
    }

  req += ") VALUES ('";
  req += name;
  req += "'";

  // Заполняем значения
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

// Сравнение служб для сортировки
bool servicesComparsion(const Service &a, const Service &b)
{
  // Если операторов поровну - то сортируем по кол-ву звонков
  if (a.remainingOKKCount == b.remainingOKKCount)
    { return a.calls > b.calls; }

  return a.remainingOKKCount > b.remainingOKKCount;
}

// Записываем количество звонков оператора по данной службе
void setCalls(OKK &op, int id, int calls)
{
  op.s[id] = calls;
  op.calls += calls;
}

// Ннада для поиска
int serviceID;

// Это для поиска службы по ID
bool serviceSearch(const Service &s)
{
  return s.id == serviceID;
}

// Планирование звонков по ОККшникам
// Вообще огонь
void planOKK(sqlite3 *db)
{
  // Считаем ожидаемое кол-во звонков
  float k = 1.0 * totalCalls / totalCalls2;
  int callsOKK = totalCalls / OKKCount;
  // Дополнительные звонки для полного учёта
  int dopCallsOKK = totalCalls - callsOKK * OKKCount;
  // Создаём и заполняем список служб
  vector<Service> svcs = vector<Service>(servicesCount);

  for (int i = 0; i < svcs.size(); i++)
    {
      svcs[i].calls = round(serviceCalls[i] * k);
      svcs[i].id = i;
    }

  for (auto &svc : svcs)
    {
      req = "SELECT rowid FROM okks WHERE ";
      req += serviceNames[svc.id];
      req += " NOT NULL;";

      if (debug)
        { cout << req << endl; }

      rc = sqlite3_exec(db, req.data(), servicesCallback, &svc, &zErrMsg);
      check();
    }

  // Обрабатываем службы
  for (int svcI = 0; svcI < servicesCount - 1; svcI++)
    {
      // Находим подходящую - она в конце
      std::sort(svcs.begin(), svcs.end(), servicesComparsion);
      Service s = svcs.back();
      svcs.pop_back();
      // Считаем кол-во звонков на оператора
      int calls = s.calls / s.remainingOKKCount;
      // Заполняем список ОККшников для службы
      req = "SELECT rowid, * FROM okks WHERE ";
      req += serviceNames[s.id];
      req += " = 0;";

      if (debug)
        { cout << req << endl; }

      vector<OKK> OKKs = vector<OKK>();
      rc = sqlite3_exec(db, req.data(), OKKCallback, &OKKs, &zErrMsg);
      check();

      // Смотрим, сможет ли он принять полученное кол-во звонков
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

          // Не смог, падла, пусть берёт тогда сколько может
          if (callsOKK - OKK.calls <= calls)
            {
              setCalls(OKK, s.id, callsOKK - OKK.calls);
              serviceID = s.id;
              s.calls -= OKK.s[s.id];
              s.remainingOKKCount--;
            }
        }

      // Считаем оставшиеся звонки
      calls = s.calls / s.remainingOKKCount;
      int dopCalls = s.calls - calls * s.remainingOKKCount;

      // Проходимся по всем ОККшникам службы
      for (auto &OKK : OKKs)
        {
          // накидываем звонков
          if (OKK.calls < callsOKK)
            {
              setCalls(OKK, s.id, calls + (dopCalls-- > 0 ? 1 : 0));
            }

          // Смотрим, сколько ещё служб у него осталось
          int freedom = 0;

          for (int i = 0; i < servicesCount; i++)
            {
              if (OKK.s[i] == 0)
                {
                  freedom++;
                }
            }

          // Дополнить оставшуюся службу!
          if (freedom == 1)
            {
              for (int i = 0; i < servicesCount; i++)
                {
                  if (OKK.s[i] == 0)
                    {
                      // нашли эту самую последнюю службу по ID, теперь нао найти и обновить объект
                      serviceID = i;
                      auto it = std::find_if(svcs.begin(), svcs.end(), serviceSearch);
                      setCalls(OKK, i, std::min(callsOKK + (dopCallsOKK-- > 0 ? 1 : 0) - OKK.calls, it->calls));
                      it->calls -= OKK.s[i];
                      it->remainingOKKCount--;
                      break;
                    }
                }
            }

          // Готовим запрос на обновление БД
          req = "UPDATE okks SET ";

          // Составляем список новых значений
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

// Подключаемся к БД
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

  // Увелчиваем производительность
  req = "PRAGMA journal_mode = OFF; PRAGMA synchronous = 0;";

  if (debug)
    { cout << req << endl; }

  rc = sqlite3_exec(db, req.data(), zeroCallback, nullptr, &zErrMsg);
  check();
  return db;
}

int main()
{
  for (int i = 0; i < servicesCount; i++)
    {
      totalCalls2 += serviceCalls[i];
    }

  sqlite3 *db = connect(false);
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
  // Закрываем базу нафиг, хоть она и в памяти
  sqlite3_close(db);
  return 0;
}

#include <iostream>
#include <random>
#include <pocyk.h>
#include <vector>
#include <algorithm>

using namespace std;

// Куча настроек категорий
const int cat0Calls = 8;
const int cat0Count = 40;
const int cat1Calls = 6;
const int cat1Count = 50;
const int cat2Calls = 2;
const int cat2Count = 10;

// Считаем суммарное положенное количество прослушек
const int totalCalls = cat0Calls * cat0Count + cat1Calls * cat1Count + cat2Calls * cat2Count;

int main()
{
  srand(time(NULL)); // Инициализация рандома
  Pocyk::init(0.5, cat0Calls, cat1Calls, cat2Calls); // Вбиваем настройки операторов
  vector<Pocyk> vec = vector<Pocyk>();

  // Заполняем массив операторами
  for (int i = 0; i < cat0Count; i++)
    {
      vec.push_back(Pocyk(0, i));
    }

  for (int i = 0; i < cat1Count; i++)
    {
      vec.push_back(Pocyk(1, i));
    }

  for (int i = 0; i < cat2Count; i++)
    {
      vec.push_back(Pocyk(2, i));
    }

  for (int week = 0; week < 1; week++) // Много недель подряд...
    {
      for (int i = 0; i < totalCalls; i++) // ... столько раз, сколько положено...
        {
          std::sort(vec.begin(), vec.end()); // Сортировка по коэффициенту

          if (vec.back().getK() == 0.0) // Если у всех он равен 0...
            {
              for (int j = 0; j < vec.size(); j++) // ..., то добавляем всем новичкам доп. прослушек
                {
                  vec[j].addCallsDno();
                }

              std::sort(vec.begin(), vec.end()); // Снова сортируем
            }

          if (1.0 * rand() / RAND_MAX < 0.33) // С какой-то вероятностью оператор может уйти в отпуск или вернуться
            {
              int toggleNumber = 1.0 * rand() / RAND_MAX * vec.size();
              vec[toggleNumber].toggleActivity();
            }

          vec.back().call(); // Оператор с самым высоким коэффициентом прослушивается

          for (int j = 0; j < vec.size() - 1; j++) // Всем остальным повезло на этот раз...
            {
              vec[j].skip();
            }
        }

      // Неделя закончилась - всем сбрасываем количество прослушиваний
      for (int j = 0; j < vec.size() - 1; j++)
        {
          vec[j].resetCalls();
        }

      cout << endl << endl; // Разделяем недели пустой строкой
    }

  Pocyk::printStat();
  cout << endl;
  return 0;
}


#include "pocyk.h"
int calcs = 0;
int callss = 0;

float localDelta = 1; // Среднее изменение множителя в случае НЕ прослушивания
std::array<int, 3> callCount; // Массив соответствия норматива прослушки и категории


Pocyk::Pocyk(int category, int number)
{
  if (category >= 0 && category <= 2)
    { this->category = category; }
  else { this->category = 0; }

  this->number = number;
  multiplier = 1;
  active = true;
  calls = callCount[category];
  calc();
}

Pocyk::~Pocyk()
{
}

// Переключение статуса активности
void Pocyk::toggleActivity()
{
  active = !active;
  calc();
}

// Получение коэффициента
float Pocyk::getK()
{
  return k;
}

// Повезло и прослушали не его
void Pocyk::skip()
{
  multiplier += localDelta * 2.0 * rand() / RAND_MAX; // Множитель растёт
  calc();
}

// Первоначальная настройка
void Pocyk::init(float delta, int cat1, int cat2, int cat3)
{
  localDelta = delta;
  callCount[0] = cat1;
  callCount[1] = cat2;
  callCount[2] = cat3;
}

// Вывод небольшой статистики
void Pocyk::printStat()
{
  std::cout << "Вычислений k: " << calcs << std::endl;
  std::cout << "Звонков: " << callss << std::endl;
  std::cout << "Всё хорошо на " << 100.0 * callss / calcs << "%" << std::endl;
}

// Вычисление коэффициента - это inline функция, поэтому всё хорошо
void Pocyk::calc()
{
  k = !active ? 0 : calls * multiplier;
  calcs++;
}

// Сброс звонков
void Pocyk::resetCalls()
{
  calls = callCount[category];
  calc();
}

// Добавление прослушиваний новичкам
// Функция вызывается у всех, но добавляет только некоторым
void Pocyk::addCallsDno()
{
  if (category == 0)
    {
      calls = callCount[0];
      calc();
    }
}

// Прослушали :-(
void Pocyk::call()
{
  calls--; // Осталось меньше прослушиваний
  multiplier /= 2; // Множитель тоже упал
  calc();
  std::cout << category << '-' << number << ' '; // Показываем, кого прослушали
  callss++;
}

bool operator<(const Pocyk &left, const Pocyk &right)
{
  return left.k < right.k;
}

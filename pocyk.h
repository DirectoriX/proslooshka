#ifndef POCYK_H
#define POCYK_H

#include <array>
#include <iostream>

class Pocyk {
  public:
    Pocyk(int category, int number);
    ~Pocyk();

    void toggleActivity(); // Переключение статуса активности
    float getK(); // Получение коэффициента
    void skip(); // Повезло и прослушали не его
    void resetCalls(); // Сброс звонков
    void addCallsDno(); // Добавление прослушиваний новичкам
    void call(); // Прослушали :-(

    friend bool operator<(const Pocyk &left, const Pocyk &right); // Нужно для сортировки

    static void init(float delta, int cat1, int cat2, int cat3); // Первоначальная настройка

    static void printStat(); // Вывод небольшой статистики

  private:
    int calls; // Количество оставшихся звонков
    int category; // Категория оператора (0 - новичок, 2 - Скрипаль)
    int number; // Номер оператора внутри категории. Только для вывода на экран
    bool active; // Флаг активности
    float multiplier; // Множитель для вычисления коэффициента
    float k; // Сам коэффициент

    inline void calc(); // Вычисление коэффициента
};

#endif // POCYK_H

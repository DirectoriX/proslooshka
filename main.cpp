#include <iostream>
#include <random>

using namespace std;

int catCount; // Количество категорий
int *peopleCount; // Количетво людей в каждой категории
int *listen; // Количество прослушиваний для каждой категории
int **count; // Количество оставшихся прослушиваний для каждого поцыка
bool **active; // Количество оставшихся прослушиваний для каждого поцыка

int totalListen, unusedListen;

void print(int number)
{
  int p = 0; // Вероятность выбора поцыка. Чем ближе к концу перебора, тем больше
  double rrr = number; // Вероятность, которая сравнивается с p
  rrr /= RAND_MAX;
  rrr *= rand();

  for (int i = 0; i < catCount; i++)
    {
      for (int j = 0; j < peopleCount[i]; j++)
        {
          p += count[i][j];

          if (active[i][j])
            {
              if (rrr < p)
                {
                  // Нашли подопытного
                  cout << i << "-" << j << " ";
                  count[i][j]--;
                  return;
                }
            }
        }
    }

  cout << "!!!" << " "; // Вот дерьмо, всех доступных прослушали!
}

int main2()
{
  srand(time(NULL));
  cout << "Введи количество категорий: ";
  cin >> catCount;
  count = new int *[catCount];
  active = new bool *[catCount];
  peopleCount = new int[catCount];
  listen = new int[catCount];
  totalListen = unusedListen = 0;

  for (int i = 0; i < catCount; i++)
    {
      cout << "Введи количество поцыков категории " << i << ": ";
      cin >> peopleCount[i];
      count[i] = new int[peopleCount[i]];
      active[i] = new bool[peopleCount[i]];
      cout << "Введи количество прослушиваний поцыков категории " << i << ": ";
      cin >> listen[i];

      for (int j = 0; j < peopleCount[i]; j++)
        {
          count[i][j] = listen[i];
          active[i][j] = true;
          totalListen += listen[i];
        }
    }

  unusedListen = totalListen;
  cout << "Всего прослушиваний: " << totalListen << endl;

  for (; unusedListen > 0; unusedListen--)
    {
      print(unusedListen); // Выбираем одну жертву
      //      // Выключаем уродов
      //      if (unusedListen == 25)
      //        { active[0][1] = false; }
      //      if (unusedListen == 24)
      //        { active[0][0] = false; }
      //      if (unusedListen == 23)
      //        { active[0][2] = false; }
      //      // Включаем уродов
      //      if (unusedListen == 6)
      //        { active[0][1] = true; }
      //      if (unusedListen == 4)
      //        { active[0][0] = true; }
      //      if (unusedListen == 2)
      //        { active[0][2] = true; }
    }

  cout << endl;
  return 0;
}

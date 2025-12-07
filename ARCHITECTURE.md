# Архитектура и объяснение кода Lab 7

## Обзор проекта

Проект реализует **многопоточную симуляцию** боев между NPC (Non-Player Characters) на карте размером 100x100. Используются три паттерна проектирования из Lab 6:
- **Factory** - для создания NPC
- **Visitor** - для определения результатов боев
- **Observer** - для уведомления о боях

**Ключевое отличие от Lab 6:** Добавлена многопоточность с тремя параллельными потоками и система боев с использованием кубика (6-гранный).

## Структура проекта

```
lab_7/
├── include/          # Заголовочные файлы
│   ├── npc.hpp       # Базовый класс NPC (с многопоточностью)
│   ├── npc_types.hpp # Статистика NPC (расстояния движения/убийства)
│   ├── bear.hpp      # Класс Bear
│   ├── elf.hpp       # Класс Elf
│   ├── robber.hpp    # Класс Robber
│   ├── fight_visitor.hpp  # Интерфейс Visitor для боев
│   ├── observer.hpp  # Интерфейс Observer и реализации
│   ├── npc_factory.hpp    # Фабрика для создания NPC
│   └── game.hpp      # Класс Game с многопоточностью
├── src/              # Реализации
│   ├── npc.cpp
│   ├── bear.cpp
│   ├── elf.cpp
│   ├── robber.cpp
│   ├── observer.cpp
│   ├── npc_factory.cpp
│   ├── game.cpp      # Реализация многопоточной игры
│   └── main.cpp      # Точка входа
└── tests/            # Unit-тесты
```

---

## Детальное объяснение компонентов

### 1. Базовый класс NPC (`npc.hpp`, `npc.cpp`)

**Назначение:** Абстрактный базовый класс для всех типов NPC с поддержкой многопоточности.

**Ключевые отличия от Lab 6:**

#### Многопоточная безопасность:
```cpp
protected:
  bool alive_;                    // Статус жизни NPC
  mutable std::shared_mutex mutex_;  // Мьютекс для потокобезопасности
```

**Почему `std::shared_mutex`?**
- Позволяет множественные одновременные чтения (`std::shared_lock`)
- Только одна запись в один момент (`std::unique_lock`)
- Оптимизирует производительность при множественных читателях

#### Новые методы для многопоточности:

**`IsAlive()` и `Kill()`:**
```cpp
bool IsAlive() const {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  return alive_;
}

void Kill() {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  alive_ = false;
}
```

**Зачем нужно:**
- `IsAlive()` - проверка статуса (множественные читатели)
- `Kill()` - изменение статуса (единственный писатель)
- Используются в боевой системе для проверки перед боем

**`Move()` - движение NPC:**
```cpp
void Move(int new_x, int new_y) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (!alive_) return;  // Мертвые NPC не двигаются
  
  // Ограничение координат картой (0-100)
  x_ = std::clamp(new_x, 0, 100);
  y_ = std::clamp(new_y, 0, 100);
}
```

**Особенности:**
- Мертвые NPC не двигаются
- Координаты ограничены картой
- Используется `std::unique_lock` для записи

**`GetMoveDistance()` и `GetKillDistance()`:**
```cpp
int GetMoveDistance() const {
  return NpcStats::GetMoveDistance(type_);
}

int GetKillDistance() const {
  return NpcStats::GetKillDistance(type_);
}
```

**Зачем:**
- Возвращают характеристики NPC из таблицы `NpcStats`
- Bear: движение 5, убийство 10
- Elf: движение 10, убийство 50
- Robber: движение 10, убийство 10

**`RollDice()` - бросок кубика:**
```cpp
int RollDice() const {
  thread_local std::random_device rd;
  thread_local std::mt19937 gen(rd());
  thread_local std::uniform_int_distribution<> dice(1, 6);
  return dice(gen);
}
```

**Почему `thread_local`?**
- Каждый поток имеет свой генератор случайных чисел
- Избегает синхронизации при генерации
- Повышает производительность в многопоточной среде

#### Предотвращение deadlock в `IsClose()`:

```cpp
bool NPC::IsClose(const std::shared_ptr<NPC>& other, size_t distance) const {
  // Блокировка в порядке адресов для предотвращения deadlock
  if (this < other.get()) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::shared_lock<std::shared_mutex> other_lock(other->mutex_);
    // ...
  } else {
    std::shared_lock<std::shared_mutex> other_lock(other->mutex_);
    std::shared_lock<std::shared_mutex> lock(mutex_);
    // ...
  }
}
```

**Почему это важно:**
- Если два потока одновременно вызывают `IsClose()` для пары (A, B) и (B, A), может возникнуть deadlock
- Блокировка в порядке адресов гарантирует одинаковый порядок для всех потоков
- Это классический паттерн предотвращения deadlock

#### Оптимизация `FightNotify()`:

```cpp
void NPC::FightNotify(...) {
  // Копируем наблюдателей, чтобы не держать блокировку во время вызова
  std::vector<std::shared_ptr<IFightObserver>> observers_copy;
  {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    observers_copy = observers_;
  }
  
  // Вызываем наблюдателей без блокировки
  for (const auto& observer : observers_copy) {
    observer->OnFight(attacker, defender, win);
  }
}
```

**Почему копирование?**
- Если `observer->OnFight()` попытается получить доступ к NPC (например, для получения имени), может возникнуть deadlock
- Копирование и освобождение блокировки перед вызовом предотвращает это
- Небольшая цена копирования оправдана безопасностью

---

### 2. Конкретные типы NPC (Bear, Elf, Robber)

**Структура аналогична Lab 6, но с логикой боя с кубиком:**

#### Пример из `bear.cpp`:

```cpp
// Visitor implementation for Bear - contains fight logic
// Bear kills Elves (with dice roll)
bool Bear::Visit(std::shared_ptr<Bear> /*defender*/) {
  return false;  // Bear cannot kill Bear
}

bool Bear::Visit(std::shared_ptr<Elf> defender) {
  if (!defender || !defender->IsAlive() || !IsAlive()) {
    return false;
  }
  // Bear kills Elf - use dice roll
  int attack = RollDice();
  int defense = defender->RollDice();
  return attack > defense;
}

bool Bear::Visit(std::shared_ptr<Robber> /*defender*/) {
  return false;  // Bear cannot kill Robber
}
```

**Ключевые отличия от Lab 6:**
1. **Проверка `IsAlive()`** - перед боем проверяем, что оба NPC живы
2. **Кубик** - используется `RollDice()` для определения результата боя
3. **Правила сохранены** - Bear убивает только Elf, Elf убивает только Robber, Robber убивает только Robber

**Почему логика в `Visit()`, а не в `Fight()`?**
- Абстрактный класс `NPC` не должен знать о конкретных типах
- Логика боя инкапсулирована в Visitor паттерне
- Соответствует принципам ООП

---

### 3. Класс Game (`game.hpp`, `game.cpp`)

**Назначение:** Оркестрация многопоточной симуляции.

#### Структура данных:

```cpp
class Game {
 private:
  std::vector<std::shared_ptr<NPC>> npcs_;
  mutable std::shared_mutex npcs_mutex_;  // Защита списка NPC
  
  std::queue<CombatTask> combat_queue_;
  std::mutex combat_queue_mutex_;
  std::condition_variable combat_queue_cv_;  // Ожидание задач боя
  
  std::atomic<bool> running_;  // Флаг работы игры
  mutable std::mutex cout_mutex_;  // Защита std::cout
};
```

**Почему разные типы мьютексов?**
- `std::shared_mutex` для `npcs_` - множественные читатели (Movement, Main потоки)
- `std::mutex` для `combat_queue_` - простая очередь, один читатель/писатель
- `std::mutex` для `cout_mutex_` - защита вывода в консоль
- `std::atomic<bool>` для `running_` - атомарный флаг, не требует мьютекса

#### Три потока:

**1. MovementThread - поток движения:**

```cpp
void Game::MovementThread() {
  while (running_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Копируем список NPC, чтобы не держать блокировку долго
    std::shared_lock<std::shared_mutex> read_lock(npcs_mutex_);
    auto npcs_copy = npcs_;
    read_lock.unlock();
    
    for (auto& npc : npcs_copy) {
      if (!npc->IsAlive()) continue;
      
      // Случайное движение
      int move_dist = npc->GetMoveDistance();
      double angle = angle_dist(gen_local);
      int new_x = current_x + static_cast<int>(move_dist * std::cos(angle));
      int new_y = current_y + static_cast<int>(move_dist * std::sin(angle));
      npc->Move(new_x, new_y);
      
      // Проверка боевых ситуаций
      for (const auto& other : npcs_copy) {
        if (npc == other || !other->IsAlive()) continue;
        
        if (npc->IsClose(other, npc->GetKillDistance())) {
          // Добавляем задачу боя в очередь
          std::lock_guard<std::mutex> queue_lock(combat_queue_mutex_);
          if (combat_queue_.size() < 500) {  // Ограничение размера очереди
            combat_queue_.push({npc, other});
            combat_queue_cv_.notify_one();
          }
        }
      }
    }
  }
}
```

**Ключевые моменты:**
- **Копирование списка NPC** - избегаем долгого удержания блокировки
- **Случайное движение** - угол и расстояние из характеристик NPC
- **Ограничение очереди** - предотвращаем переполнение памяти
- **Уведомление** - `notify_one()` пробуждает CombatThread

**2. CombatThread - поток боев:**

```cpp
void Game::CombatThread() {
  while (running_) {
    std::unique_lock<std::mutex> queue_lock(combat_queue_mutex_);
    combat_queue_cv_.wait(queue_lock, [this] { 
      return !combat_queue_.empty() || !running_; 
    });
    
    if (!running_ && combat_queue_.empty()) break;
    if (combat_queue_.empty()) continue;
    
    CombatTask task = combat_queue_.front();
    combat_queue_.pop();
    queue_lock.unlock();
    
    // Проверка, что оба живы
    if (!task.attacker->IsAlive() || !task.defender->IsAlive()) {
      continue;
    }
    
    // Атакующий атакует защитника
    auto attacker_visitor = std::dynamic_pointer_cast<FightVisitor>(task.attacker);
    bool defender_killed = false;
    if (attacker_visitor && task.defender->Accept(attacker_visitor)) {
      task.defender->Kill();
      defender_killed = true;
      task.attacker->FightNotify(task.attacker, task.defender, true);
    }
    
    // Защитник атакует атакующего (оба могут погибнуть)
    if (task.attacker->IsAlive() && !defender_killed) {
      auto defender_visitor = std::dynamic_pointer_cast<FightVisitor>(task.defender);
      if (defender_visitor && task.defender->IsAlive() && 
          task.attacker->Accept(defender_visitor)) {
        task.attacker->Kill();
        task.defender->FightNotify(task.defender, task.attacker, true);
      }
    }
  }
}
```

**Ключевые моменты:**
- **Condition Variable** - поток ждет, пока не появятся задачи или игра не остановится
- **Двусторонний бой** - оба NPC могут атаковать друг друга
- **Проверка статуса** - перед боем проверяем, что NPC живы
- **Visitor паттерн** - используется для определения результата боя

**3. MainThread - главный поток:**

```cpp
void Game::MainThread() {
  auto start_time = std::chrono::steady_clock::now();
  
  while (running_) {
    // Проверка времени игры (30 секунд)
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        current_time - start_time).count();
    
    if (elapsed >= GAME_DURATION_SECONDS) {
      running_ = false;
      break;
    }
    
    PrintMap();  // Печать карты раз в секунду
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  // Вывод выживших
  auto survivors = GetAliveNPCs();
  std::cout << "Survivors: " << survivors.size() << std::endl;
}
```

**Ключевые моменты:**
- **Таймер игры** - игра длится 30 секунд
- **Печать карты** - каждую секунду
- **Завершение** - вывод списка выживших

#### Метод `PrintMap()`:

```cpp
void Game::PrintMap() const {
  std::lock_guard<std::mutex> cout_lock(cout_mutex_);
  
  // Получаем живых NPC
  std::shared_lock<std::shared_mutex> read_lock(npcs_mutex_);
  auto npcs_copy = npcs_;
  read_lock.unlock();
  
  // Создаем карту 100x100
  std::vector<std::vector<int>> map(101, std::vector<int>(101, 0));
  
  for (const auto& npc : npcs_copy) {
    if (npc->IsAlive()) {
      int x = npc->GetX();
      int y = npc->GetY();
      map[y][x]++;
    }
  }
  
  // Печать карты
  // ...
}
```

**Особенности:**
- Использует `cout_mutex_` для защиты вывода
- Копирует список NPC для чтения
- Показывает количество NPC в каждой клетке

---

### 4. Паттерны проектирования

#### Factory Pattern (`npc_factory.hpp`, `npc_factory.cpp`)

Аналогичен Lab 6 - централизованное создание NPC.

#### Visitor Pattern (`fight_visitor.hpp`)

**Отличие от Lab 6:**
- Логика боя полностью в методах `Visit()`
- Используется кубик для определения результата
- Проверка `IsAlive()` перед боем

**Поток вызовов:**
```
CombatThread получает задачу боя
  ↓
attacker_visitor = dynamic_cast<FightVisitor>(attacker)
  ↓
defender->Accept(attacker_visitor)
  ↓
attacker->Visit(defender_typed)
  ↓
Проверка IsAlive() + RollDice()
  ↓
return attack > defense
```

#### Observer Pattern (`observer.hpp`, `observer.cpp`)

**Отличие от Lab 6:**
- `ConsoleObserver` использует `std::mutex` для защиты `std::cout`
- `FileObserver` записывает в файл (потокобезопасно через файловую систему)

```cpp
void ConsoleObserver::OnFight(...) {
  if (win) {
    std::lock_guard<std::mutex> lock(cout_mutex_);  // Защита вывода
    std::cout << "MURDER: ..." << std::endl;
  }
}
```

---

### 5. Многопоточность и синхронизация

#### Типы синхронизации:

1. **`std::shared_mutex`** - для `npcs_`:
   - Множественные читатели (Movement, Main потоки)
   - Единственный писатель (Initialize, добавление NPC)

2. **`std::mutex`** - для очереди боев:
   - Простая очередь, один читатель/писатель
   - Используется с `std::condition_variable`

3. **`std::condition_variable`** - для очереди боев:
   - CombatThread ждет появления задач
   - MovementThread уведомляет о новых задачах

4. **`std::atomic<bool>`** - для флага `running_`:
   - Атомарные операции, не требуют мьютекса
   - Используется для остановки потоков

5. **`std::mutex`** - для `std::cout`:
   - Защита вывода в консоль от гонок данных

#### Предотвращение deadlock:

1. **Порядок блокировок в `IsClose()`:**
   - Всегда блокируем в порядке адресов
   - Гарантирует одинаковый порядок для всех потоков

2. **Копирование в `FightNotify()`:**
   - Копируем наблюдателей перед вызовом
   - Освобождаем блокировку до вызова `OnFight()`

3. **Копирование списка NPC:**
   - В MovementThread копируем список перед итерацией
   - Минимизируем время удержания блокировки

4. **Ограничение размера очереди:**
   - Предотвращаем переполнение памяти
   - Ограничение на 500 задач

---

### 6. Логика боя с кубиком

#### Правила боя (сохранены из Lab 6):

- **Bear** убивает: Elf (с кубиком)
- **Elf** убивает: Robber (с кубиком)
- **Robber** убивает: Robber (с кубиком)

#### Механика кубика:

```cpp
bool Bear::Visit(std::shared_ptr<Elf> defender) {
  if (!defender || !defender->IsAlive() || !IsAlive()) {
    return false;
  }
  // Атакующий и защищающийся бросают кубик
  int attack = RollDice();      // 1-6
  int defense = defender->RollDice();  // 1-6
  return attack > defense;  // Атакующий побеждает, если больше
}
```

**Особенности:**
- Оба NPC бросают кубик
- Атакующий побеждает, если его результат больше
- При равенстве защищающийся выживает

#### Двусторонний бой:

```cpp
// Атакующий атакует защитника
if (attacker_visitor && defender->Accept(attacker_visitor)) {
  defender->Kill();
  defender_killed = true;
}

// Защитник атакует атакующего (если атакующий еще жив)
if (attacker->IsAlive() && !defender_killed) {
  if (defender_visitor && attacker->Accept(defender_visitor)) {
    attacker->Kill();
  }
}
```

**Результат:**
- Оба NPC могут погибнуть
- Один может погибнуть
- Оба могут выжить

---

### 7. Движение NPC

#### Алгоритм движения:

```cpp
// Случайный угол (0-2π)
double angle = angle_dist(gen_local);

// Новые координаты
int new_x = current_x + static_cast<int>(move_dist * std::cos(angle));
int new_y = current_y + static_cast<int>(move_dist * std::sin(angle));

npc->Move(new_x, new_y);
```

**Особенности:**
- Случайное направление (равномерное распределение углов)
- Расстояние движения из характеристик NPC
- Ограничение координат картой (0-100)

#### Расстояния движения:

- **Bear**: 5 единиц
- **Elf**: 10 единиц
- **Robber**: 10 единиц

---

### 8. Связи между компонентами

#### Диаграмма потоков:

```
Main Thread
  ├── PrintMap() каждую секунду
  └── Остановка через 30 секунд

Movement Thread
  ├── Движение NPC каждые 100ms
  ├── Проверка расстояний
  └── Добавление задач в combat_queue_

Combat Thread
  ├── Ожидание задач (condition_variable)
  ├── Обработка боев (Visitor паттерн)
  └── Убийство NPC и уведомление наблюдателей
```

#### Поток данных при бое:

```
1. MovementThread обнаруживает близких NPC
   └── Добавляет CombatTask в очередь

2. CombatThread получает задачу
   └── Проверяет IsAlive() для обоих NPC

3. attacker->Accept(defender_visitor)
   └── defender->Accept(attacker_visitor)
       └── attacker->Visit(defender_typed)
           ├── Проверка IsAlive()
           ├── RollDice() для обоих
           └── return attack > defense

4. Если победа:
   ├── defender->Kill()
   ├── attacker->FightNotify(...)
   └── Observer выводят информацию
```

---

## Важные детали реализации

### 1. Thread-local генераторы случайных чисел

```cpp
int RollDice() const {
  thread_local std::random_device rd;
  thread_local std::mt19937 gen(rd());
  thread_local std::uniform_int_distribution<> dice(1, 6);
  return dice(gen);
}
```

**Почему `thread_local`:**
- Каждый поток имеет свой генератор
- Избегает синхронизации
- Повышает производительность

### 2. Предотвращение deadlock в `IsClose()`

Блокировка в порядке адресов гарантирует одинаковый порядок для всех потоков.

### 3. Копирование наблюдателей в `FightNotify()`

Предотвращает deadlock, если наблюдатель обращается к NPC.

### 4. Ограничение размера очереди боев

```cpp
if (combat_queue_.size() < 500) {
  combat_queue_.push({npc, other});
}
```

Предотвращает переполнение памяти при высокой нагрузке.

### 5. Атомарный флаг `running_`

```cpp
std::atomic<bool> running_;
```

Не требует мьютекса, безопасен для чтения/записи из разных потоков.

---

## Отличия от Lab 6

1. **Многопоточность:**
   - Три параллельных потока
   - Синхронизация через мьютексы и condition variables

2. **Кубик в бою:**
   - Результат боя определяется броском кубика
   - Правила боя сохранены (кто кого может убить)

3. **Движение NPC:**
   - NPC двигаются случайным образом
   - Разные расстояния движения для разных типов

4. **Статус жизни:**
   - NPC могут быть живы или мертвы
   - Мертвые NPC не двигаются и не участвуют в боях

5. **Временные ограничения:**
   - Игра длится 30 секунд
   - Карта печатается каждую секунду

---

## Заключение

Проект демонстрирует:
- Использование трех паттернов проектирования (Factory, Visitor, Observer)
- Многопоточное программирование с правильной синхронизацией
- Предотвращение deadlock и гонок данных
- Асинхронную обработку боев через очередь задач
- Потокобезопасный доступ к общим ресурсам

Все компоненты связаны через интерфейсы и используют полиморфизм для гибкости и расширяемости кода, при этом обеспечивая безопасность в многопоточной среде.





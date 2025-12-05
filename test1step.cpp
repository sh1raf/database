// test_hashtable_basic.cpp
#include "../../Containers/hashtable.hpp"
#include <iostream>
#include <cassert>
#include <string>

void test_basic_operations_int() {
    std::cout << "=== Тест 1: Базовые операции с целочисленными ключами ===" << std::endl;
    
    ChainHashTable<int, std::string> table;
    
    assert(table.Insert(1, "one") == true);
    assert(table.Insert(2, "two") == true);
    assert(table.Insert(3, "three") == true);
    assert(table.GetSize() == 3);
    
    assert(table.Search(1) == "one");
    assert(table.Search(2) == "two");
    assert(table.Search(3) == "three");
    assert(table.Search(4) == "");

    assert(table.Contains(1) == true);
    assert(table.Contains(4) == false);
    
    std::cout << "✓ Базовые операции с int прошли успешно" << std::endl;
}

void test_basic_operations_string() {
    std::cout << "=== Тест 2: Базовые операции со строковыми ключами ===" << std::endl;
    
    ChainHashTable<std::string, int> table;

    assert(table.Insert("apple", 10) == true);
    assert(table.Insert("banana", 20) == true);
    assert(table.Insert("cherry", 30) == true);
    assert(table.GetSize() == 3);
    
    assert(table.Search("apple") == 10);
    assert(table.Search("banana") == 20);
    assert(table.Search("cherry") == 30);
    assert(table.Search("date") == 0);

    assert(table.Contains("apple") == true);
    assert(table.Contains("date") == false);
    
    std::cout << "✓ Базовые операции со string прошли успешно" << std::endl;
}

void test_collision_handling() {
    std::cout << "=== Тест 3: Обработка коллизий ===" << std::endl;
    
    ChainHashTable<int, std::string> table(5);

    assert(table.Insert(1, "first") == true);
    assert(table.Insert(6, "sixth") == true);
    assert(table.Insert(11, "eleventh") == true);
    
    assert(table.GetSize() == 3);
    
    assert(table.Search(1) == "first");
    assert(table.Search(6) == "sixth");
    assert(table.Search(11) == "eleventh");
    
    std::cout << "✓ Обработка коллизий прошла успешно" << std::endl;
}

void test_copy_and_assignment() {
    std::cout << "=== Тест 4: Копирование и присваивание ===" << std::endl;
    
    ChainHashTable<std::string, int> original;
    original.Insert("one", 1);
    original.Insert("two", 2);
    original.Insert("three", 3);

    ChainHashTable<std::string, int> copy(original);
    assert(copy.GetSize() == 3);
    assert(copy.Search("one") == 1);
    assert(copy.Search("two") == 2);
    assert(copy.Search("three") == 3);

    ChainHashTable<std::string, int> assigned;
    assigned = original;
    assert(assigned.GetSize() == 3);
    assert(assigned.Search("one") == 1);
    assert(assigned.Search("two") == 2);
    assert(assigned.Search("three") == 3);
    
    std::cout << "✓ Копирование и присваивание прошли успешно" << std::endl;
}

int main() {
    std::cout << "ЗАПУСК ТЕСТОВ ДЛЯ ЭТАПОВ 1-2" << std::endl;
    std::cout << "============================" << std::endl;
    
    test_basic_operations_int();
    test_basic_operations_string();
    test_collision_handling();
    test_copy_and_assignment();
    
    std::cout << std::endl << "============================" << std::endl;
    std::cout << "ВСЕ ТЕСТЫ ЭТАПОВ 1-2 ПРОЙДЕНЫ УСПЕШНО!" << std::endl;
    
    return 0;
}
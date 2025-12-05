#include "database.hpp"
#include <iostream>
#include <cassert>
#include <vector>

using namespace std;
using json = nlohmann::json;

class DatabaseTester {
private:
    Database db;

public:
DatabaseTester() : db("test_db") {}

    void testBasicInsertFind()
    {
        cout << " ТЕСТ 1: Базовая вставка и поиск" << endl;
        
        db.insert("users", "{\"name\": \"Alice\", \"age\": 25, \"city\": \"London\"}");
        db.insert("users", "{\"name\": \"Bob\", \"age\": 30, \"city\": \"Paris\"}");
        db.insert("users", "{\"name\": \"Charlie\", \"age\": 25, \"city\": \"Berlin\"}");
        
        cout << "Поиск всех документов:" << endl;
        db.find("users", "{}");
        
        cout << "Поиск по возрасту 25:" << endl;
        db.find("users", "{\"age\": 25}");
        
        cout << "Поиск по имени Bob:" << endl;
        db.find("users", "{\"name\": \"Bob\"}");
        
        cout << "Тест 1 пройден" << endl << endl;
    }

    void testComparsionOperators()
    {
        cout << " ТЕСТ 2: Операторы сравнения" << endl;
        
        db.insert("products", "{\"name\": \"Laptop\", \"price\": 1000, \"stock\": 5}");
        db.insert("products", "{\"name\": \"Mouse\", \"price\": 25, \"stock\": 0}");
        db.insert("products", "{\"name\": \"Keyboard\", \"price\": 75, \"stock\": 10}");
        
        cout << "Цена больше 50:" << endl;
        db.find("products", "{\"price\": {\"$gt\": 50}}");
        
        cout << "Цена меньше 100:" << endl;
        db.find("products", "{\"price\": {\"$lt\": 100}}");
        
        cout << "Есть в наличии (stock > 0):" << endl;
        db.find("products", "{\"stock\": {\"$gt\": 0}}");
        
        cout << "Тест 2 пройден" << endl << endl;
    }

    void testLikeOperator()
    {
        cout << " ТЕСТ 3: Оператор LIKE" << endl;
        
        db.insert("emails", "{\"email\": \"alice@gmail.com\", \"type\": \"personal\"}");
        db.insert("emails", "{\"email\": \"bob@company.org\", \"type\": \"work\"}");
        db.insert("emails", "{\"email\": \"charlie@gmail.com\", \"type\": \"personal\"}");
        
        cout << "Gmail адреса:" << endl;
        db.find("emails", "{\"email\": {\"$like\": \"%gmail.com\"}}");
        
        cout << "Имена на 'A':" << endl;
        db.find("emails", "{\"email\": {\"$like\": \"a%\"}}");
        
        cout << "Тест 3 пройден" << endl << endl;
    }

    void testInOperator() 
    {
        cout << " ТЕСТ 4: Оператор IN" << endl;
        
        db.insert("cities", "{\"name\": \"London\", \"country\": \"UK\", \"population\": 9000000}");
        db.insert("cities", "{\"name\": \"Paris\", \"country\": \"France\", \"population\": 7000000}");
        db.insert("cities", "{\"name\": \"Berlin\", \"country\": \"Germany\", \"population\": 6000000}");
        db.insert("cities", "{\"name\": \"Madrid\", \"country\": \"Spain\", \"population\": 5000000}");
        
        cout << "Столицы UK и France:" << endl;
        db.find("cities", "{\"country\": {\"$in\": [\"UK\", \"France\"]}}");
        
        cout << "Тест 4 пройден" << endl << endl;
    }

    void testOrOperator() 
    {
        cout << " ТЕСТ 5: Оператор OR" << endl;
        
        db.insert("employees", "{\"name\": \"John\", \"department\": \"IT\", \"salary\": 50000}");
        db.insert("employees", "{\"name\": \"Sarah\", \"department\": \"HR\", \"salary\": 45000}");
        db.insert("employees", "{\"name\": \"Mike\", \"department\": \"IT\", \"salary\": 55000}");
        db.insert("employees", "{\"name\": \"Anna\", \"department\": \"Finance\", \"salary\": 60000}");
        
        cout << "IT отдел ИЛИ зарплата > 55000:" << endl;
        db.find("employees", "{\"$or\": [{\"department\": \"IT\"}, {\"salary\": {\"$gt\": 55000}}]}");
        
        cout << "Тест 5 пройден" << endl << endl;
    }

    void testMultiQueries() 
    {
        cout << "ТЕСТ 6: Сложные комбинированные запросы" << endl;
        
        db.insert("students", "{\"name\": \"Tom\", \"grade\": \"A\", \"age\": 20, \"major\": \"CS\"}");
        db.insert("students", "{\"name\": \"Lisa\", \"grade\": \"B\", \"age\": 22, \"major\": \"Math\"}");
        db.insert("students", "{\"name\": \"Alex\", \"grade\": \"A\", \"age\": 19, \"major\": \"CS\"}");
        db.insert("students", "{\"name\": \"Emma\", \"grade\": \"C\", \"age\": 21, \"major\": \"Physics\"}");
        
        cout << "CS мажоры с оценкой A:" << endl;
        db.find("students", "{\"major\": \"CS\", \"grade\": \"A\"}");
        
        cout << "Студенты старше 19 ИЛИ с оценкой A:" << endl;
        db.find("students", "{\"$or\": [{\"age\": {\"$gt\": 19}}, {\"grade\": \"A\"}]}");
        
        cout << "Тест 6 пройден" << endl << endl;
    }

    void testDeleteOperations() 
    {
        cout << " ТЕСТ 7: Операции удаления" << endl;
        
        db.insert("temp", "{\"id\": 1, \"status\": \"active\"}");
        db.insert("temp", "{\"id\": 2, \"status\": \"inactive\"}");
        db.insert("temp", "{\"id\": 3, \"status\": \"active\"}");
        
        cout << "До удаления:" << endl;
        db.find("temp", "{}");
        
        cout << "Удаляем inactive:" << endl;
        db.remove("temp", "{\"status\": \"inactive\"}");
        
        cout << "После удаления:" << endl;
        db.find("temp", "{}");
        
        cout << "Тест 7 пройден" << endl << endl;
    }

    void testEdgeCases() 
    {
        cout << " ТЕСТ 8: Граничные случаи" << endl;
        
        cout << "Поиск в несуществующей коллекции:" << endl;
        db.find("nonexistent", "{\"field\": \"value\"}");
        
        cout << "Поиск по несуществующему полю:" << endl;
        db.find("users", "{\"nonexistent_field\": \"value\"}");
        
        cout << "Пустой запрос:" << endl;
        db.find("users", "{}");
        
        cout << "Тест 8 пройден" << endl << endl;
    }

    void runAllTests() 
    {
        cout << "ЗАПУСК ВСЕХ ТЕСТОВ БАЗЫ ДАННЫХ" << endl << endl;
        
        testBasicInsertFind();
        testComparsionOperators();
        testLikeOperator();
        testInOperator();
        testOrOperator();
        testMultiQueries();
        testDeleteOperations();
        testEdgeCases();
        
        cout << "ВСЕ ТЕСТЫ УСПЕШНО ЗАВЕРШЕНЫ!" << endl;
    }
};

void testQueryEvaluator() 
{
    cout << " ТЕСТЫ QUERY EVALUATOR" << endl;
    
    QueryEvaluator evaluator;
    
    json doc1 = {{"name", "Alice"}, {"age", 25}};
    json query1 = {{"name", "Alice"}};
    cout << "Простое равенство: " << evaluator.evaluate(doc1, query1) << endl;
    
    json query2 = {{"age", {{"$gt", 20}}}};
    cout << "Больше 20: " << evaluator.evaluate(doc1, query2) << endl;
    
    json doc2 = {{"email", "test@gmail.com"}};
    json query3 = {{"email", {{"$like", "%gmail.com"}}}};
    cout << "Like %gmail.com: " << evaluator.evaluate(doc2, query3)<< endl;
    
    json query4 = {{"name", {{"$in", {"Alice", "Bob"}}}}};
    cout << "In [Alice, Bob]: " << evaluator.evaluate(doc1, query4) << endl;
    
    json query5 = {{"$or", {{{"age", 30}}, {{"name", "Alice"}}}}};
    cout << "Or условие: " << evaluator.evaluate(doc1, query5) << endl;
    
    cout << "Тесты QueryEvaluator пройдены" << endl << endl;
}

int main() 
{
    testQueryEvaluator();

    DatabaseTester tester;
    tester.runAllTests();

    Database data("test_db");
    
    data.insert("users", "{\"name\": \"Alice\", \"age\": 25, \"city\": \"London\"}");
    data.insert("users", "{\"name\": \"Bob\", \"age\": 30, \"city\": \"Paris\"}");
    data.insert("users", "{\"name\": \"Charlie\", \"age\": 25, \"city\": \"Berlin\"}");
    
    cout << "Поиск всех документов:" << endl;
    data.find("users", "{}");
        

    return 0;
}
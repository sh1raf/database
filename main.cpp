#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <chrono>

#include "../../Containers/hashtable.hpp"
#include "../../Containers/Go/vector.h"
#include "database.hpp"

using nlohmann::json;

void printUsage() 
{
    cout << "Usage:" << endl;
    cout << "  ./program <database> insert '<json_document>'" << endl;
    cout << "  ./program <database> find '<json_query>'" << endl;
    cout << "  ./program <database> delete '<json_query>'" << endl;
    cout << "  ./program <database> create_index <field_name>" << endl;
    cout << endl;
    cout << "Examples:" << endl;
    cout << "  ./program mydb insert '{\"name\": \"Alice\", \"age\": 25}'" << endl;
    cout << "  ./program mydb find '{\"age\": 25}'" << endl;
    cout << "  ./program mydb find '{\"age\": {\"$gt\": 20}}'" << endl;
    cout << "  ./program mydb delete '{\"name\": \"Alice\"}'" << endl;
    cout << "  ./program mydb create_index age" << endl;
}

int main(int argc, char *argv[])
{
    auto start = chrono::high_resolution_clock::now();

    if (argc < 4) 
    {
        if (argc == 2 && string(argv[1]) == "--help") 
        {
            printUsage();

            return 0;
        }

        cerr << "Error: Invalid number of arguments." << endl;
        printUsage();

        return 1;
    }
    
    try 
    {
        string databaseName = argv[1];
        string command = argv[2];
        string argument = argv[3];
        
        Database db(databaseName);
        
        if (command == "insert") 
        {
            db.insert(databaseName, argument);
        }
        else if (command == "find") 
        {
            myVector<Document> docs;
            docs = db.find(databaseName, argument);
            
            if (docs.size() > 0)
            {
                cout << "was finded " << docs.size() << " document(s)" << endl;

                for (size_t i = 0; i < docs.size(); i++)
                {
                    cout << docs[i].getData().dump(2) << endl;
                }
            }
            else
            {
                cout << "documents not found" << endl;
            }
        }
        else if (command == "delete") 
        {
            db.remove(databaseName, argument);
        }
        else 
        {
            cerr << "Error: Unknown command '" << command << "'" << endl;
            printUsage();
            return 1;
        }
    }
    catch (const exception& e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = end - start;

    cout << endl << endl << "Operation was ended for " << chrono::duration_cast<chrono::microseconds>(duration).count()<< " microseconds" << endl;

    return 0;
}

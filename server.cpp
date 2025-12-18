#define DEFAULT_PORT 8080
#define DB_OPERATION_TIMEOUT_SEC 5

#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <map>
#include <sstream>
#include <cstring>
#include <signal.h>
#include <chrono>
#include "../database.hpp"
#include "../../../Containers/hashtable.hpp"

using namespace std;

myVector<pair<string, Database*>> databases;
int serverSocket = 0;
mutex dbMutex;

json createResponse(const string& status, const string& message, const json& data = json::array(), int count = 0) 
{
    json response;
    response["status"] = status;
    response["message"] = message;
    response["data"] = data;
    response["count"] = count;
    return response;
}

Database* getOrCreateDatabase(const string& dbName) 
{
    lock_guard<mutex> lock(dbMutex);
    
    for (size_t i = 0; i < databases.size(); i++) 
    {
        if (databases[i].first == dbName) 
        {
            return databases[i].second;
        }
    }

    Database* newDb = new Database(dbName);
    databases.push_back(make_pair(dbName, newDb));
    return newDb;
}

json proccessRequest(Database* db, const string& commandStr, const string& databaseName) 
{
    stringstream ss(commandStr);
    string operation, collectionName, rest;
    
    getline(ss, operation, ' ');
    getline(ss, collectionName, ' ');
    getline(ss, rest);
    
    auto startTime = chrono::steady_clock::now();
    try 
    {
        if (operation == "INSERT") 
        {
            lock_guard<mutex> lock(dbMutex);

            if (db->insert(collectionName, rest) == SUCCESS) 
            {
                auto endTime = chrono::steady_clock::now();
                auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime);
                
                if (duration.count() > DB_OPERATION_TIMEOUT_SEC) 
                {
                    return createResponse("error", "Insert operation timed out after " + to_string(duration.count()) + " seconds");
                }
                
                return createResponse("success", "Document inserted successfully");
            } 
            else 
            {
                return createResponse("error", "Failed to insert document");
            }
        }
        else if (operation == "FIND") 
        {
            myVector<Document> results = db->find(collectionName, rest);
            
            auto endTime = chrono::steady_clock::now();
            auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime);
            
            if (duration.count() > DB_OPERATION_TIMEOUT_SEC) 
            {
                return createResponse("error", "Find operation timed out after " + to_string(duration.count()) + " seconds");
            }
            json resultArray = json::array();

            for (size_t i = 0; i < results.size(); i++) 
            {
                resultArray.push_back(results[i].getData());
            }
            
            return createResponse("success", 
                "Found " + to_string(results.size()) + " documents", 
                resultArray, results.size());
        }
        else if (operation == "DELETE") 
        {
            lock_guard<mutex> lock(dbMutex);

            if (db->remove(collectionName, rest) == SUCCESS) 
            {
                auto endTime = chrono::steady_clock::now();
                auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime);
                
                if (duration.count() > DB_OPERATION_TIMEOUT_SEC) 
                {
                    return createResponse("error", "Delete operation timed out after " + to_string(duration.count()) + " seconds");
                }
                
                return createResponse("success", "Documents deleted successfully");
            } 
            else 
            {
                return createResponse("error", "Failed to delete documents");
            }
        }
        else 
        {
            return createResponse("error", "Unknown operation: " + operation);
        }
    } 
    catch (const exception& e) 
    {
        return createResponse("error", "Operation failed: " + string(e.what()));
    }
}

void handleUser(int userSocket) 
{
    Database* currentDb = nullptr;
    string currentDatabaseName;
    
    try 
    {
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));

        int receivedBytes = recv(userSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (receivedBytes <= 0) 
        {
            json errorResponse = createResponse("error", "Connection error");
            string responseStr = errorResponse.dump() + "\n";
            send(userSocket, responseStr.c_str(), responseStr.length(), 0);
            close(userSocket);
            return;
        }
        
        currentDatabaseName = string(buffer, receivedBytes);
        
        if (currentDatabaseName.empty()) 
        {
            json errorResponse = createResponse("error", "Database name cannot be empty");
            string responseStr = errorResponse.dump() + "\n";

            if (send(userSocket, responseStr.c_str(), responseStr.length(), 0) < 0) 
            {
                cerr << "Failed to send error response" << endl;
            }
            close(userSocket);
            return;
        }

        currentDb = getOrCreateDatabase(currentDatabaseName);

        string welcomeMsg = "Connected to database: " + currentDatabaseName + "\n";
        if (send(userSocket, welcomeMsg.c_str(), welcomeMsg.length(), 0) < 0) 
        {
            cerr << "Failed to send welcome message" << endl;
            close(userSocket);
            return;
        }
        
        cout << "Client connected to database: " << currentDatabaseName << endl;

        while (true) 
        {
            memset(buffer, 0, sizeof(buffer));
            receivedBytes = recv(userSocket, buffer, sizeof(buffer) - 1, 0);
            
            if (receivedBytes <= 0) 
            {
                if (receivedBytes == 0) 
                {
                    cout << "Client disconnected from database: " << currentDatabaseName << endl;
                } else {
                    cerr << "Receive error: " << strerror(errno) << endl;
                }
                break;
            }
            
            string commandStr = string(buffer, receivedBytes);

            cout << "Received command: " << commandStr << endl;
            
            if (commandStr == "EXIT") 
            {
                string goodbyeMsg = "Disconnected from database\n";
                
                if (send(userSocket, goodbyeMsg.c_str(), goodbyeMsg.length(), 0) < 0)
                {
                    cerr << "Failed to send goodbye message" << endl;
                }
                break;
            }

            json response;
            try 
            {
                response = proccessRequest(currentDb, commandStr, currentDatabaseName);
            } 
            catch (const exception& e) 
            {
                response = createResponse("error", "Request processing failed: " + string(e.what()));
            }
            
            string responseStr = response.dump() + "\n";
            
            if (send(userSocket, responseStr.c_str(), responseStr.length(), 0) < 0) 
            {
                cerr << "Failed to send response to client" << endl;
                break;
            }
            
            cout << "Sent response: " << response["status"] << " - " << response["message"] << endl;
        }
    }
    catch(const exception& e) 
    {
        cerr << "Error handling client: " << e.what() << endl;

        try 
        {
            json errorResponse = createResponse("error", "Server error: " + string(e.what()));
            string responseStr = errorResponse.dump() + "\n";
            send(userSocket, responseStr.c_str(), responseStr.length(), 0);
        } 
        catch (...) {   }
    }
    
    if (currentDb) 
    {
        cout << "Connection closed for database: " << currentDatabaseName << endl;
    }
    
    close(userSocket);
}

int main() 
{
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (serverSocket < 0) 
    {
        cerr << "Error creating socket!" << endl;
        return -1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(DEFAULT_PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    {
        cerr << "Error binding socket!" << endl;
        close(serverSocket);
        return -1;
    }
    
    cout << "Binding socket was successful" << endl;
    
    if (listen(serverSocket, 10) < 0)
    {
        cerr << "Listen failed!" << endl;
        close(serverSocket);
        return -1;
    }
    
    cout << "Server listening on port " << DEFAULT_PORT << endl;
    cout << "Database operation timeout: " << DB_OPERATION_TIMEOUT_SEC << " seconds" << endl;
    cout << "Waiting for connections..." << endl;
    
    while (true) 
    {
        sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
        
        if (clientSocket < 0) 
        {
            cerr << "Error accepting client connection" << endl;
            continue;
        }
        
        string clientAddr = inet_ntoa(clientAddress.sin_addr);
        int clientPort = ntohs(clientAddress.sin_port);
        
        cout << "New client connected from " << clientAddr << ":" << clientPort << endl;
        
        thread clientThread(handleUser, clientSocket);
        clientThread.detach();
    }
    
    close(serverSocket);
    return 0;
}
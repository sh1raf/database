#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include "../database.hpp"

using namespace std;

#define RESPONSE_TIMEOUT_SEC 10

string receive(int clientSocket, int timeoutSec) 
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    struct timeval timeout;
    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = 0;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    int receivedBytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (receivedBytes > 0) 
    {
        return string(buffer, receivedBytes);
    }
    else if (receivedBytes == 0) 
    {
        return "SERVER_DISCONNECTED";
    }
    else 
    {
        return "TIMEOUT_OR_ERROR";
    }
}

void printUsage()
{
    cout << "Usage: ./client --host <host> --port <port> --database <dbname>" << endl;
    cout << "Example: ./client --host localhost --port 8080 --database myDatabase" << endl;
    cout << "Response timeout: " << RESPONSE_TIMEOUT_SEC << " seconds" << endl;
}

void parseArguments(int argc, char* argv[], string& host, int& port, string& databaseName)
{
    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];
        if (i + 1 < argc)
        {
            if (arg == "--host")
            {
                host = argv[++i];
            }
            else if (arg == "--port")
            {
                port = stoi(argv[++i]);
            }
            else if (arg == "--database")
            {
                databaseName = argv[++i];
            }
        }
    }
}

int main(int argc, char* argv[])
{
    int port = 8080;
    string databaseName = "myDatabase";
    string host = "localhost";

    parseArguments(argc, argv, host, port, databaseName);
    
    if (host.empty() || port == 0 || databaseName.empty()) 
    {
        printUsage();
        return -1;
    }
    
    int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket < 0) 
    {
        cout << "Error creating socket" << endl;
        return -1;
    }

    try
    {
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        
        if (host == "localhost") 
        {
            serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
        }
        else
        {
            serverAddress.sin_addr.s_addr = inet_addr(host.c_str());
        }
        
        cout << "Connecting to " << host << ":" << port << "..." << endl;
        
        if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
        {
            cout << "Error connecting to server" << endl;
            close(clientSocket);
            return -1;
        }

        cout << "Connected to " << host << ":" << port << " successful" << endl;

        send(clientSocket, databaseName.c_str(), databaseName.length(), 0);

        string welcomeMsg = receive(clientSocket, RESPONSE_TIMEOUT_SEC);
        
        if (welcomeMsg == "TIMEOUT_OR_ERROR") 
        {
            cout << "Timeout waiting for server welcome" << endl;
            close(clientSocket);
            return -1;
        }
        else if (welcomeMsg == "SERVER_DISCONNECTED") 
        {
            cout << "Server disconnected immediately" << endl;
            close(clientSocket);
            return -1;
        }
        
        cout << "Server: " << welcomeMsg;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Connection error: " << e.what() << '\n';
        close(clientSocket);
        return -1;
    }
    
    bool exit = false;
    cout << "Connected to database: " << databaseName << endl;
    cout << "Response timeout: " << RESPONSE_TIMEOUT_SEC << " seconds" << endl;
    cout << "Enter commands (INSERT collection {'field':'value'}) or EXIT to quit" << endl;
    cout << "> " << flush;

    while (!exit)
    {
        string commandWithArgs;
        getline(cin, commandWithArgs);
        
        if (commandWithArgs.empty()) 
        {
            cout << "> " << flush;
            continue;
        }
        
        if (commandWithArgs == "EXIT")
        {
            exit = true;
            string exitMsg = "EXIT\n";
            send(clientSocket, exitMsg.c_str(), exitMsg.length(), 0);
            break;
        }

        string requestStr = commandWithArgs + "\n";

        send(clientSocket, requestStr.c_str(), requestStr.length(), 0);
        
        auto startTime = chrono::steady_clock::now();
        string response = receive(clientSocket, RESPONSE_TIMEOUT_SEC);
        auto endTime = chrono::steady_clock::now();
        auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime);
        
        if (response == "TIMEOUT_OR_ERROR") 
        {
            cout << "\nError: Server response timeout after " << duration.count() << " seconds" << endl;
            cout << "Server might be busy or operation took too long" << endl;
        }
        else if (response == "SERVER_DISCONNECTED") 
        {
            cout << "\nServer disconnected" << endl;
            break;
        }
        else 
        {
            try 
            {
                json jsonResponse = json::parse(response);
                cout << "\n[SERVER RESPONSE]" << endl;
                cout << "Status: " << jsonResponse.value("status", "unknown") << endl;
                cout << "Message: " << jsonResponse.value("message", "") << endl;
                
                if (jsonResponse.contains("data") && !jsonResponse["data"].empty()) 
                {
                    cout << "Data (" << jsonResponse.value("count", 0) << " documents):" << endl;
                    cout << jsonResponse["data"].dump(2) << endl;
                } 
                else if (jsonResponse.contains("count")) 
                {
                    cout << "Count: " << jsonResponse["count"] << endl;
                }
                
                if (duration.count() >= RESPONSE_TIMEOUT_SEC) 
                {
                    cout << "Warning: Response took " << duration.count() << " seconds (near timeout)" << endl;
                }
            } 
            catch (const json::parse_error&) 
            {
                cout << "\n[SERVER]: " << response << endl;
            }
        }
        
        cout << "> " << flush;
    }

    close(clientSocket);
    cout << "Disconnected from server" << endl;
    return 0;
}
#include <iostream>
#include <fstream>
#include <time.h>
#include <chrono>
#include <bits/stdc++.h>

#define NOOFFILES 3 // No of data loads
#define NOOFFRAMES 8 // No of frames in RAM
#define NOOFREQ 100 // No of page request to be generated

using namespace std;
using namespace std::chrono; // for time stamps


// Programs with different data loads
int fileSize[NOOFFILES] = {128, 256, 512};

// Random data generator for files
char randCharGenerator()
{
    return (char)(rand() % 26 + 97);
}

// Create programs
int createFile(string filename, int size)
{
    ofstream opFile(filename);
    
    for (int i = 0; i < size; i++)
    {
        opFile << randCharGenerator();
    }

    opFile.close();

    return 1;
}

// Data generator
int dataGenerator(int limit)
{
    return rand() % limit;
}

// Read a pg from the program
string readFromFile(string program, int pgNo)
{
    ifstream swapMemory(program);
    swapMemory.seekg(pgNo * 8, ios::beg);
    char A[9];
    swapMemory.read(A, 8);
    A[8] = 0;
    string s(A);
    swapMemory.close();

    return s;
}

// MRU implemention
void mru(string RAM[], unordered_map<int, pair<int, bool>> &pgTable, string program, int size, vector<int> reqStr)
{
    int pgfault = 0, last = 0;
    list<int> dq;                               // Deque to store the order of pages
    unordered_map<int, list<int>::iterator> mp; // Hashmap to store location of loaded pg in the deque

    ofstream log("log_" + program, ios::app);

    log << program << "\n MRU implementation\n";

    auto start = high_resolution_clock::now(); // start time stamp
    for (int i = 0; i < NOOFREQ; i++)
    {
        log << "\nRequested Page " << reqStr[i] << "\n";

        // check for pg fault
        if (!pgTable[reqStr[i]].second)
        {
            pgfault++;

            log << "Status - Page fault\n";
            // If capacity is full
            if (last >= NOOFFRAMES)
            {

                int mruElement = dq.front();
                int outFrameIndex = pgTable[mruElement].first;

                // delete pg table entry
                pgTable[mruElement].first = -1;
                pgTable[mruElement].second = false;

                log << "Removing page " << mruElement << " stored at frame " << outFrameIndex << "\n";

                // delete entry the hashmap
                mp.erase(mruElement);
                dq.pop_front(); // pop from front the latest element inserted in deque

                dq.push_front(reqStr[i]);   // push the new pg from the front into deque
                mp[reqStr[i]] = dq.begin(); // make a entry with the location in deque of pg

                // read the requested pg
                RAM[outFrameIndex] = readFromFile(program, reqStr[i]);

                // make the entry in the page table
                pgTable[reqStr[i]].first = outFrameIndex;
                pgTable[reqStr[i]].second = true;

                log << "Swapping page " << reqStr[i] << " storing at frame " << outFrameIndex << "\n";
            }
            else
            {
                // if capacity of RAM is not full
                //  read the requested pg
                RAM[last] = readFromFile(program, reqStr[i]);

                // make the entry in the page table
                pgTable[reqStr[i]].first = last;
                pgTable[reqStr[i]].second = true;

                dq.push_front(reqStr[i]);   // push the pg into deque
                mp[reqStr[i]] = dq.begin(); // make a entry with the location in deque of pg

                log << "Swapping page " << reqStr[i] << " storing at frame " << last << "\n";

                last++;
            }
        }
        else
        {
            // if pg is already present in RAM
            log << "Status - Page already present in RAM\n";
            log << "Page " << reqStr[i] << " stored at frame " << pgTable[reqStr[i]].first << "\n";

            dq.erase(mp[reqStr[i]]);    // Erase entry in deque
            dq.push_front(reqStr[i]);   // push the entry into front of deque
            mp[reqStr[i]] = dq.begin(); // update entry in hashmap
        }

        // log RAM status
        for (int i = 0; i < NOOFFRAMES; i++)
        {
            log << "Index" << i << " ";
        }
        log << "\n";
        for (int i = 0; i < NOOFFRAMES; i++)
        {
            if (RAM[i].empty())
                log << "Empty ";
            else
                log << RAM[i] << " ";
        }
        log << "\n";
    }
    auto stop = high_resolution_clock::now(); // stop time stamp
    auto duration = duration_cast<milliseconds>(stop - start);

    // Memory used to implement MRU + Page table size
    int memorySize = sizeof(int) * NOOFFRAMES +
                     ((sizeof(int) + sizeof(list<int>::iterator)) * mp.size()) + sizeof(mp) +
                     ((sizeof(int) + sizeof(pair<int, bool>)) * pgTable.size()) + sizeof(pgTable);

    cout << size << "\t\t  " << memorySize << "\t\t  " << duration.count() << "\t\t\t  " << pgfault << "\t\t\t MRU\n";

    log << "Page Faults " << pgfault << "\n";
    log << "\n ===================================== \n";

    log.close();
}

// LRU implemention
void lru(string RAM[], unordered_map<int, pair<int, bool>> &pgTable, string program, int size, vector<int> reqStr)
{
    int pgfault = 0, last = 0;
    list<int> dq;                               // Deque to store the order of pages
    unordered_map<int, list<int>::iterator> mp; // Hashmap to store location of loaded pg in the deque

    ofstream log("log_" + program, ios::app);

    log << program << "\n LRU implementation\n";

    auto start = high_resolution_clock::now(); // start time stamp
    for (int i = 0; i < NOOFREQ; i++)
    {
        log << "\nRequested Page " << reqStr[i] << "\n";

        // check for page fault
        if (!pgTable[reqStr[i]].second)
        {
            pgfault++;
            log << "Status - Page fault\n";

            // If capacity is full
            if (last >= NOOFFRAMES)
            {
                int lruElement = dq.back();
                int outFrameIndex = pgTable[lruElement].first;

                // delete pg table entry
                pgTable[lruElement].first = -1;
                pgTable[lruElement].second = false;

                log << "Removing page " << lruElement << " stored at frame " << outFrameIndex << "\n";

                // delete entry the hashmap
                mp.erase(lruElement);
                dq.pop_back(); // pop from back the first element inserted in deque

                dq.push_front(reqStr[i]);   // push the new pg from the front into deque
                mp[reqStr[i]] = dq.begin(); // make a entry with the location in deque of pg

                // read the requested pg
                RAM[outFrameIndex] = readFromFile(program, reqStr[i]);

                // make the entry in the page table
                pgTable[reqStr[i]].first = outFrameIndex;
                pgTable[reqStr[i]].second = true;

                log << "Swapping page " << reqStr[i] << " storing at frame " << outFrameIndex << "\n";
            }
            else
            {
                // read the requested pg
                RAM[last] = readFromFile(program, reqStr[i]);

                // make the entry in the page table
                pgTable[reqStr[i]].first = last;
                pgTable[reqStr[i]].second = true;

                dq.push_front(reqStr[i]);   // push the pg into deque
                mp[reqStr[i]] = dq.begin(); // make a entry with the location in deque of pg

                log << "Swapping page " << reqStr[i] << " storing at frame " << last << "\n";
                last++;
            }
        }
        else
        {
            // if pg already present in RAM
            log << "Status - Page already present in RAM\n";
            log << "Page " << reqStr[i] << " stored at frame " << pgTable[reqStr[i]].first << "\n";

            dq.erase(mp[reqStr[i]]);    // Erase entry in deque
            dq.push_front(reqStr[i]);   // push the entry into front of deque
            mp[reqStr[i]] = dq.begin(); // update entry in hashmap
        }

        // log RAM status
        for (int i = 0; i < NOOFFRAMES; i++)
        {
            log << "Index" << i << " ";
        }
        log << "\n";
        for (int i = 0; i < NOOFFRAMES; i++)
        {
            if (RAM[i].empty())
                log << "Empty ";
            else
                log << RAM[i] << " ";
        }
        log << "\n";
    }

    auto stop = high_resolution_clock::now(); // stop time stamp
    auto duration = duration_cast<milliseconds>(stop - start);

    // Memory used to implement LRU + Page table size
    int memorySize = sizeof(int) * NOOFFRAMES +
                     ((sizeof(int) + sizeof(list<int>::iterator)) * mp.size()) + sizeof(mp) +
                     ((sizeof(int) + sizeof(pair<int, bool>)) * pgTable.size()) + sizeof(pgTable);

    cout << size << "\t\t  " << memorySize << "\t\t  " << duration.count() << "\t\t\t  " << pgfault << "\t\t\t LRU\n";

    log << "Page Faults " << pgfault << "\n";
    log << "\n ===================================== \n";

    log.close();
}

// FIFO implemenetation
void fifo(string RAM[], unordered_map<int, pair<int, bool>> &pgTable, string program, int size, vector<int> reqStr)
{
    int pgfault = 0, last = 0;
    queue<int> q; // Store pg sequence for FIFO

    ofstream log("log_" + program, ios::app);

    log << program << "\n FIFO implementation\n";

    auto start = high_resolution_clock::now(); // start time stamp
    for (int i = 0; i < NOOFREQ; i++)
    {
        log << "\nRequested Page " << reqStr[i] << "\n";
        // check for page fault
        if (!pgTable[reqStr[i]].second)
        {
            pgfault++;
            log << "Status - Page fault\n";
            string s = readFromFile(program, reqStr[i]); // read pg from the program
            // If capacity of RAM is full
            if (last >= NOOFFRAMES)
            {
                int outFrameIndex = pgTable[q.front()].first;

                // Delete pg table entry
                pgTable[q.front()].first = -1;
                pgTable[q.front()].second = false;

                log << "Removing page " << q.front() << " stored at frame " << outFrameIndex << "\n";

                q.pop(); // Remove last pg from queue

                // Insert the new page
                RAM[outFrameIndex] = s;
                // Create the pg table entty
                pgTable[reqStr[i]].first = outFrameIndex;
                pgTable[reqStr[i]].second = true;
                q.push(reqStr[i]); // push into FIFO queue

                log << "Swapping page " << reqStr[i] << " storing at frame " << outFrameIndex << "\n";
            }
            else
            {
                // if capacity is there insert at last available index
                RAM[last] = s;
                pgTable[reqStr[i]].first = last;
                pgTable[reqStr[i]].second = true;
                q.push(reqStr[i]); // push to FIF) queue

                log << "Swapping page " << reqStr[i] << " storing at frame " << last << "\n";

                last++;
            }
        }
        else
        {
            log << "Status - Page already present in RAM\n";
            log << "Page " << reqStr[i] << " stored at frame " << pgTable[reqStr[i]].first << "\n";
        }

        // log status of RAM
        for (int i = 0; i < NOOFFRAMES; i++)
        {
            log << "Index" << i << " ";
        }
        log << "\n";
        for (int i = 0; i < NOOFFRAMES; i++)
        {
            if (RAM[i].empty())
                log << "Empty ";
            else
                log << RAM[i] << " ";
        }
        log << "\n";
    }

    auto stop = high_resolution_clock::now(); // stop time stamp
    auto duration = duration_cast<milliseconds>(stop - start);

    log << "Page Faults " << pgfault << "\n";
    log << "\n ===================================== \n";
    log.close();

    // Memory used to implement FIFO + Page table size
    int memorySize = sizeof(int) * NOOFFRAMES + ((sizeof(int) + sizeof(pair<int, bool>)) * pgTable.size()) + sizeof(pgTable);

    cout << size << "\t\t  " << memorySize << "\t\t  " << duration.count() << "\t\t\t  " << pgfault << "\t\t\t FIFO\n";
}

int swappingSystem(string program, int size)
{
    string RAM[NOOFFRAMES];
    unordered_map<int, pair<int, bool>> pgTable; // Pg Table stores the frame no and bool stores if the pg is loaded
    int noOfpages;
    vector<int> reqStr;
    ofstream log("log_" + program); // log file to store swap updates

    log << program << " Request String\n";

    noOfpages = size / 8;

    // Create the page request string
    for (int i = 0; i < NOOFREQ; i++)
    {
        int newReq = dataGenerator(noOfpages);
        reqStr.push_back(newReq);
        log << newReq << " ";
    }
    log << "\n";
    log.close();

    // Intialise page table
    for (int i = 0; i < noOfpages; i++)
    {
        pgTable[i].first = -1;
        pgTable[i].second = false;
    }
    fifo(RAM, pgTable, program, size, reqStr);

    // Intialise page table
    for (int i = 0; i < noOfpages; i++)
    {
        pgTable[i].first = -1;
        pgTable[i].second = false;
    }
    lru(RAM, pgTable, program, size, reqStr);

    // Intialise page table
    for (int i = 0; i < noOfpages; i++)
    {
        pgTable[i].first = -1;
        pgTable[i].second = false;
    }
    mru(RAM, pgTable, program, size, reqStr);

    return 1;
}

int main()
{
    srand(time(0));

    string path = "program_";

    // Create programs
    for (int i = 0; i < NOOFFILES; i++)
    {
        string filename = path + to_string(i);
        createFile(filename, fileSize[i]);
    }

    // Swapping system on all the programs created
    for (int i = 0; i < NOOFFILES; i++)
    {
        cout << "Data Load\t Memory Usage\t Processing Time\t Page fault Rate\t Swapping Policy\n";
        cout << "=================================================================================================\n";
        swappingSystem(path + to_string(i), fileSize[i]);
        cout << "\n=================================================================================================\n";
    }

    return 0;
}

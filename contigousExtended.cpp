#include <iostream>
#include <string>
#include <vector>
#include <chrono> // for time stamps
#include <iostream>
#include <unistd.h>
using namespace std;
using namespace std::chrono; // for time stamps
const int BLOCK_SIZE = 4096; // block size in bytes
const int NUM_BLOCKS = 512;  // total number of blocks on the disk

int blocks[NUM_BLOCKS];

struct Block
{
    int data1;
    int data2;
};
int blockContent[NUM_BLOCKS];
struct file
{
    string fileName;
    int startBlock;
};
vector<file> directory;

struct allocate
{
    string fileName;
    int startBlock;
    int endBlock;
    bool isExtend; // if this allocation is extended
    int extention; // first index of extention
};
vector<allocate> allocations;
// takes size of contigous allocation in no of blocks to be intialized and extended
// checks if possible to allocate, returns index of first block of such
// a contigous space if possible, else return -1
int isPossible(int sz)
{
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        int cnt = 0;
        for (int j = i; j < NUM_BLOCKS; j++)
        {
            if (blocks[j] == 0)
                cnt++;
            else
                break;
        }
        if (cnt >= sz)
            return i;
    }
    return -1;
}

void allocateBlocks(string fileName, int noBlocks, int ind)
{
    for (int i = ind; i < ind + noBlocks; i++)
        blocks[i] = 1;
    allocate a;
    a.fileName = fileName;
    a.startBlock = ind;
    a.endBlock = ind + noBlocks - 1;
    a.isExtend = false;
    allocations.push_back(a);
}

// size is in terms of byte here
void initAllocate(string fileName, int size)
{
    auto start = high_resolution_clock::now(); // start time stamp
    int noBlocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    int ind = isPossible(noBlocks);
    if (ind == -1)
    {
        cout << "File named :" << fileName << " cannot be intialized with size :" << size << " bytes\n";
        return;
    }

    file f;
    f.fileName = fileName;
    f.startBlock = ind;
    directory.push_back(f);
    allocateBlocks(fileName, noBlocks, ind);
    auto stop = high_resolution_clock::now();                 // stop time stamp
    auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
    cout << fileName << " got initialized in " << duration.count() << " nanoseconds\n";
}

bool isPresent(string fileName)
{
    for (auto x : directory)
        if (x.fileName == fileName)
            return true;
    return false;
}

void extendAllocate(string fileName, int size)
{
    auto start = high_resolution_clock::now(); // start time stamp
    int noBlocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int ind = isPossible(noBlocks);

    if (ind == -1)
    {
        cout << "File named :" << fileName << " cannot be extended with size :" << size << " bytes\n";
        return;
    }
    if (!isPresent(fileName))
    {
        cout << "error, no such file named : " << fileName << " on disk and hence cannot be extended\n";
        return;
    }

    for (auto &a : allocations)
    {
        if (a.fileName == fileName && a.isExtend == false)
        {
            a.isExtend = true;
            a.extention = ind;
        }
    }
    allocateBlocks(fileName, noBlocks, ind);
    auto stop = high_resolution_clock::now();                 // stop time stamp
    auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
    cout << fileName << " got extended in " << duration.count() << " nanoseconds\n";
}

void readThisAllocation(allocate a)
{
    for (int i = a.startBlock; i <= a.endBlock; i++)
    {
        // read the block data ( refer blockData global array)
    }
}

void readFile(string fileName)
{
    if (!isPresent(fileName))
    {
        cout << "error, no such file named : " << fileName << " on disk and hence cannot be read\n";
        return;
    }
    auto start = high_resolution_clock::now(); // start time stamp
    int cnt = 0;                               // no of blocks in the file
    for (auto a : allocations)
    {
        if (a.fileName == fileName && a.isExtend == false)
        {
            cnt += (a.endBlock - a.startBlock) + 1;
            // done reading
            readThisAllocation(a);
            break;
        }
        if (a.fileName == fileName)
        {
            cnt += (a.endBlock - a.startBlock) + 1;
            // read this allocation
            readThisAllocation(a);
        }
    }
    cout << "read " << cnt << " blocks and " << cnt * BLOCK_SIZE << " bytes of " << fileName << "\n";
    auto stop = high_resolution_clock::now();                 // stop time stamp
    auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
    cout << fileName << " got read in " << duration.count() << " nanoseconds\n";
}

void deleteFile(string fileName)
{
    auto start = high_resolution_clock::now(); // start time stamp
    for (auto it = directory.begin(); it != directory.end(); it++)
    {
        if ((*it).fileName == fileName)
            directory.erase(it);
    }
    for (auto it = allocations.begin(); it != allocations.end(); it++)
    {
        if ((*it).fileName == fileName)
        {
            for (int i = (*it).startBlock; i <= (*it).endBlock; i++)
                blocks[i] = 0;
            allocations.erase(it);
        }
    }
    auto stop = high_resolution_clock::now();                 // stop time stamp
    auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
    cout << fileName << " got deleted in " << duration.count() << " nanoseconds\n";
}

int main()
{
    freopen("log.txt", "a", stdout);

    initAllocate("file1.txt", 8192);
    initAllocate("file2.txt", 16384);
    initAllocate("file3.txt", 4096);
    initAllocate("file4.txt", 24576);
    readFile("file1.txt");
    readFile("file3.txt");
    long max_rss = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);

    // Convert to megabytes
    double max_rss_mb = max_rss / (1024.0 * 1024.0);

    cout << "Memory used by program: " << max_rss_mb << " MB" << endl;
    int totalBlocks = 0;
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (blocks[i])
            totalBlocks++;
    }
    cout << "Total blocks used : " << totalBlocks << endl;

    cout << "\n-----------end of contiguous extended---------------------\n";

    return 0;
}
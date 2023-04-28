#include <iostream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

const int BLOCK_SIZE = 4096; // block size in bytes
const int NUM_BLOCKS = 512;  // total number of blocks on the disk
int free_list_head = -1;
int free_block_count = 0;

struct Block
{
    bool used = false;
    int next_block = -1;
};

struct File
{
    string name;
    int start_block = -1;
    int file_size = 0;
};

class FileSystem
{
private:
    vector<Block> blocks;
    vector<File> directory;

public:
    FileSystem()
    {
        blocks.resize(NUM_BLOCKS);
        for (int i = 0; i < blocks.size(); i++)
        {
            freeBlock(i);
        }
    }

    bool createOrModifyFile(string name, int size)
    {
        auto start = high_resolution_clock::now(); // start time stamp
        int num_blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        int first_block = -1;

        // check if there is enough space
        // int free_blocks = 0;

        if (num_blocks_needed > free_block_count)
            first_block = -1;
        else
            first_block = free_list_head;

        if (first_block == -1)
        {
            auto stop = high_resolution_clock::now();                  // stop time stamp
            auto duration = duration_cast<microseconds>(stop - start); // calculate duration in microseconds
            cout << "Failed to create/modify " << name << " (not enough space) in " << duration.count() << " microseconds" << endl;
            return false;
        }

        // free blocks allocated to existing file with same name
        for (int i = 0; i < directory.size(); i++)
        {
            if (directory[i].name == name)
            {
                int block = directory[i].start_block;
                while (block != -1)
                {
                    int next_block = blocks[block].next_block;
                    freeBlock(block);
                    block = next_block;
                }
                directory.erase(directory.begin() + i);
                break;
            }
        }

        // allocate blocks to new file
        int block = getFreeBlock();
        int prev_block = block;
        directory.push_back(File{name, block, size});

        for (int i = 0; i < num_blocks_needed - 1; i++)
        {
            blocks[block].next_block = -1;
            blocks[prev_block].next_block = getFreeBlock();
            prev_block = block;
            block = blocks[prev_block].next_block;
        }

        auto stop = high_resolution_clock::now();                  // stop time stamp
        auto duration = duration_cast<microseconds>(stop - start); // calculate duration in microseconds
        cout << "Created/modified " << name << " (size: " << size << " bytes) in " << duration.count() << " microseconds" << endl;
        return true;
    }

    bool deleteFile(string name)
    {
        auto start = high_resolution_clock::now(); // start time stamp
        for (int i = 0; i < directory.size(); i++)
        {
            if (directory[i].name == name)
            {
                int block = directory[i].start_block;
                while (block != -1)
                {
                    int next_block = blocks[block].next_block;
                    freeBlock(block);
                    block = next_block;
                }
                directory.erase(directory.begin() + i);
                auto stop = high_resolution_clock::now();                  // stop time stamp
                auto duration = duration_cast<microseconds>(stop - start); // calculate duration in microseconds
                cout << "Deleted " << name << " in " << duration.count() << " microseconds" << endl;
                return true;
            }
        }
        auto stop = high_resolution_clock::now();                  // stop time stamp
        auto duration = duration_cast<microseconds>(stop - start); // calculate duration in microseconds
        cout << "Failed to delete " << name << " (not found) in " << duration.count() << " microseconds" << endl;
        return false;
    }

    void printDirectory()
    {
        cout << "Directory:" << endl;
        for (const auto &file : directory)
        {
            cout << "- " << file.name << " (size: " << file.file_size << " bytes, start block: " << file.start_block << ")" << endl;
        }
    }

private:
    void freeBlock(int block_num)
    {
        // Mark the block as unused
        blocks[block_num].used = false;
        // Add the block to the front of the free list
        blocks[block_num].next_block = free_list_head;
        free_list_head = block_num;
        free_block_count++;
    }

    int getFreeBlock()
    {
        if (free_block_count == -1)
            return -1;

        int free_block = free_list_head;

        free_list_head = blocks[free_list_head].next_block;
        free_block_count--;
        // Remove free block from list

        blocks[free_block].used = true;
        blocks[free_block].next_block = -1;

        return free_block;
    }
};

int main()
{
    FileSystem fs;

    cout << "\n------------------------------------------start-------------------------------------------------\n";

    // create or modify files
    fs.createOrModifyFile("file1.txt", 8192);
    fs.createOrModifyFile("file2.txt", 16384);
    fs.createOrModifyFile("file3.txt", 4096);
    fs.createOrModifyFile("file4.txt", 24576);

    // print directory
    fs.printDirectory();

    // delete a file
    fs.deleteFile("file2.txt");

    // print directory
    fs.printDirectory();

    // create a new file that exceeds the disk size
    fs.createOrModifyFile("file5.txt", 200000000);

    // print directory
    fs.printDirectory();

    cout << "\n--------------------------------Done---------------------------------------------\n";

    return 0;
}
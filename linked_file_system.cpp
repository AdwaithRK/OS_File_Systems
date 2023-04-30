#include <iostream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

const int BLOCK_SIZE = 4096; // block size in bytes
const int NUM_BLOCKS = 512;  // total number of blocks on the disk

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
    }

    bool createOrModifyFile(string name, int size)
    {
        auto start = high_resolution_clock::now(); // start time stamp
        int num_blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        int first_block = -1;
        int prev_block = -1;

        // check if there is enough space
        int free_blocks = 0;
        for (int i = 0; i < NUM_BLOCKS; i++)
        {
            if (!blocks[i].used)
            {
                free_blocks++;
                if (free_blocks == num_blocks_needed)
                {
                    first_block = i - num_blocks_needed + 1;
                    break;
                }
            }
            else
            {
                free_blocks = 0;
            }
        }

        if (first_block == -1)
        {
            auto stop = high_resolution_clock::now();                  // stop time stamp
            auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
            cout << "Failed to create/modify " << name << " (not enough space) in " << duration.count() << " nanoseconds" << endl;
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
                    blocks[block].used = false;
                    blocks[block].next_block = -1;
                    block = next_block;
                }
                directory.erase(directory.begin() + i);
                break;
            }
        }

        // allocate blocks to new file
        int block = first_block;
        for (int i = 0; i < num_blocks_needed; i++)
        {
            blocks[block].used = true;
            blocks[block].next_block = -1;
            if (prev_block != -1)
            {
                blocks[prev_block].next_block = block;
            }
            else
            {
                directory.push_back(File{name, block, size});
            }
            prev_block = block;
            block++;
        }

        auto stop = high_resolution_clock::now();                  // stop time stamp
        auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
        cout << "Created/modified " << name << " (size: " << size << " bytes) in " << duration.count() << " nanoseconds" << endl;
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
                    blocks[block].used = false;
                    block = next_block;
                }
                directory.erase(directory.begin() + i);
                auto stop = high_resolution_clock::now();                  // stop time stamp
                auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
                cout << "Deleted " << name << " in " << duration.count() << " nanoseconds" << endl;
                return true;
            }
        }
        auto stop = high_resolution_clock::now();                  // stop time stamp
        auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
        cout << "Failed to delete " << name << " (not found) in " << duration.count() << " nanoseconds" << endl;
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
};

int main()
{
    FileSystem fs;

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

    return 0;
}

#include <iostream>
#include <string>
#include <vector>
#include <chrono> // for time stamps
#include <iostream>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
using namespace std::chrono; // for time stamps

const int BLOCK_SIZE = 4096; // block size in bytes
const int NUM_BLOCKS = 512;  // total number of blocks on the disk

struct File
{
    string name;
    int start_block;
    int num_blocks;
};

class FileSystem
{
private:
    vector<bool> blocks;    // tracks which blocks are free or allocated
    vector<File> directory; // directory of files on the disk

public:
    FileSystem()
    {
        // initialize all blocks as free
        blocks.resize(NUM_BLOCKS, false);
    }

    bool createOrModifyFile(string name, int size)
    {
        auto start = high_resolution_clock::now();                    // start time stamp
        int num_blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE; // round up to nearest block
        int start_block = -1;

        // check if file already exists in the directory
        for (int i = 0; i < directory.size(); i++)
        {
            if (directory[i].name == name)
            {
                // free the old blocks allocated to the file
                int old_start_block = directory[i].start_block;
                int old_num_blocks = directory[i].num_blocks;
                for (int j = old_start_block; j < old_start_block + old_num_blocks; j++)
                {
                    blocks[j] = false;
                }
                directory.erase(directory.begin() + i);
                break;
            }
        }

        // search for a contiguous block of free blocks
        for (int i = 0; i < NUM_BLOCKS; i++)
        {
            if (!blocks[i])
            {
                int j = i + 1;
                while (j < NUM_BLOCKS && !blocks[j])
                {
                    j++;
                }

                if (j - i >= num_blocks_needed)
                {
                    start_block = i;
                    break;
                }

                i = j; // skip to the end of the contiguous free blocks
            }
        }

        // if a contiguous block of free blocks was found, allocate them to the file
        if (start_block >= 0)
        {
            for (int i = start_block; i < start_block + num_blocks_needed; i++)
            {
                blocks[i] = true;
            }
            directory.push_back({name, start_block, num_blocks_needed});
            auto stop = high_resolution_clock::now();                 // stop time stamp
            auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
            cout << "Created or modified " << name << " in " << duration.count() << " nanoseconds" << endl;
            return true;
        }
        else
        {
            auto stop = high_resolution_clock::now();                 // stop time stamp
            auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
            cout << "Failed to create or modify " << name << " in " << duration.count() << " nanoseconds" << endl;
            return false; // not enough contiguous free blocks available
        }
    }

    bool deleteFile(string name)
    {
        auto start = high_resolution_clock::now(); // start time stamp
        for (int i = 0; i < directory.size(); i++)
        {
            if (directory[i].name == name)
            {
                int start_block = directory[i].start_block;
                int num_blocks = directory[i].num_blocks;
                for (int j = start_block; j < start_block + num_blocks; j++)
                {
                    blocks[j] = false; // free the blocks allocated to the file
                }
                directory.erase(directory.begin() + i);                   // remove the file from the directory
                auto stop = high_resolution_clock::now();                 // stop time stamp
                auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
                cout << "Deleted " << name << " in " << duration.count() << " nanoseconds" << endl;
                return true;
            }
        }
        auto stop = high_resolution_clock::now();                 // stop time stamp
        auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
        cout << "Failed to delete " << name << " in " << duration.count() << " nanoseconds" << endl;
        return false; // file not found in the directory
    }

    bool readFile(string name)
    {
        auto start = high_resolution_clock::now(); // start time stamp

        for (const auto &file : directory)
        {
            if (file.name == name)
            {
                cout << "Blocks of file '" << name << "':" << endl;

                // Print the blocks of the file
                for (int i = file.start_block; i < file.start_block + file.num_blocks; ++i)
                {

                    // Print the block's data
                    cout << i << ",";
                }
                cout << "\n";
                auto stop = high_resolution_clock::now();                 // stop time stamp
                auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
                cout << "Read " << name << " in " << duration.count() << " nanoseconds" << endl;
                return true;
            }
        }

        cout << name << " NOT FOUND!!\n";
        auto stop = high_resolution_clock::now();                 // stop time stamp
        auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
        cout << "Failed to read " << name << " (not found) in " << duration.count() << " nanoseconds" << endl;

        return false; // file not found
    }

    void printDirectory()
    {
        cout << "Directory:" << endl;
        for (const auto &file : directory)
        {
            cout << "  " << file.name << " (" << file.num_blocks << " blocks, starting at block " << file.start_block << ")" << endl;
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

    // read existing file
    cout << "Reading file1..." << endl;
    fs.readFile("file1.txt");

    // read non-existing file
    cout << "Reading file6..." << endl;
    fs.readFile("file6.txt");

    // Get the maximum resident set size
    ifstream status("/proc/self/status");
    if (!status)
    {
        cerr << "Error: could not open /proc/self/status" << endl;
        return 1;
    }
    string line;
    while (getline(status, line))
    {
        if (line.compare(0, 7, "VmSize:") == 0)
        {
            long long mem_size = stoll(line.substr(7)); // convert from KB to bytes
            cout << "Total used memory: " << mem_size << " kilo bytes" << endl;
            break;
        }
    }
    status.close();

    return 0;
}

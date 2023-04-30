#include <iostream>
#include <vector>
#include <chrono>
using namespace std;
using namespace std::chrono;

const int DISK_SIZE = 1024 * 1024; // 1 MB
const int BLOCK_SIZE = 4096;       // 4 KB
const int NUM_BLOCKS = DISK_SIZE / BLOCK_SIZE;

struct File
{
    string name;
    int start_block;
    int file_size;
    vector<int> blocks;
};

vector<File> directory;
bool blocks[NUM_BLOCKS];

class FileSystem
{
public:
    FileSystem()
    {
        for (int i = 0; i < NUM_BLOCKS; i++)
        {
            blocks[i] = false;
        }
    }

    bool createOrModifyFile(string name, int file_size)
    {
        auto start = high_resolution_clock::now();                  // start time stamp
        int num_blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE; // round up division
        if (num_blocks > NUM_BLOCKS)
        {
            auto stop = high_resolution_clock::now();                  // stop time stamp
            auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
            cout << "Failed to create/modify " << name << " (file size exceeds disk capacity) in " << duration.count() << " nanoseconds" << endl;
            return false;
        }
        for (int i = 0; i < directory.size(); i++)
        {
            if (directory[i].name == name)
            {
                vector<int> blocks = directory[i].blocks;
                for (int j = 0; j < blocks.size(); j++)
                {
                    int block = blocks[j];
                    blocks[block] = false; // free the blocks previously allocated to the file
                }
                directory.erase(directory.begin() + i); // remove the file from the directory
                break;
            }
        }
        vector<int> blocks;
        int num_blocks_allocated = 0;
        while (num_blocks_allocated < num_blocks)
        {
            int block = findFreeBlock();
            if (block == -1)
            {
                for (int i = 0; i < blocks.size(); i++)
                {
                    int block = blocks[i];
                    blocks[block] = false; // free the blocks allocated to the file
                }
                auto stop = high_resolution_clock::now();                  // stop time stamp
                auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
                cout << "Failed to create/modify " << name << " (disk space not available) in " << duration.count() << " nanoseconds" << endl;
                return false;
            }
            blocks.push_back(block);
            blocks[block] = true; // mark block as used by the file
            num_blocks_allocated++;

        }
        directory.push_back({name, blocks[0], file_size, blocks}); // add the file to the directory
        auto stop = high_resolution_clock::now();                  // stop time stamp
        auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
        cout << "Created/modified " << name << " (file size: " << file_size << " bytes) in " << duration.count() << " nanoseconds" << endl;
        return true;
    }

    bool deleteFile(string name)
    {
        auto start = high_resolution_clock::now(); // start time stamp
        for (int i = 0; i < directory.size(); i++)
        {
            if (directory[i].name == name)
            {
                vector<int> blocks = directory[i].blocks;
                for (int j = 0; j < blocks.size(); j++)
                {
                    int block = blocks[j];
                    blocks[block] = false; // free the blocks allocated to the file
                }
                directory.erase(directory.begin() + i);                    // remove the file from the directory
                auto stop = high_resolution_clock::now();                  // stop time stamp
                auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
                cout << "Deleted " << name << " in " << duration.count() << " nanoseconds" << endl;
                return true;
            }
        }
        auto stop = high_resolution_clock::now();                  // stop time stamp
        auto duration = duration_cast<nanoseconds>(stop - start); // calculate duration in nanoseconds
        cout << "Failed to delete " << name << " (file not found) in " << duration.count() << " nanoseconds" << endl;
        return false;
    }

    void printDirectory()
    {
        cout << "Directory:\n";
        for (int i = 0; i < directory.size(); i++)
        {
            cout << directory[i].name << " (size: " << directory[i].file_size << " bytes, blocks: ";
            vector<int> blocks = directory[i].blocks;
            for (int j = 0; j < blocks.size(); j++)
            {
                cout << blocks[j];
                if (j < blocks.size() - 1)
                {
                    cout << ", ";
                }
            }
            cout << ")\n";
        }
    }

private:
    int findFreeBlock()
    {
        for (int i = 0; i < NUM_BLOCKS; i++)
        {
            if (!blocks[i])
            {
                return i;
            }
        }
        return -1;
    }
};

int main()
{
    FileSystem fileSystem;
    fileSystem.createOrModifyFile("file1.txt", 4096);
    fileSystem.createOrModifyFile("file2.txt", 8192);
    fileSystem.createOrModifyFile("file3.txt", 16384);
    fileSystem.printDirectory();
    fileSystem.createOrModifyFile("file2.txt", 16384);
    fileSystem.printDirectory();
    fileSystem.deleteFile("file1.txt");
    fileSystem.printDirectory();
    return 0;
}

// Self
#include "libsat355.h"

// std
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    // Read .txt files
    // Get them from commandline argument
    // Parse them, then create a std::vector of TLEs
    // Sort by mean motion
    // For similar MM, see if they are close in distance
    // Create a vector of 'train' objects which are themselves, vectors
    // Compare the trains to see if any endpoints actually connect
    // Coelesce the sub-trains together to create a complete train

    if (argc < 2) 
    {
        std::cout << "Please provide a file path." << std::endl;
        return 1;
    }

    std::filesystem::path filePath(argv[1]);

    if (!std::filesystem::exists(filePath)) 
    {
        std::cout << "The file " << filePath << " does not exist." << std::endl;
        return 1;
    }

    std::ifstream fileStream(filePath);
    if (!fileStream) 
    {
        std::cout << "Failed to open the file " << filePath << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(fileStream, line)) 
    {
        std::cout << line << std::endl;
    }
    return 0;
}
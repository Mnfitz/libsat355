// Self
#include "libsat355.h"
#include "app355.h"

// std
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class TLE
{
public:
    TLE(std::string line1, std::string line2, std::string line3) : 
        mLine1(line1),
        mLine2(line2),
        mLine3(line3)
    {
        // Do nothing
    }
    
    TLE() = default;

    std::string getLine1() const 
    { 
        return mLine1; 
    }
    void setLine1(std::string line1) 
    { 
        mLine1 = line1; 
    }

    std::string getLine2() const 
    { 
        return mLine2; 
    }
    void setLine2(std::string line2) 
    { 
        mLine2 = line2; 
    }

    std::string getLine3() const 
    { 
        return mLine3;
    }
    void setLine3(std::string line3) 
    { 
        mLine3 = line3; 
    }

private:
    std::string mLine1;
    std::string mLine2;
    std::string mLine3;
};


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

    std::vector<TLE> tleVector;
    std::string line;
    int lineCount = 0;
    while (std::getline(fileStream, line)) 
    {
        TLE newTLE{};
        lineCount++;

        if ((lineCount % 3) == 1)
        {
            newTLE.setLine1(line);
        }
        else if ((lineCount % 3) == 2)
        {
            newTLE.setLine2(line);
        }
        else if ((lineCount % 3) == 0)
        {
            newTLE.setLine3(line);
            tleVector.push_back(newTLE);
        }
    }
    
    std::cout << "TLE Count: " << tleVector.size() << std::endl;


    return 0;
}
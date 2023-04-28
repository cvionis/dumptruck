#include <sys/stat.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

constexpr int LSB{ 1 };
constexpr int MSB{ 0 };

long getFileSize(const std::string& filename)
{
	// Struct containing status of the provided file
	struct stat statBuffer{}; 
	int retStat{ stat(filename.c_str(), &statBuffer) };

	return (retStat == 0 ? statBuffer.st_size : -1);
}

int getEndianness()
{
	int n{ 1 };
	// Will return 1 if little-endian, 0 if big-endian
	return (*(char*)&n == 1) ;
}

int getNumDigits(int n)
{
	int digitCount{ 1 };

	while (n > 1)
	{
		n /= 10;
		++digitCount;
	}
	return digitCount;
}

// Store the provided number of bytes as unsigned chars in a vector
long readBytesFromFile(std::vector<unsigned char>& bytesBuffer, const std::string& filename)
{
	std::ifstream in(filename, std::ios::binary);
	long fileSize{ getFileSize(filename) };

	int bytesRead{ 0 }; 
	while (bytesRead < fileSize)
	{
		char c{};
		in.get(c);

		bytesBuffer.push_back(static_cast<unsigned char >(c));

		++bytesRead;
	}
	return bytesRead;
}

// Default hex dump: display bytes in endianness of user's system
void displayDefault(const std::vector<unsigned char>& bytes, long byteCount, int rowSize)
{
	int address{ 0 };
	int groupSize{ 2 };
	int currentGroup{ 1 };
	int currentByte{ 0 };

	while (currentGroup < byteCount)
	{
		// Print padded address signifying number of bytes read from beginning of file
		if (currentByte % rowSize == 0)
		{
			std::cout << std::hex
				<< std::setfill('0') << std::setw(7) << std::hex << address << ' '
				<< std::dec;
			address += rowSize;
		}
		// Get low and high bytes for each group of *groupSize* bytes
		int byteH{ static_cast<int>(bytes.at(currentGroup - 1)) };
		int byteL{ static_cast<int>(bytes.at(currentGroup)) };

		// Print two bytes from the dump in the specificed byte order (little-endian or big-endian)
		if (getEndianness() == LSB)
		{
			
			std::cout << std::setfill('0') << std::setw(2) << std::hex << byteL << std::dec;
			std::cout << std::setfill('0') << std::setw(2) << std::hex << byteH << std::dec;
		}
		else if (getEndianness() == MSB)
		{
			std::cout << std::setfill('0') << std::setw(2) << std::hex << byteH << std::dec;
			std::cout << std::setfill('0') << std::setw(2) << std::hex << byteL << std::dec;
		}

		// Formatting: separate bytes by group size with a space and print a newline when rowSize bytes are reached
		if (currentByte % groupSize == 0)
			std::cout << ' ';
		if ((currentByte + 2) % rowSize == 0) // Note: 2 is added to current byte number because it starts w/ value 0.
			std::cout << '\n';

		currentGroup += 1;
		currentByte  += 2;
	}
}
void displayBytes(const std::vector<unsigned char>& bytes, long byteCount)
{
	/* If canonical option present :
		   print canonical
	   else:
		   print default (in which little or big endian is decided)
	*/
	//displayBigEndian(bytes, byteCount);
	displayDefault(bytes, byteCount, 16);
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " filename\n";
		return EXIT_FAILURE;
	}

	std::vector<unsigned char> bytes{};
	if (!readBytesFromFile(bytes, argv[1]))
	{
		std::cout << "Error: Failed to read bytes from file '" << argv[1] << "'\n";
		return EXIT_FAILURE;
	}
	displayBytes(bytes, getFileSize(argv[1]));

	return 0;
}

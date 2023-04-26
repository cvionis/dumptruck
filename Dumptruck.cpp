#include <sys/stat.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

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

// Display bytes in big-endian hexadecimal format
void displayBigEndian(const std::vector<unsigned char>& bytes, long byteCount)
{
	int rowSize{ 8 };
	int address{ 0 };

	for (int i{ 0 }, j{ 1 }, k{ 0 }; i < byteCount; ++i, ++j, ++k)
	{
		if (k % 16 == 0)
		{
			std::cout << std::setfill('0') << std::setw(7) << std::hex << address << ' ';
			address += rowSize * 2;
		}

		int byte{ static_cast<int>(bytes.at(i)) };

		std::cout << std::setfill('0') << std::setw(2) << std::hex << byte << std::dec;

		// Formatting: add spaces between every two bytes and a newline every rowSize bytes
		if (j % 2 == 0)
			std::cout << ' ';
		if (j % (rowSize * 2) == 0)
			std::cout << '\n';
	}
}

// Display bytes in little-endian hexadecimal format
void displayLittleEndian(const std::vector<unsigned char>& bytes, long byteCount)
{
	int rowSize{ 8 };
	int address{ 0 };

	for (int i{ 1 }, j{ 1 }, k{ 0 }; i < byteCount; i += 2, ++j, ++k)
	{
		if (k % 8 == 0)
		{
			std::cout << std::setfill('0') << std::setw(7) << std::hex << address << ' ';
			address += rowSize * 2;
		}

		int byteL{ static_cast<int>(bytes.at(i)) };
		int byteH{ static_cast<int>(bytes.at(i-1)) };

		std::cout << std::setfill('0') << std::setw(2) << std::hex << byteL << std::dec;
		std::cout << std::setfill('0') << std::setw(2) << std::hex << byteH << std::dec;

		// Formatting: add spaces between every two bytes and a newline every rowSize bytes
		if (j % 1 == 0)
			std::cout << ' ';
		if (j % rowSize == 0)
			std::cout << '\n';
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
	displayLittleEndian(bytes, byteCount);
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

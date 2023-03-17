#include <sys/stat.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

/* TODO: Need to remove magic numbers and unnamed constants. */

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
	if (n < 10) return 1;

	int digitCount{ 1 };
	while (n > 1)
	{
		n /= 10;
		++digitCount;
	}
	return digitCount;
}

std::vector<unsigned char> readBytesFromFile(const std::string& filename, long byteCount)
{
	std::vector<unsigned char> bytesBuffer{};
	std::ifstream in(filename, std::ios::binary);
	long fileSize{ getFileSize(filename) };

	// Make sure the number of bytes read doesn't exceed the number of bytes in the file
	if (byteCount > fileSize) byteCount = fileSize;

	char c{};
	while (byteCount > 0)
	{
		in.get(c);
		bytesBuffer.push_back(static_cast<unsigned char >(c));

		--byteCount;
	}
	return bytesBuffer;
}

/* 
   BEFORE MOVING ON: Clean up both print###Endian functions, make them less sloppy and lazy. Remove magic numbers, the use of numbers in 
   each clear (e.g. why are you using 8 instead of 16? what do these numbers mean in this context?). Remove redundancy if possible, and
   add comments.
 */

void printBigEndian(const std::string& filename, const std::vector<unsigned char>& bytesBuffer, long byteCount, int rowSize)
{
	int address{ 0 };
	for (int i{ 0 }, j{ 1 }, k{ 0 }; i < byteCount; ++i, ++j, ++k)
	{
		if (k % 16 == 0)
		{
			std::cout << std::setfill('0') << std::setw(7) << std::hex << address << ' ';
			address += rowSize * 2;
		}

		int byte{ static_cast<int>(bytesBuffer.at(i)) };

		std::cout << std::setfill('0') << std::setw(2) << std::hex << byte << std::dec;

		// Formatting: add spaces between every two bytes and a newline every rowSize bytes
		if (j % 2 == 0)
			std::cout << ' ';
		if (j % (rowSize * 2) == 0)
			std::cout << '\n';
	}
}

void printLittleEndian(const std::string& filename, const std::vector<unsigned char>& bytesBuffer, long byteCount, int rowSize)
{
	int address{ 0 };
	for (int i{ 1 }, j{ 1 }, k{ 0 }; i < byteCount; i += 2, ++j, ++k)
	{
		if (k % 8 == 0)
		{
			std::cout << std::setfill('0') << std::setw(7) << std::hex << address << ' ';
			address += rowSize * 2;
		}

		int byteL{ static_cast<int>(bytesBuffer.at(i)) };
		int byteH{ static_cast<int>(bytesBuffer.at(i-1)) };

		std::cout << std::setfill('0') << std::setw(2) << std::hex << byteL << std::dec;
		std::cout << std::setfill('0') << std::setw(2) << std::hex << byteH << std::dec;

		// Formatting: add spaces between every two bytes and a newline every rowSize bytes
		if (j % 1 == 0)
			std::cout << ' ';
		if (j % rowSize == 0)
			std::cout << '\n';
	}
}

void printHexDump(const std::string& filename, long byteCount, int rowSize)
{
	std::vector<unsigned char> bytesBuffer{ readBytesFromFile(filename, byteCount) };
	int isLittleEndian{ getEndianness() };

	/* This conditional should be inside a "printDefault() function, which is called if no arg is applied or if
	   the user ONLY specifies endianness.

	   The second option should be printAscii()/printCanonical(), which uses big endian (formatted differently), and displays
	   ascii characters, regardless of the host machine's endianness. 
	 */
	if (!isLittleEndian)
		printBigEndian(filename, bytesBuffer, byteCount, rowSize);
	else
		printLittleEndian(filename, bytesBuffer, byteCount, rowSize);
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " filename\n";
		return EXIT_FAILURE;
	}

	// Number of 16-bit groups occupying each row -- NOTE: REMOVE THIS PARAMETER
	constexpr int rowSize{ 8 }; 

	printHexDump(argv[1], 400, rowSize);

	return 0;
}

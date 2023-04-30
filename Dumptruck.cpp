#include <sys/stat.h>

#include <algorithm>
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

// Default dump: display bytes in endianness of user's system
/* TODO: Return a value that can be used to implement error checking/handling */
void displayDefault(const std::vector<unsigned char>& bytes, long byteCount, int rowSize)
{
	int currentGroup{ 1 };
	int currentByte{ 0 };
	int address{ 0 };

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
		// TODO: Figure out how to make this check not run every iteration of this loop -- it's redundant!
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
		std::cout << ' ';

		// Print a newline when rowSize bytes are reached
		if ((currentByte + 2) % rowSize == 0) /* TODO: Figure out why adding 2 works here (same w/ adding 1 in Canonical) */
			std::cout << '\n';

		currentGroup += 2;
		currentByte  += 2;
	}
}

void displayCanonical(const std::vector<unsigned char>& bytes, long byteCount, int rowSize) 
{
	int currentGroup{ 1 };
	int currentByte{ 0 };
	int address{ 0 };
	int currentRowLen{ 0 };

	while (currentByte < byteCount)
	{
		// Print padded address signifying number of bytes read from beginning of file
		if (currentByte % rowSize == 0)
		{
			std::cout << std::hex
				<< std::setfill('0') << std::setw(7) << std::hex << address << ' '
				<< std::dec;
			address += rowSize;
		}
		int byte{ static_cast<int>(bytes.at(currentByte)) };

		std::cout << std::setfill('0') << std::setw(2) << std::hex << byte << std::dec;
		std::cout << ' ';

		if ((currentByte+1) % rowSize == 0) 
			std::cout << '\n';

		++currentByte;
	}
}

void displayASCII(const std::vector<unsigned char>& bytes, long byteCount, int rowSize) 
{
	int currentGroup{ 1 };
	int currentByte{ 0 };
	int address{ 0 };
	int currentRowLen{ 0 };

	while (currentByte < byteCount)
	{
		// Print padded address signifying number of bytes read from beginning of file
		if (currentByte % rowSize == 0)
		{
			std::cout << std::hex
				<< std::setfill('0') << std::setw(7) << std::hex << address << ' '
				<< std::dec;
			address += rowSize;
		}
		int byte{ static_cast<int>(bytes.at(currentByte)) };

		// Make sure the symbols for newline and tab are printed rather than a new line or tab
		if (byte == 0xa)
			std::cout << std::setfill(' ') << std::setw(2) << "\\n" << " ";
		if (byte == 0x9)
			std::cout << std::setfill(' ') << std::setw(2) << "\\t" << " ";

		std::cout << std::setfill(' ') << std::setw(2) << static_cast<char>(byte) << " ";

		if ((currentByte+1) % rowSize == 0) 
			std::cout << '\n';

		++currentByte;
	}
}

void hexDump(const std::vector<unsigned char>& bytes, long byteCount)
{
	std::cout << "Choose an output mode:\n\t1) Default\n\t2) Canonical\n\t3) ASCII\n\n>> ";
	int choice{};
	std::cin >> choice;

	// Temporary mode list for ease of testing different functions
	if (choice == 1)
		displayDefault(bytes, byteCount, 16);
	if (choice == 2)
		displayCanonical(bytes, byteCount, 16);
	if (choice == 3)
		displayASCII(bytes, byteCount, 16);
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
	hexDump(bytes, getFileSize(argv[1]));

	return 0;
}

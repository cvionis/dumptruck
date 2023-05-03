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

		// Print the ASCII representation of the row's bytes at the end of each row
		if (((currentByte + 1) % rowSize == 0) || (currentByte + 1) == byteCount)
		{
			// Start at the bytes at the beginning of the current row
			/* TODO: RATHER THAN USE NEXT COMMENT, STORE SUBTRAHEND IN SUITABLY NAMED VARIABLE */
			int i{ (currentByte + 1) - ((currentByte % rowSize) + 1) }; // Subtrahend gets number of bytes to 'go back' to get to beginning of row

			// Create a sequence of ASCII chars from the row's bytes to append to end of row
			std::string asciiSequence{};
			for (i; i <= currentByte; ++i)
			{
				auto asciiChar{ static_cast<char>(bytes.at(i)) };
				// Convert any newline or tab bytes into periods before including them in string
				if (asciiChar == '\n' || asciiChar == '\t')
					asciiChar = '.';
				asciiSequence += asciiChar;
			}
			// The width of one byte is 2 characters, and each is followed by a space; 2 + 1 = 3
			constexpr int spaceWidth{ 3 }; 
			constexpr int offset{ 2 };
			// Obtain the padding needed to fill the rest of the current row to rowSize
			int rowPadding((spaceWidth * (rowSize - (currentByte % rowSize)) - offset));

			std::cout << std::setfill(' ') << std::setw(rowPadding);
			std::cout << "|" << asciiSequence << "| \n";
		}
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
		{
			byte = '\0';
			std::cout << std::setfill(' ') << std::setw(2) << "\\n" << " ";
		}
		if (byte == 0x9)
		{
			byte = '\0';
			std::cout << std::setfill(' ') << std::setw(2) << "\\t" << " ";
		}

		std::cout << std::setfill(' ') << std::setw(2) << static_cast<char>(byte) << " ";

		if ((currentByte+1) % rowSize == 0) 
			std::cout << '\n';

		++currentByte;
	}
}

int getOption(const std::vector<std::string>& optionsList, const std::string& option)
{
	for (int i{ 0 }; i < optionsList.size(); ++i)
	{
		if (optionsList.at(i) == option)
		{
			// Check if the argument after the discovered option contains a digit; if so, it belongs to that option
			/* TODO: AVOID REPEATED INDEXING -- USE VARIABLE */
			if (std::isdigit(optionsList.at(i + 1).at(0)))
				return std::stoi(optionsList.at(i + 1));
			else
				return 0;
		}
	}
	return -1;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <filename>\n";
		return EXIT_FAILURE;
	}

	std::vector<unsigned char> bytes{};
	if (!readBytesFromFile(bytes, argv[1]))
	{
		std::cout << "Error: Failed to read bytes from file '" << argv[1] << "'\n";
		return EXIT_FAILURE;
	}

	// Convert argv from an array of pointers to a vector of std::strings for easier traversal
	std::vector<std::string> arguments(argv, argv + argc);

	if (getOption(arguments, "-h") == 0)
	{
		std::cout << "Here's a help menu!\n";
	}

	return 0;
}

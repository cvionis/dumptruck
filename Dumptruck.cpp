#include <sys/stat.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace optionTypes
{
	const std::string OPT_NULL{};
	const std::string OPT_EXISTS{ " " };
}

namespace endianness
{
	constexpr int LSB{ 1 };
	constexpr int MSB{ 0 };
}

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
long readBytesFromFile(std::vector<unsigned char>& bytesBuffer, const std::string& filename, const long readSize)
{
	std::ifstream in(filename, std::ios::binary);

	int bytesRead{ 0 }; 
	while (bytesRead < readSize)
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
void displayDefault(const std::vector<unsigned char>& bytes, const long byteCount, const int rowSize)
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
		if (getEndianness() == endianness::LSB)
		{
			std::cout << std::setfill('0') << std::setw(2) << std::hex << byteL << std::dec;
			std::cout << std::setfill('0') << std::setw(2) << std::hex << byteH << std::dec;
		}
		else if (getEndianness() == endianness::MSB)
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

void displayCanonical(const std::vector<unsigned char>& bytes, const long byteCount, const int rowSize) 
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

void displayASCII(const std::vector<unsigned char>& bytes, const long byteCount, const int rowSize) 
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

// Check for the presence of a provided command-line option and return its associated value
std::string getOption(const std::vector<std::string>& optionsList, const std::string& option)
{
	// Start at index 1 to ignore first argument, which is always the program name
	for (int i{ 1 }; i < optionsList.size(); ++i)
	{
		// Option exists in the argument list
		if ((optionsList.at(i) == option))
		{
			// If the option is the last element of the argument list, there's no associated value.
			if (i == optionsList.size()-1)
				return optionTypes::OPT_EXISTS;

			std::string optionValue{ optionsList.at(i + 1) };
			// Check if the option has an associated value after it in the list
			if (optionValue.at(0))
			{
				// If the value after an option isn't another option, there's an associated value
				if (optionValue.at(0) != '-')
					return optionValue;
				// If the value after the option is another option, there's no associated value
				else
					return optionTypes::OPT_EXISTS;
			}
			else
				return optionTypes::OPT_EXISTS;
		}
	}
	return optionTypes::OPT_NULL;
}

void displayUsage()
{
	std::cout << "Usage: Dumptruck [options]\n\nOptions:\n\n";
	std::cout << "-A, --ascii\t\t\tDisplay bytes in ASCII form\n"
	          << "-C, --canonical\t\t\tCanonicalize output (display hex+ASCII)\n"
			  << "-f, --file <filename>\t\tThe name of the file to read from\n"
		      << "-h, --help\t\t\tDisplay usage and options\n"
			  << "-n, --length <length>\t\tRead {length} bytes from file\n"
			  << "-r, --rowsize <size>\t\tSpecify number of bytes in each row of output\n";
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		displayUsage();
		return EXIT_FAILURE;
	}

	// Convert argv from an array of pointers to a vector of std::strings for easier traversal
	const std::vector<std::string> arguments(argv, argv + argc);

	// Check for presence of 'help' ("-h") option
	if (getOption(arguments, "-h") == optionTypes::OPT_EXISTS)
	{
		displayUsage();
		return EXIT_FAILURE;
	}

	// Check for presence of 'file' ("-f") option
	const std::string filename{ getOption(arguments, "-f") };
	if ((filename == optionTypes::OPT_NULL) || (filename == optionTypes::OPT_EXISTS))
	{
		displayUsage();
		return EXIT_FAILURE;
	}

	long fileSize{ getFileSize(filename) };
	long readSize{};
	std::string readSizeOpt{ getOption(arguments, "-n") };
	// Set value of 'readSize' ("-n") option
	if (readSizeOpt == optionTypes::OPT_NULL || readSizeOpt == optionTypes::OPT_EXISTS)
		readSize = getFileSize(filename);
	else 
		readSize = std::stoi(readSizeOpt);

	if (readSize > fileSize) readSize = fileSize;

	std::vector<unsigned char> bytes{};
	if (!readBytesFromFile(bytes, filename, readSize))
	{
		std::cout << "Error: Failed to read bytes from file '" << filename << "'\n";
		return EXIT_FAILURE;
	}

	int rowSize{ 16 };
	std::string rowSizeOpt{ getOption(arguments, "-r") };
	// Set value of 'rowSize' ("-r") option
	if ((rowSizeOpt != optionTypes::OPT_NULL) && rowSizeOpt != (optionTypes::OPT_EXISTS))
		rowSize = std::stoi(rowSizeOpt);

	/* Determine display mode from options
	 * NOTE: If multiple display options are used, the last one typed will be used */
	if (getOption(arguments, "-A") == optionTypes::OPT_EXISTS)
		displayASCII(bytes, readSize, rowSize);
	else if (getOption(arguments, "-C") == optionTypes::OPT_EXISTS)
		displayCanonical(bytes, readSize, rowSize);
	else
		displayDefault(bytes, readSize, rowSize);

	return 0;
}

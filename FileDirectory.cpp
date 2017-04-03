#include "stdafx.h"
#include "FileDirectory.h"
#include <iostream>
#include <iomanip>

using namespace std;

FileDirectory::FileDirectory()
{
	// Initialize all entries in the fileDirectory and FAT16 to be 0, i.e.safe values.
	for (int i = 0; i < 256; i++) FAT16[i] = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 32; j++) fileDirectory[i][j] = 0;
	}
}

//
// Function name: createFile
// Purpose: Create a file with specified file name and number of bytes.
// Inputs: filename[] : file name (char array)
//         numberBytes: number of bytes in the file (file size) 
// Outputs: boolean
//         true : available space for a new file
//         false: no space for a new file
bool FileDirectory::createFile(char filename[], int numberBytes)
{
	int i;
	unsigned short int freeClusters = 0;
	unsigned short int numberClusters = numberBytes / 4;

	// (1) To check if there is an unused entry in the File Directory. (i.e.the first character of the file name in the File Directory is zero). Return false if not true.
	for (i = 0; i < 4; i++)
	{
		if (fileDirectory[i][0] == 0) break;
	}

	// If there is no space for a new file
	if (i == 4)
		return false;

	// (2) To check if there are enough unused clusters to store the file with the numberBytes. Return false if not true.
	for (int k = 0; k < 256; k++)
	{
		if (FAT16[k] == 0 || FAT16[k] == 1) freeClusters++;	// Find unused/free clusters and count how many are available.
	}

	return (numberClusters > freeClusters) ? false : true;
}

//
// Function name: deleteFile
// Purpose: Delete a file with specified file name.
// Inputs: filename[] : file name (char array)
// Outputs: boolean
//         true : file name is matching
//         false: file name is not matching
bool FileDirectory::deleteFile(char filename[])
{
	int i, j, k;
	unsigned short int firstClusterAddress, clusterAddress;

	// (0) To check if the file to be deleted, filename[], is in the Directory. If not, return false.
	for (i = 0; i < 4; i++)
	{
		// If name is different, skip to next one
		for (j = 0; j < 8; j++)
		{
			if (fileDirectory[i][j] != filename[j]) break;
		}

		// If name is matched
		if (j == 8)
		{
			// (1) To change the first character of the file name in the File Directory to be zero.
			fileDirectory[i][0] = 0;

			firstClusterAddress = fileDirectory[i][26] + (fileDirectory[i][27] << 8);
			clusterAddress = firstClusterAddress;

			// (2) To change all entries of the clusters of this file in the FAT to 1, i.e., unused.
			for (k = 0; k < 256 && clusterAddress < 0xFFF8; k++)
			{
				unsigned short int temp = FAT16[clusterAddress];
				FAT16[clusterAddress] = 1;	// Store successive cluster addresses
				clusterAddress = temp;
			}

			// Skip the loop after processing
			break;
		}
	}

	// If there is no matching name, return false.  Otherwise, return true.
	return (i == 4) ? false : true;
}

//
// Function name: readFile
// Purpose: Read data from data[] array from the file with the specified file name.
// Inputs: filename[] : file name (char array)
// Outputs: boolean
//         true : file is found
//         false: file is not found
bool FileDirectory::readFile(char filename[])
{
	// Purpose: to read  of data from data[] array from the file with the specified file name.
	int i, j, k, p;
	unsigned short int firstClusterAddress;
	unsigned short int clusterAddress;
	unsigned short int storeClusterLocation[256];
	unsigned char localData[1024];
	unsigned short int info[10];

	// (1) To check if the file to be read, filename[], is in the Directory. If not, return false.
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (fileDirectory[i][j] != filename[j])	break;
		}

		// Found a file in the directory matching the input file name.
		if (j == 8)
		{
			// (2) Use the file name to get the file information from the File Directory, including date, time, number of bytes and the first cluster address.
			for (int h = 0; h < 10; h++)
			{
				info[h] = fileDirectory[i][22 + h];	// 22+h because time starts at byte 22 and number of bytes ends at byte 31.
			}

			// (3) Use the first cluster address to get all the cluster addresses of this file from the FAT - 16.
			firstClusterAddress = fileDirectory[i][26] + (fileDirectory[i][27] << 8);
			clusterAddress = firstClusterAddress;
			// Work through each next cluster address until we reach the 
			for (k = 0; k < 256 && clusterAddress < 0xFFF8; k++)
			{
				storeClusterLocation[k] = clusterAddress;
				clusterAddress = FAT16[clusterAddress];	//Store successive cluster addresses
			}

			// (4) Use all the cluster addresses to read the data from the disk / flash memory.
			p = 0;
			for (int m = 0; m <= k; m++)
			{
				for (int l = 0; l < 4; l++)	// l is displacement within cluster.  
				{
					localData[p++] = data[storeClusterLocation[m] * 4 + l];	// Multiply by 4 because there are 4 bytes / cluster.
				}
			}
			break;
		}	// End of j loop
	}	// End of i loop
	if (i == 4)	return false;	// If file is not found, return false.
	else return true;
}

//
// Function name: writeFile
// Purpose: Write numberBytes bytes of data from data[] array into the file with the specified file name.
// Inputs: filename[] : file name (char array)
//         numberBytes: number of bytes in the file (file size) 
//         _data[]    : data to be stored in the file
//         year, month, day : File date information
//         hour, minute, second : File time information
// Outputs: boolean
//         true : file write is successful
//         false: no space is available for the file, or file directory is full
bool FileDirectory::writeFile(char filename[], int numberBytes, char _data[], int year, int month, int day, int hour, int minute, int second)
{
	unsigned short int firstClusterAddress;
	unsigned short int nextClusterAddress;
	unsigned short int currentClusterAddress;
	unsigned int empty;

	// Check if space is available
	if (!createFile(filename, numberBytes))
		return false;


	// (0) To look for the first unused entry(0 or 1) in the FAT - 16; and use it as the First Cluster Address.
	int i;
	for (i = 2; i < 256; i++)
	{
		if (FAT16[i] == 0 || FAT16[i] == 1)
		{
			firstClusterAddress = i;
			break;
		}
	}

	// If there is no space available
	if (i == 256)
		return false;

	// (1) To look for the next unused entry(0 or 1) in the FAT - 16; and use it as the Next Cluster Address; and write its value into the FAT - 16.
	int numberClusters = numberBytes / 4 + ((numberBytes % 4 == 0) ? 0 : 1);	// Use number of bytes to find how many clusters required.
	currentClusterAddress = firstClusterAddress;

	// (2) Repeat Step 2 until all clusters are found and the FAT - 16 is updated.
	int dataIndex = 0;
	int previousClusterAddress;

	for (i = firstClusterAddress + 1; i < 256 && numberClusters > 0; i++)
	{
		// If this cluster is empty
		if (FAT16[i] == 0 || FAT16[i] == 1)
		{
			nextClusterAddress = i;
			numberClusters--;

			// Update FAT table to point next cluster address
			FAT16[currentClusterAddress] = nextClusterAddress;

			// Write data to disk
			for (int k = 0; k < 4 && dataIndex < numberBytes; k++)
			{
				data[currentClusterAddress * 4 + k] = _data[dataIndex++];
			}

			// Store previous cluster address for the last cluaster
			previousClusterAddress = currentClusterAddress;

			// Update currentClusterAddress
			currentClusterAddress = nextClusterAddress;
		}
	}

	// Mark end of file
	FAT16[previousClusterAddress] = 0xFFF8;


	// Find empty space in file directory for new file info
	for (empty = 0; empty < 4; empty++)
	{
		if (fileDirectory[empty][0] == 0)
		{
			break;
		}
	}

	// (3) To write / update the file name; extension; date; time; file length and first cluster address into the first unused entry in the File Directory;
	// Write file name into ith entry of the file directory
	for (int j = 0; j < 8; j++)
	{
		fileDirectory[empty][j] = filename[j];
	}
	// Write date into directory[25:24]
	fileDirectory[empty][25] = ((year - 1980) << 1) + (month >> 3);
	fileDirectory[empty][24] = ((month & 0x7) << 5) + day;
	// Write time into directory[23:22]
	fileDirectory[empty][23] = (hour << 3) + (minute >> 3);
	fileDirectory[empty][22] = (minute << 5) + (second >> 1);
	// Write first sector address into directory[27:26]
	fileDirectory[empty][27] = firstClusterAddress >> 8;
	fileDirectory[empty][26] = (unsigned char)firstClusterAddress;
	// Write file length into directory[31:28]
	fileDirectory[empty][31] = numberBytes >> 24;
	fileDirectory[empty][30] = numberBytes >> 16;
	fileDirectory[empty][29] = numberBytes >> 8;
	fileDirectory[empty][28] = numberBytes;

	return true;
}

//
// Function name: printClusters
// Purpose: Print all the clusters of a file.
// Inputs: filename[] : file name (char array)
// Outputs: void
void FileDirectory::printClusters(char filename[])
{
	// Purpose: to print all the clusters of a file.
	int i, j, k;
	unsigned short int firstClusterAddress;
	unsigned short int clusterAddress;
	unsigned short int storeClusterLocation[256];

	// (1) To check if the file to be printed, filename[], is in the Directory. If not, return false.
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (fileDirectory[i][j] != filename[j])	break;
		}

		// Found a file in the directory matching the input file name.
		if (j == 8)
		{
			//(2) Use the file name to get the file information from the File Directory, including the first cluster address.
			firstClusterAddress = fileDirectory[i][26] + (fileDirectory[i][27] << 8);

			// (3) Use the first cluster address to get all cluster addresses from the FAT - 16.
			clusterAddress = firstClusterAddress;
			// Work through each next cluster address until we reach the end of file
			for (k = 0; k < 256 && clusterAddress < 0xFFF8; k++)
			{
				storeClusterLocation[k] = clusterAddress;
				clusterAddress = FAT16[clusterAddress];	// Store successive cluster addresses
				cout << storeClusterLocation[k] << ((clusterAddress < 0xFFF8) ? "->" : ""); 
			}
			break;
		}	// End of j loop
	}	// End of i loop

	cout << endl << endl;
}

//
// Function name: printDirectory
// Purpose: Prints all the files of the directory.
// Inputs: None
// Outputs: void
void FileDirectory::printDirectory()
{
	unsigned short int date, time, length;

	for (int i = 0; i < 4; i++)
	{
		// (1) If the file name is valid, print file name, '.', and file extension. Invalid file name if first char is 0.
		if (fileDirectory[i][0] != 0)
		{
			for (int j = 0; j < 8; j++)
			{
				cout << fileDirectory[i][j];
			}
			cout << '.';
			for (int j = 8; j < 11; j++)
			{
				cout << fileDirectory[i][j];
			}

			// (2) Print date.
			date = (fileDirectory[i][25] << 8) + fileDirectory[i][24];
			cout << "  " << 1980 + ((date & 0xFE00) >> 9);	// Output year
			cout << '/' << ((date & 0x01E0) >> 5);	// Output month
			cout << '/' << (date & 0x001F);	// Output day

			// (3) Print time.
			time = (fileDirectory[i][23] << 8) + fileDirectory[i][22];
			cout << "  " << ((time & 0xF800) >> 11);	// Output hour
			cout << ":" << ((time & 0x07E0) >> 5);	// Output minute
			cout << ":" << ((time & 0x001F) << 1);	// Output second. Shift left by 1 to multiply by 2 because we originally had seconds/2.  

			// (4) Print file length.
			length = (fileDirectory[i][31] << 24) + (fileDirectory[i][30] << 16) + (fileDirectory[i][29] << 8) + fileDirectory[i][28];
			cout << " " << length << " bytes" << endl;
		}
	}
}

//
// Function name: printData
// Purpose: Prints the data of a file.
// Inputs: filename[] : file name (char array)
// Outputs: void
void FileDirectory::printData(char filename[])
{
	// Prints the data of a file.
	int i, j, k, p;
	unsigned short int firstClusterAddress;
	unsigned short int clusterAddress;
	unsigned short int storeClusterLocation[256];
	unsigned char localData[1024];

	// To check if the file to be read, filename[], is in the Directory. If not, quit this module.
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (fileDirectory[i][j] != filename[j])	break;
		}

		// Found a file in the directory matching the input file name.
		if (j == 8)
		{
			// (1) Use the file name to get the file information from the File Directory; including the first cluster address;
			firstClusterAddress = fileDirectory[i][26] + (fileDirectory[i][27] << 8);
			int filesize = ((int)fileDirectory[i][31] << 24) + ((int)fileDirectory[i][30] << 16) + ((int)fileDirectory[i][29] << 8) + (int)fileDirectory[i][28];

			// (2) Use the first cluster address to get all cluster addresses from the FAT - 16;
			clusterAddress = firstClusterAddress;
			// Work through each next cluster address until we reach the end of file
			for (k = 0; k < 256 && clusterAddress < 0xFFF8; k++)
			{
				storeClusterLocation[k] = clusterAddress;
				clusterAddress = FAT16[clusterAddress];	// Store successive cluster addresses
			}

			// (3) Use cluster address to read the data of the file. Use the file length to print these data in hexadecimal format.
			p = 0;
			for (int m = 0; m <= k; m++)
			{
				for (int l = 0; l < 4 && p < filesize; l++)	// l is displacement within cluster.  
				{
					localData[p++] = data[storeClusterLocation[m] * 4 + l];	// Multiply by 4 because there are 4 bytes / cluster.
				}
			}

			// Print out data
			cout << "Data:" << endl;
			for (int i = 0; i < filesize; i++)
			{
				cout << hex << setw(2) << (int)localData[i] << " ";
			}

			// Put it back to dec for other modules
			cout << dec << endl;

			break;
		}	// End of j loop
	}	// End of i loop
	cout << endl << endl << endl;
}
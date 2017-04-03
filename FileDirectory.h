#pragma once
class FileDirectory {
public:
	FileDirectory();	// Initialize all entries in the fileDirectory and FAT16 to be 0; i.e.safe values.

	bool createFile(char filename[], int numberBytes);
	/*
	(1)	To check if there is an unused entry in the File Directory;  (i.e.the first character of the file name in the File Directory is zero).Return false if not true.
	(2)	To check if there are enough unused clusters to store the file with the numberBytes.Return false if not true.
	*/
	bool deleteFile(char filename[]);
	/*
	(0)	To check if the file to be deleted; filename[]; is in the Directory.If not; return false.
	(1)	To change the first character of the file name in the File Directory to be zero;
	(2)	To change all entries of the clusters of this file in the FAT to 1; i.e.; unused.
	*/
	bool readFile(char filename[]);
	/*
	Purpose: to read  of data from data[] array from the file with the specified file name.
	(0)	To check if the file to be deleted; filename[]; is in the Directory.If not; return false.
	(1)	Use the file name to get the file information from the File Directory; including date; time; number of bytes and the first cluster address;
	(2)	Use the first cluster address to get all the cluster addresses of this file from the FAT - 16;
	(3)	Use all the cluster addresses to read the data from the disk / flash memory.
	*/
	bool writeFile(char filename[], int numberBytes, char data[], int year, int month, int day, int hour, int minute, int second);
	/*
	Purpose: to write numberBytes bytes of data from data[] array into the file with the specified file name
	(0)	To look for the first unused entry(0 or 1) in the FAT - 16; and use it as the First Cluster Address.
	(1)	To look for the next unused entry(0 or 1) in the FAT - 16; and use it as the Next Cluster Address; and write its value into the FAT - 16.
	(2)	Repeat Step 2 until all clusters are found and the FAT - 16 is updated.
	(3)	To write / update the file name; extension; date; time; file length and first cluster address into the first unused entry in the File Directory;
	*/
	void printClusters(char filename[]);
	/*
	Purpose: to print all the clusters of a file.
	(1)	To check if the file to be printed; filename[]; is in the Directory.If not; return false.
	(2)	Use the file name to get the file information from the File Directory; including the first cluster address;
	(3)	Use the first cluster address to get all cluster addresses from the FAT - 16;
	*/
	void printDirectory();	// Prints all the  files of the directory.
	/*
	(1)	Use the file name to get the file information from the File Directory; including the first cluster address;
	(2)	Use the first cluster address to get all cluster addresses from the FAT - 16;
	*/
	void printData(char filename[]);	// Prints the data of a file.
	/*
	(1)	Use the file name to get the file information from the File Directory; including the first cluster address;
	(2)	Use the first cluster address to get all cluster addresses from the FAT - 16;
	(3)	Use cluster address to read the data of the file. Use the file length to print these data in hexadecimal format.
	*/

private:
	unsigned char	fileDirectory[4][32];	/* a max of 4 file directories. Each has 32 bytes.
											File Name[7:0]: 8-byte
											File Name Extension[8:10]: 3-byte
											Date[25:24] and Time[23:22] of file creating or modification
											16-bit First sector address[27:26]
											32-bit File size[31:28], for a file size up to 4 Gbytes
											The remaining bytes are for other File attributes  and future expansion. */

	unsigned short int	FAT16[256];		// 256 clusters only
	unsigned char data[1024];			// 256 clusters * 4 bytes/cluster = 1,024 bytes.

};
// CPPsystem.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "FileDirectory.h"
#include <iostream>

using namespace std;

int main()
{
	FileDirectory f1;

	// 1. Create and write a file, file1, of 40 bytes.
	char data1[40];
	for (int i = 0; i < 40; i++) data1[i] = i;
	if (f1.writeFile("file1", 40, data1, 2017, 01, 17, 13, 11, 10) == true)
	{
		f1.printDirectory();
		f1.printClusters("file1");
		f1.printData("file1");
	}
	else
		cout << "File creation failure (file1)" << endl;

	// 2. Create and write a file, file1, of 40 bytes with same data again.
	if (f1.writeFile("file1", 40, data1, 2017, 01, 17, 13, 11, 10) == true)
	{
		f1.printDirectory();
		f1.printClusters("file1");
		f1.printData("file1");
	}
	else
		cout << "File creation failure (file1)" << endl;

	// 3. Create and write a file, file2, of 200 bytes.
	char data2[200];
	for (int i = 0; i < 200; i++) data2[i] = (i + 1);
	if (f1.writeFile("file2", 200, data2, 2017, 10, 10, 10, 10, 10) == true)
	{
		f1.printDirectory();
		f1.printClusters("file2");
		f1.printData("file2");
	}
	else
		cout << "File creation failure (file2)" << endl;

	// 4. Create and write a file, file3, of 300 bytes.
	if (f1.writeFile("file3", 300, "hakjunkim", 2017, 1, 18, 11, 12, 13) == true)
	{
		f1.printDirectory();
		f1.printClusters("file3");
		f1.printData("file3");	// Prints string in ASCII code but after the string, garbage data is printed
								// because file system expected 300 bytes of data, but only part of it is actual data.
	}
	else
		cout << "File creation failure (file3)" << endl;

	// 5. Create and write a file, file4, of 500 bytes.
	if (f1.writeFile("file4", 500, "test4", 2017, 1, 19, 11, 10, 9) == true)
	{
		f1.printDirectory();
		f1.printClusters("file4");
	}
	else
		cout << "File creation failure (file4)" << endl << endl;

	// 6. Delete file2. 1024 bytes max but current total is 1040 bytes. Need to delete file2 to make up some space to perform step 7.
	if (f1.deleteFile("file2") == true)
		cout << "file2 is deleted" << endl << endl;

	// 7. Create and write a file, file4, of 500 bytes.
	if (f1.writeFile("file4", 500, "test4", 2017, 1, 19, 11, 10, 9) == true)
	{
		f1.printDirectory();
		f1.printClusters("file4");
	}
	else
		cout << "File creation failure (file4)" << endl;

	return 0;
}
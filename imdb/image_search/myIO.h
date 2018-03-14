#pragma once
#include <string>
#include <vector>
#include <shlobj.h>
#include <Commdlg.h>
#include <ShellAPI.h>

using namespace std;

struct myIO {
	static int getSubFolders(string& folder, vector<string>& subFolders);
	static int getNames(string &nameW, vector<string> &names, string &dir);
	static int getNamesNE(string& nameWC, vector<string> &names, string &dir, string &ext);
	static BOOL mkDir(string&  path);
	// inline
	static inline string getFolder(string& path);
	static inline string getSubFolder(string& path);
	static inline string getExtention(string name);
	static inline string getNameNE(string& path);
	static inline bool fileExist(string& filePath);
};

// inline functions
string myIO::getFolder(string& path) {
	return path.substr(0, path.find_last_of("\\/") + 1);
}

string myIO::getSubFolder(string& path) {
	string folder = path.substr(0, path.find_last_of("\\/"));
	return folder.substr(folder.find_last_of("\\/") + 1);
}

string myIO::getExtention(string name)
{
	return name.substr(name.find_last_of('.'));
}

string myIO::getNameNE(string& path)
{
	int start = path.find_last_of("\\/") + 1;
	int end = path.find_last_of('.');
	if (end >= 0)
		return path.substr(start, end - start);
	else
		return path.substr(start, path.find_last_not_of(' ') + 1 - start);
}

bool myIO::fileExist(string& filePath)
{
	if (filePath.size() == 0)
		return false;
	DWORD attr = GetFileAttributesA((filePath).c_str());
	return attr == FILE_ATTRIBUTE_NORMAL || attr == FILE_ATTRIBUTE_ARCHIVE;//GetLastError() != ERROR_FILE_NOT_FOUND;
}
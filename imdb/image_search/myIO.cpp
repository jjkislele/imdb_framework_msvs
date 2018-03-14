#include <shlobj.h>
#include <Commdlg.h>
#include <ShellAPI.h>

#include "myIO.h"

using namespace std;

int myIO::getSubFolders(string& folder, vector<string>& subFolders)
{
	subFolders.clear();
	WIN32_FIND_DATAA fileFindData;
	string nameWC = folder + "\\*";
	HANDLE hFind = ::FindFirstFileA(nameWC.c_str(), &fileFindData);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	do {
		if (fileFindData.cFileName[0] == '.')
			continue; // filter the '..' and '.' in the path
		if (fileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			subFolders.push_back(fileFindData.cFileName);
	} while (::FindNextFileA(hFind, &fileFindData));
	FindClose(hFind);
	return (int)subFolders.size();
}

int myIO::getNames(string &nameW, vector<string> &names, string &dir)
{
	dir = getFolder(nameW);
	names.clear();
	names.reserve(10000);
	WIN32_FIND_DATAA fileFindData;
	HANDLE hFind = ::FindFirstFileA((nameW).c_str(), &fileFindData);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	do {
		if (fileFindData.cFileName[0] == '.')
			continue; // filter the '..' and '.' in the path
		if (fileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue; // Ignore sub-folders
		names.push_back(fileFindData.cFileName);
	} while (::FindNextFileA(hFind, &fileFindData));
	FindClose(hFind);
	return (int)names.size();
}

int myIO::getNamesNE(string& nameWC, vector<string> &names, string &dir, string &ext)
{
	int fNum = getNames(nameWC, names, dir);
	ext = getExtention(nameWC);
	for (int i = 0; i < fNum; i++)
		names[i] = getNameNE(names[i]);
	return fNum;
}

BOOL myIO::mkDir(string&  _path)
{
	if (_path.size() == 0)
		return FALSE;

	static char buffer[1024];
	strcpy(buffer, (_path).c_str());
	for (int i = 0; buffer[i] != 0; i++) {
		if (buffer[i] == '\\' || buffer[i] == '/') {
			buffer[i] = '\0';
			CreateDirectoryA(buffer, 0);
			buffer[i] = '/';
		}
	}
	return CreateDirectoryA((_path).c_str(), 0);
}
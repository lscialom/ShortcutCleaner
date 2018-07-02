#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <regex>
#include <iostream>

#include "windows.h"
#include "shobjidl.h"
#include "shlguid.h"
#include "strsafe.h"

#include <shlwapi.h>

static std::vector<std::wstring> filesToDelete;

HRESULT ResolveShortcut(HWND hwnd, LPWSTR lpszLinkFile, LPSTR lpszPath, int iPathBufferSize)
{
	HRESULT hres;
	IShellLink* psl;
	char szGotPath[MAX_PATH];
	char szDescription[MAX_PATH];
	WIN32_FIND_DATA wfd;

	*lpszPath = 0; // Assume failure 

				   // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
				   // has already been called. 
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		// Get a pointer to the IPersistFile interface. 
		hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

		if (SUCCEEDED(hres))
		{
			//WCHAR wsz[MAX_PATH];

			// Ensure that the string is Unicode. 
			//MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH);

			// Add code here to check return value from MultiByteWideChar 
			// for success.

			// Load the shortcut. 
			hres = ppf->Load(lpszLinkFile, STGM_READ);

			if (SUCCEEDED(hres))
			{
				// Get the path to the link target. 
				hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_SHORTPATH);

				if (SUCCEEDED(hres))
				{
					// Get the description of the target. 
					hres = psl->GetDescription(szDescription, MAX_PATH);

					if (SUCCEEDED(hres))
					{
						hres = StringCbCopy(lpszPath, iPathBufferSize, szGotPath);
						if (SUCCEEDED(hres))
						{
							// Handle success
						}
						else
						{
							// Handle the error
						}
					}
				}
			}

			// Release the pointer to the IPersistFile interface. 
			ppf->Release();
		}

		// Release the pointer to the IShellLink interface. 
		psl->Release();
	}
	return hres;
}

void ProcessFile(wchar_t* sPath)
{
	//Check file extension
	if (!std::regex_match(sPath, std::wregex(L".*\\.lnk")))
		return;

	char buffer[MAX_PATH];
	ResolveShortcut(nullptr, sPath, buffer, MAX_PATH);

	if (buffer && buffer[0] != '\0')
	{
		if (!PathFileExistsA(buffer))
		{
			wprintf(L"%s\n", sPath);
			filesToDelete.push_back(sPath);
		}
	}
}

void ProcessDirectory(wchar_t* path)
{
	WIN32_FIND_DATAW fdFile;
	HANDLE hFind = NULL;

	wchar_t sPath[MAX_PATH];

	swprintf(sPath, MAX_PATH, L"%s\\*", path);

	if ((hFind = FindFirstFileW(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", path);
		return;
	}

	//Find will always return "." and ".." as the first two directories. 
	FindNextFileW(hFind, &fdFile);

	while (FindNextFileW(hFind, &fdFile))
	{
		swprintf(sPath, MAX_PATH, L"%s\\%s", path, fdFile.cFileName);

		if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			ProcessDirectory(sPath);
		else
			ProcessFile(sPath);
	} 

	FindClose(hFind);
}

int wmain(int argc, wchar_t* argv[], wchar_t *envp[])
{
	if (argc > 1)
	{
		printf("The following files will be deleted:\n");

		CoInitialize(0);

		for(int i = 1; i<argc; ++i)
		{
			if (GetFileAttributesW(argv[i]) & FILE_ATTRIBUTE_DIRECTORY)
				ProcessDirectory(argv[i]);
			else
				ProcessFile(argv[i]);
		}

		CoUninitialize();

		printf("Proceed ? (y/n)");

		char buffer = '\0';
		std::cin >> buffer;

		if (buffer == 'y')
		{
			for (size_t i = 0; i < filesToDelete.size(); ++i)
			{
				wprintf(L"Deleting %s\n", filesToDelete[i].c_str());
				DeleteFileW(filesToDelete[i].c_str());
			}

			printf("Done.\n");
		}
		else
			printf("Aborted.\n");
	}

	system("pause");

	return 0;
}
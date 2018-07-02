#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "windows.h"
#include "shobjidl.h"
#include "shlguid.h"
#include "strsafe.h"

#include <shlwapi.h>

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
	wprintf(L"File: %s\n", sPath);

	char buffer[MAX_PATH];
	ResolveShortcut(nullptr, sPath, buffer, MAX_PATH);

	if (buffer && buffer[0] != '\0')
	{
		if (!PathFileExistsA(buffer))
		{
			printf("%s not found.\n", buffer);
			wprintf(L"Deleting: %s\n", sPath);

			DeleteFileW(sPath);
		}
	}
}

bool ProcessPath(wchar_t* path)
{
	if (!(GetFileAttributesW(path) & FILE_ATTRIBUTE_DIRECTORY))
	{
		ProcessFile(path);
		return true;
	}

	WIN32_FIND_DATAW fdFile;
	HANDLE hFind = NULL;

	wchar_t sPath[MAX_PATH];

	//Specify a file mask. *.* = We want everything! 
	swprintf(sPath, MAX_PATH, L"%s\\*.*", path);

	if ((hFind = FindFirstFileW(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", path);
		return false;
	}

	do
	{
		//Find first file will always return "."
		//    and ".." as the first two directories. 
		if (wcscmp(fdFile.cFileName, L".") != 0
			&& wcscmp(fdFile.cFileName, L"..") != 0)
		{
			swprintf(sPath, MAX_PATH, L"%s\\%s", path, fdFile.cFileName);

			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				wprintf(L"Directory: %s\n", sPath);
				ProcessPath(sPath);
			}
			else
				ProcessFile(sPath);
		}
	} while (FindNextFileW(hFind, &fdFile));

	FindClose(hFind);

	return true;
}

//TODO Manage argv being a folder ?
int wmain(int argc, wchar_t* argv[], wchar_t *envp[])
{
	if (argc > 1)
	{
		CoInitialize(0);

		for(int i = 1; i<argc; ++i)
			ProcessPath(argv[i]);
	}

	system("pause");

	return 0;
}
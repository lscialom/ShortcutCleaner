#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "windows.h"
#include "shobjidl.h"
#include "shlguid.h"
#include "strsafe.h"

#include <shlwapi.h>

HRESULT ResolveIt(HWND hwnd, LPWSTR lpszLinkFile, LPSTR lpszPath, int iPathBufferSize)
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

//TODO Manage argv being a folder ?
int wmain(int argc, wchar_t* argv[], wchar_t *envp[])
{
	if (argc > 1)
	{
		CoInitialize(0);

		for(int i = 1; i<argc; ++i)
		{
			//Ignore everything but lnk files
			//Not necessary
			//if(std::wstring(wcsrchr(argv[i], '.')) != L".lnk")
			//	continue;

			char buffer[MAX_PATH];
			ResolveIt(nullptr, argv[i], buffer, MAX_PATH);

			if(buffer && buffer[0] != '\0')
			{
				if(!PathFileExistsA(buffer))
				{
					wprintf(L"Deleting: %s\n", argv[i]);
					DeleteFileW(argv[i]);
				}
			}
		}
	}

	system("pause");

	return 0;
}
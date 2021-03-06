#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>


DWORD RvaToOffset(PIMAGE_SECTION_HEADER piSectionHeader, WORD numberOfSections, DWORD rva)
{
	for (WORD i = 0; i < numberOfSections; i++)
	{
		// La RVA est-elle dans cette section?
		if ((rva >= piSectionHeader[i].VirtualAddress) && (rva < piSectionHeader[i].VirtualAddress + piSectionHeader[i].SizeOfRawData))
		{
			rva -= piSectionHeader[i].VirtualAddress;
			rva += piSectionHeader[i].PointerToRawData;

			return rva;
		}
	}

	return -1;
}

void ReadCString(FILE * pfile, LPSTR name)
{
	DWORD n = 0;
	do
	{
		fread(name + n, sizeof(char), 1, pfile);
		n++;
	} while (name[n - 1] != 0 && n < 1023);

	name[n] = 0;
}

std::vector<std::string> GetDllFunList(const std::string & dllPath )
{
	FILE * pfile = NULL;
	char name[1024];
	DWORD namePos = -1;
	DWORD offsetOfEntryExport = -1;
	DWORD offsetOfNames = -1;
	DWORD offsetOfNamePos = -1;

	IMAGE_DOS_HEADER        iDosHeader;
	IMAGE_NT_HEADERS        iNtHeaders;
	PIMAGE_SECTION_HEADER   piSectionHeader;
	IMAGE_EXPORT_DIRECTORY  iExportDir;

	std::vector<std::string> retVec;
	LPSTR lpstrFile = (char *)dllPath.c_str();

	//LVITEM lvI;
	//lvI.mask = LVIF_TEXT;


	pfile = NULL;
	fopen_s(&pfile,lpstrFile, "rb");

	if (pfile == NULL)
	{
		//MessageBoxA(NULL, "Impossible d'ouvrir le fichier.", "Erreur", MB_OK | MB_ICONERROR);
		return retVec;
	}

	fread(&iDosHeader, sizeof(IMAGE_DOS_HEADER), 1, pfile);

	if (iDosHeader.e_magic != IMAGE_DOS_SIGNATURE)
	{
		//MessageBoxA(NULL, "Le fichier n'est pas valide.", "Erreur", MB_OK | MB_ICONERROR);
		fclose(pfile);
		return retVec;
	}

	fseek(pfile, iDosHeader.e_lfanew, SEEK_SET);

	fread(&iNtHeaders, sizeof(IMAGE_NT_HEADERS), 1, pfile);

	if (iNtHeaders.Signature != IMAGE_NT_SIGNATURE)
	{
		//MessageBoxA(NULL, "Le fichier n'est pas valide.", "Erreur", MB_OK | MB_ICONERROR);
		fclose(pfile);
		return retVec;
	}

	piSectionHeader = (PIMAGE_SECTION_HEADER)malloc(sizeof(IMAGE_SECTION_HEADER) * iNtHeaders.FileHeader.NumberOfSections);

	for (unsigned i = 0; i < iNtHeaders.FileHeader.NumberOfSections; i++)
	{
		fread(&piSectionHeader[i], sizeof(IMAGE_SECTION_HEADER), 1, pfile);
	}

	offsetOfEntryExport = RvaToOffset(piSectionHeader, iNtHeaders.FileHeader.NumberOfSections, iNtHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	if (offsetOfEntryExport == -1)
	{
		//MessageBoxA(NULL, "Impossible de trouver la table des fonctions exportées.", "Erreur", MB_OK | MB_ICONERROR);
		fclose(pfile);
		return retVec;
	}

	fseek(pfile, offsetOfEntryExport, SEEK_SET);

	fread(&iExportDir, sizeof(IMAGE_EXPORT_DIRECTORY), 1, pfile);

	offsetOfNames = RvaToOffset(piSectionHeader, iNtHeaders.FileHeader.NumberOfSections, iExportDir.AddressOfNames);

	if (offsetOfNames == -1)
	{
		//MessageBoxA(NULL, "Impossible d'aller à l'adresse des noms.", "Erreur", MB_OK | MB_ICONERROR);
		fclose(pfile);
		return retVec;
	}

	for (DWORD i = 0; i < iExportDir.NumberOfNames; i++)
	{
		fseek(pfile, offsetOfNames + i * sizeof(DWORD), SEEK_SET);
		fread(&namePos, sizeof(DWORD), 1, pfile);

		offsetOfNamePos = RvaToOffset(piSectionHeader, iNtHeaders.FileHeader.NumberOfSections, namePos);

		if (offsetOfNamePos == -1)
		{
			//MessageBoxA(NULL, "Impossible d'aller à l'adresse du nom.", "Erreur", MB_OK | MB_ICONERROR);
			fclose(pfile);
			retVec.clear();
			return retVec;
		}
		fseek(pfile, offsetOfNamePos, SEEK_SET);
		ReadCString(pfile, name);
		retVec.push_back(name);
	}
	fclose(pfile);
	return retVec;
}

extern "C"
{
    /*
        descrpition:get export fun split with ";"
        return:
             0:success
            -1:dllPath is null
            -2:funVec is null
            -3:dll wrong or no export fun
            -4:funVec to small
    */
	int __declspec(dllexport) DllFunView(const char * dllPath,char * funVec,int funVecLen)
	{
		if(!dllPath)
		{
			return -1;
		}
        if(!funVec)
		{
			return -2;
		}
		std::vector<std::string> dlllist = GetDllFunList(dllPath);
		std::string retStr;
        
		for(int i = 0;i < (int)dlllist.size() - 1;++i)
		{
			retStr += dlllist.at(i) + ";";
		}
		if(dlllist.size() > 0)
		{
			retStr += dlllist.at(dlllist.size() - 1);
		}else
        {
            return -3;
        }
        if(funVecLen < retStr.length() + 1)
        {
            return -4;
        }
        memcpy_s(funVec,funVecLen,retStr.c_str(),retStr.length() + 1);
        return 0;
	}
}

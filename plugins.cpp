#include <windows.h>
#include <dirent.h>
#include "plugins.h"
#include <string.h>

void load_plugins(LPCTSTR directory, list<HMODULE>& list_plugins)
{
	DIR *dir;
	struct dirent *ent;
	
	if ((dir = opendir(directory)) != NULL) 
	{
		while((ent = readdir(dir)) != NULL)
		{
			char *str = (char*)calloc(MAX_PATH,sizeof(char));
			strcat(str,directory);
			strcat(str,ent->d_name);
			HMODULE h = LoadLibrary(str);
			if(h != NULL)
				list_plugins.push_back(h);
		}
		closedir (dir);
	}
	return;
}
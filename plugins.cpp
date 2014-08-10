#include <windows.h>
#include <dirent.h>
#include "plugins.h"

void load_plugins(LPCTSTR directory, list<HMODULE>& list_plugins)
{
	DIR *dir;
	struct dirent *ent;
	
	if ((dir = opendir(directory)) != NULL) 
	{
		while((ent = readdir(dir)) != NULL)
			list_plugins.push_back(LoadLibrary(ent->d_name));
		closedir (dir);
	}
	return;
}
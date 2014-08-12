#include <windows.h>
#include <dirent.h>
#include <string.h>

#include "plugins.h"

void load_plugins(LPCTSTR directory, struct WS_plugins *list)
{
	DIR *dir;
	struct dirent *ent;
	struct WS_plugins *t;	

	if ((dir = opendir(directory)) != NULL) 
	{
		while((ent = readdir(dir)) != NULL)
		{
			char *str = (char*)calloc(MAX_PATH,sizeof(char));
			strcat(str,directory);
			strcat(str,ent->d_name);
			HMODULE h = LoadLibrary(str);
			if(h != NULL)
			{
				t = (struct WS_plugins*)malloc(sizeof(struct WS_plugins));
				t->plugin = h;
				list_add(&(t->plugins),&(list->plugins));
			}
		}
		closedir (dir);
	}
	return;
}

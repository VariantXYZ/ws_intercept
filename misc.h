#ifndef MISC_H
#define MISC_H

BOOL apply_patch(BYTE eType, DWORD dwAddress, const void *pTarget,DWORD *orig_size, BYTE *replaced);
BYTE* get_mac_addr();

#endif
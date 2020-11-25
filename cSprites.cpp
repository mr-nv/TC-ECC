//==============================================================================
/*
		drawing sprites from multiple-sprite files by name - by tabris
																			  */
//==============================================================================
#include "cSprites.h"
spritedata_t spritedata;
#pragma warning(disable:4786)
#pragma warning(disable:4800)
#pragma warning(disable:4244)
#pragma warning(disable:4101)
#pragma warning(disable:4715)
#pragma warning(disable:4305)
//==============================================================================
void LoadSprites(client_sprite_t *pList, int iCount)
{
    int i;
    client_sprite_t *p;
    char buf[512];

    spritedata.iSpritesLoaded = 0;

    if(!pList)
    {
        return;
    }

    p = pList;

    i = iCount;

    while(i-- && p->szName[0] != 0x00 && spritedata.iSpritesLoaded < MAX_SPRITES)
    {
        strncpy(spritedata.spriteinfo[spritedata.iSpritesLoaded].szName, p->szName,MAX_SPRITES);
        strncpy(spritedata.spriteinfo[spritedata.iSpritesLoaded].szSprite, p->szSprite,MAX_SPRITES);

        sprintf(buf, "sprites/%s.spr", p->szSprite);
        spritedata.spriteinfo[spritedata.iSpritesLoaded].hspr = gEngfuncs.pfnSPR_Load(buf);

        memcpy(&(spritedata.spriteinfo[spritedata.iSpritesLoaded].rc), &(p->rc), sizeof(p->rc));
        spritedata.spriteinfo[spritedata.iSpritesLoaded].iRes = p->iRes;

        p++;
        spritedata.iSpritesLoaded++;

    }
}
//==============================================================================
client_sprite_t* _cdecl hookpfnSPR_GetList(char *psz, int *piCount)
{
    client_sprite_t *ret;

    ret = gEngfuncs.pfnSPR_GetList(psz, piCount);
	client_sprite_t *pList;
    if(!strcmp(psz, "sprites/WEAPONLIST_knife.txt")) 
    {
        int iCount;
        pList = gEngfuncs.pfnSPR_GetList("sprites/hud.txt", &iCount);
		if(pList)
		{
            LoadSprites(pList, iCount);
		}
    }
    return ret;
}
//==============================================================================
int GetSpriteIndex(char *szName)
{
    int i;

    if(szName[0] == 0x00)
        return -1;

    for(i = 0; i < spritedata.iSpritesLoaded; i++)
    {
        if(spritedata.spriteinfo[i].iRes == 640)
        {
            if(!strcmp(spritedata.spriteinfo[i].szName, szName))
            {
                return i;
            }
        }
    }
}
//==============================================================================
// End.
//==============================================================================
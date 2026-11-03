#include "../FileIO/StdAfx.h"
#include "../FileIO/Streams.h"
#include "../ADOImport/BasicDB.h"
#include "../DBFormat/DataFormat.h"
#include "../DBFormat/DataRPG.h"
#include <cstdio>
#include <cstdlib>
#include <exception>

int main(int argc, char** argv)
{
    if(argc!=3)return 2;
    try {
        CFileStream database;database.OpenRead(argv[1]);
        NDatabase::Serialize(database,CStructureSaver::READ);
        const int id=std::atoi(argv[2]);auto* weapon=NDb::GetWeapon(id);
        if(!weapon || !weapon->pWeaponType)return 3;
        const bool expected=id!=1;const int type=weapon->pWeaponType->nWeaponTypeID;
        std::printf("native weapon dependency record=%d type=%d launcher=%d expected=%d\n",id,type,int(weapon->bBazookaLogic),int(expected));
        if((type==5)!=expected || weapon->bBazookaLogic!=expected){std::fputs("FAIL: weapon logic was derived before its referenced type was imported\n",stderr);return 1;}
        return 0;
    } catch(const std::exception& error){std::fprintf(stderr,"FAIL: %s\n",error.what());return 4;}
}

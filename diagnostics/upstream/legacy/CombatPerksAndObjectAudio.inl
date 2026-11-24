static void ExplosionEvasionPerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=16,"unknown native explosion evasion mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);auto mission=NativeMissionWithUntakenPerk(39);auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();auto* perk=NDb::GetDBPerk(39);Require(perk && !perk->params.empty() && perk->params[0]>0 && perk->params[0]<2 && !mission->HasPerk(40),"explosion evasion fixture lacks authored39 or has40");const float authored=perk->params[0];struct Restore {NDb::CDBPerk* perk;float value;~Restore(){perk->params[0]=value;}}restore{perk,authored};
    const bool acquired=mode!=0 && mode!=4 && mode!=5;const int skill=mode==8?1:mode==9?23:mode==15?140:21;const int damage=mode==8?100:40;const bool intended=mode==13,worn=mode==14;const NRPG::EAttackType attackType=mode==0 || mode==3?NRPG::AT_NORMAL:mode==2 || mode==5 || mode==9 || mode==10 || mode==12 || mode==14 || mode==15?NRPG::AT_FRAGMENT:NRPG::AT_BLAST_WAVE;
    if(mode==6 || mode==7 || mode==8 || mode==9 || mode==13 || mode==14)perk->params[0]=.5f;if(mode==10)perk->params[0]=1.25f;if(mode==11)perk->params[0]=-1;if(mode==12)perk->params[0]=-1.5f;if(mode==15)perk->params[0]=1;if(mode==16)perk->params[0]=0;
    const int effective=mode==6 || mode==7 || mode==13 || mode==14?32:mode==8?2:mode==9?34:mode==10?47:mode==11?0:mode==12?-10:mode==15?280:mode==1 || mode==2?int(std::nearbyint(float((1.0+double(authored))*21))):skill;const int expectedDamage=mode==13 || mode==14 || mode==11?40:mode==6 || mode==7?28:mode==8?98:mode==9?27:mode==10?22:mode==12?44:mode==15?0:mode==1 || mode==2?40-(std::max)(0,int(float(effective*40)*.01f)):32;
    unit->Skills(NDb::ST_IC).SetConst(skill);unit->Skills(NDb::ST_IC).SetProgress(0);unit->Skills(NDb::ST_VP).SetConst(500);unit->nHealedVP=0;const int points=tree->GetPerkPoints();if(acquired){tree->AddPerkPoints(1);tree->TakePerk(39);}Require(mission->HasPerk(39)==acquired && tree->GetPerkPoints()==points && mission->GetIC()==skill && mission->GetTotalVP()==500 && !mission->GetPanzerklein(),"explosion evasion native grant/health/baseIC setup failed");
    CObj<NRPG::CDynamicSkill> shell;if(worn){NDb::CPanzerklein* suit=nullptr;CDBIterator<NDb::CPanzerklein> suits(*NDatabase::GetTable<NDb::CPanzerklein>());while(suits.MoveNext())if(suits.Get()->nMaxVP>0 && (!suit || suits.Get()->GetRecordID()<suit->GetRecordID()))suit=suits.Get();Require(suit,"explosion evasion fixture lacks authored suit");shell=new NRPG::CDynamicSkill(suit->nMaxVP,suit->nMaxVP);mission->SetPanzerklein(suit,shell,nullptr);}
    auto* armor=NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR);Require(armor && armor->pMaterial,"explosion evasion fixture lacks body armor");int damageType=-1;CDBIterator<NDb::CRPGDmgToArmor> types(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());while(types.MoveNext())if(types.Get()->armors[armor->pMaterial->nDR]>0 && (damageType<0 || types.Get()->GetRecordID()<damageType))damageType=types.Get()->GetRecordID();Require(damageType>=0,"explosion evasion fixture lacks flesh damage type");NRPG::CAttackPortion portion(100000,damageType,0.f,damage,damage,0,0,nullptr,intended?mission.GetPtr():nullptr,1.f,-1);portion.atkType=attackType;portion.bBypassPK=worn;Require(portion.CalcStructDmg(nullptr,armor,0)==damage,"explosion evasion fixture lost fixed base damage");
    auto selected=GlobalGameRandom();bool found=false;unsigned int roll=0;for(int i=0;i<10000;++i){auto probe=selected;roll=probe.Get(100);if(mode==6?roll>=21 && roll<32:roll<static_cast<unsigned int>(mode==11?21:(std::min)(skill,effective))){found=true;break;}selected.Get();}Require(found,"explosion evasion fixture lacks selected chance/percentage-isolating roll");GlobalGameRandom()=selected;auto expectedRandom=selected;bool evaded=false;if(!intended && !worn)evaded=expectedRandom.Get(100)<static_cast<unsigned int>(effective);expectedRandom.Get(100);Require(evaded==(!intended && !worn && mode!=11),"explosion evasion golden chance setup failed");const int shellHP=worn?int(*shell):0;auto* inventory=unit->GetInventory();const auto received=mission->ProcessAttack(nullptr,NAI::HL_BODY,&portion,nullptr);auto actualRandom=GlobalGameRandom();bool same=true;for(int i=0;i<4;++i)same&=actualRandom.Get()==expectedRandom.Get();const float practice=unit->Skills(NDb::ST_IC).GetProgress();std::printf("native explosion evasion mode=%d persona=%d acquired=%d attack=%d param=%.9g baseIC=%d effective=%d roll=%u evaded=%d damage=%d expected=%d HPdrop=%d RNGmatches=%d practice=%.9g\n",mode,unit->pPers->GetRecordID(),int(acquired),int(attackType),perk->params[0],skill,effective,roll,int(evaded),received.nDmg,expectedDamage,500-int(unit->Skills(NDb::ST_VP)),int(same),practice);std::fflush(stdout);
    Require(received.nDmg==expectedDamage && received.type==NRPG::RD_HUMAN && 500-int(unit->Skills(NDb::ST_VP))==expectedDamage && same,"native explosion evasion ignored39 in chance/late percentage or changed original rounding/RNG");Require(mission->GetIC()==skill && int(unit->Skills(NDb::ST_IC))==skill && tree->GetPerkPoints()==points && unit->GetInventory()==inventory && !mission->IsDead() && !mission->IsUnconscious() && (!worn || int(*shell)==shellHP) && ((intended || worn)?practice==0:practice>0) && portion.atkType==attackType && portion.nDmgMin==damage && portion.nDmgMax==damage,"explosion evasion changed baseIC/points/inventory/shell/input or omitted scope/practice");
}

static void UnsignedICChance(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=7,"unknown native unsigned IC mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto mission=NativeMissionWithUntakenPerk(40);auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();auto* perk=NDb::GetDBPerk(40);Require(perk && !perk->params.empty() && perk->params[0]==.15f && !mission->HasPerk(39),"unsigned IC fixture lacks authored40/no39");const float authored=perk->params[0];struct Restore {NDb::CDBPerk* perk;float value;~Restore(){perk->params[0]=value;}}restore{perk,authored};
    const bool acquired=mode!=0 && mode!=6;const int skill=mode==6?140:21;int expectedIC=mode==1?24:mode==2 || mode==7?-10:mode==3?-21:mode==4?0:skill;if(mode==2 || mode==7)perk->params[0]=-1.5f;if(mode==3)perk->params[0]=-2;if(mode==4)perk->params[0]=-1;if(mode==5)perk->params[0]=0;unit->Skills(NDb::ST_IC).SetConst(skill);unit->Skills(NDb::ST_IC).SetProgress(0);const int points=tree->GetPerkPoints();if(acquired){tree->AddPerkPoints(1);tree->TakePerk(40);}Require(mission->HasPerk(40)==acquired && tree->GetPerkPoints()==points && mission->GetIC()==expectedIC,"unsigned IC native grant/skill setup failed");
    if(mode==7){unit->Skills(NDb::ST_VP).SetConst(500);unit->nHealedVP=0;auto* armor=NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR);Require(armor && armor->pMaterial && mission->GetTotalVP()==500 && !mission->GetPanzerklein(),"negative IC damage fixture lacks healthy human/body armor");int damageType=-1;CDBIterator<NDb::CRPGDmgToArmor> types(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());while(types.MoveNext())if(types.Get()->armors[armor->pMaterial->nDR]>0 && (damageType<0 || types.Get()->GetRecordID()<damageType))damageType=types.Get()->GetRecordID();Require(damageType>=0,"negative IC damage fixture lacks flesh damage type");NRPG::CAttackPortion portion(100000,damageType,0.f,40,40,0,0,nullptr,nullptr,1.f,-1);Require(portion.CalcStructDmg(nullptr,armor,0)==40,"negative IC damage fixture lost base40");auto expectedRandom=GlobalGameRandom();const unsigned int roll=expectedRandom.Get(100);Require(roll<static_cast<unsigned int>(expectedIC),"negative original IC comparison did not succeed");expectedRandom.Get(100);const auto received=mission->ProcessAttack(nullptr,NAI::HL_BODY,&portion,nullptr);auto actualRandom=GlobalGameRandom();bool same=true;for(int i=0;i<4;++i)same&=actualRandom.Get()==expectedRandom.Get();std::printf("native unsigned IC mode=7 persona=%d IC=%d roll=%u damage=%d expected=43 HPdrop=%d RNGmatches=%d\n",unit->pPers->GetRecordID(),expectedIC,roll,received.nDmg,500-int(unit->Skills(NDb::ST_VP)),int(same));std::fflush(stdout);Require(received.nDmg==43 && received.type==NRPG::RD_HUMAN && 500-int(unit->Skills(NDb::ST_VP))==43 && same && !mission->IsDead() && !mission->IsUnconscious(),"negative IC did not preserve original unsigned chance and late damage amplification");return;}
    const int ap=mission->GetAP(),hp=mission->GetTotalVP();const float xp=unit->fXP;auto* inventory=unit->GetInventory();auto expectedRandom=GlobalGameRandom();const unsigned int roll=expectedRandom.Get(100);const bool expected=roll<static_cast<unsigned int>(expectedIC);const bool actual=mission->CheckIC();auto actualRandom=GlobalGameRandom();bool same=true;for(int i=0;i<4;++i)same&=actualRandom.Get()==expectedRandom.Get();std::printf("native unsigned IC mode=%d persona=%d acquired=%d IC=%d roll=%u result=%d expected=%d RNGmatches=%d practice=%.9g\n",mode,unit->pPers->GetRecordID(),int(acquired),expectedIC,roll,int(actual),int(expected),int(same),unit->Skills(NDb::ST_IC).GetProgress());std::fflush(stdout);Require(actual==expected && same,"native CheckIC differs from original unsigned percent comparison or consumed the wrong RNG sequence");Require(mission->GetAP()==ap && mission->GetTotalVP()==hp && tree->GetPerkPoints()==points && unit->fXP==xp && unit->GetInventory()==inventory && int(unit->Skills(NDb::ST_IC))==skill && unit->Skills(NDb::ST_IC).GetProgress()>0,"native CheckIC changed unrelated state or omitted existing IC practice");
}

static void BetterEvasionPerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=11,"unknown native better-evasion mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto mission=NativeMissionWithUntakenPerk(40);auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();auto* perk=NDb::GetDBPerk(40);Require(perk && !perk->params.empty() && perk->params[0]>0 && perk->params[0]<1,"evasion fixture lacks an ordinary authored40 bonus");const float authored=perk->params[0];struct Restore {NDb::CDBPerk* perk;float value;~Restore(){perk->params[0]=value;}}restore{perk,authored};
    int skill=21,expected=int(std::nearbyint(float((1.0+double(authored))*21)));const bool acquired=mode!=0 && mode!=10;if(mode==0)expected=21;if(mode==2){skill=0;expected=0;}if(mode==3){skill=140;expected=int(std::nearbyint(float((1.0+double(authored))*140)));}if(mode==4){perk->params[0]=.5f;expected=32;}if(mode==5){skill=23;perk->params[0]=.5f;expected=34;}if(mode==6){perk->params[0]=0;expected=21;}if(mode==7){perk->params[0]=-1;expected=0;}if(mode==8){perk->params[0]=-1.5f;expected=-10;}if(mode==9){perk->params[0]=1.25f;expected=47;}if(mode==10){skill=140;expected=140;}if(mode==11){skill=1;perk->params[0]=.5f;expected=2;}unit->Skills(NDb::ST_IC).SetConst(skill);Require(int(unit->Skills(NDb::ST_IC))==skill,"native effective IC setup failed");
    const int points=tree->GetPerkPoints();if(acquired){tree->AddPerkPoints(1);tree->TakePerk(40);}Require(mission->HasPerk(40)==acquired && tree->GetPerkPoints()==points,"native40 grant failed or changed point budget");const int ap=mission->GetAP(),hp=mission->GetTotalVP();const float xp=unit->fXP;auto random=GlobalGameRandom();const int actual=mission->GetIC();std::printf("native better-evasion mode=%d persona=%d acquired=%d skill=%d authored=%.6g param=%.6g IC=%d expected=%d\n",mode,unit->pPers->GetRecordID(),int(acquired),skill,authored,perk->params[0],actual,expected);std::fflush(stdout);
    Require(actual==expected,"native IC query ignored40, truncated instead of rounded or invented a clamp");Require(mission->GetAP()==ap && mission->GetTotalVP()==hp && tree->GetPerkPoints()==points && unit->fXP==xp && int(unit->Skills(NDb::ST_IC))==skill && random.Get()==GlobalGameRandom().Get(),"IC query changed AP/HP/points/XP/skill/RNG");
}

static void CheapMeleeAPPerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=11,"unknown native cheap-melee AP mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto mission=NativeMissionWithUntakenPerk(32);auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();auto* perk=NDb::GetDBPerk(32);Require(perk && !perk->params.empty() && !mission->HasPerk(43),"melee AP fixture lacks authored32 or has cheap throw43");auto item=NativeEquipCostMelee(unit,true);auto* record=item->GetDBMeleeWeapon();
    const float authored=perk->params[0];Require(authored>0 && authored<1,"melee AP fixture lacks an ordinary authored reduction");struct Restore {NDb::CDBPerk* perk;float value;~Restore(){perk->params[0]=value;}}restore{perk,authored};
    if(mode==2)unit->Skills(NDb::ST_MELEE).SetConst(0);if(mode==3 || mode==4)unit->Skills(NDb::ST_MELEE).SetConst(NRPG::N_MAX_SKILL);if(mode==5)perk->params[0]=.375f;if(mode==6)perk->params[0]=0;if(mode==7)perk->params[0]=1;if(mode==8)perk->params[0]=1.25f;if(mode==9)perk->params[0]=-.25f;
    const bool acquired=mode!=0 && mode!=4;const auto action=mode==10?NRPG::AC_THROW_KNIFE:mode==11?NRPG::AC_OPEN_CLOSE:NRPG::AC_MELEE;const auto pose=mode==2?NAI::CROUCH:mode==3 || mode==11?NAI::CRAWL:mode==4?NAI::RUN:NAI::WALK;const int baseline=mission->GetActionAP(pose,action);Require(baseline>0,"melee AP fixture lacks a positive native base cost");if(mode==2)Require(baseline==record->nMaxAP,"zero melee skill did not produce the authored maximum AP");if(mode==3 || mode==4)Require(baseline==record->nMinAP,"denominator140 melee skill did not produce authored minimum AP");
    const int expected=acquired && action==NRPG::AC_MELEE?int((1.0-double(perk->params[0]))*baseline):baseline,ap=mission->GetAP(),hp=mission->GetTotalVP(),points=tree->GetPerkPoints(),quantity=item->GetQuantity(),melee=unit->Skills(NDb::ST_MELEE),throwing=unit->Skills(NDb::ST_THROWING);const float xp=unit->fXP;auto random=GlobalGameRandom();if(acquired){tree->AddPerkPoints(1);tree->TakePerk(32);}Require(mission->HasPerk(32)==acquired && tree->GetPerkPoints()==points,"native32 grant failed or changed point budget");
    const int actual=mission->GetActionAP(pose,action);std::printf("native cheap-melee AP mode=%d acquired=%d persona=%d item=%d skill=%d action=%d authored=%.6g param=%.6g baseline=%d cost=%d expected=%d\n",mode,int(acquired),unit->pPers->GetRecordID(),record->GetRecordID(),melee,int(action),authored,perk->params[0],baseline,actual,expected);std::fflush(stdout);
    Require(actual==expected,"native melee AP query ignored32 or changed original truncation/no-floor/action scope");
    Require(mission->GetAP()==ap && mission->GetTotalVP()==hp && tree->GetPerkPoints()==points && unit->fXP==xp && int(unit->Skills(NDb::ST_MELEE))==melee && int(unit->Skills(NDb::ST_THROWING))==throwing && unit->GetMeleeWeaponItem()==item.GetPtr() && item->GetQuantity()==quantity && random.Get()==GlobalGameRandom().Get(),"melee AP query changed AP/HP/points/XP/skills/weapon/quantity/RNG");
}

static void CorpseTrackerAPPerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=23,"unknown native corpse-tracker AP mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto mission=NativeMissionWithUntakenPerk(42);auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();auto* perk=NDb::GetDBPerk(42);Require(perk && !perk->params.empty() && !mission->HasPerk(34) && !mission->HasPerk(35),"corpse-cost fixture lacks authored42 or has pose abilities");
    const float authored=perk->params[0];Require(std::isfinite(authored) && authored>=0 && authored<=100,"corpse-cost fixture lacks an ordinary finite authored parameter");struct Restore {NDb::CDBPerk* perk;float value;~Restore(){perk->params[0]=value;}}restore{perk,authored};
    NDb::CPanzerklein* record=nullptr;CDBIterator<NDb::CPanzerklein> suits(*NDatabase::GetTable<NDb::CPanzerklein>());while(suits.MoveNext()){auto* suit=suits.Get();if(suit->nMaxVP>0 && suit->nAddMoveAP>0 && (!record || suit->GetRecordID()<record->GetRecordID()))record=suit;}Require(record,"corpse-cost fixture lacks an authored suit");
    CObj<NRPG::CDynamicSkill> shellHP=new NRPG::CDynamicSkill(record->nMaxVP,record->nMaxVP);const bool worn=(mode>=9 && mode<=12)||mode==14||mode==20||mode==21||mode==23,acquired=mode!=0 && mode!=20;if(worn)mission->SetPanzerklein(record,shellHP,nullptr);
    const NAI::EPose poses[]={NAI::CRAWL,NAI::CRAWL,NAI::CRAWL,NAI::CROUCH,NAI::CROUCH,NAI::WALK,NAI::WALK,NAI::RUN,NAI::RUN,NAI::WALK,NAI::RUN,NAI::CRAWL,NAI::CROUCH,NAI::WALK,NAI::WALK,NAI::CRAWL,NAI::WALK,NAI::WALK,NAI::WALK,NAI::WALK,NAI::WALK,NAI::CRAWL,NAI::WALK,NAI::CROUCH};
    const NRPG::EAction actions[]={NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_DIAGONAL,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_DIAGONAL,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_DIAGONAL,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_DIAGONAL,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_DIAGONAL,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_DIAGONAL,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_DIAGONAL,NRPG::AC_TAKE_CORPSE,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_DIAGONAL,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_DIAGONAL,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_NONE,NRPG::AC_MOVE_CORPSE_DIAGONAL};
    if(mode==16 || mode==17 || mode==21)perk->params[0]=5.75f;if(mode==18 || mode==19)perk->params[0]=-2.75f;if(mode==23)perk->params[0]=-.75f;
    const auto pose=poses[mode];const auto action=actions[mode];const bool corpse=action==NRPG::AC_MOVE_CORPSE_SIDE || action==NRPG::AC_MOVE_CORPSE_DIAGONAL;const int ordinary=(NAI::RUN-pose+1)*2+(worn?record->nAddMoveAP:0),baseline=mission->GetActionAP(pose,action);int expected=action==NRPG::AC_MOVE_CORPSE_DIAGONAL?ordinary*225/100:action==NRPG::AC_MOVE_DIAGONAL || action==NRPG::AC_MOVE_CORPSE_SIDE?ordinary*15/10:ordinary;
    if(acquired && corpse)expected=int(action==NRPG::AC_MOVE_CORPSE_SIDE?double(perk->params[0]):double(perk->params[0])*1.5);if(worn && (pose==NAI::CRAWL || pose==NAI::CROUCH))expected=1;if(action==NRPG::AC_TAKE_CORPSE)expected=12;if(action==NRPG::AC_NONE)expected=0;
    const int ap=mission->GetAP(),hp=mission->GetTotalVP(),shell=*shellHP,points=tree->GetPerkPoints();const float xp=unit->fXP;auto random=GlobalGameRandom();if(acquired){tree->AddPerkPoints(1);tree->TakePerk(42);}Require(mission->HasPerk(42)==acquired && tree->GetPerkPoints()==points,"native42 grant failed or changed point budget");
    const int actual=mission->GetActionAP(pose,action);std::printf("native corpse-tracker AP mode=%d acquired=%d worn=%d persona=%d pose=%d action=%d authored=%.6g param=%.6g baseline=%d cost=%d expected=%d\n",mode,int(acquired),int(worn),unit->pPers->GetRecordID(),int(pose),int(action),authored,perk->params[0],baseline,actual,expected);std::fflush(stdout);
    Require(actual==expected,"native corpse AP ignored42 or changed truncation, valid-pose/PK/early-action order");
    Require(mission->GetAP()==ap && mission->GetTotalVP()==hp && int(*shellHP)==shell && tree->GetPerkPoints()==points && unit->fXP==xp && (mission->GetPanzerklein()==record)==worn && random.Get()==GlobalGameRandom().Get(),"corpse AP query changed AP/HP/suit/points/XP/equipment/RNG");
}

static void CrouchCostsWalkPerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=15,"unknown native crouch-costs-walk mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto mission=NativeMissionWithUntakenPerk(35);auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();Require(!mission->HasPerk(34) && !mission->HasPerk(35) && !mission->HasPerk(42),"crouch-cost fixture has a pose/corpse ability");
    NDb::CPanzerklein* record=nullptr;CDBIterator<NDb::CPanzerklein> suits(*NDatabase::GetTable<NDb::CPanzerklein>());while(suits.MoveNext()){auto* suit=suits.Get();if(suit->nMaxVP>0 && suit->nAddMoveAP>0 && (!record || suit->GetRecordID()<record->GetRecordID()))record=suit;}Require(record,"crouch-cost fixture lacks an authored suit");
    CObj<NRPG::CDynamicSkill> shellHP=new NRPG::CDynamicSkill(record->nMaxVP,record->nMaxVP);const bool worn=(mode>=9 && mode<=12)||mode==15,acquired=mode!=0 && mode!=12;if(worn)mission->SetPanzerklein(record,shellHP,nullptr);
    const NAI::EPose poses[]={NAI::CROUCH,NAI::CROUCH,NAI::CROUCH,NAI::CROUCH,NAI::CROUCH,NAI::CRAWL,NAI::WALK,NAI::RUN,NAI::CROUCH,NAI::CROUCH,NAI::CRAWL,NAI::WALK,NAI::CROUCH,NAI::CROUCH,NAI::CRAWL,NAI::CROUCH};
    const NRPG::EAction actions[]={NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_DIAGONAL,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_DIAGONAL,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_DIAGONAL,NRPG::AC_OPEN_CLOSE,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_SIDE,NRPG::AC_POSE_WALK,NRPG::AC_ROTATE,NRPG::AC_MOVE_CORPSE_DIAGONAL};
    const int values[]={6,4,6,6,9,8,4,3,4,0,1,0,1,0,0,0};const int baseline=mission->GetActionAP(poses[mode],actions[mode]);int expected=values[mode];if(mode==9 || mode==11)expected=4+record->nAddMoveAP;if(mode==15)expected=(4+record->nAddMoveAP)*225/100;if(mode==13 || mode==14)expected=baseline;
    const int ap=mission->GetAP(),hp=mission->GetTotalVP(),shell=*shellHP,points=tree->GetPerkPoints();const float xp=unit->fXP;auto random=GlobalGameRandom();if(acquired){tree->AddPerkPoints(1);tree->TakePerk(35);}Require(!mission->HasPerk(34) && mission->HasPerk(35)==acquired && tree->GetPerkPoints()==points,"native35 grant failed, changed point budget or invented34 ownership");
    const int actual=mission->GetActionAP(poses[mode],actions[mode]);std::printf("native crouch-costs-walk mode=%d acquired=%d worn=%d persona=%d pose=%d action=%d baseline=%d cost=%d expected=%d\n",mode,int(acquired),int(worn),unit->pPers->GetRecordID(),int(poses[mode]),int(actions[mode]),baseline,actual,expected);std::fflush(stdout);
    Require(actual==expected,"native AP query ignored crouch-to-walk35 or original PK/early-action order");
    Require(mission->GetAP()==ap && mission->GetTotalVP()==hp && int(*shellHP)==shell && tree->GetPerkPoints()==points && unit->fXP==xp && (mission->GetPanzerklein()==record)==worn && random.Get()==GlobalGameRandom().Get(),"crouch-cost query changed AP/HP/suit/points/XP/equipment/RNG");
}

static void ProneCostsCrouchPerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=15,"unknown native prone-costs-crouch mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto mission=NativeMissionWithUntakenPerk(34);auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();Require(!mission->HasPerk(34) && !mission->HasPerk(35) && !mission->HasPerk(42),"prone-cost fixture has a pose/corpse ability");
    NDb::CPanzerklein* record=nullptr;CDBIterator<NDb::CPanzerklein> suits(*NDatabase::GetTable<NDb::CPanzerklein>());while(suits.MoveNext()){auto* suit=suits.Get();if(suit->nMaxVP>0 && suit->nAddMoveAP>0 && (!record || suit->GetRecordID()<record->GetRecordID()))record=suit;}Require(record,"prone-cost fixture lacks an authored suit");
    CObj<NRPG::CDynamicSkill> shellHP=new NRPG::CDynamicSkill(record->nMaxVP,record->nMaxVP);const bool worn=mode>=11 && mode<=14,acquired=mode!=0 && mode!=10 && mode!=14;if(worn)mission->SetPanzerklein(record,shellHP,nullptr);
    const NAI::EPose poses[]={NAI::CRAWL,NAI::CRAWL,NAI::CRAWL,NAI::CRAWL,NAI::CRAWL,NAI::CROUCH,NAI::WALK,NAI::RUN,NAI::CRAWL,NAI::CRAWL,NAI::CRAWL,NAI::CRAWL,NAI::CROUCH,NAI::WALK,NAI::CRAWL,NAI::CRAWL};
    const NRPG::EAction actions[]={NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_DIAGONAL,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_DIAGONAL,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_DIAGONAL,NRPG::AC_OPEN_CLOSE,NRPG::AC_ROTATE,NRPG::AC_MOVE_CORPSE_DIAGONAL,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_SIDE,NRPG::AC_POSE_WALK};
    const int values[]={8,6,9,9,13,6,4,3,4,0,18,1,1,0,1,0};const int baseline=mission->GetActionAP(poses[mode],actions[mode]);int expected=values[mode];if(mode==9 || mode==15)expected=baseline;if(mode==13)expected=4+record->nAddMoveAP;
    const int ap=mission->GetAP(),hp=mission->GetTotalVP(),shell=*shellHP,points=tree->GetPerkPoints();const float xp=unit->fXP;auto random=GlobalGameRandom();if(acquired){tree->AddPerkPoints(1);tree->TakePerk(34);}Require(mission->HasPerk(34)==acquired && tree->GetPerkPoints()==points,"native34 grant failed or changed point budget");
    const int actual=mission->GetActionAP(poses[mode],actions[mode]);std::printf("native prone-costs-crouch mode=%d acquired=%d worn=%d pose=%d action=%d baseline=%d cost=%d expected=%d\n",mode,int(acquired),int(worn),int(poses[mode]),int(actions[mode]),baseline,actual,expected);std::fflush(stdout);
    Require(actual==expected,"native AP query ignored prone-to-crouch ability34 or changed an unrelated/PK control");
    Require(mission->GetAP()==ap && mission->GetTotalVP()==hp && int(*shellHP)==shell && tree->GetPerkPoints()==points && unit->fXP==xp && (mission->GetPanzerklein()==record)==worn && random.Get()==GlobalGameRandom().Get(),"prone-cost query changed AP/HP/suit/points/XP/equipment/RNG");
}

static void PKUnsupportedPoseCost(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=15,"unknown native PK pose-cost mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto mission=NativeMissionWithUntakenPerk(34);auto* unit=mission->GetRPGUnit();Require(!mission->HasPerk(34) && !mission->HasPerk(35) && !mission->HasPerk(42),"PK pose fixture has a pose/corpse ability");
    NDb::CPanzerklein* record=nullptr;CDBIterator<NDb::CPanzerklein> suits(*NDatabase::GetTable<NDb::CPanzerklein>());while(suits.MoveNext()){auto* suit=suits.Get();if(suit->nMaxVP>0 && suit->nAddMoveAP>0 && (!record || suit->GetRecordID()<record->GetRecordID()))record=suit;}Require(record,"PK pose fixture lacks an authored suit with positive added move AP");
    CObj<NRPG::CDynamicSkill> shellHP=new NRPG::CDynamicSkill(record->nMaxVP,record->nMaxVP);const bool worn=mode!=10 && mode!=11 && mode!=15;if(worn)mission->SetPanzerklein(record,shellHP,nullptr);Require((mission->GetPanzerklein()==record)==worn,"native SetPanzerklein did not establish the requested worn state");
    const NRPG::EAction moves[]={NRPG::AC_MOVE_SIDE,NRPG::AC_MOVE_DIAGONAL,NRPG::AC_MOVE_CORPSE_SIDE,NRPG::AC_MOVE_CORPSE_DIAGONAL};NAI::EPose pose=mode<4 || mode==10 || mode==12 || mode==15?NAI::CRAWL:mode<8 || mode==11 || mode==13?NAI::CROUCH:mode==9 || mode==14?NAI::RUN:NAI::WALK;
    NRPG::EAction action=mode<8?moves[mode%4]:mode==9 || mode==11?NRPG::AC_MOVE_DIAGONAL:mode==12?NRPG::AC_OPEN_CLOSE:mode==13?NRPG::AC_NONE:mode==14?NRPG::AC_MOVE_CORPSE_SIDE:mode==15?NRPG::AC_MOVE_CORPSE_DIAGONAL:NRPG::AC_MOVE_SIDE;
    const int ordinary=(NAI::RUN-pose+1)*2+(worn?record->nAddMoveAP:0);int expected=action==NRPG::AC_MOVE_DIAGONAL || action==NRPG::AC_MOVE_CORPSE_SIDE?ordinary*15/10:action==NRPG::AC_MOVE_CORPSE_DIAGONAL?ordinary*225/100:ordinary;
    if(mode<8)expected=1;if(mode==12)expected=4;if(mode==13)expected=0;
    const int ap=mission->GetAP(),hp=mission->GetTotalVP(),shell=*shellHP,points=unit->GetPerksTree()->GetPerkPoints();const float xp=unit->fXP;auto random=GlobalGameRandom();
    const int actual=mission->GetActionAP(pose,action);std::printf("native PK pose-cost mode=%d worn=%d suit=%d add=%d pose=%d action=%d cost=%d expected=%d\n",mode,int(worn),record->GetRecordID(),record->nAddMoveAP,int(pose),int(action),actual,expected);std::fflush(stdout);
    Require(actual==expected,"native worn-PK pose query missed the original unsupported-pose1 guard or changed a control cost");
    Require(mission->GetAP()==ap && mission->GetTotalVP()==hp && int(*shellHP)==shell && unit->GetPerksTree()->GetPerkPoints()==points && unit->fXP==xp && (mission->GetPanzerklein()==record)==worn && random.Get()==GlobalGameRandom().Get(),"PK cost query changed AP/HP/suit health/points/XP/equipment/RNG");
}

static void ObserverStealthPerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=13,"unknown native observer-stealth mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto observer=NativeMissionWithUntakenPerk(75),target=NativeMissionWithUntakenPerk(75);auto* unit=observer->GetRPGUnit();auto* victim=target->GetRPGUnit();auto* perk=NDb::GetDBPerk(75);Require(perk && !perk->params.empty() && !unit->HasPerk(25),"observer-stealth fixture lacks authored node/parameter or has25");
    const float authored=perk->params[0];Require(std::fabs(authored)<2 && int((1.0+double(authored))*100)!=100,"observer-stealth fixture lacks an observable ordinary authored bonus");struct Restore {NDb::CDBPerk* perk;float value;~Restore(){perk->params[0]=value;}}restore{perk,authored};
    const bool acquired=mode!=0 && mode!=3 && mode!=7 && mode!=12,targetAcquired=mode==12 || mode==13;const int spot=mode==2 || (mode>=5 && mode<=8)?17:100,stealth=mode==2 || mode==3 || mode==4?0:mode>=5 && mode<=8?7:100;const float distance=mode==9?15.f:mode==10?100.f:0.f;
    unit->Skills(NDb::ST_SPOT).SetConst(spot);victim->Skills(NDb::ST_STEALTH).SetConst(stealth);if(mode==5 || mode==10)perk->params[0]=.25f;if(mode==6)perk->params[0]=-.25f;if(mode==7)perk->params[0]=1.75f;if(mode==8)perk->params[0]=-1.5f;if(mode==11)perk->params[0]=0;
    auto grant=[](NRPG::CUnit* value){auto* tree=value->GetPerksTree();const int points=tree->GetPerkPoints();tree->AddPerkPoints(1);tree->TakePerk(75);Require(value->HasPerk(75) && tree->GetPerkPoints()==points,"native75 grant failed or changed point budget");};if(acquired)grant(unit);if(targetAcquired)grant(victim);Require(unit->HasPerk(75)==acquired && victim->HasPerk(75)==targetAcquired,"observer-stealth fixture ability ownership is wrong");
    const int ap=observer->GetAP(),hp=observer->GetTotalVP(),targetAP=target->GetAP(),targetHP=target->GetTotalVP();const float xp=unit->fXP,targetXP=victim->fXP;auto random=GlobalGameRandom();
    const int adjusted=acquired?int((1.0+double(perk->params[0]))*stealth):stealth,raw=int(45.f-(44.f/30.f)*distance)+spot-adjusted,expected=(std::max)(0,(std::min)(100,raw)),actual=observer->GetUnhideProbability(target,distance);
    std::printf("native observer-stealth mode=%d acquired=%d targetAcquired=%d distance=%.6g spot=%d stealth=%d param=%.6g probability=%d expected=%d\n",mode,int(acquired),int(targetAcquired),distance,spot,stealth,perk->params[0],actual,expected);std::fflush(stdout);
    Require(actual==expected,"native detection ignored observer75 scaling target stealth or changed original ownership/truncation/clamp");
    Require(int(unit->Skills(NDb::ST_SPOT))==spot && int(victim->Skills(NDb::ST_STEALTH))==stealth && observer->GetAP()==ap && observer->GetTotalVP()==hp && target->GetAP()==targetAP && target->GetTotalVP()==targetHP && unit->fXP==xp && victim->fXP==targetXP && random.Get()==GlobalGameRandom().Get(),"observer75 query changed skills/AP/HP/XP/RNG");
}

static void BetterSpottingPerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=13,"unknown native better-spotting mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto observer=NativeMissionWithUntakenPerk(25),target=NativeMissionWithUntakenPerk(25);auto* unit=observer->GetRPGUnit();auto* victim=target->GetRPGUnit();auto* perk=NDb::GetDBPerk(25);Require(perk && !perk->params.empty() && !unit->HasPerk(75),"spotting fixture lacks its authored node/parameter or has75");
    const float authored=perk->params[0];Require(authored>0 && authored<2,"spotting fixture lacks an ordinary positive authored bonus");struct Restore {NDb::CDBPerk* perk;float value;~Restore(){perk->params[0]=value;}}restore{perk,authored};
    const bool acquired=mode!=0 && mode!=3 && mode!=7 && mode!=12,targetAcquired=mode==12 || mode==13;const int spot=mode==2?0:mode==3 || mode==4?100:mode>=5 && mode<=8?7:21,stealth=mode==3 || mode==4?0:30;const float distance=mode==9?15.f:mode==10?100.f:0.f;
    unit->Skills(NDb::ST_SPOT).SetConst(spot);victim->Skills(NDb::ST_STEALTH).SetConst(stealth);if(mode==5)perk->params[0]=.25f;if(mode==6)perk->params[0]=-.25f;if(mode==7)perk->params[0]=1.75f;if(mode==8)perk->params[0]=-1.5f;if(mode==11)perk->params[0]=0;
    auto grant=[](NRPG::CUnit* value){auto* tree=value->GetPerksTree();const int points=tree->GetPerkPoints();tree->AddPerkPoints(1);tree->TakePerk(25);Require(value->HasPerk(25) && tree->GetPerkPoints()==points,"native spotting grant failed or changed point budget");};if(acquired)grant(unit);if(targetAcquired)grant(victim);Require(unit->HasPerk(25)==acquired && victim->HasPerk(25)==targetAcquired,"spotting fixture ability ownership is wrong");
    const int ap=observer->GetAP(),hp=observer->GetTotalVP(),targetAP=target->GetAP(),targetHP=target->GetTotalVP();const float xp=unit->fXP,targetXP=victim->fXP;auto random=GlobalGameRandom();
    const int adjusted=acquired?int((1.0+double(perk->params[0]))*spot):spot,raw=int(45.f-(44.f/30.f)*distance)+adjusted-stealth,expected=(std::max)(0,(std::min)(100,raw)),actual=observer->GetUnhideProbability(target,distance);
    std::printf("native better-spotting mode=%d acquired=%d targetAcquired=%d distance=%.6g spot=%d stealth=%d param=%.6g probability=%d expected=%d\n",mode,int(acquired),int(targetAcquired),distance,spot,stealth,perk->params[0],actual,expected);std::fflush(stdout);
    Require(actual==expected,"native detection query ignored observer perk25 or changed ownership/truncation/clamp");
    Require(int(unit->Skills(NDb::ST_SPOT))==spot && int(victim->Skills(NDb::ST_STEALTH))==stealth && observer->GetAP()==ap && observer->GetTotalVP()==hp && target->GetAP()==targetAP && target->GetTotalVP()==targetHP && unit->fXP==xp && victim->fXP==targetXP && random.Get()==GlobalGameRandom().Get(),"spotting query changed skills/AP/HP/XP/RNG");
}

static void UnhideBasePrecision(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=7,"unknown native unhide-base mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto observer=NativeMissionWithUntakenPerk(25),target=NativeMissionWithUntakenPerk(25);auto* unit=observer->GetRPGUnit();auto* victim=target->GetRPGUnit();Require(!unit->HasPerk(25) && !unit->HasPerk(75),"unhide baseline fixture has an active perception perk");
    const float distance=mode==0 || mode==7?0.f:mode==2?30.f:mode==3?5.f:mode==4?40.f:15.f;const int spot=mode==5?70:mode==6?0:mode==7?100:17,stealth=mode==5?30:mode==6?100:mode==7?0:17;
    unit->Skills(NDb::ST_SPOT).SetConst(spot);victim->Skills(NDb::ST_STEALTH).SetConst(stealth);const int expected=mode==0?45:mode==1?23:mode==2?1:mode==3?37:mode==4?0:mode==5?63:mode==6?0:100;
    const int ap=observer->GetAP(),hp=observer->GetTotalVP(),targetAP=target->GetAP(),targetHP=target->GetTotalVP();const float xp=unit->fXP,targetXP=victim->fXP;auto random=GlobalGameRandom();
    const int actual=observer->GetUnhideProbability(target,distance);std::printf("native unhide-base mode=%d distance=%.9g spot=%d stealth=%d probability=%d expected=%d\n",mode,distance,spot,stealth,actual,expected);std::fflush(stdout);
    Require(actual==expected,"native detection baseline rounded its distance product before original truncation");
    Require(int(unit->Skills(NDb::ST_SPOT))==spot && int(victim->Skills(NDb::ST_STEALTH))==stealth && observer->GetAP()==ap && observer->GetTotalVP()==hp && target->GetAP()==targetAP && target->GetTotalVP()==targetHP && unit->fXP==xp && victim->fXP==targetXP && random.Get()==GlobalGameRandom().Get(),"detection query changed native skills/AP/HP/XP/RNG");
}

static void DisarmTrapsPerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=15,"unknown native disarm mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto mission=NativeMissionWithUntakenPerk(51);auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();auto* perk=NDb::GetDBPerk(51);auto* constants=NDb::GetDBMinesConstants();Require(perk && !perk->params.empty() && constants,"disarm fixture lacks authored perk/mines constants");
    const float authored=perk->params[0];Require(authored>=1 && authored<50,"disarm fixture lacks an ordinary positive original bonus");struct Restore {NDb::CDBPerk* perk;float value;~Restore(){perk->params[0]=value;}}restore{perk,authored};
    const bool acquired=mode!=0 && mode!=8 && mode!=13 && mode!=15;const int skill=mode==12?0:7;unit->Skills(NDb::ST_ENGINEERING).SetConst(skill);
    if(mode==4)perk->params[0]=1.75f;if(mode==5)perk->params[0]=-.75f;if(mode==10)perk->params[0]=0;if(mode==11)perk->params[0]=-20;
    const int baseline=mode==6 || mode==7 || mode==15?100:mode==13 || mode==14?0:40,dc=constants->nBaseDisarmProb+skill-baseline,modifier=mode==8?-500:mode==9?500:0;
    const int adjusted=acquired?int(double(skill)+perk->params[0]):skill,prob=(std::min)(constants->nBaseDisarmProb+adjusted-dc,95);
    const unsigned targetRoll=mode==2?5:mode==3?99:mode==4?40:mode==5?39:mode==6 || mode==15?95:mode==7?94:mode==11?25:mode==13 || mode==14?0:40;
    const int ap=mission->GetAP(),hp=mission->GetTotalVP(),points=tree->GetPerkPoints();const float xp=unit->fXP;
    if(acquired){tree->AddPerkPoints(1);tree->TakePerk(51);}Require(mission->HasPerk(51)==acquired && tree->GetPerkPoints()==points,"native disarm grant failed or changed point budget");
    unsigned seed=0;auto expectedRng=GlobalGameRandom();for(unsigned candidate=1;candidate<10000;++candidate){auto probe=GlobalGameRandom();probe.SeedForHarness(candidate);if(probe.Get(0,100)==targetRoll){seed=candidate;expectedRng=probe;break;}}Require(seed,"disarm fixture could not select its deterministic boundary roll");
    GlobalGameRandom().SeedForHarness(seed);const bool expected=targetRoll<static_cast<unsigned>(prob),actual=mission->CanClear(dc,modifier);
    std::printf("native disarm mode=%d acquired=%d skill=%d param=%.6g dc=%d mod=%d prob=%d seed=%u roll=%u result=%d expected=%d\n",mode,int(acquired),skill,perk->params[0],dc,modifier,prob,seed,targetRoll,int(actual),int(expected));std::fflush(stdout);
    Require(actual==expected,"native disarm roll ignored perk51 or changed truncation/cap/strict comparison");
    Require(int(unit->Skills(NDb::ST_ENGINEERING))==skill && mission->GetAP()==ap && mission->GetTotalVP()==hp && unit->fXP==xp && expectedRng.Get()==GlobalGameRandom().Get(),"disarm query changed skill/AP/HP/XP or consumed other than one gameplay roll");
}

static void ClasslessPerkQuery(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=6,"unknown native classless-perk mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::IUnitMission> mission;CObj<NRPG::CUnit> unit;const int id=mode==0?12:mode==1?85:mode==3?51:mode==6?50:49;
    if(mode<4){auto* person=NDb::GetPers(mode%2?888:887);Require(person && !person->pClass && person->pModel && person->pBaseValue,"classless fixture lacks authored gender-preview persona");unit=new NRPG::CUnit(person);Require(!unit->GetPerksTree(),"classless native persona unexpectedly received a perk tree");}
    else {mission=NativeMissionWithUntakenPerk(id);unit=mission->GetRPGUnit();if(mode>=5){auto* tree=unit->GetPerksTree();const int points=tree->GetPerkPoints();tree->AddPerkPoints(1);tree->TakePerk(id);Require(tree->GetPerkPoints()==points,"classed native grant changed its point budget");}}
    const int ap=unit->Skills(NDb::ST_AP),hp=unit->Skills(NDb::ST_VP);const float xp=unit->fXP;auto random=GlobalGameRandom();float a=11,b=22,c=33;const bool outputs=mode==2 || mode==3 || mode==4 || mode==5,expected=mode>=5;
    std::printf("native classless-perk mode=%d persona=%d tree=%d id=%d before-query\n",mode,unit->pPers->GetRecordID(),int(unit->GetPerksTree()!=nullptr),id);std::fflush(stdout);
    const bool actual=outputs?unit->HasPerk(id,&a,&b,&c):unit->HasPerk(id);
    std::printf("native classless-perk result mode=%d has=%d expected=%d out=%.6g,%.6g,%.6g\n",mode,int(actual),int(expected),a,b,c);std::fflush(stdout);Require(actual==expected,"native perk lookup changed missing/classed ability semantics");
    if(outputs){if(expected){auto* record=NDb::GetDBPerk(id);Require(record && record->params.size()>=3 && a==record->params[0] && b==record->params[1] && c==record->params[2],"classed acquired perk lost its authored output parameters");}else Require(a==11 && b==22 && c==33,"missing perk query wrote output parameters");}
    Require(int(unit->Skills(NDb::ST_AP))==ap && int(unit->Skills(NDb::ST_VP))==hp && unit->fXP==xp && random.Get()==GlobalGameRandom().Get(),"perk query changed AP/HP/XP/RNG");
}

static void DifficultTrapsPerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=15,"unknown native trap-difficulty mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto mission=NativeMissionWithUntakenPerk(49);auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();auto* perk=NDb::GetDBPerk(49);Require(perk && !perk->params.empty(),"trap fixture lacks authored Param1");
    const float authored=perk->params[0];Require(authored>0,"trap fixture lacks positive original bonus");struct Restore {NDb::CDBPerk* perk;float value;~Restore(){perk->params[0]=value;}}restore{perk,authored};
    const bool mine=mode%2!=0,acquired=mode!=0 && mode!=1 && mode!=12 && mode!=13;const int skill=mode==4 || mode==5?0:mode>=6?7:21;unit->Skills(NDb::ST_ENGINEERING).SetConst(skill);
    if(mode==6 || mode==7)perk->params[0]=.25f;if(mode==8 || mode==9)perk->params[0]=-.25f;if(mode==10 || mode==11)perk->params[0]=-1.5f;if(mode==12 || mode==13)perk->params[0]=1.75f;
    NDb::CRPGGrenade* grenade=nullptr;NDb::CRPGMine* mineRecord=nullptr;CDBIterator<NDb::CRPGGrenade> grenades(*NDatabase::GetTable<NDb::CRPGGrenade>());while(grenades.MoveNext())if(!grenade || grenades.Get()->GetRecordID()<grenade->GetRecordID())grenade=grenades.Get();
    CDBIterator<NDb::CRPGMine> mines(*NDatabase::GetTable<NDb::CRPGMine>());while(mines.MoveNext())if(!mineRecord || mines.Get()->GetRecordID()<mineRecord->GetRecordID())mineRecord=mines.Get();Require(grenade && mineRecord,"trap fixture lacks authored mine/grenade records");
    const int ap=mission->GetAP(),hp=mission->GetTotalVP(),points=tree->GetPerkPoints();const float xp=unit->fXP;auto random=GlobalGameRandom();
    if(acquired){tree->AddPerkPoints(1);tree->TakePerk(49);}Require(mission->HasPerk(49)==acquired && tree->GetPerkPoints()==points,"trap-difficulty native grant failed or changed points");
    const int expected=acquired?int((1.0+double(perk->params[0]))*skill):skill,actual=mine?mission->GetMineDC(mode==15?nullptr:mineRecord):mission->GetGrenadeTrapDC(mode==14?nullptr:grenade);
    std::printf("native trap-difficulty mode=%d mine=%d acquired=%d skill=%d param=%.6g difficulty=%d expected=%d\n",mode,int(mine),int(acquired),skill,perk->params[0],actual,expected);std::fflush(stdout);
    Require(actual==expected,"native trap difficulty ignored perk49 or changed original truncation/no-clamp rule");
    Require(int(unit->Skills(NDb::ST_ENGINEERING))==skill && mission->GetAP()==ap && mission->GetTotalVP()==hp && unit->fXP==xp && random.Get()==GlobalGameRandom().Get(),"trap-difficulty query changed skill/AP/HP/XP/RNG");
}

static void ZeroingBonusPerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=14,"unknown native zeroing mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto mission=NativeMissionWithUntakenPerk(85);auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();auto* record=NDb::GetDBPerk(85);Require(record && !record->params.empty(),"zeroing fixture lacks authored Param1");
    const float authored=record->params[0];struct Restore {NDb::CDBPerk* record;float value;~Restore(){record->params[0]=value;}}restore{record,authored};
    const int cap=mission->GetToHitConstants()->nMaxShotsRepeat;Require(cap>=3 && authored>0,"zeroing fixture lacks positive original repeat cap/bonus");
    bool acquired=mode>=4 && mode!=11;int repeats=(mode==1 || mode==5 || mode==9 || mode==10 || mode==11 || mode==13 || mode==14)?2:(mode==2 || mode==6)?cap:(mode==3 || mode==7)?cap+2:0;
    if(mode==8 || mode==9)record->params[0]=1.75f;if(mode==10 || mode==11 || mode==12)record->params[0]=-.75f;if(mode==13)record->params[0]=-3.5f;if(mode==14)record->params[0]=0;
    mission->RegisterAction(NRPG::AC_OPEN_CLOSE);for(int i=0;i<repeats;++i)mission->RegisterAction(NRPG::AC_END_SHOOT);Require(mission->GetLastActionTimes()==repeats,"native repeat setup did not register completed shots");
    const int ap=mission->GetAP(),hp=mission->GetTotalVP(),points=tree->GetPerkPoints();const float xp=unit->fXP;auto random=GlobalGameRandom();
    if(acquired){tree->AddPerkPoints(1);tree->TakePerk(85);}Require(mission->HasPerk(85)==acquired && tree->GetPerkPoints()==points,"zeroing native grant failed or changed its point budget");
    CObj<NRPG::CTileToHitCalcer> calcer=new NRPG::CTileToHitCalcer;calcer->pUnitMission=mission;
    const int adjusted=acquired?int(double(repeats)+record->params[0]):repeats;const float expected=(std::min)(adjusted,cap)*2,actual=calcer->GetRS();
    std::printf("native zeroing mode=%d acquired=%d repeats=%d cap=%d param=%.6g bonus=%.6g expected=%.6g\n",mode,int(acquired),repeats,cap,record->params[0],actual,expected);std::fflush(stdout);
    Require(actual==expected,"shared repeated-shot consumer ignored perk85 or changed truncation/cap order");
    Require(mission->GetLastActionTimes()==repeats && mission->GetAP()==ap && mission->GetTotalVP()==hp && unit->fXP==xp && random.Get()==GlobalGameRandom().Get(),"zeroing query changed repeat count/AP/HP/XP/RNG");
}

static void BestShootingPosePerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=10,"unknown native best-pose mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto mission=NativeMissionWithUntakenPerk(12);auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();NDb::CRPGWeapon* record=nullptr;
    CDBIterator<NDb::CRPGWeapon> rows(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while(rows.MoveNext()){auto* candidate=rows.Get();if(candidate->pItem && candidate->pWeaponType && candidate->pWeaponType->fCrouchBonus>0 && candidate->pWeaponType->fCrawlBonus>candidate->pWeaponType->fCrouchBonus && (!record || candidate->GetRecordID()<record->GetRecordID()))record=candidate;}
    Require(record,"best-pose fixture lacks an authored weapon with distinct positive stance bonuses");CObj<NRPG::CWeaponItem> weapon=new NRPG::CWeaponItem(record);weapon->nQuantity=1;auto* inventory=unit->GetInventory();CObj<NRPG::IInventoryItem> previous=inventory->TakeOff(NDb::SLOT_1);
    Require(inventory->Equip(NDb::SLOT_1,weapon) && inventory->Activate(NDb::SLOT_1) && unit->GetWeaponItem()==weapon.GetPtr(),"best-pose fixture could not activate its authored native weapon");
    if(mode==10){CObj<NRPG::IInventoryItem> first=inventory->TakeOff(NDb::SLOT_1),second=inventory->TakeOff(NDb::SLOT_2);Require(!unit->GetWeaponItem(),"unarmed best-pose control retained an active firearm");}
    auto* type=record->pWeaponType.GetPtr();
    const float crouch=type->fCrouchBonus,crawl=type->fCrawlBonus;struct Restore {NDb::CRPGWeaponType* type;float crouch,crawl;~Restore(){type->fCrouchBonus=crouch;type->fCrawlBonus=crawl;}}restore{type,crouch,crawl};
    const bool acquired=mode==3 || mode==4 || mode==5 || mode==6 || mode==7 || mode==10;NAI::EPose pose=mode<6?(mode%3==0?NAI::WALK:mode%3==1?NAI::CROUCH:NAI::CRAWL):NAI::CROUCH;
    if(mode==6 || mode==9)type->fCrouchBonus=type->fCrawlBonus=7.f;if(mode==7 || mode==8){type->fCrouchBonus=-2.f;type->fCrawlBonus=-3.f;}
    const int ap=mission->GetAP(),hp=mission->GetTotalVP(),points=tree->GetPerkPoints();const float xp=unit->fXP;auto random=GlobalGameRandom();
    if(acquired){tree->AddPerkPoints(1);tree->TakePerk(12);}Require(mission->HasPerk(12)==acquired && tree->GetPerkPoints()==points,"best-pose native perk grant failed or changed its point budget");
    CObj<NRPG::CTileToHitCalcer> calcer=new NRPG::CTileToHitCalcer;calcer->pUnitMission=mission;calcer->pWeaponItem=mode==10?nullptr:weapon.GetPtr();calcer->eCurPose=pose;
    float expected=pose==NAI::CROUCH?int(type->fCrouchBonus):pose==NAI::CRAWL?int(type->fCrawlBonus):0;
    if(acquired)expected=(mode==6 || mode==7 || mode==10)?0:int(crawl);
    const float actual=calcer->GetStance();std::printf("native best-pose mode=%d acquired=%d weapon=%d pose=%d crouch=%.6g crawl=%.6g bonus=%.6g expected=%.6g\n",mode,int(acquired),record->GetRecordID(),int(pose),type->fCrouchBonus,type->fCrawlBonus,actual,expected);std::fflush(stdout);
    Require(std::fabs(actual-expected)<1e-6f,"shared stance consumer ignored perk12 or changed the original tie/nonpositive rule");
    Require(calcer->eCurPose==pose && mission->GetAP()==ap && mission->GetTotalVP()==hp && unit->fXP==xp && random.Get()==GlobalGameRandom().Get(),"best-pose query changed cached pose/AP/HP/XP/RNG");
}

static void PerfectEngineeringToolEligibility(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=5,"unknown native perfect-engineering mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    vector<int> trees;CDBIterator<NDb::CDBPerkTreeNode> nodes(*NDatabase::GetTable<NDb::CDBPerkTreeNode>());
    while(nodes.MoveNext())if(nodes.Get()->pPerk && nodes.Get()->pPerk->GetRecordID()==50)trees.push_back(nodes.Get()->nTreeID);
    vector<NDb::CRPGPers*> candidates;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext()){auto* person=people.Get();if(person->pModel && person->pClass && person->pBaseValue && person->pName && person->pClass->skills[NDb::ST_LEVEL]>0 && !person->pPanzerklein && !person->pDefaultWearsPanzerklein && std::find(trees.begin(),trees.end(),person->pClass->nPerkTreeID)!=trees.end())candidates.push_back(person);}
    std::sort(candidates.begin(),candidates.end(),[](NDb::CRPGPers* a,NDb::CRPGPers* b){return a->GetRecordID()<b->GetRecordID();});
    CObj<NRPG::IUnitMission> mission;
    for(auto* person:candidates){CObj<NRPG::IUnitMission> candidate=NRPG::CreateUnit(person);auto* unit=candidate->GetRPGUnit();auto* tree=unit->GetPerksTree();if(!tree || unit->HasPerk(50) || candidate->GetTotalVP()<=0)continue;vector<CPtr<NRPG::CPerk>> perks;tree->GetAllPerks(&perks);bool found=false;for(auto perk:perks)if(perk->GetDBPerk()->GetRecordID()==50){found=true;break;}if(found){mission=candidate;break;}}
    Require(mission,"engineering fixture lacks an actual authored RPG unit with untaken perk50");auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();NDb::CRPGTool* record=nullptr;NDb::CRPGTool* fallback=nullptr;
    CDBIterator<NDb::CRPGTool> rows(*NDatabase::GetTable<NDb::CRPGTool>());
    while(rows.MoveNext()){auto* tool=rows.Get();if(!tool->pItem || tool->nCharges<=0 || tool->nNeededEngSkill<=0)continue;if(!fallback || tool->GetRecordID()<fallback->GetRecordID())fallback=tool;if((!tool->pNeededPerk || unit->HasPerk(tool->pNeededPerk->GetRecordID())) && (!record || tool->GetRecordID()<record->GetRecordID()))record=tool;}
    if(!record)record=fallback;Require(record,"engineering fixture lacks a charged authored skill-gated tool");CObj<NRPG::CToolItem> tool=new NRPG::CToolItem(record);tool->nQuantity=1;
    const bool acquired=mode==1 || mode==2 || mode==5,empty=mode==2 || mode==4,qualified=mode==0 || mode==4 || mode==5;unit->Skills(NDb::ST_ENGINEERING).SetConst(qualified?record->nNeededEngSkill:0);
    if(empty){const int original=tool->GetIncQuantity();for(int i=0;i<original;++i)tool->SpendOneCharge();Require(tool->GetIncQuantity()==0 && tool->GetQuantity()==1,"empty-tool fixture lost its actual stack or retained charges");}
    const int ap=mission->GetAP(),hp=mission->GetTotalVP(),points=tree->GetPerkPoints(),charges=tool->GetIncQuantity(),stack=tool->GetQuantity(),skill=unit->Skills(NDb::ST_ENGINEERING);const float xp=unit->fXP;auto random=GlobalGameRandom();
    if(acquired){tree->AddPerkPoints(1);tree->TakePerk(50);}Require(mission->HasPerk(50)==acquired && tree->GetPerkPoints()==points,"native perfect-engineering grant failed or changed its point budget");
    const bool ordinaryPerk=!record->pNeededPerk || unit->HasPerk(record->pNeededPerk->GetRecordID());const bool expected=!empty && (acquired || (qualified && ordinaryPerk)),actual=tool->CanBeUsed(unit);
    std::printf("native perfect-engineering mode=%d acquired=%d person=%d record=%d skill=%d required=%d charges=%d stack=%d usable=%d expected=%d\n",mode,int(acquired),unit->GetPers()->GetRecordID(),record->GetRecordID(),skill,record->nNeededEngSkill,charges,stack,int(actual),int(expected));std::fflush(stdout);
    Require(actual==expected,"perfect engineering did not bypass the tool skill gate or bypassed its charge gate");
    Require(mission->GetAP()==ap && mission->GetTotalVP()==hp && unit->fXP==xp && int(unit->Skills(NDb::ST_ENGINEERING))==skill && tool->GetIncQuantity()==charges && tool->GetQuantity()==stack && random.Get()==GlobalGameRandom().Get(),"tool eligibility query spent charges/AP/HP/XP or changed skill/RNG");
}

static void FirstTurnPenaltyPerk(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=5,"unknown native first-turn perk mode");auto world=FixtureWorld(databasePath,resourcePath,true);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto entry:units) {
        auto* candidate=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());if(!candidate || !candidate->CanFight() || candidate->IsWearingPK())continue;
        auto* unit=candidate->GetUnitRPG()->GetRPGUnit();auto* tree=unit->GetPerksTree();if(!tree || unit->HasPerk(23))continue;
        vector<CPtr<NRPG::CPerk>> perks;tree->GetAllPerks(&perks);bool found=false;
        for(auto perk:perks)if(perk->GetDBPerk()->GetRecordID()==23){found=true;break;}if(found){actor=candidate;break;}
    }
    Require(actor,"first-turn perk fixture has no actual actor with an untaken authored perk23");
    auto* mission=actor->GetUnitRPG();auto* unit=mission->GetRPGUnit();auto* tree=unit->GetPerksTree();auto* constants=mission->GetToHitConstants();
    const float originalCoefficient=constants->fFirstRoundCoeff;Require(originalCoefficient>0 && originalCoefficient<1,"first-turn perk fixture has no authored penalty coefficient");
    struct RestoreCoefficient {float& value;float original;~RestoreCoefficient(){value=original;}}restore{constants->fFirstRoundCoeff,originalCoefficient};
    const bool acquired=mode!=0 && mode!=5,first=mode==0 || mode==1 || mode==3;
    if(mode==3 || mode==4)constants->fFirstRoundCoeff=1.2f;
    const int ap=actor->GetAP(),hp=mission->GetTotalVP(),points=tree->GetPerkPoints();const float xp=unit->fXP;const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
    if(acquired){tree->AddPerkPoints(1);tree->TakePerk(23);}Require(mission->HasPerk(23)==acquired && tree->GetPerkPoints()==points,"native first-turn perk grant changed the point budget or failed to take its authored node");
    CObj<NRPG::CTileToHitCalcer> calcer=new NRPG::CTileToHitCalcer(actor,position.GetPose(),3,position.GetCP(),0,100.f,first,false,CVec3(1,1,1),position.GetCP()+CVec3(3*FP_GRID_STEP,0,0),0);
    const float actual=calcer->GetFRMult(),expected=acquired?Max(constants->fFirstRoundCoeff,1.f):first?constants->fFirstRoundCoeff:1.f;
    std::printf("native first-turn perk mode=%d acquired=%d first=%d coefficient=%.6g multiplier=%.6g expected=%.6g\n",mode,int(acquired),int(first),constants->fFirstRoundCoeff,actual,expected);std::fflush(stdout);
    Require(std::fabs(actual-expected)<1e-6f,"native first-round multiplier ignored perk23 or capped its original coefficient bonus");
    Require(actor->GetAP()==ap && mission->GetTotalVP()==hp && unit->fXP==xp && actor->GetPosition()==position && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"first-round perk query changed AP/HP/XP/position/time/RNG");
}

class NativeGrenadeInputProbe: public NWorld::CExecThrowGrenade
{
public:
    NativeGrenadeInputProbe(NWorld::CUnitServer* actor,const CVec3& target):NWorld::CExecThrowGrenade(actor,target){}
    using NWorld::CExecThrowGrenade::CreateToHitCalcer;
    using NWorld::CExecThrowGrenade::CheckToHitAndDelay;
};
static NWorld::SGrenadeParams NativeThrowParameters(NWorld::CExecThrowGrenade* executor)
{
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(executor,&refs);NWorld::SGrenadeParams params;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(3,&params);}return params;
}
static void NativeGrenadeFuse(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<16,"unknown native grenade fuse mode");const bool script=mode>=8;const int scenario=mode%8;const bool owned=(scenario%2)==1;
    NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false,true);auto& world=fixture.world;auto* anchor=fixture.actor;
    NDb::CRPGGrenade* selected=nullptr;CDBIterator<NDb::CRPGGrenade> rows(*NDatabase::GetTable<NDb::CRPGGrenade>());while(rows.MoveNext()){auto* g=rows.Get();if(g->nMaxDelay>0 && g->pItem && g->pItem->pModel && g->pItem->nWeight>0 && (!selected || g->GetRecordID()<selected->GetRecordID()))selected=g;}Require(selected,"fuse fixture lacks an authored timed grenade");fixture.regular=selected;
    auto mission=NativeMissionWithUntakenPerk(90);auto* unit=mission->GetRPGUnit();Require(!mission->HasPerk(91),"fuse fixture starts with unrelated accuracy perk");
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(anchor->GetPosition().GetCP(),20.f),&places);NAI::SPathPlace free;bool found=false;
    for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetMoving(0);if(network->IsPassable(p) && !network->IsLocked(p)){free=p;found=true;break;}}Require(found,"fuse fixture lacks a free actual actor placement");
    CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(free,mission,anchor->GetTBSPlayer(),"NativeGrenadeFuse");Require(actor && actor->CanFight(),"fuse fixture cannot deploy its actual actor");
    std::vector<CObj<NRPG::CSkillModifier>> held;unit->Skills(NDb::ST_STR).SetConst(10);const int raw=scenario==4 || scenario==5?140:scenario==2 || scenario==3?79:70;
    NativeSetTrainingStableSkill(unit,NDb::ST_THROWING,raw,held);NativeSetTrainingStableSkill(unit,NDb::ST_VP,500,held);unit->nHealedVP=0;
    CObj<NRPG::CSkillModifier> cap;if(scenario==2 || scenario==3){cap=new NRPG::CSkillModifier(&unit->Skills(NDb::ST_VP),NRPG::SSkillModifyInfo(1.f,-200));unit->Skills(NDb::ST_VP).SetValue(300);}if(scenario>=6)unit->Skills(NDb::ST_VP).SetValue(200);
    CObj<NRPG::CGrenadeItem> grenade=new NRPG::CGrenadeItem(fixture.regular);grenade->nQuantity=1;auto* inventory=unit->GetInventory();CObj<NRPG::IInventoryItem> previous=inventory->TakeOff(NDb::SLOT_1);
    Require(inventory->Equip(NDb::SLOT_1,grenade) && inventory->Activate(NDb::SLOT_1),"fuse fixture cannot activate its actual grenade");actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(mission->GetWeaponType());
    auto* tree=unit->GetPerksTree();const int points=tree->GetPerkPoints();if(owned){auto* perk=NDb::GetDBPerk(90);Require(perk,"fuse fixture lacks authored90");const string label=perk->szUserName;struct Restore{NDb::CDBPerk* p;string text;~Restore(){p->szUserName=text;}}restore{perk,label};perk->szUserName="Native perfect grenade fuse";tree->AddPerkPoints(1);tree->TakePerk(90);}
    Require(mission->HasPerk(90)==owned && tree->GetPerkPoints()==points && int(unit->Skills(NDb::ST_THROWING))==raw,"fuse fixture changed its raw skill/point budget");
    world->GetGame()->SetNight(false);CObj<NativeGrenadeInputProbe> probe;
    for(float radius:{.5f,1.f,2.f,3.f,5.f}){for(auto offset:{CVec3(radius,0,.1f),CVec3(-radius,0,.1f),CVec3(0,radius,.1f),CVec3(0,-radius,.1f)}){CObj<NativeGrenadeInputProbe> candidate=new NativeGrenadeInputProbe(actor,actor->GetPosition().GetCP()+offset);if(candidate->CanDoIt(actor->GetPosition())==NWorld::UCR_OK && actor->CanSpendAP(candidate->GetStartAP())){probe=candidate;break;}}if(probe)break;}
    Require(probe,"fuse fixture lacks a valid native ballistic trajectory");probe->Run();Require(probe->IsExecuting(),"fuse fixture failed to enter the actual executor");const auto before=NativeThrowParameters(probe);auto calcer=probe->CreateToHitCalcer(grenade);
    const int chance=script?100:calcer->GetToHit();const float skill=mission->GetWeaponSkill(grenade,0,0);const float expectedSkill=scenario==2 || scenario==3?71.099998474121094f:scenario>=6?52.5f:float(raw);
    Require(skill==expectedSkill && calcer->GetSkill()==int(expectedSkill),"fuse fixture has unexpected fractional shared/cached skill inputs");
    if(script)probe->Cancel();const int ap=actor->GetAP(),hp=mission->GetTotalVP(),quantity=grenade->GetQuantity();const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();const auto initialCount=world->GetMiscObjects()->size();
    CRandomGenerator reference;int seed=0,scatterRoll=0,fuseRoll=0;const int desired=scenario==1 || scenario==4 || scenario==5?99:scenario==2 || scenario==3?71:scenario>=6?52:-1;
    for(seed=1;seed<100000;++seed){reference.SeedForHarness(seed);scatterRoll=reference.Get(100);if(scatterRoll>chance)continue;fuseRoll=reference.Get(100);if(desired>=0?fuseRoll==desired:fuseRoll<int(skill))break;}
    Require(seed<100000,"fuse fixture cannot isolate the intended random threshold");reference.SeedForHarness(seed);const auto originalInput=reference;Require(reference.Get(100)==scatterRoll,"fuse seed changed");const int maxDelay=fixture.regular->nMaxDelay;float expected=maxDelay-Clamp(before.fT,0.f,float(maxDelay));bool misfire=false;
    if(!owned){Require(reference.Get(100)==fuseRoll,"fuse roll changed");misfire=fuseRoll>=99 || fuseRoll>=skill;if(misfire){expected*=reference.GetFloat(.5f,2.f);expected=Clamp(expected,0.f,float(maxDelay)*.75f);}}
    unsigned int expectedBits=0,actualBits=0;std::memcpy(&expectedBits,&expected,4);float actual=-1;bool timer=true,velocity=true;
    static_assert(sizeof(CRandomGenerator)==0x810,"original ISAAC state layout changed");static_assert(sizeof(NWorld::SGrenadeParams)==44,"original grenade parameter layout changed");
    std::printf("native grenade fuse fixture mode=%d grenade=%d maxDelay=%d paramsHex=",mode,fixture.regular->GetRecordID(),maxDelay);auto dump=[](const void* object,size_t n){auto* p=static_cast<const unsigned char*>(object);for(size_t i=0;i<n;++i)std::printf("%02x",unsigned(p[i]));};dump(&before,sizeof(before));std::printf(" inputRandom=");dump(&originalInput,sizeof(originalInput));std::printf(" expectedRandom=");dump(&reference,sizeof(reference));std::printf("\n");std::fflush(stdout);
    GlobalGameRandom().SeedForHarness(seed);
    if(script){NWorld::UnitThrowGrenade(actor,fixture.regular,before.ptOriginalTarget,100);Require(world->GetMiscObjects()->size()==initialCount+1,"script fuse did not launch exactly one actual projectile");auto last=world->GetMiscObjects()->end();--last;CStructureNetworkContext refs;auto fields=BorrowedObjectFields(last->GetPtr(),&refs);STime explode=0;CPtr<NAnimation::CAnimator> animator;CPtr<NWorld::CUnitServer> owner;CDBPtr<NDb::CRPGGrenade> record;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&explode);reader.Add(10,&animator);reader.Add(9,&owner);reader.Add(7,&record);}auto* physics=dynamic_cast<NAnimation::CASphereSet*>(animator.GetPtr());Require(physics && owner.GetPtr()==actor.GetPtr() && record.GetPtr()==fixture.regular,"script fuse lost its actual owner/record/physics");const auto state=ReadItemPhysics(physics);velocity=fabs2(state.posVelocity-before.vel)<1e-8f;timer=explode==time+STime(int((float(maxDelay)-expected)*1000));actual=float(explode-time)/1000.f;world->GetMiscObjects()->erase(last);
    }else{probe->CheckToHitAndDelay(grenade);const auto after=NativeThrowParameters(probe);actual=after.fT;std::memcpy(&actualBits,&actual,4);timer=actualBits==expectedBits;velocity=after.vel==before.vel && after.ptStart==before.ptStart;}
    // Script construction adds the existing three angular-momentum draws after the fuse.
    if(script)for(int i=0;i<3;++i)reference.GetFloat(-2.f,2.f);
    auto continuation=GlobalGameRandom();bool rng=true;for(int i=0;i<8;++i)rng&=reference.Get()==continuation.Get();
    std::printf("native grenade fuse mode=%d script=%d persona=%d tree=%d owned=%d raw=%d VP=%d theory=%d skill=%.9g cached=%d chance=%d seed=%d scatter=%d fuse=%d misfire=%d before=%.9g expected=%.9g actual=%.9g expectedBits=%08x actualBits=%08x timer=%d velocity=%d RNG=%d\n",mode,int(script),unit->pPers->GetRecordID(),unit->pPers->pClass->nPerkTreeID,int(owned),raw,mission->GetTotalVP(),unit->Skills(NDb::ST_VP).GetTheoreticalMax(),skill,calcer->GetSkill(),chance,seed,scatterRoll,fuseRoll,int(misfire),before.fT,expected,actual,expectedBits,actualBits,int(timer),int(velocity),int(rng));std::fflush(stdout);
    Require(actor->GetAP()==ap && mission->GetTotalVP()==hp && grenade->GetQuantity()==quantity && inventory->GetActive()==grenade.GetPtr() && actor->GetPosition()==position && world->GetTime()->GetValue()==time && world->GetMiscObjects()->size()==initialCount && tree->GetPerkPoints()==points,"fuse query changed AP/HP/item/position/time/points or leaked projectile");if(!script)probe->Cancel();
    Require(timer && velocity && rng,"native grenade fuse ignores90, truncates fractional shared skill or consumes an extra gameplay random draw");
}


static void NativeGrenadeHideLaunch(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<3,"unknown grenade hide launch mode");NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false,true);auto& world=fixture.world;auto* actor=fixture.actor;auto executor=fixture.executor;
    const bool hidden=mode!=0;actor->Hide(hidden,false);Require(actor->IsHiding()==hidden,"grenade hide fixture cannot establish actual concealment");const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),cost=executor->GetStartAP(),quantity=fixture.grenade->GetQuantity();const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();
    executor->Run();Require(executor->IsExecuting(),"grenade hide fixture did not enter actual Run");const bool afterHide=actor->IsHiding();CStructureNetworkContext refs;auto fields=BorrowedObjectFields(executor,&refs);bool requested=false;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(6,&requested);}
    if(mode==2){fields.Seek(0);CObj<NWorld::CExecThrowGrenade> restored=new NWorld::CExecThrowGrenade;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);}executor=restored;}
    auto setBatch=[&](bool delay,bool queued){CMemoryStream data;CStructureNetworkContext context;{CStructureSaver writer(data,CStructureSaver::WRITE,&context);writer.Add(53,&delay);writer.Add(54,&queued);}data.Seek(0);{CStructureSaver reader(data,CStructureSaver::READ,&context);static_cast<CObjectBase*>(world.GetPtr())->operator&(reader);}};
    auto batch=[&](){CStructureNetworkContext context;auto data=BorrowedObjectFields(world,&context);bool delay=false,queued=false;{CStructureSaver reader(data,CStructureSaver::READ,&context);reader.Add(53,&delay);reader.Add(54,&queued);}return std::make_pair(delay,queued);};const auto oldBatch=batch();setBatch(true,false);const int afterAP=actor->GetAP();executor->AnimationFinished();const auto afterBatch=batch();setBatch(oldBatch.first,oldBatch.second);
    std::printf("native grenade hide launch mode=%d beforeHidden=%d afterHidden=%d requested=%d queued=%d restored=%d APBefore=%d APAfter=%d cost=%d\n",mode,int(hidden),int(afterHide),int(requested),int(afterBatch.second),int(mode==2),ap,afterAP,cost);std::fflush(stdout);
    Require(actor->GetAP()==ap-cost && actor->GetAP()==afterAP && actor->GetUnitRPG()->GetTotalVP()==hp && fixture.grenade->GetQuantity()==quantity && actor->GetPosition()==position && world->GetTime()->GetValue()==time && executor->GetState()==NWorld::CCommandExecute::FINISHED,"hide launch/completion changed HP/item/position/time, paid AP twice or failed completion");
    Require(!afterHide && requested==hidden && afterBatch==std::make_pair(true,hidden),"actual grenade Run did not leave concealment and preserve its deferred visibility request through completion/save");
}

static void NativeGrenadePKLaunch(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<3,"unknown grenade PK launch mode");NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false,true);auto& world=fixture.world;auto* actor=fixture.actor;CObj<NWorld::CUnitServer> shell;
    if(mode!=0){auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),20.f),&places);NAI::SPathPlace place;bool found=false;for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetMoving(0);if(network->IsPassable(p) && !network->IsLocked(p) && NAI::IsBigLockerPlacePassable(network,p)){place=p;found=true;break;}}Require(found,"PK launch lacks a free actual big-locker placement");
        NDb::CRPGPers* suit=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());while(people.MoveNext()){auto* p=people.Get();if(p->pPanzerklein && p->pModel && !p->pPanzerklein->bHasNoHead && p->pPanzerklein->nMaxVP>0 && p->pPanzerklein->nGrenadeStrength>0 && (!suit || p->GetRecordID()<suit->GetRecordID()))suit=p;}Require(suit,"PK launch lacks an authored shell");shell=world->AddUnitInGame(place,NRPG::CreateUnit(suit),nullptr);Require(shell && shell->IsEmptyPK() && shell->WearAsPK(true),"PK launch cannot prepare actual empty shell");actor->SetPosition(shell->GetPosition());actor->FlipPanzerklein(shell,false,false);Require(actor->IsWearingPK() && actor->GetWearingDBPK()==suit->pPanzerklein.GetPtr(),"PK launch did not establish real worn identity");auto* inv=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> old=inv->TakeOff(NDb::SLOT_1);Require(inv->Equip(NDb::SLOT_1,fixture.grenade) && inv->Activate(NDb::SLOT_1),"PK launch cannot reactivate its actual grenade");}
    // Fold authored worn-PK attribute changes into derived VP before measuring launch invariants.
    actor->GetUnitRPG()->GetRPGUnit()->UpdateSkills();
    CObj<NativeGrenadeInputProbe> probe;CVec3 target;for(float radius:{.5f,1.f,2.f,3.f,5.f}){for(auto offset:{CVec3(radius,0,.1f),CVec3(-radius,0,.1f),CVec3(0,radius,.1f),CVec3(0,-radius,.1f)}){target=actor->GetPosition().GetCP()+offset;CObj<NativeGrenadeInputProbe> candidate=new NativeGrenadeInputProbe(actor,target);if(candidate->CanDoIt(actor->GetPosition())==NWorld::UCR_OK && actor->CanSpendAP(candidate->GetStartAP())){probe=candidate;break;}}if(probe)break;}Require(probe,"PK launch lacks a valid actual grenade trajectory");
    const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),quantity=fixture.grenade->GetQuantity();const auto count=world->GetMiscObjects()->size();CVec3 origin;int side=-1;
    if(mode==2){NWorld::UnitThrowGrenade(actor,fixture.regular,target,100);Require(world->GetMiscObjects()->size()==count+1,"PK script launch did not create an actual projectile");auto last=world->GetMiscObjects()->end();--last;CStructureNetworkContext refs;auto fields=BorrowedObjectFields(last->GetPtr(),&refs);CPtr<NAnimation::CAnimator> animator;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(10,&animator);}auto* physics=dynamic_cast<NAnimation::CASphereSet*>(animator.GetPtr());Require(physics,"PK script launch lost actual physics");origin=ReadItemPhysics(physics).pos;world->GetMiscObjects()->erase(last);
    }else{probe->Run();Require(probe->IsExecuting(),"PK launch did not enter actual Run");auto params=NativeThrowParameters(probe);origin=params.ptStart;side=params.nSide;probe->Cancel();}
    const float angle=position.GetDirection();const CVec3 left(-NWorld::F_GRENADE_CHECK_SIDE*sin(angle),NWorld::F_GRENADE_CHECK_SIDE*cos(angle),0);const float error=fabs2(origin-(position.GetCenter()+left));
    std::printf("native grenade PK launch mode=%d PK=%d side=%d originDelta=%.9g,%.9g,%.9g expectedLeft=%.9g,%.9g,%.9g error=%.9g\n",mode,int(actor->IsWearingPK()),side,origin.x-position.GetCenter().x,origin.y-position.GetCenter().y,origin.z-position.GetCenter().z,left.x,left.y,left.z,error);std::fflush(stdout);
    std::printf("native PK invariant mode=%d HP=%d/%d position=%d time=%llu/%llu quantity=%d/%d AP=%d/%d count=%zu/%zu hidden=%d\n",mode,hp,actor->GetUnitRPG()->GetTotalVP(),int(actor->GetPosition()==position),static_cast<unsigned long long>(time),static_cast<unsigned long long>(world->GetTime()->GetValue()),quantity,fixture.grenade->GetQuantity(),ap,actor->GetAP(),size_t(count),world->GetMiscObjects()->size(),int(actor->IsHiding()));std::fflush(stdout);
    Require(actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && world->GetTime()->GetValue()==time && fixture.grenade->GetQuantity()==quantity && (mode!=2 || actor->GetAP()==ap) && world->GetMiscObjects()->size()==count,"PK launch changed HP/item/position/time or script AP/projectile ownership");
    Require(mode==0 || (error<1e-8f && (mode==2 || side==0)),"actual PK grenade trajectory did not use the left launch origin with animation side0");
}

static void GrenadeThrowNightInput(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=2,"unknown native grenade throw night mode");NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false,true);
    auto& world=fixture.world;auto* actor=fixture.actor;auto* mission=actor->GetUnitRPG();const bool night=mode!=0;
    std::printf("throw night fixture unitFirst=%d worldFirst=%d nightVision=%d\n",int(mission->IsFirstTurn()),int(world->IsFirstTurn()),int(mission->HasPerk(0x15)));
    Require(!mission->HasPerk(0x15),"grenade night fixture already has night vision");
    world->GetGame()->SetNight(night);CVec3 target;
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(fixture.executor,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&target);}
    CObj<NativeGrenadeInputProbe> probe=new NativeGrenadeInputProbe(actor,target);
    if(mode<2) {
        const int ap=actor->GetAP(),hp=mission->GetTotalVP(),quantity=fixture.grenade->GetQuantity();const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
        auto calcer=probe->CreateToHitCalcer(fixture.grenade);const float light=calcer->GetLight();const int chance=calcer->GetToHit();
        std::printf("native throw night mode=%d ownerNight=%d calcerNight=%d light=%.6g first=%d chance=%d\n",mode,int(night),int(calcer->bNight),light,int(calcer->bFirstRound),chance);std::fflush(stdout);
        Require(calcer->bNight==night && std::fabs(light-(night?.9f:1.f))<1e-6f,"live grenade executor dropped its owning game's night input");
        Require(actor->GetAP()==ap && mission->GetTotalVP()==hp && fixture.grenade->GetQuantity()==quantity && actor->GetPosition()==position && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"throw accuracy construction changed AP/HP/quantity/position/time/RNG");return;
    }
    int expectedChance=-1,legacyChance=-1;
    for(int distance=1;distance<=30;++distance) {
        const auto point=actor->GetPosition().GetCP()+CVec3(float(distance)*FP_GRID_STEP,0,.1f);
        CObj<NativeGrenadeInputProbe> candidate=new NativeGrenadeInputProbe(actor,point);
        if(candidate->CanDoIt(actor->GetPosition())!=NWorld::UCR_OK || !actor->CanSpendAP(candidate->GetStartAP()))continue;
        CObj<NRPG::CGrenadeToHitCalcer> correct=new NRPG::CGrenadeToHitCalcer(actor,actor->GetPose(),distance,actor->GetPosition().GetCP(),mission->IsFirstTurn(),true,CVec3(1,1,1),point,fixture.grenade);
        CObj<NRPG::CGrenadeToHitCalcer> old=new NRPG::CGrenadeToHitCalcer(actor,actor->GetPose(),distance,actor->GetPosition().GetCP(),mission->IsFirstTurn(),false,CVec3(1,1,1),point,fixture.grenade);
        const int a=correct->GetToHit(),b=old->GetToHit();if(a>=b || a<=2 || b>=100)continue;
        probe=candidate;target=point;expectedChance=a;legacyChance=b;break;
    }
    Require(expectedChance>=0,"throw night fixture lacks an observable valid native trajectory");
    probe->Run();Require(probe->IsExecuting(),"night throw did not enter the actual native executor");
    const auto before=NativeThrowParameters(probe);const int ap=actor->GetAP(),hp=mission->GetTotalVP(),quantity=fixture.grenade->GetQuantity();
    int seed=0,roll=0;CRandomGenerator reference;
    for(seed=1;seed<1000;++seed){reference.SeedForHarness(seed);roll=reference.Get(100);if(roll>expectedChance && roll<=legacyChance)break;}
    Require(seed<1000,"throw night fixture cannot distinguish scatter thresholds");CVec3 expected=before.vel;const float spread=.2f*fabs(expected);
    expected.x+=reference.GetFloat(-spread,spread);expected.y+=reference.GetFloat(-spread,spread);expected.z+=reference.GetFloat(-spread,spread);
    GlobalGameRandom().SeedForHarness(seed);probe->CheckToHitAndDelay(fixture.grenade);const auto after=NativeThrowParameters(probe);const float error=fabs2(after.vel-expected);
    std::printf("native throw night scatter expectedChance=%d legacyChance=%d seed=%d roll=%d velocityError=%.7g difference=%.7g\n",expectedChance,legacyChance,seed,roll,error,fabs2(after.vel-before.vel));std::fflush(stdout);
    Require(error<1e-6f,"actual grenade scatter consumer used daytime chance in a night mission");
    Require(actor->GetAP()==ap && mission->GetTotalVP()==hp && fixture.grenade->GetQuantity()==quantity && after.ptStart==before.ptStart,"night scatter consumed additional AP/HP/inventory or changed the native launch origin");probe->Cancel();
}

static void GrenadeUnitFirstTurnInput(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=3,"unknown native grenade unit-turn mode");NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false,true);
    auto& world=fixture.world;auto* actor=fixture.actor;auto* mission=actor->GetUnitRPG();
    for(int refresh=0;refresh<3 && mission->IsFirstTurn();++refresh)mission->StartNewTurn(actor->GetPosition().GetCP());
    Require(!mission->IsFirstTurn() && world->IsFirstTurn(),"unit-turn fixture lacks real differing mission/world turn states");
    if(mode!=0)mission->AddMoveInLastTurn(4);const bool night=mode==2;world->GetGame()->SetNight(night);
    CVec3 target;{CStructureNetworkContext refs;auto fields=BorrowedObjectFields(fixture.executor,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&target);}
    CObj<NativeGrenadeInputProbe> probe=new NativeGrenadeInputProbe(actor,target);
    if(mode<3) {
        const int ap=actor->GetAP(),hp=mission->GetTotalVP(),quantity=fixture.grenade->GetQuantity();const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
        auto calcer=probe->CreateToHitCalcer(fixture.grenade);const int chance=calcer->GetToHit();const float move=calcer->GetSMove();const float firstMultiplier=calcer->GetFRMult();
        std::printf("native grenade unit-turn mode=%d unitFirst=%d worldFirst=%d calcerFirst=%d night=%d moved=%d penalty=%.6g firstMultiplier=%.6g chance=%d\n",mode,int(mission->IsFirstTurn()),int(world->IsFirstTurn()),int(calcer->bFirstRound),int(calcer->bNight),mission->GetMoveInLastTurn(),move,firstMultiplier,chance);std::fflush(stdout);
        if(mode!=0)Require(calcer->bFirstRound==mission->IsFirstTurn() && std::fabs(firstMultiplier-1.f)<1e-6f,"grenade executor used the world first-turn state and wrong first-round multiplier");
        Require(calcer->bNight==night && actor->GetAP()==ap && mission->GetTotalVP()==hp && fixture.grenade->GetQuantity()==quantity && actor->GetPosition()==position && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"per-unit turn accuracy query changed night/AP/HP/quantity/position/time/RNG");return;
    }
    int expectedChance=-1,legacyChance=-1;
    for(int distance=1;distance<=30;++distance) {
        const auto point=actor->GetPosition().GetCP()+CVec3(float(distance)*FP_GRID_STEP,0,.1f);CObj<NativeGrenadeInputProbe> candidate=new NativeGrenadeInputProbe(actor,point);
        if(candidate->CanDoIt(actor->GetPosition())!=NWorld::UCR_OK || !actor->CanSpendAP(candidate->GetStartAP()))continue;
        CObj<NRPG::CGrenadeToHitCalcer> correct=new NRPG::CGrenadeToHitCalcer(actor,actor->GetPose(),distance,actor->GetPosition().GetCP(),mission->IsFirstTurn(),night,CVec3(1,1,1),point,fixture.grenade);
        CObj<NRPG::CGrenadeToHitCalcer> old=new NRPG::CGrenadeToHitCalcer(actor,actor->GetPose(),distance,actor->GetPosition().GetCP(),world->IsFirstTurn(),night,CVec3(1,1,1),point,fixture.grenade);
        int a=correct->GetToHit(),b=old->GetToHit();if(a==b || Min(a,b)<=2 || Max(a,b)>=100)continue;probe=candidate;target=point;expectedChance=a;legacyChance=b;break;
    }
    Require(expectedChance>=0,"unit-turn scatter fixture lacks an observable valid native trajectory");probe->Run();Require(probe->IsExecuting(),"unit-turn throw did not start its actual executor");
    const auto before=NativeThrowParameters(probe);const int ap=actor->GetAP(),hp=mission->GetTotalVP(),quantity=fixture.grenade->GetQuantity();int seed=0,roll=0;CRandomGenerator reference;
    for(seed=1;seed<1000;++seed){reference.SeedForHarness(seed);roll=reference.Get(100);if(roll>Min(expectedChance,legacyChance) && roll<=Max(expectedChance,legacyChance))break;}
    Require(seed<1000,"unit-turn fixture cannot distinguish scatter thresholds");CVec3 expected=before.vel;if(roll>expectedChance){const float spread=.2f*fabs(expected);expected.x+=reference.GetFloat(-spread,spread);expected.y+=reference.GetFloat(-spread,spread);expected.z+=reference.GetFloat(-spread,spread);}
    GlobalGameRandom().SeedForHarness(seed);probe->CheckToHitAndDelay(fixture.grenade);const auto after=NativeThrowParameters(probe);const float error=fabs2(after.vel-expected);
    std::printf("native grenade unit-turn scatter expectedChance=%d legacyChance=%d seed=%d roll=%d velocityError=%.7g difference=%.7g\n",expectedChance,legacyChance,seed,roll,error,fabs2(after.vel-before.vel));std::fflush(stdout);
    Require(error<1e-6f,"actual grenade scatter consumer used world-turn accuracy instead of the actor's turn state");
    Require(actor->GetAP()==ap && mission->GetTotalVP()==hp && fixture.grenade->GetQuantity()==quantity && after.ptStart==before.ptStart,"unit-turn scatter consumed extra AP/HP/inventory or changed launch origin");probe->Cancel();
}

static void NightAIConstructorInputs(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=2,"unknown native AI night mode");auto world=FixtureWorld(databasePath,resourcePath,false);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;NWorld::CUnitServer* target=nullptr;
    for(auto entry:units){auto* candidate=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());if(candidate && candidate->CanFight() && !candidate->IsWearingPK() && !candidate->GetUnitRPG()->HasPerk(0x15) && NRPG::GetToHitType(candidate)==NRPG::TH_SHOOT && NAI::GetAIUnit(candidate)){actor=candidate;break;}}
    Require(actor,"AI night fixture lacks a native firearm actor without night vision");
    for(auto entry:units){auto* candidate=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());if(candidate && candidate!=actor && candidate->CanFight() && NAI::GetAIUnit(candidate)){target=candidate;break;}}
    Require(target,"AI night fixture lacks a native target wrapper");auto* shooterAI=NAI::GetAIUnit(actor);auto* targetAI=NAI::GetAIUnit(target);
    const bool night=mode!=0;world->GetGame()->SetNight(night);
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
    CObj<NRPG::CAIUnitToHitCalcer> calcer;
    if(mode==2)calcer=new NRPG::CAIUnitToHitCalcer(shooterAI,position,targetAI,100,NAI::HL_BODY,0,actor->GetUnitRPG()->GetWeaponItem(),0);
    else calcer=new NRPG::CAIUnitToHitCalcer(shooterAI,position,targetAI,100.f,NAI::HL_BODY,0,actor->GetUnitRPG()->GetWeaponItem(),0);
    const float light=calcer->GetLight();const int chance=calcer->GetToHit();const float expected=night?.9f:1.f;
    std::printf("native AI night mode=%d ownerNight=%d calcerNight=%d light=%.6g expected=%.6g chance=%d\n",mode,int(world->GetGame()->IsNight()),int(calcer->bNight),light,expected,chance);std::fflush(stdout);
    Require(calcer->bNight==night && std::fabs(light-expected)<1e-6f,"AI accuracy constructor dropped the owning game's night input");
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"AI accuracy query changed AP/HP/position/time/RNG");
}

static void NightPreviewInputs(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=4,"unknown native night preview mode");
    NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false,true);auto& world=fixture.world;auto* actor=fixture.actor;
    Require(!actor->GetUnitRPG()->HasPerk(0x15),"night preview fixture already has night vision");
    CObj<NRPG::CWeaponItem> weapon;
    if(mode!=3) {
        NDb::CRPGWeapon* record=nullptr;const bool rocket=mode==1 || mode==2;
        CDBIterator<NDb::CRPGWeapon> rows(*NDatabase::GetTable<NDb::CRPGWeapon>());
        while(rows.MoveNext()){auto* candidate=rows.Get();if(candidate->pItem && candidate->pAnimWeaponType && candidate->bBazookaLogic==rocket && (!record || candidate->GetRecordID()<record->GetRecordID()))record=candidate;}
        Require(record,"night preview lacks authored firearm/launcher");weapon=new NRPG::CWeaponItem(record);
        auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous=inventory->TakeOff(NDb::SLOT_1);
        Require(inventory->Equip(NDb::SLOT_1,weapon) && inventory->Activate(NDb::SLOT_1),"night preview cannot equip its authored weapon");
        if(!rocket)Require(weapon->SetShootMode(NDb::SM_Snap),"night preview firearm has no native snap mode");
        Require(NRPG::GetToHitType(actor)==(rocket?NRPG::TH_RLAUNCHER:NRPG::TH_SHOOT),"night preview did not select the actual weapon action");
    }
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
    const CVec3 origin=position.GetCP(),illumination(1,1,1);const bool first=false;
    int distance=0,day=0,night=0;CVec3 target;
    auto reference=[&](int dist,bool dark,const CVec3& point) {
        CObj<NRPG::CToHitCalcer> calcer;
        if(mode==3)calcer=new NRPG::CGrenadeToHitCalcer(actor,position.GetPose(),dist,origin,first,dark,illumination,point);
        else if(mode==1 || mode==2)calcer=new NRPG::CRLauncherToHitCalcer(actor,position.GetPose(),dist,origin,100.f,0,first,dark,illumination,point);
        else calcer=new NRPG::CTileToHitCalcer(actor,position.GetPose(),dist,origin,0,100.f,first,dark,illumination,point,0);
        return calcer->GetToHit();
    };
    for(int d=1;d<=150;++d){auto point=origin+CVec3(float(d)*FP_GRID_STEP,0,0);int a=reference(d,false,point),b=reference(d,true,point);if(a>b && b>2 && a<100){distance=d;target=point;day=a;night=b;break;}}
    Require(distance>0,"night preview lacks an observable non-clamped native chance");
    auto query=[&]() {
        if(mode==3)return NRPG::GetGrenadeToHit(actor,position.GetPose(),distance,origin,first,target,illumination);
        if(mode==2)return NRPG::GetRLauncherToHit(actor,position.GetPose(),distance,origin,target,NAI::THL_MIDDLE,0,first,illumination);
        return NRPG::GetTileToHit(actor,position.GetPose(),distance,origin,target,NAI::THL_MIDDLE,0,100,first,illumination,0);
    };
    if(mode==4){actor->SetScriptToHit(37);day=night=37;}
    world->GetGame()->SetNight(false);const int actualDay=query();Require(actualDay==day,"day preview control changed before the night boundary");
    world->GetGame()->SetNight(true);const int actualNight=query();
    std::printf("native night preview mode=%d distance=%d day=%d/%d night=%d/%d\n",mode,distance,actualDay,day,actualNight,night);std::fflush(stdout);
    Require(actualNight==night,"native accuracy dispatcher did not forward its owning mission's night flag");
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"night preview query changed AP/HP/position/time/RNG");
}

static void ScriptGrenadeExplicitChance(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=6,"unknown scripted grenade chance mode");
    NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false,true);auto& world=fixture.world;auto* actor=fixture.actor;
    CStructureNetworkContext targetRefs;auto targetFields=BorrowedObjectFields(fixture.executor,&targetRefs);CVec3 target;
    {CStructureSaver reader(targetFields,CStructureSaver::READ,&targetRefs);reader.Add(2,&target);}
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),quantity=fixture.grenade->GetQuantity();
    const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();const auto initialCount=world->GetMiscObjects()->size();const bool action=world->IsAction();
    CRandomGenerator reference;reference.SeedForHarness(42);const int roll=reference.Get(100);Require(roll>0 && roll<100,"script chance seed does not provide an interior boundary");
    GlobalGameRandom().SeedForHarness(42);
    if(mode==6) {
        auto random=GlobalGameRandom();NWorld::UnitThrowGrenade(nullptr,fixture.regular,target,100);NWorld::UnitThrowGrenade(actor,nullptr,target,100);
        Require(world->GetMiscObjects()->size()==initialCount && random.Get()==GlobalGameRandom().Get(),"invalid script grenade consumed RNG or spawned a projectile");return;
    }
    struct Launch {CVec3 position,velocity;};
    auto launch=[&](int chance) {
        const auto count=world->GetMiscObjects()->size();GlobalGameRandom().SeedForHarness(42);
        NWorld::UnitThrowGrenade(actor,fixture.regular,target,chance);
        Require(world->GetMiscObjects()->size()==count+1,"scripted grenade did not launch exactly one actual projectile");
        auto last=world->GetMiscObjects()->end();--last;auto* grenade=last->GetPtr();
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(grenade,&refs);CPtr<NAnimation::CAnimator> animator;CPtr<NWorld::CUnitServer> owner;CDBPtr<NDb::CRPGGrenade> record;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(10,&animator);reader.Add(9,&owner);reader.Add(7,&record);}
        auto* physics=dynamic_cast<NAnimation::CASphereSet*>(animator.GetPtr());Require(physics && owner.GetPtr()==actor && record.GetPtr()==fixture.regular,"script grenade lost its actual physics/owner/record");
        const auto state=ReadItemPhysics(physics);Launch result{state.pos,state.posVelocity};world->GetMiscObjects()->erase(last);return result;
    };
    const auto baseline=launch(100);const int chances[]={100,0,roll,roll-1,-1,101};const int chance=chances[mode];const bool scatter=roll>chance;
    CVec3 expected=baseline.velocity;
    if(scatter){const float spread=.2f*fabs(baseline.velocity);expected.x+=reference.GetFloat(-spread,spread);expected.y+=reference.GetFloat(-spread,spread);expected.z+=reference.GetFloat(-spread,spread);}
    const auto actual=launch(chance);const float error=fabs2(actual.velocity-expected);
    std::printf("native script grenade mode=%d chance=%d roll=%d scatter=%d velocityError=%.7g difference=%.7g\n",mode,chance,roll,int(scatter),error,fabs2(actual.velocity-baseline.velocity));std::fflush(stdout);
    Require(error<1e-6f && fabs2(actual.position-baseline.position)<1e-8f,"scripted grenade ignored its explicit chance or changed the strict random boundary");
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && fixture.grenade->GetQuantity()==quantity && actor->GetPosition()==position && world->GetTime()->GetValue()==time && world->GetMiscObjects()->size()==initialCount && world->IsAction()==action,"script chance consumed inventory/AP/HP/time or leaked projectile action ownership");
}

static vector<int> NativeVPBoostDurations(NRPG::IUnitMission* mission,int* modifierCount=nullptr)
{
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(mission,&refs);vector<int> durations;vector<CPtr<CObjectBase>> modifiers;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(38,&durations);reader.Add(37,&modifiers);}
    if(modifierCount)*modifierCount=int(modifiers.size());return durations;
}
static void VPBoostImmediateOverdose(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=6,"unknown native immediate VP overdose mode");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    auto* unit=game->players.front()->mercs.front().GetPtr();NWorld::CUnitServer* actor=nullptr;
    vector<CPtr<NWorld::CUnit>> people;world->GetAllUnits(&people);
    for(auto person:people)if(person->GetRPG()->GetRPGUnit()==unit){actor=dynamic_cast<NWorld::CUnitServer*>(person.GetPtr());break;}
    Require(actor && actor->CanFight() && unit->GetDeathVP()<0,"VP overdose fixture lacks its actual living deployed hero/reserve");
    auto* mission=actor->GetUnitRPG();int originalModifiers=0;
    Require(NativeVPBoostDurations(mission,&originalModifiers).empty() && !mission->IsUnconscious(),"VP overdose fixture starts with existing drugs or unconsciousness");
    const vector<vector<int>> scenarios={{0},{1,1},{2,2},{2,3,6},{2,3,5},{3},{0}};
    const auto& durations=scenarios[mode];const bool dead=mode==6,overdose=mode==0 || mode==1 || mode==4;
    if(dead){unit->Kill();Require(mission->IsDead(),"dead VP overdose control is not actually dead");}
    const int hp=unit->Skills(NDb::ST_VP),ap=actor->GetAP();const float xp=unit->fXP;const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
    for(int duration:durations)mission->AddVPBoost(dead?0.f:10.f,duration);
    int modifierCount=0;const auto actual=NativeVPBoostDurations(mission,&modifierCount);
    std::printf("native VP immediate mode=%d doses=%zu durations=%zu modifiers=%d HP=%d->%d unconscious=%d dead=%d expectedOverdose=%d\n",mode,durations.size(),actual.size(),modifierCount,hp,int(unit->Skills(NDb::ST_VP)),int(mission->IsUnconscious()),int(mission->IsDead()),int(overdose));std::fflush(stdout);
    const bool cleared=overdose || dead;
    Require(cleared?actual.empty():actual==durations,"native VP boost did not clear an overdose or changed a safe duration list");
    Require(mission->IsUnconscious()==overdose && (dead?mission->IsDead() && int(unit->Skills(NDb::ST_VP))==hp:!mission->IsDead() && (overdose?int(unit->Skills(NDb::ST_VP))==0:int(unit->Skills(NDb::ST_VP))>hp && int(unit->Skills(NDb::ST_VP))<=unit->Skills(NDb::ST_VP).GetMaxValue())),
        "native VP overdose missed unconsciousness/zero-health or changed the safe/dead control");
    Require(modifierCount==originalModifiers+int(durations.size()),"overdose handling removed the existing skill modifiers instead of only the duration list");
    Require(actor->GetAP()==ap && unit->fXP==xp && actor->GetPosition()==position && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"VP overdose bookkeeping changed AP/XP/position/time or gameplay RNG");
}

static void VPBoostDurationCountdown(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=8,"unknown native VP countdown mode");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    auto* unit=game->players.front()->mercs.front().GetPtr();NWorld::CUnitServer* actor=nullptr;
    vector<CPtr<NWorld::CUnit>> people;world->GetAllUnits(&people);
    for(auto person:people)if(person->GetRPG()->GetRPGUnit()==unit){actor=dynamic_cast<NWorld::CUnitServer*>(person.GetPtr());break;}
    Require(actor && actor->CanFight() && unit->GetDeathVP()<0,"VP countdown fixture lacks its native deployed hero");
    auto* mission=actor->GetUnitRPG();const vector<vector<int>> scenarios={{3},{2,2},{4,4},{4,4},{1},{},{3,6},{2,2},{2,2}};
    const int ticks[]={1,1,2,3,1,2,2,1,1};const bool overdose=mode==1 || mode==3 || mode==4 || mode==6 || mode==8;
    for(int duration:scenarios[mode])mission->AddVPBoost(10.f,duration);
    Require(!mission->IsUnconscious() && NativeVPBoostDurations(mission)==scenarios[mode],"countdown fixture overdosed before its periodic edge");
    // Refreshing AP is a separate boundary and must not consume duration.
    mission->StartNewTurn(actor->GetPosition().GetCP());
    Require(NativeVPBoostDurations(mission)==scenarios[mode],"AP refresh consumed VP boost duration");
    int modifiers=0;NativeVPBoostDurations(mission,&modifiers);
    const int hp=unit->Skills(NDb::ST_VP),ap=actor->GetAP();const float xp=unit->fXP;const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
    if(mode==7)world->bNetworkReplica=true;
    for(int tick=0;tick<ticks[mode];++tick) {
        if(mode>=7)actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);
        else {int bleed=-1;mission->DoRegenerations(world,&bleed);Require(bleed==0,"VP countdown unexpectedly produced bleeding");}
    }
    vector<int> expected=scenarios[mode];if(mode!=7)for(auto& duration:expected)duration-=ticks[mode];if(overdose)expected.clear();
    int afterModifiers=0;const auto actual=NativeVPBoostDurations(mission,&afterModifiers);
    std::printf("native VP countdown mode=%d ticks=%d remaining=%zu first=%d HP=%d->%d unconscious=%d actorUnconscious=%d expectedOverdose=%d\n",mode,ticks[mode],actual.size(),actual.empty()?-999:actual.front(),hp,int(unit->Skills(NDb::ST_VP)),int(mission->IsUnconscious()),int(actor->IsUnconscious()),int(overdose));std::fflush(stdout);
    Require(actual==expected,"native periodic regeneration did not count down VP durations and recheck overdose");
    Require(mission->IsUnconscious()==overdose && !mission->IsDead() && int(unit->Skills(NDb::ST_VP))==(overdose?0:hp),"VP countdown changed the strict threshold or missed unconsciousness");
    Require(afterModifiers==modifiers,"VP countdown removed the existing skill modifiers");
    if(mode==8)Require(actor->IsUnconscious(),"native periodic callback did not synchronize actor unconsciousness");
    Require((overdose?actor->GetAP()<=ap && mission->HasCritical(NDb::C_VP):actor->GetAP()==ap),"VP countdown changed AP without the existing wound critical");
    Require(unit->fXP==xp && actor->GetPosition()==position && world->GetTime()->GetValue()==time,"VP countdown changed XP/position/time");
    if(mode<7)Require(random.Get()==GlobalGameRandom().Get(),"direct VP countdown changed gameplay RNG");
}

class DebugExplosionWorld: public NWorld::CWorld
{
    OBJECT_NOCOPY_METHODS(DebugExplosionWorld);
public:
    using NWorld::CWorld::AddGrenadeExplosion;
    explicit DebugExplosionWorld(NRPG::CGlobalGame* game=nullptr):NWorld::CWorld(game){}
    int blasts=0;CVec3 point;CPtr<NDb::CRPGGrenade> record;CPtr<NWorld::CUnitServer> owner;CPtr<CObjectBase> ignition;
    const NWorld::SPerkMineModifiers* modifiers=nullptr;
    void AddGrenadeExplosion(const CVec3& where,NDb::CRPGGrenade* grenade,NWorld::CUnitServer* unit,CObjectBase* igniter,const NWorld::SPerkMineModifiers* mods) override {
        ++blasts;point=where;record=grenade;owner=unit;ignition=igniter;modifiers=mods;
    }
};
static void DebugExplosionRouting(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=2,"unknown native debug explosion routing mode");
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(1);CObj<DebugExplosionWorld> world=new DebugExplosionWorld(game);
    CObj<NWorld::CPostWorldCreateInfo> post;world->CreateRandom(810,{},true,{},0,&post,SRandomSeed(123));world->RunPostInit(nullptr);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);Require(!units.empty(),"debug explosion fixture has no actual initial roster");
    if(mode==0){for(auto unit:units){auto* server=dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());Require(server,"debug explosion fixture contains a non-native actor");world->RemoveUnit(server);}units.clear();world->GetAllUnits(&units);Require(units.empty(),"empty-roster debug control retained a world unit");}
    CRay ray;CVec3 expectedPoint;CObjectBase* hitObject=nullptr;int hitLocation=-1;bool hit=false;
    const auto bounds=world->GetMapSafeZone();
    vector<CVec3> candidates;
    for(auto object:world->GetObjectsForHarness())candidates.push_back(object->GetPosition().ptPos);
    for(int y=0;y<=4;++y)for(int x=0;x<=4;++x)candidates.emplace_back(bounds.minx+(bounds.maxx-bounds.minx)*float(x+1)/6.f,bounds.miny+(bounds.maxy-bounds.miny)*float(y+1)/6.f,0);
    for(const auto& candidate:candidates){ray.ptOrigin=CVec3(candidate.x,candidate.y,1000);ray.ptDir=CVec3(0,0,-1);if(NWorld::TraceRay(world,ray,1000,&hitObject,&hitLocation,&expectedPoint)){hit=true;break;}}
    Require(hit,"debug explosion fixture found no real native collision point");
    if(mode==2){ray.ptOrigin=CVec3(100000,100000,1000);ray.ptDir=CVec3(0,0,1);Require(!NWorld::TraceRay(world,ray,1000,&hitObject,&hitLocation,&expectedPoint),"no-hit control still intersects native map geometry");}
    Require(NDb::GetRPGGrenade(16) && NDb::GetRPGGrenade(21),"debug explosion fixture lacks original/legacy authored presets");
    const auto time=world->GetTime()->GetValue();const bool action=world->IsAction();auto random=GlobalGameRandom();
    world->MakeExplosion(ray,1000);
    std::printf("native debug explosion mode=%d units=%zu calls=%d preset=%d owner=%d ignition=%d modifiers=%d pointError=%.6g\n",mode,units.size(),world->blasts,world->record?world->record->GetRecordID():-1,int(IsValid(world->owner)),int(IsValid(world->ignition)),int(world->modifiers!=nullptr),world->blasts?fabs2(world->point-expectedPoint):0);std::fflush(stdout);
    Require(mode==2?world->blasts==0:world->blasts==1 && world->record.GetPtr()==NDb::GetRPGGrenade(16) && !world->owner && !world->ignition && !world->modifiers && fabs2(world->point-expectedPoint)<1e-8f,
        "debug explosion required an arbitrary unit or forwarded the wrong authored preset/owner/perk context");
    Require(world->GetTime()->GetValue()==time && world->IsAction()==action && random.Get()==GlobalGameRandom().Get(),"debug explosion producer advanced world time/action/RNG before the captured blast boundary");
}

static void MainMenuVersionCaption(const char* databasePath,const char* resourcePath)
{
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm native main-menu version regression");
    CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    CObj<NGame::CMissionBase> menu=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* page=menu?menu->GetInterface()->GetChildByID("mainmenuUI"):nullptr;
    auto* version=page?dynamic_cast<NUI::CText*>(page->GetChildByID("version")):nullptr;
    Require(menu && page && version && menu->GetWorld() && menu->GetRenderGame(),"main-menu version fixture lost its native world/menu/template caption");
    const auto localized=NUI::GetDBString(0x4f17);Require(!localized.empty(),"main-menu version fixture has no localized version string");
    const wstring prefix=L"<font size=14pt face=Courier><right><color=grey>"+localized+L" ";
    auto verify=[&]() {
        const auto text=version->GetText();
        std::printf("native main-menu version: actual=");
        for(auto ch:text)if(ch>=32 && ch<=126)std::putchar(int(ch));else std::printf("\\u%04x",unsigned(ch));
        std::printf(" localizedLength=%zu\n",localized.size());std::fflush(stdout);
        Require(text.find(prefix)==0,"main-menu version retained its template caption instead of localized original styling/version/date");
        const auto date=text.substr(prefix.size());
        const wstring months=L"Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec";
        Require(date.size()==11 && date[3]==L' ' && date[6]==L' ' && months.find(date.substr(0,3))!=wstring::npos &&
            std::wcstol(date.c_str()+3,nullptr,10)>=1 && std::wcstol(date.c_str()+3,nullptr,10)<=31 &&
            std::wcstol(date.c_str()+7,nullptr,10)>=2000,"main-menu version omitted or duplicated its build date");
        return text;
    };
    const auto first=verify();ICamera::SCameraPos camera;menu->GetCamera()->GetPlacement(&camera);
    const auto currentWorld=menu->GetWorld();const auto depth=NMainLoop::GetInterfaceStackDepth();
    version->SetText(L"stale template version",true);
    page->ProcessMessage(NUI::SEvent(NUI::EVENT_TEMPLATELOADCOMPLETE));
    Require(verify()==first,"repeated menu template completion appended a second version/date or retained stale text");
    ICamera::SCameraPos after;menu->GetCamera()->GetPlacement(&after);
    Require(menu->GetWorld()==currentWorld && after.ptAnchor==camera.ptAnchor && after.fRod==camera.fRod && NMainLoop::GetInterfaceStackDepth()==depth && !NMainLoop::HaveInterfaceCommand(),"updating the menu version moved its camera, recreated its world or queued an interface command");
    NMainLoop::DoneInterface();
}

static void ChapterPanzerkleinGate(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=6,"unknown chapter PK gate mode");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm native chapter deployment gate regression");
    NGame::InitLoadingScreen();
    struct InfoRestore {CObj<CChapterInfo> info;vector<SChapterSector> original;CVec2 deploy;~InfoRestore(){if(info){info->sectorsSet.swap(original);info->vDeployPos=deploy;}}} restore;
    NDb::CChapterMap* map=nullptr;NDb::CDBScenarioZone* zoneRecord=nullptr;SChapterSector selected;
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    while(maps.MoveNext()) {
        auto* candidate=maps.Get();if(!IsValid(candidate->pBackground))continue;
        CDGPtr<CPtrFuncBase<CChapterInfo>> loader=shareChapterInfo.Get(candidate->GetRecordID());loader.Refresh();auto* info=loader->GetValue();if(!info)continue;
        for(const auto& sector:info->sectorsSet) {
            if(sector.eType!=ZONE || sector.pointsSet.size()<2)continue;
            auto* zone=NDb::GetDBScenarioZone(sector.nTemplate);if(!zone || !IsValid(zone->pScenario))continue;
            if(zone->bAllowPK!=(mode==3))continue;
            selected=sector;map=candidate;zoneRecord=zone;restore.info=info;break;
        }
        if(map)break;
    }
    Require(map && zoneRecord && restore.info,"chapter PK gate lacks an authored chapter zone with the requested deployment flag");
    restore.original=restore.info->sectorsSet;restore.deploy=restore.info->vDeployPos;
    restore.info->sectorsSet=vector<SChapterSector>{selected};
    restore.info->vDeployPos=selected.pointsSet.front()+CVec2(10,10);
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(zoneRecord->pScenario->GetRecordID());
    CObj<NRPG::CGlobalPlayer> player=NRPG::CreateGlobalPlayer(vector<int>{54,103});game->players.push_back(player);
    Require(player->mercs.size()==2,"chapter PK gate lacks its two actual hired/fielded mercs");
    CObj<NGame::CICBeginChapter> enter=new NGame::CICBeginChapter(map->GetRecordID(),game);enter->Exec();
    CObj<NGame::CMissionBase> chapter=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* ui=chapter?dynamic_cast<NUI::CChapterMapUI*>(chapter->GetInterface()->GetChildByID("chaptermapUI")):nullptr;
    auto* activeWorld=chapter?dynamic_cast<NWorld::CWorld*>(chapter->GetWorld()):nullptr;
    Require(ui && activeWorld && !NMainLoop::HaveInterfaceCommand(),"chapter PK fixture did not create its actual chapter UI/world");
    const string travel=ui->GetTravelForDiagnostics();std::printf("%s",travel.c_str());std::fflush(stdout);
    Require(travel.find(" type=0 template="+std::to_string(selected.nTemplate)+" visible=")!=string::npos && travel.find(" hit=1 ")!=string::npos,"chapter PK fixture did not place its real team marker inside its one authored zone");
    auto zone=game->pScenarioTracker->GetZoneByDBZone(zoneRecord);Require(IsValid(zone),"chapter PK fixture has no actual scenario-zone instance");
    NDb::CRPGPers* suit=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext())if(IsValid(people.Get()->pPanzerklein)){suit=people.Get();break;}
    Require(suit,"chapter PK fixture lacks an actual authored suit persona");
    auto* wearer=player->mercs.back().GetPtr();
    if(mode!=1)wearer->pPanzerklein=suit;
    if(mode==2)wearer->Kill();
    if(mode==4){player->mercs.pop_back();Require(std::find(player->totalMercs.begin(),player->totalMercs.end(),wearer)!=player->totalMercs.end(),"reserve-suit fixture lost its hired reserve");}
    if(mode==5)player->mercs.insert(player->mercs.begin(),nullptr);
    if(mode==6)game->players.clear();
    const bool blocked=mode==0 || mode==5;
    const auto warning=NUI::GetDBString(20995);Require(!warning.empty(),"chapter PK gate has no localized denial message");
    auto warnings=[&](){return std::count_if(consoleLines.begin(),consoleLines.end(),[&](const SConsoleLine& line){return line.eType==STREAM_GAME && line.szText.find(warning)!=wstring::npos;});};
    const int beforeWarnings=int(warnings()),money=player->nMoney;const auto position=game->vChapterPos;const auto time=activeWorld->GetTime()->GetValue();auto rng=GlobalGameRandom();
    Require(ui->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY,string("action"))),"chapter PK gate did not consume the native marker action");
    int commands=0,containers=0;
    while(NMainLoop::HaveInterfaceCommand()){CObj<NMainLoop::CInterfaceCommand> command=NMainLoop::TakeInterfaceCommandForHarness();++commands;if(dynamic_cast<NMainLoop::CICContainer*>(command.GetPtr()))++containers;}
    const int newWarnings=int(warnings())-beforeWarnings;
    std::printf("native chapter PK gate mode=%d map=%d zone=%d allowPK=%d dead=%d fielded=%zu players=%zu commands=%d containers=%d warnings=%d blocked=%d\n",
        mode,map->GetRecordID(),zoneRecord->GetRecordID(),int(zoneRecord->bAllowPK),int(wearer->IsDead()),player->mercs.size(),game->players.size(),commands,containers,newWarnings,int(blocked));std::fflush(stdout);
    Require(blocked?(commands==0 && newWarnings==1):(commands==1 && containers==1 && newWarnings==0),"chapter marker ignored the authored PK deployment ban or blocked a valid living/dead/reserve/allowed-zone control");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==chapter.GetPtr() && player->nMoney==money && game->vChapterPos==position && activeWorld->GetTime()->GetValue()==time && rng.Get()==GlobalGameRandom().Get(),"dry chapter deployment click changed the current mission, money, travel position, world time or gameplay RNG");
    NMainLoop::DoneInterface();
}

class WeatherEffectLifetime: public CObjectBase
{
    OBJECT_NOCOPY_METHODS(WeatherEffectLifetime);
    int* live=nullptr;
public:
    explicit WeatherEffectLifetime(int* count=nullptr):live(count){if(live)++*live;}
    ~WeatherEffectLifetime(){if(live)--*live;}
};
class WeatherPlacementView: public HeadStateView
{
    OBJECT_NOCOPY_METHODS(WeatherPlacementView);
public:
    int live=0,calls=0,wrapped=0;bool previousReleased=true;
    SFBTransform placement;CPtr<NDb::CEffect> effect;CPtr<CFuncBase<STime>> clock;
    CObjectBase* CreateParticles(NDb::CEffect* value,STime start,CFuncBase<STime>* time,const SFBTransform& transform,const NGScene::SRoomInfo&,NGScene::IParticleFilter*) override {
        Require(value && time && start==0,"weather placement lost its authored effect or render clock");
        previousReleased&=live==0;++calls;effect=value;clock=time;placement=transform;wrapped=0;
        for(auto entry:value->instances)if(IsValid(entry) && IsValid(entry->pParticle) && (entry->pParticle->vWrapSize.x>0 || entry->pParticle->vWrapSize.y>0))++wrapped;
        return new WeatherEffectLifetime(&live);
    }
};
class WeatherPlacementMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(WeatherPlacementMission);
public:
    using NGame::CMission::ShowWeatherEffect;
    ICamera* GetCamera() const override {return pCamera;}
    void Attach(NWorld::CWorld* world,WeatherPlacementView* view,NRender::IRenderGame* renderer){pWorld=world;pScene=view;pRender=renderer;pCamera=::CreateCamera(CAMERA_PC);}
};
static void WeatherWorldPlacement(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=3,"unknown native weather placement mode");
    auto world=FixtureWorld(databasePath,resourcePath);CObj<WeatherPlacementView> view=new WeatherPlacementView;
    CObj<NRender::IRenderGame> renderer=NRender::CreateRenderGame(world,view,nullptr);
    CObj<WeatherPlacementMission> mission=new WeatherPlacementMission;mission->Attach(world,view,renderer);
    const STime before=world->GetTime()->GetValue();const bool action=world->IsAction();auto random=GlobalGameRandom();
    for(int iteration=0;iteration<(mode==3?3:1);++iteration) {
        const CVec3 anchor=mode==2?VNULL3:iteration==1?CVec3(-71,35,9):CVec3(42,27,11);
        ICamera::SCameraPos camera;mission->GetCamera()->GetPlacement(&camera);camera.ptAnchor=anchor;mission->GetCamera()->SetPlacement(camera);
        const int id=mode==1 || iteration==1?732:817;mission->ShowWeatherEffect(id);
        const auto offset=view->placement.forward.GetTranslation();
        std::printf("native weather mode=%d iteration=%d template=%d anchor=%.1f,%.1f,%.1f offset=%.1f,%.1f,%.1f wrapped=%d live=%d calls=%d released=%d\n",
            mode,iteration,id,anchor.x,anchor.y,anchor.z,offset.x,offset.y,offset.z,view->wrapped,view->live,view->calls,int(view->previousReleased));std::fflush(stdout);
        Require(view->wrapped>0 && IsValid(view->effect) && view->clock.GetPtr()==renderer->GetTime(),"native weather fixture has no wrapped authored particle or lost its own render time node");
        Require(offset==VNULL3,"wrapped world-space weather was translated by the camera anchor a second time");
        for(const auto& point:{VNULL3,CVec3(1,0,0),CVec3(0,1,0),CVec3(0,0,1)}) {
            CVec3 forward,backward;view->placement.forward.RotateHVector(&forward,point);view->placement.backward.RotateHVector(&backward,point);
            Require(fabs2(forward-point)<1e-8f && fabs2(backward-point)<1e-8f,"weather placement changed native scale, rotation or inverse transform");
        }
        Require(view->live==1 && view->calls==iteration+1 && view->previousReleased,"weather replacement retained an older effect while creating the next one");
    }
    Require(world->GetTime()->GetValue()==before && world->IsAction()==action && random.Get()==GlobalGameRandom().Get(),"test weather changed world time, action count or gameplay RNG");
    mission=nullptr;Require(view->live==0,"weather handle outlived its owning mission");
}

static void UnitPreviewShotOrigin(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=2,"unknown unit preview shot origin mode");
    auto world=FixtureWorld(databasePath,resourcePath,true);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
    const NAI::EHitLocation location=mode==1?NAI::HL_HEAD:mode==2?NAI::HL_BODY:NAI::HL_ANY;
    int samples=0;
    for(auto entry:units) {
        auto* actor=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(!actor || !actor->CanFight() || actor->IsEmptyPK() || actor->IsWearingPK())continue;
        auto* firearm=actor->GetUnitRPG()->GetWeaponItem();
        if(!firearm || !firearm->SetShootMode(NDb::SM_Snap) || NRPG::GetToHitType(actor)!=NRPG::TH_SHOOT)continue;
        vector<NRPG::CAttackPortion> attacks;actor->GetUnitRPG()->CreateAttack(&attacks,false);if(attacks.empty())continue;
        const auto original=actor->GetPosition();
        struct Restore {NWorld::CUnitServer* actor;NAI::SUnitPosition position;~Restore(){actor->SetTemporaryPosition(position);}}restore{actor,original};
        for(auto stance:{NAI::CM_STAND,NAI::CM_CROUCH,NAI::CM_LAY}) {
            auto position=original;position.pos.p.SetPose(stance);actor->SetTemporaryPosition(position);
            for(auto other:units) {
                auto* target=dynamic_cast<NWorld::CUnitServer*>(other.GetPtr());
                if(!target || target==actor || !target->CanFight() || target->IsEmptyPK() || target->IsWearingPK())continue;
                auto aimed=position;aimed.pos.p.SetDirection(NRPG::GetShootDirection(aimed.pos.pNet,aimed.pos.p,target->GetPosition().GetCP()));
                CObj<NRPG::CCoverInfo> cover=world->GetGame()->CalcCovers(actor->GetAttackOrigin(aimed),attacks.front(),actor,target,location,actor->GetMinClearDistance());
                const auto captured=CoverSnapshot(cover);const int hitCover=NRPG::GetHitCover(actor,cover);if(hitCover<=0)continue;
                const int distance=int(fabs(captured.source-target->GetPosition().GetCP())*FP_INV_GRID_STEP);
                const int legacyDistance=int(fabs(position.GetCP()-target->GetPosition().GetCP())/FP_GRID_STEP);
                vector<int> accessible;const bool backstab=!target->IsUnitVisible(actor);
                auto chance=[&](int d,const CVec3& origin){return NRPG::GetToHit(actor,position.GetPose(),d,origin,target->GetPosition().pos,location,
                    actor->GetCarefulShotExtraAP(),target,accessible,hitCover,actor->GetUnitRPG()->IsFirstTurn(),CVec3(1,1,1),backstab,0,world->GetGame()->IsNight());};
                const int expected=chance(distance,captured.source),legacy=chance(legacyDistance,position.GetCenter());++samples;
                if(expected==legacy || expected<=2 || expected>=100)continue;
                const int ap=actor->GetAP(),hp=target->GetUnitRPG()->GetTotalVP(),ammo=firearm->GetAmmoQuantity();auto rng=GlobalGameRandom();
                std::printf("native shot context mode=%d visible=%d audible=%d night=%d backstab=%d\n",mode,int(target->IsUnitVisible(actor)),int(target->IsUnitAudible(actor)),int(world->GetGame()->IsNight()),int(backstab));
                const int actual=world->GetGame()->GetCompositeToHit(actor,target,location,actor->GetUnitRPG()->IsFirstTurn());
                std::printf("native shot origin mode=%d stance=%d samples=%d distance=%d/%d originDelta=%.6g cover=%d chance=%d expected=%d legacy=%d originOnly=%d distanceOnly=%d\n",
                    mode,int(stance),samples,distance,legacyDistance,fabs(captured.source-position.GetCenter()),hitCover,actual,expected,legacy,
                    chance(legacyDistance,captured.source),chance(distance,position.GetCenter()));std::fflush(stdout);
                Require(actual==expected,"unit preview calculated distance or target area from the actor tile/center instead of its actual cover-ray origin");
                Require(actor->GetPosition()==position && actor->GetAP()==ap && target->GetUnitRPG()->GetTotalVP()==hp && firearm->GetAmmoQuantity()==ammo && !actor->HasCommand() && rng.Get()==GlobalGameRandom().Get(),
                    "unit preview changed native temporary stance, AP, health, ammunition, action queue or gameplay RNG");return;
            }
        }
    }
    std::printf("native shot origin examined samples=%d\n",samples);std::fflush(stdout);
    Require(false,"native origin fixture has no observable unclamped chance difference");
}

// Observe the world's public map path used by the shared native selector.
// The proxy forwards to the same real map; no world/grid fields are replaced.
class NativeMeleePreviewHullWorld : public NWorld::CWorld
{
    OBJECT_NOCOPY_METHODS(NativeMeleePreviewHullWorld);
public:
    CPtr<NAI::IAIMap> observedMap;
    explicit NativeMeleePreviewHullWorld(NRPG::CGlobalGame* game=nullptr):NWorld::CWorld(game){}
    NAI::IAIMap* GetAIMap() override { return observedMap ? observedMap.GetPtr() : NWorld::CWorld::GetAIMap(); }
};

class NativeMeleePreviewHullMap : public CoverOriginMapProbe
{
    OBJECT_NOCOPY_METHODS(NativeMeleePreviewHullMap);
public:
    int calls=0;
    CObjectBase* received=nullptr;
    vector<int> locations;
    explicit NativeMeleePreviewHullMap(NAI::IAIMap* map=nullptr):CoverOriginMapProbe(map){}
    void GetAccessibleUnitHL(vector<int>* out,const CVec3& from,CObjectBase* hull,float distance) override
    {
        ++calls;received=hull;native->GetAccessibleUnitHL(out,from,hull,distance);locations=*out;
    }
};

static void MeleeReachPreview(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<9,"unknown melee reach preview case");const bool distant=mode==1 || mode==2 || mode==3 || mode==4 || mode==6 || mode==7,unarmed=mode==4 || mode==5,scripted=mode==6 || mode==7;const auto location=mode==2 || mode==8?NAI::HL_HEAD:mode==3?NAI::HL_BODY:NAI::HL_ANY;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsWearingPK() && !unit->IsEmptyPK()){actor=unit;break;}Require(actor,"melee reach preview lacks actual native actor");auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous=inventory->TakeOff(NDb::SLOT_1);CObj<NRPG::CMeleeWeaponItem> blade;CObj<NRPG::CWeaponItem> gun;
    if(!unarmed && mode!=7){NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> records(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());while(records.MoveNext()){auto* p=records.Get();if(p->bThrowing==(mode==6) && p->pItem && p->pItem->pModel && (!record || p->GetRecordID()<record->GetRecordID()))record=p;}Require(record,"melee reach preview lacks authored real blade");blade=new NRPG::CMeleeWeaponItem(record);blade->nQuantity=1;Require(inventory->Equip(NDb::SLOT_1,blade) && inventory->Activate(NDb::SLOT_1),"melee reach preview cannot equip native blade");}
    if(mode==7){NDb::CRPGWeapon* record=nullptr;CDBIterator<NDb::CRPGWeapon> records(*NDatabase::GetTable<NDb::CRPGWeapon>());while(records.MoveNext()){auto* p=records.Get();if(p->shootModes[NDb::SM_Snap] && !p->bBazookaLogic && p->pItem && p->pItem->pModel && p->pInnerClip && p->pWeaponType && (!record || p->GetRecordID()<record->GetRecordID()))record=p;}Require(record,"melee reach preview lacks actual authored firearm");gun=new NRPG::CWeaponItem(record);Require(gun->SetShootMode(NDb::SM_Snap) && gun->GetAmmoQuantity()>0 && inventory->Equip(NDb::SLOT_1,gun) && inventory->Activate(NDb::SLOT_1),"melee reach preview cannot equip loaded firearm");}
    const auto action=NRPG::GetToHitType(actor);Require(action==(mode==6?NRPG::TH_THROWING:mode==7?NRPG::TH_SHOOT:NRPG::TH_MELEE),"melee reach preview has incorrect native action type");actor->animator.SetActiveItem(!unarmed);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());actor->animator.AlignTime();actor->animator.PlaceUnit(actor->GetPosition());auto* network=world->GetPathNetwork();const auto from=actor->GetPosition();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(from.GetCP(),distant?5.f:1.f),&places);NAI::SPathPlace targetPlace;bool found=false;
    for(auto p:places){if(!p.IsIntegral() || p.IsFinal())continue;p.SetPose(NAI::CM_STAND);p.SetMoving(0);NAI::SPosition at(p,network);const auto delta=at.GetCP()-from.GetCP();if(fabs(delta.z)>.1f || (distant?fabs2(delta)<sqr(3.f):fabs2(delta)<.1f || fabs2(delta)>sqr(F_MELEE_DISTANCE)) || !network->IsPassable(p) || network->IsLocked(p))continue;targetPlace=p;found=true;break;}Require(found,"melee reach preview has no actual free target placement");CObj<NWorld::CUnitServer> target=world->AddUnitInGame(targetPlace,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer(),"NativeMeleeReachTarget");Require(target && target->CanFight(),"melee reach preview cannot create real target");target->animator.AlignTime();target->animator.PlaceUnit(target->GetPosition());MarkNewDGFrame();world->GetAIMap()->Sync();world->GetGame()->SetNight(false);CVec3 center;Require(world->GetAIMap()->GetUnitHLPos(&center,world->GetAIMap()->GetHull(target),NAI::HL_ANY),"melee reach preview lacks actual target hull center");const bool reachable=NWorld::CanMeleeAttack(actor,from,center);Require(reachable==!distant,"melee reach preview independent native reach differs from actual near/distant placement");if(scripted)actor->SetScriptToHit(37);
    CObj<NScript::CScript> script=NScript::CreateScript(world);NScript::luaPushCPtr(*script,actor);script->SetGlobal("upstreamReachActor");NScript::luaPushCPtr(*script,target.GetPtr());script->SetGlobal("upstreamReachTarget");const int hp=actor->GetUnitRPG()->GetTotalVP(),ap=actor->GetAP(),targetHP=target->GetUnitRPG()->GetTotalVP(),quantity=inventory->GetActive()?inventory->GetActive()->GetQuantity():0,ammo=gun?gun->GetAmmoQuantity():-1;const auto targetPosition=target->GetPosition();const auto time=world->GetTime()->GetValue();auto* active=inventory->GetActive();GlobalGameRandom().SeedForHarness(726);auto random=GlobalGameRandom();const int chance=world->GetGame()->GetCompositeToHit(actor,target,location,world->IsFirstTurn());RunScript(script,"MeleeReachLuaChance=UnitGetToHitUnit(upstreamReachActor,upstreamReachTarget,1); MeleeReachLuaDone=17");const auto result=script->GetGlobal("MeleeReachLuaChance"),done=script->GetGlobal("MeleeReachLuaDone");Require(result.IsNumber() && done.IsNumber() && done.GetNumber()==17,"actual Lua melee query did not return its numeric result");const int luaChance=int(result.GetNumber());auto after=GlobalGameRandom();unsigned tails[4];bool same=true;for(int i=0;i<4;++i){tails[i]=after.Get();same&=tails[i]==random.Get();}
    const bool pure=actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && target->GetUnitRPG()->GetTotalVP()==targetHP && actor->GetPosition()==from && target->GetPosition()==targetPosition && inventory->GetActive()==active && (!active || active->GetQuantity()==quantity) && (!gun || gun->GetAmmoQuantity()==ammo) && time==world->GetTime()->GetValue() && !actor->HasCommand();std::printf("MELEE-REACH mode=%d distant=%d unarmed=%d scripted=%d action=%d location=%d reachable=%d chance=%d luaHead=%d HP=%d AP=%d targetHP=%d quantity=%d ammo=%d pure=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,int(distant),int(unarmed),int(scripted),int(action),int(location),int(reachable),chance,luaChance,hp,ap,targetHP,quantity,ammo,int(pure),int(same),tails[0],tails[1],tails[2],tails[3]);std::fflush(stdout);Require(pure && same,"melee reach query changed native actor/target/item/time/command/RNG controls");if(distant && !scripted)Require(chance==-1 && luaChance==-1,"unreachable melee preview returned a numeric chance instead of the unavailable sentinel through native game/Lua");else if(scripted)Require(chance==37 && luaChance==37,"ranged native/Lua controls lost actual script37 override");else Require(chance>=0 && luaChance>=0,"reachable native/Lua melee control lost a real numeric chance");
}


static void PreviewRocketCover(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<2,"unknown native rocket-cover preview mode");const bool unit=mode==0,blocked=mode==1;CObj<NRPG::CGlobalGame> global;auto world=FixtureWorld(databasePath,resourcePath,true,&global);auto* network=dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());Require(network,"rocket preview lacks actual native path network");list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;for(auto entry:units)if(entry && entry->CanFight() && !entry->IsAIUnit() && !entry->IsWearingPK() && !entry->IsEmptyPK()){actor=entry;break;}Require(actor,"rocket preview lacks actual party actor");auto* mission=actor->GetUnitRPG();auto* inventory=mission->GetInventory();NDb::CRPGWeapon* record=nullptr;CDBIterator<NDb::CRPGWeapon> rows(*NDatabase::GetTable<NDb::CRPGWeapon>());while(rows.MoveNext()){auto* p=rows.Get();if(p->bBazookaLogic && p->pItem && p->pItem->pModel && p->pInnerClip && p->pWeaponType && (!record || p->GetRecordID()<record->GetRecordID()))record=p;}Require(record,"rocket preview lacks authored launcher");CObj<NRPG::CWeaponItem> launcher=new NRPG::CWeaponItem(record);launcher->nQuantity=1;CObj<NRPG::IInventoryItem> old=inventory->TakeOff(NDb::SLOT_1);Require(launcher->GetAmmoQuantity()>0 && inventory->Equip(NDb::SLOT_1,launcher) && inventory->Activate(NDb::SLOT_1) && NRPG::GetToHitType(actor)==NRPG::TH_RLAUNCHER,"rocket preview cannot equip actual loaded launcher");
    actor->SetWalkWithoutWeapon(false);actor->SetUndrawItem(false);actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(mission->GetWeaponType());actor->animator.AlignTime();actor->animator.PlaceUnit(actor->GetPosition());mission->StartNewTurn(actor->GetPosition().GetCP());MarkNewDGFrame();world->GetAIMap()->Sync();world->GetGame()->SetNight(false);auto position=actor->GetPosition();vector<NRPG::CAttackPortion> attacks;mission->CreateAttack(&attacks,false);Require(!attacks.empty() && attacks.front().nK>1,"rocket preview lacks native projectile momentum greater than unit trace coefficient");auto nativePortion=attacks.front();nativePortion.nK=1;
    CObj<NWorld::CUnitServer> target;CObj<NRPG::CCoverInfo> selected,legacySelected;CVec3 point=VNULL3;float fraction=-1,legacyFraction=-1;int expected=-1,legacyChance=-1,examined=0,rayCount=0,penetrating=0,nativeInteger=-2;
    auto fractionOf=[&](const CObj<NRPG::CCoverInfo>& cover){const auto snapshot=CoverSnapshot(cover);int count=0;for(const auto& ray:snapshot.hits)if(ray.penetrate)++count;return count>0 && !snapshot.hits.empty()?(100.f*count/snapshot.hits.size())*((snapshot.piercing/count)/attacks.front().nK):0.f;};
    vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(position.GetCP(),20.f),&places);
    for(auto p:places){
        if(!p.IsIntegral() || p.IsFinal())continue;p.SetPose(NAI::CM_STAND);p.SetMoving(0);const auto ground=NAI::SPosition(p,network).GetCP();if(fabs2(ground-position.GetCP())<4.f || fabs(ground.z-position.GetCP().z)>2.f)continue;
        if(unit && (!network->IsPassable(p) || network->IsLocked(p)))continue;
        if(unit){if(!target)target=world->AddUnitInGame(p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer(),"NativeRocketPreviewTarget");else{auto at=target->GetPosition();at.pos=NAI::SPosition(p,network);target->SetPosition(at);}Require(target && target->CanFight(),"rocket preview cannot deploy actual native human target");target->GetUnitRPG()->StartNewTurn(target->GetPosition().GetCP());target->animator.AlignTime();target->animator.PlaceUnit(target->GetPosition());MarkNewDGFrame();world->GetAIMap()->Sync();world->GetGame()->SetNight(false);}
        for(float height:{0.f,.625f,1.25f}){
            if(unit && height!=0)continue;++examined;const auto at=unit?target->GetPosition().GetCP():ground+CVec3(0,0,height);auto directed=position;directed.pos.p.SetDirection(NRPG::GetShootDirection(network,position.pos.p,at));const auto pass=network->GetPassability(directed.pos.p);if(pass==NAI::AIP_NOT_PASSABLE || pass==NAI::AIP_CANNOT_LAY)continue;const auto origin=actor->GetAttackOrigin(directed);
            CObj<NRPG::CCoverInfo> cover=unit?world->GetGame()->CalcCovers(origin,nativePortion,actor,target,NAI::HL_ANY,actor->GetMinClearDistance()):world->GetGame()->CalcCoversForTile(origin,nativePortion,actor,at,actor->GetMinClearDistance());if(!cover)continue;const auto snapshot=CoverSnapshot(cover);if(snapshot.hits.empty() && !blocked)continue;const float value=fractionOf(cover);if(blocked?value!=0:value<=0 || value>=1)continue;
            CObj<NRPG::CCoverInfo> previous=unit?world->GetGame()->CalcCovers(origin,attacks.front(),actor,target,NAI::HL_ANY,actor->GetMinClearDistance()):world->GetGame()->CalcCoversForTile(origin,attacks.front(),actor,at,actor->GetMinClearDistance());if(!previous)continue;const float oldValue=fractionOf(previous);if(oldValue<=0)continue;const int distance=int(fabs(snapshot.source-at)*FP_INV_GRID_STEP);CObj<NRPG::CRLauncherToHitCalcer> correct=new NRPG::CRLauncherToHitCalcer(actor,position.GetPose(),distance,snapshot.source,value,actor->GetCarefulShotExtraAP(),mission->IsFirstTurn(),false,CVec3(1,1,1),unit?CVec3(1,1,1):at);CObj<NRPG::CRLauncherToHitCalcer> oldCalcer=new NRPG::CRLauncherToHitCalcer(actor,position.GetPose(),distance,snapshot.source,oldValue,actor->GetCarefulShotExtraAP(),mission->IsFirstTurn(),false,CVec3(1,1,1),unit?CVec3(1,1,1):at);const int chance=correct->GetToHit(),oldChance=oldCalcer->GetToHit();if(oldChance<=2 || oldChance>=100 || (blocked?chance!=0:chance!=oldChance))continue;
            point=at;selected=cover;legacySelected=previous;fraction=value;legacyFraction=oldValue;expected=chance;legacyChance=oldChance;rayCount=int(snapshot.hits.size());for(const auto& ray:snapshot.hits)if(ray.penetrate)++penetrating;nativeInteger=NRPG::GetHitCover(actor,cover);break;
        }if(selected)break;
    }
    std::printf("ROCKET-PREVIEW-INPUT mode=%d unit=%d blocked=%d found=%d examined=%d weapon=%d nK=%d fraction=%.9g oldFraction=%.9g integerCover=%d rays=%d penetrating=%d pointX=%.9g pointY=%.9g pointZ=%.9g expected=%d legacy=%d\n",mode,int(unit),int(blocked),int(selected.GetPtr()!=nullptr),examined,record->GetRecordID(),attacks.front().nK,fraction,legacyFraction,nativeInteger,rayCount,penetrating,point.x,point.y,point.z,expected,legacyChance);std::fflush(stdout);Require(selected && legacySelected && expected>=0 && legacyChance>0,"rocket preview lacks actual native fractional-cover clear/control or weak-obstacle blocked witnesses");
    const int hp=mission->GetTotalVP(),ap=actor->GetAP(),ammo=launcher->GetAmmoQuantity(),targetHP=target?target->GetUnitRPG()->GetTotalVP():-1;const auto targetPosition=target?target->GetPosition():position;const auto time=world->GetTime()->GetValue();auto* active=inventory->GetActive();GlobalGameRandom().SeedForHarness(731);auto reference=GlobalGameRandom();vector<int> locations;const int helper=unit?NRPG::GetAttackerToHit(actor,target,actor->GetCarefulShotExtraAP(),NAI::HL_ANY,locations,selected,mission->IsFirstTurn(),0):NRPG::GetAttackerTileToHit(actor,point,actor->GetCarefulShotExtraAP(),NAI::THL_MIDDLE,selected,mission->IsFirstTurn(),0);const int game=unit?world->GetGame()->GetCompositeToHit(actor,target,NAI::HL_ANY,false):world->GetGame()->GetTileCompositeToHit(actor,point,NAI::THL_MIDDLE,false);const int actual=mode==0?helper:game,control=mode==0?game:helper,controlExpected=mode==0?expected:0;
    auto after=GlobalGameRandom();bool rng=true;unsigned tail[4];for(int i=0;i<4;++i){tail[i]=after.Get();rng&=tail[i]==reference.Get();}const bool pure=actor->GetPosition()==position && hp==mission->GetTotalVP() && ap==actor->GetAP() && ammo==launcher->GetAmmoQuantity() && active==inventory->GetActive() && active->GetQuantity()==1 && time==world->GetTime()->GetValue() && !actor->HasCommand() && (!target || (target->GetPosition()==targetPosition && target->GetUnitRPG()->GetTotalVP()==targetHP));
    std::printf("ROCKET-PREVIEW mode=%d unit=%d blocked=%d actual=%d expected=%d control=%d controlExpected=%d helper=%d game=%d HP=%d AP=%d ammo=%d targetHP=%d pure=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,int(unit),int(blocked),actual,expected,control,controlExpected,helper,game,hp,ap,ammo,targetHP,int(pure),int(rng),tail[0],tail[1],tail[2],tail[3]);std::fflush(stdout);Require(pure && rng,"rocket cover queries changed real native physical/inventory/time/RNG controls");Require(control==controlExpected,"rocket cover independent legacy/control path changed");Require(actual==expected,"rocket preview lost a positive fractional gate or traced through an obstacle with the projectile momentum coefficient");
}

class PreviewObjectWireMission: public NGame::CMissionBase
{
    OBJECT_NOCOPY_METHODS(PreviewObjectWireMission);
public:
    vector<CPtr<NGame::IUnitTracker>> selected;
    CPtr<CObjectBase> target;
    CVec3 point=VNULL3;
    void Attach(NWorld::CWorld* world,NRPG::CGlobalGame* game){pWorld=world;pGlobalGame=game;}
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override{return false;}
    int CountSelected() override{return int(selected.size());}
    bool IsRealTime() const override{return false;}
    void GetSelectedUnits(vector<CPtr<NGame::IUnitTracker>>* out) const override{*out=selected;}
    CObjectBase* GetStateTarget(bool* fromWorld=nullptr) const override{if(fromWorld)*fromWorld=true;return target;}
    bool GetTracePosition(CVec3* out) const override{*out=point;return true;}
    void CanDoCommand(NWorld::CCmd*,bool,NGame::SActionInfo* info) override{*info=NGame::SActionInfo();info->SetValid();info->nMinAP=info->nMaxAP=7;}
};
class PreviewObjectWireState: public NGame::CStateAttack
{
public:
    void Attach(NGame::IMission* mission){CStateBase::Initialize(mission);}
    void Refresh(){UpdateInfo();UpdateCursorInfo();}
};

static void PreviewObjectWire(const char* databasePath,const char* resourcePath)
{
    GlobalGameRandom().SeedForHarness(42);std::srand(42);CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
      S2Net::NetworkMatchPreset preset;preset.squadSize=1;preset.personas={54};auto host=std::make_shared<S2Net::NetworkWorld>(true,preset),client=std::make_shared<S2Net::NetworkWorld>(false);
      NWorld::CUnitServer* actor=nullptr;NWorld::CObjectServerBase* hostTarget=nullptr;int stage=0,delivered=0;bool pureAll=true,randomAll=true;std::vector<unsigned> kinds,targets;std::vector<int> chances;
      auto hostCallbacks=host->Callbacks(),clientCallbacks=client->Callbacks();auto nativeQuery=hostCallbacks.query;auto nativeResult=clientCallbacks.result;
      hostCallbacks.query=[&](unsigned slot,const S2Net::Bytes& bytes){
        Require(actor && hostTarget && bytes.size()>=24,"object wire query lacks real actor/target/packet");size_t at=4;const auto kind=S2Net::Get32(bytes,at);at=12;const auto targetID=S2Net::Get32(bytes,at);const auto position=actor->GetPosition();const int hp=actor->GetUnitRPG()->GetTotalVP(),ap=actor->GetAP(),targetHP=hostTarget->GetHP();const auto time=actor->GetWorld()->GetTime()->GetValue();auto random=GlobalGameRandom();auto reply=nativeQuery(slot,bytes);auto after=GlobalGameRandom();bool same=true;for(int i=0;i<4;++i)same&=random.Get()==after.Get();const bool pure=actor->GetPosition()==position && hp==actor->GetUnitRPG()->GetTotalVP() && ap==actor->GetAP() && targetHP==hostTarget->GetHP() && time==actor->GetWorld()->GetTime()->GetValue();pureAll&=pure;randomAll&=same;at=8;const auto status=S2Net::Get32(reply,at),value=S2Net::Get32(reply,at);kinds.push_back(kind);targets.push_back(targetID!=0);std::printf("NETWORK-OBJECT-PREVIEW stage=%d door=%d kind=%u targetPresent=%d chance=%d status=%u pure=%d RNGsame=%d\n",stage,int(dynamic_cast<NWorld::CWindowDoor*>(hostTarget)!=nullptr),unsigned(kind),int(targetID!=0),int(std::int32_t(value)),unsigned(status),int(pure),int(same));Require(status==NWorld::UCR_OK,"object wire authoritative native query failed");return reply;
      };
      clientCallbacks.result=[&](std::uint64_t id,const S2Net::Bytes& bytes,bool preview){nativeResult(id,bytes,preview);if(preview){size_t at=8;const auto status=S2Net::Get32(bytes,at),chance=S2Net::Get32(bytes,at);Require(status==NWorld::UCR_OK,"object wire reply failed");chances.push_back(int(std::int32_t(chance)));++delivered;}};
      auto server=std::make_shared<S2Net::NetworkSession>(S2Net::Compatibility{"object-preview-wire","data"},hostCallbacks),peer=std::make_shared<S2Net::NetworkSession>(S2Net::Compatibility{"object-preview-wire","data"},clientCallbacks);server->Host(0);peer->Connect("127.0.0.1",server->Port());
      auto until=[&](auto condition){const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(15);while(!condition() && std::chrono::steady_clock::now()<deadline){server->Poll();peer->Poll();if(host->Match().world)while(CObj<NWorld::CUICmd> ui=host->Match().world->GetUICommand())server->Submit(host->EncodeCommand(new NWorld::CCmdInterfaceEvent(ui->GetID())));if(client->Match().world)while(CObj<NWorld::CUICmd> ui=client->Match().world->GetUICommand()){}Require(server->State()!=S2Net::SessionState::Failed && peer->State()!=S2Net::SessionState::Failed,"object preview wire connection failed");std::this_thread::sleep_for(std::chrono::milliseconds(2));}Require(condition(),"object preview wire timed out");};
      until([&]{return server->State()==S2Net::SessionState::Playing && peer->State()==S2Net::SessionState::Playing;});host->BindSession(server);client->BindSession(peer);actor=host->Match().players[1]->GetPlayerUnits().front().GetPtr();Require(actor && actor->CanFight() && actor->GetUnitRPG()->GetWeaponItem() && actor->GetUnitRPG()->GetWeaponItem()->GetAmmoQuantity()>0 && NRPG::GetToHitType(actor)==NRPG::TH_SHOOT,"object wire lacks actual owned loaded firearm actor");actor->SetScriptToHit(37);until([&]{return client->Match().players[1]->GetPlayerUnits().front()->GetScriptToHit()==37;});
      for(stage=0;stage<2;++stage){
        CVec3 center=VNULL3,surface=VNULL3;hostTarget=nullptr;auto* map=host->Match().world->GetAIMap();MarkNewDGFrame();map->Sync();
        for(auto object:host->Match().world->GetObjectsForHarness()){
          if(!object || !object->IsTargetable() || object->GetHP()<=0 || (dynamic_cast<NWorld::CWindowDoor*>(object.GetPtr())!=nullptr)!=(stage==1))continue;CVec3 point;if(!map->GetUnitHLPos(&point,map->GetHull(object),NAI::HL_ANY))continue;CRay ray;bool foundPick=false;CVec3 pickedPoint=VNULL3;for(const auto& direction:{CVec3(1,0,0),CVec3(-1,0,0),CVec3(0,1,0),CVec3(0,-1,0),CVec3(0,0,-1)}){ray.ptOrigin=point-direction*10;ray.ptDir=direction;vector<NAI::SInterval> hits;map->Trace(ray,&hits,NWorld::TS_PICK);const NAI::SInterval* first=nullptr;for(const auto& hit:hits)if(hit.enter.fT>0 && (!first || hit.enter.fT<first->enter.fT))first=&hit;if(first && first->pSrc->pUserData==object.GetPtr()){pickedPoint=ray.Get(first->enter.fT);foundPick=true;break;}}if(!foundPick)continue;hostTarget=object;center=point;surface=pickedPoint;break;
        }Require(hostTarget,"object wire lacks actual targetable prop/door and native cursor intersection");
        NWorld::CObjectServerBase* replicaTarget=nullptr;auto findTarget=[&]{auto* replicaMap=client->Match().world->GetAIMap();MarkNewDGFrame();replicaMap->Sync();for(auto object:client->Match().world->GetObjectsForHarness()){if(!object || object->GetDBObjectID()!=hostTarget->GetDBObjectID() || (dynamic_cast<NWorld::CWindowDoor*>(object.GetPtr())!=nullptr)!=(stage==1))continue;CVec3 point;if(replicaMap->GetUnitHLPos(&point,replicaMap->GetHull(object),NAI::HL_ANY) && fabs2(point-center)<.000001f){replicaTarget=object.GetPtr();return true;}}return false;};until(findTarget);
        auto* unit=client->Match().players[1]->GetPlayerUnits().front().GetPtr();Require(unit && unit->GetWorld()->bNetworkReplica && replicaTarget,"object wire lacks real replica actor/target");client->SetPreviewSelection({unit});CObj<PreviewObjectWireMission> mission=new PreviewObjectWireMission;mission->Attach(client->Match().world,client->Match().game);mission->target=replicaTarget;mission->point=surface;CObj<NGame::CUnitTracker> tracker=new NGame::CUnitTracker(mission,unit);mission->selected.push_back(tracker.GetPtr());CObj<PreviewObjectWireState> state=new PreviewObjectWireState;state->Attach(mission);const int before=delivered;state->Refresh();until([&]{return delivered>before;});state->Refresh();Require(state->GetCursorInfo().wsText.find(L"37%")!=wstring::npos && chances.back()==37,"actual attack caption lost the signed native37 result cache control");
      }
      peer->Finish("Object preview wire test complete");server->Poll();peer->Poll();std::printf("NETWORK-OBJECT-PREVIEW-COMPLETE replies=%d pure=%d RNGsame=%d\n",delivered,int(pureAll),int(randomAll));Require(pureAll && randomAll && chances==std::vector<int>({37,37}),"object wire physical/result controls failed");Require(kinds==std::vector<unsigned>({0,0}) && targets==std::vector<unsigned>({1,1}),"attack UI sent cursor tile queries instead of native object IDs for prop and door");return;
}

class PreviewCommandMission: public MeleeReachCaptionMission
{
    OBJECT_NOCOPY_METHODS(PreviewCommandMission);
public:
    CVec3 point=VNULL3;
    bool GetTracePosition(CVec3* out) const override { *out=point;return true; }
};

static void PreviewCommandObject(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<2,"unknown native command-object preview mode");const bool door=mode==1;CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);auto* network=dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());Require(network,"command-object preview lacks native path network");
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;for(auto u:units)if(u && u->CanFight() && !u->IsAIUnit() && !u->IsWearingPK() && !u->IsEmptyPK()){actor=u;break;}Require(actor,"command-object preview lacks actual party actor");auto* mission=actor->GetUnitRPG();auto* inventory=mission->GetInventory();auto gun=NativeEquipAccuracyType(mission->GetRPGUnit(),NDb::WT_PISTOL,NDb::SM_Snap);Require(gun && gun->GetAmmoQuantity()>0 && mission->GetBulletsQuantityInShot()==1,"command-object preview lacks actual loaded single-shot pistol");actor->SetWalkWithoutWeapon(false);actor->SetUndrawItem(false);actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(mission->GetWeaponType());mission->StartNewTurn(actor->GetPosition().GetCP());MarkNewDGFrame();world->GetAIMap()->Sync();world->GetGame()->SetNight(false);
    NWorld::CObjectServerBase* selected=nullptr;CVec3 center=VNULL3,surface=VNULL3,eye=VNULL3;int expected=-1,legacy=-1,examined=0,nativeCoverValue=-2,legacyCoverValue=-2,picked=0,zeroChances=0,positiveChances=0,sameChances=0;vector<NRPG::CAttackPortion> attacks;mission->CreateAttack(&attacks,false);Require(!attacks.empty(),"command-object preview lacks native attack portion");
    vector<CPtr<NWorld::CObjectServerBase>> objects;for(auto o:world->GetObjectsForHarness())if(o && o->IsTargetable() && o->GetHP()>0 && (dynamic_cast<NWorld::CWindowDoor*>(o.GetPtr())!=nullptr)==door)objects.push_back(o.GetPtr());
    for(auto object:objects){
        CVec3 target;auto* map=world->GetAIMap();if(!map->GetUnitHLPos(&target,map->GetHull(object),NAI::HL_ANY))continue;
        CRay cursor;CVec3 hitPoint=VNULL3;bool cursorFound=false;
        for(const auto& direction:{CVec3(1,0,0),CVec3(-1,0,0),CVec3(0,1,0),CVec3(0,-1,0),CVec3(0,0,-1)}){cursor.ptOrigin=target-direction*10;cursor.ptDir=direction;vector<NAI::SInterval> intervals;map->Trace(cursor,&intervals,NWorld::TS_PICK);const NAI::SInterval* first=nullptr;for(const auto& hit:intervals)if(hit.enter.fT>0 && (!first || hit.enter.fT<first->enter.fT))first=&hit;if(first && first->pSrc->pUserData==object.GetPtr()){hitPoint=cursor.Get(first->enter.fT);cursorFound=true;++picked;break;}}
        if(!cursorFound)continue;
        vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(target,10.f),&places);
        for(auto p:places){
            if(!p.IsIntegral() || p.IsFinal())continue;p.SetPose(NAI::CM_STAND);p.SetMoving(0);const auto ground=NAI::SPosition(p,network).GetCP();if(fabs2(ground-target)<4.f || !network->IsPassable(p) || network->IsLocked(p))continue;auto from=actor->GetPosition();p.SetDirection(NRPG::GetShootDirection(network,p,target));if(network->GetPassability(p)!=NAI::AIP_YES)continue;from.pos=NAI::SPosition(p,network);from.SetPose(NAI::WALK);actor->SetPosition(from);actor->animator.AlignTime();actor->animator.PlaceUnit(actor->GetPosition());mission->StartNewTurn(actor->GetPosition().GetCP());MarkNewDGFrame();map->Sync();world->GetGame()->SetNight(false);++examined;
            vector<NAI::SInterval> checked;map->Trace(cursor,&checked,NWorld::TS_PICK);const NAI::SInterval* closest=nullptr;for(const auto& hit:checked)if(hit.enter.fT>0 && (!closest || hit.enter.fT<closest->enter.fT))closest=&hit;if(!closest || closest->pSrc->pUserData!=object.GetPtr())continue;hitPoint=cursor.Get(closest->enter.fT);
            CObj<NRPG::CCoverInfo> cover=world->GetGame()->CalcCovers(actor->GetAttackOrigin(from),attacks.front(),actor,object,NAI::HL_ANY,actor->GetMinClearDistance());if(!cover)continue;const int chance=NRPG::GetAttackerTileToHit(actor,target,actor->GetCarefulShotExtraAP(),NAI::THL_LOWER,cover,mission->IsFirstTurn(),0);if(chance<0 || chance>100)continue;if(chance==0)++zeroChances;else ++positiveChances;
            auto directed=from;directed.pos.p.SetDirection(NRPG::GetShootDirection(network,from.pos.p,hitPoint));const auto desiredPass=network->GetPassability(directed.pos.p);if(desiredPass==NAI::AIP_NOT_PASSABLE || desiredPass==NAI::AIP_CANNOT_LAY)continue;CObj<NRPG::CCoverInfo> oldCover=world->GetGame()->CalcCoversForTile(actor->GetAttackOrigin(directed),attacks.front(),actor,hitPoint,actor->GetMinClearDistance());if(!oldCover)continue;const int oldChance=NRPG::GetAttackerTileToHit(actor,hitPoint,actor->GetCarefulShotExtraAP(),NAI::THL_MIDDLE,oldCover,mission->IsFirstTurn(),0);if(chance==oldChance){++sameChances;continue;}
            selected=object;center=target;surface=hitPoint;eye=cursor.ptOrigin;expected=chance;legacy=oldChance;nativeCoverValue=NRPG::GetHitCover(actor,cover);legacyCoverValue=NRPG::GetHitCover(actor,oldCover);break;
        }if(selected)break;
    }
    std::printf("OBJECT-PREVIEW-INPUT mode=%d found=%d examined=%d picked=%d zero=%d positive=%d same=%d record=%d expected=%d legacy=%d cover=%d oldCover=%d centerX=%.9g centerY=%.9g centerZ=%.9g surfaceX=%.9g surfaceY=%.9g surfaceZ=%.9g\n",mode,int(selected!=nullptr),examined,picked,zeroChances,positiveChances,sameChances,selected?selected->GetDBObjectID():-1,expected,legacy,nativeCoverValue,legacyCoverValue,center.x,center.y,center.z,surface.x,surface.y,surface.z);std::fflush(stdout);Require(selected && expected>=0 && expected!=legacy,"command-object preview lacks real ordinary object/door, actual cursor intersection and independent differing native references");
    CObj<PreviewCommandMission> ui=new PreviewCommandMission;ui->Attach(world,game);ui->target=selected;ui->point=surface;CObj<NGame::CUnitTracker> tracker=new NGame::CUnitTracker(ui,actor);ui->selection.push_back(tracker.GetPtr());CObj<AttackState> state=new AttackState;state->Attach(ui);CObj<NWorld::CCmd> command=state->TargetCommand();auto* objectCommand=dynamic_cast<NWorld::CCmdShootObject*>(command.GetPtr());Require(objectCommand && objectCommand->pTarget.GetPtr()==selected,"command-object preview lost native object command identity");NGame::SActionInfo info;info.SetValid();info.nMinAP=info.nMaxAP=7;SetAttackPreview(state,info);
    const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();const int hp=mission->GetTotalVP(),ap=actor->GetAP(),objectHP=selected->GetHP(),ammo=gun->GetAmmoQuantity();auto* active=inventory->GetActive();GlobalGameRandom().SeedForHarness(730);auto reference=GlobalGameRandom();state->RefreshCaption();const auto caption=state->GetCursorInfo().wsText;WCHAR percent[32];swprintf(percent,sizeof(percent)/sizeof(percent[0]),L"%d%%",expected);const auto expectedCaption=NUI::GetDBString(20276)+NUI::GetDBString(19808)+L"7"+NUI::GetDBString(expected>=2?20276:19807)+NUI::GetDBString(19809)+percent;int actual=-1;const auto end=caption.rfind(L'%');if(end!=wstring::npos){size_t begin=end;while(begin>0 && caption[begin-1]>=L'0' && caption[begin-1]<=L'9')--begin;actual=int(std::wcstol(caption.substr(begin,end-begin).c_str(),nullptr,10));}
    auto after=GlobalGameRandom();bool rng=true;unsigned tail[4];for(int i=0;i<4;++i){tail[i]=after.Get();rng&=tail[i]==reference.Get();}const bool pure=actor->GetPosition()==position && actor->GetAP()==ap && mission->GetTotalVP()==hp && selected->GetHP()==objectHP && gun->GetAmmoQuantity()==ammo && inventory->GetActive()==active && world->GetTime()->GetValue()==time && !actor->HasCommand();
    std::printf("OBJECT-PREVIEW mode=%d door=%d record=%d commandObject=1 actual=%d expected=%d legacy=%d HP=%d objectHP=%d AP=%d ammo=%d pure=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,int(door),selected->GetDBObjectID(),actual,expected,legacy,hp,objectHP,ap,ammo,int(pure),int(rng),tail[0],tail[1],tail[2],tail[3]);std::fflush(stdout);Require(pure && rng,"command-object caption query changed actual actor/object/inventory/time/RNG");Require(caption==expectedCaption,"attack caption used cursor tile fallback instead of the commanded native object hull");
}


static void PreviewObjectFacing(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<2,"unknown native object-facing preview mode");const bool blocked=mode==0;CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);auto* network=dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());Require(network,"object-facing preview lacks native path network");list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;for(auto u:units)if(u && u->CanFight() && !u->IsAIUnit() && !u->IsWearingPK() && !u->IsEmptyPK()){actor=u;break;}Require(actor,"object-facing preview lacks actual party actor");auto* mission=actor->GetUnitRPG();auto* inventory=mission->GetInventory();auto gun=NativeEquipAccuracyType(mission->GetRPGUnit(),NDb::WT_PISTOL,NDb::SM_Snap);Require(gun && gun->GetAmmoQuantity()>0 && NRPG::GetToHitType(actor)==NRPG::TH_SHOOT && mission->GetBulletsQuantityInShot()==1,"object-facing preview lacks actual loaded single-shot firearm");actor->SetWalkWithoutWeapon(false);actor->SetUndrawItem(false);actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(mission->GetWeaponType());mission->StartNewTurn(actor->GetPosition().GetCP());MarkNewDGFrame();world->GetAIMap()->Sync();world->GetGame()->SetNight(false);
    NWorld::CObjectServerBase* target=nullptr;CVec3 center=VNULL3,surface=VNULL3;int examined=0,currentPass=-1,desiredPass=-1,standingPass=-1,surfacePass=-1,expected=-2,expectedTile=-2;auto* map=world->GetAIMap();
    for(auto object:world->GetObjectsForHarness()){
        if(!object || !object->IsTargetable() || object->GetHP()<=0)continue;CVec3 point;if(!map->GetUnitHLPos(&point,map->GetHull(object),NAI::HL_ANY))continue;vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(point,10.f),&places);
        for(auto base:places){
            if(!base.IsIntegral() || base.IsFinal())continue;base.SetPose(NAI::CM_LAY);base.SetMoving(0);if(fabs2(NAI::SPosition(base,network).GetCP()-point)<4.f)continue;
            for(int direction=0;direction<8;++direction){
                auto source=base;source.SetDirection(NAI::EDirection(direction));auto desired=source;desired.SetDirection(NRPG::GetShootDirection(network,source,point));if(!blocked && desired.GetDirection()!=source.GetDirection())continue;auto standing=desired;standing.SetPose(NAI::CM_STAND);network->Unlock(actor);const int c=int(network->GetPassability(source)),d=int(network->GetPassability(desired)),s=int(network->GetPassability(standing));const bool free=network->IsPassable(source);network->Lock(actor,actor->GetPosition().pos.p);++examined;if(c!=NAI::AIP_YES || !free || s!=NAI::AIP_YES || (blocked?d!=NAI::AIP_NOT_PASSABLE && d!=NAI::AIP_CANNOT_LAY:d!=NAI::AIP_YES))continue;
                auto from=actor->GetPosition();from.pos=NAI::SPosition(source,network);from.SetPose(NAI::CRAWL);actor->SetPosition(from);actor->animator.AlignTime();actor->animator.PlaceUnit(actor->GetPosition());mission->StartNewTurn(actor->GetPosition().GetCP());MarkNewDGFrame();map->Sync();world->GetGame()->SetNight(false);bool picked=false;CVec3 pickedPoint=VNULL3;int wantedSurface=-1;
                for(const auto& rayDirection:{CVec3(0,0,-1),CVec3(1,0,0),CVec3(-1,0,0),CVec3(0,1,0),CVec3(0,-1,0)}){CRay ray;ray.ptOrigin=point-rayDirection*10;ray.ptDir=rayDirection;vector<NAI::SInterval> hits;map->Trace(ray,&hits,NWorld::TS_PICK);const NAI::SInterval* first=nullptr;for(const auto& hit:hits)if(hit.enter.fT>0 && (!first || hit.enter.fT<first->enter.fT))first=&hit;if(!first || first->pSrc->pUserData!=object.GetPtr())continue;const auto at=ray.Get(first->enter.fT);auto toSurface=actor->GetPosition().pos.p;toSurface.SetDirection(NRPG::GetShootDirection(network,toSurface,at));const int pass=int(network->GetPassability(toSurface));if(blocked?pass!=NAI::AIP_NOT_PASSABLE && pass!=NAI::AIP_CANNOT_LAY:pass==NAI::AIP_NOT_PASSABLE || pass==NAI::AIP_CANNOT_LAY)continue;picked=true;pickedPoint=at;wantedSurface=pass;break;}
                if(!picked)continue;
                target=object;center=point;surface=pickedPoint;currentPass=c;desiredPass=d;standingPass=s;surfacePass=wantedSurface;
                if(blocked){expected=expectedTile=-1;}else{vector<NRPG::CAttackPortion> attacks;mission->CreateAttack(&attacks,false);Require(!attacks.empty(),"object-facing preview lacks native attack portion");auto objectPose=actor->GetPosition();objectPose.pos.p.SetDirection(NRPG::GetShootDirection(network,objectPose.pos.p,center));CObj<NRPG::CCoverInfo> objectCover=world->GetGame()->CalcCovers(actor->GetAttackOrigin(objectPose),attacks.front(),actor,target,NAI::HL_ANY,actor->GetMinClearDistance());auto tilePose=actor->GetPosition();tilePose.pos.p.SetDirection(NRPG::GetShootDirection(network,tilePose.pos.p,surface));CObj<NRPG::CCoverInfo> tileCover=world->GetGame()->CalcCoversForTile(actor->GetAttackOrigin(tilePose),attacks.front(),actor,surface,actor->GetMinClearDistance());Require(objectCover && tileCover,"object-facing preview lacks real object and tile covers");expected=NRPG::GetAttackerTileToHit(actor,center,actor->GetCarefulShotExtraAP(),NAI::THL_LOWER,objectCover,mission->IsFirstTurn(),0);expectedTile=NRPG::GetAttackerTileToHit(actor,surface,actor->GetCarefulShotExtraAP(),NAI::THL_MIDDLE,tileCover,mission->IsFirstTurn(),0);}
                break;
            }if(target)break;
        }if(target)break;
    }
    std::printf("OBJECT-FACING-INPUT mode=%d blocked=%d found=%d examined=%d record=%d currentPass=%d desiredPass=%d surfacePass=%d standingPass=%d expected=%d expectedTile=%d centerX=%.9g centerY=%.9g centerZ=%.9g surfaceX=%.9g surfaceY=%.9g surfaceZ=%.9g\n",mode,int(blocked),int(target!=nullptr),examined,target?target->GetDBObjectID():-1,currentPass,desiredPass,surfacePass,standingPass,expected,expectedTile,center.x,center.y,center.z,surface.x,surface.y,surface.z);std::fflush(stdout);Require(target && expected>=-1 && expectedTile>=-1,"object-facing preview lacks actual prone placement, native facing classification and pickable nonunit hull");
    CObj<PreviewCommandMission> ui=new PreviewCommandMission;ui->Attach(world,game);ui->target=target;ui->point=surface;CObj<NGame::CUnitTracker> tracker=new NGame::CUnitTracker(ui,actor);ui->selection.push_back(tracker.GetPtr());CObj<AttackState> state=new AttackState;state->Attach(ui);NGame::SActionInfo info;info.SetValid();info.nMinAP=info.nMaxAP=7;SetAttackPreview(state,info);const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();const int hp=mission->GetTotalVP(),ap=actor->GetAP(),targetHP=target->GetHP(),ammo=gun->GetAmmoQuantity();auto* active=inventory->GetActive();GlobalGameRandom().SeedForHarness(732);auto reference=GlobalGameRandom();const int tile=world->GetGame()->GetTileCompositeToHit(actor,surface,NAI::THL_MIDDLE,false),actual=world->GetGame()->GetCompositeToHit(actor,target,NAI::HL_ANY,false);state->RefreshCaption();const auto caption=state->GetCursorInfo().wsText;auto expectedCaption=NUI::GetDBString(20276)+NUI::GetDBString(19808)+L"7";if(expected!=-1){WCHAR percent[32];swprintf(percent,sizeof(percent)/sizeof(percent[0]),L"%d%%",expected);expectedCaption+=NUI::GetDBString(expected>=2?20276:19807)+NUI::GetDBString(19809)+percent;}
    auto after=GlobalGameRandom();bool rng=true;unsigned tail[4];for(int i=0;i<4;++i){tail[i]=after.Get();rng&=tail[i]==reference.Get();}const bool pure=actor->GetPosition()==position && actor->GetAP()==ap && mission->GetTotalVP()==hp && target->GetHP()==targetHP && gun->GetAmmoQuantity()==ammo && inventory->GetActive()==active && active->GetQuantity()==1 && world->GetTime()->GetValue()==time && !actor->HasCommand();
    std::printf("OBJECT-FACING mode=%d blocked=%d actual=%d expected=%d tile=%d expectedTile=%d captionMatches=%d HP=%d targetHP=%d AP=%d ammo=%d pure=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,int(blocked),actual,expected,tile,expectedTile,int(caption==expectedCaption),hp,targetHP,ap,ammo,int(pure),int(rng),tail[0],tail[1],tail[2],tail[3]);std::fflush(stdout);Require(pure && rng && tile==expectedTile,"object-facing preview changed native physical/RNG or existing tile-direction control");Require(actual==expected && caption==expectedCaption,"generic object preview lost the existing impossible-facing sentinel and AP-only caption");
}

static void PreviewDispatch(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<6,"unknown native preview dispatcher mode");const bool rocket=mode==3 || mode==4,inert=mode==5;
    NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false,true);auto& world=fixture.world;auto* actor=fixture.actor;auto* mission=actor->GetUnitRPG();auto* inventory=mission->GetInventory();CObj<NRPG::CWeaponItem> launcher;
    if(!rocket && !inert){
        NDb::CRPGGrenade* record=nullptr;CDBIterator<NDb::CRPGGrenade> rows(*NDatabase::GetTable<NDb::CRPGGrenade>());while(rows.MoveNext()){auto* p=rows.Get();if(p->pItem && p->pItem->pModel && p->pItem->nWeight>0 && (!record || p->GetRecordID()<record->GetRecordID()))record=p;}Require(record,"preview dispatch lacks lowest authored ordinary grenade");
        CVec3 throwPoint;{CStructureNetworkContext refs;auto fields=BorrowedObjectFields(fixture.executor,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&throwPoint);}
        CObj<NRPG::IInventoryItem> previous=inventory->TakeOff(NDb::SLOT_1);fixture.regular=record;fixture.grenade=new NRPG::CGrenadeItem(record);fixture.grenade->nQuantity=1;Require(inventory->Equip(NDb::SLOT_1,fixture.grenade) && inventory->Activate(NDb::SLOT_1),"preview dispatch cannot equip deterministic authored grenade");fixture.executor=new NWorld::CExecThrowGrenade(actor,throwPoint);Require(fixture.executor->CanDoIt(actor->GetPosition())==NWorld::UCR_OK && actor->CanSpendAP(fixture.executor->GetStartAP()),"preview dispatch deterministic grenade lacks actual valid native throw");
    }
    if(rocket){
        NDb::CRPGWeapon* record=nullptr;CDBIterator<NDb::CRPGWeapon> rows(*NDatabase::GetTable<NDb::CRPGWeapon>());
        while(rows.MoveNext()){auto* p=rows.Get();if(p->bBazookaLogic && p->pItem && p->pItem->pModel && p->pInnerClip && p->pWeaponType && (!record || p->GetRecordID()<record->GetRecordID()))record=p;}
        Require(record,"preview dispatch lacks actual authored launcher");launcher=new NRPG::CWeaponItem(record);launcher->nQuantity=1;CObj<NRPG::IInventoryItem> previous=inventory->TakeOff(NDb::SLOT_1);Require(launcher->GetAmmoQuantity()>0 && inventory->Equip(NDb::SLOT_1,launcher) && inventory->Activate(NDb::SLOT_1),"preview dispatch cannot equip loaded native launcher");
    }
    CObj<NRPG::CFirstAidItem> kit;
    if(inert){NDb::CRPGFirstAid* record=nullptr;CDBIterator<NDb::CRPGFirstAid> rows(*NDatabase::GetTable<NDb::CRPGFirstAid>());while(rows.MoveNext()){auto* p=rows.Get();if(p->pItem && p->pItem->pModel && (!record || p->GetRecordID()<record->GetRecordID()))record=p;}Require(record,"preview dispatch lacks actual native nonweapon kit");kit=new NRPG::CFirstAidItem(record);kit->nQuantity=1;CObj<NRPG::IInventoryItem> previous=inventory->TakeOff(NDb::SLOT_1);Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"preview dispatch cannot equip actual nonweapon kit");}
    Require(inert?dynamic_cast<NRPG::CFirstAidItem*>(inventory->GetActive())!=nullptr:int(NRPG::GetToHitType(actor))==(rocket?3:4),"preview dispatch has wrong actual action type");
    actor->SetWalkWithoutWeapon(false);actor->SetUndrawItem(false);actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(mission->GetWeaponType());actor->animator.AlignTime();actor->animator.PlaceUnit(actor->GetPosition());mission->StartNewTurn(actor->GetPosition().GetCP());MarkNewDGFrame();world->GetAIMap()->Sync();world->GetGame()->SetNight(false);
    auto* network=dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());Require(network,"preview dispatch lacks native path network");const auto position=actor->GetPosition();CVec3 point=VNULL3;CObj<NWorld::CUnitServer> target;CObj<NRPG::CCoverInfo> nativeCover;int expected=-1,examined=0,normalCover=-2,coefficient=-1,rayCount=0,penetrating=0;float fractionalCover=-1;
    if(inert){expected=5;}else if(mode==1){
        vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(position.GetCP(),5.f),&places);
        for(auto p:places){if(!p.IsIntegral() || p.IsFinal())continue;p.SetPose(NAI::CM_STAND);p.SetMoving(0);const auto at=NAI::SPosition(p,network).GetCP();if(fabs(at.z-position.GetCP().z)>.1f || fabs2(at-position.GetCP())<1.f || !network->IsPassable(p) || network->IsLocked(p))continue;
            ++examined;target=world->AddUnitInGame(p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer(),"NativePreviewGrenadeTarget");Require(target && target->CanFight(),"preview dispatch cannot create native unit target");target->GetUnitRPG()->StartNewTurn(target->GetPosition().GetCP());target->animator.AlignTime();target->animator.PlaceUnit(target->GetPosition());MarkNewDGFrame();world->GetAIMap()->Sync();world->GetGame()->SetNight(false);Require(world->GetAIMap()->GetUnitHLPos(&point,world->GetAIMap()->GetHull(target),NAI::HL_ANY),"preview dispatch lacks actual unit hull center");expected=world->GetGame()->GetGrenadeCompositeToHit(actor,point,mission->IsFirstTurn(),nullptr);break;
        }
        Require(target && expected>0,"preview dispatch lacks positive native unit-center grenade reference");
    }else if(!rocket){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(fixture.executor,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&point);expected=world->GetGame()->GetGrenadeCompositeToHit(actor,point,mission->IsFirstTurn(),nullptr);Require(expected>0,"preview dispatch lacks positive native tile grenade reference");}
    else{
        vector<NRPG::CAttackPortion> attacks;mission->CreateAttack(&attacks,false);Require(!attacks.empty() && attacks.front().nK>0,"preview dispatch launcher lacks actual native attack portion");coefficient=attacks.front().nK;auto nativePortion=attacks.front();nativePortion.nK=1;
        vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(position.GetCP(),20.f),&places);CObj<NRPG::CCoverInfo> selected;
        for(auto p:places){if(!p.IsIntegral() || p.IsFinal())continue;const auto ground=NAI::SPosition(p,network).GetCP();if(fabs2(ground-position.GetCP())<4.f)continue;
            for(float height:{0.f,.625f,1.25f}){++examined;const auto at=ground+CVec3(0,0,height);auto directed=position;directed.pos.p.SetDirection(NRPG::GetShootDirection(network,position.pos.p,at));const auto pass=network->GetPassability(directed.pos.p);if(pass==NAI::AIP_NOT_PASSABLE || pass==NAI::AIP_CANNOT_LAY)continue;
                CObj<NRPG::CCoverInfo> cover=world->GetGame()->CalcCoversForTile(actor->GetAttackOrigin(directed),nativePortion,actor,at,actor->GetMinClearDistance());if(!cover)continue;const auto snapshot=CoverSnapshot(cover);if(snapshot.hits.empty())continue;int count=0;for(const auto& ray:snapshot.hits)if(ray.penetrate)++count;if(!count || snapshot.piercing<=0)continue;
                const float value=(100.f*count/snapshot.hits.size())*((snapshot.piercing/count)/coefficient);const int distance=int(fabs(snapshot.source-at)*FP_INV_GRID_STEP);CObj<NRPG::CRLauncherToHitCalcer> calcer=new NRPG::CRLauncherToHitCalcer(actor,position.GetPose(),distance,snapshot.source,value,actor->GetCarefulShotExtraAP(),mission->IsFirstTurn(),false,CVec3(1,1,1),at);const int chance=calcer->GetToHit();if(chance<=0 || chance>=100)continue;
                point=at;expected=chance;selected=cover;nativeCover=cover;fractionalCover=value;normalCover=NRPG::GetHitCover(actor,cover);rayCount=int(snapshot.hits.size());penetrating=count;break;
            }if(selected)break;
        }Require(selected && expected>0,"preview dispatch lacks native positive launcher cover/calcer reference");
    }
    const auto origin=position.GetCP();std::printf("PREVIEW-DISPATCH-INPUT mode=%d actorX=%.9g actorY=%.9g actorZ=%.9g targetX=%.9g targetY=%.9g targetZ=%.9g distance=%d activeID=%d skill=%d strength=%d night=%d weather=%d\n",mode,origin.x,origin.y,origin.z,point.x,point.y,point.z,int(fabs(origin-point)*FP_INV_GRID_STEP),inventory->GetActive()->GetDBItem()->GetRecordID(),int(mission->GetWeaponSkill(inventory->GetActive(),0,0)),int(mission->GetRPGUnit()->Skills(NDb::ST_STR)),int(world->GetGame()->IsNight()),int(world->GetWeather()));std::fflush(stdout);
    const int hp=mission->GetTotalVP(),ap=actor->GetAP(),ammo=launcher?launcher->GetAmmoQuantity():-1,quantity=inventory->GetActive()->GetQuantity(),targetHP=target?target->GetUnitRPG()->GetTotalVP():-1;const auto targetPosition=target?target->GetPosition():position;const auto time=world->GetTime()->GetValue();auto* active=inventory->GetActive();GlobalGameRandom().SeedForHarness(729);auto reference=GlobalGameRandom();
    const int actual=inert?int(NRPG::GetToHitType(actor)):mode==4?NRPG::GetAttackerTileToHit(actor,point,actor->GetCarefulShotExtraAP(),NAI::THL_MIDDLE,nativeCover,mission->IsFirstTurn(),0):mode==1?world->GetGame()->GetCompositeToHit(actor,target,NAI::HL_ANY,false):mode==2?world->GetGame()->GetGrenadeCompositeToHit(actor,point,mission->IsFirstTurn(),nullptr):world->GetGame()->GetTileCompositeToHit(actor,point,NAI::THL_MIDDLE,false);
    auto after=GlobalGameRandom();unsigned tail[4];bool rng=true;for(int i=0;i<4;++i){tail[i]=after.Get();rng&=tail[i]==reference.Get();}const bool pure=actor->GetPosition()==position && mission->GetTotalVP()==hp && actor->GetAP()==ap && (launcher?launcher->GetAmmoQuantity():-1)==ammo && inventory->GetActive()==active && active->GetQuantity()==quantity && world->GetTime()->GetValue()==time && !actor->HasCommand() && !world->GetGame()->IsNight() && (!target || (target->GetPosition()==targetPosition && target->GetUnitRPG()->GetTotalVP()==targetHP));
    std::printf("PREVIEW-DISPATCH mode=%d type=%d examined=%d unitFirst=%d actual=%d expected=%d nK=%d integerCover=%d fractionalCover=%.9g rays=%d penetrating=%d HP=%d AP=%d ammo=%d quantity=%d pure=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,int(NRPG::GetToHitType(actor)),examined,int(mission->IsFirstTurn()),actual,expected,coefficient,normalCover,fractionalCover,rayCount,penetrating,hp,ap,ammo,quantity,int(pure),int(rng),tail[0],tail[1],tail[2],tail[3]);std::fflush(stdout);
    Require(pure && rng,"preview dispatch query changed native physical state/time/RNG");Require(actual==expected,"preview dispatcher differs from actual native grenade/launcher reference");
}


static void PreviewFirstTurnLua(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<17,"unknown native preview first-turn/Lua case");
    const bool lua=mode>=8,tile=mode>=4 && mode<8,late=!lua && (mode&2),argument=(mode&1)!=0;
    CObj<NRPG::CGlobalGame> global;auto world=FixtureWorld(databasePath,resourcePath,true,&global);
    auto* network=world->GetPathNetwork();list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsEmptyPK() && !unit->IsWearingPK()){actor=unit;break;}
    Require(actor,"preview first-turn/Lua lacks native party actor");
    auto* deployedActor=actor;CObj<NWorld::CUnitServer> freshActor;vector<NAI::SPathPlace> freshPlaces;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),3.f),&freshPlaces);
    for(auto p:freshPlaces){if(!p.IsIntegral() || p.IsFinal())continue;p.SetPose(NAI::CM_STAND);p.SetMoving(0);if(!network->IsPassable(p) || network->IsLocked(p))continue;freshActor=world->AddUnitInGame(p,NRPG::CreateUnit(NDb::GetPers(54)),deployedActor->GetTBSPlayer(),"NativeFreshPreviewActor");if(freshActor)break;}
    Require(freshActor && freshActor->CanFight(),"preview first-turn/Lua cannot add a genuine fresh native actor");actor=freshActor.GetPtr();
    std::printf("PREVIEW-OWNERSHIP-COUNTER mode=%d deployedFirst=%d freshFirst=%d worldFirst=%d\n",mode,int(deployedActor->GetUnitRPG()->IsFirstTurn()),int(actor->GetUnitRPG()->IsFirstTurn()),int(world->IsFirstTurn()));std::fflush(stdout);
    auto* inventory=actor->GetUnitRPG()->GetInventory();
    CObj<NRPG::IInventoryItem> previous=inventory->TakeOff(NDb::SLOT_1);CObj<NRPG::CWeaponItem> gun;CObj<NRPG::CMeleeWeaponItem> blade;
    if(lua){
        NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> records(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
        while(records.MoveNext()){auto* p=records.Get();if(!p->bThrowing && p->pItem && p->pItem->pModel && (!record || p->GetRecordID()<record->GetRecordID()))record=p;}
        Require(record,"preview Lua lacks actual authored nonthrowing blade");blade=new NRPG::CMeleeWeaponItem(record);blade->nQuantity=1;Require(inventory->Equip(NDb::SLOT_1,blade) && inventory->Activate(NDb::SLOT_1),"preview Lua cannot equip native blade");
    }else gun=NativeEquipAccuracyType(actor->GetUnitRPG()->GetRPGUnit(),NDb::WT_PISTOL,NDb::SM_Snap);
    Require((lua?NRPG::TH_MELEE:NRPG::TH_SHOOT)==NRPG::GetToHitType(actor),"preview first-turn/Lua has wrong actual action type");
    actor->SetWalkWithoutWeapon(false);actor->SetUndrawItem(false);actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());actor->animator.AlignTime();actor->animator.PlaceUnit(actor->GetPosition());
    auto* mission=dynamic_cast<NRPG::IUnitMission*>(actor->GetRPG());Require(mission && actor->GetScriptToHit()<0,"preview first-turn/Lua lacks ordinary native mission");
    mission->StartNewTurn(actor->GetPosition().GetCP());
    if(late)mission->StartNewTurn(actor->GetPosition().GetCP());
    Require(mission->IsFirstTurn()==!late && world->IsFirstTurn(),"preview first-turn fixture did not independently separate unit counter from world turn");
    MarkNewDGFrame();world->GetAIMap()->Sync();world->GetGame()->SetNight(false);
    const auto position=actor->GetPosition();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(position.GetCP(),lua?1.f:12.f),&places);
    CObj<NWorld::CUnitServer> target;CVec3 point=VNULL3;CObj<NRPG::CCoverInfo> selected;
    int expected=-1,otherFirst=-1,examined=0;const int args[]={-1,0,1,2,3,4,5,-2,6};const int luaArgument=lua?args[mode-8]:0;
    const auto location=lua && luaArgument>=NAI::HL_ANY && luaArgument<NAI::N_HL?NAI::EHitLocation(luaArgument):lua?NAI::HL_HEAD:NAI::HL_ANY;
    vector<NRPG::CAttackPortion> attacks;mission->CreateAttack(&attacks,false);Require(!attacks.empty(),"preview first-turn/Lua lacks actual native attack portion");
    if(!tile){
        for(auto p:places){
            if(!p.IsIntegral() || p.IsFinal())continue;p.SetPose(NAI::CM_STAND);p.SetMoving(0);const auto at=NAI::SPosition(p,network).GetCP();const float distance=fabs2(at-position.GetCP());
            if(fabs(at.z-position.GetCP().z)>.1f || (lua?distance<.1f || distance>sqr(F_MELEE_DISTANCE):distance<9.f || distance>100.f) || !network->IsPassable(p) || network->IsLocked(p))continue;
            ++examined;
            if(!target)target=world->AddUnitInGame(p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer(),"NativePreviewOwnershipTarget");
            else{auto targetPosition=target->GetPosition();targetPosition.pos=NAI::SPosition(p,network);target->SetPosition(targetPosition);}
            Require(target && target->CanFight(),"preview first-turn/Lua cannot deploy native target");target->GetUnitRPG()->StartNewTurn(target->GetPosition().GetCP());Require(fabs2(target->GetPosition().GetCP()-target->GetUnitRPG()->GetTurnStartCP())==0,"preview target did not establish a native stationary turn start");target->animator.AlignTime();target->animator.PlaceUnit(target->GetPosition());MarkNewDGFrame();world->GetAIMap()->Sync();world->GetGame()->SetNight(false);
            if(lua){CVec3 center;Require(world->GetAIMap()->GetUnitHLPos(&center,world->GetAIMap()->GetHull(target),NAI::HL_ANY) && NWorld::CanMeleeAttack(actor,position,center),"preview Lua native target is not actually reachable");expected=world->GetGame()->GetCompositeToHit(actor,target,location,mission->IsFirstTurn());otherFirst=world->GetGame()->GetCompositeToHit(actor,target,NAI::HL_HEAD,mission->IsFirstTurn());break;}
            auto directed=position;directed.pos.p.SetDirection(NRPG::GetShootDirection(network,position.pos.p,target->GetPosition().GetCP()));
            auto pass=network->GetPassability(directed.pos.p);if(pass==NAI::AIP_NOT_PASSABLE || pass==NAI::AIP_CANNOT_LAY)continue;
            CObj<NRPG::CCoverInfo> cover=world->GetGame()->CalcCovers(actor->GetAttackOrigin(directed),attacks.front(),actor,target,NAI::HL_ANY,actor->GetMinClearDistance());Require(cover,"preview first-turn lacks native object cover");vector<int> accessible;
            expected=NRPG::GetAttackerToHit(actor,target,actor->GetCarefulShotExtraAP(),NAI::HL_ANY,accessible,cover,mission->IsFirstTurn(),0);otherFirst=NRPG::GetAttackerToHit(actor,target,actor->GetCarefulShotExtraAP(),NAI::HL_ANY,accessible,cover,!mission->IsFirstTurn(),0);
            if(expected>0 && expected<100 && otherFirst>0 && otherFirst<100 && expected!=otherFirst){selected=cover;break;}
        }
        Require(target && (lua || selected),"preview first-turn/Lua lacks independent reachable native target and non-clamped turn witness");
    }else{
        for(auto p:places){
            if(!p.IsIntegral() || p.IsFinal())continue;const auto ground=NAI::SPosition(p,network).GetCP();if(fabs2(ground-position.GetCP())<9.f)continue;
            for(float height:{0.f,.625f,1.25f}){
                ++examined;const auto at=ground+CVec3(0,0,height);auto directed=position;directed.pos.p.SetDirection(NRPG::GetShootDirection(network,position.pos.p,at));const auto pass=network->GetPassability(directed.pos.p);if(pass==NAI::AIP_NOT_PASSABLE || pass==NAI::AIP_CANNOT_LAY)continue;
                CObj<NRPG::CCoverInfo> cover=world->GetGame()->CalcCoversForTile(actor->GetAttackOrigin(directed),attacks.front(),actor,at,actor->GetMinClearDistance());Require(cover,"preview first-turn lacks native tile cover");
                expected=NRPG::GetAttackerTileToHit(actor,at,actor->GetCarefulShotExtraAP(),NAI::THL_LOWER,cover,mission->IsFirstTurn(),0);otherFirst=NRPG::GetAttackerTileToHit(actor,at,actor->GetCarefulShotExtraAP(),NAI::THL_LOWER,cover,!mission->IsFirstTurn(),0);
                if(expected>0 && expected<100 && otherFirst>0 && otherFirst<100 && expected!=otherFirst){point=at;selected=cover;break;}
            }
            if(selected)break;
        }
        Require(selected,"preview first-turn lacks independent non-clamped tile turn witness");
    }
    std::printf("PREVIEW-OWNERSHIP-ENV mode=%d skill=%d night=%d weather=%d targetTurnAfter=%.9g\n",mode,int(mission->GetWeaponSkill(inventory->GetActive(),0,0)),int(world->GetGame()->IsNight()),int(world->GetWeather()),target?fabs(target->GetPosition().GetCP()-target->GetUnitRPG()->GetTurnStartCP()):-1.f);std::fflush(stdout);
    CObj<NScript::CScript> script;
    if(lua){script=NScript::CreateScript(world);NScript::luaPushCPtr(*script,actor);script->SetGlobal("previewOwnershipActor");NScript::luaPushCPtr(*script,target.GetPtr());script->SetGlobal("previewOwnershipTarget");}
    const int hp=mission->GetTotalVP(),ap=actor->GetAP(),ammo=gun?gun->GetAmmoQuantity():-1,quantity=inventory->GetActive()->GetQuantity(),targetHP=target?target->GetUnitRPG()->GetTotalVP():-1;const auto targetPosition=target?target->GetPosition():position;const auto time=world->GetTime()->GetValue();auto* active=inventory->GetActive();
    GlobalGameRandom().SeedForHarness(728);auto reference=GlobalGameRandom();int actual=-2;
    if(lua){char code[256];if(mode==8)std::snprintf(code,sizeof(code),"PreviewOwnershipChance=UnitGetToHitUnit(previewOwnershipActor,previewOwnershipTarget); PreviewOwnershipDone=17");else std::snprintf(code,sizeof(code),"PreviewOwnershipChance=UnitGetToHitUnit(previewOwnershipActor,previewOwnershipTarget,%d); PreviewOwnershipDone=17",luaArgument);RunScript(script,code);const auto value=script->GetGlobal("PreviewOwnershipChance"),done=script->GetGlobal("PreviewOwnershipDone");Require(value.IsNumber() && done.IsNumber() && done.GetNumber()==17,"actual Lua preview did not return its numeric result");actual=int(value.GetNumber());}
    else actual=tile?world->GetGame()->GetTileCompositeToHit(actor,point,NAI::THL_LOWER,argument):world->GetGame()->GetCompositeToHit(actor,target,NAI::HL_ANY,argument);
    auto after=GlobalGameRandom();unsigned tail[4];bool rng=true;for(int i=0;i<4;++i){tail[i]=after.Get();rng&=tail[i]==reference.Get();}
    const bool pure=position==actor->GetPosition() && hp==mission->GetTotalVP() && ap==actor->GetAP() && ammo==(gun?gun->GetAmmoQuantity():-1) && quantity==inventory->GetActive()->GetQuantity() && inventory->GetActive()==active && time==world->GetTime()->GetValue() && !actor->HasCommand() && !world->GetGame()->IsNight() && (!target || (targetHP==target->GetUnitRPG()->GetTotalVP() && targetPosition==target->GetPosition())) && mission->IsFirstTurn()==!late && world->IsFirstTurn();
    std::printf("PREVIEW-OWNERSHIP mode=%d tile=%d lua=%d late=%d argument=%d luaArgument=%d location=%d unitFirst=%d worldFirst=%d examined=%d actual=%d expected=%d other=%d HP=%d AP=%d ammo=%d quantity=%d pure=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,int(tile),int(lua),int(late),int(argument),luaArgument,int(location),int(mission->IsFirstTurn()),int(world->IsFirstTurn()),examined,actual,expected,otherFirst,hp,ap,ammo,quantity,int(pure),int(rng),tail[0],tail[1],tail[2],tail[3]);std::fflush(stdout);
    Require(pure && rng,"preview first-turn/Lua query changed physical controls, unit/world turn state or RNG");
    Require(actual==expected,lua?"Lua preview ignored the original hit-location argument/default":"preview used its logging argument instead of the native unit first-turn state");
}


static void TileShotPreview(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<7,"unknown native tile shot preview case");
    const bool blockedPose=mode==0,burst=mode==3 || mode==6;
    const auto pose=blockedPose?NAI::CM_LAY:mode==2?NAI::CM_CROUCH:NAI::CM_STAND;
    CObj<NRPG::CGlobalGame> global;
    auto world=FixtureWorld(databasePath,resourcePath,true,&global);
    auto* network=dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());
    Require(network,"tile shot preview lacks the actual native path network");
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsWearingPK() && !unit->IsEmptyPK()){actor=unit;break;}
    Require(actor,"tile shot preview lacks a native party actor");
    const auto shot=mode==6?NDb::SM_LongBurst:burst?NDb::SM_ShortBurst:NDb::SM_Snap;
    CObj<NRPG::CWeaponItem> gun;
    CDBIterator<NDb::CRPGWeapon> records(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while(records.MoveNext()){
        auto* p=records.Get();
        if(!p->shootModes[shot] || p->bBazookaLogic || !p->pItem || !p->pItem->pModel || !p->pInnerClip || !p->pWeaponType || (burst && p->nRoF<12))continue;
        CObj<NRPG::CWeaponItem> candidate=new NRPG::CWeaponItem(p);
        if(candidate->GetAmmoQuantity()>0 && (!gun || p->GetRecordID()<gun->GetDBWeapon()->GetRecordID()))gun=candidate;
    }
    Require(gun && gun->SetShootMode(shot),"tile shot preview lacks an actual loaded authored shoot mode");
    auto* inventory=actor->GetUnitRPG()->GetInventory();
    CObj<NRPG::IInventoryItem> previous=inventory->TakeOff(NDb::SLOT_1);
    Require(inventory->Equip(NDb::SLOT_1,gun) && inventory->Activate(NDb::SLOT_1) && NRPG::GetToHitType(actor)==NRPG::TH_SHOOT,"tile shot preview cannot equip native firearm");
    actor->SetWalkWithoutWeapon(false);actor->SetUndrawItem(false);
    actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
    const auto initial=actor->GetPosition();
    vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(initial.GetCP(),blockedPose?8.f:1.f),&places);
    NAI::SUnitPosition from=initial;CVec3 target=VNULL3;
    bool foundPose=false;int currentPass=-1,desiredPass=-1;
    for(auto base:places){
        if(!base.IsIntegral() || base.IsFinal() || base.GetLayer()!=initial.pos.p.GetLayer())continue;
        for(int d=0;d<8;++d){
            auto source=base;source.SetPose(pose);source.SetDirection(d);source.SetMoving(0);
            const auto at=NAI::SPosition(source,network).GetCP();
            target=at+CVec3(float(cos(network->GetDirection(source)+1.57079632679f)),float(sin(network->GetDirection(source)+1.57079632679f)),0)*3.125f;
            auto desired=source;desired.SetDirection(NRPG::GetShootDirection(network,source,target));
            network->Unlock(actor);currentPass=network->GetPassability(source);desiredPass=network->GetPassability(desired);network->Lock(actor,actor->GetPosition().pos.p);
            if(currentPass!=NAI::AIP_YES || (blockedPose?desiredPass!=NAI::AIP_CANNOT_LAY:desiredPass!=NAI::AIP_YES))continue;
            from=initial;from.pos=NAI::SPosition(source,network);from.SetPose(blockedPose?NAI::CRAWL:pose==NAI::CM_CROUCH?NAI::CROUCH:NAI::WALK);
            foundPose=true;break;
        }
        if(foundPose)break;
    }
    Require(foundPose,"tile shot preview lacks independently passable current pose and desired-facing witness");
    actor->SetPosition(from);actor->animator.AlignTime();actor->animator.PlaceUnit(from);MarkNewDGFrame();world->GetAIMap()->Sync();world->GetGame()->SetNight(false);
    auto* mission=dynamic_cast<NRPG::IUnitMission*>(actor->GetRPG());Require(mission && actor->GetScriptToHit()<0,"tile shot preview lacks unscripted native mission");
    vector<NRPG::CAttackPortion> attacks;mission->CreateAttack(&attacks,false);Require(!attacks.empty(),"tile shot preview lacks actual native attack");
    const bool first=mission->IsFirstTurn();const int bullets=Max(1,mission->GetBulletsQuantityInShot());Require(burst?bullets>1:bullets==1,"tile shot preview has no native multi-bullet or single-shot witness");
    int expected=-1,legacy=-2,firstBullet=-1,cover=-2,examined=0;vector<int> bulletChances;
    CVec3 oldOrigin=actor->GetAttackOrigin(from),newOrigin=VNULL3;
    if(!blockedPose){
        vector<NAI::SPathPlace> targets;network->GetNearPlaces(SSphere(from.GetCP(),20.f),&targets);
        bool selected=false;
        for(auto p:targets){
            if(!p.IsIntegral() || p.IsFinal())continue;
            const auto ground=NAI::SPosition(p,network).GetCP();if(fabs2(ground-from.GetCP())<4.f)continue;
            for(float height:{0.f,0.625f,1.25f,2.5f}){
                ++examined;const auto at=ground+CVec3(0,0,height);
                auto directed=from;directed.pos.p.SetDirection(NRPG::GetShootDirection(network,from.pos.p,at));
                network->Unlock(actor);const int pass=network->GetPassability(directed.pos.p);network->Lock(actor,from.pos.p);if(pass!=NAI::AIP_YES)continue;
                const auto origin=actor->GetAttackOrigin(directed);
                CObj<NRPG::CCoverInfo> rays=world->GetGame()->CalcCoversForTile(origin,attacks.front(),actor,at,actor->GetMinClearDistance());
                CObj<NRPG::CCoverInfo> oldRays=world->GetGame()->CalcCoversForTile(oldOrigin,attacks.front(),actor,at,actor->GetMinClearDistance());
                Require(rays && oldRays,"tile shot preview lacks actual native cover results");
                const auto snapshot=CoverSnapshot(rays);Require(snapshot.source==origin,"tile shot preview lost actual solver origin");
                const int hit=NRPG::GetHitCover(actor,rays),oldHit=NRPG::GetHitCover(actor,oldRays);
                vector<int> chances;int sum=0;
                for(int i=0;i<bullets;++i){const int chance=NRPG::GetAttackerTileToHit(actor,at,actor->GetCarefulShotExtraAP(),NAI::THL_LOWER,rays,first,i);chances.push_back(chance);sum+=chance;}
                const int average=sum/bullets;
                const int before=NRPG::GetTileToHit(actor,actor->GetPose(),int(fabs(from.GetCP()-at)/FP_GRID_STEP),from.GetCP(),at,NAI::THL_LOWER,actor->GetCarefulShotExtraAP(),oldHit,first);
                if(mode==5?(hit>0 || average!=0 || before!=0):mode==4?(hit<=0 || average!=before):(hit<=0 || average==before || (burst && average==chances.front())))continue;
                target=at;newOrigin=origin;desiredPass=pass;expected=average;legacy=before;firstBullet=chances.front();cover=hit;bulletChances=chances;selected=true;break;
            }
            if(selected)break;
        }
        Require(selected,"tile shot preview lacks independently distinguishing cover/origin/burst inputs or exact control");
    }
    std::printf("TILE-SHOT-INPUT mode=%d pose=%d currentPass=%d desiredPass=%d weapon=%d shot=%d bullets=%d examined=%d cover=%d expected=%d old=%d firstBullet=%d originChanged=%d\n",mode,int(pose),currentPass,desiredPass,gun->GetDBWeapon()->GetRecordID(),int(shot),bullets,examined,cover,expected,legacy,firstBullet,int(newOrigin!=oldOrigin));
    for(size_t i=0;i<bulletChances.size();++i)std::printf("TILE-SHOT-BULLET mode=%d index=%u chance=%d\n",mode,unsigned(i),bulletChances[i]);
    const auto position=actor->GetPosition();const int hp=mission->GetTotalVP(),ap=actor->GetAP(),ammo=gun->GetAmmoQuantity();const auto time=world->GetTime()->GetValue();
    GlobalGameRandom().SeedForHarness(727);auto reference=GlobalGameRandom();
    const int actual=world->GetGame()->GetTileCompositeToHit(actor,target,NAI::THL_LOWER,first);
    auto after=GlobalGameRandom();unsigned tail[4];bool rng=true;for(int i=0;i<4;++i){tail[i]=after.Get();rng&=tail[i]==reference.Get();}
    const bool pure=position==actor->GetPosition() && hp==mission->GetTotalVP() && ap==actor->GetAP() && ammo==gun->GetAmmoQuantity() && inventory->GetActive()==gun.GetPtr() && time==world->GetTime()->GetValue() && !actor->HasCommand() && !world->GetGame()->IsNight();
    std::printf("TILE-SHOT mode=%d actual=%d expected=%d old=%d HP=%d AP=%d ammo=%d pure=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,actual,expected,legacy,hp,ap,ammo,int(pure),int(rng),tail[0],tail[1],tail[2],tail[3]);std::fflush(stdout);
    Require(pure && rng,"tile shot preview changed native actor/item/time/night/command/RNG controls");
    Require(actual==expected,"tile shot preview lost desired-pose clearance, actual-origin helper or per-bullet average");
}


static void TileCoverOrigin(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<5,"unknown tile cover origin case");CObj<NRPG::CGlobalGame> global;auto world=FixtureWorld(databasePath,resourcePath,true,&global);auto* network=world->GetPathNetwork();list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsWearingPK() && !unit->IsEmptyPK()){actor=unit;break;}Require(actor,"tile cover origin lacks real native actor");
    CObj<NRPG::CWeaponItem> gun;CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());while(weapons.MoveNext()){auto* p=weapons.Get();if(!p->shootModes[NDb::SM_Snap] || p->bBazookaLogic || !p->pItem || !p->pItem->pModel || !p->pInnerClip || !p->pWeaponType)continue;CObj<NRPG::CWeaponItem> candidate=new NRPG::CWeaponItem(p);if(candidate->GetWeaponType()==NDb::WT_PISTOL && candidate->GetAmmoQuantity()>0 && (!gun || p->GetRecordID()<gun->GetDBWeapon()->GetRecordID()))gun=candidate;}Require(gun && gun->SetShootMode(NDb::SM_Snap),"tile cover origin lacks actual loaded Snap pistol");auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous=inventory->TakeOff(NDb::SLOT_1);Require(inventory->Equip(NDb::SLOT_1,gun) && inventory->Activate(NDb::SLOT_1) && NWorld::GetActionType(actor)==NWorld::AT_SHOOT,"tile cover origin cannot equip authored pistol");actor->SetWalkWithoutWeapon(false);actor->SetUndrawItem(false);actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
    const auto pose=mode==1?NAI::CM_CROUCH:NAI::CM_STAND;auto position=actor->GetPosition();auto source=position.pos.p;source.SetPose(pose);network->Unlock(actor);const bool sourceFree=network->GetPassability(source)==NAI::AIP_YES;network->Lock(actor,position.pos.p);Require(sourceFree,"tile cover origin native pose is not physically available");position.pos.p=source;position.SetPose(pose==NAI::CM_CROUCH?NAI::CROUCH:NAI::WALK);actor->SetPosition(position);actor->animator.AlignTime();actor->animator.PlaceUnit(position);MarkNewDGFrame();world->GetAIMap()->Sync();
    world->GetGame()->SetNight(false);
    auto* mission=dynamic_cast<NRPG::IUnitMission*>(actor->GetRPG());Require(mission && NRPG::GetToHitType(actor)==NRPG::TH_SHOOT && actor->GetScriptToHit()<0,"tile cover origin lacks ordinary unscripted firearm routing");vector<NRPG::CAttackPortion> attacks;mission->CreateAttack(&attacks,false);Require(!attacks.empty(),"tile cover origin lacks native attack portion");const CVec3 feet=position.GetCP(),center=position.GetCenter(),origin=mode==3?center:actor->GetAttackOrigin(position,mode==2);Require(mode==3 || fabs2(origin-center)>1e-6f,"tile cover origin muzzle collapsed to old center");
    vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(feet,20.f),&places);CObj<NRPG::CCoverInfo> selected;CVec3 target;int expected=-1,old=-1,cover=-1,distance=-1,oldDistance=-1,examined=0;const bool first=world->IsFirstTurn();
    for(auto point:places){if(!point.IsIntegral() || point.IsFinal())continue;const CVec3 ground=NAI::SPosition(point,network).GetCP();const float d=fabs2(ground-feet);if(d<4.f || d>400.f)continue;for(float h: {0.f,0.625f,1.25f,2.5f}){++examined;const CVec3 at=ground+CVec3(0,0,h);CObj<NRPG::CCoverInfo> actual=world->GetGame()->CalcCoversForTile(origin,attacks.front(),actor,at,actor->GetMinClearDistance());Require(actual,"tile cover origin solver returned no native cover");const auto snapshot=CoverSnapshot(actual);Require(snapshot.source==origin,"tile cover origin solver did not retain actual input source");const int hit=NRPG::GetHitCover(actor,actual),n=int(fabs(snapshot.source-at)*FP_INV_GRID_STEP),legacy=int(fabs(feet-at)/FP_GRID_STEP);int now=NRPG::GetTileToHit(actor,actor->GetPose(),n,snapshot.source,at,NAI::THL_LOWER,0,hit,first,CVec3(1,1,1),0);int before=NRPG::GetTileToHit(actor,actor->GetPose(),legacy,center,at,NAI::THL_LOWER,0,hit,first,CVec3(1,1,1),0);if(mode<3?(hit<=0 || now==before):(mode==3?(hit<=0 || now!=before || n!=legacy):(hit>0 || now!=0 || before!=0)))continue;selected=actual;target=at;expected=now;old=before;cover=hit;distance=n;oldDistance=legacy;break;}if(selected)break;}
    std::printf("TILE-COVER-INPUT mode=%d found=%d examined=%d cover=%d distance=%d oldDistance=%d expected=%d old=%d\n",mode,int(selected!=nullptr),examined,cover,distance,oldDistance,expected,old);std::fflush(stdout);Require(selected,"tile cover origin lacks independently distinguishing native geometry/formula inputs");
    std::printf("TILE-COVER-ENV mode=%d night=%d first=%d skill=%d weather=%d actor=%d weapon=%d pose=%d\n",mode,int(world->GetGame()->IsNight()),int(first),int(mission->GetWeaponSkill(gun.GetPtr(),0,0)),int(world->GetWeather()),actor->GetRPG()->GetRPGPers()->GetRecordID(),gun->GetDBWeapon()->GetRecordID(),int(actor->GetPose()));std::fflush(stdout);
    const int hp=actor->GetUnitRPG()->GetTotalVP(),ap=actor->GetAP(),ammo=gun->GetAmmoQuantity();const auto time=world->GetTime()->GetValue();const auto beforePosition=actor->GetPosition();GlobalGameRandom().SeedForHarness(724);auto reference=GlobalGameRandom();const int actual=NRPG::GetAttackerTileToHit(actor,target,0,NAI::THL_LOWER,selected,first,0);auto after=GlobalGameRandom();unsigned tail[4];bool rng=true;for(int i=0;i<4;++i){tail[i]=after.Get();rng&=tail[i]==reference.Get();}const bool pure=!world->GetGame()->IsNight() && hp==actor->GetUnitRPG()->GetTotalVP() && ap==actor->GetAP() && ammo==gun->GetAmmoQuantity() && beforePosition==actor->GetPosition() && time==world->GetTime()->GetValue() && inventory->GetActive()==gun.GetPtr();std::printf("TILE-COVER mode=%d actual=%d expected=%d old=%d HP=%d AP=%d ammo=%d pure=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,actual,expected,old,hp,ap,ammo,int(pure),int(rng),tail[0],tail[1],tail[2],tail[3]);std::fflush(stdout);Require(pure && rng,"tile chance query changed native physical controls or RNG");GlobalGameRandom().SeedForHarness(725);auto rollReference=GlobalGameRandom();const int roll=rollReference.Get(0,100);Require(roll>0,"tile cover execution reference has unusable zero roll");int integrated=-1;const float factor=NRPG::CheckTileToHit(actor,target,0,NAI::THL_LOWER,selected,first,&integrated,0);auto rollAfter=GlobalGameRandom();unsigned rollTail[4];bool oneDraw=true;for(int i=0;i<4;++i){rollTail[i]=rollAfter.Get();oneDraw&=rollTail[i]==rollReference.Get();}bool integration=!world->GetGame()->IsNight() && integrated==actual && factor==float(actual)/float(roll) && oneDraw && hp==actor->GetUnitRPG()->GetTotalVP() && ap==actor->GetAP() && ammo==gun->GetAmmoQuantity() && beforePosition==actor->GetPosition() && time==world->GetTime()->GetValue();std::printf("TILE-COVER-EXEC mode=%d integrated=%d roll=%d valid=%d oneDraw=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,integrated,roll,int(integration),int(oneDraw),rollTail[0],rollTail[1],rollTail[2],rollTail[3]);std::fflush(stdout);Require(integration,"tile cover execution helper lost query routing, physical controls or one-roll contract");Require(actual==expected,"tile chance query used actor feet/center instead of native cover source");
}


static void NativeLedgeRetreat(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game, 4517);
    auto* net = world->GetPathNetwork();
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto unit : units)
        if (!unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK() && unit->GetUnitRPG()->CanMove())
        {
            actor = unit;
            break;
        }
    Require(actor, "ledge fixture has no movable party member");
    vector<NAI::SPathPlace> places;
    net->GetNearPlaces(SSphere(actor->GetPosition().GetCP(), 40.f), &places);
    NAI::SPathPlace edge, back;
    bool found = false;
    for (auto point : places)
    {
        if (!point.IsIntegral() || point.IsFinal())
            continue;
        for (int dir = 0; dir < 8; dir += 2)
        {
            point.SetPose(NAI::CM_INACTIVE);
            point.SetDirection(dir);
            point.SetMoving(0);
            if (net->GetPassability(point) != NAI::AIP_YES)
                continue;
            const int dx = NAI::nMoveShift[dir][0], dy = NAI::nMoveShift[dir][1];
            const int x = point.GetX(), y = point.GetY();
            if (x < 1 || y < 1)
                continue;
            auto landing = NAI::SPathPlace(x + dx, y + dy, point.GetLayer(), dir, NAI::CM_CROUCH, 0);
            auto approach = NAI::SPathPlace(x - dx, y - dy, point.GetLayer(), dir, NAI::CM_CROUCH, 0);
            if (net->GetPassability(approach) != NAI::AIP_YES || net->GetPassability(landing) != NAI::AIP_YES ||
                NAI::GetTransitionType(net, point, landing) != NAI::TT_JUMP ||
                NAI::GetTransitionType(net, point, approach) != NAI::TT_MOVE ||
                NAI::GetTransitionType(net, approach, point) != NAI::TT_MOVE)
                continue;
            edge = point;
            back = approach;
            found = true;
            break;
        }
        if (found)
            break;
    }
    Require(found, "fixture map has no authored ledge with a real drop and a flat approach");
    auto from = actor->GetPosition(), atEdge = from;
    from.pos.p = back;
    from.bRun = false;
    atEdge.pos.p = edge;
    atEdge.bRun = false;
    actor->SetPosition(from);
    actor->animator.AlignTime();
    actor->animator.PlaceUnit(from);
    actor->animator.SetActiveItem(false);
    actor->SetUndrawItem(true);
    actor->animator.Climb(from, atEdge, false);
    actor->SetPosition(atEdge);
    world->GetTime()->Set(actor->animator.GetTimeEnd());
    auto path = CObj<NAI::CPath>(NWorld::FindPath(net, actor, edge, vector<NAI::SPathPlace>{back},
        nullptr, false, NAI::PF_USE_POSEDIR));
    Require(path && path->points.size() == 2 && path->points.back() == back,
        "the pathfinder could not back off directly from the authored ledge");
    NWorld::EUnitCommandResult result = NWorld::UCR_OK;
    CObj<NWorld::CCommandExecute> move = NWorld::CreateMoveExecutor(actor, path, NAI::PF_USE_POSEDIR,
        NWorld::ITEM_NO_MATTER, &result);
    Require(move && result == NWorld::UCR_OK, "the game refused the direct ledge retreat");
    const int hp = actor->GetUnitRPG()->GetTotalVP();
    const STime start = actor->animator.GetTimeEnd();
    const auto canMove = actor->CanDoGameMove(from);
    move->Run();
    for (int step = 0; step < 8 && move->GetState() == NWorld::CCommandExecute::RUNNING; ++step)
    {
        move->TimeLabelReached();
        world->GetTime()->Set(actor->animator.GetTimeEnd());
        move->AnimationFinished();
    }
    const auto reached = actor->GetPosition().pos.p;
    std::printf("ledge retreat: edge=%08x goal=%08x reached=%08x permission=%d state=%d clip=%lldms\n",
        unsigned(edge.GetBits()), unsigned(back.GetBits()), unsigned(reached.GetBits()), int(canMove),
        int(move->GetState()), static_cast<long long>(actor->animator.GetTimeEnd() - start));
    Require(reached == back && actor->animator.GetTimeEnd() > start,
        "the retreat did not step back from the ledge");
    Require(move->GetState() == NWorld::CCommandExecute::FINISHED && actor->GetUnitRPG()->GetTotalVP() == hp,
        "the retreat did not finish safely");
}

static void InactiveDoorTurn(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game,4517);auto* network=dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());Require(network,"inactive door turn lacks real path network");
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;for(auto unit:units)if(!unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK() && unit->GetUnitRPG()->CanMove()){actor=unit;break;}Require(actor,"inactive door turn lacks movable native human");
    CObj<NAI::CPath> selected;NWorld::CWindowDoor* selectedDoor=nullptr;NAI::SPathPlace source;int best=-1,candidates=0,generated=0,opposed=0;bool opening=false;
    for(int layer=0;layer<network->GetNumLayers() && !selected;++layer){auto* nodes=network->GetLayer(layer);for(int index=0;index<int(nodes->ladders.size()) && !selected;++index){const auto& ladder=nodes->ladders[index];if(ladder.pointPassable.empty())continue;auto landing=ladder.placeOnBottom;landing.SetPose(NAI::CM_STAND);NAI::SPathPlace rung(index,0,layer,(4+2*int(ladder.eDir))&7,NAI::CM_INACTIVE,0);rung.SetIntegral(0);
        if(!landing.IsIntegral() || landing.IsFinal() || network->GetPassability(rung)!=NAI::AIP_YES || network->IsLocked(rung,false))continue;
        for(int id:network->GetFlippersAt(landing)){NWorld::CWindowDoor* door=nullptr;for(const auto& entry:world->GetObjectsForHarness()){auto* candidate=dynamic_cast<NWorld::CWindowDoor*>(entry.GetPtr());if(candidate && network->GetFlipper(candidate)==network->GetFlipper(id)){door=candidate;break;}}if(!door || door->IsBroken() || door->IsMineSet())continue;door->LockDoor(false,0,1);
            for(int state=0;state<2 && !selected;++state){door->OpenClose(state!=0,true,nullptr);world->GetAIMap()->Sync();++candidates;CObj<NAI::CPath> path=new NAI::CPath;path->pNet=network;path->AddNewPoint(rung);NAI::CPath::SPathAction requested;requested.where=rung;requested.action=state?NAI::PA_CLOSE:NAI::PA_OPEN;requested.pObject=door;path->actions.push_back(requested);++generated;
                const auto& action=path->actions.front();if(!(action.where==rung) || action.pObject.GetPtr()!=door)continue;vector<NAI::SPathPlace> approaches;door->GetApproaches(&approaches,network);int bestDiff=7,newDir=-1;for(auto approach:approaches){int dir=network->GetClosestDir(rung,approach);if(fabs2(NAI::SPosition(rung,network).GetCP()-NAI::SPosition(approach,network).GetCP())<0.5f){int d=abs(approach.GetDirection()-dir);if(d>4)dir=(approach.GetDirection()+dir)/2+4;else if(d<4)dir=(approach.GetDirection()+dir)/2;if(dir>7)dir-=8;}int diff=abs(dir-rung.GetDirection());if(diff>4)diff=8-diff;if(diff<bestDiff){bestDiff=diff;newDir=dir;}}
                const auto swing=door->GetChangeStateDirection(action.action==NAI::PA_OPEN);const float angle=NAI::SPosition(rung,network).GetDirection();if(swing.x*cos(angle)+swing.y*sin(angle)>0 || newDir<0 || newDir==rung.GetDirection())continue;++opposed;auto invalid=rung;invalid.SetDirection(newDir);if(network->GetPassability(invalid)==NAI::AIP_YES)continue;selected=path;source=rung;selectedDoor=door;best=newDir;opening=action.action==NAI::PA_OPEN;break;
            }if(selected)break;
        }
    }}
    std::printf("INACTIVE-DOOR-INPUT mode=%d candidates=%d generated=%d opposed=%d found=%d\n",mode,candidates,generated,opposed,int(selected!=nullptr));std::fflush(stdout);Require(selected && selectedDoor,"inactive door turn lacks independently valid native ladder point with an explicit native door request and invalid turn");
    auto position=actor->GetPosition();position.pos.p=source;actor->SetPosition(position);actor->animator.AlignTime();actor->animator.PlaceUnit(position);world->GetAIMap()->Sync();network->Unlock(actor);const int sourcePass=int(network->GetPassability(source));network->Lock(actor,source);std::printf("INACTIVE-DOOR-PLACEMENT expected=%08X actual=%08X pass=%d\n",unsigned(source.GetBits()),unsigned(actor->GetPosition().pos.p.GetBits()),sourcePass);Require(actor->GetPosition().pos.p==source && sourcePass==NAI::AIP_YES,"inactive door turn lost real ladder position");
    CObj<NWorld::CCmdOpenClose> command=new NWorld::CCmdOpenClose(selectedDoor,opening);CObj<NWorld::CExecOpenClose> validator=new NWorld::CExecOpenClose(actor,command);Require(validator->CanDoIt(position)==NWorld::UCR_OK,"inactive door turn action is not independently allowed");
    if(mode==1)selected->actions.clear();if(mode>=2){bool free=false;vector<NAI::SPathPlace> approaches;selectedDoor->GetApproaches(&approaches,network);for(auto point:approaches){point.SetPose(NAI::CM_STAND);point.SetDirection(source.GetDirection());if(network->GetPassability(point)!=NAI::AIP_YES || network->IsLocked(point,false))continue;int diff=7,dirBest=-1;for(auto approach:approaches){int dir=network->GetClosestDir(point,approach);if(fabs2(NAI::SPosition(point,network).GetCP()-NAI::SPosition(approach,network).GetCP())<0.5f){int d=abs(approach.GetDirection()-dir);if(d>4)dir=(approach.GetDirection()+dir)/2+4;else if(d<4)dir=(approach.GetDirection()+dir)/2;if(dir>7)dir-=8;}int d=abs(dir-point.GetDirection());if(d>4)d=8-d;if(d<diff){diff=d;dirBest=dir;}}const auto swing=selectedDoor->GetChangeStateDirection(opening);const float angle=NAI::SPosition(point,network).GetDirection();auto turned=point;turned.SetDirection(dirBest);if(dirBest<0 || dirBest==point.GetDirection() || swing.x*cos(angle)+swing.y*sin(angle)>0 || network->GetPassability(turned)!=NAI::AIP_YES)continue;source=point;best=dirBest;free=true;break;}Require(free,"active door control lacks a real free authored approach and valid required turn");position.pos.p=source;actor->SetPosition(position);actor->animator.PlaceUnit(position);selected->points.front()=source;selected->actions.front().where=source;}
    if (mode >= 3)
    {
        Require(validator->CanDoIt(position) == NWorld::UCR_OK &&
            actor->CanSpendAP(2 * validator->GetStartAP()),
            "queued door fixture lacks a usable approach or AP for both actions");
        CObj<NWorld::CExecQueue> queue = new NWorld::CExecQueue(actor);
        queue->AddExecutor(validator);
        validator = nullptr;
        CObj<NWorld::CCmdOpenClose> opposite = new NWorld::CCmdOpenClose(selectedDoor, !opening);
        queue->AddExecutor(new NWorld::CExecOpenClose(actor, opposite));
        queue->Run();
        auto* current = queue->GetFrontExecutor();
        Require(current && current->IsExecuting() && selectedDoor->IsOpen() != opening,
            "the first door animation did not begin before the interrupt");
        const int remainingAP = actor->GetAP();
        if (mode == 3)
            queue->Cancel();
        Require(queue->GetFrontExecutor() == current && current->IsExecuting(),
            "the running door animation was discarded early");
        queue->TimeLabelReached();
        Require(selectedDoor->IsOpen() == opening, "the running door animation did not finish opening or closing");
        queue->AnimationFinished();
        if (mode == 3)
        {
            Require(queue->GetState() == NWorld::CCommandExecute::FAILED &&
                !queue->GetFrontExecutor() && actor->GetAP() == remainingAP,
                "interrupt allowed the next queued door action to start");
        }
        else
        {
            Require(queue->GetFrontExecutor() && queue->GetFrontExecutor()->IsExecuting(),
                "the ordinary door queue did not continue");
            queue->TimeLabelReached();
            queue->AnimationFinished();
            Require(queue->GetState() == NWorld::CCommandExecute::FINISHED &&
                selectedDoor->IsOpen() != opening, "the second door action did not finish");
        }
        return;
    }
    const int hp=actor->GetUnitRPG()->GetTotalVP(),ap=actor->GetAP();const auto time=world->GetTime()->GetValue();const bool doorState=selectedDoor->IsOpen();GlobalGameRandom().SeedForHarness(723);auto random=GlobalGameRandom();CObj<NWorld::CExecQueue> queue=new NWorld::CExecQueue(actor);queue->AddPath(selected,NAI::PF_DEFAULT,NWorld::ITEM_NO_MATTER);auto* front=queue->GetFrontExecutor();auto* mover=dynamic_cast<NWorld::IExecMove*>(front);Require(mover,"inactive door turn lost first native mover");NAI::SPathPlace goal;NAI::EFindPathParams params;NWorld::ENeedActiveItem active;mover->GetDesiredPlace(&goal,&params,&active);CObj<NAI::CPath> prepared=front->GetCurrentPath();int bad=0;network->Unlock(actor);if(prepared)for(auto point:prepared->points)if(!point.IsIntegral() && network->GetPassability(point)!=NAI::AIP_YES)++bad;network->Lock(actor,source);
    auto after=GlobalGameRandom();unsigned tails[4];bool same=true;for(int i=0;i<4;++i){tails[i]=after.Get();same&=tails[i]==random.Get();}bool pure=actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && time==world->GetTime()->GetValue() && doorState==selectedDoor->IsOpen();std::printf("INACTIVE-DOOR mode=%d pose=%d dir=%d best=%d goalDir=%d goalIntegral=%d points=%u bad=%d HP=%d AP=%d pure=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,int(source.GetPose()),source.GetDirection(),best,goal.GetDirection(),int(goal.IsIntegral()),prepared?unsigned(prepared->points.size()):0,bad,hp,ap,int(pure),int(same),tails[0],tails[1],tails[2],tails[3]);std::fflush(stdout);Require(pure && same,"inactive door preparation changed physical controls or RNG");Require(bad==0 && (mode!=0 || goal==source) && (mode!=2 || goal.GetDirection()==best),"door queue inserted a physically invalid turn on a native inactive ladder point");
}


static void PreviewMeleeSelector(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<2,"unknown native melee selector preview mode");const bool wantEmpty=mode==0;CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);auto* network=dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());Require(network,"melee selector lacks actual path network");list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;for(auto u:units)if(u && u->CanFight() && !u->IsAIUnit() && !u->IsWearingPK() && !u->IsEmptyPK()){actor=u;break;}Require(actor,"melee selector lacks real party actor");auto* mission=actor->GetUnitRPG();auto* inventory=mission->GetInventory();NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> records(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());while(records.MoveNext()){auto* p=records.Get();if(!p->bThrowing && p->pItem && p->pItem->pModel && (!record || p->GetRecordID()<record->GetRecordID()))record=p;}Require(record,"melee selector lacks authored blade");CObj<NRPG::CMeleeWeaponItem> blade=new NRPG::CMeleeWeaponItem(record);blade->nQuantity=1;CObj<NRPG::IInventoryItem> old=inventory->TakeOff(NDb::SLOT_1);Require(inventory->Equip(NDb::SLOT_1,blade) && inventory->Activate(NDb::SLOT_1) && NRPG::GetToHitType(actor)==NRPG::TH_MELEE,"melee selector cannot equip native melee action");actor->SetWalkWithoutWeapon(false);actor->SetUndrawItem(false);actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(mission->GetWeaponType());auto* map=world->GetAIMap();const auto initial=actor->GetPosition();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(initial.GetCP(),4.f),&places);CObj<NWorld::CUnitServer> target;
    for(auto p:places){if(!p.IsIntegral() || p.IsFinal())continue;p.SetPose(NAI::CM_STAND);p.SetMoving(0);const auto at=NAI::SPosition(p,network).GetCP();if(fabs2(at-initial.GetCP())<1.f || !network->IsPassable(p) || network->IsLocked(p))continue;target=world->AddUnitInGame(p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer(),"NativeMeleeSelectorTarget");break;}Require(target && target->CanFight(),"melee selector cannot deploy actual native target");const auto targetInitial=target->GetPosition();vector<NAI::SPathPlace> actorPlaces;network->GetNearPlaces(SSphere(targetInitial.GetCP(),3.f),&actorPlaces);bool found=false,selected=false;int examined=0,sourcePass=-1,desiredPass=-1,legacyCount=-1;CVec3 center=VNULL3;NAI::SUnitPosition directed;NRPG::STargetHLInfo selectedInfo;
    for(const auto targetPose:{NAI::CM_LAY,NAI::CM_STAND,NAI::CM_CROUCH}){
        for(int td=0;td<8 && !found;++td){auto tp=targetInitial.pos.p;tp.SetPose(targetPose);tp.SetDirection(NAI::EDirection(td));tp.SetMoving(0);network->Unlock(target);const bool targetFree=network->GetPassability(tp)==NAI::AIP_YES && network->IsPassable(tp);network->Lock(target,target->GetPosition().pos.p);if(!targetFree)continue;auto to=targetInitial;to.pos=NAI::SPosition(tp,network);to.SetPose(targetPose==NAI::CM_LAY?NAI::CRAWL:targetPose==NAI::CM_CROUCH?NAI::CROUCH:NAI::WALK);target->SetPosition(to);target->animator.AlignTime();target->animator.PlaceUnit(to);target->GetUnitRPG()->StartNewTurn(to.GetCP());MarkNewDGFrame();map->Sync();if(!map->GetUnitHLPos(&center,map->GetHull(target),NAI::HL_ANY))continue;
            for(auto base:actorPlaces){if(found)break;if(!base.IsIntegral() || base.IsFinal())continue;const auto cp=NAI::SPosition(base,network).GetCP();if(fabs2(cp-to.GetCP())<.1f || sqr(cp.x-center.x)+sqr(cp.y-center.y)>sqr(F_MELEE_DISTANCE))continue;
                for(const auto actorPose:{NAI::CM_STAND,NAI::CM_LAY,NAI::CM_CROUCH}){if(found)break;auto source=base;source.SetPose(actorPose);source.SetMoving(0);source.SetDirection(NRPG::GetShootDirection(network,source,center));network->Unlock(actor);const int pass=int(network->GetPassability(source));const bool free=network->IsPassable(source);network->Lock(actor,actor->GetPosition().pos.p);++examined;if(pass!=NAI::AIP_YES || !free)continue;auto from=initial;from.pos=NAI::SPosition(source,network);from.SetPose(actorPose==NAI::CM_LAY?NAI::CRAWL:actorPose==NAI::CM_CROUCH?NAI::CROUCH:NAI::WALK);if(!NWorld::CanMeleeAttack(actor,from,center))continue;actor->SetPosition(from);actor->animator.AlignTime();actor->animator.PlaceUnit(from);mission->StartNewTurn(from.GetCP());MarkNewDGFrame();map->Sync();world->GetGame()->SetNight(false);directed=actor->GetPosition();directed.pos.p.SetDirection(NRPG::GetShootDirection(network,directed.pos.p,center));const int actualPass=int(network->GetPassability(directed.pos.p));if(actualPass==NAI::AIP_NOT_PASSABLE || actualPass==NAI::AIP_CANNOT_LAY)continue;NRPG::STargetHLInfo info;const bool ok=NRPG::SelectTargetHLs(actor,directed,&info,target,NAI::HL_ANY);vector<int> legacy;map->GetAccessibleUnitHL(&legacy,actor->GetPosition().GetCenter(),map->GetHull(target),F_MELEE_DISTANCE);if(wantEmpty?(ok || !info.accessibleHLs.empty() || !legacy.empty()):(!ok || info.accessibleHLs.empty() || legacy.empty()))continue;found=true;selected=ok;selectedInfo=info;sourcePass=pass;desiredPass=actualPass;legacyCount=int(legacy.size());break;}
            }
        }if(found)break;
    }
    std::printf("MELEE-SELECTOR-INPUT mode=%d wantEmpty=%d found=%d examined=%d selected=%d accessible=%zu legacyCount=%d sourcePass=%d desiredPass=%d reach=%d centerX=%.9g centerY=%.9g centerZ=%.9g\n",mode,int(wantEmpty),int(found),examined,int(selected),selectedInfo.accessibleHLs.size(),legacyCount,sourcePass,desiredPass,int(found && NWorld::CanMeleeAttack(actor,actor->GetPosition(),center)),center.x,center.y,center.z);std::fflush(stdout);Require(found,"melee selector lacks actual reachable placement with declared native empty/nonempty hit-location set");actor->SetScriptToHit(37);vector<NRPG::CAttackPortion> attacks;mission->CreateAttack(&attacks,false);Require(!attacks.empty(),"melee selector lacks actual native attack");CObj<NRPG::CCoverInfo> cover=world->GetGame()->CalcCovers(NRPG::GetMeleeAttackPos(actor,center),attacks.front(),actor,target,NAI::HL_ANY,0);Require(cover,"melee selector lacks real native cover");const int expected=NRPG::GetAttackerToHit(actor,target,actor->GetCarefulShotExtraAP(),selectedInfo.eHL,selectedInfo.accessibleHLs,cover,mission->IsFirstTurn(),0);Require(expected==37,"melee selector native direct calcer lost script37 control");CObj<NScript::CScript> script=NScript::CreateScript(world);NScript::luaPushCPtr(*script,actor);script->SetGlobal("upstreamSelectorActor");NScript::luaPushCPtr(*script,target.GetPtr());script->SetGlobal("upstreamSelectorTarget");const auto position=actor->GetPosition(),targetPosition=target->GetPosition();const auto time=world->GetTime()->GetValue();const int hp=mission->GetTotalVP(),ap=actor->GetAP(),targetHP=target->GetUnitRPG()->GetTotalVP(),quantity=blade->GetQuantity();GlobalGameRandom().SeedForHarness(734);auto reference=GlobalGameRandom();const int actual=world->GetGame()->GetCompositeToHit(actor,target,NAI::HL_ANY,false);RunScript(script,"MeleeSelectorChance=UnitGetToHitUnit(upstreamSelectorActor,upstreamSelectorTarget,-1)");const auto lua=script->GetGlobal("MeleeSelectorChance");Require(lua.IsNumber(),"melee selector real Lua query did not return a number");auto after=GlobalGameRandom();bool rng=true;unsigned tail[4];for(int i=0;i<4;++i){tail[i]=after.Get();rng&=tail[i]==reference.Get();}const bool pure=actor->GetPosition()==position && target->GetPosition()==targetPosition && actor->GetAP()==ap && mission->GetTotalVP()==hp && target->GetUnitRPG()->GetTotalVP()==targetHP && inventory->GetActive()==blade.GetPtr() && blade->GetQuantity()==quantity && world->GetTime()->GetValue()==time && !actor->HasCommand();
    std::printf("MELEE-SELECTOR mode=%d wantEmpty=%d selected=%d accessible=%zu actual=%d lua=%d expected=%d HP=%d targetHP=%d AP=%d quantity=%d pure=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,int(wantEmpty),int(selected),selectedInfo.accessibleHLs.size(),actual,int(lua.GetNumber()),expected,hp,targetHP,ap,quantity,int(pure),int(rng),tail[0],tail[1],tail[2],tail[3]);std::fflush(stdout);Require(pure && rng,"melee selector preview changed physical/query RNG controls");Require(actual==expected && int(lua.GetNumber())==expected,"unit preview discarded the native calcer when hit-location selection returned false");
}

static void PreviewUnitFacing(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<4,"unknown native unit-facing preview mode");const int caseMode=mode;const int sourceModes[]={1,3,5,7};mode=sourceModes[mode];const bool object=(mode&1)!=0,blocked=mode<2;const auto pose=blocked || mode>=6?NAI::CM_LAY:mode>=4?NAI::CM_CROUCH:NAI::CM_STAND;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);world->GetAIMap()->Sync();auto* network=dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());Require(network,"attack pose lacks actual native network");list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsEmptyPK() && !unit->IsWearingPK()){actor=unit;break;}Require(actor,"attack pose lacks actual party actor");
    CObj<NRPG::CWeaponItem> gun;CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());while(weapons.MoveNext()){auto* p=weapons.Get();if(!p->shootModes[NDb::SM_Snap] || p->bBazookaLogic || !p->pItem || !p->pItem->pModel || !p->pInnerClip || !p->pWeaponType)continue;CObj<NRPG::CWeaponItem> candidate=new NRPG::CWeaponItem(p);if(candidate->GetWeaponType()==NDb::WT_PISTOL && candidate->GetAmmoQuantity()>0 && (!gun || p->GetRecordID()<gun->GetDBWeapon()->GetRecordID()))gun=candidate;}
    Require(gun && gun->SetShootMode(NDb::SM_Snap),"attack pose lacks authored loaded snap pistol");auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> old=inventory->TakeOff(NDb::SLOT_1);Require(inventory->Equip(NDb::SLOT_1,gun) && inventory->Activate(NDb::SLOT_1) && NWorld::GetActionType(actor)==NWorld::AT_SHOOT,"attack pose cannot equip actual shooting action");actor->SetWalkWithoutWeapon(false);actor->SetUndrawItem(false);actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
    const auto initial=actor->GetPosition();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(initial.GetCP(),8.f),&places);CObj<NWorld::CUnitServer> target;
    if(object){for(auto p:places){if(!p.IsIntegral() || p.IsFinal() || p.GetLayer()!=initial.pos.p.GetLayer())continue;p.SetPose(NAI::CM_STAND);p.SetMoving(0);NAI::SPosition candidate(p,network);const float d=fabs2(candidate.GetCP()-initial.GetCP());if(d<4.f || d>36.f || !network->IsPassable(p) || network->IsLocked(p))continue;target=world->AddUnitInGame(p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer(),"NativeAttackPoseTarget");break;}Require(target && target->CanFight(),"attack pose cannot deploy actual object-branch unit target");target->animator.AlignTime();target->animator.PlaceUnit(target->GetPosition());}
    CVec3 objectPoint=VNULL3;if(object){MarkNewDGFrame();world->GetAIMap()->Sync();Require(world->GetAIMap()->GetHull(target),"attack pose lacks actual target hull");world->GetAIMap()->GetUnitHLPos(&objectPoint,world->GetAIMap()->GetHull(target),NAI::HL_BODY);}
    bool found=false;NAI::SUnitPosition from,standing;CVec3 point;int currentPass=-1,desiredPass=-1,standPass=-1,direct=-1,examined=0;CObj<NWorld::CCmd> command;
    for(auto base:places){if(found)break;if(!base.IsIntegral() || base.IsFinal() || base.GetLayer()!=initial.pos.p.GetLayer())continue;const auto at=network->GetCP(base);if(object && (fabs2(at-target->GetPosition().GetCP())<1.f || fabs2(at-target->GetPosition().GetCP())>36.f))continue;
        for(int d=0;d<8 && !found;++d){auto source=base;source.SetPose(pose);source.SetDirection(d);source.SetMoving(0);auto desired=source;point=object?objectPoint:at+CVec3(float(cos(network->GetDirection(source)+(blocked?1.57079632679f:0))),float(sin(network->GetDirection(source)+(blocked?1.57079632679f:0))),0)*3.125f;desired.SetDirection(NRPG::GetShootDirection(network,source,point));if(!blocked && desired.GetDirection()!=source.GetDirection())continue;if(blocked && desired.GetDirection()==source.GetDirection())continue;auto upright=desired;upright.SetPose(NAI::CM_STAND);
            network->Unlock(actor);currentPass=int(network->GetPassability(source));desiredPass=int(network->GetPassability(desired));standPass=int(network->GetPassability(upright));const bool free=network->IsPassable(source) && (blocked?(desiredPass==NAI::AIP_CANNOT_LAY && standPass==NAI::AIP_YES && network->IsPassable(upright)):(desiredPass==NAI::AIP_YES && network->IsPassable(desired)));network->Lock(actor,actor->GetPosition().pos.p);if(currentPass!=NAI::AIP_YES || !free)continue;
            ++examined;from=initial;from.pos=NAI::SPosition(source,network);from.SetPose(pose==NAI::CM_LAY?NAI::CRAWL:pose==NAI::CM_CROUCH?NAI::CROUCH:NAI::WALK);actor->SetPosition(from);actor->animator.AlignTime();actor->animator.PlaceUnit(from);MarkNewDGFrame();world->GetAIMap()->Sync();standing=from;standing.pos.p=upright;if(object){CObj<NWorld::CExecShootUnit> probe=new NWorld::CExecShootUnit(actor,target,NAI::HL_ANY,0);direct=int(probe->CanDoIt(blocked?standing:from));}else{CObj<NWorld::CExecShootTile> probe=new NWorld::CExecShootTile(actor,point);direct=int(probe->CanDoIt(blocked?standing:from));}if(direct!=NWorld::UCR_OK)continue;if(object)command=new NWorld::CCmdShootObject(target,0);else command=new NWorld::CCmdShootTile(point);found=true;}
    }
    std::printf("UNIT-FACING-INPUT sourceMode=%d object=%d blocked=%d examined=%d found=%d direct=%d currentPass=%d desiredPass=%d standPass=%d\n",mode,int(object),int(blocked),examined,int(found),direct,currentPass,desiredPass,standPass);std::fflush(stdout);Require(found,"attack pose lacks real current-pose/turn-clearance and native valid action witnesses");
    Require(target,"unit-facing preview lacks native target");auto* mission=actor->GetUnitRPG();mission->StartNewTurn(actor->GetPosition().GetCP());target->GetUnitRPG()->StartNewTurn(target->GetPosition().GetCP());world->GetGame()->SetNight(false);actor->SetScriptToHit(37);CVec3 center=VNULL3;Require(world->GetAIMap()->GetUnitHLPos(&center,world->GetAIMap()->GetHull(target),NAI::HL_ANY),"unit-facing preview lacks actual ANY hull center");auto desired=actor->GetPosition();desired.pos.p.SetDirection(NRPG::GetShootDirection(network,desired.pos.p,center));const int pass=int(network->GetPassability(desired.pos.p));Require(blocked?(pass==NAI::AIP_NOT_PASSABLE || pass==NAI::AIP_CANNOT_LAY):(pass!=NAI::AIP_NOT_PASSABLE && pass!=NAI::AIP_CANNOT_LAY),"unit-facing preview native ANY facing differs from setup input classification");
    int expected=-1;if(!blocked){vector<NRPG::CAttackPortion> attacks;mission->CreateAttack(&attacks,false);Require(!attacks.empty(),"unit-facing preview lacks native attack");CObj<NRPG::CCoverInfo> cover=world->GetGame()->CalcCovers(actor->GetAttackOrigin(desired),attacks.front(),actor,target,NAI::HL_ANY,actor->GetMinClearDistance());Require(cover,"unit-facing preview lacks native cover");NRPG::STargetHLInfo info;Require(NRPG::SelectTargetHLs(actor,desired,&info,target,NAI::HL_ANY),"unit-facing pistol selector control failed");expected=NRPG::GetAttackerToHit(actor,target,0,info.eHL,info.accessibleHLs,cover,mission->IsFirstTurn(),0);Require(expected==37,"unit-facing clear control lacks native script37");}
    const auto position=actor->GetPosition(),targetPosition=target->GetPosition();const int hp=mission->GetTotalVP(),ap=actor->GetAP(),ammo=gun->GetAmmoQuantity(),targetHP=target->GetUnitRPG()->GetTotalVP();const auto time=world->GetTime()->GetValue();GlobalGameRandom().SeedForHarness(733);auto reference=GlobalGameRandom();const int actual=world->GetGame()->GetCompositeToHit(actor,target,NAI::HL_ANY,false);auto after=GlobalGameRandom();bool rng=true;unsigned tail[4];for(int i=0;i<4;++i){tail[i]=after.Get();rng&=tail[i]==reference.Get();}const bool pure=actor->GetPosition()==position && target->GetPosition()==targetPosition && actor->GetAP()==ap && mission->GetTotalVP()==hp && target->GetUnitRPG()->GetTotalVP()==targetHP && gun->GetAmmoQuantity()==ammo && inventory->GetActive()==gun.GetPtr() && world->GetTime()->GetValue()==time && !actor->HasCommand();
    std::printf("UNIT-FACING mode=%d sourceMode=%d blocked=%d pose=%d pass=%d actual=%d expected=%d HP=%d targetHP=%d AP=%d ammo=%d pure=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",caseMode,mode,int(blocked),int(pose),pass,actual,expected,hp,targetHP,ap,ammo,int(pure),int(rng),tail[0],tail[1],tail[2],tail[3]);std::fflush(stdout);Require(pure && rng,"unit-facing preview changed physical or queryRNG controls");Require(actual==expected,"unit-facing preview did not preserve native impossible-direction sentinel");
}

static void NativeDirectedAttackPose(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<8,"unknown directed attack pose mode");const bool object=(mode&1)!=0,blocked=mode<2;const auto pose=blocked || mode>=6?NAI::CM_LAY:mode>=4?NAI::CM_CROUCH:NAI::CM_STAND;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);world->GetAIMap()->Sync();auto* network=dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());Require(network,"attack pose lacks actual native network");list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsEmptyPK() && !unit->IsWearingPK()){actor=unit;break;}Require(actor,"attack pose lacks actual party actor");
    CObj<NRPG::CWeaponItem> gun;CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());while(weapons.MoveNext()){auto* p=weapons.Get();if(!p->shootModes[NDb::SM_Snap] || p->bBazookaLogic || !p->pItem || !p->pItem->pModel || !p->pInnerClip || !p->pWeaponType)continue;CObj<NRPG::CWeaponItem> candidate=new NRPG::CWeaponItem(p);if(candidate->GetWeaponType()==NDb::WT_PISTOL && candidate->GetAmmoQuantity()>0 && (!gun || p->GetRecordID()<gun->GetDBWeapon()->GetRecordID()))gun=candidate;}
    Require(gun && gun->SetShootMode(NDb::SM_Snap),"attack pose lacks authored loaded snap pistol");auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> old=inventory->TakeOff(NDb::SLOT_1);Require(inventory->Equip(NDb::SLOT_1,gun) && inventory->Activate(NDb::SLOT_1) && NWorld::GetActionType(actor)==NWorld::AT_SHOOT,"attack pose cannot equip actual shooting action");actor->SetWalkWithoutWeapon(false);actor->SetUndrawItem(false);actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
    const auto initial=actor->GetPosition();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(initial.GetCP(),8.f),&places);CObj<NWorld::CUnitServer> target;
    if(object){for(auto p:places){if(!p.IsIntegral() || p.IsFinal() || p.GetLayer()!=initial.pos.p.GetLayer())continue;p.SetPose(NAI::CM_STAND);p.SetMoving(0);NAI::SPosition candidate(p,network);const float d=fabs2(candidate.GetCP()-initial.GetCP());if(d<4.f || d>36.f || !network->IsPassable(p) || network->IsLocked(p))continue;target=world->AddUnitInGame(p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer(),"NativeAttackPoseTarget");break;}Require(target && target->CanFight(),"attack pose cannot deploy actual object-branch unit target");target->animator.AlignTime();target->animator.PlaceUnit(target->GetPosition());}
    CVec3 objectPoint=VNULL3;if(object){MarkNewDGFrame();world->GetAIMap()->Sync();Require(world->GetAIMap()->GetHull(target),"attack pose lacks actual target hull");world->GetAIMap()->GetUnitHLPos(&objectPoint,world->GetAIMap()->GetHull(target),NAI::HL_BODY);}
    bool found=false;NAI::SUnitPosition from,standing;CVec3 point;int currentPass=-1,desiredPass=-1,standPass=-1,direct=-1,examined=0;CObj<NWorld::CCmd> command;
    for(auto base:places){if(found)break;if(!base.IsIntegral() || base.IsFinal() || base.GetLayer()!=initial.pos.p.GetLayer())continue;const auto at=network->GetCP(base);if(object && (fabs2(at-target->GetPosition().GetCP())<1.f || fabs2(at-target->GetPosition().GetCP())>36.f))continue;
        for(int d=0;d<8 && !found;++d){auto source=base;source.SetPose(pose);source.SetDirection(d);source.SetMoving(0);auto desired=source;point=object?objectPoint:at+CVec3(float(cos(network->GetDirection(source)+(blocked?1.57079632679f:0))),float(sin(network->GetDirection(source)+(blocked?1.57079632679f:0))),0)*3.125f;desired.SetDirection(NRPG::GetShootDirection(network,source,point));if(!blocked && desired.GetDirection()!=source.GetDirection())continue;if(blocked && desired.GetDirection()==source.GetDirection())continue;auto upright=desired;upright.SetPose(NAI::CM_STAND);
            network->Unlock(actor);currentPass=int(network->GetPassability(source));desiredPass=int(network->GetPassability(desired));standPass=int(network->GetPassability(upright));const bool free=network->IsPassable(source) && (blocked?(desiredPass==NAI::AIP_CANNOT_LAY && standPass==NAI::AIP_YES && network->IsPassable(upright)):(desiredPass==NAI::AIP_YES && network->IsPassable(desired)));network->Lock(actor,actor->GetPosition().pos.p);if(currentPass!=NAI::AIP_YES || !free)continue;
            ++examined;from=initial;from.pos=NAI::SPosition(source,network);from.SetPose(pose==NAI::CM_LAY?NAI::CRAWL:pose==NAI::CM_CROUCH?NAI::CROUCH:NAI::WALK);actor->SetPosition(from);actor->animator.AlignTime();actor->animator.PlaceUnit(from);MarkNewDGFrame();world->GetAIMap()->Sync();standing=from;standing.pos.p=upright;if(object){CObj<NWorld::CExecShootUnit> probe=new NWorld::CExecShootUnit(actor,target,NAI::HL_ANY,0);direct=int(probe->CanDoIt(blocked?standing:from));}else{CObj<NWorld::CExecShootTile> probe=new NWorld::CExecShootTile(actor,point);direct=int(probe->CanDoIt(blocked?standing:from));}if(direct!=NWorld::UCR_OK)continue;if(object)command=new NWorld::CCmdShootObject(target,0);else command=new NWorld::CCmdShootTile(point);found=true;}
    }
    std::printf("ATTACK-POSE-INPUT mode=%d object=%d blocked=%d examined=%d found=%d direct=%d currentPass=%d desiredPass=%d standPass=%d\n",mode,int(object),int(blocked),examined,int(found),direct,currentPass,desiredPass,standPass);std::fflush(stdout);Require(found,"attack pose lacks real current-pose/turn-clearance and native valid action witnesses");
    const auto position=actor->GetPosition();const int hp=actor->GetUnitRPG()->GetTotalVP(),ap=actor->GetAP(),ammo=gun->GetAmmoQuantity();const auto time=world->GetTime()->GetValue();GlobalGameRandom().SeedForHarness(722);auto reference=GlobalGameRandom();NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCommandExecute> root=NWorld::CreateActionExecutor(actor,command,&result);int planned=-1,frontMove=0,start=-1;if(root){auto* queue=dynamic_cast<NWorld::CExecQueue*>(root.GetPtr());Require(queue,"attack pose lost real native movement/action queue");NAI::SPathPlace goal;NAI::EFindPathParams params;NWorld::ENeedActiveItem active;queue->GetDesiredPlace(&goal,&params,&active);planned=int(goal.GetPose());frontMove=dynamic_cast<NWorld::IExecMove*>(queue->GetFrontExecutor())!=nullptr;start=root->GetStartAP();}
    auto random=GlobalGameRandom();bool same=true;unsigned words[4];for(int i=0;i<4;++i){words[i]=random.Get();same&=words[i]==reference.Get();}
    std::printf("ATTACK-POSE mode=%d object=%d blocked=%d pose=%d queued=%d result=%d planned=%d frontMove=%d start=%d HP=%d AP=%d ammo=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,int(object),int(blocked),int(pose),int(root.GetPtr()!=nullptr),int(result),planned,frontMove,start,hp,ap,ammo,int(same),words[0],words[1],words[2],words[3]);std::fflush(stdout);
    Require(actor->GetPosition()==position && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && gun->GetAmmoQuantity()==ammo && inventory->GetActive()==gun.GetPtr() && world->GetTime()->GetValue()==time && !actor->HasCommand(),"attack pose query changed native actor/item/time/command controls");
    if(blocked)Require(!root,"directed attack silently planned a different stance when the current pose could not face its target");else Require(root && result==NWorld::UCR_OK && planned==int(pose),"directed attack changed a free standing/crouched/prone native control");
}

static void NativeMeleePreviewHull(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<6,"unknown melee preview hull mode");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game,810,[](NRPG::CGlobalGame* global)->NWorld::CWorld* { return new NativeMeleePreviewHullWorld(global); });
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsEmptyPK() && !unit->IsWearingPK()){actor=unit;break;}
    Require(actor,"melee preview hull lacks actual party actor");auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::CMeleeWeaponItem> blade;
    if(mode<4 || mode==5){NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> weapons(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());while(weapons.MoveNext()){auto* p=weapons.Get();if(p->bThrowing==(mode==3) && p->pItem && p->pItem->pModel && (!record || p->GetRecordID()<record->GetRecordID()))record=p;}
        Require(record,"melee preview hull lacks authored blade");blade=new NRPG::CMeleeWeaponItem(record);blade->nQuantity=1;CObj<NRPG::IInventoryItem> old=inventory->TakeOff(NDb::SLOT_1);Require(inventory->Equip(NDb::SLOT_1,blade) && inventory->Activate(NDb::SLOT_1),"melee preview hull cannot equip authored blade");}
    else {NDb::CRPGWeapon* record=nullptr;CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());while(weapons.MoveNext()){auto* p=weapons.Get();if(p->shootModes[NDb::SM_Snap] && !p->bBazookaLogic && p->pItem && p->pItem->pModel && p->pInnerClip && p->pWeaponType && (!record || p->GetRecordID()<record->GetRecordID()))record=p;}
        Require(record,"melee preview hull lacks authored snap firearm");CObj<NRPG::CWeaponItem> weapon=new NRPG::CWeaponItem(record);CObj<NRPG::IInventoryItem> old=inventory->TakeOff(NDb::SLOT_1);Require(inventory->Equip(NDb::SLOT_1,weapon) && inventory->Activate(NDb::SLOT_1) && weapon->GetAmmoQuantity()>0 && weapon->SetShootMode(NDb::SM_Snap),"melee preview hull cannot equip actual loaded firearm control");}
    const auto action=NRPG::GetToHitType(actor);Require(action==((mode<3 || mode==5)?NRPG::TH_MELEE:mode==3?NRPG::TH_THROWING:NRPG::TH_SHOOT),"melee preview hull has wrong actual action type");
    actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());actor->animator.AlignTime();actor->animator.PlaceUnit(actor->GetPosition());
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),mode==5?3.f:1.f),&places);NAI::SPathPlace adjacent;bool found=false;const auto from=actor->GetPosition();
    for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetMoving(0);NAI::SPosition pos(p,from.pos.pNet);const auto delta=pos.GetCP()-from.GetCP();if(fabs(delta.z)<.1f && (mode==5?(fabs2(delta)>sqr(1.4f) && fabs2(delta)<sqr(2.f)):(fabs2(delta)>.1f && fabs2(delta)<sqr(F_MELEE_DISTANCE))) && network->IsPassable(p) && !network->IsLocked(p)){adjacent=p;found=true;break;}}
    Require(found,"melee preview hull lacks actual free adjacent reachable tile");CObj<NWorld::CUnitServer> target=world->AddUnitInGame(adjacent,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer(),"NativeMeleePreviewHullTarget");Require(target && target->CanFight(),"melee preview hull cannot deploy real target");target->animator.AlignTime();target->animator.PlaceUnit(target->GetPosition());MarkNewDGFrame();world->GetAIMap()->Sync();
    auto* map=world->GetAIMap();CObjectBase* targetHull=map->GetHull(target);CObjectBase* actorHull=map->GetHull(actor);Require(targetHull && actorHull && targetHull!=actorHull,"melee preview hull lacks distinct real collision hulls");vector<int> expected,self;map->GetAccessibleUnitHL(&expected,actor->GetPosition().GetCenter(),targetHull,F_MELEE_DISTANCE);map->GetAccessibleUnitHL(&self,actor->GetPosition().GetCenter(),actorHull,F_MELEE_DISTANCE);
    Require(mode==5?(expected.empty() && !self.empty()):(!expected.empty() && expected!=self),"melee preview hull lacks a native reachable-hit-location witness distinguishing the two real hulls");
    if(mode==5){CVec3 center;Require(map->GetUnitHLPos(&center,targetHull,NAI::HL_ANY) && !NWorld::CanMeleeAttack(actor,from,center),"far hull fixture does not independently satisfy the new unreachable-center contract");}
    CObj<NativeMeleePreviewHullMap> probe=new NativeMeleePreviewHullMap(map);auto* observedWorld=dynamic_cast<NativeMeleePreviewHullWorld*>(world.GetPtr());Require(observedWorld,"melee preview lacks public world-map observer");observedWorld->observedMap=probe;CDGPtr<CFuncBase<STerrainInfo>> terrain=world->GetTerrainInfo();terrain.Refresh();CObj<NRPG::IGame> preview=NRPG::CreateGame(probe,network,terrain->GetValue());
    // Ranged controls exercise the real script override and action routing;
    // they do not claim ordinary ranged formula parity.
    if(mode==3 || mode==4)actor->SetScriptToHit(37);
    const auto targetPos=target->GetPosition();const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),targetHP=target->GetUnitRPG()->GetTotalVP();const auto* active=inventory->GetActive();const int quantity=active->GetQuantity();const auto time=world->GetTime()->GetValue();const auto location=mode==1?NAI::HL_HEAD:mode==2?NAI::HL_BODY:NAI::HL_ANY;GlobalGameRandom().SeedForHarness(721);auto referenceRandom=GlobalGameRandom();
    const int chance=preview->GetCompositeToHit(actor,target,location,world->IsFirstTurn());auto actualRandom=GlobalGameRandom();unsigned words[4];bool same=true;for(int i=0;i<4;++i){words[i]=actualRandom.Get();same&=words[i]==referenceRandom.Get();}
    std::printf("MELEE-PREVIEW-HULL mode=%d action=%d location=%d scripted=%d calls=%d targetHull=%d selfHull=%d expectedCount=%zu selfCount=%zu observedCount=%zu nativeSetMatches=%d chance=%d AP=%d HP=%d targetHP=%d RNGsame=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,int(action),int(location),int(mode==3 || mode==4),probe->calls,int(probe->received==targetHull),int(probe->received==actorHull),expected.size(),self.size(),probe->locations.size(),int(probe->locations==expected),chance,ap,hp,targetHP,int(same),words[0],words[1],words[2],words[3]);std::fflush(stdout);
    Require(actor->GetPosition()==from && target->GetPosition()==targetPos && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && target->GetUnitRPG()->GetTotalVP()==targetHP && inventory->GetActive()==active && active->GetQuantity()==quantity && world->GetTime()->GetValue()==time && !actor->HasCommand() && same,"melee preview changed native actor/target/item/time/RNG controls");
    if(mode==5)Require(chance==-1,"unreachable melee preview must report the original unavailable sentinel before querying hit locations");
    if(mode==5)Require(probe->calls==0 && probe->locations.empty(),"unreachable melee preview performed hit-location work before its independent reach gate");
    else if(mode<3)Require(probe->calls==1 && probe->received==targetHull && probe->locations==expected,"melee preview queried the attacker's collision hull instead of the target's native reachable hit locations");
    else Require(probe->calls==0 && chance==37,"scripted ranged blade/firearm control queried melee locations or lost its native override");
}

class UnitPreviewMapProbe: public CoverOriginMapProbe
{
    OBJECT_NOCOPY_METHODS(UnitPreviewMapProbe);
public:
    int meleeQueries=0;
    explicit UnitPreviewMapProbe(NAI::IAIMap* map=nullptr):CoverOriginMapProbe(map){}
    void GetAccessibleUnitHL(vector<int>* out,const CVec3& from,CObjectBase* hull,float distance) override {
        ++meleeQueries;native->GetAccessibleUnitHL(out,from,hull,distance);
    }
};
static void KnifeUnitPreview(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=5,"unknown knife preview mode");
    if(mode==3){MeleeReachCaption(databasePath,resourcePath);return;}
    auto world=FixtureWorld(databasePath,resourcePath,true);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;
    for(auto entry:units) {
        auto* person=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(!person || !person->CanFight() || person->IsEmptyPK() || person->IsWearingPK())continue;
        if(mode==4){auto* weapon=person->GetUnitRPG()->GetWeaponItem();if(!weapon || !weapon->SetShootMode(NDb::SM_Snap) || NRPG::GetToHitType(person)!=NRPG::TH_SHOOT)continue;}
        actor=person;break;
    }
    Require(actor,"knife preview fixture has no native ordinary attacker");
    CObj<NRPG::IInventoryItem> previous;CObj<NRPG::CMeleeWeaponItem> blade;
    auto* inventory=actor->GetUnitRPG()->GetInventory();
    if(mode!=4) {
        NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> records(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
        while(records.MoveNext())if(records.Get()->bThrowing && IsValid(records.Get()->pItem) && IsValid(records.Get()->pItem->pModel)){
            CObj<NRPG::CMeleeWeaponItem> candidate=new NRPG::CMeleeWeaponItem(records.Get());
            if(NDb::IsMeleeWeapon(candidate->GetWeaponType())){record=records.Get();break;}
        }
        Require(record,"knife preview fixture has no authored throwing item");
        blade=new NRPG::CMeleeWeaponItem(record);blade->nQuantity=1;
        previous=inventory->Get(NDb::SLOT_1)?inventory->TakeOff(NDb::SLOT_1):nullptr;
        Require(inventory->Equip(NDb::SLOT_1,blade) && inventory->Activate(NDb::SLOT_1) && NRPG::GetToHitType(actor)==NRPG::TH_THROWING && NDb::IsMeleeWeapon(actor->GetUnitRPG()->GetWeaponType()),
            "knife preview fixture could not equip its actual ranged action");
        actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
    }
    NWorld::CUnitServer* target=nullptr;
    const NAI::EHitLocation location=mode==1?NAI::HL_HEAD:mode==2?NAI::HL_BODY:NAI::HL_ANY;
    if(mode==5) {
        for(auto entry:units)if(entry.GetPtr()!=actor && entry->CanFight()){target=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());if(target)break;}
        Require(target,"replica preview fixture has no native target");int calls=0;
        world->bNetworkReplica=true;
        world->networkHitChance=[&](int kind,NWorld::CUnit* from,CObjectBase* to,CVec3,int hitLocation,bool first){
            ++calls;Require(kind==0 && from==actor && to==target && hitLocation==location && first==actor->GetUnitRPG()->IsFirstTurn(),"replica preview changed its remote query");return 73;};
        Require(world->GetGame()->GetCompositeToHit(actor,target,location,actor->GetUnitRPG()->IsFirstTurn())==73 && calls==1,"replica preview did not delegate exactly once");
        world->networkHitChance=nullptr;
        Require(world->GetGame()->GetCompositeToHit(actor,target,location,actor->GetUnitRPG()->IsFirstTurn())==0,"replica preview lost its unavailable remote control");return;
    }
    vector<NRPG::CAttackPortion> attacks;actor->GetUnitRPG()->CreateAttack(&attacks,false);
    Require(!attacks.empty(),"native preview fixture has no authored attack portion");
    int expected=0,hitCover=0;float distance=0;
    for(auto entry:units) {
        auto* person=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(!person || person==actor || !person->CanFight() || person->IsEmptyPK() || person->IsWearingPK())continue;
        distance=fabs(actor->GetPosition().GetCP()-person->GetPosition().GetCP());if(distance<2.5f)continue;
        auto pos=actor->GetPosition();pos.pos.p.SetDirection(NRPG::GetShootDirection(pos.pos.pNet,pos.pos.p,person->GetPosition().GetCP()));
        CObj<NRPG::CCoverInfo> cover=world->GetGame()->CalcCovers(actor->GetAttackOrigin(pos),attacks.front(),actor,person,location,actor->GetMinClearDistance());
        hitCover=NRPG::GetHitCover(actor,cover);if(hitCover<=0)continue;
        vector<int> noMeleeLocations;
        expected=0;const int rounds=actor->GetUnitRPG()->GetBulletsQuantityInShot();
        const CVec3 rayOrigin=CoverSnapshot(cover).source;
        const int rayDistance=int(fabs(rayOrigin-person->GetPosition().GetCP())*FP_INV_GRID_STEP);
        for(int i=0;i<rounds;++i)expected+=NRPG::GetToHit(actor,actor->GetPose(),rayDistance,rayOrigin,
            person->GetPosition().pos,location,actor->GetCarefulShotExtraAP(),person,noMeleeLocations,hitCover,actor->GetUnitRPG()->IsFirstTurn(),CVec3(1,1,1),
            !person->IsUnitVisible(actor),i,world->GetGame()->IsNight());
        expected/=rounds;if(expected>0){target=person;break;}
    }
    Require(target && expected>0,"native preview fixture has no positive unobstructed ranged unit chance outside arm reach");
    const auto from=actor->GetPosition(),to=target->GetPosition();const int ap=actor->GetAP(),hp=target->GetUnitRPG()->GetTotalVP();
    const auto* item=inventory->GetActive();const int quantity=item->GetQuantity();const auto rng=GlobalGameRandom();
    CObj<UnitPreviewMapProbe> probe=new UnitPreviewMapProbe(world->GetAIMap());
    CDGPtr<CFuncBase<STerrainInfo>> terrain=world->GetTerrainInfo();terrain.Refresh();
    CObj<NRPG::IGame> preview=NRPG::CreateGame(probe,world->GetPathNetwork(),terrain->GetValue());
    auto aimed=from;aimed.pos.p.SetDirection(NRPG::GetShootDirection(aimed.pos.pNet,aimed.pos.p,to.GetCP()));
    const CVec3 expectedOrigin=actor->GetAttackOrigin(aimed);
    const int actual=preview->GetCompositeToHit(actor,target,location,actor->GetUnitRPG()->IsFirstTurn());
    Require(probe->traces.size()==2,"unit preview lost actual high/low native cover projections");
    float originError=0;for(const auto& trace:probe->traces)originError=Max(originError,fabs(trace.origin-expectedOrigin));
    std::printf("native unit preview mode=%d action=%d distance=%.3f cover=%d chance=%d/%d location=%d originError=%.6g meleeQueries=%d night=%d visible=%d audible=%d\n",mode,int(NRPG::GetToHitType(actor)),distance,hitCover,actual,expected,int(location),originError,probe->meleeQueries,int(world->GetGame()->IsNight()),int(target->IsUnitVisible(actor)),int(target->IsUnitAudible(actor)));std::fflush(stdout);
    Require(originError<2e-4f && probe->meleeQueries==0,"ranged knife preview cast from melee height or requested arm-reach hit locations");
    Require(actual==expected && world->GetGame()->GetCompositeToHit(actor,target,location,actor->GetUnitRPG()->IsFirstTurn())==expected,"native ranged unit preview changed the existing chance calculation");
    auto expectedRng=rng;
    Require(actor->GetPosition()==from && target->GetPosition()==to && actor->GetAP()==ap && target->GetUnitRPG()->GetTotalVP()==hp && inventory->GetActive()==item && item->GetQuantity()==quantity && !actor->HasCommand() && expectedRng.Get()==GlobalGameRandom().Get(),
        "native unit preview changed an actor, its item, AP, health, action queue or gameplay RNG");
}

static void KnifeUnitDispatch(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=4,"unknown game knife dispatch mode");
    auto world=FixtureWorld(databasePath,resourcePath);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
    vector<NWorld::CUnitServer*> people;
    for(auto unit:units) {
        auto* actor=dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if(actor && actor->CanFight() && !actor->IsEmptyPK() && !actor->IsWearingPK() && actor->GetPosition().pos.p.GetPose()==NAI::CM_STAND)people.push_back(actor);
    }
    NWorld::CUnitServer *actor=nullptr,*target=nullptr;float nearest=1e20f;
    for(auto* a:people)for(auto* b:people)if(a!=b) {
        const float distance=fabs2(a->GetPosition().GetCP()-b->GetPosition().GetCP());
        if(distance>0.25f && distance<nearest){nearest=distance;actor=a;target=b;}
    }
    Require(actor && target && nearest<64.f,"unit dispatch fixture lacks two nearby standing native actors");
    NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> records(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
    while(records.MoveNext())if(records.Get()->bThrowing && IsValid(records.Get()->pItem) && IsValid(records.Get()->pItem->pModel)){record=records.Get();break;}
    Require(record,"unit dispatch fixture has no authored game blade");
    CObj<NRPG::CMeleeWeaponItem> blade=new NRPG::CMeleeWeaponItem(record);blade->nQuantity=1;
    auto* inventory=actor->GetUnitRPG()->GetInventory();
    CObj<NRPG::IInventoryItem> previous=inventory->Get(NDb::SLOT_1)?inventory->TakeOff(NDb::SLOT_1):nullptr;
    Require(inventory->Equip(NDb::SLOT_1,blade) && inventory->Activate(NDb::SLOT_1) && NWorld::GetActionType(actor)==NWorld::AT_THROW,
        "unit dispatch fixture could not equip its native game item");
    auto from=actor->GetPosition();const auto delta=target->GetPosition().GetCP()-from.GetCP();
    bool away=false;
    for(int direction=0;direction<8;++direction) {
        auto candidate=from;candidate.pos.p.SetDirection(direction);const float angle=candidate.pos.GetDirection();
        if(delta.x*cos(angle)+delta.y*sin(angle)<-0.5f*std::sqrt(delta.x*delta.x+delta.y*delta.y)){from=candidate;away=true;break;}
    }
    Require(away,"unit dispatch fixture could not select an opposing heading");
    actor->animator.PlaceUnit(from);actor->SetPosition(from);
    actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
    const NAI::EHitLocation location=mode==1?NAI::HL_HEAD:mode==2?NAI::HL_BODY:NAI::HL_ANY;
    CVec3 expectedPoint;world->GetAIMap()->GetUnitHLPos(&expectedPoint,world->GetAIMap()->GetHull(target),location);
    if(mode<3) {
        CObj<NWorld::CExecThrowKnife> direct=new NWorld::CExecThrowKnife(actor,location,target);
        Require(direct->CanDoIt(from)==NWorld::UCR_OK,"unit dispatch fixture target fails the existing native eligibility check");
    }
    CObj<NWorld::CCmd> command;
    if(mode==3) {
        bool found=false;
        for(const auto& offset:{CVec3(1,0,0),CVec3(-1,0,0),CVec3(0,1,0),CVec3(0,-1,0)}) {
            const auto point=from.GetCP()+offset;
            CObj<NWorld::CExecThrowKnife> probe=new NWorld::CExecThrowKnife(actor,point+CVec3(0,0,0.5f));
            if(probe->CanDoIt(from)==NWorld::UCR_OK){expectedPoint=point+CVec3(0,0,0.5f);command=new NWorld::CCmdShootTile(point);found=true;break;}
        }
        Require(found,"point control has no valid nearby native throw");
    } else {command=new NWorld::CCmdShootObject(mode==4?nullptr:target,0,location);if(mode==4)expectedPoint=from.GetCP()+CVec3(0,0,0.5f);}
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();const auto targetPlace=target->GetPosition();
    NWorld::EUnitCommandResult result=NWorld::UCR_UNAVAILABLE;
    CObj<NWorld::CCommandExecute> executor=NWorld::CreateExecutor(actor,command,&result);
    Require(executor && result==(mode==4?NWorld::UCR_NO_TARGET:NWorld::UCR_OK),"dispatch fixture command is not eligible for native preview");
    CPtr<NWorld::CExecThrowKnife> action=dynamic_cast<NWorld::CExecThrowKnife*>(executor.GetPtr());
    CStructureNetworkContext graph;graph.include=[&](CObjectBase* object){if(auto* found=dynamic_cast<NWorld::CExecThrowKnife*>(object))action=found;return dynamic_cast<NWorld::CSimpleExecQueue*>(object)!=nullptr;};
    CMemoryStream traversal;{CStructureSaver writer(traversal,CStructureSaver::WRITE,&graph);static_cast<CObjectBase*>(executor.GetPtr())->operator&(writer);}
    Require(IsValid(action),"game command queue contains no native throw executor");
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(action,&refs);
    CVec3 point;CPtr<NWorld::CUnitServer> savedTarget;NAI::EHitLocation savedLocation=NAI::HL_BODY;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&point);reader.Add(3,&savedTarget);reader.Add(5,&savedLocation);}
    std::printf("game knife dispatch mode=%d result=%d unitTarget=%d hitLocation=%d pointError=%.6g\n",mode,int(result),int(savedTarget.GetPtr()==target),int(savedLocation),fabs2(point-expectedPoint));std::fflush(stdout);
    Require(savedTarget.GetPtr()==(mode<3?target:nullptr) && savedLocation==location && fabs2(point-expectedPoint)<1e-8f,
        "unit throw command lost its target, hit location or native aim point by becoming a tile throw");
    if(mode<3) {
        CObj<NAI::CPath> path=executor->GetCurrentPath();Require(path && path->points.size()>=2,"unit throw did not queue a facing adjustment");
        NAI::SPosition destination(path->points.back(),world->GetPathNetwork());const float angle=destination.GetDirection();
        Require(path->points.back().GetX()==from.pos.p.GetX() && path->points.back().GetY()==from.pos.p.GetY() &&
            path->points.back().GetLayer()==from.pos.p.GetLayer() && delta.x*cos(angle)+delta.y*sin(angle)>0,
            "unit throw facing queue moved away from its tile or turned away from the native target");
    }
    fields.Seek(0);CObj<NWorld::CExecThrowKnife> restored=new NWorld::CExecThrowKnife;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);}
    CStructureNetworkContext newRefs;auto restoredFields=BorrowedObjectFields(restored,&newRefs);CPtr<NWorld::CUnitServer> loadedTarget;CVec3 loadedPoint;NAI::EHitLocation loadedLocation=NAI::HL_BODY;
    {CStructureSaver reader(restoredFields,CStructureSaver::READ,&newRefs);reader.Add(2,&loadedPoint);reader.Add(3,&loadedTarget);reader.Add(5,&loadedLocation);}
    Require(loadedTarget.GetPtr()==savedTarget.GetPtr() && loadedLocation==savedLocation && fabs2(loadedPoint-point)<1e-10f,"fresh executor restore lost its native target or aim point");
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==from && target->GetPosition()==targetPlace && inventory->GetActive()==blade.GetPtr() && blade->GetQuantity()==1,
        "command preview spent AP/health, moved an actor or consumed the equipped game item");
}

static void KnifeTimeRebase(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=3,"unknown knife clock mode");
    auto world=FixtureWorld(databasePath,resourcePath);
    NDb::CRPGMeleeWeapon* record=nullptr;
    CDBIterator<NDb::CRPGMeleeWeapon> records(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
    while(records.MoveNext())if(records.Get()->bThrowing && IsValid(records.Get()->pItem) && IsValid(records.Get()->pItem->pModel)){record=records.Get();break;}
    Require(record,"knife clock fixture lacks an authored throwing knife");
    SRand random;CObj<NDb::CModel> model=record->pItem->pModel->CreateModel(&random);
    CObj<NRPG::CMeleeWeaponItem> item=new NRPG::CMeleeWeaponItem(record);
    const CVec3 origin(11,12,13),velocity(3,4,5);
    const STime now=world->GetTime()->GetValue(),throwTime=now+(mode==0?500:0);
    const bool wasAction=world->IsAction();NRPG::CAttackPortion attack;
    CObj<NWorld::IDynamicObject> knife=NWorld::CreateKnifeServer(world,origin,velocity,throwTime,100.f,model,attack,item,nullptr);
    KnifePosePresentation visual;dynamic_cast<NWorld::IVisObj*>(knife.GetPtr())->Visit(&visual);
    auto* skeleton=dynamic_cast<NAnimation::CSkeletonAnimator*>(visual.animation.GetPtr());Require(skeleton,"knife did not expose its actual skeleton animator");
    auto sequence=ReadAnimationSequence(skeleton);
    CObj<NAnimation::CAnimator> linear=sequence->GetAnimator(now);
    Require(IsValid(linear) && pSSClasses->GetObjectTypeID(linear.GetPtr())==int(0x10661181),"knife fixture did not reach the actual registered linear animator");
    NAnimation::SSkeletonPose pose(1);
    if(mode==1)linear->GetFrame(now,&pose);
    const STime sample=mode==1?now-100:mode==2?now+125:now;
    const STime epoch=mode==0?now:mode==1?now-100:throwTime;
    linear->GetFrame(sample,&pose);
    const CVec3 expected=origin+velocity*(mode==2?0.125f:0.f);
    std::printf("knife clock mode=%d throw=%d sample=%d pos=(%.3f,%.3f,%.3f) error=%.6g\n",mode,int(throwTime),int(sample),pose[0].pos.x,pose[0].pos.y,pose[0].pos.z,fabs2(pose[0].pos-expected));std::fflush(stdout);
    Require(fabs2(pose[0].pos-expected)<1e-8f,"early knife pose underflowed its unsigned flight clock");
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(linear,&refs);
    STime savedStart=0;CVec3 savedOrigin,savedVelocity;CQuat rotation;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&savedStart);reader.Add(3,&savedOrigin);reader.Add(4,&savedVelocity);reader.Add(5,&rotation);}
    Require(savedStart==epoch && savedOrigin==origin && savedVelocity==velocity,"knife rebase changed saved origin/velocity or failed to persist its new epoch");
    CMemoryStream saved;{CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(2,&linear);}saved.Seek(0);
    CObj<NAnimation::CAnimator> restored;{CStructureSaver reader(saved,CStructureSaver::READ);reader.Add(2,&restored);}
    Require(IsValid(restored) && pSSClasses->GetObjectTypeID(restored.GetPtr())==int(0x10661181),"knife animation full save/load changed its native type");
    NAnimation::SSkeletonPose loaded(1);linear->GetFrame(epoch+250,&pose);restored->GetFrame(epoch+250,&loaded);
    const CVec3 future=origin+velocity*0.25f;
    Require(fabs2(pose[0].pos-future)<1e-8f && fabs2(loaded[0].pos-future)<1e-8f &&
        fabs2(pose[0].rot.Rotate(CVec3(0,0,-1))-rotation.Rotate(CVec3(0,0,-1)))<1e-10f &&
        fabs2(loaded[0].rot.Rotate(CVec3(0,0,-1))-rotation.Rotate(CVec3(0,0,-1)))<1e-10f,
        "knife clock rebase changed forward motion or the saved knife orientation");
    knife=nullptr;Require(world->IsAction()==wasAction,"knife clock inspection leaked its native action counter");
}

static void CorpseNullTaker(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=3,"unknown corpse null-taker mode");
    S2Platform::SetErrorDialogs(false);
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
    vector<NWorld::CUnitServer*> humans;
    for(const auto& unit:units) {
        auto* actor=dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if(actor && actor->CanFight() && !actor->IsEmptyPK() && !actor->IsWearingPK())humans.push_back(actor);
        if(humans.size()==2)break;
    }
    Require(humans.size()==2,"null-taker fixture needs a real body and carrier");
    auto* body=humans[0];auto* carrier=humans[1];
    body->GetUnitRPG()->GetRPGUnit()->Kill();body->SetState(new NWorld::CUnitStateDeath(body));
    Require(!body->animator.bIsCarried,"new corpse unexpectedly starts carried");
    if(mode==3)body->animator.BeTaken(carrier,&carrier->animator,body);
    auto* skeleton=body->animator.GetSkeletonAnimator();Require(skeleton,"null-taker fixture has no actual body skeleton animator");
    skeleton->pTime.Refresh();const STime now=skeleton->pTime->GetValue();
    auto sequence=ReadAnimationSequence(skeleton);
    CPtr<NAnimation::CAnimator> active=sequence->GetAnimator(now);Require(IsValid(active),"null-taker fixture has no current native animation");
    const bool carried=body->animator.bIsCarried;
    auto* bodyHead=body->GetUnitRPG()->GetRPGUnit()->GetHeadInfo();auto* carrierHead=carrier->GetUnitRPG()->GetRPGUnit()->GetHeadInfo();
    const int ap=carrier->GetAP(),health=carrier->GetUnitRPG()->GetTotalVP();
    const auto bodyPlace=body->GetPosition().GetCP(),carrierPlace=carrier->GetPosition().GetCP();
    NWorld::CUnit* wanted=(mode==1 || mode==3)?nullptr:carrier;
    std::printf("native corpse BeTaken mode=%d taker=%s previousCarried=%d carrierRef=%s\n",mode,mode==2?"actual":"NULL",int(carried),wanted?"actual":"NULL");std::fflush(stdout);
    body->animator.BeTaken(wanted,mode==2?&carrier->animator:nullptr,body);
    Require(body->animator.GetCorpseCarrier()==wanted,"BeTaken lost the supplied carrier reference");
    Require(body->animator.GetSkeletonAnimator()==skeleton && ReadAnimationSequence(skeleton).GetPtr()==sequence.GetPtr(),
        "BeTaken replaced the body's native skeleton or animation sequence object");
    auto* after=sequence->GetAnimator(now);
    if(mode==2)Require(body->animator.bIsCarried && after!=active.GetPtr(),"normal corpse pickup lost its native dynamics transition");
    else Require(body->animator.bIsCarried==carried && after==active.GetPtr(),"null taker changed the carried flag or native dynamics animation");
    Require(body->GetUnitRPG()->GetRPGUnit()->GetHeadInfo()==bodyHead && carrier->GetUnitRPG()->GetRPGUnit()->GetHeadInfo()==carrierHead,
        "body attachment changed either unit's separate facial-head object");
    Require(carrier->GetAP()==ap && carrier->GetUnitRPG()->GetTotalVP()==health && body->GetUnitRPG()->IsDead() &&
        fabs2(body->GetPosition().GetCP()-bodyPlace)<1e-10f && fabs2(carrier->GetPosition().GetCP()-carrierPlace)<1e-10f,
        "body animator attachment changed gameplay health/AP/death/positions");
    std::printf("native corpse BeTaken mode=%d carrierOK=1 carried=%d sameAnimation=%d head/AP/health/positions=unchanged\n",mode,int(body->animator.bIsCarried),int(after==active.GetPtr()));std::fflush(stdout);
}

static void CorpseCarrierAdmission(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=7,"unknown carrier admission mode");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
    vector<NWorld::CUnitServer*> ordinary;
    for(auto entry:units) {
        auto* unit=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK())ordinary.push_back(unit);
        if(ordinary.size()==2)break;
    }
    Require(ordinary.size()==2,"carrier admission needs two ordinary native actors");
    auto* actor=ordinary[0];auto* body=ordinary[1];
    body->GetUnitRPG()->GetRPGUnit()->Kill();body->SetState(new NWorld::CUnitStateDeath(body));
    CObj<NWorld::CExecCorpse> pickup=new NWorld::CExecCorpse(actor,body,true);
    pickup->Run();pickup->TimeLabelReached();
    auto state=ReadCorpseCancelUnitState(actor);
    Require(dynamic_cast<NWorld::CUnitStateCorpseCarrier*>(state.GetPtr()) && actor->GetCorpse()==body && body->GetCorpseCarrier()==actor,
        "carrier admission failed to establish actual reciprocal carrying");
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();
    const auto place=actor->GetPosition();
    auto* actorHead=actor->GetUnitRPG()->GetRPGUnit()->GetHeadInfo();auto* bodyHead=body->GetUnitRPG()->GetRPGUnit()->GetHeadInfo();
    CObj<NWorld::CCmd> command;
    switch(mode) {
        case 0:command=new NWorld::CCmdUsePassage(nullptr);break;
        case 1:{auto position=place;command=new NWorld::CCmdTeleport(position);break;}
        case 2:command=new NWorld::CCmdPlayAnimation(4437,false);break;
        case 3:command=new NWorld::CCmdTalk(body);break;
        case 4:command=new NWorld::CCmdNotHeroWantsToTalk;break;
        case 5:command=new NWorld::CCmdPath(place.pos);break;
        case 6:command=new NWorld::CCmdLook(place.pos);break;
        case 7:command=new NWorld::CCmdHide(true);break;
    }
    NWorld::EUnitCommandResult expectedResult=NWorld::UCR_OK,actualResult=NWorld::UCR_OK;
    CObj<NWorld::CCommandExecute> expected;
    if(mode!=7)expected=NWorld::CreateExecutor(actor,command,&expectedResult);
    CObj<NWorld::CCommandExecute> actual=state->CreateExecutor(command,&actualResult);
    std::printf("carrier admission mode=%d expected=%d/%d actual=%d/%d\n",mode,int(IsValid(expected)),int(expectedResult),int(IsValid(actual)),int(actualResult));std::fflush(stdout);
    if(mode==7)Require(!actual && actualResult==NWorld::UCR_UNAVAILABLE,"carrier admitted an unrelated Hide order");
    else {
        Require(actualResult==expectedResult && IsValid(actual)==IsValid(expected),"carrier rejected a command before its normal dispatch validation");
        if(actual)Require(typeid(*actual)==typeid(*expected),"carrier chose a different executor for an admitted command");
    }
    if(mode==1 || mode==2) {
        Require(IsValid(actual),"scripted carrier command lacks its native executor");
        actual->Run();
        if(mode==2)actual->AnimationFinished();
        Require(actual->GetState()==NWorld::CCommandExecute::FINISHED,"scripted carrier command did not finish");
    }
    Require(ReadCorpseCancelUnitState(actor).GetPtr()==state.GetPtr() && actor->GetCorpse()==body && body->GetCorpseCarrier()==actor && body->animator.bIsCarried,
        "admitted carrier command detached the body or replaced its carrying state");
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==place && body->IsDead() &&
        actor->GetUnitRPG()->GetRPGUnit()->GetHeadInfo()==actorHead && body->GetUnitRPG()->GetRPGUnit()->GetHeadInfo()==bodyHead,
        "carrier admission changed AP/health/position/death/facial-head identity");
}

struct AuthoredObjectStageAudio
{
    vector<vector<NDb::CSound*>> sounds;
    vector<NDb::CSoundEffect*> effects;
    AuthoredObjectStageAudio(NDb::CObject* record,int stage)
    {
        for(auto* node=record;IsValid(node);node=node->pChild) {
            auto* model=node->pModels[stage].GetPtr();if(!model)continue;
            if(model->eSoundType==NDb::ST_PERMANENT && IsValid(model->pSound)) {
                vector<NDb::CSound*> choices;
                for(auto variant:model->pSound->variants)if(IsValid(variant))choices.push_back(variant->pSound);
                sounds.push_back(choices);
            }
            if(IsValid(model->pSoundEffect))effects.push_back(model->pSoundEffect);
        }
    }
    bool Matches(const ObjectStageAudio& actual) const
    {
        if(actual.sounds.size()!=sounds.size() || actual.effects!=effects)return false;
        for(int i=0;i<sounds.size();++i)if(std::find(sounds[i].begin(),sounds[i].end(),actual.sounds[i])==sounds[i].end())return false;
        return true;
    }
};

static void ObjectSoundStage(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=11,"unknown object sound stage mode");
    auto world=FixtureWorld(databasePath,resourcePath);
    // The default mission has no differing stage audio. Instantiate an authored
    // staged record in the real world instead of treating missing fixture data as RED.
    CObj<NDb::CObject> record;
    CDBIterator<NDb::CRndObject> records(*NDatabase::GetTable<NDb::CRndObject>());
    SRand random;
    while(records.MoveNext()) {
        auto* candidate=records.Get();
        if(!IsValid(candidate->pTemplate))continue;
        CObj<NDb::CObject> created=candidate->CreateObject(&random,{});
        if(created->GetStagesQuantity()<2)continue;
        if(mode>=8 && (IsValid(created->pGrenade) || IsValid(created->pModels[1]->pAttachedGrenade)))continue;
        AuthoredObjectStageAudio first(created,0),second(created,1);
        if(mode>=6) { if(first.sounds.empty())continue; }
        else if(mode>=4) { if(first.effects.empty())continue; }
        else if(first.sounds==second.sounds && first.effects==second.effects)continue;
        record=created;break;
    }
    Require(IsValid(record),"database has no staged object with differing authored audio");
    CObj<NRPG::IObject> rpg=NRPG::CreateObject(record);
    Require(IsValid(rpg),"authored staged RPG object construction failed");
    NWorld::SObjectPlace placement{};placement.ptScale=CVec3(1,1,1);
    CObj<NWorld::CObjectServer> object=new NWorld::CObjectServer(world,placement,false,record,rpg,{},NWorld::TOD_ANYTIME);
    if(mode>=8) {
        CObj<NWorld::CWorld> other=new NWorld::CWorld(world->GetGlobalGame());
        world->GetTime()->Set(9000);world->GetAimTime()->Set(1375);
        other->GetAimTime()->Set(9999);
        auto ownTracker=world->ActionTrackerChunk(),otherTracker=other->ActionTrackerChunk();
        const int ownLag=*ownTracker.pnActionLag,otherLag=*otherTracker.pnActionLag;
        const bool otherWasAction=*otherTracker.pbWasAction;
        Require(ownLag<10 && otherLag<10,"object attack fixture already has a long action lag");
        struct RestoreContext {NWorld::CWorld* previous=NWorld::pCurrentWorld;~RestoreContext(){NWorld::pCurrentWorld=previous;}} restore;
        NWorld::pCurrentWorld=mode==9?other.GetPtr():mode==10?world.GetPtr():nullptr;
        NRPG::CAttackPortion attack(0,0,0.f,0,0,0);
        CRay ray;ray.ptOrigin=placement.ptPos;ray.ptDir=CVec3(1,0,0);
        if(mode!=11)attack.MakeClickOfDeath(ray);
        const int beforeHP=object->GetHP();
        const int damage=object->ProcessAttack(world,0,&attack,ray.ptDir,nullptr);
        const int stage=object->GetDestroyStage();
        world->GetAimTime()->Set(1500);
        std::printf("object attack world mode=%d stage=%d HP=%d->%d damage=%d age=%d ownerLag=%d foreignLag=%d\n",mode,stage,beforeHP,object->GetHP(),damage,object->GetTimeSinceLastStageChange(),*ownTracker.pnActionLag,*otherTracker.pnActionLag);std::fflush(stdout);
        if(mode==11) {
            Require(stage==0 && object->GetHP()==beforeHP && damage==0 && object->GetTimeSinceLastStageChange()==1500 && *ownTracker.pnActionLag==ownLag,
                "unchanged object attack changed stage timing or action lag");
        } else {
            Require(stage==1 && damage==beforeHP-object->GetHP() && damage>0,"object attack failed to advance exactly one authored stage");
            Require(object->GetTimeSinceLastStageChange()==125,"object attack stamped another world's aim clock");
            Require(*ownTracker.pnActionLag==10,"object attack touched another world's action counter");
        }
        Require(*otherTracker.pnActionLag==otherLag && *otherTracker.pbWasAction==otherWasAction,
            "object attack changed a foreign world's action tracker");
        return;
    }
    if(mode>=4) {
        world->GetTime()->Set(100);world->GetAimTime()->Set(100);
        object->SetDestroyStage(1);
        const STime begin=1375;
        world->GetTime()->Set(3000);world->GetAimTime()->Set(begin);
        object->SetDestroyStage(0);
        world->GetTime()->Set(9000);world->GetAimTime()->Set(1500);
        if(mode%2)object->SetDestroyStage(0); // unchanged stage must not restart audio
        ObjectStageAudio actual;object->Visit(&actual);
        auto& times=mode>=6?actual.soundTimes:actual.effectTimes;
        Require(!times.empty(),"authored record did not emit selected stage audio");
        std::printf("object sound clock mode=%d db=%d expected=%d actual=%d count=%zu\n",mode,object->GetDBObjectID(),int(begin),int(times.front()),times.size());std::fflush(stdout);
        Require(std::all_of(times.begin(),times.end(),[&](STime time){return time==begin;}),
            "object stage audio lost its stage-change timestamp");
        Require(object->GetTimeSinceLastStageChange()==125,"sound visit or unchanged stage reset gameplay stage age");
        return;
    }
    const int previous=mode==1?1:0,target=mode==1 || mode==3?0:1;
    object->SetDestroyStage(previous);
    NWorld::IRenderVisitor render;
    object->NWorld::CObjectServerBase::Visit(&render);
    ObjectStageAudio primed;object->Visit(&primed);
    Require(AuthoredObjectStageAudio(object->GetDBObjectForHarness(),previous).Matches(primed),"primed object sound does not match the authored previous stage");
    object->SetDestroyStage(target);
    if(mode==2)object->NWorld::CObjectServerBase::Visit(&render);
    const int health=object->GetHP();const auto place=object->GetPosition();
    ObjectStageAudio actual;object->Visit(&actual);
    AuthoredObjectStageAudio wanted(object->GetDBObjectForHarness(),target);
    std::printf("object sound stage mode=%d db=%d previous=%d actualRPG=%d sounds=%zu/%zu effects=%zu/%zu renderPrimed=%d\n",mode,object->GetDBObjectID(),previous,object->GetDestroyStage(),actual.sounds.size(),wanted.sounds.size(),actual.effects.size(),wanted.effects.size(),int(mode==2));std::fflush(stdout);
    Require(wanted.Matches(actual),"sound visitor used a stale visual destroy stage instead of the current RPG stage");
    Require(object->GetDestroyStage()==target && object->GetHP()==health && fabs2(object->GetPosition().ptPos-place.ptPos)<1e-10f,
        "object sound visit changed gameplay stage/health/position");
}

static void PKShellWorld(const char* databasePath, const char* resourcePath, bool extractPilot, bool checkModel = false, int checkFeedback = 0)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer *pilot = nullptr, *rescuer = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (!unit || !unit->CanFight()) continue;
        if (!unit->IsAIUnit()) pilot = unit; else if (!rescuer) rescuer = unit;
    }
    Require(pilot && rescuer, "dead-pilot fixture has no pilot or rescuer");
    NDb::CRPGPers* suitRecord = nullptr;
    auto* table = NDatabase::GetTable<NDb::CRPGPers>();
    CDBIterator<NDb::CRPGPers> it(*table);
    while (it.MoveNext()) {
        auto* record = it.Get();
        if (record->pPanzerklein && !record->pPanzerklein->bHasNoHead && record->pModel) { suitRecord = record; break; }
    }
    Require(suitRecord != nullptr, "fixture database has no regular PK shell");
    auto* network = world->GetPathNetwork();
    std::vector<NAI::SPathPlace> places;
    network->GetNearPlaces(SSphere(pilot->GetPosition().GetCP(), 20.f), &places);
    NAI::SPathPlace anchor;
    bool found = false;
    for (auto place : places) {
        place.SetPose(NAI::CM_STAND); place.SetDirection(0);
        if (NAI::IsBigLockerPlacePassable(network, place)) { anchor = place; found = true; break; }
    }
    Require(found, "fixture has no unobstructed PK footprint");
    CObj<NWorld::CUnitServer> shell = world->AddUnitInGame(anchor, NRPG::CreateUnit(suitRecord), nullptr);
    Require(shell && shell->IsEmptyPK(), "could not create a PK shell");
    if (checkFeedback == 4 || checkFeedback == 5) {
        Require(shell->WearAsPK(true), "hidden PK fixture could not prepare its shell");
        pilot->SetPosition(shell->GetPosition());
        pilot->FlipPanzerklein(shell, false, false);
        Require(pilot->IsWearingPK(), "hidden PK fixture did not equip its pilot");
        const int ap = pilot->GetRPG()->GetAP();
        for (bool hidden: {false, true}) {
            pilot->GetUnitRPG()->SetHiding(hidden);
            HiddenUnitPresentation shown;
            pilot->Visit(&shown);
            Require(shown.meshes == 1 && shown.alienMesh == hidden && shown.alienBegin == int(hidden)
                    && shown.alienEnd == int(hidden) && shown.alienDepth == 0,
                "hidden Panzerklein mesh did not use its bracketed alien material");
            Require(shown.filters.empty() && shown.flares == 0,
                "Panzerklein incorrectly received the human hiding tint or flare");
            Require(pilot->GetUnitRPG()->IsHiding() == hidden && pilot->GetRPG()->GetAP() == ap,
                "PK hiding presentation changed hiding state or action points");
        }
        if (checkFeedback == 5) {
            world->StartSequence();
            Require(world->IsSequence() && pilot->GetUnitRPG()->IsHiding(), "PK sequence fixture lost its hidden state");
            HiddenUnitPresentation cinematic;
            pilot->Visit(&cinematic);
            Require(cinematic.meshes == 1 && !cinematic.alienMesh && cinematic.alienBegin == 0
                    && cinematic.alienEnd == 0 && cinematic.filters.empty() && cinematic.flares == 0,
                "cinematic sequence still rendered the Panzerklein hiding material");
            Require(pilot->GetUnitRPG()->IsHiding(), "cinematic rendering cleared the PK's logical hiding state");
            world->EndOfTurn();
            Require(!world->IsSequence(), "PK sequence fixture did not leave the sequence");
            HiddenUnitPresentation resumed;
            pilot->Visit(&resumed);
            Require(resumed.alienMesh && resumed.alienBegin == 1 && resumed.alienEnd == 1,
                "PK hiding presentation did not resume after the sequence");
        }
        // The existing global material override remains independent of hiding.
        pilot->GetUnitRPG()->SetHiding(false);
        NGlobal::SetVar("i_am_an_alien", 1);
        HiddenUnitPresentation forced;
        pilot->Visit(&forced);
        NGlobal::SetVar("i_am_an_alien", 0);
        Require(forced.alienMesh && forced.alienBegin == 1 && forced.alienEnd == 1 && forced.alienDepth == 0,
            "PK hiding material changed the existing global alien-style override");
        return;
    }
    if (checkFeedback == 3) {
        Require(shell->WearAsPK(true), "turn-animation fixture could not prepare its shell");
        pilot->SetPosition(shell->GetPosition());
        pilot->FlipPanzerklein(shell, false, false);
        Require(pilot->IsWearingPK(), "turn-animation fixture did not put on the suit");
        const int ap = pilot->GetRPG()->GetAP();
        auto sequence = ReadAnimationSequence(pilot->animator.GetSkeletonAnimator());
        const auto time = pilot->animator.GetTimeEnd();
        CPtr<NAnimation::CAnimator> before = sequence->GetAnimator(time);
        Require(IsValid(before), "fixture PK has no active animation track");
        pilot->animator.StartNewTurn(pilot->GetPosition());
        Require(sequence->GetAnimator(time) == before.GetPtr(),
            "new turn replaced the Panzerklein animation with a normal standing track");
        Require(pilot->GetRPG()->GetAP() == ap, "PK turn animation changed action points");
        pilot->FlipPanzerklein(nullptr, false, false);
        pilot->animator.PlaceUnit(pilot->GetPosition());
        sequence = ReadAnimationSequence(pilot->animator.GetSkeletonAnimator());
        const auto ordinaryTime = pilot->animator.GetTimeEnd();
        before = sequence->GetAnimator(ordinaryTime);
        pilot->animator.StartNewTurn(pilot->GetPosition());
        Require(sequence->GetAnimator(ordinaryTime) != before.GetPtr(),
            "ordinary human lost the original turn-start standing refresh");
        return;
    }
    if (checkFeedback) {
        CObj<NWorld::CUnitStateNormal> state = new NWorld::CUnitStateNormal(pilot);
        if (checkFeedback == 1) {
            CObj<NWorld::CCmdHeal> command = new NWorld::CCmdHeal(nullptr);
            for (int value = NDb::C_PANZERKLEIN_AXIS; value <= NDb::C_PANZERKLEIN_TERRORS_HWG; ++value) {
                const auto status = static_cast<NDb::ECritical>(value);
                pilot->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_ANY, status));
                Require(pilot->GetUnitRPG()->HasCritical(status), "PK feedback fixture did not apply a status");
                auto result = NWorld::UCR_NO_TARGET;
                Require(state->IsCriticalsFailCommand(command, &result) && result == NWorld::UCR_PK_BAN,
                    "PK command ban reported an ordinary critical-injury rejection");
                Require(pilot->GetUnitRPG()->RemoveCritical(status), "PK fixture status could not be removed");
            }
            pilot->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD, NDb::C_BLIND, 3));
            auto result = NWorld::UCR_NO_TARGET;
            Require(state->IsCriticalsFailCommand(command, &result) && result == NWorld::UCR_CRITICALS_BAN,
                "ordinary injury incorrectly reported a PK restriction");
            pilot->GetUnitRPG()->RemoveCritical(NDb::C_BLIND);
            result = NWorld::UCR_NO_TARGET;
            Require(!state->IsCriticalsFailCommand(command, &result) && result == NWorld::UCR_NO_TARGET,
                "successful command probe did not preserve its prior result");
            return;
        }
        Require(shell->WearAsPK(true), "PK feedback fixture could not prepare its shell");
        pilot->SetPosition(shell->GetPosition());
        pilot->FlipPanzerklein(shell, false, false);
        Require(pilot->IsWearingPK(), "PK feedback fixture did not put on the suit");
        const int ap = pilot->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
        for (auto pose : {NAI::CRAWL, NAI::RUN}) {
            CObj<NWorld::CCmdWishPose> command = new NWorld::CCmdWishPose(pose);
            auto result = NWorld::UCR_NO_TARGET;
            Require(state->IsCriticalsFailCommand(command, &result) && result == NWorld::UCR_PK_BAN,
                "worn-PK pose ban reported an ordinary critical-injury rejection");
        }
        CObj<NWorld::CCmdWishPose> allowed = new NWorld::CCmdWishPose(NAI::WALK);
        auto result = NWorld::UCR_NO_TARGET;
        Require(!state->IsCriticalsFailCommand(allowed, &result) && result == NWorld::UCR_NO_TARGET,
            "allowed PK pose did not preserve its prior eligibility result");
        Require(pilot->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
            "PK eligibility feedback changed action points");
        return;
    }

    if (checkModel) {
        auto verifyPrivateModel = [](NWorld::CUnitServer* unit, NDb::CModel* source) {
            auto* live = unit->GetModel();
            Require(live && live != source, "PK mode change reused the shared RPG model");
            Require(live->pGeometry.GetPtr() == source->pGeometry.GetPtr() &&
                live->pSkeleton.GetPtr() == source->pSkeleton.GetPtr(), "private PK model lost its geometry or skeleton");
            auto geometry = source->pGeometry;
            Require(IsValid(geometry), "PK model fixture has no geometry");
            live->pGeometry = nullptr;
            Require(source->pGeometry.GetPtr() == geometry.GetPtr(), "live model mutation changed the RPG source model");
            live->pGeometry = geometry;
        };
        auto* humanModel = pilot->GetUnitRPG()->GetRPGUnit()->pModel.GetPtr();
        auto* suitModel = shell->GetUnitRPG()->GetRPGUnit()->pModel.GetPtr();
        Require(shell->WearAsPK(true), "could not prepare the model ownership fixture");
        pilot->SetPosition(shell->GetPosition());
        pilot->FlipPanzerklein(shell, false, false);
        verifyPrivateModel(pilot, suitModel);
        pilot->FlipPanzerklein(nullptr, false, false);
        verifyPrivateModel(pilot, humanModel);
        return;
    }
    if (!extractPilot) {
        NAI::SPosition target = shell->GetPosition().pos;
        bool targetFound = false;
        for (auto place : places) {
            place.SetPose(NAI::CM_STAND); place.SetDirection(0);
            if (!NAI::IsBigLockerPlacePassable(network, place)) continue;
            target.p = place;
            if (fabs2(target.GetCP() - shell->GetPosition().GetCP()) > 9.f) { targetFound = true; break; }
        }
        Require(targetFound, "fixture has no distant PK waypoint");
        CObj<NDb::CWaypointName> name = new NDb::CWaypointName;
        name->szName = "upstream_pk";
        CObj<CMapWaypoint> authored = new CMapWaypoint;
        authored->pName = name;
        authored->pos.ptPos = target.GetCP();
        world->AddWaypoint(authored);
        auto* waypoint = world->GetWaypoint(name->szName);
        Require(waypoint != nullptr, "PK waypoint was not registered");
        waypoint->pos = target;
        CObj<NScript::CScript> script = NScript::CreateScript(world);
        NScript::luaPushCPtr(*script, shell); script->SetGlobal("upstreamShell");
        RunScript(script, "UnitSetToWaypoint(upstreamShell, 'upstream_pk')");
        Require(shell->GetPosition().GetPose() == NAI::CRAWL, "scripted PK relocation lost the parked pose");
        Require(fabs2(shell->GetPosition().GetCP() - target.GetCP()) < .0025f, "scripted empty PK did not reach its waypoint");
        return;
    }
    Require(shell->WearAsPK(true), "could not prepare a worn PK shell");
    pilot->SetPosition(shell->GetPosition());
    pilot->FlipPanzerklein(shell, false, false);
    pilot->GetUnitRPG()->GetRPGUnit()->Kill();
    pilot->SetState(new NWorld::CUnitStateDeath(pilot));
    pilot->SetPosition(pilot->GetPosition());
    Require(pilot->IsDead() && pilot->IsWearingPK() && network->IsLocked(anchor, true),
        "dead pilot did not retain the worn footprint");
    CObj<NWorld::CExecCorpse> spill = new NWorld::CExecCorpse(rescuer, pilot, true);
    spill->Run();
    Require(!pilot->IsWearingPK() && shell->IsEmptyPK(), "corpse extraction left the suit occupied");
    Require(shell->GetPosition().pos.p.GetX() == anchor.GetX() && shell->GetPosition().pos.p.GetY() == anchor.GetY(),
        "restoring the shell treated its dead pilot as an obstacle and relocated it");
}

static void NativePKPilotExit(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<10,"unknown native PK pilot exit mode");CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* pilot=nullptr;
    for(auto u:units){auto* s=dynamic_cast<NWorld::CUnitServer*>(u.GetPtr());if(s && s->CanFight() && !s->IsAIUnit() && !s->IsEmptyPK() && !s->IsWearingPK()){pilot=s;break;}}Require(pilot,"PK exit lacks actual living player pilot");NDb::CRPGPers* suit=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());while(people.MoveNext()){auto* p=people.Get();if(p->pPanzerklein && !p->pPanzerklein->bHasNoHead && p->pModel && p->pPanzerklein->nMaxVP>0 && (!suit || p->GetRecordID()<suit->GetRecordID()))suit=p;}Require(suit,"PK exit lacks authored regular shell");
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(pilot->GetPosition().GetCP(),20.f),&places);NAI::SPathPlace anchor;bool found=false;const int direction=mode<8?mode:0;
    for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetMoving(0);p.SetDirection(direction);if(network->IsPassable(p) && !network->IsLocked(p) && NAI::IsBigLockerPlacePassable(network,p)){if(mode<8){NAI::SUnitPosition candidate;candidate.pos=NAI::SPosition(p,network);candidate.SetPose(NAI::WALK);const CVec3 cp=candidate.GetCP();const float a=candidate.GetDirection();NAI::SUnitPosition exit=candidate;exit.pos=NAI::GetNearestPosition(CVec3(cp.x+1.45f*cos(a),cp.y+1.45f*sin(a),cp.z),network,true,cp+CVec3(0,0,1));exit.SetPose(candidate.GetPose());exit.pos.p.SetDirection(candidate.GetDir());if(network->GetPassability(exit.pos.p)!=NAI::AIP_YES || fabs2(exit.GetCP()-cp)<=.25f)continue;}anchor=p;found=true;break;}}Require(found,"PK exit lacks actual free standing big footprint and distinct passable snapped exit");CObj<NWorld::CUnitServer> shell=world->AddUnitInGame(anchor,NRPG::CreateUnit(suit),nullptr);Require(shell && shell->IsEmptyPK() && shell->WearAsPK(true),"PK exit cannot prepare actual shell");pilot->SetPosition(shell->GetPosition());if(mode!=9)pilot->FlipPanzerklein(shell,false,false);pilot->GetUnitRPG()->GetRPGUnit()->UpdateSkills();Require(pilot->IsWearingPK()==(mode!=9),"PK exit lacks actual worn/nonworn identity");
    CObj<NWorld::CExecPanzerklein> executor=new NWorld::CExecPanzerklein(pilot);const auto initial=pilot->GetPosition();const int ap=pilot->GetAP(),shellHP=shell->GetUnitRPG()->GetTotalVP(),healed=pilot->GetUnitRPG()->GetRPGUnit()->nHealedVP;const auto time=world->GetTime()->GetValue();auto* inventory=pilot->GetUnitRPG()->GetInventory();auto random=GlobalGameRandom();
    if(mode>=8){const auto result=executor->CanDoIt(initial,mode==8);std::printf("native PK pilot exit mode=%d direction=%d query=1 result=%d expected=%d worn=%d\n",mode,direction,int(result),int(mode==8?NWorld::UCR_NO_TARGET:NWorld::UCR_INVALID_COMMAND),int(pilot->IsWearingPK()));std::fflush(stdout);Require(result==(mode==8?NWorld::UCR_NO_TARGET:NWorld::UCR_INVALID_COMMAND) && pilot->GetPosition()==initial && pilot->GetAP()==ap && pilot->GetUnitRPG()->GetInventory()==inventory && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"PK exit query controls changed ordinary eligibility, ownership, AP, pose, time or RNG");return;}
    Require(executor->CanDoIt(initial)==NWorld::UCR_OK,"worn PK exit lost native availability");network->Unlock(pilot);const CVec3 point=initial.GetCP();const float angle=initial.GetDirection();const CVec3 ahead(point.x+1.45f*cos(angle),point.y+1.45f*sin(angle),point.z);NAI::SUnitPosition expected=initial;expected.pos=NAI::GetNearestPosition(ahead,network,true,point+CVec3(0,0,1));expected.SetPose(initial.GetPose());expected.pos.p.SetDirection(initial.GetDir());const auto pass=network->GetPassability(expected.pos.p);network->Lock(pilot,initial.pos.p);Require(pass==NAI::AIP_YES && fabs2(expected.GetCP()-point)>.25f,"PK exit fixture lacks a distinct passable real snapped exit");
    const bool returned=executor->TimeLabelReached();const auto after=pilot->GetPosition(),parked=shell->GetPosition();auto animationPoint=[&](NWorld::CUnitServer* unit){auto sequence=ReadAnimationSequence(unit->animator.GetSkeletonAnimator());Require(sequence,"PK exit lost actual animation sequence");CObj<NAnimation::CAnimator> root=sequence->GetAnimator(unit->animator.GetTimeEnd());vector<CObj<NAnimation::CAnimator>> queue;std::set<CObjectBase*> seen;queue.push_back(root);for(std::size_t i=0;i<queue.size() && i<64;++i){auto* value=queue[i].GetPtr();if(!value || !seen.insert(value).second)continue;CStructureNetworkContext refs;auto fields=BorrowedObjectFields(value,&refs);if(dynamic_cast<NAnimation::CStandAnimator*>(value)){CVec3 pos;CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&pos);return pos;}for(const auto& ref:refs.refs)if(auto* next=dynamic_cast<NAnimation::CAnimator*>(ref.second.GetPtr()))queue.push_back(next);}Require(false,"PK exit cannot resolve actual standing placement from current animation graph");return VNULL3;};const CVec3 pilotAnim=animationPoint(pilot),shellAnim=animationPoint(shell);CVec3 expectedPilot=expected.GetCP(),expectedShell=point;expectedPilot.z=expectedShell.z=0;const float pilotError=fabs(after.GetCP()-expected.GetCP()),shellError=fabs(parked.GetCP()-point),pilotAnimError=fabs(pilotAnim-expectedPilot),shellAnimError=fabs(shellAnim-expectedShell);
    std::printf("native PK pilot exit mode=%d direction=%d query=0 persona=%d shellPersona=%d pass=%d fromX=%.9g fromY=%.9g expectedX=%.9g expectedY=%.9g pilotX=%.9g pilotY=%.9g shellX=%.9g shellY=%.9g pilotError=%.9g shellError=%.9g pilotAnimError=%.9g shellAnimError=%.9g pilotPose=%d expectedPose=%d shellPose=%d worn=%d empty=%d returned=%d\n",mode,direction,pilot->GetUnitRPG()->GetRPGPers()->GetRecordID(),suit->GetRecordID(),int(pass),point.x,point.y,expected.GetCP().x,expected.GetCP().y,after.GetCP().x,after.GetCP().y,parked.GetCP().x,parked.GetCP().y,pilotError,shellError,pilotAnimError,shellAnimError,int(after.GetPose()),int(initial.GetPose()),int(parked.GetPose()),int(pilot->IsWearingPK()),int(shell->IsEmptyPK()),int(returned));std::fflush(stdout);
    Require(!returned && !pilot->IsWearingPK() && shell->IsEmptyPK() && pilot->CanFight() && pilot->GetAP()==ap && shell->GetUnitRPG()->GetTotalVP()==shellHP && pilot->GetUnitRPG()->GetRPGUnit()->nHealedVP==healed && pilot->GetUnitRPG()->GetInventory()==inventory && world->GetTime()->GetValue()==time,"PK exit changed callback/identity/AP/HP/inventory/time controls");
    Require(pilotError<.0001f && shellError<.0001f && pilotAnimError<.0001f && shellAnimError<.0001f && after.GetPose()==initial.GetPose() && after.GetDir()==initial.GetDir() && parked.GetPose()==NAI::CRAWL && parked.GetDir()==initial.GetDir() && !parked.bRun,"PK exit moved the shell instead of its pilot or left simulation/animation placements inconsistent");
}

struct NativeWornPKFixture
{
    CObj<NRPG::CGlobalGame> game;CObj<NWorld::CWorld> world;NWorld::CUnitServer* pilot=nullptr;CObj<NWorld::CUnitServer> shell;NDb::CRPGPers* suit=nullptr;
    NativeWornPKFixture(const char* databasePath,const char* resourcePath,bool worn=true)
    {
        world=FixtureWorld(databasePath,resourcePath,true,&game);vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);for(auto u:units){auto* s=dynamic_cast<NWorld::CUnitServer*>(u.GetPtr());if(s && s->CanFight() && !s->IsAIUnit() && !s->IsEmptyPK() && !s->IsWearingPK()){pilot=s;break;}}Require(pilot,"PK collapse lacks actual human pilot");CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());while(people.MoveNext()){auto* p=people.Get();if(p->pPanzerklein && !p->pPanzerklein->bHasNoHead && p->pModel && p->pPanzerklein->nMaxVP>0 && (!suit || p->GetRecordID()<suit->GetRecordID()))suit=p;}Require(suit,"PK collapse lacks authored regular suit");auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(pilot->GetPosition().GetCP(),20.f),&places);NAI::SPathPlace anchor;bool found=false;for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetMoving(0);p.SetDirection(0);if(network->IsPassable(p) && !network->IsLocked(p) && NAI::IsBigLockerPlacePassable(network,p)){anchor=p;found=true;break;}}Require(found,"PK collapse lacks free actual footprint");shell=world->AddUnitInGame(anchor,NRPG::CreateUnit(suit),nullptr);Require(shell && shell->WearAsPK(true),"PK collapse cannot prepare actual shell");pilot->SetPosition(shell->GetPosition());if(worn)pilot->FlipPanzerklein(shell,false,false);pilot->GetUnitRPG()->GetRPGUnit()->UpdateSkills();Require(pilot->IsWearingPK()==worn && pilot->CanFight(),"PK collapse lacks actual worn identity/living state");world->GetTime()->Set(1000);pilot->animator.AlignTime();pilot->animator.PlaceUnit(pilot->GetPosition());
    }
};

static void NativePKCollapse(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<7,"unknown native PK collapse mode");const bool worn=mode!=5,expected=mode<5;NativeWornPKFixture fixture(databasePath,resourcePath,worn);auto& world=fixture.world;auto* pilot=fixture.pilot;auto* mission=pilot->GetUnitRPG();if(mode==3){auto pos=pilot->GetPosition();pos.SetPose(NAI::CRAWL);pilot->SetPosition(pos);pilot->animator.PlaceUnit(pos);}const auto before=pilot->GetPosition();const int ap=pilot->GetAP(),hp=mission->GetTotalVP(),shellHP=fixture.shell->GetUnitRPG()->GetTotalVP();const int clipID=mode==3?2466:2484;auto* authored=NDb::GetDBAnimation(clipID);auto* expectedSound=NDb::GetSound(16584);Require(authored && expectedSound,"PK collapse lacks authored frozen clip or impact sound");const auto previous=NativeCannonAnimationClips(pilot->animator.GetSkeletonAnimator());std::set<NWorld::IDynamicObject*> oldObjects;for(const auto& object:*world->GetMiscObjects())oldObjects.insert(object.GetPtr());const auto time=world->GetTime()->GetValue();
    if(mode==0 || mode==3){mission->GetRPGUnit()->Kill();pilot->SyncConscious();}
    else if(mode==1 || mode==2 || mode==5)pilot->MakeUnconscious(VNULL3,mode==2 || mode==5,mode!=2 && mode!=5);
    else{if(mode==4){mission->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_STUN,3));Require(mission->HasCritical(NDb::C_STUN),"PK collapse cannot apply actual stun");}auto* armor=mission->GetRPGArmor();Require(armor && armor->pMaterial,"PK collapse lacks actual worn armor");int type=-1;CDBIterator<NDb::CRPGDmgToArmor> types(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());while(types.MoveNext())if(types.Get()->armors[armor->pMaterial->nDR]>0 && (type<0 || types.Get()->GetRecordID()<type))type=types.Get()->GetRecordID();Require(type>=0,"PK collapse lacks valid damage type");NRPG::CAttackPortion attack(0,type,0.f,0,0,0,0,nullptr,mission,1.f,-1);Require(pilot->ProcessAttack(world,NAI::HL_BODY,&attack,VNULL3,armor)==0,"PK stun animation fixture unexpectedly dealt damage");}
    const auto after=NativeCannonAnimationClips(pilot->animator.GetSkeletonAnimator());int clips=0;STime duration=0;for(auto* clip:after)if(std::find(previous.begin(),previous.end(),clip)==previous.end() && clip->GetRecordID()==clipID){++clips;duration=clip->GetTime();}auto* state=dynamic_cast<NAnimation::CSkeletonState*>(pilot->animator.GetSkeletonState());Require(state,"PK collapse lost native skeleton state");const bool frozen=(state->state.cIdleBannedFlags & NAnimation::E_INTERNAL_IDLE_OFF)!=0;ObjectStageAudio audio;for(const auto& object:*world->GetMiscObjects())if(!oldObjects.count(object.GetPtr()))if(auto* sound=dynamic_cast<NWorld::C3DSound*>(object.GetPtr()))sound->Visit(&audio);const int sounds=int(std::count(audio.sounds.begin(),audio.sounds.end(),expectedSound));
    std::printf("native PK collapse mode=%d worn=%d persona=%d suit=%d pose=%d run=%d expected=%d clip=%d clips=%d duration=%u frozen=%d sounds=%d AP=%d afterAP=%d HP=%d afterHP=%d dead=%d unconscious=%d fight=%d end=%u\n",mode,int(worn),mission->GetRPGPers()->GetRecordID(),fixture.suit->GetRecordID(),int(before.GetPose()),int(before.bRun),int(expected),clipID,clips,unsigned(duration),int(frozen),sounds,ap,pilot->GetAP(),hp,mission->GetTotalVP(),int(pilot->IsDead()),int(pilot->IsUnconscious()),int(pilot->CanFight()),unsigned(pilot->animator.GetTimeEnd()));std::fflush(stdout);
    Require(pilot->GetPosition()==before && pilot->IsWearingPK()==worn && (!worn || pilot->GetWearingPK()==fixture.shell.GetPtr()) && fixture.shell->GetUnitRPG()->GetTotalVP()==shellHP && world->GetTime()->GetValue()==time,"PK collapse changed logical position/worn identity/shell HP/time");
    Require(expected?(clips==1 && duration>0 && frozen && sounds==1):(clips==0 && sounds==0),"PK collapse omitted the authored held-last-frame clip/impact sound or affected a non-PK/non-stun control");
}

static void NativePKCorpseGate(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<4,"unknown native PK corpse gate mode");const bool worn=mode!=2,down=mode!=3;NativeWornPKFixture fixture(databasePath,resourcePath,worn);auto* pilot=fixture.pilot;auto* mission=pilot->GetUnitRPG();if(mode==1){mission->GetRPGUnit()->SetUnconscious(true);pilot->SetState(new NWorld::CUnitStateUnconscious(pilot));}else if(down){mission->GetRPGUnit()->Kill();pilot->SetState(new NWorld::CUnitStateDeath(pilot));}Require(pilot->CanFight()==!down && pilot->IsWearingPK()==worn && pilot->GetCorpseHLs()->empty(),"PK corpse gate lacks actual fresh downed/worn/cache state");const auto before=pilot->GetPosition();const int ap=pilot->GetAP(),hp=mission->GetTotalVP();const auto time=fixture.world->GetTime()->GetValue();pilot->NWorld::CDumbUnitServer::Segment();const int count=int(pilot->GetCorpseHLs()->size()),expected=mode==2?6:0;
    std::printf("native PK corpse gate mode=%d worn=%d down=%d persona=%d suit=%d beforeHL=0 afterHL=%d expectedHL=%d positionChanged=%d AP=%d afterAP=%d HP=%d afterHP=%d\n",mode,int(worn),int(down),mission->GetRPGPers()->GetRecordID(),fixture.suit->GetRecordID(),count,expected,int(!(pilot->GetPosition()==before)),ap,pilot->GetAP(),hp,mission->GetTotalVP());std::fflush(stdout);
    Require(pilot->IsWearingPK()==worn && pilot->CanFight()==!down && pilot->GetAP()==ap && mission->GetTotalVP()==hp && fixture.world->GetTime()->GetValue()==time,"PK corpse gate changed worn/consciousness/AP/HP/time controls");Require(count==expected && (mode==2 || pilot->GetPosition()==before),"worn PK entered ordinary corpse position/hit-location refresh");
}

static NDb::CAISound* NativeHearingSound(NRPG::IUnitMission* listener,bool loud)
{
    const int boundary=listener->GetAISoundConstants()->nLoudSound;Require(boundary>0,"hearing lacks positive authored loud threshold");NDb::CAISound* result=nullptr;CDBIterator<NDb::CAISound> sounds(*NDatabase::GetTable<NDb::CAISound>());while(sounds.MoveNext()){auto* s=sounds.Get();const int radius=s->GetRadiusFromAISoundType(0);if(radius>0 && (radius>=boundary)==loud && (!result || s->GetRecordID()<result->GetRecordID()))result=s;}Require(result,"hearing lacks an actual authored quiet/loud sound");return result;
}

static void NativeHearingProbability(const char* databasePath,int mode)
{
    Require(mode>=0 && mode<6,"unknown native hearing probability mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);const int id=mode<2?20:76;auto listener=NativeMissionWithUntakenPerk(id);auto* unit=listener->GetRPGUnit();CObj<NRPG::IUnitMission> reference=NRPG::CreateUnit(unit->pPers),source=NRPG::CreateUnit(NDb::GetPers(103));auto* tree=unit->GetPerksTree();const int points=tree->GetPerkPoints();if(mode<4){tree->AddPerkPoints(1);tree->TakePerk(id);Require(listener->HasPerk(id) && tree->GetPerkPoints()==points,"hearing cannot grant actual owned authored perk");}float parameter=1;const bool owned=listener->HasPerk(id,&parameter);Require(mode>=4 || (owned && parameter>0 && std::isfinite(parameter)),"hearing lacks positive authored perk parameter");std::vector<CObj<NRPG::CSkillModifier>> held;
    const bool loud=mode==2 || mode==3;NDb::SAISound sound={NativeHearingSound(listener,loud),0,1.f};float distance=0;int expected=-1,spot=20,referenceSpot=20,legacySpot=20;CObj<NRPG::CDynamicSkill> shellVP;
    if(mode<2){spot=mode==0?1:2;for(;spot<200;++spot){const float product=float(spot)*parameter;referenceSpot=int(product);legacySpot=int(product+.5f);if(referenceSpot>0 && (mode==0?legacySpot!=referenceSpot:legacySpot==referenceSpot))break;}Require(spot<200,"hearing authored spot multiplier cannot isolate integer rounding boundary");NativeSetTrainingStableSkill(unit,NDb::ST_SPOT,spot,held);NativeSetTrainingStableSkill(source->GetRPGUnit(),NDb::ST_STEALTH,40,held);bool found=false;for(int step=0;step<400;++step){const float d=step*.125f;NativeSetTrainingStableSkill(reference->GetRPGUnit(),NDb::ST_SPOT,referenceSpot,held);const int exact=reference->GetHearingProbability(source,d,sound,nullptr);NativeSetTrainingStableSkill(reference->GetRPGUnit(),NDb::ST_SPOT,legacySpot,held);const int old=reference->GetHearingProbability(source,d,sound,nullptr);if(exact>0 && exact<95 && (mode==0?exact!=old:true)){distance=d;expected=exact;found=true;break;}}Require(found,"hearing lacks an unclamped independent unperked reference boundary");NativeSetTrainingStableSkill(reference->GetRPGUnit(),NDb::ST_SPOT,referenceSpot,held);}
    else{NativeSetTrainingStableSkill(unit,NDb::ST_SPOT,spot,held);NativeSetTrainingStableSkill(reference->GetRPGUnit(),NDb::ST_SPOT,spot,held);if(mode==2){Require(parameter!=1.f,"hearing sensor modifier does not distinguish its human branch");float low=0,high=10000;for(int i=0;i<32;++i){const float middle=(low+high)*.5f;if(reference->GetHearingProbability(source,middle,sound,nullptr)==100)low=middle;else high=middle;}distance=low*(1.f+parameter)*.5f;expected=reference->GetHearingProbability(source,distance,sound,nullptr);Require(expected==(parameter>1.f?0:100),"hearing lacks actual human hard-range boundary");}else if(mode==3){NDb::CPanzerklein* record=nullptr;CDBIterator<NDb::CPanzerklein> suits(*NDatabase::GetTable<NDb::CPanzerklein>());while(suits.MoveNext())if(suits.Get()->nMaxVP>0 && suits.Get()->fSensorRange>0 && (!record || suits.Get()->GetRecordID()<record->GetRecordID()))record=suits.Get();Require(record,"hearing lacks actual PK sensor record");shellVP=new NRPG::CDynamicSkill(record->nMaxVP,record->nMaxVP);listener->SetPanzerklein(record,shellVP,nullptr);expected=100;}else{source=nullptr;expected=mode==5?0:100;if(mode==5){listener->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_DEAF,3));Require(listener->HasCritical(NDb::C_DEAF),"hearing cannot establish actual deaf critical");}}}
    const int hp=listener->GetTotalVP(),ap=listener->GetAP();const float progress=unit->Skills(NDb::ST_SPOT).GetProgress();auto random=GlobalGameRandom();bool audible=true;std::printf("native hearing probability input mode=%d persona=%d perk=%d owned=%d parameter=%.9g spot=%d referenceSpot=%d legacySpot=%d sound=%d radius=%d loud=%d source=%d distance=%.9g expected=%d\n",mode,unit->pPers->GetRecordID(),id,int(owned),parameter,spot,referenceSpot,legacySpot,sound.pAISound->GetRecordID(),sound.pAISound->GetRadiusFromAISoundType(0),int(loud),int(bool(source)),distance,expected);std::fflush(stdout);const int actual=listener->GetHearingProbability(source,distance,sound,&audible);
    std::printf("native hearing probability result mode=%d actual=%d expected=%d audible=%d expectedAudible=%d\n",mode,actual,expected,int(audible),int(mode<2));std::fflush(stdout);Require(actual==expected && audible==(mode<2),"hearing rounded SPOT, applied sensor perk outside PK or mishandled an environmental source");Require(listener->GetTotalVP()==hp && listener->GetAP()==ap && unit->Skills(NDb::ST_SPOT).GetProgress()==progress && random.Get()==GlobalGameRandom().Get(),"hearing probability query trained a skill or changed AP/HP/gameplay RNG");
}

static void NativeHearingPractice(const char* databasePath,int mode)
{
    Require(mode>=0 && mode<4,"unknown native hearing practice mode");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);auto* persona=NDb::GetPers(2);auto make=[&](){CObj<NRPG::CUnit> u=new NRPG::CUnit(persona);for(int i=0;i<NDb::SKILL_TYPE_NUMBERS;++i){auto& c=u->Skills(i);c.FreezeToMax(i!=NDb::ST_VP && i!=NDb::ST_AP);c.SetXPPart(0);c.SetProgress(0);c.Reset();}u->UpdateSkills();u->fXP=1.f;return u;};auto unit=make(),reference=make();CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(unit),source=NRPG::CreateUnit(NDb::GetPers(103));source->GetRPGUnit()->Skills(NDb::ST_STEALTH).SetConst(0);const bool loud=mode==0 || mode==3,heard=mode<2,train=mode==1;NDb::SAISound sound={NativeHearingSound(mission,loud),0,1.f};const float distance=mode==3?10000.f:0.f;bool quiet=false;const int probability=mission->GetHearingProbability(source,distance*1.6f,sound,&quiet);Require(quiet==!loud && probability>=0 && probability<=100,"hearing practice lacks actual quiet/loud branch");unsigned seed=0;CRandomGenerator rng;for(seed=1;seed<10000;++seed){rng.SeedForHarness(seed);const int roll=rng.Get(99);if((probability>roll)==heard)break;}Require(seed<10000,"hearing practice cannot isolate actual success/miss roll");if(train)reference->UseSkill(NDb::ST_SPOT,0);GlobalGameRandom().SeedForHarness(seed);auto expectedRNG=GlobalGameRandom();const unsigned roll=expectedRNG.Get(99);const int ap=mission->GetAP(),hp=mission->GetTotalVP();const float xp=unit->fXP;const bool actual=mission->CanHearSound(CVec3(distance,0,0),VNULL3,sound,source);auto actualRNG=GlobalGameRandom();bool sameRNG=true,cells=true;for(int i=0;i<4;++i)sameRNG&=actualRNG.Get()==expectedRNG.Get();for(int i=0;i<NDb::SKILL_TYPE_NUMBERS;++i){auto& a=unit->Skills(i);auto& b=reference->Skills(i);cells&=a.GetTheoreticalMax()==b.GetTheoreticalMax() && a.GetXPPart()==b.GetXPPart() && a.GetMaxValue()==b.GetMaxValue() && a.GetProgress()==b.GetProgress() && int(a)==int(b);}
    std::printf("native hearing practice mode=%d sound=%d loud=%d probability=%d seed=%u roll=%u heard=%d expectedHeard=%d train=%d spotProgress=%.9g expectedSpot=%.9g intProgress=%.9g expectedINT=%.9g cells=%d RNG=%d XP=%.9g\n",mode,sound.pAISound->GetRecordID(),int(loud),probability,seed,roll,int(actual),int(heard),int(train),unit->Skills(NDb::ST_SPOT).GetProgress(),reference->Skills(NDb::ST_SPOT).GetProgress(),unit->Skills(NDb::ST_INT).GetProgress(),reference->Skills(NDb::ST_INT).GetProgress(),int(cells),int(sameRNG),xp);std::fflush(stdout);Require(actual==heard && cells,"guaranteed loud hearing trained SPOT/INT or quiet hearing lost its native practice");Require(sameRNG && mission->GetAP()==ap && mission->GetTotalVP()==hp && unit->fXP==xp,"hearing practice changed d99 continuation/AP/HP/XP pool");
}

static void NativeMovementNoise(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<4,"unknown native movement noise mode");CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;for(auto u:units){auto* s=dynamic_cast<NWorld::CUnitServer*>(u.GetPtr());if(s && s->CanFight() && !s->IsAIUnit() && !s->IsEmptyPK() && !s->IsWearingPK()){actor=s;break;}}Require(actor,"movement noise lacks actual human producer");auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),3.f),&places);NAI::SPathPlace point;bool found=false;for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetDirection(0);p.SetMoving(0);if(network->IsPassable(p) && !network->IsLocked(p)){point=p;found=true;break;}}Require(found,"movement noise lacks actual free listener place");CObj<NWorld::CUnitServer> listener=world->AddUnitInGame(point,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer(),"NativeMovementNoiseListener");Require(listener && listener->CanFight(),"movement noise cannot create actual listener");auto initial=actor->GetPosition();initial.SetPose(mode==1 || mode==2?NAI::WALK:NAI::CRAWL);actor->SetPosition(initial);world->UpdateVisible();const NDb::SAISound prime={NativeHearingSound(listener->GetUnitRPG(),true),0,1.f};world->MakeAISound(prime,actor);Require(listener->IsAudible(actor),"movement noise lacks actual prior loud-hearing control");listener->ClearSound(actor);listener->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_SPOT).SetProgress(0);listener->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_INT).SetProgress(0);Require(listener->GetSounds()->empty() && listener->IsAudible(actor),"movement noise did not retain its actual known-audible state while clearing the old event");auto target=initial;if(mode==2)target.SetPose(NAI::CRAWL);if(mode==3)target.pos.p.SetDirection((initial.GetDir()+1)%8);const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();const auto time=world->GetTime()->GetValue();actor->SetPosition(target);int heard=0,objects=0;for(const auto& sound:*listener->GetSounds())if(sound.pWho==actor){++heard;objects+=int(sound.objects.size());}const int expected=mode==1?1:0;
    std::printf("native movement noise mode=%d beforePose=%d afterPose=%d beforeDir=%d afterDir=%d heard=%d expected=%d objects=%d AP=%d afterAP=%d HP=%d afterHP=%d\n",mode,int(initial.GetPose()),int(target.GetPose()),int(initial.GetDir()),int(target.GetDir()),heard,expected,objects,ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP());std::fflush(stdout);Require(actor->GetPosition()==target && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && world->GetTime()->GetValue()==time,"movement noise changed target/AP/HP/world time");Require(heard==expected,"crawling SetPosition emitted a movement noise event or walking lost its ordinary event");
}

struct NativeHearingOwners
{
    CObj<NRPG::CGlobalGame> game;
    CObj<NWorld::CWorld> world;
    NWorld::CPlayer* human=nullptr;
    NWorld::CPlayer* ai=nullptr;
    NWorld::CUnitServer* humanUnit=nullptr;
    NWorld::CUnitServer* aiUnit=nullptr;
    NativeHearingOwners(const char* db,const char* res)
    {
        world=FixtureWorld(db,res,true,&game);
        vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
        for(auto u:units){auto* s=dynamic_cast<NWorld::CUnitServer*>(u.GetPtr());
            if(!s || !s->CanFight() || s->IsEmptyPK() || s->IsWearingPK())continue;
            if(s->IsAIUnit() && !aiUnit){aiUnit=s;ai=dynamic_cast<NWorld::CPlayer*>(s->GetPlayer());}
            if(!s->IsAIUnit() && !humanUnit){humanUnit=s;human=dynamic_cast<NWorld::CPlayer*>(s->GetPlayer());}}
        Require(human && ai && humanUnit && aiUnit && human!=ai,"heard AI lacks actual deployed human and scenario AI owners");
    }
    NAI::SPathPlace FreePlace(const CVec3& center,float radius)
    {
        auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;
        network->GetNearPlaces(SSphere(center,radius),&places);
        for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetMoving(0);p.SetDirection(0);
            if(network->IsPassable(p) && !network->IsLocked(p))return p;}
        Require(false,"heard AI has no actual free standing place");return NAI::SPathPlace();
    }
};

static void NativeRPGOwnershipAI(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<11,"unknown native RPG ownership AI mode");
    NativeHearingOwners f(databasePath,resourcePath);CObj<NWorld::CUnitServer> held;
    NWorld::CUnitServer* actor=mode==0?f.aiUnit:mode==1?f.humanUnit:nullptr;
    if(!actor){const bool initialAI=mode==2 || mode==5 || mode==6 || mode==7 || mode==10;
        held=f.world->AddUnitInGame(f.FreePlace(f.humanUnit->GetPosition().GetCP(),20.f),NRPG::CreateUnit(NDb::GetPers(54)),initialAI?f.ai:f.human,"NativeOwnershipAI");actor=held;}
    Require(actor && actor->CanFight(),"ownership AI cannot create actual native actor");
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();const auto pos=actor->GetPosition();
    auto* identity=actor->GetUnitRPG()->GetRPGUnit();const auto time=f.world->GetTime()->GetValue();
    if(mode==4)f.world->ChangeUnitPlayer(actor,f.ai);
    if(mode==5)f.world->ChangeUnitPlayer(actor,f.human);
    if(mode==6)actor->SetPlayer(nullptr);
    if(mode==7)f.ai->SetCommander(new NAI::CSequenceCommander(f.world));
    if(mode==8 || mode==9)f.human->SetCommander(new NAI::CAICommander(f.world,f.human));
    if(mode==9)actor->SetPlayer(f.human);
    const bool expected=mode==0 || mode==2 || mode==4 || mode==7 || mode==9 || mode==10;
    const bool actual=actor->GetUnitRPG()->IsAIPlayer();bool saved=actual;
    if(mode==10){CStructureNetworkContext refs;auto bytes=BorrowedObjectFields(actor->GetUnitRPG(),&refs);
        CStructureSaver reader(bytes,CStructureSaver::READ,&refs);reader.Add(46,&saved);}
    std::printf("native RPG ownership AI mode=%d persona=%d commanderAI=%d RPG=%d expected=%d saved46=%d AP=%d afterAP=%d HP=%d afterHP=%d identity=%d position=%d time=%d\n",mode,identity->pPers->GetRecordID(),int(actor->IsAIUnit()),int(actual),int(expected),int(saved),ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP(),int(identity==actor->GetUnitRPG()->GetRPGUnit()),int(pos==actor->GetPosition()),int(time==f.world->GetTime()->GetValue()));std::fflush(stdout);
    Require(actual==expected && saved==expected,"native owner assignment did not publish or retain the saved RPG AI flag");
    Require(ap==actor->GetAP() && hp==actor->GetUnitRPG()->GetTotalVP() && pos==actor->GetPosition() && identity==actor->GetUnitRPG()->GetRPGUnit() && time==f.world->GetTime()->GetValue(),"ownership AI changed AP/HP/position/identity/time");
}

static void NativeRPGOwnershipDamage(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<10,"unknown native RPG ownership damage mode");NativeHearingOwners f(databasePath,resourcePath);
    NWorld::CUnitServer* actor=mode==0?f.aiUnit:mode==1?f.humanUnit:nullptr;CObj<NWorld::CUnitServer> held;
    if(!actor){const bool initialAI=mode==2 || mode==5 || mode==7 || mode==9;
        held=f.world->AddUnitInGame(f.FreePlace(f.humanUnit->GetPosition().GetCP(),20.f),NRPG::CreateUnit(NDb::GetPers(54)),initialAI?f.ai:f.human,"NativeOwnershipDamage");actor=held;}
    if(mode==4)f.world->ChangeUnitPlayer(actor,f.ai);
    if(mode==5)f.world->ChangeUnitPlayer(actor,f.human);
    if(mode==7)f.ai->SetCommander(new NAI::CSequenceCommander(f.world));
    if(mode==8)f.human->SetCommander(new NAI::CAICommander(f.world,f.human));
    NDb::CDBDifficulty* authored=nullptr;CDBIterator<NDb::CDBDifficulty> records(*NDatabase::GetTable<NDb::CDBDifficulty>());
    while(records.MoveNext()){auto* d=records.Get();if(d->fEnemyDamageMult>0 && d->fOurDamageMult>0 && d->fEnemyDamageMult!=d->fOurDamageMult && (!authored || d->GetRecordID()<authored->GetRecordID()))authored=d;}
    Require(authored,"ownership damage lacks authored distinct enemy/our coefficients");f.game->pDifficulty=authored;
    auto* armor=f.humanUnit->GetUnitRPG()->GetRPGArmor();Require(armor && armor->pMaterial && armor->pMaterial->GetRecordID()==NDb::CRPGMaterial::HUMAN_BODY,"ownership damage lacks actual human flesh armor");
    int type=-1;CDBIterator<NDb::CRPGDmgToArmor> types(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());
    while(types.MoveNext())if(types.Get()->armors[armor->pMaterial->nDR]>0 && (type<0 || types.Get()->GetRecordID()<type))type=types.Get()->GetRecordID();
    Require(type>=0,"ownership damage lacks authored positive damage type");
    const bool anonymous=mode==6 || mode==9,expectedAI=mode==0 || mode==2 || mode==4 || mode==7;
    NRPG::CAttackPortion attack(100000,type,0.f,100,100,0,0,anonymous?nullptr:actor->GetUnitRPG(),f.humanUnit->GetUnitRPG(),1.f,-1);
    const int base=attack.CalcStructDmg(nullptr,armor,0);Require(base==100,"ownership damage lacks literal fixed-damage reference");
    const float factor=anonymous?1.f:expectedAI?authored->fEnemyDamageMult:authored->fOurDamageMult;
    const int expected=int(std::nearbyint(float(base)*factor));const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),targetHP=f.humanUnit->GetUnitRPG()->GetTotalVP();const auto pos=actor->GetPosition();auto random=GlobalGameRandom();
    const int actual=attack.CalcStructDmg(f.world,armor,0);auto afterRandom=GlobalGameRandom();bool sameRNG=true;for(int i=0;i<4;++i)sameRNG&=random.Get()==afterRandom.Get();
    std::printf("native RPG ownership damage mode=%d persona=%d difficulty=%d enemy=%.9g ours=%.9g RPG=%d commanderAI=%d anonymous=%d base=%d factor=%.9g actual=%d expected=%d AP=%d afterAP=%d HP=%d afterHP=%d targetHP=%d afterTargetHP=%d RNG=%d position=%d\n",mode,actor->GetUnitRPG()->GetRPGPersID(),authored->GetRecordID(),authored->fEnemyDamageMult,authored->fOurDamageMult,int(actor->GetUnitRPG()->IsAIPlayer()),int(actor->IsAIUnit()),int(anonymous),base,factor,actual,expected,ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP(),targetHP,f.humanUnit->GetUnitRPG()->GetTotalVP(),int(sameRNG),int(pos==actor->GetPosition()));std::fflush(stdout);
    Require(actual==expected,"native structural damage used our difficulty multiplier for a saved AI-owned attacker");
    Require(sameRNG && ap==actor->GetAP() && hp==actor->GetUnitRPG()->GetTotalVP() && targetHP==f.humanUnit->GetUnitRPG()->GetTotalVP() && pos==actor->GetPosition(),"ownership damage query changed combat state/RNG");
}

static void NativeHeardAICamera(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<12,"unknown native heard AI camera mode");NativeHearingOwners f(databasePath,resourcePath);
    vector<CPtr<NWorld::CUnit>> prior;f.world->GetAllUnits(&prior);
    for(auto u:prior){auto* s=dynamic_cast<NWorld::CUnitServer*>(u.GetPtr());if(s && s->CanFight())s->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_DEAF,3));}
    auto* network=f.world->GetPathNetwork();const auto first=f.FreePlace(f.humanUnit->GetPosition().GetCP(),3.f);
    const bool humanSource=mode==3,AIListener=mode==4;
    CObj<NWorld::CUnitServer> source=f.world->AddUnitInGame(first,NRPG::CreateUnit(NDb::GetPers(103)),humanSource?f.human:f.ai,"NativeHeardAISource");
    Require(source,"heard AI cannot create actual source");
    const auto second=f.FreePlace(source->GetPosition().GetCP(),3.f);
    CObj<NWorld::CUnitServer> listener=f.world->AddUnitInGame(second,NRPG::CreateUnit(NDb::GetPers(54)),AIListener?f.ai:f.human,"NativeHeardAIListener");
    Require(listener && listener->CanFight(),"heard AI cannot create actual listener");
    CObj<NWorld::CUnitServer> extra;
    if(mode==9)extra=f.world->AddUnitInGame(f.FreePlace(source->GetPosition().GetCP(),3.f),NRPG::CreateUnit(NDb::GetPers(54)),f.human,"NativeHeardAIExtraListener");
    if(mode!=5){listener->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_BLIND,3));if(extra)extra->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_BLIND,3));}
    else listener->GetUnitRPG()->GetRPGUnit()->SetCheat(NRPG::CHEAT_SEEALL,true);
    if(mode==6)listener->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_DEAF,3));
    if(mode!=2)f.world->GivePlayerTurn(f.human);
    f.world->UpdateVisible();
    // Use the real ownerless sequence state; omitting GivePlayerTurn does not clear a prior turn.
    if(mode==2)f.world->StartSequence();
    Require(f.world->IsTurnBased()==(mode!=2) && (mode!=2 || f.world->IsSequence()),"heard AI lacks actual owned-turn/ownerless-sequence control");
    auto isVisible=[&](NWorld::CUnitServer* u){const auto& v=u->GetTBSVisible();return std::find(v.begin(),v.end(),source.GetPtr())!=v.end();};
    Require(isVisible(listener)==(mode==5),"heard AI lacks actual invisible/see-all visibility control");
    if(extra)Require(!isVisible(extra),"heard AI extra actual blind listener unexpectedly sees source");
    NDb::SAISound sound={NativeHearingSound(listener->GetUnitRPG(),true),0,1.f};
    Require(fabs(source->GetPosition().GetCP()-listener->GetPosition().GetCP())*1.6f < sound.pAISound->GetRadiusFromAISoundType(0),"heard AI source falls outside actual loud hearing radius");
    auto drain=[&](){int cameras=0;while(auto* p=f.world->GetUICommand()){CObj<NWorld::CUICmd> c=p;if(dynamic_cast<NWorld::CUICmdPointCamera*>(p))++cameras;}return cameras;};
    drain();if(mode==1){f.world->MakeAISound(sound,source);Require(listener->IsAudible(source),"heard AI failed to prime actual already-audible sound");drain();listener->ClearSound(source);}
    if(mode==10)f.ai->SetCommander(new NAI::CSequenceCommander(f.world));
    if(mode==11)f.human->SetCommander(new NAI::CAICommander(f.world,f.human));
    if(mode==7)sound.pAISound=nullptr;
    if(mode==8){source->GetUnitRPG()->GetRPGUnit()->Kill();source->SyncConscious();Require(!source->CanFight(),"heard AI cannot establish actual dead-source control");drain();}
    const int ap=listener->GetAP(),hp=listener->GetUnitRPG()->GetTotalVP();const auto pos=source->GetPosition();const auto time=f.world->GetTime()->GetValue();
    const bool priorAudible=listener->IsAudible(source);f.world->MakeAISound(sound,source);
    int cameras=0,payload=0,other=0;while(auto* p=f.world->GetUICommand()){CObj<NWorld::CUICmd> c=p;auto* camera=dynamic_cast<NWorld::CUICmdPointCamera*>(p);if(!camera){++other;continue;}++cameras;
        payload+=int(camera->GetPriority()==NWorld::PR_UNIT_SOUND && camera->ptExplosion==pos.GetCP() && !camera->bUseSloMo && camera->fSloMoIncrProbability==0.f && camera->bNoZoom && camera->nExecTime==0 && camera->nFloor==pos.pos.GetFloor());}
    const int expected=mode==0 || mode==1 || mode==10 || mode==11?1:mode==9?2:0;
    const bool heard=mode!=6 && mode!=7 && mode!=8;int events=0;for(const auto& s:*listener->GetSounds())events+=int(s.pWho==source);
    std::printf("native heard AI camera mode=%d TBS=%d sourceRPG=%d listenerRPG=%d sourceCommanderAI=%d listenerCommanderAI=%d visible=%d priorAudible=%d audible=%d heardEvents=%d expectedHeard=%d cameras=%d expected=%d payload=%d other=%d AP=%d afterAP=%d HP=%d afterHP=%d floor=%d\n",mode,int(f.world->IsTurnBased()),int(source->GetUnitRPG()->IsAIPlayer()),int(listener->GetUnitRPG()->IsAIPlayer()),int(source->IsAIUnit()),int(listener->IsAIUnit()),int(isVisible(listener)),int(priorAudible),int(listener->IsAudible(source)),events,int(heard),cameras,expected,payload,other,ap,listener->GetAP(),hp,listener->GetUnitRPG()->GetTotalVP(),pos.pos.GetFloor());std::fflush(stdout);
    Require(events==int(heard) && listener->IsAudible(source)==(heard || priorAudible),"heard AI changed real hearing event/audible controls");
    Require(cameras==expected && payload==expected && other==0,"heard AI failed to queue exact point/floor/priority camera payload for an unseen AI source");
    Require(ap==listener->GetAP() && hp==listener->GetUnitRPG()->GetTotalVP() && pos==source->GetPosition() && time==f.world->GetTime()->GetValue(),"heard AI focus changed AP/HP/position/world time");
}

class NativeCoverFramingMap:public CoverOriginMapProbe
{
    OBJECT_NOCOPY_METHODS(NativeCoverFramingMap);
public:
    CVec3 expectedDirection,farPoint;
    int requested=-99,locators=0,grids=0,farInside=0;
    float axisError=0;
    explicit NativeCoverFramingMap(NAI::IAIMap* m=nullptr):CoverOriginMapProbe(m){}
    bool GetUnitHLPos(CVec3* out,CObjectBase* hull,int id) override
    {
        requested=id;++locators;return native->GetUnitHLPos(out,hull,id);
    }
    void TraceGrid(NAI::CFastRenderer* grid,int mask,ESort sort,const NAI::CFloorsSet& floors,ESplitTerrainHGroups split,bool doors) override
    {
        CVec3 axis;grid->GetDir(&axis,grid->resGrid.GetXSize()*.5f-.5f,grid->resGrid.GetYSize()*.5f-.5f);
        axisError=Max(axisError,fabs(axis-expectedDirection));++grids;farInside+=int(grid->TestSphere(farPoint,.001f));
        CoverOriginMapProbe::TraceGrid(grid,mask,sort,floors,split,doors);
    }
};

static NRPG::CAttackPortion NativeCoverFixedAttack(NWorld::CUnitServer* actor,NWorld::CUnitServer* target)
{
    auto* armor=target->GetUnitRPG()->GetRPGArmor();Require(armor && armor->pMaterial,"cover policy lacks actual flesh material");
    int type=-1;CDBIterator<NDb::CRPGDmgToArmor> types(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());
    while(types.MoveNext())if(types.Get()->armors[armor->pMaterial->nDR]>0 && (type<0 || types.Get()->GetRecordID()<type))type=types.Get()->GetRecordID();
    Require(type>=0,"cover policy lacks authored positive damage type");return NRPG::CAttackPortion(100000,type,0.f,1,1,0,0,actor->GetUnitRPG(),target->GetUnitRPG(),1.f,-1);
}

static void NativeCoverFramingPolicy(const char* databasePath,const char* resourcePath,int mode,bool extent)
{
    Require(mode>=0 && mode<(extent?6:8),"unknown native cover framing policy mode");NativeHearingOwners f(databasePath,resourcePath);
    auto* actor=f.humanUnit;auto* target=f.aiUnit;auto* game=f.world->GetGame();auto* map=f.world->GetAIMap();map->Sync();
    CVec3 whole;Require(map->GetUnitHLPos(&whole,map->GetHull(target),NAI::HL_ANY),"cover framing lacks actual whole-hull locator");
    const int parts[]={NAI::HL_ANY,NAI::HL_BODY,NAI::HL_HEAD,NAI::HL_RHAND,NAI::HL_LHAND,NAI::HL_RLEG,NAI::HL_LLEG,NAI::HL_ANY};
    const bool tile=extent && mode>=4,human=extent?mode==1 || mode==3:mode%2;
    const int part=extent?NAI::HL_ANY:parts[mode];const float distance=extent && (mode==2 || mode==3 || mode==5)?40.f:8.f;
    const CVec3 aim=tile?CVec3(0,distance,0):whole,source=aim-CVec3(0,distance,0);
    CObj<NativeCoverFramingMap> probe=new NativeCoverFramingMap(map);probe->expectedDirection=CVec3(0,1,0);probe->farPoint=source+CVec3(0,distance+1.f,0);
    struct Restore{NRPG::IGame* game;CPtr<NAI::IAIMap> map;~Restore(){SetCoverProbeMap(game,map);}}restore{game,map};SetCoverProbeMap(game,probe);
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),targetHP=target->GetUnitRPG()->GetTotalVP();const auto actorPos=actor->GetPosition(),targetPos=target->GetPosition();const auto time=f.world->GetTime()->GetValue();auto random=GlobalGameRandom();
    auto attack=NativeCoverFixedAttack(actor,target);CObj<NRPG::CCoverInfo> cover=tile?game->CalcCoversForTile(source,attack,actor,aim,0.f):game->CalcCovers(source,attack,actor,target,part,1.f,!human);
    auto afterRandom=GlobalGameRandom();bool sameRNG=true;for(int i=0;i<4;++i)sameRNG&=random.Get()==afterRandom.Get();const int expectedGrids=tile || !human?1:2;
    std::printf("native cover framing policy mode=%d extent=%d tile=%d human=%d requestedPart=%d actualLocator=%d locators=%d distance=%.9g grids=%d expectedGrids=%d farInside=%d axisError=%.9g AP=%d afterAP=%d HP=%d afterHP=%d targetHP=%d afterTargetHP=%d RNG=%d\n",mode,int(extent),int(tile),int(human),part,probe->requested,probe->locators,distance,probe->grids,expectedGrids,probe->farInside,probe->axisError,ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP(),targetHP,target->GetUnitRPG()->GetTotalVP(),int(sameRNG));std::fflush(stdout);
    Require(cover && probe->grids==expectedGrids && probe->locators==(tile?0:1),"cover framing did not traverse actual producer/map/grid boundaries");
    Require(sameRNG && ap==actor->GetAP() && hp==actor->GetUnitRPG()->GetTotalVP() && targetHP==target->GetUnitRPG()->GetTotalVP() && actorPos==actor->GetPosition() && targetPos==target->GetPosition() && time==f.world->GetTime()->GetValue(),"cover framing query changed combat inputs/time/RNG");
    if(extent)Require(probe->farInside==expectedGrids,"native cover frustum stopped at the target and excluded geometry just beyond it");
    else Require(probe->requested==NAI::HL_ANY && probe->axisError<.00001f,"native cover projection centered a selected body part instead of the whole hull");
}

static void NativeCoverRangePolicy(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<8,"unknown native cover range policy mode");NativeHearingOwners f(databasePath,resourcePath);auto* actor=f.humanUnit;auto* target=f.aiUnit;
    const bool tile=mode>=3 && mode<=6,human=mode==0 || mode==2 || mode==7;const float distance=mode==0 || mode==1 || mode==4 || mode==5?40.f:8.f;
    auto* map=f.world->GetAIMap();map->Sync();CVec3 whole;Require(map->GetUnitHLPos(&whole,map->GetHull(target),NAI::HL_ANY),"cover range lacks actual hull locator");
    const CVec3 aim=tile?CVec3(0,distance,0):whole,source=aim-CVec3(0,distance,0);CObj<NWorld::CUnitServer> ally;
    if(tile)ally=f.world->AddUnitInGame(f.FreePlace(actor->GetPosition().GetCP(),10.f),NRPG::CreateUnit(NDb::GetPers(54)),f.human,"NativeCoverClearanceAlly");
    Require(!tile || (ally && actor->GetDiplomacyState(ally)==NDb::DS_ALLY),"cover range lacks real separate ally identity");
    CObj<CoverProjectionMapProbe> probe=new CoverProjectionMapProbe(map);probe->source=source;probe->cutoff=30.f;
    float front=distance-.5f,thickness=1.f;
    if(tile){front=mode==3?7.5f:mode==4?31.f:mode==5?29.f:7.f;thickness=.125f;}
    probe->Box(tile?ally.GetPtr():target,source+CVec3(-.3f,front,-.3f),source+CVec3(.3f,front+thickness,.3f));
    auto attack=NativeCoverFixedAttack(actor,target);Require(attack.CanDealDmg(probe->surface.pArmor) && !attack.IsArmorIgnored(probe->surface.pArmor),"cover range has no actual eligible material");
    auto* game=f.world->GetGame();struct Restore{NRPG::IGame* game;CPtr<NAI::IAIMap> map;~Restore(){SetCoverProbeMap(game,map);}}restore{game,map};SetCoverProbeMap(game,probe);
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),targetHP=target->GetUnitRPG()->GetTotalVP(),allyHP=ally?ally->GetUnitRPG()->GetTotalVP():0;const auto pos=actor->GetPosition();auto random=GlobalGameRandom();
    CObj<NRPG::CCoverInfo> cover=tile?game->CalcCoversForTile(source,attack,actor,aim,0.f):game->CalcCovers(source,attack,actor,target,NAI::HL_ANY,1.f,!human);
    const int score=NRPG::GetHitCover(actor,cover);const bool expectedPositive=mode==2 || mode==3 || mode==4 || mode==7;auto afterRandom=GlobalGameRandom();bool sameRNG=true;for(int i=0;i<4;++i)sameRNG&=random.Get()==afterRandom.Get();
    std::printf("native cover range policy mode=%d tile=%d human=%d distance=%.9g front=%.9g thickness=%.9g score=%d expectedPositive=%d intersections=%d grids=%zu AP=%d afterAP=%d HP=%d afterHP=%d targetHP=%d afterTargetHP=%d allyHP=%d afterAllyHP=%d RNG=%d\n",mode,int(tile),int(human),distance,front,thickness,score,int(expectedPositive),probe->intersections,probe->traces.size(),ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP(),targetHP,target->GetUnitRPG()->GetTotalVP(),allyHP,ally?ally->GetUnitRPG()->GetTotalVP():0,int(sameRNG));std::fflush(stdout);
    Require(cover && probe->intersections>0 && probe->traces.size()==(human?2u:1u),"cover range did not rasterize actual closed-box geometry");
    Require(sameRNG && ap==actor->GetAP() && hp==actor->GetUnitRPG()->GetTotalVP() && targetHP==target->GetUnitRPG()->GetTotalVP() && allyHP==(ally?ally->GetUnitRPG()->GetTotalVP():0) && pos==actor->GetPosition(),"cover range query changed native combat state/RNG");
    Require((score>0)==expectedPositive,"native cover range retained an uncapped human ray or the old tile pullback");
}

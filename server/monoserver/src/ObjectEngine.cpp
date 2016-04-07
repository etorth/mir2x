#include "stdafx.h"

#define _RANGE_X			12
#define _RANGE_Y			12

#define _CACHE_TICK			500

#define HEALTHFILLTICK		1500
#define SPELLFILLTICK		800

CMagicInfo*		GetMagicInfo(int nMagicID);
CMirMap*		GetMap(char *pszMapName);

MonsterObject* AddCreature(CMirMap* pMap, int nX, int nY, int nMonRace, bool fSearch);

CharObject::CharObject(CUserInfo*	pUserInfo)
{ 
//	InitializeCriticalSection(&m_cs);

	m_pUserInfo					= pUserInfo;

	m_nCurrX					= 0;
	m_nCurrY					= 0;

	ZeroMemory(m_szName, sizeof(m_szName));

	m_dwHealthTick				= 0;
	m_dwSpellTick				= 0;
	m_dwTickSec					= 0;

	m_fOpenHealth				= false;

	m_nCharStatusEx				= 0;
	m_nCharStatus				= 0;

	m_IncHealing				= 0;
	m_IncHealth					= 0;
	m_IncSpell					= 0;

	m_btPerHealing				= 5;
	m_btPerHealth				= 5;
	m_btPerSpell				= 5;

	m_dwIncHealthSpellTime		= 0;

	m_dwTickSec					= GetTickCount();

	m_nEvent					= -1; // No Event

	ZeroMemory(m_StateAttrV, sizeof(m_StateAttrV));
	ZeroMemory(m_szName, sizeof(m_szName));

	m_sHitSpeed					= 0;

	m_fAbilMagicBubbleDefence		= false;
	m_btMagicBubbleDefenceLevel	= 0;

	m_fInspector				= false;
	m_fFastTrain				= false;

	m_IncHealth					= 0;
	m_IncSpell					= 0;

	m_fIsGhost					= false;
	m_fIsNeverDie				= false;

	m_btSpeedPoint				= DEFSPEED;

	m_fHumHideMode				= false;
	m_fFixedHideMode			= false;

	m_fStoneMode				= false;
	m_fStickMode				= false;

	m_pMasterObject				= NULL;
	m_pTargetObject				= NULL;
}

CharObject::~CharObject()
{
	PLISTNODE		pListNode = m_xVisibleObjectList.GetHead();

	while (pListNode)
	{
		CVisibleObject* pVisibleObject = m_xVisibleObjectList.GetData(pListNode);

		if(pVisibleObject) 
		{
			delete pVisibleObject;
			pVisibleObject = NULL;
		}

		pListNode = m_xVisibleObjectList.RemoveNode(pListNode);
	} // while (pListNode)

//	DeleteCriticalSection(&m_cs);
}

void CharObject::SelectTarget(CharObject* pTargetObject)
{
	m_pTargetObject			= pTargetObject;
	m_dwTargetFocusTime		= GetTickCount();
}

void CharObject::GetFrontPosition(int &nX, int &nY)
{
	int nHeight = m_pMap->m_stMapFH.shHeight;
	int nWidth	= m_pMap->m_stMapFH.shWidth;

	nX = m_nCurrX;
	nY = m_nCurrY;

	switch (m_nDirection)
	{
		case DR_UP:
			if(nY > 0) nY--;
			break;
		case DR_DOWN:
			if(nY < nHeight - 1) nY++;
			break;
		case DR_LEFT:
			if(nX > 0) nX--;
			break;
		case DR_RIGHT:
			if(nX < nWidth - 1) nX++;
			break;
		case DR_UPLEFT:
		{
            if(nX > 0 && nY > 0)
			{
               nX--;
               nY--;
            }

			break;
		}
		case DR_UPRIGHT:
		{
            if(nX > 0 && nY < nHeight - 1)
			{
               nX++;
               nY--;
			}

			break;
		}
		case DR_DOWNLEFT:
		{
            if(nX < nWidth - 1 && nY > 0)
			{
               nX--;
               nY++;
            }

			break;
		}
		case DR_DOWNRIGHT:
		{
            if(nX < nWidth - 1 && nY < nHeight - 1)
			{
               nX++;
               nY++;
            }

			break;
		}
	}
}

int CharObject::GetBack(int nDirection)
{
	switch (nDirection)
	{
		case DR_UP:
			return DR_DOWN;
		case DR_DOWN:
			return DR_UP;
		case DR_LEFT:
			return DR_RIGHT;
		case DR_RIGHT:
			return DR_LEFT;
		case DR_UPLEFT:
			return DR_DOWNRIGHT;
		case DR_UPRIGHT:
			return DR_DOWNLEFT;
		case DR_DOWNLEFT:
			return DR_UPRIGHT;
		case DR_DOWNRIGHT:
			return DR_UPLEFT;
	}

	return nDirection;
}

bool CharObject::GetBackPosition(int &nX, int &nY)
{
	int nHeight = m_pMap->m_stMapFH.shHeight;
	int nWidth	= m_pMap->m_stMapFH.shWidth;

	nX = m_nCurrX;
	nY = m_nCurrY;

	switch (m_nDirection)
	{
		case DR_UP:
			if(nY < nHeight - 1) nY++;
			break;
		case DR_DOWN:
			if(nY > 0) nY--;
			break;
		case DR_LEFT:
			if(nX < nWidth - 1) nX++;
			break;
		case DR_RIGHT:
			if(nX > 0) nX--;
			break;
		case DR_UPLEFT:
			if((nX < nWidth - 1) && (nY < nHeight - 1))
			{
				nX++;
				nY++;
			}

			break;
		case DR_UPRIGHT:
			if((nX < nWidth - 1) && (nY > 0))
			{
				nX--;
				nY++;
			}

			break;
		case DR_DOWNLEFT:
			if((nX > 0) && (nY < nHeight - 1))
			{
				nX++;
				nY--;
			}

			break;
		case DR_DOWNRIGHT:
			if((nX > 0) && (nY > 0))
			{
				nX--;
				nY--;
			}

			break;
	}

	return TRUE;
}

bool CharObject::IsProperTarget(CharObject* pTargetObject)
{
//	if target.BoSysopMode or target.BoStoneMode then  //운영자...
//       Result := false;

	return TRUE;
}

bool CharObject::IsFriend(CharObject* pCharObject)
{
	if(pCharObject->m_wObjectType & _OBJECT_HUMAN)
	{
		if(pCharObject->m_pUserInfo)
		{
			switch (pCharObject->m_pUserInfo->GetAttackMode())
			{
				case HAM_ALL:
					return TRUE;
				case HAM_PEACE:
					return TRUE;
			}
		}
	}
/*   function IsFriend (cret: TCreature): Boolean;
   begin
      Result := false;
      if cret.RaceServer = RC_USERHUMAN then begin  //대상이 사람인 경우만
         //공격형태 설정에 따라 다름
         case HumAttackMode of
            HAM_ALL:  Result := TRUE;
            HAM_PEACE: Result := TRUE;
            HAM_GROUP:
               begin
                  if cret = self then
                     Result := TRUE;
                  if IsGroupMember (cret) then
                     Result := TRUE;
               end;
            HAM_GUILD:
               begin
                  if cret = self then
                     Result := TRUE;
                  if MyGuild <> nil then begin
                     if TGuild(MyGuild).IsMember(cret.UserName) then
                        Result := TRUE;
                     if BoGuildWarArea and (cret.MyGuild <> nil) then begin  //문파전,공성전 지역에 있음
                        if TGuild(MyGuild).IsAllyGuild(TGuild(cret.MyGuild)) then
                           Result := TRUE;
                     end;
                  end;
               end;
            HAM_PKATTACK:
               begin
                  if cret = self then Result := TRUE;
                  if PKLevel >= 2 then begin //내가 빨갱이
                     if cret.PKLevel < 2 then Result := TRUE;
                  end else begin  //내가 흰둥이
                     if cret.PKLevel >= 2 then Result := TRUE;
                  end;
               end;
         end;
      end;
   end;
*/
	return false;
}

bool CharObject::IsProperFriend(CharObject* pCharObject)
{
	bool fFlag = false;

	if(pCharObject)
	{
		if(m_wObjectType & _OBJECT_ANIMAL)
		{
			if(pCharObject->m_wObjectType & _OBJECT_ANIMAL)
				fFlag = TRUE;

			if(pCharObject->m_pMasterObject)
				fFlag = false;
		}
		else
		{
			if(m_wObjectType & _OBJECT_HUMAN)
			{
				fFlag = IsFriend(pCharObject);

				if(pCharObject->m_wObjectType & _OBJECT_ANIMAL)
				{
					if(pCharObject->m_pMasterObject == this)
						fFlag = TRUE;
					else if(pCharObject->m_pMasterObject)
						fFlag = IsFriend(pCharObject->m_pMasterObject);

				}
			}
			else
				fFlag = TRUE;
		}
	}

	return fFlag;
}

UINT CharObject::GetCharStatus()
{
	UINT s = 0;

	for (int i = 0; i < MAX_STATUS_ATTRIBUTE; i++)
	{
		if(m_StateAttrV[i] > 0)
			s |= (0x80000000 >> i);
	}

	return (s | (m_nCharStatusEx & 0x000FFFFF));
}

bool CharObject::RestoreHealSpell()
{
	if(m_fIsNeverDie)
	{
		m_WAbility.HP = m_WAbility.MaxHP;
		m_WAbility.MP = m_WAbility.MaxMP;
	}

	int n = (GetTickCount() - m_dwTickSec) / 20;  //초당 50
	
	m_dwTickSec = GetTickCount();

	m_dwHealthTick += n;
	m_dwSpellTick  += n;

	if(!m_fIsDead)
	{
		int		nPlus = 0;
		bool 	fHealthSpellChanged = false;

		if(m_WAbility.HP < m_WAbility.MaxHP)
		{
            if(m_dwHealthTick >= HEALTHFILLTICK)
			{
               nPlus = m_WAbility.MaxHP / 15 + 1;
               
				if(m_WAbility.HP + nPlus < m_WAbility.MaxHP)
					m_WAbility.HP = m_WAbility.HP + nPlus;
				else 
					m_WAbility.HP = m_WAbility.MaxHP;

				fHealthSpellChanged = TRUE;
            }
		}

		if(m_WAbility.MP < m_WAbility.MaxMP)
		{
            if(m_dwSpellTick >= SPELLFILLTICK)
			{
               nPlus = m_WAbility.MaxMP / 18 + 1;
               
				if(m_WAbility.MP + nPlus < m_WAbility.MaxMP)
					m_WAbility.MP = m_WAbility.MP + nPlus;
				else 
					m_WAbility.MP = m_WAbility.MaxMP;

				fHealthSpellChanged = TRUE;
            }
		}
	
		if(m_WAbility.HP == 0)
		{
			return TRUE;
		}
/*        if WAbil.HP = 0 then begin
            if BoAbilRevival then begin //재생 능력이 있다.
               if GetTickCount - LatestRevivalTime > 60 * 1000 then begin
                  LatestRevivalTime := GetTickCount;
                  //반지를 닳게 한다.
                  ItemDamageRevivalRing;
                  WAbil.HP := WAbil.MaxHP;
                  HealthSpellChanged;
                  SysMsg ('반지의 힘으로 몸이 재생되었습니다.', 1);
               end;
            end;
            if WAbil.HP = 0 then
               Die;
         end;	*/

		if(fHealthSpellChanged)
			HealthSpellChanged();

		if(m_dwHealthTick >= HEALTHFILLTICK)
			m_dwHealthTick = 0;
		if(m_dwSpellTick >= SPELLFILLTICK)
			m_dwSpellTick = 0;
	}
	else
	{
		if(GetTickCount() - m_dwDeathTime > 3 * 60 * 1000)
			MakeGhost();

		return false;
	}

	if((!m_fIsDead) && (m_IncSpell > 0) || (m_IncHealth > 0) || (m_IncHealing > 0))
	{
		int		nHP = 0, nMP = 0;

		DWORD	inchstime = 600 - _MIN(400, m_Ability.Level * 10);

		if(GetTickCount() - m_dwIncHealthSpellTime >= inchstime)
		{
			n = _MIN(200, (GetTickCount() - m_dwIncHealthSpellTime) - inchstime);

			m_dwIncHealthSpellTime = GetTickCount() + n;

			if((m_IncSpell > 0) || (m_IncHealth > 0) || (m_IncHealing > 0))
			{
				if(m_btPerHealing <= 0) m_btPerHealing = 1;
				if(m_btPerSpell <= 0) m_btPerSpell = 1;
				if(m_btPerHealth <= 0) m_btPerHealth = 1;

				if(m_IncHealth < m_btPerHealth)
				{
					nHP += m_IncHealth;
					m_IncHealth = 0;
				}
				else
				{
					nHP += m_btPerHealth;
					m_IncHealth = m_IncHealth - m_btPerHealth;
				}		

				if(m_IncSpell < m_btPerSpell)
				{
					nMP += m_IncSpell;
					m_IncSpell = 0;
				}
				else
				{
					nHP += m_btPerSpell;
					m_IncSpell = m_IncSpell - m_btPerSpell;
				}		

				if(m_IncHealing < m_btPerHealing)
				{
					nHP += m_IncHealing;
					m_IncHealing = 0;
				}
				else
				{
					nHP += m_btPerHealing;
					m_IncHealing = m_IncHealing - m_btPerHealing;
				}		
			}

			m_btPerHealing	= 5;
			m_btPerHealth	= 5 + (m_Ability.Level / 10);
			m_btPerSpell	= 5 + (m_Ability.Level / 10);

			IncHealthSpell(nHP, nMP);

			if(m_WAbility.HP == m_WAbility.MaxHP)
			{
				m_IncHealth		= 0;
				m_IncHealing	= 0;
			}

			if(m_WAbility.MP == m_WAbility.MaxMP)
				m_IncSpell		= 0;
		}
	}
	else
		m_dwIncHealthSpellTime = GetTickCount();

	if	((int)m_dwHealthTick < -HEALTHFILLTICK)
	{
		if(m_WAbility.HP > 1)
		{
			m_WAbility.HP--;
			m_dwHealthTick += HEALTHFILLTICK;

			HealthSpellChanged();
		}
	}

	return false;
}

/*
procedure TCreature.ScatterGolds;
const
   ONEDROP = 10000;
var
   i, ngold: integer;
begin
   if Gold > 0 then begin
      for i:=0 to 8 do begin
         if Gold > ONEDROP then begin
            ngold := ONEDROP;
            Gold := Gold - ONEDROP;
         end else begin
            ngold := Gold;
            Gold := 0;
         end;
         if ngold > 0 then begin
            if not DropGoldDown (ngold, TRUE) then begin
               Gold := Gold + ngold;
               break;
            end;
         end else break;
      end;
      GoldChanged;
   end;
end;
*/
bool CharObject::DropItemDown(_LPTUSERITEMRCD lpTItemRcd, int nRange, bool fIsGenItem)
{
	CMapItem* xpMapItem = new CMapItem;

	xpMapItem->nCount = 1;

	if(fIsGenItem)
	{
		_LPTGENERALITEMRCD lpTGenItemRcd = NULL;
		
		lpTGenItemRcd = (_LPTGENERALITEMRCD)lpTItemRcd;

		xpMapItem->wLooks		= (WORD)g_pStdItemEtc[lpTGenItemRcd->nStdIndex].dwLooks;
		xpMapItem->btAniCount	= (BYTE)0;
		
		xpMapItem->pItem		= (LONG)lpTGenItemRcd;
		memmove(xpMapItem->szName, g_pStdItemEtc[lpTGenItemRcd->nStdIndex].szName, memlen(g_pStdItemEtc[lpTGenItemRcd->nStdIndex].szName));
	}
	else
	{
		xpMapItem->wLooks		= (WORD)g_pStdItemSpecial[lpTItemRcd->nStdIndex].dwLooks;
		xpMapItem->btAniCount	= (BYTE)g_pStdItemSpecial[lpTItemRcd->nStdIndex].wAniCount;
		
		xpMapItem->pItem		= (LONG)lpTItemRcd;
		
		if(strlen(lpTItemRcd->szPrefixName))
		{
			strcpy(xpMapItem->szName, lpTItemRcd->szPrefixName);
			strcat(xpMapItem->szName, " ");
			strcat(xpMapItem->szName, g_pStdItemSpecial[lpTItemRcd->nStdIndex].szName);
		}
		else
			strcpy(xpMapItem->szName, g_pStdItemSpecial[lpTItemRcd->nStdIndex].szName);
	}

	int nX, nY;

	m_pMap->GetDropPosition(m_nCurrX, m_nCurrY, nRange, nX, nY);
	m_pMap->AddNewObject(nX, nY, OS_ITEMOBJECT, (VOID *)xpMapItem);

	AddRefMsg(RM_ITEMSHOW, xpMapItem->wLooks, (int)xpMapItem, nX, nY, xpMapItem->szName);

	return TRUE;
}
/*
function  TCreature.DropItemDown (ui: TUserItem; scatterrange: integer; diedrop: Boolean): Boolean;
var
   dx, dy, idura: integer;
   pmi, pr: PTMapItem;
   ps: PTStdItem;
   logcap: string;
begin
   Result := false;
   ps := UserEngine.GetStdItem (ui.Index);
   if ps <> nil then begin

      if ps.StdMode = 40 then begin //고기를 땅에 떨어뜨린 경우 고기 품질이 떨어진다.
         idura := ui.Dura; //word이므로.
         idura := idura - 2000; //고기 품질이 떨어진다.
         if idura < 0 then idura := 0;
         ui.Dura := idura;
      end;

      new (pmi);
      pmi.UserItem := ui;
      pmi.Name := ps.Name;
      pmi.Looks := ps.Looks;
      if ps.StdMode = 45 then begin  //주사위, 목재
         pmi.Looks := GetRandomLook (pmi.Looks, ps.Shape);
      end;
      pmi.AniCount := ps.AniCount;
      pmi.Reserved := 0;
      pmi.Count := 1;
      GetDropPosition (CX, CY, scatterrange, dx, dy);
      pr := Penvir.AddToMap (dx, dy, OS_ITEMOBJECT, TObject (pmi));  //한셀에 5개 이상의 아이템이 있으면 실패한다.
      if pr = pmi then begin
         //아이템은 추가 아니면 실패..
         SendRefMsg (RM_ITEMSHOW, pmi.Looks, integer(pmi), dx, dy, pmi.Name);
         //떨어뜨림
         if diedrop then logcap := '떨굼_'
         else logcap := '버림_';
         if not IsCheapStuff (ps.StdMode) then
            AddUserLog (logcap +
                        MapName + '_' +
                        IntToStr(CX) + '_' +
                        IntToStr(CY) + '_' +
                        BoolToStr(RaceServer = RC_USERHUMAN) + '_' +
                        UserName + '_' +
                        UserEngine.GetStdItemName (ui.Index) + '_' +
                        IntToStr(ui.MakeIndex));
         Result := TRUE;
      end else begin
         //실패인경우
         Dispose (pmi);
      end;
   end;
end;
}
*/

void CharObject::Run()
{
	if(m_fOpenHealth)
	{
		if(GetTickCount() - m_dwOpenHealthStart > m_dwOpenHealthTime)
			BreakOpenHealth();
	}

	bool fChg = false;
	bool fNeedRecalc = false;

	for (int i = 0; i < MAX_STATUS_ATTRIBUTE; i++)
	{
		if(m_StateAttrV[i] > 0 && m_StateAttrV[i] < 60000)
		{
			if(GetTickCount() - m_dwStatusTime[i] > 1000)
			{
				m_StateAttrV[i]		-= 1;
				m_dwStatusTime[i]	+= 1000;

				if(m_StateAttrV[i] == 0)
				{
					fChg = TRUE;

					switch (i)
					{
						case STATE_DEFENCEUP:
							fNeedRecalc = TRUE;
							SysMsg("방어력 상승 해제", 1);
							break;
                        case STATE_MAGDEFENCEUP:
							fNeedRecalc = TRUE;
							SysMsg("마항력 상승 해제", 1);
							break;
/*                        case STATE_TRANSPARENT:
                           begin
                              BoHumHideMode := false;
                           end; */
                        case STATE_BUBBLEDEFENCEUP:
							m_fAbilMagicBubbleDefence = false;
							break;
					}
				}
			}
		}
	}

	if(fChg)
	{
		m_nCharStatus = GetCharStatus();
		AddRefMsg(RM_CHARSTATUSCHANGED, m_sHitSpeed/*wparam*/, m_nCharStatus, 0, 0, NULL);
	}

	if(fNeedRecalc)
	{
		((CPlayerObject*)this)->RecalcAbilitys();
		AddProcess(this, RM_ABILITY, 0, 0, 0, 0, NULL);
	}
}

void CharObject::Die()
{
	if(m_fIsNeverDie) return;

	m_IncHealing	= 0;
	m_IncHealth		= 0;
	m_IncSpell		= 0;

	m_dwDeathTime	= GetTickCount();

	if(m_wObjectType & (_OBJECT_ANIMAL | _OBJECT_HUMAN))
		AddRefMsg(RM_DEATH, m_nDirection, m_nCurrX, m_nCurrY, 1, NULL);

	m_fIsDead		= TRUE;
/*
   try
      boPK := false;
      if(not BoVentureServer) and (not PEnvir.FightZone) and (not PEnvir.Fight3Zone) then begin
         //PK금지 구역인 경우
         if(RaceServer = RC_USERHUMAN) and (LastHiter <> nil) and (PKLevel < 2) then begin
            //죽은자가 사람, 때린자 있음, 죽은자가 PK아님
            if LastHiter.RaceServer = RC_USERHUMAN then
               boPK := TRUE;
            if LastHiter.Master <> nil then
               if LastHiter.Master.RaceServer = RC_USERHUMAN then begin
                  LastHiter := LastHiter.Master; //주인이 때린 것으로 간주
                  boPK := TRUE;
               end;
         end;
      end;

      if boPK and (LastHiter <> nil) then begin
         //사람이 죽은 경우, 모험서버에서는 해당안됨
         //사람이 선량한 사람을 죽임.

         //문파전으로 죽음
         guildwarkill := false;
         if(MyGuild <> nil) and (LastHiter.MyGuild <> nil) then begin
            //둘다 문파에 가입된 상태에서
            if GetGuildRelation (self, LastHiter) = 2 then  //문파전 중임
               guildwarkill := TRUE;  //문파전으로 죽음, 빨갱이 안됨
         end;

         //공성전으로 죽음
         if UserCastle.BoCastleUnderAttack then
            if(BoInFreePKArea) or (UserCastle.IsCastleWarArea (PEnvir, CX, CY)) then
               guildwarkill := TRUE;

         if not guildwarkill then begin //문파전으로 죽음
            if not LastHiter.IsGoodKilling(self) then begin
               LastHiter.IncPkPoint (100); //
               LastHiter.SysMsg ('당신은 살인을 하였습니다.', 0);
               //살인한 사람 행운 감소
               LastHiter.AddBodyLuck (-500);
               if PkLevel < 1 then //죽은 사람이 착한 사람
                  if Random(5) = 0 then //살인을 하면 무기가 저주를 받는다.
                     LastHiter.MakeWeaponUnlock;
            end else
               LastHiter.SysMsg ('[정당방어 규칙에 의해 보호됨]', 1);
         end;

      end;
   except
      MainOutMessage ('[Exception] TCreature.Die 2');
   end;

   try
      if(not PEnvir.FightZone) and
         (not PEnvir.Fight3Zone) and
         (not BoAnimal) then begin //동물이면 썰어야 고기가 나온다.
         DropUseItems;
         if(Master = nil) and (not BoNoItem) then
            ScatterBagItems;  //주인있는 몹은 물건을 흘리지 않음
         if(RaceServer >= RC_ANIMAL) and (Master = nil) and (not BoNoItem) then
            ScatterGolds;
         //죽은 경우 행운치 감소
         AddBodyLuck ( - (50 - (50 - Abil.Level * 5)));
      end;

      //문파 대전 중
      if PEnvir.Fight3Zone then begin
         //3번 죽어도 되는 대련인 경우
         Inc (FightZoneDieCount);
         if MyGuild <> nil then begin
            TGuild(MyGuild).TeamFightWhoDead (UserName);
         end;

         //점수 계산
         if(LastHiter <> nil) then begin
            if(LastHiter.MyGuild <> nil) and (MyGuild <> nil) then begin
               TGuild(LastHiter.MyGuild).TeamFightWhoWinPoint (LastHiter.UserName, 100);  //matchpoint 증가, 개인성적 기록
               str := TGuild(LastHiter.MyGuild).GuildName + ':' +
                      IntToStr(TGuild(LastHiter.MyGuild).MatchPoint) + '  ' +
                      TGuild(MyGuild).GuildName + ':' +
                      IntToStr(TGuild(MyGuild).MatchPoint);
               UserEngine.CryCry (RM_CRY, PEnvir, CX, CY, 10000, '- ' + str);  //현맵 전체에게 알린다.
            end;
         end;
      end;

      //로그를 남긴다.
      if RaceServer = RC_USERHUMAN then begin
         if LastHiter <> nil then begin
            if LastHiter.RaceServer = RC_USERHUMAN then str := LastHiter.UserName
            else str := '#' + LastHiter.UserName;
         end else str := '####';
         AddUserLog ('죽음_' +
                     MapName + '_' +
                     IntToStr(CX) + '_' +
                     IntToStr(CY) + '_' +
                     UserName + '_' +
                     str + '_' +
                     'FZ-' + BoolToStr(PEnvir.FightZone) + '_' +
                     'F3-' + BoolToStr(PEnvir.Fight3Zone));
      end;

      SendRefMsg (RM_DEATH, Dir, CX, CY, 1, '');
   except
      MainOutMessage ('[Exception] TCreature.Die 3');
   end;
end;
*/
}

bool CharObject::GetAvailablePosition(CMirMap* pMap, int &nX, int &nY, int nRange)
{
	if(pMap->CanMove(nX, nY))
		return TRUE;

	int nOrgX		= nX;
	int nOrgY		= nY;
	int nLoonCnt	= (4 * nRange) * (nRange + 1);

	for (int i = 0; i < nLoonCnt; i++)
	{
		nX = nOrgX + g_SearchTable[i].x;
		nY = nOrgY + g_SearchTable[i].y;

		if(pMap->CanMove(nX, nY))
			return TRUE;
	}
	
	nX = nOrgX;
	nY = nOrgY;

	return false;
}

bool CharObject::GetNextPosition(int nSX, int nSY, int nDir, int nDistance, int& nX, int& nY)
{
	nX = nSX;
	nY = nSY;

	switch (nDir)
	{
		case DR_UP:
			if(nY > (nDistance - 1)) nY -= nDistance;
			break;
		case DR_DOWN:
			if(nY < m_pMap->m_stMapFH.shHeight - nDistance) nY += nDistance;
			break;
		case DR_LEFT:
			if(nX > (nDistance - 1)) nX -= nDistance;
			break;
		case DR_RIGHT:
			if(nX < m_pMap->m_stMapFH.shWidth - nDistance) nX += nDistance;
			break;
		case DR_UPLEFT:
		{
			if((nX > nDistance - 1) && (nY > nDistance - 1))
			{
				nX -= nDistance;
				nY -= nDistance;
			}

			break;
		}
		case DR_UPRIGHT:
		{
			if((nX > nDistance - 1) && (nY < m_pMap->m_stMapFH.shHeight - nDistance))
			{
				nX += nDistance;
				nY -= nDistance;
			}

			break;
		}
		case DR_DOWNLEFT:
		{
			if((nX < m_pMap->m_stMapFH.shWidth - nDistance) && (nY > nDistance - 1))
			{
				nX -= nDistance;
				nY += nDistance;
			}

			break;
		}
		case DR_DOWNRIGHT:
		{
			if((nX < m_pMap->m_stMapFH.shWidth - nDistance) && (nY < m_pMap->m_stMapFH.shHeight - nDistance))
			{
				nX += nDistance;
				nY += nDistance;
			}

			break;
		}
	}

	if((m_nCurrX == nX) && (m_nCurrY == nY))
		return false;

	return TRUE;
}

int	CharObject::GetNextDirection(int nStartX, int nStartY, int nTargetX, int nTargetY)
{
	int nFlagX, nFlagY;

	if(nStartX < nTargetX) nFlagX = 1;
	else if(nStartX == nTargetX) nFlagX = 0;
	else
		nFlagX = -1;

	if(abs(nStartY - nTargetY) > 2)
	{
		if((nStartY >= nTargetY - 1) && (nStartY <= nTargetY + 1))
			nFlagX = 0;
	}

	if(nStartY < nTargetY) nFlagY = 1;
	else if(nStartY == nTargetY) nFlagY = 0;
	else
		nFlagY = -1;

	if(abs(nStartX - nTargetX) > 2)
	{
		if((nStartX >= nTargetX - 1) && (nStartX <= nTargetX + 1))
			nFlagY = 0;
	}

	if((nFlagX == 0) && (nFlagY == -1)) return DR_UP;
	else if((nFlagX == 1) && (nFlagY == -1)) return DR_UPRIGHT;
	else if((nFlagX == 1) && (nFlagY == 0)) return DR_RIGHT;
	else if((nFlagX == 1) && (nFlagY == 1)) return DR_DOWNRIGHT;
	else if((nFlagX == 0) && (nFlagY == 1)) return DR_DOWN;
	else if((nFlagX == -1) && (nFlagY == 1)) return DR_DOWNLEFT;
	else if((nFlagX == -1) && (nFlagY == 0)) return DR_LEFT;
	else if((nFlagX == -1) && (nFlagY == -1)) return DR_UPLEFT;

	return DR_DOWN;
}

CharObject* CharObject::GetFrontObject()
{
	int nX, nY;

	GetFrontPosition(nX, nY);

	CharObject* pCharObject = m_pMap->GetObject(nX, nY);

	if(pCharObject)
		return pCharObject;

	return NULL;
}

void CharObject::UpdateDelayProcessCheckParam1(
        CharObject* pCharObject,
        WORD wIdent, WORD wParam, 
        DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData, int nDelay)
{
	_LPTPROCESSMSG	lpProcessMsg;

	int nCount = m_DelayProcessQ.GetCount();

	if(nCount)
	{
		for (int i = 0; i < nCount; i++)
		{
			lpProcessMsg = (_LPTPROCESSMSG)m_DelayProcessQ.PopQ();

			if(lpProcessMsg)
			{
				if(lpProcessMsg->wIdent == wIdent && lpProcessMsg->lParam1 == lParam1)
				{
					if(lpProcessMsg->pszData)
					{
						delete [] lpProcessMsg->pszData;
						lpProcessMsg->pszData = NULL;
					}
					
					delete lpProcessMsg;
					lpProcessMsg = NULL;
				}
				else
					m_DelayProcessQ.PushQ((BYTE *)lpProcessMsg);
			}
		}
	}

	AddDelayProcess(pCharObject, wIdent, wParam, lParam1, lParam2, lParam3, pszData, nDelay);
}

void CharObject::UpdateProcess(CharObject* pCharObject, WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData)
{
	_LPTPROCESSMSG	lpProcessMsg;

//	EnterCriticalSection(&m_cs);

	int nCount = m_ProcessQ.GetCount();

	if(nCount)
	{
		for (int i = 0; i < nCount; i++)
		{
			lpProcessMsg = (_LPTPROCESSMSG)m_ProcessQ.PopQ();

			if(lpProcessMsg)
			{
				if(lpProcessMsg->wIdent == wIdent)
				{
					if(lpProcessMsg->pszData)
					{
						delete [] lpProcessMsg->pszData;
						lpProcessMsg->pszData = NULL;
					}
					
					delete lpProcessMsg;
					lpProcessMsg = NULL;
				}
				else
					m_ProcessQ.PushQ((BYTE *)lpProcessMsg);
			}
		}
	}

//	LeaveCriticalSection(&m_cs);

	AddProcess(pCharObject, wIdent, wParam, lParam1, lParam2, lParam3, pszData);
}

void CharObject::AddProcess(CharObject* pCharObject, WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData)
{
//	EnterCriticalSection(&m_cs);

	_LPTPROCESSMSG lpProcessMsg = new _TPROCESSMSG;

	if(!m_fIsGhost)
	{
		if(lpProcessMsg)
		{
			lpProcessMsg->wIdent			= wIdent;
			lpProcessMsg->wParam			= wParam;
			lpProcessMsg->lParam1			= lParam1;
			lpProcessMsg->lParam2			= lParam2;
			lpProcessMsg->lParam3			= lParam3;

			lpProcessMsg->dwDeliveryTime	= 0;

			lpProcessMsg->pCharObject		= pCharObject;

			if(pszData)
			{
				int nLen = memlen(pszData);

				lpProcessMsg->pszData = new char[nLen];
				memmove(lpProcessMsg->pszData, pszData, nLen);
			}
			else
				lpProcessMsg->pszData		= NULL;

			m_ProcessQ.PushQ((BYTE *)lpProcessMsg);
		}
	}

//	LeaveCriticalSection(&m_cs);
}

void CharObject::AddDelayProcess(CharObject* pCharObject, WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData, int nDelay)
{
//	EnterCriticalSection(&m_cs);

	_LPTPROCESSMSG lpProcessMsg = new _TPROCESSMSG;

	if(lpProcessMsg)
	{
		lpProcessMsg->wIdent			= wIdent;
		lpProcessMsg->wParam			= wParam;
		lpProcessMsg->lParam1			= lParam1;
		lpProcessMsg->lParam2			= lParam2;
		lpProcessMsg->lParam3			= lParam3;

		lpProcessMsg->dwDeliveryTime	= GetTickCount() + nDelay;

		lpProcessMsg->pCharObject		= pCharObject;

		if(pszData)
		{
			int nLen = memlen(pszData);

			lpProcessMsg->pszData = new char[nLen];
			memmove(lpProcessMsg->pszData, pszData, nLen);
		}
		else
			lpProcessMsg->pszData		= NULL;

		m_DelayProcessQ.PushQ((BYTE *)lpProcessMsg);
	}

//	LeaveCriticalSection(&m_cs);
}

void CharObject::AddRefMsg(
        WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData)
{
	CMapCellInfo*	pMapCellInfo	= NULL;
	CharObject*	pCharObject		= NULL;

	int nStartX = m_nCurrX - _RANGE_X;
	int nEndX	= m_nCurrX + _RANGE_X;
	int nStartY = m_nCurrY - _RANGE_Y;
	int nEndY	= m_nCurrY + _RANGE_Y;

	if(m_fInspector) return;

	if(GetTickCount() - m_dwCacheTick > _CACHE_TICK || m_xCacheObjectList.GetCount() == 0)
	{
		m_xCacheObjectList.Clear();

		for (int x = nStartX; x <= nEndX; x++)
		{
			for (int y = nStartY; y <= nEndY; y++)
			{
				if(pMapCellInfo = m_pMap->GetMapCellInfo(x, y))
				{
					if(pMapCellInfo->m_xpObjectList)
					{
						if(pMapCellInfo->m_xpObjectList->GetCount())
						{
							PLISTNODE pListNode = pMapCellInfo->m_xpObjectList->GetHead();

							while (pListNode)
							{
								_LPTOSOBJECT pOSObject = pMapCellInfo->m_xpObjectList->GetData(pListNode);

								if(pOSObject->btType == OS_MOVINGOBJECT)
								{
									pCharObject = (CharObject*)pOSObject->pObject;

									if(!pCharObject->m_fIsGhost)
									{
										if(pCharObject->m_wObjectType & _OBJECT_HUMAN)
										{	
											pCharObject->AddProcess(this, wIdent, wParam, lParam1, lParam2, lParam3, pszData);					
											m_xCacheObjectList.AddNewNode(pCharObject);
										}
									}
								}
								
								pListNode = pMapCellInfo->m_xpObjectList->GetNext(pListNode);
							} // while (pListNode)
						} // if(pMapCellInfo->m_pObjectList.GetCount())
					}
				} // if(pMapCellInfo)
			}// for (y)
		} // for (x)

		m_dwCacheTick = GetTickCount();
	}
	else
	{
		PLISTNODE pListNode = m_xCacheObjectList.GetHead();

		while (pListNode)
		{
			CharObject* pCharObject = m_xCacheObjectList.GetData(pListNode);

			if(!pCharObject->m_fIsGhost)
			{
				if((pCharObject->m_pMap = m_pMap) && (abs(pCharObject->m_nCurrX - m_nCurrX) <= 11) && 
					(abs(pCharObject->m_nCurrY - m_nCurrY) <= 11))
				{
					if(pCharObject->m_wObjectType & _OBJECT_HUMAN)
						pCharObject->AddProcess(this, wIdent, wParam, lParam1, lParam2, lParam3, pszData);
//                     end else begin
//                      if cret.WantRefMsg and ((msg = RM_STRUCK) or (msg = RM_HEAR) or (msg = RM_DEATH)) then
//                       cret.SendMsg (self, msg, wparam, lparam1, lparam2, lparam3, str);
				}
			}
			
			pListNode = m_xCacheObjectList.GetNext(pListNode);
		} // while (pListNode)
	}
}

void CharObject::SpaceMove(int nX, int nY, CMirMap* pMirMap)
{
	CVisibleObject* pVisibleObject;

	if(m_pMap->RemoveObject(m_nCurrX, m_nCurrY, OS_MOVINGOBJECT, this))
	{
		PLISTNODE pListNode = m_xVisibleObjectList.GetHead();

		while (pListNode)
		{
			pVisibleObject = m_xVisibleObjectList.GetData(pListNode);

			if(pVisibleObject)
			{
				delete pVisibleObject;
				pVisibleObject = NULL;
			}
			
			pListNode = m_xVisibleObjectList.RemoveNode(pListNode);
		} // while (pListNode)

		m_pMap = pMirMap;

		m_nCurrX = nX;
		m_nCurrY = nY;

		if(m_pMap->AddNewObject(m_nCurrX, m_nCurrY, OS_MOVINGOBJECT, this))
		{
			AddProcess(this, RM_CLEAROBJECTS, 0, 0, 0, 0, NULL);
			AddProcess(this, RM_CHANGEMAP, 0, 0, 0, 0, NULL);

			AddRefMsg(RM_SPACEMOVE_SHOW, m_nDirection, m_nCurrX, m_nCurrY, 0, NULL);
		}
	}
}

void CharObject::UpdateVisibleObject(CharObject* pCharObject)
{
	CVisibleObject* pVisibleObject;

	PLISTNODE		pListNode = m_xVisibleObjectList.GetHead();

	while (pListNode)
	{
		pVisibleObject = m_xVisibleObjectList.GetData(pListNode);

		if(pVisibleObject->pObject == pCharObject)
		{
			pVisibleObject->nVisibleFlag = 1;
			return;
		}
		
		pListNode = m_xVisibleObjectList.GetNext(pListNode);
	} // while (pListNode)

	CVisibleObject* pNewVisibleObject = new CVisibleObject;

	pNewVisibleObject->nVisibleFlag	= 2;
	pNewVisibleObject->pObject		= pCharObject;

	m_xVisibleObjectList.AddNewNode(pNewVisibleObject);
}

void CharObject::UpdateVisibleItem(int nX, int nY, CMapItem* pMapItem)
{
	CVisibleMapItem* pVisibleItem;

	PLISTNODE		pListNode = m_xVisibleItemList.GetHead();

	while (pListNode)
	{
		pVisibleItem = m_xVisibleItemList.GetData(pListNode);

		if(pVisibleItem->pMapItem == pMapItem)
		{
			pVisibleItem->nVisibleFlag = 1;
			return;
		}
		
		pListNode = m_xVisibleItemList.GetNext(pListNode);
	} // while (pListNode)

	CVisibleMapItem* pVisibleNewItem = new CVisibleMapItem;

	pVisibleNewItem->nVisibleFlag	= 2;
	pVisibleNewItem->wX				= (WORD)nX;
	pVisibleNewItem->wY				= (WORD)nY;
	pVisibleNewItem->pMapItem		= pMapItem;

	m_xVisibleItemList.AddNewNode(pVisibleNewItem);
}

void CharObject::UpdateVisibleEvent(CEvent* pEvent)
{
	CVisibleEvent* pVisibleEvent;

	PLISTNODE		pListNode = m_xVisibleEventList.GetHead();

	while (pListNode)
	{
		pVisibleEvent = m_xVisibleEventList.GetData(pListNode);

		if(pVisibleEvent->pEvent == pEvent)
		{
			pVisibleEvent->nVisibleFlag = 1;
			return;
		}
		
		pListNode = m_xVisibleEventList.GetNext(pListNode);
	} // while (pListNode)

	CVisibleEvent* pVisibleNewEvent = new CVisibleEvent;

	pVisibleNewEvent->nVisibleFlag	= 2;
	pVisibleNewEvent->pEvent		= pEvent;

	m_xVisibleEventList.AddNewNode(pVisibleNewEvent);
}

void CharObject::SearchViewRange()
{
	int nStartX = m_nCurrX - m_nViewRange;
	int nEndX	= m_nCurrX + m_nViewRange;
	int nStartY = m_nCurrY - m_nViewRange;
	int nEndY	= m_nCurrY + m_nViewRange;

	CMapCellInfo*		pMapCellInfo;
	_LPTOSOBJECT		pOSObject;	
	CharObject*		pCharObject;
	CEvent*				pEvent;
	PLISTNODE			pListNode;
	CVisibleObject*		pVisibleObject;
	CVisibleMapItem*	pVisibleItem;
	CVisibleEvent*		pVisibleEvent;

	// Clear VisibleObjectList
	if(m_xVisibleObjectList.GetCount())
	{
		pListNode = m_xVisibleObjectList.GetHead();

		while (pListNode)
		{
			if(pVisibleObject = m_xVisibleObjectList.GetData(pListNode))
				pVisibleObject->nVisibleFlag = 0;

			pListNode = m_xVisibleObjectList.GetNext(pListNode);
		}
	}

	// Clear VisibleMapItem
	if(m_xVisibleItemList.GetCount())
	{
		pListNode = m_xVisibleItemList.GetHead();

		while (pListNode)
		{
			if(pVisibleItem = m_xVisibleItemList.GetData(pListNode))
				pVisibleItem->nVisibleFlag = 0;

			pListNode = m_xVisibleItemList.GetNext(pListNode);
		}
	}

	// Clear VisibleEvent
	if(m_xVisibleEventList.GetCount())
	{
		pListNode = m_xVisibleEventList.GetHead();

		while (pListNode)
		{
			if(pVisibleEvent = m_xVisibleEventList.GetData(pListNode))
				pVisibleEvent->nVisibleFlag = 0;

			pListNode = m_xVisibleEventList.GetNext(pListNode);
		}
	}

	// Search VisibleAllObjectList
	for (int x = nStartX; x <= nEndX; x++)
	{
		for (int y = nStartY; y <= nEndY; y++)
		{
			pMapCellInfo = m_pMap->GetMapCellInfo(x, y);

			if(pMapCellInfo)
			{
				if(pMapCellInfo->m_xpObjectList)
				{
					if(pMapCellInfo->m_xpObjectList->GetCount())
					{
						pListNode = pMapCellInfo->m_xpObjectList->GetHead();

						while (pListNode)
						{
							pOSObject = pMapCellInfo->m_xpObjectList->GetData(pListNode);

							if(pOSObject)
							{
								if(pOSObject->btType == OS_MOVINGOBJECT)
								{
									pCharObject = (CharObject*)pOSObject->pObject;
									
									if(!pCharObject->m_fInspector && !pCharObject->m_fIsGhost && !pCharObject->m_fHideMode)
										UpdateVisibleObject(pCharObject);
								}
								else if(pOSObject->btType == OS_ITEMOBJECT)
								{
									if(GetTickCount() - pOSObject->dwAddTime > 60 * 60 * 1000)
									{
										delete pOSObject;
										pOSObject = NULL;

										pListNode = pMapCellInfo->m_xpObjectList->RemoveNode(pListNode);
										continue;
									}
									else
										UpdateVisibleItem(x, y, (CMapItem*)pOSObject->pObject);
								}
								else if(pOSObject->btType == OS_EVENTOBJECT)
								{
									pEvent = (CEvent*)pOSObject->pObject;

									if(pEvent->m_fVisible)
										UpdateVisibleEvent(pEvent);
								}
							}
							
							pListNode = pMapCellInfo->m_xpObjectList->GetNext(pListNode);
						} // while (pListNode)
					} // if(pMapCellInfo->m_pObjectList.GetCount())
				}
			} // if(pMapCellInfo)
		}// for (y)
	} // for (x)

	if(m_xVisibleObjectList.GetCount())
	{
		pListNode = m_xVisibleObjectList.GetHead();

		while (pListNode)
		{
			if(pVisibleObject = m_xVisibleObjectList.GetData(pListNode))
			{
				if(pVisibleObject->nVisibleFlag == 0)
				{
					if(!pVisibleObject->pObject->m_fHideMode)
						AddProcess(pVisibleObject->pObject, RM_DISAPPEAR, 0, 0, 0, 0, NULL);

					delete pVisibleObject;
					pVisibleObject = NULL;
					
					pListNode = m_xVisibleObjectList.RemoveNode(pListNode);

					continue;
				}
				else
				{
					if(m_wObjectType & _OBJECT_HUMAN)
					{
						if(pVisibleObject->nVisibleFlag == 2)	// 2:New Object
						{
							if(pVisibleObject->pObject != this)
							{
								if(pVisibleObject->pObject->m_fIsDead)
								{
									AddProcess(pVisibleObject->pObject, RM_DEATH, pVisibleObject->pObject->m_nDirection, 
																		pVisibleObject->pObject->m_nCurrX, pVisibleObject->pObject->m_nCurrY, 0, NULL);
								}
								else
									AddProcess(pVisibleObject->pObject, RM_TURN, pVisibleObject->pObject->m_nDirection, 
																		pVisibleObject->pObject->m_nCurrX, pVisibleObject->pObject->m_nCurrY, 
																		0, pVisibleObject->pObject->m_szName);
							}
						}
					}
				} // if(pVisibleObject->nVisibleFlag == 0)
			} // if(pVisibleObject = m_xVisibleObjectList.GetData(pListNode))
			
			pListNode = m_xVisibleObjectList.GetNext(pListNode);
		} // while (pListNode)
	}

	// Update Map Item
	if(m_xVisibleItemList.GetCount())
	{
		pListNode = m_xVisibleItemList.GetHead();

		while (pListNode)
		{
			if(pVisibleItem = m_xVisibleItemList.GetData(pListNode))
			{
				if(pVisibleItem->nVisibleFlag == 0)
				{
					AddProcess(this, RM_ITEMHIDE, 0, (LPARAM)pVisibleItem->pMapItem, pVisibleItem->wX, pVisibleItem->wY, NULL);
					
					delete pVisibleItem;
					pVisibleItem = NULL;
					
					pListNode = m_xVisibleItemList.RemoveNode(pListNode);

					continue;
				}
				else
				{
					if(pVisibleItem->nVisibleFlag == 2)	// 2:New Item
						AddProcess(this, RM_ITEMSHOW, pVisibleItem->pMapItem->wLooks, (LPARAM)pVisibleItem->pMapItem, 
										pVisibleItem->wX, pVisibleItem->wY, pVisibleItem->pMapItem->szName);	
				} // if(pVisibleObject->nVisibleFlag == 0)
			} // if(pVisibleObject = m_xVisibleItemList.GetData(pListNode))
			
			pListNode = m_xVisibleItemList.GetNext(pListNode);
		} // while (pListNode)
	}

	// Update Event Item
	if(m_xVisibleEventList.GetCount())
	{
		pListNode = m_xVisibleEventList.GetHead();

		while (pListNode)
		{
			if(pVisibleEvent = m_xVisibleEventList.GetData(pListNode))
			{
				if(pVisibleEvent->nVisibleFlag == 0)
				{
					AddProcess(this, RM_HIDEEVENT, 0, (LPARAM)pVisibleEvent->pEvent, pVisibleEvent->pEvent->m_nX, pVisibleEvent->pEvent->m_nY);
					
					pListNode = m_xVisibleEventList.RemoveNode(pListNode);

					delete pVisibleEvent;
					pVisibleEvent = NULL;

					continue;
				}
				else if(pVisibleEvent->nVisibleFlag == 2) 
				{
					AddProcess(this, RM_SHOWEVENT, pVisibleEvent->pEvent->m_nEventType, (LPARAM)pVisibleEvent->pEvent, 
									MAKELONG(pVisibleEvent->pEvent->m_nX, pVisibleEvent->pEvent->m_nEventParam), pVisibleEvent->pEvent->m_nY);	
				}
			}
			
			pListNode = m_xVisibleEventList.GetNext(pListNode);
		} // while (pListNode)
	}
}

void CharObject::Disappear()
{
	if(m_pMap)
	{
		m_pMap->RemoveObject(m_nCurrX, m_nCurrY, OS_MOVINGOBJECT, this);
		AddRefMsg(RM_DISAPPEAR, 0, 0, 0, 0, NULL);
	}
}

void CharObject::MakeGhost()
{
	m_fIsGhost		= TRUE;
	m_dwGhostTime	= GetTickCount();
   
	Disappear();
}

void CharObject::TurnTo(int nDir)
{
	AddRefMsg(RM_TURN, nDir, m_nCurrX, m_nCurrY, 0, m_szName);

	m_nDirection = nDir;
}

bool CharObject::TurnXY(int nX, int nY, int nDir)
{
	if(m_nCurrX == nX && m_nCurrY == nY)
	{
		AddRefMsg(RM_TURN, nDir, m_nCurrX, m_nCurrY, 0, m_szName);

		m_nDirection = nDir;

		return TRUE;
	}

	return false;
}

void CharObject::WalkNextPos(int nDir, int& nX, int& nY)
{
	switch (nDir)
	{
		case DR_UP:
			nX = m_nCurrX;
			nY = m_nCurrY - 1;
			break;
		case DR_UPRIGHT:
			nX = m_nCurrX + 1;
			nY = m_nCurrY - 1;
			break;
		case DR_RIGHT:
			nX = m_nCurrX + 1;
			nY = m_nCurrY;
			break;
		case DR_DOWNRIGHT:
			nX = m_nCurrX + 1;
			nY = m_nCurrY + 1;
			break;
		case DR_DOWN:
			nX = m_nCurrX;
			nY = m_nCurrY + 1;
			break;
		case DR_DOWNLEFT:
			nX = m_nCurrX - 1;
			nY = m_nCurrY + 1;
			break;
		case DR_LEFT:
			nX = m_nCurrX - 1;
			nY = m_nCurrY;
			break;
		case DR_UPLEFT:
			nX = m_nCurrX - 1;
			nY = m_nCurrY - 1;
			break;
	}
}

bool CharObject::WalkTo(int nDir)
{
	int nX, nY;

	WalkNextPos(nDir, nX, nY);

	bool fResult = WalkXY(nX, nY, nDir);

	if(fResult)
	{
		if(m_fFixedHideMode)
		{
           if(m_fHumHideMode)
              m_StateAttrV[STATE_TRANSPARENT] = 1;
		}
	}

	return fResult;
}

bool CharObject::RunTo(int nDir)
{
	int nX = m_nCurrX, nY = m_nCurrY;
	int nWalk = 2;

	if(m_tFeatureEx.btHorse)
		nWalk = 3;

	for (int i = 0; i < nWalk; i++)
	{
		switch (nDir)
		{
			case DR_UP:
				nY--;
				break;
			case DR_UPRIGHT:
				nX++;
				nY--;
				break;
			case DR_RIGHT:
				nX++;
				break;
			case DR_DOWNRIGHT:
				nX++;
				nY++;
				break;
			case DR_DOWN:
				nY++;
				break;
			case DR_DOWNLEFT:
				nX--;
				nY++;
				break;
			case DR_LEFT:
				nX--;
				break;
			case DR_UPLEFT:
				nX--;
				nY--;
				break;
		}
		
		if(m_pMap->CanMove(nX, nY) == false)
			return false;
	}

	return RunXY(nX, nY, nDir); 
}

bool CharObject::WalkXY(int nX, int nY, int nDir)
{
	int nFlag = m_pMap->CheckDoorEvent(nX, nY, m_nEvent);

	if(nFlag)
	{
		if(nFlag == _DOOR_OPEN)
			AddRefMsg(RM_DOOROPEN, 1, m_nEvent, 0, 0, NULL);
	}

	if(m_pMap->CanMove(nX, nY))
	{
		if(m_pMap->MoveToMovingObject(m_nCurrX, m_nCurrY, nX, nY, this))
		{
//#ifdef _DEBUG
//	_RPT4(_CRT_WARN, "WALK:%d, %d -> %d, %d", m_nCurrX, m_nCurrY, nX, nY);
//	_RPT1(_CRT_WARN, " [%d]\n", nDir); 
//#endif
			AddRefMsg(RM_WALK, nDir, m_nCurrX, m_nCurrY, 0, NULL);

			m_nCurrX		= nX;
			m_nCurrY		= nY;

			m_nDirection	= nDir;

			m_dwHealthTick	-= 20;

			return TRUE;
		}
	}

	return false;
}

bool CharObject::RunXY(int nX, int nY, int nDir)
{
	if(m_pMap->MoveToMovingObject(m_nCurrX, m_nCurrY, nX, nY, this))
	{
//#ifdef _DEBUG
//	_RPT4(_CRT_WARN, "RUN:%d, %d -> %d, %d", m_nCurrX, m_nCurrY, nX, nY);
//	_RPT1(_CRT_WARN, " [%d]\n", nDir); 
//#endif
		AddRefMsg(RM_RUN, nDir, m_nCurrX, m_nCurrY, 0, NULL);

		m_nCurrX		= nX;
		m_nCurrY		= nY;
		m_nDirection	= nDir;

		m_dwHealthTick	-= 150;
		m_dwSpellTick	-= 10;
		m_dwSpellTick	= _MAX(0, m_dwSpellTick);
		m_btPerHealth--;
		m_btPerSpell--;

		return TRUE;
	}

	return false;
}

void CharObject::IncHealthSpell(int nHP, int nMP)
{
	if(nHP >= 0)
	{
		if(m_WAbility.HP + nHP < m_WAbility.MaxHP)
			m_WAbility.HP += nHP;
		else 
			m_WAbility.HP = m_WAbility.MaxHP;
	}

	if(nMP >= 0)
	{
		if(m_WAbility.MP + nMP < m_WAbility.MaxMP)
			m_WAbility.MP += nMP;
		else 
			m_WAbility.MP = m_WAbility.MaxMP;
	}

	HealthSpellChanged();
}

//val : (+) dec spell
//      (-) inc spell
void CharObject::DamageSpell(int nVal)
{
	if(nVal > 0)
	{
		if(m_WAbility.MP - nVal > 0)
			m_WAbility.MP = m_WAbility.MP - nVal;
		else
			m_WAbility.MP = 0;
	}
	else
	{
		if(m_WAbility.MP - nVal < m_WAbility.MaxMP)
			m_WAbility.MP = m_WAbility.MP - nVal;
		else
			m_WAbility.MP = m_WAbility.MaxMP;
	}
}

//nDamage : if(+) damage health
//			if(-) healing health
void CharObject::DamageHealth(int nDamage)
{
/*  보호의반지 
	if BoMagicShield and (damage > 0) and (WAbil.MP > 0) then begin
      spdam := Round (damage * 1.5);
      if integer(WAbil.MP) >= spdam then begin
         WAbil.MP := WAbil.MP - spdam;
         spdam := 0;
      end else begin
         spdam := spdam - WAbil.MP;
         WAbil.MP := 0;
      end;
      damage := Round (spdam / 1.5);
      HealthSpellChanged;
   end; */
	if(nDamage > 0)
	{
		if(m_WAbility.HP - nDamage > 0)
			m_WAbility.HP = m_WAbility.HP - nDamage;
		else
			m_WAbility.HP = 0;
	}
	else
	{
		if(m_WAbility.HP - nDamage < m_WAbility.MaxHP)
			m_WAbility.HP = m_WAbility.HP - nDamage;
		else
			m_WAbility.HP = m_WAbility.MaxHP;
	}
}

int CharObject::GetMagicStruckDamage(int nDamage)
{
	int nArmor = 0;

	if((HIBYTE(m_WAbility.MAC) - LOBYTE(m_WAbility.MAC) + 1) > 0)
		nArmor = LOBYTE(m_WAbility.MAC) + (rand() % (HIBYTE(m_WAbility.MAC) - LOBYTE(m_WAbility.MAC) + 1));
	else
		nArmor = LOBYTE(m_WAbility.MAC);

	nDamage	= _MAX(0, nDamage - nArmor);

	if(nDamage > 0)
	{
		if(m_fAbilMagicBubbleDefence) // 주술의막
		{
			nDamage = ROUND(nDamage / 100 * (m_btMagicBubbleDefenceLevel + 2) * 8);
			DamageBubbleDefence();
		}
	}										  

	return nDamage;
}

void CharObject::StruckDamage(int nDamage)
{
	if(nDamage > 0)
	{
		bool	fReCalc = false;
		int		wDam = (rand() % 10) + 5;

		if(m_StateAttrV[POISON_DAMAGEARMOR] > 0)
		{
			wDam	= ROUND(wDam * 1.2);
			nDamage	= ROUND(nDamage * 1.2);
		}

		if(m_pUserInfo)
		{
			_LPTUSERITEMRCD	lptDress = m_pUserInfo->GetDress();

			if(lptDress)
			{
				int nDura		= lptDress->nDura;
				int nOldDura	= ROUND(nDura / 1000);
				
				nDura	-= wDam;

				if(nDura <= 0)	// 닳아 없어짐
				{
					lptDress->nDura = nDura = 0;

//               hum.SendDelItem (UseItems[U_DRESS]); //클라이언트에 없어진거 보냄
					m_pUserInfo->SetEmptyDress();

					AddProcess(this, RM_DURACHANGE, U_DRESS, nDura, lptDress->nDuraMax, 0); 
					AddProcess(this, RM_FEATURECHANGED, 0, GetFeatureToLong(), 0, 0);
					
					fReCalc = TRUE;
				}
				else
					lptDress->nDura = nDura;

				if(nOldDura != ROUND(nDura / 1000))
					AddProcess(this, RM_DURACHANGE, U_DRESS, lptDress->nDura, lptDress->nDuraMax, 0); 
			}

			_LPTUSERITEMRCD	lptUseItem;

			for (int i = 0; i < 8; i++)
			{
				lptUseItem = m_pUserInfo->GetUseItem(i);

				if(lptUseItem && (rand() % 8) == 0)
				{
					int nDura		= lptUseItem->nDura;
					int nOldDura	= ROUND(nDura / 1000);
					
					nDura	-= wDam;

					if(nDura <= 0)	// 닳아 없어짐
					{
						lptUseItem->nDura = nDura = 0;

	//               hum.SendDelItem (UseItems[U_DRESS]); //클라이언트에 없어진거 보냄
						m_pUserInfo->SetEmptyUseItem(i);

						AddProcess(this, RM_DURACHANGE, i, nDura, lptUseItem->nDuraMax, 0); 
						AddProcess(this, RM_FEATURECHANGED, 0, GetFeatureToLong(), 0, 0);

						fReCalc = TRUE;
					}
					else
						lptUseItem->nDura = nDura;

					if(nOldDura != ROUND(nDura / 1000))
						AddProcess(this, RM_DURACHANGE, i, lptUseItem->nDura, lptUseItem->nDuraMax, 0); 
				}
			}
		}

		if(fReCalc && m_pUserInfo)
			((CPlayerObject*)this)->RecalcAbilitys();

		DamageHealth(nDamage);
	}
}

int CharObject::GetHitStruckDamage(int nDamage)
{
	int nArmor;
	int nRnd = ((HIBYTE(m_WAbility.AC) - LOBYTE(m_WAbility.AC)) + 1);

	if(nRnd > 0)
		nArmor = LOBYTE(m_WAbility.AC) + (rand() % nRnd);
	else
		nArmor = LOBYTE(m_WAbility.AC);

	nDamage = _MAX(0, nDamage - nArmor);

	if(nDamage > 0)
	{
		if(m_fAbilMagicBubbleDefence) // 주술의막
		{
			nDamage = ROUND(nDamage / 100 * (m_btMagicBubbleDefenceLevel + 2) * 8);
			DamageBubbleDefence();
		}
	}										  

	return nDamage;
}

int CharObject::GetAttackPower(int nDamage, int nVal)
{
	int nPower = 0;

	if(nVal < 0) nVal = 0;

//	if(Lucky > 0)
//	{
//	}
//	else
//	{
		nPower = nDamage + (rand() % (nVal + 1));
//	}

	return nPower;
/*var
   v1, v2: integer;
begin
   if val < 0 then val := 0;
   if Luck > 0 then begin
      if Random(10 - _MIN(9,Luck)) = 0 then Result := damage + val //행운인경우
      else Result := damage + Random(val + 1);
   end else begin
      Result := damage + Random(val + 1);
      if Luck < 0 then begin
         if Random(10 - _MAX(0,-Luck)) = 0 then Result := damage;  //불운인경우
      end;
   end; */
}

void CharObject::DoDamageWeapon(int nDamage)
{
	if(m_pUserInfo)
	{
		_LPTUSERITEMRCD	lptWeapon = m_pUserInfo->GetWeapon();

		if(lptWeapon)
		{
			int nDura		= lptWeapon->nDura;
			int nOldDura	= ROUND(nDura / 1000);
			
			nDura	-= nDamage;

			if(nDura <= 0)	// 닳아 없어짐
			{
				lptWeapon->nDura = nDura = 0;
//               hum.SendDelItem (UseItems[U_DRESS]); //클라이언트에 없어진거 보냄

				((CPlayerObject*)this)->RecalcAbilitys();

				m_pUserInfo->SetEmptyWeapon();

				AddProcess(this, RM_DURACHANGE, U_WEAPON, nDura, lptWeapon->nDuraMax, 0); 
			}
			else
				lptWeapon->nDura = nDura;

			if(nOldDura != ROUND(nDura / 1000))
				AddProcess(this, RM_DURACHANGE, U_WEAPON, lptWeapon->nDura, lptWeapon->nDuraMax, 0); 
		}
	}
}

bool CharObject::_Attack(WORD wHitMode, CharObject* pObject)
{
	int nPower	= GetAttackPower(LOBYTE(m_WAbility.DC), (HIBYTE(m_WAbility.DC) - LOBYTE(m_WAbility.DC)));
	int nSecPwr	= 0;
	int nWeaponDamage;

	if(pObject)
	{
		if(wHitMode == CM_POWERHIT && m_pUserInfo->m_fPowerHitSkill)
		{
			m_pUserInfo->m_fPowerHitSkill = false;
			nPower += m_btHitPlus;
		}

		if(wHitMode == CM_FIREHIT && m_pUserInfo->m_fFireHitSkill)
		{
			m_pUserInfo->m_fFireHitSkill = false;
			
			nPower += ROUND(nPower / 100 * (m_nHitDouble * 10));
		}
	}
	else
	{
		if(wHitMode == CM_POWERHIT && m_pUserInfo->m_fPowerHitSkill)
		{
			m_pUserInfo->m_fPowerHitSkill = false;
			nPower += m_btHitPlus;
		}
	}

	if(wHitMode == CM_LONGHIT)
	{
		nSecPwr	= 0;

		if(m_pUserInfo->m_lpTMagicErgumSkill)
			nSecPwr = ROUND(nPower / (3/*MaxTrainLevel*/ + 2) * (m_pUserInfo->m_lpTMagicErgumSkill->btLevel + 2));
		else
			nSecPwr = nPower;

		if(nSecPwr > 0)
			SwordLongAttack(nSecPwr);
	}

	if(wHitMode == CM_WIDEHIT)
	{
		nSecPwr	= 0;

		if(m_pUserInfo->m_lpTMagicBanwolSkill)
			nSecPwr = ROUND(nPower / (3/*MaxTrainLevel*/ + 10) * (m_pUserInfo->m_lpTMagicBanwolSkill->btLevel + 2));
		else
			nSecPwr = nPower;

		if(nSecPwr > 0)
			SwordWideAttack(nSecPwr);
	}

	if(pObject)
	{
		if(IsProperTarget(pObject))
		{
			if(pObject->m_btHitPoint > 0)
			{
				if(m_btHitPoint < rand() % pObject->m_btSpeedPoint)
					nPower = 0;
			}
		}
		else
			nPower = 0;
	}	

	if(nPower > 0)
	{
		nPower			= pObject->GetHitStruckDamage(nPower);
		nWeaponDamage	= (rand() % 5) + 2 - m_AddAbility.WeaponStrong;
	
/*		if(m_wObjectType & _OBJECT_HUMAN)
		{
				case CM_FIREHIT:
				{
					if(m_pUserInfo->m_lpTMagicFireSwordSkill)
						nSecPwr = _ROUND(nPwr / (3 + 2) * (m_pUserInfo->m_lpTMagicFireSwordSkill->btLevel + 2));
					else
						nSecPwr = nPower;

					break;
				}		  
		}
*/
		if(nWeaponDamage > 0)
		{
			if(m_pUserInfo->GetWeapon()) 
				DoDamageWeapon(nWeaponDamage);
		}

		if(nPower > 0)
		{
			pObject->StruckDamage(nPower);

			// Debug Code
#ifdef _DEBUG
			char szMsg[64];

			if(m_wObjectType & _OBJECT_HUMAN)
			{
				sprintf(szMsg, "%s 맞음 - PW:%d HP:%d, 무기내구닳음:%d", pObject->m_szName, nPower, pObject->m_WAbility.HP, nWeaponDamage);
				SysMsg(szMsg, 0);
			}
			else if(pObject->m_wObjectType & _OBJECT_HUMAN)
			{
				sprintf(szMsg, "%s 맞음 - PW:%d HP:%d", pObject->m_szName, nPower, pObject->m_WAbility.HP);
				pObject->SysMsg(szMsg, 0);
			}
			// Debug Code
#endif

			AddDelayProcess(pObject, RM_STRUCK, nPower, pObject->m_WAbility.HP, pObject->m_WAbility.MaxHP, (LONG)this, NULL, 550);

			if(!(pObject->m_wObjectType & _OBJECT_HUMAN))
				pObject->AddProcess(pObject, RM_STRUCK, nPower, pObject->m_WAbility.HP, pObject->m_WAbility.MaxHP, (LONG)this, NULL);

			return TRUE;
		}
	}

	return false;
}									   

bool CharObject::HitXY(WORD wIdent, int nX, int nY, int nDir, int nHitStyle)
{
	if((GetTickCount() - m_dwLatestHitTime) < 600)
		m_nHitTimeOverCount++;
	else
		m_nHitTimeOverCount = 0;

	if(m_nHitTimeOverCount < 2)
	{
		if(m_nCurrX == nX && m_nCurrY == nY)
		{
			if(wIdent == CM_WIDEHIT && m_pUserInfo->m_lpTMagicBanwolSkill)
			{
				if(m_WAbility.MP > 0)
				{
					int nLevel		= m_pUserInfo->GetMagicRcdByID(_SKILL_BANWOL)->btLevel;
					int nSpellPoint = GetMagicInfo(_SKILL_BANWOL)->GetSpellPoint(nLevel);
					DamageSpell(nSpellPoint);
					HealthSpellChanged();
				}
				else
					wIdent = CM_HIT;
			}

			m_nDirection		= nDir;
			m_dwLatestHitTime	= GetTickCount();

			CharObject* pObject = GetFrontObject();

			if(pObject)
			{
				if(_Attack(wIdent, pObject))
				{
					SelectTarget(pObject);

					m_dwHealthTick	-= 100;
					m_dwSpellTick	-= 100;
					m_dwSpellTick	= _MAX(0, m_dwSpellTick);
					m_btPerHealth	-= 2;
					m_btPerSpell	-= 2;
				}
			}

			if(m_wObjectType & _OBJECT_HUMAN)
			{
				switch (wIdent)
				{
					case CM_HIT:
						AddRefMsg(RM_HIT, nDir, m_nCurrX, m_nCurrY, nHitStyle, NULL);
						break;
					case CM_WIDEHIT:
						AddRefMsg(RM_WIDEHIT, nDir, m_nCurrX, m_nCurrY, nHitStyle, NULL);
						break;
					case CM_LONGHIT:
						AddRefMsg(RM_LONGHIT, nDir, m_nCurrX, m_nCurrY, nHitStyle, NULL);
						break;
					case CM_FIREHIT:
						AddRefMsg(RM_FIREHIT, nDir, m_nCurrX, m_nCurrY, nHitStyle, NULL);
						break;
					case CM_POWERHIT:
						AddRefMsg(RM_POWERHIT, nDir, m_nCurrX, m_nCurrY, nHitStyle, NULL);
						break;
				}
			}

			if(m_pUserInfo->m_lpTMagicPowerHitSkill)		// 예도 검법
			{
				m_pUserInfo->m_btAttackSkillCount--;

				if(m_pUserInfo->m_btAttackSkillCount == m_pUserInfo->m_btAttackSkillPointCount)
				{
					m_pUserInfo->m_fPowerHitSkill = TRUE;
					SendSocket(NULL, "+PWR");
				}

				if(m_pUserInfo->m_btAttackSkillCount <= 0)
				{
					m_pUserInfo->m_btAttackSkillCount		= 7 - m_pUserInfo->m_lpTMagicPowerHitSkill->btLevel;
					m_pUserInfo->m_btAttackSkillPointCount	= rand() % m_pUserInfo->m_btAttackSkillCount;
				}
			}

			return TRUE;
		}
	}

	return false;
}

void CharObject::DoPushed(int nDirection)
{
	int nFrontX, nFrontY;

	m_nDirection = nDirection;

	GetFrontPosition(nFrontX, nFrontY);

	if(m_pMap->MoveToMovingObject(m_nCurrX, m_nCurrY, nFrontX, nFrontY, this))
	{
		AddRefMsg(RM_PUSH, GetBack(nDirection), m_nCurrX, m_nCurrY, 0, NULL);

		m_nCurrX = nFrontX;
		m_nCurrY = nFrontY;
	}
}

void CharObject::GoldChanged()
{
	if(m_wObjectType & _OBJECT_HUMAN)
		UpdateProcess(this, RM_GOLDCHANGED, 0, 0, 0, 0, NULL);
}

void CharObject::HealthSpellChanged()
{
	if(m_wObjectType & _OBJECT_HUMAN)
		UpdateProcess(this, RM_HEALTHSPELLCHANGED, 0, 0, 0, 0, NULL);

	if(m_fOpenHealth)
		AddRefMsg(RM_HEALTHSPELLCHANGED, 0, 0, 0, 0, NULL);
}

CharObject* CharObject::AddCreatureSysop(int nX, int nY, CMonRaceInfo* pMonRaceInfo, bool fSearch)
{
	MonsterObject* pMonObject = AddCreature(m_pMap, nX, nY, pMonRaceInfo->nIndex, fSearch);

	if(pMonObject)
	{
		if(g_pMonGenInfo)
			g_pMonGenInfo[g_nNumOfMonGenInfo].xMonsterObjList.AddNewNode(pMonObject);
	}

	return (CharObject*)pMonObject;
}

void CharObject::DoMotaebo()
{
	int nFrontX, nFrontY;

	CharObject* pCharObject = GetFrontObject();

	if(pCharObject)
	{
		for (int i = 0; i < 5; i++)
		{
			pCharObject->DoPushed(m_nDirection);

			GetFrontPosition(nFrontX, nFrontY);
			
			if(m_pMap->MoveToMovingObject(m_nCurrX, m_nCurrY, nFrontX, nFrontY, this))
			{
				AddRefMsg(RM_RUSH, m_nDirection, m_nCurrX, m_nCurrY, 0, NULL);

				m_nCurrX		= nFrontX;
				m_nCurrY		= nFrontY;

				if(!(pCharObject = GetFrontObject()))
					break;
			}
			else 
				break;
		}
	}
	else
	{
		for (int i = 0; i < 5; i++)
		{
			GetFrontPosition(nFrontX, nFrontY);
			
			if(m_pMap->MoveToMovingObject(m_nCurrX, m_nCurrY, nFrontX, nFrontY, this))
			{
				AddRefMsg(RM_RUSH, m_nDirection, m_nCurrX, m_nCurrY, 0, NULL);

				m_nCurrX		= nFrontX;
				m_nCurrY		= nFrontY;
			}
			else
				break;
		}
	}
}

/* **************************************************************************************

										탐기파연

   ************************************************************************************** */

bool CharObject::DoShowHP(CharObject* pObject, CMagicInfo* pMagicInfo, int nLevel)
{
	if(pObject)
	{
		if(!pObject->m_fOpenHealth)
		{
			if((rand() % 6) <= (3 + m_Ability.Level))
			{
				pObject->m_dwOpenHealthStart	= GetTickCount();
				pObject->m_dwOpenHealthTime		= pMagicInfo->GetPower13(30 + GetRPow(m_WAbility.SC) * 2, nLevel) * 1000;
				
				pObject->AddDelayProcess(pObject, RM_DOOPENHEALTH, 0, 0, 0, 0, NULL, 3000);

				return TRUE;
			}
		}
	}

	return false;
}

void CharObject::MakeOpenHealth()
{
	m_fOpenHealth		= TRUE;
   
	m_nCharStatusEx		|= STATE_OPENHEATH;
	m_nCharStatus		= GetCharStatus();
   
	AddRefMsg(RM_OPENHEALTH, 0, m_WAbility.HP/*lparam1*/, m_WAbility.MaxHP/*lparam2*/, 0, NULL);
}

void CharObject::BreakOpenHealth()
{
	m_fOpenHealth		= false;
   
	m_nCharStatusEx		^= STATE_OPENHEATH;
	m_nCharStatus		= GetCharStatus();
   
	AddRefMsg(RM_CLOSEHEALTH, 0, 0, 0, 0, NULL);
}

/* **************************************************************************************

										무태보

   ************************************************************************************** */

int CharObject::CharPushed(int nDir, int nPushCount)
{
	int nFrontX, nFrontY;
	int nResult = 0;

	for (int i = 0; i < nPushCount; i++)
	{
		GetFrontPosition(nFrontX, nFrontY);

		if(m_pMap->CanMove(nFrontX, nFrontY, false))
		{
			if(m_pMap->MoveToMovingObject(m_nCurrX, m_nCurrY, nFrontX, nFrontY, this))
			{
				m_nCurrX = nFrontX;
				m_nCurrY = nFrontY;

				AddRefMsg(RM_PUSH, GetBack(nDir), m_nCurrX, m_nCurrY, 0, NULL);

				nResult++;
			}
		}
	}

	m_nDirection = GetBack(nDir);

	return nResult;

/*function  TCreature.CharPushed (ndir, pushcount: integer): integer;  //어떤 힘에 의해서 밀려나다.
var
   i, nx, ny, olddir, oldx, oldy: integer;
begin
   Result := 0;
   olddir := Dir;
   oldx := CX;
   oldy := CY;
   Dir := ndir;
   for i:=0 to pushcount-1 do begin
      GetFrontPosition (self, nx, ny);
      if PEnvir.CanWalk (nx, ny, false{겸침허용안함}) then begin
         if PEnvir.MoveToMovingObject (CX, CY, self, nx, ny, false) > 0 then begin
            CX := nx;
            CY := ny;
            SendRefMsg (RM_PUSH, GetBack(ndir), CX, CY, 0, '');
            Inc (Result);
            if RaceServer >= RC_ANIMAL then
               WalkTime := WalkTime + 800; //밀리면서 늦게 때린다.
         end else
            break;
      end else
         break;
   end;
   Dir := GetBack(ndir); //olddir;
end;
*/
}

/* **************************************************************************************

										화염풍

   ************************************************************************************** */

int CharObject::MagicPushArround(int nPushLevel)
{
	int nLevelGap, nPush,nDir;
	int nResult = 0;

	if(m_xVisibleObjectList.GetCount())
	{
		PLISTNODE		pListNode = m_xVisibleObjectList.GetHead();
		CVisibleObject*	pVisibleObject;

		while (pListNode)
		{
			if(pVisibleObject = m_xVisibleObjectList.GetData(pListNode))
			{
				if(abs(m_nCurrX - pVisibleObject->pObject->m_nCurrX) <= 1 && abs(m_nCurrY - pVisibleObject->pObject->m_nCurrY) <= 1)
				{
					if(!pVisibleObject->pObject->m_fIsDead && pVisibleObject->pObject != this)
					{
						if(m_Ability.Level > pVisibleObject->pObject->m_Ability.Level)
						{
							nLevelGap = m_Ability.Level - pVisibleObject->pObject->m_Ability.Level;
							
							if((rand() % 20) < 6 + nPushLevel * 3 + nLevelGap)
							{
								if(IsProperTarget(pVisibleObject->pObject))
								{
									nPush	= 1 + _MAX(0, nPushLevel - 1) + rand() % 2;
									nDir	= GetNextDirection (pVisibleObject->pObject->m_nCurrX, pVisibleObject->pObject->m_nCurrY);
									pVisibleObject->pObject->CharPushed(nDir, nPush);
									nResult++;
								}
							}
						}
					}
				}
			} // if(pVisibleObject = m_xVisibleObjectList.GetData(pListNode))
			
			pListNode = m_xVisibleObjectList.GetNext(pListNode);
		} // while (pListNode)
	}

	return nResult;

/*function  TMagicManager.MagicPushArround (user: TCreature; pushlevel: integer): integer;
var
   i, ndir, levelgap, push: integer;
   cret: TCreature;
begin
   Result := 0;
   for i:=0 to user.VisibleActors.Count-1 do begin
      cret := TCreature (PTVisibleActor(user.VisibleActors[i]).cret);
      if(abs(user.CX-cret.CX) <= 1) and (abs(user.CY-cret.CY) <= 1) then begin
         if(not cret.Death) and (cret <> user) then begin
            if(user.Abil.Level > cret.Abil.Level) and (not cret.StickMode) then begin
               levelgap := user.Abil.Level - cret.Abil.Level;
               if(Random(20) < 6+pushlevel*3+levelgap) then begin  //수련정도에 따라서
                  if user.IsProperTarget(cret) then begin
                     push := 1 + _MAX(0,pushlevel-1) + Random(2);
                     ndir := GetNextDirection (user.CX, user.CY, cret.CX, cret.CY);
                     cret.CharPushed (ndir, push);
                     Inc (Result);
                  end;
               end;
            end;
         end;
      end;
   end;
end;
*/
}

/* **************************************************************************************

										염사장, 뢰인장

   ************************************************************************************** */

int CharObject::MagicPassThroughMagic(int nStartX, int nStartY, int nTargetX, int nTargetY, int nDir, int nPwr, bool fUndeadAttack)
{
	int				nResult = 0;
	CharObject*	pObject;

	for (int i = 0; i < 12; i++)
	{
		pObject = m_pMap->GetObject(nStartX, nStartY);

		if(pObject)
		{
			if(IsProperTarget(pObject))
			{
				if(pObject->m_btAntiMagic <= rand() % 10)
				{
					if(fUndeadAttack)
						nPwr = ROUND(nPwr * 1.5);

					pObject->AddDelayProcess(this, RM_MAGSTRUCK, 0, nPwr, 0, 0, NULL, 600);

					nResult++;
				}
			}
		}

		if(!((abs(nStartX - nTargetX) <= 0) && (abs(nStartY - nTargetY) <= 0)))
		{
			nDir = GetNextDirection(nStartX, nStartY, nTargetX, nTargetY);

			if(!(GetNextPosition(nStartX, nStartY, nDir, 1, nStartX, nStartY)))
				break;
		}
		else
			break;
	}

	return nResult;
/*
function  TCreature.MagicPassThroughMagic (sx, sy, tx, ty, ndir, magpwr: integer; undeadattack: Boolean): integer;
var
   i, tcount, acpwr: integer;
   cret: TCreature;
begin
   tcount := 0;
   for i:=0 to 12 do begin
      cret := TCreature (PEnvir.GetCreature (sx, sy, TRUE));
      if cret <> nil then begin
         //if(RaceServer = RC_USERHUMAN) and (cret.RaceServer = RC_USERHUMAN) and ((cret.InSafeZone) or (InSafeZone)) then
         //   continue;  //안전지대
         if IsProperTarget (cret) then begin
            if cret.AntiMagic <= Random(10) then begin  //마법 회피가 있음
               if undeadattack then  //언데드 몬스터에게 공격력 강화인 경우
                  acpwr := Round (magpwr * 1.5)
               else
                  acpwr := magpwr;
               cret.SendDelayMsg (self, RM_MAGSTRUCK, 0, acpwr, 0, 0, '', 600);
               Inc (tcount);
            end;
         end;
      end;
      if not ((abs(sx-tx) <= 0) and (abs(sy-ty) <= 0)) then begin
         ndir := GetNextDirection (sx, sy, tx, ty);
         if not GetNextPosition (PEnvir, sx, sy, ndir, 1, sx, sy) then
            break;
      end else
         break;
   end;
   Result := tcount;
end; */
}

/* **************************************************************************************

										주술의막

   ************************************************************************************** */

bool CharObject::MagicBubbleDefenceUp(int nLevel, int nSec)
{
	if(m_StateAttrV[STATE_BUBBLEDEFENCEUP] == 0)
	{
		UINT nOldStatus = m_nCharStatus;

		m_StateAttrV[STATE_BUBBLEDEFENCEUP]		= (WORD)nSec; 
		m_dwStatusTime[STATE_BUBBLEDEFENCEUP]	= GetTickCount();

		m_nCharStatus = GetCharStatus();

		if(nOldStatus != m_nCharStatus)
			AddRefMsg(RM_CHARSTATUSCHANGED, m_sHitSpeed/*wparam*/, m_nCharStatus, 0, 0, NULL);

		m_fAbilMagicBubbleDefence		= TRUE;
		m_btMagicBubbleDefenceLevel	= nLevel;

		return TRUE;
	}

	return false;
}

void CharObject::DamageBubbleDefence()
{
	if(m_StateAttrV[STATE_BUBBLEDEFENCEUP] > 0)
	{
		if(m_StateAttrV[STATE_BUBBLEDEFENCEUP] > 3)
			m_StateAttrV[STATE_BUBBLEDEFENCEUP] -= 3;
		else
			m_StateAttrV[STATE_BUBBLEDEFENCEUP] = 1;
	}
}

/* **************************************************************************************

										지염술

   ************************************************************************************** */

int CharObject::MagicMakeFireCross(int nDamage, int nHTime, int nX, int nY)
{
   if(!(m_pMap->GetEvent(nX, nY - 1)))
   {
	   CEvent *pEvent = (CEvent*)new CFireBurnEvent(nX, nY - 1, nHTime * 100, this, nDamage);
	   g_xEventList.AddNewNode(pEvent);
   }
   
   if(!(m_pMap->GetEvent(nX - 1, nY)))
   {
	   CEvent *pEvent = (CEvent*)new CFireBurnEvent(nX - 1, nY, nHTime * 100, this, nDamage);
	   g_xEventList.AddNewNode(pEvent);
   }
   
   if(!(m_pMap->GetEvent(nX, nY)))
   {
	   CEvent *pEvent = (CEvent*)new CFireBurnEvent(nX, nY, nHTime * 100, this, nDamage);
	   g_xEventList.AddNewNode(pEvent);
   }

   if(!(m_pMap->GetEvent(nX + 1, nY)))
   {
	   CEvent *pEvent = (CEvent*)new CFireBurnEvent(nX + 1, nY, nHTime * 100, this, nDamage);
	   g_xEventList.AddNewNode(pEvent);
   }

   if(!(m_pMap->GetEvent(nX, nY + 1)))
   {
	   CEvent *pEvent = (CEvent*)new CFireBurnEvent(nX, nY + 1, nHTime * 100, this, nDamage);
	   g_xEventList.AddNewNode(pEvent);
   }

   return 1;
}

/* **************************************************************************************

										결계

   ************************************************************************************** */

int CharObject::MagicMakeHolyCurtain(int nHTime, int nX, int nY)
{
	int						nResult = 0;
	CWHList<CharObject*>	ObjectList;
	CHolySeizeInfo*			pHolySeizeInfo = NULL;

	if(m_pMap->CanMove(nX, nY, TRUE))
	{
		m_pMap->GetMapObject(nX, nY, 1, &ObjectList);

		if(ObjectList.GetCount())
		{
			PLISTNODE		pListNode = ObjectList.GetHead();
			MonsterObject*	pTargetObject;

			while (pListNode)
			{
				if(pTargetObject = (MonsterObject*)ObjectList.GetData(pListNode))
				{
					if((pTargetObject->m_wObjectType & _OBJECT_ANIMAL) && (pTargetObject->m_Ability.Level < m_Ability.Level - 1 + rand() % 4) &&
						 (pTargetObject->m_Ability.Level < 50) && (pTargetObject->m_pMasterObject == NULL))
					{
						pTargetObject->MakeHolySeize(nHTime * 1000);

						if(!pHolySeizeInfo)
						{
							pHolySeizeInfo = new CHolySeizeInfo;

							for (int i = 0; i < 7; i++)
								pHolySeizeInfo->Event[0] = NULL;

							pHolySeizeInfo->dwOpenTime	= GetTickCount();
							pHolySeizeInfo->dwSeizeTime = nHTime * 1000;

						}

						pHolySeizeInfo->SeizeList.AddNewNode(pTargetObject);
												
						nResult++;					
					}
				}
			
				pListNode = ObjectList.RemoveNode(pListNode);
			}
		}

		if(nResult > 0 && pHolySeizeInfo)
		{
		   CEvent *pEvent = (CEvent*)new CHolyCurtainEvent(nX - 1, nY - 2, nHTime * 1000, this);
		   g_xEventList.AddNewNode(pEvent);
		   pHolySeizeInfo->Event[0] = pEvent;
		   pEvent = (CEvent*)new CHolyCurtainEvent(nX + 1, nY - 2, nHTime * 1000, this);
		   g_xEventList.AddNewNode(pEvent);
		   pHolySeizeInfo->Event[1] = pEvent;
		   pEvent = (CEvent*)new CHolyCurtainEvent(nX - 2, nY - 1, nHTime * 1000, this);
		   g_xEventList.AddNewNode(pEvent);
		   pHolySeizeInfo->Event[2] = pEvent;
		   pEvent = (CEvent*)new CHolyCurtainEvent(nX + 2, nY - 1, nHTime * 1000, this);
		   g_xEventList.AddNewNode(pEvent);
		   pHolySeizeInfo->Event[3] = pEvent;
		   pEvent = (CEvent*)new CHolyCurtainEvent(nX - 2, nY + 1, nHTime * 1000, this);
		   g_xEventList.AddNewNode(pEvent);
		   pHolySeizeInfo->Event[4] = pEvent;
		   pEvent = (CEvent*)new CHolyCurtainEvent(nX + 2, nY + 1, nHTime * 1000, this);
		   g_xEventList.AddNewNode(pEvent);
		   pHolySeizeInfo->Event[5] = pEvent;
		   pEvent = (CEvent*)new CHolyCurtainEvent(nX - 1, nY + 2, nHTime * 1000, this);
		   g_xEventList.AddNewNode(pEvent);
		   pHolySeizeInfo->Event[6] = pEvent;
		   pEvent = (CEvent*)new CHolyCurtainEvent(nX + 1, nY + 2, nHTime * 1000, this);
		   g_xEventList.AddNewNode(pEvent);
		   pHolySeizeInfo->Event[7] = pEvent;

		   g_xHolySeizeList.AddNewNode(pHolySeizeInfo);
		}
	}

	return nResult;					
}

/* **************************************************************************************

										어검술

   ************************************************************************************** */

bool CharObject::DirectAttack(CharObject* pTargetObject, int nDamage)
{
	if(pTargetObject->m_wObjectType & _OBJECT_HUMAN)
		return false;

	if(IsProperTarget(pTargetObject))
	{
		if(rand() % pTargetObject->m_btSpeedPoint < m_btHitPoint)
		{
			pTargetObject->StruckDamage(nDamage);
			pTargetObject->AddDelayProcess(pTargetObject, RM_STRUCK, nDamage, pTargetObject->m_WAbility.HP, pTargetObject->m_WAbility.MaxHP, (LONG)this, NULL, 550);

			return TRUE;
		}
	}

	return false;
}

bool CharObject::SwordLongAttack(int nDamage)
{
	int nX = 0, nY = 0;

	if(GetNextPosition(m_nDirection, 2, nX, nY))
	{
		CharObject* pTargetObject = m_pMap->GetObject(nX, nY);

		if(pTargetObject && (nDamage > 0))
		{
			if(IsProperTarget(pTargetObject))
			{
				DirectAttack(pTargetObject, nDamage);
				SelectTarget(pTargetObject);

					// Debug Code
#ifdef _DEBUG
					char szMsg[64];

					sprintf(szMsg, "%s 어검 맞음 - PW:%d HP:%d", pTargetObject->m_szName, nDamage, pTargetObject->m_WAbility.HP);
					SysMsg(szMsg, 0);
					// Debug Code
#endif
				return TRUE;
			}
		}
	}

	return false;
}

/* **************************************************************************************

										반월검법

   ************************************************************************************** */

bool CharObject::SwordWideAttack(int nDamage)
{
	static int WideSearchPos[8] = { 6, 7, 0, 1, 2, 3, 4, 5 };
	static int WideAttack[3] = { 6, 7, 1 };

	int nDir;
	int nX, nY;

	for (int i = 0; i < 3; i++)
	{
		nDir = (m_nDirection + WideAttack[i]) % 8;

		if(GetNextPosition(nDir, 1, nX, nY))
		{
			CharObject* pTargetObject = m_pMap->GetObject(nX, nY);

			if(pTargetObject && (nDamage > 0))
			{
				if(IsProperTarget(pTargetObject))
				{
					DirectAttack(pTargetObject, nDamage);
					SelectTarget(pTargetObject);

					// Debug Code
#ifdef _DEBUG
					char szMsg[64];

					sprintf(szMsg, "%s 반월 맞음 - PW:%d HP:%d", pTargetObject->m_szName, nDamage, pTargetObject->m_WAbility.HP);
					SysMsg(szMsg, 0);
					// Debug Code
#endif
					return TRUE;
				}
			}
		}
	}

	return false;
}

/* **************************************************************************************

										뢰설화

   ************************************************************************************** */

bool CharObject::MagicElecBlizzard(int nPower)
{
	bool					fFlag = false;
	CWHList<CharObject*>	ObjectList;

	m_pMap->GetMapObject(m_nCurrX, m_nCurrY, 2, &ObjectList);

	if(ObjectList.GetCount())
	{
		PLISTNODE		pListNode = ObjectList.GetHead();
		CharObject*	pTargetObject;

		while (pListNode)
		{
			if(pTargetObject = ObjectList.GetData(pListNode))
			{
				if(pTargetObject->m_wObjectType & _OBJECT_ANIMAL)
				{
					if(((MonsterObject*)pTargetObject)->m_btLifeAttrib)
						pTargetObject->AddProcess(this, RM_MAGSTRUCK, 0, nPower, 0, 0);
					else
						pTargetObject->AddProcess(this, RM_MAGSTRUCK, 0, nPower / 10, 0, 0);

					fFlag = TRUE;
				}
			}
		
			pListNode = ObjectList.RemoveNode(pListNode);
		}
	}

	return fFlag;
}

/* **************************************************************************************

									빙설풍, 폭열파

   ************************************************************************************** */

bool CharObject::MagicBigExplosion(int nPower, int nX, int nY, int nWide)
{
	bool					fFlag = false;
	CWHList<CharObject*>	ObjectList;

	m_pMap->GetMapObject(nX, nY, nWide, &ObjectList);

	if(ObjectList.GetCount())
	{
		PLISTNODE		pListNode = ObjectList.GetHead();
		CharObject*	pTargetObject;

		while (pListNode)
		{
			if(pTargetObject = ObjectList.GetData(pListNode))
			{
				if(IsProperTarget(pTargetObject))
				{
					SelectTarget(pTargetObject);
					pTargetObject->AddProcess(this, RM_MAGSTRUCK, 0, nPower, 0, 0);
					fFlag = TRUE;
				}
			}
		
			pListNode = ObjectList.RemoveNode(pListNode);
		}
	}

	return fFlag;
}

/* **************************************************************************************

										대회복술

   ************************************************************************************** */

bool CharObject::MagicBigHealing(int nPower, int nX, int nY)
{
	bool					fFlag = false;
	CWHList<CharObject*>	ObjectList;

	m_pMap->GetMapObject(nX, nY, 1, &ObjectList);

	if(ObjectList.GetCount())
	{
		PLISTNODE		pListNode = ObjectList.GetHead();
		CharObject*	pTargetObject;

		while (pListNode)
		{
			if(pTargetObject = ObjectList.GetData(pListNode))
			{
				if(IsProperFriend(pTargetObject))
				{
					if(pTargetObject->m_WAbility.HP < pTargetObject->m_WAbility.MaxHP)
					{
						pTargetObject->AddDelayProcess(this, RM_MAGHEALING, 0, nPower, 0, 0, NULL, 800);
						fFlag = TRUE;
					}
				}
			}
		
			pListNode = ObjectList.RemoveNode(pListNode);
		}
	}

	return fFlag;
}

// nState: STATE_MAGICDEFENCEUP
//         STATE_DEFENCEUP
bool CharObject::MagicMakeDefenceUp(int nState, int nSec)
{
    // 1. set new defense state
    m_StateAttrV[nState] = std::max(m_StateAttrV[nState], nSec);

    // 2. use gui show info as: ``defense up for xxx sec"
    extern MonoServer *g_MonoServer;

    // 3. recaluclate ability attributes

    if(nState == STATE_DEFENCEUP){
        g_MonoServer->AddLog(LOGTYPE_INFO, "Defense up for %d sec", nSec);
    }else if(nState == STATE_MAGDEFENCEUP){
        g_MonoServer->AddLog(LOGTYPE_INFO, "Magic defense up for %d sec", nSec);
    }

	wsprintf(wszMsg2, wszMsg, nSec);
	WideCharToMultiByte(CP_ACP, 0, wszMsg2, -1, szMsg, sizeof(szMsg), NULL, NULL);
	SysMsg(szMsg, 1);


    if(Human()){
        ((CPlayerObject*)this)->CalculateAbility();
        ((CPlayerObject*)this)->SendAbility();
    }

    // 4. make log
}

bool CharObject::MagicMakeDefenceArea(int nX, int nY, int nRange, int nSec, int nState)
{
	int nStartX = nX - nRange;
	int nEndX	= nX + nRange;
	int nStartY = nY - nRange;
	int nEndY	= nY + nRange;
	int nCount	= 0;

	__try
	{
		for (int x = nStartX; x <= nEndX; x++)
		{
			for (int y = nStartY; y <= nEndY; y++)
			{
				if(CMapCellInfo* pMapCellInfo = m_pMap->GetMapCellInfo(x, y))
				{
					if(pMapCellInfo->m_xpObjectList)
					{
						if(pMapCellInfo->m_xpObjectList->GetCount())
						{
							PLISTNODE pListNode = pMapCellInfo->m_xpObjectList->GetHead();

							while (pListNode)
							{
								_LPTOSOBJECT pOSObject = pMapCellInfo->m_xpObjectList->GetData(pListNode);

								if(pOSObject)
								{
									if(pOSObject->btType == OS_MOVINGOBJECT)
									{
										CharObject* pCharObject = (CharObject*)pOSObject->pObject;

										if(!pCharObject->m_fIsGhost)
										{
											if(IsProperFriend(pCharObject))
											{
												pCharObject->MagicDefenceUp(nState, nSec);
												nCount++;
											}
										}
									}
								}
								
								pListNode = pMapCellInfo->m_xpObjectList->GetNext(pListNode);
							} // while (pListNode)
						} // if(pMapCellInfo->m_pObjectList.GetCount())
					}
				} // if(pMapCellInfo)
			}// for (y)
		} // for (x)
	}
	__finally
	{
	}

	return nCount;
}

/* **************************************************************************************

										은신술

   ************************************************************************************** */

bool CharObject::MagicMakePrivateTransparent(int nHTime)
{
	if(m_StateAttrV[STATE_TRANSPARENT] > 0)
		return false;

	CWHList<CharObject*>	ObjectList;

	m_pMap->GetMapObject(m_nCurrX, m_nCurrY, 9, &ObjectList);

	if(ObjectList.GetCount())
	{
		PLISTNODE		pListNode = ObjectList.GetHead();
		CharObject*	pTargetObject;

		while (pListNode)
		{
			if(pTargetObject = ObjectList.GetData(pListNode))
			{
				if(pTargetObject->m_wObjectType & _OBJECT_ANIMAL)
				{
					if(pTargetObject->m_pTargetObject == this)
					{
						if((abs(pTargetObject->m_nCurrX - m_nCurrX) > 1) || (abs(pTargetObject->m_nCurrY - m_nCurrY) > 1) || (rand() % 2) == 0)
							pTargetObject->m_pTargetObject = NULL;
					}
				}
			}
		
			pListNode = ObjectList.RemoveNode(pListNode);
		}
	}

	m_StateAttrV[STATE_TRANSPARENT] = nHTime;

	m_nCharStatus = GetCharStatus();
	AddRefMsg(RM_CHARSTATUSCHANGED, m_sHitSpeed/*wparam*/, m_nCharStatus, 0, 0, NULL);

	m_fHumHideMode		= TRUE;
	m_fFixedHideMode	= TRUE;

	return false;
}

/* **************************************************************************************

										대은신술

   ************************************************************************************** */

bool CharObject::MagicMakeGroupTransparent(int nX, int nY, int nHTime)
{
	bool					fFlag = false;
	CWHList<CharObject*>	ObjectList;

	m_pMap->GetMapObject(nX, nY, 1, &ObjectList);

	if(ObjectList.GetCount())
	{
		PLISTNODE		pListNode = ObjectList.GetHead();
		CharObject*	pTargetObject;

		while (pListNode)
		{
			if(pTargetObject = ObjectList.GetData(pListNode))
			{
				if(IsProperFriend(pTargetObject))
				{
					if(pTargetObject->m_StateAttrV[STATE_TRANSPARENT] == 0)
					{
						AddDelayProcess(pTargetObject, RM_TRANSPARENT, 0, nHTime, 0, 0, NULL, 800);
						fFlag = TRUE;
					}
				}
			}
		
			pListNode = ObjectList.RemoveNode(pListNode);
		}
	}

	return fFlag;
}

bool CharObject::MagicKillUndead(CharObject* pTargetObject, int nLevel)
{
	MonsterObject *pMonsterObject = (MonsterObject*)pTargetObject;

	if(pMonsterObject->m_btLifeAttrib == LA_UNDEAD)
	{
		pMonsterObject->Struck(this);

		SelectTarget(pMonsterObject);

		if((pMonsterObject->m_Ability.Level < m_Ability.Level - 1 + (rand() % 4)) && (pMonsterObject->m_Ability.Level < 50))
		{
			int nlvGap = m_Ability.Level - pMonsterObject->m_Ability.Level;

			if(rand() % 100 < (15 + nLevel * 7 + nlvGap))
			{
				pMonsterObject->SetLastHiter(this);
				pMonsterObject->m_WAbility.HP = 0;
				return TRUE;
			}
		}
	}

	return false;
}

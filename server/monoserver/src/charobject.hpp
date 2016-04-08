

#pragma once


#include "../def/_orzex/queue.h"


#define DEFSPEED					14

#define _CHAT_COLOR1				0			//RGB(  0,   0,   0); // 투과색.
#define _CHAT_COLOR2				1			//RGB( 10,  10,  10); // 검정색.
#define _CHAT_COLOR3				2			//RGB(255, 255, 255); // 흰색.
#define _CHAT_COLOR4				3			//RGB(255,   0,   0); // 빨강.
#define _CHAT_COLOR5				4			//RGB(  0, 255,   0); // 녹색.
#define _CHAT_COLOR6				5			//RGB(  0,   0, 255); // 푸른색
#define _CHAT_COLOR7				6			//RGB(255, 255,   0); // 노랑색.
#define _CHAT_COLOR8				7			//RGB(255, 128,   0); // 주황색

#define DR_UP						0
#define DR_UPRIGHT					1
#define DR_RIGHT					2
#define DR_DOWNRIGHT				3
#define DR_DOWN						4
#define DR_DOWNLEFT					5
#define DR_LEFT						6
#define DR_UPLEFT					7

#define _DOOR_NOT					0
#define _DOOR_OPEN					1
#define _DOOR_MAPMOVE_FRONT			2
#define _DOOR_MAPMOVE_BACK			3

// Status
#define MAX_STATUS_ATTRIBUTE		12

#define POISON_DECHEALTH			0
#define POISON_DAMAGEARMOR			1
#define POISON_LOCKSPELL			2
#define POISON_DONTMOVE				4
#define POISON_STONE				5
#define STATE_TRANSPARENT			8
#define STATE_DEFENCEUP				9
#define STATE_MAGDEFENCEUP			10
#define STATE_BUBBLEDEFENCEUP		11

#define	STATE_STONE_MODE			0x00000001;
#define	STATE_OPENHEATH				0x00000002;  //체력 공개상태

class CCharObject;
class CMirMap;
class CUserInfo;

typedef struct tag_TPROCESSMSG
{
    WORD			wIdent;
    WORD			wParam;
    DWORD			lParam1;
    DWORD			lParam2;
    DWORD			lParam3;

    DWORD			dwDeliveryTime;

    CCharObject*	pCharObject;

    char			*pszData;
} _TPROCESSMSG, *_LPTPROCESSMSG;

/*
*/

#define _OBJECT_HUMAN			1
#define _OBJECT_MONSTER			2
#define _OBJECT_ANIMAL			6
#define _OBJECT_NPC				8

typedef struct tag_TOBJECTFEATURE
{
    BYTE		btGender;
    BYTE		btWear;
    BYTE		btHair;
    BYTE		btWeapon;
} _TOBJECTFEATURE, _LPTOBJECTFEATURE;

typedef struct tag_TOBJECTFEATUREEX
{
    BYTE		btHorse;
    WORD		dwHairColor;
    WORD		dwWearColor;
} _TOBJECTFEATUREEX, _LPTOBJECTFEATUREEX;

class CVisibleObject
{
    public:
        int				nVisibleFlag;
        CCharObject*	pObject;
};

class CVisibleEvent
{
    public:
        int				nVisibleFlag;
        CEvent*			pEvent;
};

class CVisibleMapItem
{
    public:
        int				nVisibleFlag;
        WORD			wX;
        WORD			wY;
        CMapItem*		pMapItem;
};

#pragma pack(1)
class CObjectAbility
{
    public:
        BYTE	Level;

        WORD	HP;
        WORD	MP;
        WORD	MaxHP;
        WORD	MaxMP;
        WORD	Weight;
        WORD	MaxWeight;

        DWORD	Exp;
        DWORD	MaxExp;

        BYTE	WearWeight;
        BYTE	MaxWearWeight;
        BYTE	HandWeight;
        BYTE	MaxHandWeight;

        WORD	DC;
        WORD	MC;
        WORD	SC;
        WORD	AC;
        WORD	MAC;

        WORD	m_wWater;
        WORD	m_wFire;
        WORD	m_wWind;
        WORD	m_wLight;
        WORD	m_wEarth;
};

class CObjectAddAbility	// 아이템 착용으로 늘어나는 능력치
{
    public:
        WORD	HP;
        WORD	MP;
        WORD	HIT;
        WORD	SPEED;
        WORD	AC;
        WORD	MAC;
        WORD	DC;
        WORD	MC;
        WORD	SC;
        WORD	AntiPoison;
        WORD	PoisonRecover;
        WORD	HealthRecover;
        WORD	SpellRecover;
        WORD	AntiMagic;			//마법 회피율
        BYTE	Luck;				//행운 포인트
        BYTE	UnLuck;				//불행 포인트
        BYTE	WeaponStrong;
        short	HitSpeed;
};
#pragma pack(8)

class CCharObject
{
    public:
        // 1. won't care the return type of the task

            CharObject(UserInfo* pUserInfo)
                : m_UserInfo(pUserInfo)
                  , m_CurrX(0)
                  , m_CurrX(0)
                  , m_Name("")
                  , m_HealthTick(0)
                  , m_SpellTick(0)
                  , m_TickSec(0)
                  , m_OpenHealth(false)
                  , m_CharStatusEx(false)
                  , m_CharStatus(0)
                  , m_IncHealing(0)
                  , m_IncHealth(0)
                  , m_IncSpell(0)
                  , m_PerHealing(5)
                  , m_PerHealth(5)
                  , m_PerSpell(5)
                  , m_IncHealthSpellTime(0)
                  , m_Event(-1)
                  , m_HitSpeed(0)
                  , m_AbilityMagicBubbleDefence(0)
                  , m_MagicBubbleDefenceLevel(0)
                  , m_Inspector(false)
                  , m_FastTrain(false)
                  , m_Ghost(false)
                  , m_IsNeverDie(false)
                  , m_SpeedPoint(SYS_DEFAULTSPEED)
                  , m_HumanHideMode(false)
                  , m_FixedHideMode(false)
                  , m_StoneMode(false)
                  , m_StickMode(false)
                  , m_MasterObject(nullptr)
                  , m_TargetObject(nullptr)
            {
                extern MonoServer *g_MonoServer;
                m_TickSec = g_MonoServer->GetTickCount();
                m_StateAttrV.fill(0);
            }

            public:
            CUserInfo*					m_pUserInfo;

            CWHQueue					m_ProcessQ;
            CWHQueue					m_DelayProcessQ;

            int							m_nCurrX;
            int							m_nCurrY;
            int							m_nDirection;

            int							m_nEvent;

            BYTE						m_btLight;

            _TOBJECTFEATURE				m_tFeature;
            _TOBJECTFEATUREEX			m_tFeatureEx;

            CMirMap*					m_pMap;

            WORD						m_wObjectType;

            CWHList<CVisibleObject*>	m_xVisibleObjectList;
            CWHList<CVisibleMapItem*>	m_xVisibleItemList;
            CWHList<CVisibleEvent*>		m_xVisibleEventList;
            CWHList<CCharObject*>		m_xCacheObjectList;
            DWORD						m_dwCacheTick;

            int							m_nViewRange;
            DWORD						m_dwSearchTime;
            DWORD						m_dwSearchTick;

            DWORD						m_dwLatestHitTime;
            UINT						m_nHitTimeOverCount;

            char						m_szName[14];

            CObjectAbility				m_Ability;
            CObjectAbility				m_WAbility;
            CObjectAddAbility			m_AddAbility;

            UINT						m_nCharStatusEx;
            UINT						m_nCharStatus;

            short						m_sHitSpeed;

            int							m_nHitDouble;

            DWORD						m_dwWalkTime;

            WORD						m_wStatusArr[MAX_STATUS_ATTRIBUTE];
            DWORD						m_dwStatusTime[MAX_STATUS_ATTRIBUTE];

            BYTE						m_btHitPlus;

            BYTE						m_btAntiMagic;
            BYTE						m_btHitPoint;
            BYTE						m_btSpeedPoint;
            BYTE						m_btAntiPoison;
            BYTE						m_btPoisonRecover;
            BYTE						m_btHealthRecover;
            BYTE						m_btSpellRecover;

            BOOL						m_fIsDead;
            DWORD						m_dwDeathTime;
            BOOL						m_fIsGhost;
            DWORD						m_dwGhostTime;

            BOOL						m_fOpenHealth;
            DWORD						m_dwOpenHealthStart;
            DWORD						m_dwOpenHealthTime;
            DWORD						m_dwIncHealthSpellTime;

            DWORD						m_dwHealthTick;
            DWORD						m_dwSpellTick;
            DWORD						m_dwTickSec;

            WORD						m_IncHealing;
            BYTE						m_btPerHealing;
            WORD						m_IncHealth;
            BYTE						m_btPerHealth;
            WORD						m_IncSpell;
            BYTE						m_btPerSpell;

            BOOL						m_fAbilMagBubbleDefence;
            BYTE						m_btMagBubbleDefenceLevel;

            BOOL						m_fInspector;
            BOOL						m_fFastTrain;
            BOOL						m_fHideMode;
            BOOL						m_fIsNeverDie;
            BOOL						m_fStoneMode;
            BOOL						m_fStickMode;

            CCharObject*				m_pMasterObject;
            CCharObject*				m_pTargetObject;
            CCharObject*				m_pLastHitterObject;
            CCharObject*				m_pExpHitterObject;
            DWORD						m_dwTargetFocusTime;

            BOOL						m_fHumHideMode;
            BOOL						m_fFixedHideMode;

            public:
            CCharObject(CUserInfo*	pUserInfo);
            virtual ~CCharObject();

            BOOL	GetBackPosition(int &nX, int &nY);
            int		GetBack(int nDirection);
            void	GetFrontPosition(int &nX, int &nY);

            void	UpdateDelayProcessCheckParam1(CCharObject* pCharObject, WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData, int nDelay);
            void	UpdateProcess(CCharObject* pCharObject, WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData);
            void	AddProcess(CCharObject* pCharObject, WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData = NULL);
            void	AddDelayProcess(CCharObject* pCharObject, WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData, int nDelay);
            void	AddRefMsg(WORD wIdent, WORD wParam, DWORD lParam1, DWORD lParam2, DWORD lParam3, char *pszData);

            BOOL	GetNextPosition(int nSX, int nSY, int nDir, int nDistance, int& nX, int& nY);
            __inline BOOL	GetNextPosition(int nDir, int nDistance, int& nX, int& nY)
            { return GetNextPosition(m_nCurrX, m_nCurrY, nDir, nDistance, nX, nY); }
            int		GetNextDirection(int nStartX, int nStartY, int nTargetX, int nTargetY);
            __inline int	GetNextDirection(int nTargetX, int nTargetY)
            { return GetNextDirection(m_nCurrX, m_nCurrY, nTargetX, nTargetY); }

            void	UpdateVisibleObject(CCharObject* pCharObject);
            void	UpdateVisibleItem(int nX, int nY, CMapItem* pMapItem);
            void	UpdateVisibleEvent(CEvent* pEvent);

            void	SelectTarget(CCharObject* pTargetObject);

            void	IncHealthSpell(int nHP, int nMP);
            void	DamageSpell(int nVal);
            void	DamageHealth(int nDamage);
            int		GetMagStruckDamage(int nDamage);
            void	StruckDamage(int nDamage);
            int		GetHitStruckDamage(int nDamage);
            int		GetAttackPower(int nDamage, int nVal);
            BOOL	_Attack(WORD wHitMode, CCharObject* pObject);

            void	WalkNextPos(int nDir, int& nX, int& nY);
            void	TurnTo(int nDir);
            BOOL	TurnXY(int nX, int nY, int nDir);
            virtual BOOL	WalkTo(int nDir);
            BOOL	WalkXY(int nX, int nY, int nDir);
            BOOL	RunTo(int nDir);
            BOOL	RunXY(int nX, int nY, int nDir);
            BOOL	HitXY(WORD wIdent, int nX, int nY, int nDir, int nHitStyle);

            void	DoDamageWeapon(int nDamage);

            void	Disappear();
            void	MakeGhost();

            BOOL	DropItemDown(_LPTUSERITEMRCD lpTItemRcd, int nRange, BOOL fIsGenItem);

            CCharObject* AddCreatureSysop(int nX, int nY, CMonRaceInfo* pMonRaceInfo, BOOL fSearch);

            void	DoPushed(int nDirection);
            void	DoMotaebo();
            int		CharPushed(int nDir, int nPushCount);
            BOOL	DoShowHP(CCharObject* pObject, CMagicInfo* pMagicInfo, int nLevel);
            void	MakeOpenHealth();
            void	BreakOpenHealth();
            int		MagPushArround(int nPushLevel);
            int		MagPassThroughMagic(int nStartX, int nStartY, int nTargetX, int nTargetY, int nDir, int nPwr, BOOL fUndeadAttack);
            BOOL	MagBubbleDefenceUp(int nLevel, int nSec);
            void	DamageBubbleDefence();
            int		MagMakeFireCross(int nDamage, int nHTime, int nX, int nY);
            BOOL	DirectAttack(CCharObject* pTargetObject, int nDamage);
            BOOL	SwordLongAttack(int nDamage);
            BOOL	SwordWideAttack(int nDamage);
            BOOL	MagBigExplosion(int nPower, int nX, int nY, int nWide);
            BOOL	MagBigHealing(int nPower, int nX, int nY);
            BOOL	MagDefenceUp(int nState, int nSec);
            BOOL	MagMakeDefenceArea(int nX, int nY, int nRange, int nSec, int nState);
            BOOL	MagElecBlizzard(int nPower);
            BOOL	MagMakePrivateTransparent(int nHTime);
            int		MagMakeHolyCurtain(int nHTime, int nX, int nY);
            BOOL	MagMakeGroupTransparent(int nX, int nY, int nHTime);
            BOOL	MagTurnUndead(CCharObject* pTargetObject, int nX, int nY, int nLevel);

            virtual BOOL	IsProperTarget(CCharObject* pCharObject);
            BOOL	IsFriend(CCharObject* pCharObject);
            BOOL	IsProperFriend(CCharObject* pCharObject);

            void	SpaceMove(int nX, int nY, CMirMap* pMirMap);
            void	GoldChanged();
            void	HealthSpellChanged();

            BOOL	GetAvailablePosition(CMirMap* pMap, int &nX, int &nY, int nRange);

            void	GetQueryUserName(CCharObject* pObject, int nX, int nY);

            void	SendSocket(_LPTDEFAULTMESSAGE lpDefMsg, char *pszPacket);

            void	Die();

            BOOL	RestoreHealSpell();

            UINT	GetCharStatus();

            virtual WORD	GetThisCharColor() = 0;
            virtual void	GetCharName(char *pszCharName) = 0;
            virtual void	Run();
            virtual void	SearchViewRange();

            CCharObject*	GetFrontObject();

            __inline void SysMsg(char *pszMsg, int nMode)
                /* { ( nMode == 1 ? AddProcess(this, RM_SYSMESSAGE2, 0, 0, 0, 0, pszMsg) : AddProcess(this, RM_SYSMESSAGE, 0, 0, 0, 0, pszMsg)); } */
            {
                if(nMode == 1){
                    AddProcess(this, RM_SYSMESSAGE2, 0, 0, 0, 0, pszMsg);
                }else{
                    AddProcess(this, RM_SYSMESSAGE, 0, 0, 0, 0, pszMsg);
                }
            }
            __inline LONG GetFeatureToLong()
            { return (LONG) MAKELONG(MAKEWORD(m_tFeature.btGender, m_tFeature.btWear), MAKEWORD(m_tFeature.btHair, m_tFeature.btWeapon)); }
            __inline BYTE GetRPow(WORD wPower)
            { if (HIBYTE(wPower) > LOBYTE(wPower)) return LOBYTE(wPower) + (rand() % (HIBYTE(wPower) - LOBYTE(wPower) + 1)); else return LOBYTE(wPower); }

            public:


        };

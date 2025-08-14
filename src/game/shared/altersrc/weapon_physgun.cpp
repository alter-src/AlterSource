//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "beam_shared.h"
#ifndef CLIENT_DLL
#include "player.h"
#endif
#include "gamerules.h"
#ifdef CLIENT_DLL
#include "clienteffectprecachesystem.h"
#endif
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#ifndef CLIENT_DLL
#include "baseviewmodel.h"
#endif
#include "vphysics/constraints.h"
#include "physics.h"
#include "in_buttons.h"
#include "IEffects.h"
#include "soundenvelope.h"
#include "engine/IEngineSound.h"
#ifndef CLIENT_DLL
#include "ndebugoverlay.h"
#endif
#include "physics_saverestore.h"
#ifndef CLIENT_DLL
#include "player_pickup.h"
#endif
#include "SoundEmitterSystem/isoundemittersystembase.h"
#ifdef CLIENT_DLL
#include "model_types.h"
#include "view_shared.h"
#include "view.h"
#include "iviewrender.h"
#include "ragdoll.h"
#else
#include "physics_prop_ragdoll.h"
#endif

#ifdef CLIENT_DLL
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialproxy.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static int g_physgunBeam1;
static int g_physgunBeam;
static int g_physgunGlow;

#define PHYSGUN_BEAM_SPRITE1	"sprites/physbeam1.vmt"
#define PHYSGUN_BEAM_SPRITE		"sprites/physbeam.vmt"
#define PHYSGUN_BEAM_GLOW		"sprites/physglow.vmt"

#define	PHYSGUN_SKIN	1

class CWeaponPhysicsGun;

#ifdef CLIENT_DLL
CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectGravityGun )
	CLIENTEFFECT_MATERIAL( PHYSGUN_BEAM_SPRITE1 )
	CLIENTEFFECT_MATERIAL( PHYSGUN_BEAM_SPRITE )
	CLIENTEFFECT_MATERIAL( PHYSGUN_BEAM_GLOW )
CLIENTEFFECT_REGISTER_END()

ConVar physgun_r("physgun_r", "0", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_CLIENTDLL );
ConVar physgun_g("physgun_g", "229", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_CLIENTDLL );
ConVar physgun_b("physgun_b", "238", FCVAR_USERINFO |  FCVAR_ARCHIVE | FCVAR_CLIENTDLL );
#endif

IPhysicsObject *GetPhysObjFromPhysicsBone( CBaseEntity *pEntity, short physicsbone )
{
	if( pEntity->IsNPC() )
	{
		return pEntity->VPhysicsGetObject();
	}

	CBaseAnimating *pModel = static_cast< CBaseAnimating * >( pEntity );
	if ( pModel != NULL )
	{
		IPhysicsObject	*pPhysicsObject = NULL;
		
		//Find the real object we hit.
		if( physicsbone >= 0 )
		{
#ifdef CLIENT_DLL
			if ( pModel->m_pRagdoll )
			{
				CRagdoll *pCRagdoll = dynamic_cast < CRagdoll * > ( pModel->m_pRagdoll );
#else
				// Affect the object
				CRagdollProp *pCRagdoll = dynamic_cast<CRagdollProp*>( pEntity );
#endif
				if ( pCRagdoll )
				{
					ragdoll_t *pRagdollT = pCRagdoll->GetRagdoll();

					if ( physicsbone < pRagdollT->listCount )
					{
						pPhysicsObject = pRagdollT->list[physicsbone].pObject;
					}
					return pPhysicsObject;
				}
#ifdef CLIENT_DLL
			}
#endif
		}
	}

	return pEntity->VPhysicsGetObject();
}

class CGravControllerPoint : public IMotionEvent
{
	DECLARE_SIMPLE_DATADESC();

public:
	CGravControllerPoint( void );
	~CGravControllerPoint( void );
	void AttachEntity( CBasePlayer *pPlayer, CBaseEntity *pEntity, IPhysicsObject *pPhys, short physicsbone, const Vector &position );
	void DetachEntity( void );

	bool UpdateObject( CBasePlayer *pPlayer, CBaseEntity *pEntity );

	void SetTargetPosition( const Vector &target, const QAngle &targetOrientation )
	{
		m_shadow.targetPosition = target;
		m_shadow.targetRotation = targetOrientation;

		m_timeToArrive = gpGlobals->frametime;

		CBaseEntity *pAttached = m_attachedEntity;
		if ( pAttached )
		{
			IPhysicsObject *pObj = GetPhysObjFromPhysicsBone( pAttached, m_attachedPhysicsBone );
			
			if ( pObj != NULL )
			{
				pObj->Wake();
			}
			else
			{
				DetachEntity();
			}
		}
	}
	QAngle TransformAnglesToPlayerSpace( const QAngle &anglesIn, CBasePlayer *pPlayer );
	QAngle TransformAnglesFromPlayerSpace( const QAngle &anglesIn, CBasePlayer *pPlayer );

	IMotionEvent::simresult_e Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular );
	Vector			m_localPosition;
	Vector			m_targetPosition;
	Vector			m_worldPosition;
	float			m_saveDamping;
	float			m_saveMass;
	float			m_maxAcceleration;
	Vector			m_maxAngularAcceleration;
	EHANDLE			m_attachedEntity;
	short			m_attachedPhysicsBone;
	QAngle			m_targetRotation;
	float			m_timeToArrive;

#if 1
	// adnan
	// set up the modified pickup angles... allow the player to rotate the object in their grip
	QAngle		m_vecRotatedCarryAngles;
	bool			m_bHasRotatedCarryAngles;
	// end adnan
#endif

	IPhysicsMotionController *m_controller;

private:
	hlshadowcontrol_params_t	m_shadow;
};

#ifdef CLIENT_DLL
float fadeSpeed = 0.5f;
bool fadingOut = true;
static float lastPhysgunR = -1.0f, lastPhysgunG = -1.0f, lastPhysgunB = -1.0f;

class PlayerWeaponColorProxy : public IMaterialProxy
{
public:
	virtual bool Init(IMaterial* pMaterial, KeyValues* pKeyValues);
	virtual void OnBind(void* pC_BaseEntity);
	virtual void Release();
	virtual IMaterial* GetMaterial();

private:
	IMaterial* m_pMaterial;
	IMaterialVar* m_pResultVar;
};

bool PlayerWeaponColorProxy::Init(IMaterial* pMaterial, KeyValues* pKeyValues)
{
	bool foundVar;
	m_pResultVar = pMaterial->FindVar("$selfillumtint", &foundVar, false);
	m_pMaterial = pMaterial;
	return foundVar;
}

void PlayerWeaponColorProxy::OnBind(void* pC_BaseEntity)
{
	if (m_pResultVar)
	{
		float currentR = physgun_r.GetFloat();
		float currentG = physgun_g.GetFloat();
		float currentB = physgun_b.GetFloat();

		float targetRed = currentR + 50.0f;
		float targetGreen = currentG + 50.0f;
		float targetBlue = currentB + 50.0f;

		static float red = currentR, green = currentG, blue = currentB;

		if (currentR != lastPhysgunR || currentG != lastPhysgunG || currentB != lastPhysgunB)
		{
			red = currentR;
			green = currentG;
			blue = currentB;
			lastPhysgunR = currentR;
			lastPhysgunG = currentG;
			lastPhysgunB = currentB;
		}

		if (fadingOut)
		{
			if (red < targetRed)
				red += fadeSpeed;
			if (green < targetGreen)
				green += fadeSpeed;
			if (blue < targetBlue)
				blue += fadeSpeed;
			if (red >= targetRed && green >= targetGreen && blue >= targetBlue)
				fadingOut = false;
		} else {
			if (red > currentR)
				red -= fadeSpeed;
			if (green > currentG)
				green -= fadeSpeed;
			if (blue > currentB)
				blue -= fadeSpeed;
			if (red <= currentR && green <= currentG && blue <= currentB)
				fadingOut = true;
		}

		float r = red / 255.0f;
		float g = green / 255.0f;
		float b = blue / 255.0f;

		m_pResultVar->SetVecValue(r, g, b);
	}
}

void PlayerWeaponColorProxy::Release()
{}

IMaterial* PlayerWeaponColorProxy::GetMaterial()
{
	return m_pMaterial;
}

EXPOSE_INTERFACE(PlayerWeaponColorProxy, IMaterialProxy, "PlayerWeaponColor" IMATERIAL_PROXY_INTERFACE_VERSION);
#endif

BEGIN_SIMPLE_DATADESC( CGravControllerPoint )

	DEFINE_FIELD( m_localPosition,		FIELD_VECTOR ),
	DEFINE_FIELD( m_targetPosition,		FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_worldPosition,		FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_saveDamping,			FIELD_FLOAT ),
	DEFINE_FIELD( m_saveMass,			FIELD_FLOAT ),
	DEFINE_FIELD( m_maxAcceleration,		FIELD_FLOAT ),
	DEFINE_FIELD( m_maxAngularAcceleration,	FIELD_VECTOR ),
	DEFINE_FIELD( m_attachedEntity,		FIELD_EHANDLE ),
	DEFINE_FIELD( m_attachedPhysicsBone,		FIELD_SHORT ),
	DEFINE_FIELD( m_targetRotation,		FIELD_VECTOR ),
	DEFINE_FIELD( m_timeToArrive,			FIELD_FLOAT ),
#if 1
	// adnan
	// set up the fields for our added vars
	DEFINE_FIELD( m_vecRotatedCarryAngles, FIELD_VECTOR ),
	DEFINE_FIELD( m_bHasRotatedCarryAngles, FIELD_BOOLEAN ),
	// end adnan
#endif

	// Physptrs can't be saved in embedded classes... this is to silence classcheck
	// DEFINE_PHYSPTR( m_controller ),

END_DATADESC()


CGravControllerPoint::CGravControllerPoint( void )
{
	m_shadow.dampFactor = 0.8;
	m_shadow.teleportDistance = 0;
	// make this controller really stiff!
	m_shadow.maxSpeed = 5000;
	m_shadow.maxAngular = m_shadow.maxSpeed;
	m_shadow.maxDampSpeed = m_shadow.maxSpeed*2;
	m_shadow.maxDampAngular = m_shadow.maxAngular*2;
	m_attachedEntity = NULL;
	m_attachedPhysicsBone = 0;

#if 1
	// adnan
	// initialize our added vars
	m_vecRotatedCarryAngles = vec3_angle;
	m_bHasRotatedCarryAngles = false;
	// end adnan
#endif
}

CGravControllerPoint::~CGravControllerPoint( void )
{
	DetachEntity();
}


QAngle CGravControllerPoint::TransformAnglesToPlayerSpace( const QAngle &anglesIn, CBasePlayer *pPlayer )
{
	matrix3x4_t test;
	QAngle angleTest = pPlayer->EyeAngles();
	angleTest.x = 0;
	AngleMatrix( angleTest, test );
	return TransformAnglesToLocalSpace( anglesIn, test );
}

QAngle CGravControllerPoint::TransformAnglesFromPlayerSpace( const QAngle &anglesIn, CBasePlayer *pPlayer )
{
	matrix3x4_t test;
	QAngle angleTest = pPlayer->EyeAngles();
	angleTest.x = 0;
	AngleMatrix( angleTest, test );
	return TransformAnglesToWorldSpace( anglesIn, test );
}


void CGravControllerPoint::AttachEntity( CBasePlayer *pPlayer, CBaseEntity *pEntity, IPhysicsObject *pPhys, short physicsbone, const Vector &vGrabPosition )
{
	m_attachedEntity = pEntity;
	m_attachedPhysicsBone = physicsbone;
	pPhys->WorldToLocal( &m_localPosition, vGrabPosition );
	m_worldPosition = vGrabPosition;
	pPhys->GetDamping( NULL, &m_saveDamping );
	m_saveMass = pPhys->GetMass();
	float damping = 2;
	pPhys->SetDamping( NULL, &damping );
	pPhys->SetMass( 50000 );
	m_controller = physenv->CreateMotionController( this );
	m_controller->AttachObject( pPhys, true );
	Vector position;
	QAngle angles;
	pPhys->GetPosition( &position, &angles );
	SetTargetPosition( vGrabPosition, angles );
	m_targetRotation = TransformAnglesToPlayerSpace( angles, pPlayer );
#if 1
	// adnan
	// we need to grab the preferred/non preferred carry angles here for the rotatedcarryangles
	m_vecRotatedCarryAngles = m_targetRotation;
	// end adnan
#endif
}

void CGravControllerPoint::DetachEntity( void )
{
	CBaseEntity *pEntity = m_attachedEntity;
	if ( pEntity )
	{
		IPhysicsObject *pPhys = GetPhysObjFromPhysicsBone( pEntity, m_attachedPhysicsBone );
		if ( pPhys )
		{
			// on the odd chance that it's gone to sleep while under anti-gravity
			pPhys->Wake();
			pPhys->SetDamping( NULL, &m_saveDamping );
			pPhys->SetMass( m_saveMass );
		}
	}
	m_attachedEntity = NULL;
	m_attachedPhysicsBone = 0;
	if ( physenv )
	{
		physenv->DestroyMotionController( m_controller );
	}
	m_controller = NULL;

	// UNDONE: Does this help the networking?
	m_targetPosition = vec3_origin;
	m_worldPosition = vec3_origin;
}

void AxisAngleQAngle( const Vector &axis, float angle, QAngle &outAngles )
{
	// map back to HL rotation axes
	outAngles.z = axis.x * angle;
	outAngles.x = axis.y * angle;
	outAngles.y = axis.z * angle;
}

IMotionEvent::simresult_e CGravControllerPoint::Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular )
{
	hlshadowcontrol_params_t shadowParams = m_shadow;
#ifndef CLIENT_DLL
	m_timeToArrive = pObject->ComputeShadowControl( shadowParams, m_timeToArrive, deltaTime );
#else
	m_timeToArrive = pObject->ComputeShadowControl( shadowParams, (TICK_INTERVAL*2), deltaTime );
#endif
	
	linear.Init();
	angular.Init();

	return SIM_LOCAL_ACCELERATION;
}


#ifdef CLIENT_DLL
#define CWeaponPhysicsGun C_WeaponPhysicsGun
#endif

class CWeaponPhysicsGun : public CBaseHL2MPCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponPhysicsGun, CBaseHL2MPCombatWeapon );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponPhysicsGun();

#ifdef CLIENT_DLL
	void GetRenderBounds( Vector& mins, Vector& maxs )
	{
		BaseClass::GetRenderBounds( mins, maxs );

		// add to the bounds, don't clear them.
		// ClearBounds( mins, maxs );
		AddPointToBounds( vec3_origin, mins, maxs );
		AddPointToBounds( m_targetPosition, mins, maxs );
		AddPointToBounds( m_worldPosition, mins, maxs );
		CBaseEntity *pEntity = GetBeamEntity();
		if ( pEntity )
		{
			mins -= pEntity->GetRenderOrigin();
			maxs -= pEntity->GetRenderOrigin();
		}
	}

	void GetRenderBoundsWorldspace( Vector& mins, Vector& maxs )
	{
		BaseClass::GetRenderBoundsWorldspace( mins, maxs );

		// add to the bounds, don't clear them.
		// ClearBounds( mins, maxs );
		AddPointToBounds( vec3_origin, mins, maxs );
		AddPointToBounds( m_targetPosition, mins, maxs );
		AddPointToBounds( m_worldPosition, mins, maxs );
		mins -= GetRenderOrigin();
		maxs -= GetRenderOrigin();
	}

	int KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
	{
		if ( gHUD.m_iKeyBits & IN_ATTACK )
		{
			switch ( keynum )
			{
			case MOUSE_WHEEL_UP:
				m_bInWeapon1 = true;
				// gHUD.m_iKeyBits |= IN_WEAPON1;
				if ( gpGlobals->maxClients > 1 )
					gHUD.m_bSkipClear = true;
				return 0;

			case MOUSE_WHEEL_DOWN:
				m_bInWeapon2 = true;
				// gHUD.m_iKeyBits |= IN_WEAPON2;
				if ( gpGlobals->maxClients > 1 )
					gHUD.m_bSkipClear = true;
				return 0;
			}
		}

		// Allow engine to process
		return BaseClass::KeyInput( down, keynum, pszCurrentBinding );
	}

	void HandleInput()
	{
		if ( m_bInWeapon1 )
		{
			gHUD.m_iKeyBits |= IN_WEAPON1;
			m_bInWeapon1 = false;
		}

		if ( m_bInWeapon2 )
		{
			gHUD.m_iKeyBits |= IN_WEAPON2;
			m_bInWeapon2 = false;
		}
	}

	int	 DrawModel( int flags );
	void ViewModelDrawn( C_BaseViewModel *pBaseViewModel );
	bool IsTransparent( void );

	// We need to render opaque and translucent pieces
	RenderGroup_t	GetRenderGroup( void ) {	return RENDER_GROUP_TWOPASS;	}
#endif

	void Spawn( void );
	void OnRestore( void );
	void Precache( void );

#if 1
	// adnan
	// for overriding the mouse -> view angles (but still calc view angles)
	bool OverrideViewAngles( void );
	// end adnan
#endif

	virtual void	UpdateOnRemove(void);
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void ItemPreFrame( void );
	void ItemPostFrame( void );
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo )
	{
		EffectDestroy();
		SoundDestroy();
		return BaseClass::Holster( pSwitchingTo );
	}

	bool Reload( void );
	void Drop(const Vector &vecVelocity)
	{
		EffectDestroy();
		SoundDestroy();

#ifndef CLIENT_DLL
		UTIL_Remove( this );
#endif
	}

	bool HasAnyAmmo( void );

	void AttachObject( CBaseEntity *pEdict, IPhysicsObject *pPhysics, short physicsbone, const Vector& start, const Vector &end, float distance );
	void UpdateObject( void );
	void DetachObject( void );

	void TraceLine( trace_t *ptr );

	void EffectCreate( void );
	void EffectUpdate( void );
	void EffectDestroy( void );

	void SoundCreate( void );
	void SoundDestroy( void );
	void SoundStop( void );
	void SoundStart( void );
	void SoundUpdate( void );

	int ObjectCaps( void ) 
	{ 
		int caps = BaseClass::ObjectCaps();
		if ( m_active )
		{
			caps |= FCAP_DIRECTIONAL_USE;
		}
		return caps;
	}

	CBaseEntity *GetBeamEntity();

	void OpenElements( void );
	void CloseElements( void );

private:
	CNetworkVar( int, m_active );
	bool		m_useDown;
	CNetworkHandle( CBaseEntity, m_hObject );
	CNetworkVar( int, m_physicsBone );
	float		m_distance;
	float		m_movementLength;
	int			m_soundState;
	Vector		m_originalObjectPosition;
	CNetworkVector	( m_targetPosition );
	CNetworkVector	( m_worldPosition );

#if 1
	// adnan
	// this is how we tell if we're rotating what we're holding
	CNetworkVar( bool, m_bIsCurrentlyRotating );
	// end adnan
#endif

	CGravControllerPoint		m_gravCallback;

	bool		m_bInWeapon1;
	bool		m_bInWeapon2;

	CNetworkVar( float, m_flElementPosition );    // Current prong position (0=closed, 1=open)
	CNetworkVar( float, m_flElementDestination ); // Target position (0 or 1)
	CNetworkVar( bool, m_bOpen );                 // Are prongs open?
	int			m_poseActive;                     // Cached index of "active" pose parameter
	bool		m_sbStaticPoseParamsLoaded;       // Have we loaded the pose parameter?

	DECLARE_ACTTABLE();
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPhysicsGun, DT_WeaponPhysicsGun )

BEGIN_NETWORK_TABLE( CWeaponPhysicsGun, DT_WeaponPhysicsGun )
#ifdef CLIENT_DLL
	RecvPropEHandle( RECVINFO( m_hObject ) ),
	RecvPropInt( RECVINFO( m_physicsBone ) ),
	RecvPropVector( RECVINFO( m_targetPosition ) ),
	RecvPropVector( RECVINFO( m_worldPosition ) ),
	RecvPropInt( RECVINFO(m_active) ),
#if 1
	// adnan
	// also receive if we're rotating what we're holding (by pressing use)
	RecvPropBool( RECVINFO( m_bIsCurrentlyRotating ) ),
	// end adnan
#endif
	RecvPropFloat( RECVINFO( m_flElementPosition ) ),
	RecvPropFloat( RECVINFO( m_flElementDestination ) ),
	RecvPropBool( RECVINFO( m_bOpen ) ),
#else
	SendPropEHandle( SENDINFO( m_hObject ) ),
	SendPropInt( SENDINFO( m_physicsBone ) ),
	SendPropVector(SENDINFO( m_targetPosition ), -1, SPROP_COORD),
	SendPropVector(SENDINFO( m_worldPosition ), -1, SPROP_COORD),
	SendPropInt( SENDINFO(m_active), 1, SPROP_UNSIGNED ),
#if 1
	// adnan
	// need to seind if we're rotating what we're holding
	SendPropBool( SENDINFO( m_bIsCurrentlyRotating ) ),
	// end adnan
#endif
	SendPropFloat( SENDINFO( m_flElementPosition ) ),
	SendPropFloat( SENDINFO( m_flElementDestination ) ),
	SendPropBool( SENDINFO( m_bOpen ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponPhysicsGun )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_physgun, CWeaponPhysicsGun );
PRECACHE_WEAPON_REGISTER(weapon_physgun);

acttable_t	CWeaponPhysicsGun::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_PHYSGUN,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_PHYSGUN,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_PHYSGUN,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_PHYSGUN,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_PHYSGUN,					false },
};

IMPLEMENT_ACTTABLE(CWeaponPhysicsGun);


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CWeaponPhysicsGun )

	DEFINE_FIELD( m_active,				FIELD_INTEGER ),
	DEFINE_FIELD( m_useDown,				FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hObject,				FIELD_EHANDLE ),
	DEFINE_FIELD( m_physicsBone,				FIELD_INTEGER ),
	DEFINE_FIELD( m_distance,			FIELD_FLOAT ),
	DEFINE_FIELD( m_movementLength,		FIELD_FLOAT ),
	DEFINE_FIELD( m_soundState,			FIELD_INTEGER ),
	DEFINE_FIELD( m_originalObjectPosition,	FIELD_POSITION_VECTOR ),
#if 1
	// adnan
	DEFINE_FIELD( m_bIsCurrentlyRotating, FIELD_BOOLEAN ),
	// end adnan
#endif
	DEFINE_EMBEDDED( m_gravCallback ),
	// Physptrs can't be saved in embedded classes..
	DEFINE_PHYSPTR( m_gravCallback.m_controller ),

	DEFINE_FIELD( m_flElementPosition, FIELD_FLOAT ),
	DEFINE_FIELD( m_flElementDestination, FIELD_FLOAT ),
	DEFINE_FIELD( m_bOpen, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_poseActive, FIELD_INTEGER ),
	DEFINE_FIELD( m_sbStaticPoseParamsLoaded, FIELD_BOOLEAN ),

END_DATADESC()

//=========================================================
//=========================================================

CWeaponPhysicsGun::CWeaponPhysicsGun()
{
	m_active = false;
	m_bFiresUnderwater = true;
	m_bInWeapon1 = false;
	m_bInWeapon2 = false;
	m_flElementPosition = 0.0f;
	m_flElementDestination = 0.0f;
	m_bOpen = false;
	m_poseActive = -1;
	m_sbStaticPoseParamsLoaded = false;
}


//-----------------------------------------------------------------------------
// On Remove
//-----------------------------------------------------------------------------
void CWeaponPhysicsGun::UpdateOnRemove(void)
{
	EffectDestroy();
	SoundDestroy();
	BaseClass::UpdateOnRemove();
}

void CWeaponPhysicsGun::OpenElements( void )
{
	if ( m_bOpen )
		return;

	m_flElementDestination = 1.0f;
	m_bOpen = true;
}

void CWeaponPhysicsGun::CloseElements( void )
{
	if ( !m_bOpen )
		return;

	m_flElementDestination = 0.0f;
	m_bOpen = false;

	SendWeaponAnim( ACT_VM_IDLE );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// adnan
// want to add an angles modifier key
bool CGravControllerPoint::UpdateObject( CBasePlayer *pPlayer, CBaseEntity *pEntity )
{
	IPhysicsObject *pPhysics = GetPhysObjFromPhysicsBone( pEntity, m_attachedPhysicsBone );
	if ( !pEntity || !pPhysics )
	{
		return false;
	}

#if 1
	// adnan
	// if we've been rotating it, set it to its proper new angles (change m_attachedAnglesPlayerSpace while modifier)
	//Pickup_GetRotatedCarryAngles( pEntity, pPlayer, pPlayer->EntityToWorldTransform(), angles );
	// added the ... && (mousedx | mousedy) so we dont have to calculate if no mouse movement
	// UPDATE: m_vecRotatedCarryAngles has become a temp variable... can be cleaned up by using actual temp vars
#ifdef CLIENT_DLL
	if( m_bHasRotatedCarryAngles && (pPlayer->m_pCurrentCommand->mousedx || pPlayer->m_pCurrentCommand->mousedy) )
#else
	if( m_bHasRotatedCarryAngles && (pPlayer->GetCurrentCommand()->mousedx || pPlayer->GetCurrentCommand()->mousedy) )
#endif
	{
		// method II: relative orientation
		VMatrix vDeltaRotation, vCurrentRotation, vNewRotation;
		
		MatrixFromAngles( m_targetRotation, vCurrentRotation );

#ifdef CLIENT_DLL
		m_vecRotatedCarryAngles[YAW] = pPlayer->m_pCurrentCommand->mousedx*0.2;
		m_vecRotatedCarryAngles[PITCH] = pPlayer->m_pCurrentCommand->mousedy*-0.2;
#else
		m_vecRotatedCarryAngles[YAW] = pPlayer->GetCurrentCommand()->mousedx*0.2;
		m_vecRotatedCarryAngles[PITCH] = pPlayer->GetCurrentCommand()->mousedy*-0.2;
#endif
		m_vecRotatedCarryAngles[ROLL] = 0;
		MatrixFromAngles( m_vecRotatedCarryAngles, vDeltaRotation );

		MatrixMultiply(vDeltaRotation, vCurrentRotation, vNewRotation);
		MatrixToAngles( vNewRotation, m_targetRotation );
	}
	// end adnan
#endif

	SetTargetPosition( m_targetPosition, m_targetRotation );

	return true;
}

#if 1
// adnan
// this is where we say that we dont want ot apply the current calculated view angles
//-----------------------------------------------------------------------------
// Purpose: Allow weapons to override mouse input to viewangles (for orbiting)
//-----------------------------------------------------------------------------
bool CWeaponPhysicsGun::OverrideViewAngles( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if(!pPlayer)
		return false;

	if (m_bIsCurrentlyRotating) {
		return true;
	}

	return false;
}
// end adnan
#endif

//=========================================================
//=========================================================
void CWeaponPhysicsGun::Spawn( )
{
	BaseClass::Spawn();
//	SetModel( GetWorldModel() );

	// The physgun uses a different skin
	m_nSkin = PHYSGUN_SKIN;

	FallInit();
}

void CWeaponPhysicsGun::OnRestore( void )
{
	BaseClass::OnRestore();

	if ( m_gravCallback.m_controller )
	{
		m_gravCallback.m_controller->SetEventHandler( &m_gravCallback );
	}
}


//=========================================================
//=========================================================
void CWeaponPhysicsGun::Precache( void )
{
	BaseClass::Precache();

	g_physgunBeam1 = PrecacheModel(PHYSGUN_BEAM_SPRITE1);
	g_physgunBeam = PrecacheModel(PHYSGUN_BEAM_SPRITE);
	g_physgunGlow = PrecacheModel(PHYSGUN_BEAM_GLOW);
}

void CWeaponPhysicsGun::EffectCreate( void )
{
	EffectUpdate();
	m_active = true;
}


// Andrew; added so we can trace both in EffectUpdate and DrawModel with the same results
void CWeaponPhysicsGun::TraceLine( trace_t *ptr )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	Vector start, forward, right;
	pOwner->EyeVectors( &forward, &right, NULL );

	start = pOwner->Weapon_ShootPosition();
	Vector end = start + forward * 4096;

	// UTIL_TraceLine( start, end, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, ptr );
	UTIL_TraceLine( start, end, MASK_SHOT|CONTENTS_GRATE, pOwner, COLLISION_GROUP_NONE, ptr );
}

void CWeaponPhysicsGun::EffectUpdate( void )
{
	Vector start, forward, right;
	trace_t tr;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	pOwner->EyeVectors( &forward, &right, NULL );

	start = pOwner->Weapon_ShootPosition();

	TraceLine( &tr );
	Vector end = tr.endpos;
	float distance = tr.fraction * 4096;

	if ( m_hObject == NULL && tr.DidHitNonWorldEntity() )
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		AttachObject( pEntity, GetPhysObjFromPhysicsBone( pEntity, tr.physicsbone ), tr.physicsbone, start, tr.endpos, distance );
	}

	// Add the incremental player yaw to the target transform
	QAngle angles = m_gravCallback.TransformAnglesFromPlayerSpace( m_gravCallback.m_targetRotation, pOwner );

	CBaseEntity *pObject = m_hObject;
	if ( pObject )
	{
		// one hell of a hack
		const char* pszPropName = pObject->GetClassname();

#ifdef GAME_DLL
		IPhysicsObject* pFreeze = GetPhysObjFromPhysicsBone( pObject, m_physicsBone );

		if ( strcmp( pszPropName, "prop_vehicle_jeep" ) != 0 ) // make sure it's not a jeep
		{
			pFreeze->EnableMotion( true ); // unfreeze, if possible
		}
#endif 

		if ( m_useDown )
		{
#if 1
			if ( pOwner->m_afButtonPressed & IN_ATTACK2 )
			{
				m_useDown = false;
				if ( strcmp( pszPropName, "prop_vehicle_jeep" ) != 0 ) // make sure it's not a jeep
				{
#ifdef GAME_DLL
					pFreeze->EnableMotion( false ); // freeze the object
#endif
				}
			}
#endif
			if ( pOwner->m_afButtonPressed & IN_USE )
			{
				m_useDown = false;
			}
		}
		else 
		{
#if 1
			if ( pOwner->m_afButtonPressed & IN_ATTACK2 )
			{
				m_useDown = false;
				if ( strcmp( pszPropName, "prop_vehicle_jeep" ) != 0 ) // make sure it's not a jeep
				{
#ifdef GAME_DLL
					pFreeze->EnableMotion( false ); // freeze the object
#endif
				}
			}
#endif
			if ( pOwner->m_afButtonPressed & IN_USE )
			{
				m_useDown = true;
			}
		}

#if NEVER
		if ( m_useDown )
		{
#ifndef CLIENT_DLL
			pOwner->SetPhysicsFlag( PFLAG_DIROVERRIDE, true );
#endif
			if ( pOwner->m_nButtons & IN_FORWARD )
			{
				m_distance = Approach( 1024, m_distance, gpGlobals->frametime * 100 );
			}
			if ( pOwner->m_nButtons & IN_BACK )
			{
				m_distance = Approach( 40, m_distance, gpGlobals->frametime * 100 );
			}
		}
#endif

		if ( pOwner->m_nButtons & IN_WEAPON1 )
		{
			m_distance = Approach( 1024, m_distance, m_distance * 0.1 + 5.0f );
#ifdef CLIENT_DLL
			if ( gpGlobals->maxClients > 1 )
			{
				gHUD.m_bSkipClear = false;
			}
#endif
		}
		if ( pOwner->m_nButtons & IN_WEAPON2 )
		{
			m_distance = Approach( 40, m_distance, m_distance * 0.1 + 5.0f );
#ifdef CLIENT_DLL
			if ( gpGlobals->maxClients > 1 )
			{
				gHUD.m_bSkipClear = false;
			}
#endif
		}

		IPhysicsObject *pPhys = GetPhysObjFromPhysicsBone( pObject, m_physicsBone );
		if ( pPhys )
		{
			if ( pPhys->IsAsleep() )
			{
				// on the odd chance that it's gone to sleep while under anti-gravity
				pPhys->Wake();
			}

			Vector newPosition = start + forward * m_distance;
			Vector offset;
			pPhys->LocalToWorld( &offset, m_worldPosition );
			Vector vecOrigin;
			pPhys->GetPosition( &vecOrigin, NULL );
			m_gravCallback.SetTargetPosition( newPosition + (vecOrigin - offset), angles );
			Vector dir = (newPosition - pObject->GetLocalOrigin());
			m_movementLength = dir.Length();
		}
	}
	else
	{
		m_targetPosition = end;
		//m_gravCallback.SetTargetPosition( end, m_gravCallback.m_targetRotation );
	}
}

void CWeaponPhysicsGun::SoundCreate( void )
{
}


void CWeaponPhysicsGun::SoundDestroy( void )
{
}


void CWeaponPhysicsGun::SoundStop( void )
{
}

void CWeaponPhysicsGun::SoundStart( void )
{
}

void CWeaponPhysicsGun::SoundUpdate( void )
{
}


CBaseEntity *CWeaponPhysicsGun::GetBeamEntity()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return NULL;

	// Make sure I've got a view model
	CBaseViewModel *vm = pOwner->GetViewModel();
	if ( vm )
		return vm;

	return pOwner;
}

void CWeaponPhysicsGun::EffectDestroy( void )
{
#ifdef CLIENT_DLL
	gHUD.m_bSkipClear = false;
#endif
	m_active = false;
	SoundStop();

	DetachObject();
}

void CWeaponPhysicsGun::UpdateObject( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	Assert( pPlayer );

	CBaseEntity *pObject = m_hObject;
	if ( !pObject )
		return;

	if ( !m_gravCallback.UpdateObject( pPlayer, pObject ) )
	{
		DetachObject();
		return;
	}
}

void CWeaponPhysicsGun::DetachObject( void )
{
	if ( m_hObject )
	{
#ifndef CLIENT_DLL
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		Pickup_OnPhysGunDrop( m_hObject, pOwner, DROPPED_BY_CANNON );
#endif

		IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int count = m_hObject->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );
		for ( int i = 0; i < count; i++ )
		{
			PhysClearGameFlags( pList[i], FVPHYSICS_PLAYER_HELD );
		}
		m_gravCallback.DetachEntity();
		m_hObject = NULL;
		m_physicsBone = 0;

		CloseElements();
	}
}

void CWeaponPhysicsGun::AttachObject( CBaseEntity *pObject, IPhysicsObject *pPhysics, short physicsbone, const Vector& start, const Vector &end, float distance )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if( !pOwner )
		return;
	m_hObject = pObject;
	m_physicsBone = physicsbone;
	m_useDown = false;
	if ( pPhysics && pObject->GetMoveType() == MOVETYPE_VPHYSICS )
	{
		m_distance = distance;

		Vector worldPosition;
		pPhysics->WorldToLocal( &worldPosition, end );
		m_worldPosition = worldPosition;
		Vector vecOrigin;
		pPhysics->GetPosition( &vecOrigin, NULL );
		m_gravCallback.AttachEntity( pOwner, pObject, pPhysics, physicsbone, vecOrigin );

		m_originalObjectPosition = vecOrigin;

		pPhysics->Wake();
		IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int count = pObject->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );
		for ( int i = 0; i < count; i++ )
		{
			PhysSetGameFlags( pList[i], FVPHYSICS_PLAYER_HELD );
		}

#ifndef CLIENT_DLL
		Pickup_OnPhysGunPickup( pObject, pOwner );
#endif

		OpenElements();
	}
	else
	{
		m_hObject = NULL;
		m_physicsBone = 0;
	}
}

//=========================================================
//=========================================================
void CWeaponPhysicsGun::PrimaryAttack( void )
{
	if ( !m_active )
	{
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );
		EffectCreate();
		SoundCreate();
	}
	else
	{
		EffectUpdate();
		SoundUpdate();
	}
}

void CWeaponPhysicsGun::SecondaryAttack( void )
{
	return;
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Third-person function call to render world model
//-----------------------------------------------------------------------------
int CWeaponPhysicsGun::DrawModel( int flags )
{
	// Only render these on the transparent pass
	if ( flags & STUDIO_TRANSPARENCY )
	{
		if ( !m_active )
			return 0;

		C_BasePlayer *pOwner = ToBasePlayer( GetOwner() );

		if ( !pOwner )
			return 0;

		Vector points[3];
		QAngle tmpAngle;

		C_BaseEntity *pObject = m_hObject;
		//if ( pObject == NULL )
		//	return 0;

		GetAttachment( 1, points[0], tmpAngle );

		// a little noise 11t & 13t should be somewhat non-periodic looking
		//points[1].z += 4*sin( gpGlobals->curtime*11 ) + 5*cos( gpGlobals->curtime*13 );
		if ( pObject == NULL )
		{
			//points[2] = m_targetPosition;
			trace_t tr;
			TraceLine( &tr );
			points[2] = tr.endpos;
		}
		else
		{
			pObject->EntityToWorldSpace( m_worldPosition, &points[2] );
		}

		Vector forward, right, up;
		QAngle playerAngles = pOwner->EyeAngles();
		AngleVectors( playerAngles, &forward, &right, &up );
		if ( pObject == NULL )
		{
			Vector vecDir = points[2] - points[0];
			VectorNormalize( vecDir );
			points[1] = points[0] + 0.5f * (vecDir * points[2].DistTo(points[0]));
		}
		else
		{
			Vector vecSrc = pOwner->Weapon_ShootPosition( );
			points[1] = vecSrc + 0.5f * (forward * points[2].DistTo(points[0]));
		}
		
		IMaterial* pMat = materials->FindMaterial( PHYSGUN_BEAM_SPRITE1, TEXTURE_GROUP_CLIENT_EFFECTS );
		if (pObject)
			pMat = materials->FindMaterial( PHYSGUN_BEAM_SPRITE, TEXTURE_GROUP_CLIENT_EFFECTS );
		Vector color;
		color.Init(1,1,1);

		float scrollOffset = gpGlobals->curtime - (int)gpGlobals->curtime;
		CMatRenderContextPtr pRenderContext( materials );
		pRenderContext->Bind( pMat );
		DrawBeamQuadratic( points[0], points[1], points[2], pObject ? 13/3.0f : 13/5.0f, color, scrollOffset );
		DrawBeamQuadratic( points[0], points[1], points[2], pObject ? 13/3.0f : 13/5.0f, color, -scrollOffset );

		IMaterial *pMaterial = materials->FindMaterial( PHYSGUN_BEAM_GLOW, TEXTURE_GROUP_CLIENT_EFFECTS );
		color32 clr = { physgun_r.GetInt(), physgun_g.GetInt(), physgun_b.GetInt(), 255 };

		float scale = random->RandomFloat( 3, 5 ) * ( pObject ? 2 : 2 );

		// Draw the sprite
		pRenderContext->Bind( pMaterial );
		for ( int i = 0; i < 3; i++ )
			DrawSprite( points[2], scale, scale, clr );

		return 1;
	}

	// Only do this on the opaque pass
	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: First-person function call after viewmodel has been drawn
//-----------------------------------------------------------------------------
void CWeaponPhysicsGun::ViewModelDrawn( C_BaseViewModel *pBaseViewModel )
{
	if ( !m_active )
		return;

	// Render our effects
	C_BasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( !pOwner )
		return;

	Vector points[3];
	QAngle tmpAngle;

	C_BaseEntity *pObject = m_hObject;
	//if ( pObject == NULL )
	//	return;

	pBaseViewModel->GetAttachment( 1, points[0], tmpAngle );

	// a little noise 11t & 13t should be somewhat non-periodic looking
	//points[1].z += 4*sin( gpGlobals->curtime*11 ) + 5*cos( gpGlobals->curtime*13 );
	if ( pObject == NULL )
	{
		//points[2] = m_targetPosition;
		trace_t tr;
		TraceLine( &tr );
		points[2] = tr.endpos;
	}
	else
	{
		pObject->EntityToWorldSpace(m_worldPosition, &points[2]);
	}

	Vector forward, right, up;
	QAngle playerAngles = pOwner->EyeAngles();
	AngleVectors( playerAngles, &forward, &right, &up );
	Vector vecSrc = pOwner->Weapon_ShootPosition( );
	points[1] = vecSrc + 0.5f * (forward * points[2].DistTo(points[0]));
	
	IMaterial* pMat = materials->FindMaterial(PHYSGUN_BEAM_SPRITE1, TEXTURE_GROUP_CLIENT_EFFECTS);
	if (pObject)
		pMat = materials->FindMaterial(PHYSGUN_BEAM_SPRITE, TEXTURE_GROUP_CLIENT_EFFECTS);

	Vector color;
	color.Init(1, 1, 1);

	// Now draw it.
	CViewSetup beamView = *view->GetPlayerViewSetup();

	Frustum dummyFrustum;
	render->Push3DView( beamView, 0, NULL, dummyFrustum );

	float scrollOffset = gpGlobals->curtime - (int)gpGlobals->curtime;
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( pMat );
#if 1
	// HACK HACK:  Munge the depth range to prevent view model from poking into walls, etc.
	// Force clipped down range
	pRenderContext->DepthRange( 0.1f, 0.2f );
#endif
	DrawBeamQuadratic( points[0], points[1], points[2], pObject ? 13/3.0f : 13/5.0f, color, scrollOffset );
	DrawBeamQuadratic( points[0], points[1], points[2], pObject ? 13/3.0f : 13/5.0f, color, -scrollOffset );

	IMaterial *pMaterial = materials->FindMaterial( PHYSGUN_BEAM_GLOW, TEXTURE_GROUP_CLIENT_EFFECTS );
	color32 clr = { physgun_r.GetInt(), physgun_g.GetInt(), physgun_b.GetInt(), 255 };

	float scale = random->RandomFloat( 3, 5 ) * ( pObject ? 3 : 2 );

	// Draw the sprite
	pRenderContext->Bind( pMaterial );
	for ( int i = 0; i < 3; i++ )
	{
		DrawSprite( points[2], scale, scale, clr );
	}
#if 1
	pRenderContext->DepthRange( 0.0f, 1.0f );
#endif

#ifdef CLIENT_DLL
	// funny glow
	float scale1 = random->RandomFloat(15, 20) * 2;

	IMaterial* pMat1 = materials->FindMaterial("sprites/glow04_noz", TEXTURE_GROUP_PARTICLE, true);
	Vector color1;
	color1.Init(0, 255, 0);

	DrawSprite(points[0], scale1, scale1, clr);
	pRenderContext->Bind(pMat1);
	for (int i = 1; i < 3; i++)
		DrawSprite(points[0], scale1, scale1, clr);
#endif

	render->PopView( dummyFrustum );

	// Pass this back up
	BaseClass::ViewModelDrawn( pBaseViewModel );
}

//-----------------------------------------------------------------------------
// Purpose: We are always considered transparent
//-----------------------------------------------------------------------------
bool CWeaponPhysicsGun::IsTransparent( void )
{
	return true;
}

#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponPhysicsGun::ItemPreFrame()
{
	BaseClass::ItemPreFrame();

#ifdef GAME_DLL
	// Update the object if the weapon is switched on.
	if( m_active )
	{
		UpdateObject();
	}

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	CBaseViewModel *vm = pOwner->GetViewModel();
	if ( !vm )
		return;

	if ( !m_sbStaticPoseParamsLoaded )
	{
		m_poseActive = vm->LookupPoseParameter( "active" );
		m_sbStaticPoseParamsLoaded = true;
	}

	m_flElementPosition = UTIL_Approach( m_flElementDestination, m_flElementPosition, 0.1f );
	vm->SetPoseParameter( m_poseActive, m_flElementPosition );
#endif
}


void CWeaponPhysicsGun::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
		return;

#if 1
	// adnan
	// this is where we check if we're orbiting the object
	
	// if we're holding something and pressing use,
	//  then set us in the orbiting state
	//  - this will indicate to OverrideMouseInput that we should zero the input and update our delta angles
	//  UPDATE: not anymore.  now this just sets our state variables.
	CBaseEntity *pObject = m_hObject;
	if( pObject ) {

		if((pOwner->m_nButtons & IN_ATTACK) && (pOwner->m_nButtons & IN_USE) ) {
			m_gravCallback.m_bHasRotatedCarryAngles = true;
			
			// did we JUST hit use?
			//  if so, grab the current angles to begin with as the rotated angles
			if( !(pOwner->m_afButtonLast & IN_USE) ) {
				m_gravCallback.m_vecRotatedCarryAngles = pObject->GetAbsAngles();
			}

			m_bIsCurrentlyRotating = true;
		} else {
			m_gravCallback.m_bHasRotatedCarryAngles = false;

			m_bIsCurrentlyRotating = false;
		}
	} else {
		m_bIsCurrentlyRotating = false;

		m_gravCallback.m_bHasRotatedCarryAngles = false;
	}
	// end adnan
#endif

	if ( pOwner->m_nButtons & IN_ATTACK )
	{
#if 1
		if( (pOwner->m_nButtons & IN_USE) ) {
			pOwner->m_vecUseAngles = pOwner->pl.v_angle;
		}
#endif
		if ( pOwner->m_afButtonPressed & IN_ATTACK2 )
		{
			SecondaryAttack();
		}
		else if ( pOwner->m_nButtons & IN_ATTACK2 )
		{
			if ( m_active )
			{
				EffectDestroy();
				SoundDestroy();
			}
			WeaponIdle( );
			return;
		}
		PrimaryAttack();
	}
	else 
	{
		if ( m_active )
		{
			EffectDestroy();
			SoundDestroy();
		}
		WeaponIdle( );
		return;
	}
	if ( pOwner->m_afButtonPressed & IN_RELOAD )
	{
		Reload();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPhysicsGun::HasAnyAmmo( void )
{
	//Always report that we have ammo
	return true;
}

//=========================================================
//=========================================================
bool CWeaponPhysicsGun::Reload( void )
{
	return false;
}
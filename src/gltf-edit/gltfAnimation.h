#pragma once
#include "../bgfx-imgui/imgui_impl_bgfx.h"
#include "gltfProperties.h"

class gltfAnimEditor : public imDrawable {
public:
	static void Init();
	gltfAnimEditor( ) { };
	~gltfAnimEditor( ) { };
	bool imDraw( ) override;
	bool Show( bool visible ) override;
	bool isVisible( ) override;
};
extern gltfAnimEditor * animEditor;

class gltfArticulatedFigure {
public:
	gltfArticulatedFigure( gltfSkin * skin, gltfData * data );
	~gltfArticulatedFigure( ) { };
	void Advance( gltfData *data ) { };
	void ComputeJoints();
	static void DrawJoints();
private:
	gltfSkin * currentSkin;
	gltfData * currentData;;
	idList<idMat4> jointMatrices;
	idList<idMat4> jointNormalMatrices;

};


//
// animation channels
// these can be changed by modmakers and licensees to be whatever they need.
const int ANIM_NumAnimChannels = 5;
const int ANIM_MaxAnimsPerChannel = 3;
const int ANIM_MaxSyncedAnims = 3;

//
// animation channels.  make sure to change script/doom_defs.script if you add any channels, or change their order
//
const int ANIMCHANNEL_ALL = 0;
const int ANIMCHANNEL_TORSO = 1;
const int ANIMCHANNEL_LEGS = 2;
const int ANIMCHANNEL_HEAD = 3;
const int ANIMCHANNEL_EYELIDS = 4;

// for converting from 24 frames per second to milliseconds
ID_INLINE int FRAME2MS( int framenum ) {
	return ( framenum * 1000 ) / 24;
}


typedef enum {
	FC_SCRIPTFUNCTION,
	FC_SCRIPTFUNCTIONOBJECT,
	FC_EVENTFUNCTION,
	FC_SOUND,
	FC_SOUND_VOICE,
	FC_SOUND_VOICE2,
	FC_SOUND_BODY,
	FC_SOUND_BODY2,
	FC_SOUND_BODY3,
	FC_SOUND_WEAPON,
	FC_SOUND_ITEM,
	FC_SOUND_GLOBAL,
	FC_SOUND_CHATTER,
	FC_SKIN,
	FC_TRIGGER,
	FC_TRIGGER_SMOKE_PARTICLE,
	FC_MELEE,
	FC_DIRECTDAMAGE,
	FC_BEGINATTACK,
	FC_ENDATTACK,
	FC_MUZZLEFLASH,
	FC_CREATEMISSILE,
	FC_LAUNCHMISSILE,
	FC_FIREMISSILEATTARGET,
	FC_FOOTSTEP,
	FC_LEFTFOOT,
	FC_RIGHTFOOT,
	FC_ENABLE_EYE_FOCUS,
	FC_DISABLE_EYE_FOCUS,
	FC_FX,
	FC_DISABLE_GRAVITY,
	FC_ENABLE_GRAVITY,
	FC_JUMP,
	FC_ENABLE_CLIP,
	FC_DISABLE_CLIP,
	FC_ENABLE_WALK_IK,
	FC_DISABLE_WALK_IK,
	FC_ENABLE_LEG_IK,
	FC_DISABLE_LEG_IK,
	FC_RECORDDEMO,
	FC_AVIGAME
} frameCommandType_t;

typedef struct {
	frameCommandType_t		type;
	idStr *string;

	union {
		//const idSoundShader *soundShader;
		//const function_t *function;
		//const idDeclSkin *skin;
		int					index;
	};
} frameCommand_t;

typedef struct {
	bool					prevent_idle_override : 1;
	bool					random_cycle_start : 1;
	bool					ai_no_turn : 1;
	bool					anim_turn : 1;
} animFlags_t;

typedef struct {
	int						nameIndex;
	int						parentNum;
	int						animBits;
	int						firstComponent;
} jointAnimInfo_t;

typedef enum {
	INVALID_JOINT = -1
} jointHandle_t;

typedef struct {
	int		cycleCount;	// how many times the anim has wrapped to the begining (0 for clamped anims)
	int		frame1;
	int		frame2;
	float	frontlerp;
	float	backlerp;
} frameBlend_t;

typedef struct {
	int						num;
	int						firstCommand;
} frameLookup_t;


//
// joint modifier modes.  make sure to change script/doom_defs.script if you add any, or change their order.
//
typedef enum {
	JOINTMOD_NONE,				// no modification
	JOINTMOD_LOCAL,				// modifies the joint's position or orientation in joint local space
	JOINTMOD_LOCAL_OVERRIDE,	// sets the joint's position or orientation in joint local space
	JOINTMOD_WORLD,				// modifies joint's position or orientation in model space
	JOINTMOD_WORLD_OVERRIDE		// sets the joint's position or orientation in model space
} jointModTransform_t;

typedef struct {
	jointHandle_t			jointnum;
	idMat3					mat;
	idVec3					pos;
	jointModTransform_t		transform_pos;
	jointModTransform_t		transform_axis;
} jointMod_t;

#define	AN

/*
==============================================================================================

idAnim

==============================================================================================
*/

class idAnim {
private:
	const class gltfData		*modelDef;
	const gltfAnimation			*anims[ ANIM_MaxSyncedAnims ];
	int							numAnims;
	idStr						name;
	idStr						realname;
	idList<frameLookup_t>		frameLookup;
	idList<frameCommand_t>		frameCommands;
	animFlags_t					flags;

public:
	idAnim();
	idAnim( const gltfData *modelDef, const idAnim *anim );
	~idAnim();

	void						SetAnim( const gltfData *modelDef, const char *sourcename, const char *animname, int num, const gltfAnimation *md5anims[ ANIM_MaxSyncedAnims ] );
	const char					*Name( void ) const;
	const char					*FullName( void ) const;
	const gltfAnimation			*gltfAnim( int num ) const;
	const gltfData				*ModelDef( void ) const;
	int							Length( void ) const;
	int							NumFrames( void ) const;
	int							NumAnims( void ) const;
	const idVec3				&TotalMovementDelta( void ) const;
	bool						GetOrigin( idVec3 &offset, int animNum, int time, int cyclecount ) const;
	bool						GetOriginRotation( idQuat &rotation, int animNum, int currentTime, int cyclecount ) const;
	bool						GetBounds( idBounds &bounds, int animNum, int time, int cyclecount ) const;
	const char					*AddFrameCommand( const class gltfData *modelDef, int framenum, idLexer &src, const idDict *def );
	//void						CallFrameCommands( idEntity *ent, int from, int to ) const;
	bool						HasFrameCommands( void ) const;

	// returns first frame (zero based) that command occurs.  returns -1 if not found.
	int							FindFrameForFrameCommand( frameCommandType_t framecommand, const frameCommand_t **command ) const;
	void						SetAnimFlags( const animFlags_t &animflags );
	const animFlags_t			&GetAnimFlags( void ) const;
};



/*
==============================================================================================

idAnimBlend

==============================================================================================
*/

class idAnimBlend {
private:
	const class gltfData		*modelDef;
	int							starttime;
	int							endtime;
	int							timeOffset;
	float						rate;

	int							blendStartTime;
	int							blendDuration;
	float						blendStartValue;
	float						blendEndValue;

	float						animWeights[ ANIM_MaxSyncedAnims ];
	short						cycle;
	short						frame;
	short						animNum;
	bool						allowMove;
	bool						allowFrameCommands;

	friend class				idAnimator;

	void						Reset( const gltfData *_modelDef );
	//void						CallFrameCommands( idEntity *ent, int fromtime, int totime ) const;
	void						SetFrame( const gltfData *modelDef, int animnum, int frame, int currenttime, int blendtime );
	void						CycleAnim( const gltfData *modelDef, int animnum, int currenttime, int blendtime );
	void						PlayAnim( const gltfData *modelDef, int animnum, int currenttime, int blendtime );
	bool						BlendAnim( int currentTime, int channel, int numJoints, idJointQuat *blendFrame, float &blendWeight, bool removeOrigin, bool overrideBlend, bool printInfo ) const;
	void						BlendOrigin( int currentTime, idVec3 &blendPos, float &blendWeight, bool removeOriginOffset ) const;
	void						BlendDelta( int fromtime, int totime, idVec3 &blendDelta, float &blendWeight ) const;
	void						BlendDeltaRotation( int fromtime, int totime, idQuat &blendDelta, float &blendWeight ) const;
	bool						AddBounds( int currentTime, idBounds &bounds, bool removeOriginOffset ) const;

public:
	idAnimBlend();
	//void						Save( idSaveGame *savefile ) const;
	//void						Restore( idRestoreGame *savefile, const idDeclModelDef *modelDef );
	const char					*AnimName( void ) const;
	const char					*AnimFullName( void ) const;
	float						GetWeight( int currenttime ) const;
	float						GetFinalWeight( void ) const;
	void						SetWeight( float newweight, int currenttime, int blendtime );
	int							NumSyncedAnims( void ) const;
	bool						SetSyncedAnimWeight( int num, float weight );
	void						Clear( int currentTime, int clearTime );
	bool						IsDone( int currentTime ) const;
	bool						FrameHasChanged( int currentTime ) const;
	int							GetCycleCount( void ) const;
	void						SetCycleCount( int count );
	void						SetPlaybackRate( int currentTime, float newRate );
	float						GetPlaybackRate( void ) const;
	void						SetStartTime( int startTime );
	int							GetStartTime( void ) const;
	int							GetEndTime( void ) const;
	int							GetFrameNumber( int currenttime ) const;
	int							AnimTime( int currenttime ) const;
	int							NumFrames( void ) const;
	int							Length( void ) const;
	int							PlayLength( void ) const;
	void						AllowMovement( bool allow );
	void						AllowFrameCommands( bool allow );
	const idAnim				*Anim( void ) const;
	int							AnimNum( void ) const;
};

/*
==============================================================================================

idAFPoseJointMod

==============================================================================================
*/

typedef enum {
	AF_JOINTMOD_AXIS,
	AF_JOINTMOD_ORIGIN,
	AF_JOINTMOD_BOTH
} AFJointModType_t;

class idAFPoseJointMod {
public:
	idAFPoseJointMod( void );

	AFJointModType_t			mod;
	idMat3						axis;
	idVec3						origin;
};

ID_INLINE idAFPoseJointMod::idAFPoseJointMod( void ) {
	mod = AF_JOINTMOD_AXIS;
	axis.Identity();
	origin.Zero();
}

/*
==============================================================================================

idAnimator

==============================================================================================
*/

class idAnimator {
public:
	idAnimator();
	~idAnimator();

	size_t						Allocated( void ) const;
	size_t						Size( void ) const;

	//void						Save( idSaveGame *savefile ) const;					// archives object for save game file
	//void						Restore( idRestoreGame *savefile );					// unarchives object from save game file

	//void						SetEntity( idEntity *ent );
	//idEntity					*GetEntity( void ) const ;
	void						RemoveOriginOffset( bool remove );
	bool						RemoveOrigin( void ) const;

	void						GetJointList( const char *jointnames, idList<jointHandle_t> &jointList ) const;

	int							NumAnims( void ) const;
	const idAnim				*GetAnim( int index ) const;
	int							GetAnim( const char *name ) const;
	bool						HasAnim( const char *name ) const;

	void						ServiceAnims( int fromtime, int totime );
	bool						IsAnimating( int currentTime ) const;

	void						GetJoints( int *numJoints, idJointMat **jointsPtr );
	int							NumJoints( void ) const;
	jointHandle_t				GetFirstChild( jointHandle_t jointnum ) const;
	jointHandle_t				GetFirstChild( const char *name ) const;

	gltfMesh					*SetModel( const char *modelname );
	gltfMesh					*ModelHandle( void ) const;
	const gltfData				*ModelDef( void ) const;

	void						ForceUpdate( void );
	void						ClearForceUpdate( void );
	bool						CreateFrame( int animtime, bool force );
	bool						FrameHasChanged( int animtime ) const;
	void						GetDelta( int fromtime, int totime, idVec3 &delta ) const;
	bool						GetDeltaRotation( int fromtime, int totime, idMat3 &delta ) const;
	void						GetOrigin( int currentTime, idVec3 &pos ) const;
	bool						GetBounds( int currentTime, idBounds &bounds );

	idAnimBlend					*CurrentAnim( int channelNum );
	void						Clear( int channelNum, int currentTime, int cleartime );
	void						SetFrame( int channelNum, int animnum, int frame, int currenttime, int blendtime );
	void						CycleAnim( int channelNum, int animnum, int currenttime, int blendtime );
	void						PlayAnim( int channelNum, int animnum, int currenttime, int blendTime );

	// copies the current anim from fromChannelNum to channelNum.
	// the copied anim will have frame commands disabled to avoid executing them twice.
	void						SyncAnimChannels( int channelNum, int fromChannelNum, int currenttime, int blendTime );

	void						SetJointPos( jointHandle_t jointnum, jointModTransform_t transform_type, const idVec3 &pos );
	void						SetJointAxis( jointHandle_t jointnum, jointModTransform_t transform_type, const idMat3 &mat );
	void						ClearJoint( jointHandle_t jointnum );
	void						ClearAllJoints( void );

	void						InitAFPose( void );
	void						SetAFPoseJointMod( const jointHandle_t jointNum, const AFJointModType_t mod, const idMat3 &axis, const idVec3 &origin );
	void						FinishAFPose( int animnum, const idBounds &bounds, const int time );
	void						SetAFPoseBlendWeight( float blendWeight );
	bool						BlendAFPose( idJointQuat *blendFrame ) const;
	void						ClearAFPose( void );

	void						ClearAllAnims( int currentTime, int cleartime );

	jointHandle_t				GetJointHandle( const char *name ) const;
	const char *				GetJointName( jointHandle_t handle ) const;
	int							GetChannelForJoint( jointHandle_t joint ) const;
	bool						GetJointTransform( jointHandle_t jointHandle, int currenttime, idVec3 &offset, idMat3 &axis );
	bool						GetJointLocalTransform( jointHandle_t jointHandle, int currentTime, idVec3 &offset, idMat3 &axis );

	const animFlags_t			GetAnimFlags( int animnum ) const;
	int							NumFrames( int animnum ) const;
	int							NumSyncedAnims( int animnum ) const;
	const char					*AnimName( int animnum ) const;
	const char					*AnimFullName( int animnum ) const;
	int							AnimLength( int animnum ) const;
	const idVec3				&TotalMovementDelta( int animnum ) const;

private:
	void						FreeData( void );
	void						PushAnims( int channel, int currentTime, int blendTime );

private:
	//const idDeclModelDef *		modelDef;
	//idEntity *					entity;

	idAnimBlend					channels[ ANIM_NumAnimChannels ][ ANIM_MaxAnimsPerChannel ];
	idList<jointMod_t *>		jointMods;
	int							numJoints;
	idJointMat *				joints;

	mutable int					lastTransformTime;		// mutable because the value is updated in CreateFrame
	mutable bool				stoppedAnimatingUpdate;
	bool						removeOriginOffset;
	bool						forceUpdate;

	idBounds					frameBounds;

	float						AFPoseBlendWeight;
	idList<int>					AFPoseJoints;
	idList<idAFPoseJointMod>	AFPoseJointMods;
	idList<idJointQuat>			AFPoseJointFrame;
	idBounds					AFPoseBounds;
	int							AFPoseTime;
};

#include "gltfAnimation.h"
#include "../idFramework/Common.h"
#include "../idFramework/idlib/containers/HashTable.h"
#include "../idFramework/idlib/geometry/JointTransform.h"

gltfAnimEditor * animEditor = nullptr;

idCVar gltfAnim_timescale( "gltfAnim_timescale", "1", CVAR_FLOAT, "timescale for swf files" );
idCVar r_showSkel( "r_showSkel", "0", CVAR_RENDERER | CVAR_INTEGER, "draw the skeleton when model animates, 1 = draw model with skeleton, 2 = draw skeleton only", 0, 2, idCmdSystem::ArgCompletion_Integer<0, 2> );
idCVar r_jointNameScale( "r_jointNameScale", "0.02", CVAR_RENDERER | CVAR_FLOAT, "size of joint names when r_showskel is set to 1" );
idCVar r_jointNameOffset( "r_jointNameOffset", "0.5", CVAR_RENDERER | CVAR_FLOAT, "offset of joint names when r_showskel is set to 1" );

void gltfAnimEditor::Init( ) {

}

bool gltfAnimEditor::imDraw( ) {
	common->DWarning( "The method or operation is not implemented." );
	return false;
}

bool gltfAnimEditor::Show( bool visible ) {
	common->DWarning( "The method or operation is not implemented." );
	return false;
}

bool gltfAnimEditor::isVisible( ) {
	common->DWarning( "The method or operation is not implemented." );
	return true;
}

struct gltfAnimState {
	float startTime = 0.0f;
	float lastAnimTime = 0.0f;
	int currentFrame = -1;
	int frameRate = -1;
};

idHashTable<gltfAnimState> gAnimStates;

// Advances a single or all animations in this data.
//->note: gltfAnimation * is not checked to be part of this gltfData!
void gltfData::Advance( gltfAnimation *anim /*= nullptr */ ) {
	if ((anim && !anim->channels.Num( ))
		|| (anim == nullptr && !AnimationList().Num())) 
		return;

	gltfAnimation * targetAnim = !anim ? *animations.begin().p : anim ;

	int animCount = 0;
	while ( targetAnim ) {
		//common->Printf("Animation ^1%s \n",targetAnim->name.c_str() );

		float currentTime = Sys_Milliseconds( ) / 1000.0f;
		int framesToRun = 0;
		for ( auto * channel : targetAnim->channels)
		{
			auto & sampler = *targetAnim->samplers[channel->sampler];

			auto * input = accessors[sampler.input];
			auto * output = accessors[sampler.output];
			auto * target = nodes[channel->target.node];
			float targetTime = 0.0f;

			gltfAnimState * state;
			bool hasState = gAnimStates.Get(target->name.c_str(),&state );
			if ( !hasState )
			{
				state = new gltfAnimState();
				gAnimStates.Set(target->name.c_str(),*state );
			}

			idList<float> & timeStamps = GetAccessorView(input);
			
			if ( gltfAnim_timescale.GetFloat( ) > 0.0f ) {
				if ( state->lastAnimTime == 0 ) {
					state->lastAnimTime = currentTime;
					state->startTime = currentTime;
					state->currentFrame = 0;
				} else {

					int nextFrame = ( state->currentFrame + 1) % timeStamps.Num();
					float deltaTime = ((currentTime - state->lastAnimTime)* gltfAnim_timescale.GetFloat( ))  ;
					float frameDelta = (timeStamps[nextFrame] - timeStamps[state->currentFrame ]);

					if (state->currentFrame + 1 >= timeStamps.Num()  && deltaTime > frameDelta ) {
						state->currentFrame = 0;
						state->lastAnimTime = currentTime;
						state->startTime = currentTime;
					}else if ( deltaTime >= frameDelta ) {
						state->currentFrame = nextFrame;
						state->lastAnimTime = currentTime;
					}

					switch ( channel->target.TRS ) {
					case gltfAnimation_Channel_Target::rotation:
					{

						idList<idQuat*> & values = GetAccessorView<idQuat>(output);
						if ( values.Num( ) == 1 )
							target->rotation = *values[state->currentFrame];
						else {
							switch ( sampler.intType ) {
							case gltfAnimation_Sampler::linear:
								target->rotation = target->rotation.Slerp( *values[state->currentFrame], *values[nextFrame], ( 1.0f / frameDelta ) * deltaTime );
								break;
							case gltfAnimation_Sampler::step:
								target->rotation = *values[state->currentFrame];
								break;
							case gltfAnimation_Sampler::cubicSpline:
								common->Warning( " cubicSpline not implemented for TRS rotation" );
								break;
							default:
							case gltfAnimation_Sampler::count:
								common->Warning( " Unrecognized interpolation type" );
								break;
							}
						}
					}	break;

					case  gltfAnimation_Channel_Target::translation:
					{
						idList<idVec3*> & values = GetAccessorView<idVec3>(output);
						if ( values.Num( ) == 1 )
							target->translation = *values[state->currentFrame];
						else {
						switch ( sampler.intType ) {
						case gltfAnimation_Sampler::linear:
							target->translation.Lerp( *values[state->currentFrame], *values[nextFrame], ( 1.0f / frameDelta ) * deltaTime );
							break;
						case gltfAnimation_Sampler::step:
							target->translation = *values[state->currentFrame];
							break;
						case gltfAnimation_Sampler::cubicSpline:
							common->Warning( " cubicSpline not implemented for TRS translation" );
							break;
						default:
						case gltfAnimation_Sampler::count:
							common->Warning( " Unrecognized interpolation type" );
							break;
						}
						}
					}	break;
					case  gltfAnimation_Channel_Target::scale:
					{
						idList<idVec3*> & values = GetAccessorView<idVec3>(output);
						if ( values.Num() == 1)
							target->scale = *values[state->currentFrame];
						else {
							switch ( sampler.intType ) {
							case gltfAnimation_Sampler::linear:
								target->scale.Lerp( *values[state->currentFrame], *values[nextFrame], ( 1.0f / frameDelta ) * deltaTime );
								break;
							case gltfAnimation_Sampler::step:
								target->scale = *values[state->currentFrame];
								break;
							case gltfAnimation_Sampler::cubicSpline:
								common->Warning( " cubicSpline not implemented for TRS scale" );
								break;
							default:
							case gltfAnimation_Sampler::count:
								common->Warning( " Unrecognized interpolation type" );
								break;
							}
						}
					}	break;
					case  gltfAnimation_Channel_Target::weights:
					{

						common->Warning( " TRS weights not implemented" );

					}	break;
					case gltfAnimation_Channel_Target::count:
					default:
						common->Warning( " Unrecognized TRS prop" );
						break;
					}

					target->dirty = true;

				}
			}

			//common->Printf( "^8Animating ^1%s ^8for ^2%s ^8(^3%s^8:^5%fs^8{^5%f^8}) \n",
			//	channel->target.path.c_str( ),
			//	target->name.c_str( ),
			//	sampler.interpolation.c_str( ),
			//	currentTime ,
			//	state->startTime - timeStamps[state->currentFrame]);
		}

		animCount++;
		if ( !anim )
		{
			if ( animations.Num() > animCount )
				targetAnim = animations[animCount];
			else
				targetAnim = nullptr;
		}else 
			targetAnim = nullptr;

		
	}

	//common->Printf("Advanced %i anims\n",animCount );

}

gltfArticulatedFigure::gltfArticulatedFigure( gltfSkin *skin , gltfData * data) : currentSkin( skin ), currentData( data )  {
	ComputeJoints();
}

void gltfArticulatedFigure::ComputeJoints( ) {
	//auto * invBindMatAcc = currentData->AccessorList()[currentSkin->inverseBindMatrices];
	//idList<idMat4*> &invBindMats = currentData->GetAccessorView<idMat4>(invBindMatAcc);
	//
	//auto & nodeList = currentData->NodeList();
	//int count = 0;
	//for (int joint : currentSkin->joints )
	//{
	//	auto * node = nodeList[joint];
	//	idMat4 & jointMat = jointMatrices.Alloc();
	//	idMat4 * bindMat = invBindMats[count];

	//	currentData->ResolveNodeMatrix(node);
	//	jointMat = node->matrix;
	//	jointMat *= *bindMat;
	//	jointMatrices.Append( jointMat );
	//	count++;
	//}

	//this.jointMatrices = [];
	//this.jointNormalMatrices = [];
	//
	//let i = 0;
	//for ( const joint of this.joints ) {
	//	const node = gltf.nodes[joint];
	//
	//	let jointMatrix = mat4.create( );
	//	let ibm = jsToGlSlice( ibmAccessor, i++ * 16, 16 );
	//	mat4.mul( jointMatrix, node.worldTransform, ibm );
	//	mat4.mul( jointMatrix, parentNode.inverseWorldTransform, jointMatrix );
	//	this.jointMatrices.push( jointMatrix );
	//
	//	let normalMatrix = mat4.create( );
	//	mat4.invert( normalMatrix, jointMatrix );
	//	mat4.transpose( normalMatrix, normalMatrix );
	//	this.jointNormalMatrices.push( normalMatrix );
	//}
}

void gltfArticulatedFigure::DrawJoints( ) {
	//int					i;
	//int					num;
	//idVec3				pos;
	//const idJointMat	*joint;
	//const idMD5Joint	*md5Joint;
	//int					parentNum;
	//
	//num = ent->numJoints;
	//joint = ent->joints;
	//md5Joint = joints.Ptr( );
	//for ( i = 0; i < num; i++, joint++, md5Joint++ ) {
	//	pos = ent->origin + joint->ToVec3( ) * ent->axis;
	//	if ( md5Joint->parent ) {
	//		parentNum = md5Joint->parent - joints.Ptr( );
	//		session->rw->DebugLine( colorWhite, ent->origin + ent->joints[parentNum].ToVec3( ) * ent->axis, pos );
	//	}
	//
	//	session->rw->DebugLine( colorRed, pos, pos + joint->ToMat3( )[0] * 2.0f * ent->axis );
	//	session->rw->DebugLine( colorGreen, pos, pos + joint->ToMat3( )[1] * 2.0f * ent->axis );
	//	session->rw->DebugLine( colorBlue, pos, pos + joint->ToMat3( )[2] * 2.0f * ent->axis );
	//}
	//
	//idBounds bounds;
	//
	//bounds.FromTransformedBounds( ent->bounds, vec3_zero, ent->axis );
	//session->rw->DebugBounds( colorMagenta, bounds, ent->origin );
	//
	//if ( ( r_jointNameScale.GetFloat( ) != 0.0f ) && ( bounds.Expand( 128.0f ).ContainsPoint( view->renderView.vieworg - ent->origin ) ) ) {
	//	idVec3	offset( 0, 0, r_jointNameOffset.GetFloat( ) );
	//	float	scale;
	//
	//	scale = r_jointNameScale.GetFloat( );
	//	joint = ent->joints;
	//	num = ent->numJoints;
	//	for ( i = 0; i < num; i++, joint++ ) {
	//		pos = ent->origin + joint->ToVec3( ) * ent->axis;
	//		session->rw->DrawText( joints[i].name, pos + offset, scale, colorWhite, view->renderView.viewaxis, 1 );
	//	}
	//}
}

/***********************************************************************

idAnim

***********************************************************************/

/*
=====================
idAnim::idAnim
=====================
*/
idAnim::idAnim() {
	modelDef = NULL;
	numAnims = 0;
	memset( anims, 0, sizeof( anims ) );
	memset( &flags, 0, sizeof( flags ) );
}

/*
=====================
idAnim::idAnim
=====================
*/
idAnim::idAnim( const gltfData *modelDef, const idAnim *anim ) {
	int i;

	this->modelDef = modelDef;
	numAnims = anim->numAnims;
	name = anim->name;
	realname = anim->realname;
	flags = anim->flags;

	memset( anims, 0, sizeof( anims ) );
	for( i = 0; i < numAnims; i++ ) {
		anims[ i ] = anim->anims[ i ];
		//anims[ i ]->IncreaseRefs();
	}

	frameLookup.SetNum( anim->frameLookup.Num() );
	memcpy( frameLookup.Ptr(), anim->frameLookup.Ptr(), frameLookup.MemoryUsed() );

	frameCommands.SetNum( anim->frameCommands.Num() );
	for( i = 0; i < frameCommands.Num(); i++ ) {
		frameCommands[ i ] = anim->frameCommands[ i ];
		if ( anim->frameCommands[ i ].string ) {
			frameCommands[ i ].string = new idStr( *anim->frameCommands[ i ].string );
		}
	}
}

/*
=====================
idAnim::~idAnim
=====================
*/
idAnim::~idAnim() {
	int i;

	for( i = 0; i < numAnims; i++ ) {
		//anims[ i ]->DecreaseRefs();
	}

	for( i = 0; i < frameCommands.Num(); i++ ) {
		delete frameCommands[ i ].string;
	}
}

/*
=====================
idAnim::SetAnim
=====================
*/
void idAnim::SetAnim( const gltfData *modelDef, const char *sourcename, const char *animname, int num, const gltfAnimation *md5anims[ ANIM_MaxSyncedAnims ] ) {
	int i;

	this->modelDef = modelDef;

	for( i = 0; i < numAnims; i++ ) {
		anims[ i ]->DecreaseRefs();
		anims[ i ] = NULL;
	}

	assert( ( num > 0 ) && ( num <= ANIM_MaxSyncedAnims ) );
	numAnims	= num;
	realname	= sourcename;
	name		= animname;

	for( i = 0; i < num; i++ ) {
		anims[ i ] = md5anims[ i ];
		anims[ i ]->IncreaseRefs();
	}

	memset( &flags, 0, sizeof( flags ) );

	for( i = 0; i < frameCommands.Num(); i++ ) {
		delete frameCommands[ i ].string;
	}

	frameLookup.Clear();
	frameCommands.Clear();
}

/*
=====================
idAnim::Name
=====================
*/
const char *idAnim::Name( void ) const {
	return name;
}

/*
=====================
idAnim::FullName
=====================
*/
const char *idAnim::FullName( void ) const {
	return realname;
}

/*
=====================
idAnim::MD5Anim

index 0 will never be NULL.  Any anim >= NumAnims will return NULL.
=====================
*/
const gltfAnimation *idAnim::gltfAnim( int num ) const {
	if ( anims == NULL || anims[0] == NULL ) {
		return NULL;
	}
	return anims[ num ];
}

/*
=====================
idAnim::ModelDef
=====================
*/
const gltfData *idAnim::ModelDef( void ) const {
	return modelDef;
}

/*
=====================
idAnim::Length
=====================
*/
int idAnim::Length( void ) const {
	if ( !anims[ 0 ] ) {
		return 0;
	}

	return -1;// anims[ 0 ]->
}

/*
=====================
idAnim::NumFrames
=====================
*/
int	idAnim::NumFrames( void ) const {
	if ( !anims[ 0 ] ) {
		return 0;
	}

	return anims[ 0 ]->NumFrames();
}

/*
=====================
idAnim::NumAnims
=====================
*/
int	idAnim::NumAnims( void ) const {
	return numAnims;
}

/*
=====================
idAnim::TotalMovementDelta
=====================
*/
const idVec3 &idAnim::TotalMovementDelta( void ) const {
	if ( !anims[ 0 ] ) {
		return vec3_zero;
	}

	return anims[ 0 ]->TotalMovementDelta();
}

/*
=====================
idAnim::GetOrigin
=====================
*/
bool idAnim::GetOrigin( idVec3 &offset, int animNum, int currentTime, int cyclecount ) const {
	if ( !anims[ animNum ] ) {
		offset.Zero();
		return false;
	}

	anims[ animNum ]->GetOrigin( offset, currentTime, cyclecount );
	return true;
}

/*
=====================
idAnim::GetOriginRotation
=====================
*/
bool idAnim::GetOriginRotation( idQuat &rotation, int animNum, int currentTime, int cyclecount ) const {
	if ( !anims[ animNum ] ) {
		rotation.Set( 0.0f, 0.0f, 0.0f, 1.0f );
		return false;
	}

	anims[ animNum ]->GetOriginRotation( rotation, currentTime, cyclecount );
	return true;
}

/*
=====================
idAnim::GetBounds
=====================
*/
ID_INLINE bool idAnim::GetBounds( idBounds &bounds, int animNum, int currentTime, int cyclecount ) const {
	if ( !anims[ animNum ] ) {
		return false;
	}

	anims[ animNum ]->GetBounds( bounds, currentTime, cyclecount );
	return true;
}


/*
=====================
idAnim::AddFrameCommand

Returns NULL if no error.
=====================
*/
const char *idAnim::AddFrameCommand( const gltfData *modelDef, int framenum, idLexer &src, const idDict *def ) {return ""; }
//	int					i;
//	int					index;
//	idStr				text;
//	idStr				funcname;
//	frameCommand_t		fc;
//	idToken				token;
//	const jointInfo_t	*jointInfo;
//
//	// make sure we're within bounds
//	if ( ( framenum < 1 ) || ( framenum > anims[ 0 ]->NumFrames() ) ) {
//		return va( "Frame %d out of range", framenum );
//	}
//
//	// frame numbers are 1 based in .def files, but 0 based internally
//	framenum--;
//
//	memset( &fc, 0, sizeof( fc ) );
//
//	if( !src.ReadTokenOnLine( &token ) ) {
//		return "Unexpected end of line";
//	}
//	if ( token == "call" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SCRIPTFUNCTION;
//		fc.function = gameLocal.program.FindFunction( token );
//		if ( !fc.function ) {
//			return va( "Function '%s' not found", token.c_str() );
//		}
//	} else if ( token == "object_call" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SCRIPTFUNCTIONOBJECT;
//		fc.string = new idStr( token );
//	} else if ( token == "event" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_EVENTFUNCTION;
//		const idEventDef *ev = idEventDef::FindEvent( token );
//		if ( !ev ) {
//			return va( "Event '%s' not found", token.c_str() );
//		}
//		if ( ev->GetNumArgs() != 0 ) {
//			return va( "Event '%s' has arguments", token.c_str() );
//		}
//		fc.string = new idStr( token );
//	} else if ( token == "sound" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SOUND;
//		if ( !token.Cmpn( "snd_", 4 ) ) {
//			fc.string = new idStr( token );
//		} else {
//			fc.soundShader = declManager->FindSound( token );
//			if ( fc.soundShader->GetState() == DS_DEFAULTED ) {
//				gameLocal.Warning( "Sound '%s' not found", token.c_str() );
//			}
//		}
//	} else if ( token == "sound_voice" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SOUND_VOICE;
//		if ( !token.Cmpn( "snd_", 4 ) ) {
//			fc.string = new idStr( token );
//		} else {
//			fc.soundShader = declManager->FindSound( token );
//			if ( fc.soundShader->GetState() == DS_DEFAULTED ) {
//				gameLocal.Warning( "Sound '%s' not found", token.c_str() );
//			}
//		}
//	} else if ( token == "sound_voice2" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SOUND_VOICE2;
//		if ( !token.Cmpn( "snd_", 4 ) ) {
//			fc.string = new idStr( token );
//		} else {
//			fc.soundShader = declManager->FindSound( token );
//			if ( fc.soundShader->GetState() == DS_DEFAULTED ) {
//				gameLocal.Warning( "Sound '%s' not found", token.c_str() );
//			}
//		}
//	} else if ( token == "sound_body" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SOUND_BODY;
//		if ( !token.Cmpn( "snd_", 4 ) ) {
//			fc.string = new idStr( token );
//		} else {
//			fc.soundShader = declManager->FindSound( token );
//			if ( fc.soundShader->GetState() == DS_DEFAULTED ) {
//				gameLocal.Warning( "Sound '%s' not found", token.c_str() );
//			}
//		}
//	} else if ( token == "sound_body2" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SOUND_BODY2;
//		if ( !token.Cmpn( "snd_", 4 ) ) {
//			fc.string = new idStr( token );
//		} else {
//			fc.soundShader = declManager->FindSound( token );
//			if ( fc.soundShader->GetState() == DS_DEFAULTED ) {
//				gameLocal.Warning( "Sound '%s' not found", token.c_str() );
//			}
//		}
//	} else if ( token == "sound_body3" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SOUND_BODY3;
//		if ( !token.Cmpn( "snd_", 4 ) ) {
//			fc.string = new idStr( token );
//		} else {
//			fc.soundShader = declManager->FindSound( token );
//			if ( fc.soundShader->GetState() == DS_DEFAULTED ) {
//				gameLocal.Warning( "Sound '%s' not found", token.c_str() );
//			}
//		}
//	} else if ( token == "sound_weapon" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SOUND_WEAPON;
//		if ( !token.Cmpn( "snd_", 4 ) ) {
//			fc.string = new idStr( token );
//		} else {
//			fc.soundShader = declManager->FindSound( token );
//			if ( fc.soundShader->GetState() == DS_DEFAULTED ) {
//				gameLocal.Warning( "Sound '%s' not found", token.c_str() );
//			}
//		}
//	} else if ( token == "sound_global" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SOUND_GLOBAL;
//		if ( !token.Cmpn( "snd_", 4 ) ) {
//			fc.string = new idStr( token );
//		} else {
//			fc.soundShader = declManager->FindSound( token );
//			if ( fc.soundShader->GetState() == DS_DEFAULTED ) {
//				gameLocal.Warning( "Sound '%s' not found", token.c_str() );
//			}
//		}
//	} else if ( token == "sound_item" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SOUND_ITEM;
//		if ( !token.Cmpn( "snd_", 4 ) ) {
//			fc.string = new idStr( token );
//		} else {
//			fc.soundShader = declManager->FindSound( token );
//			if ( fc.soundShader->GetState() == DS_DEFAULTED ) {
//				gameLocal.Warning( "Sound '%s' not found", token.c_str() );
//			}
//		}
//	} else if ( token == "sound_chatter" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SOUND_CHATTER;
//		if ( !token.Cmpn( "snd_", 4 ) ) {
//			fc.string = new idStr( token );
//		} else {
//			fc.soundShader = declManager->FindSound( token );
//			if ( fc.soundShader->GetState() == DS_DEFAULTED ) {
//				gameLocal.Warning( "Sound '%s' not found", token.c_str() );
//			}
//		}
//	} else if ( token == "skin" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_SKIN;
//		if ( token == "none" ) {
//			fc.skin = NULL;
//		} else {
//			fc.skin = declManager->FindSkin( token );
//			if ( !fc.skin ) {
//				return va( "Skin '%s' not found", token.c_str() );
//			}
//		}
//	} else if ( token == "fx" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_FX;
//		if ( !declManager->FindType( DECL_FX, token.c_str() ) ) {
//			return va( "fx '%s' not found", token.c_str() );
//		}
//		fc.string = new idStr( token );
//	} else if ( token == "trigger" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_TRIGGER;
//		fc.string = new idStr( token );
//	} else if ( token == "triggerSmokeParticle" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_TRIGGER_SMOKE_PARTICLE;
//		fc.string = new idStr( token );
//	} else if ( token == "melee" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_MELEE;
//		if ( !gameLocal.FindEntityDef( token.c_str(), false ) ) {
//			return va( "Unknown entityDef '%s'", token.c_str() );
//		}
//		fc.string = new idStr( token );
//	} else if ( token == "direct_damage" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_DIRECTDAMAGE;
//		if ( !gameLocal.FindEntityDef( token.c_str(), false ) ) {
//			return va( "Unknown entityDef '%s'", token.c_str() );
//		}
//		fc.string = new idStr( token );
//	} else if ( token == "attack_begin" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_BEGINATTACK;
//		if ( !gameLocal.FindEntityDef( token.c_str(), false ) ) {
//			return va( "Unknown entityDef '%s'", token.c_str() );
//		}
//		fc.string = new idStr( token );
//	} else if ( token == "attack_end" ) {
//		fc.type = FC_ENDATTACK;
//	} else if ( token == "muzzle_flash" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		if ( ( token != "" ) && !modelDef->FindJoint( token ) ) {
//			return va( "Joint '%s' not found", token.c_str() );
//		}
//		fc.type = FC_MUZZLEFLASH;
//		fc.string = new idStr( token );
//	} else if ( token == "muzzle_flash" ) {
//		fc.type = FC_MUZZLEFLASH;
//		fc.string = new idStr( "" );
//	} else if ( token == "create_missile" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		if ( !modelDef->FindJoint( token ) ) {
//			return va( "Joint '%s' not found", token.c_str() );
//		}
//		fc.type = FC_CREATEMISSILE;
//		fc.string = new idStr( token );
//	} else if ( token == "launch_missile" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		if ( !modelDef->FindJoint( token ) ) {
//			return va( "Joint '%s' not found", token.c_str() );
//		}
//		fc.type = FC_LAUNCHMISSILE;
//		fc.string = new idStr( token );
//	} else if ( token == "fire_missile_at_target" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		jointInfo = modelDef->FindJoint( token );
//		if ( !jointInfo ) {
//			return va( "Joint '%s' not found", token.c_str() );
//		}
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_FIREMISSILEATTARGET;
//		fc.string = new idStr( token );
//		fc.index = jointInfo->num;
//#ifdef _D3XP
//	} else if ( token == "launch_projectile" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		if ( !declManager->FindDeclWithoutParsing( DECL_ENTITYDEF, token, false ) ) {
//			return "Unknown projectile def";
//		}
//		fc.type = FC_LAUNCH_PROJECTILE;
//		fc.string = new idStr( token );
//	} else if ( token == "trigger_fx" ) {
//
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		jointInfo = modelDef->FindJoint( token );
//		if ( !jointInfo ) {
//			return va( "Joint '%s' not found", token.c_str() );
//		}
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		if ( !declManager->FindType( DECL_FX, token, false ) ) {
//			return "Unknown FX def";
//		}
//
//		fc.type = FC_TRIGGER_FX;
//		fc.string = new idStr( token );
//		fc.index = jointInfo->num;
//
//	} else if ( token == "start_emitter" ) {
//
//		idStr str;
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		str = token + " ";
//
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		jointInfo = modelDef->FindJoint( token );
//		if ( !jointInfo ) {
//			return va( "Joint '%s' not found", token.c_str() );
//		}
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		str += token;
//		fc.type = FC_START_EMITTER;
//		fc.string = new idStr( str );
//		fc.index = jointInfo->num;
//
//	} else if ( token == "stop_emitter" ) {
//
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_STOP_EMITTER;
//		fc.string = new idStr( token );
//#endif
//	} else if ( token == "footstep" ) {
//		fc.type = FC_FOOTSTEP;
//	} else if ( token == "leftfoot" ) {
//		fc.type = FC_LEFTFOOT;
//	} else if ( token == "rightfoot" ) {
//		fc.type = FC_RIGHTFOOT;
//	} else if ( token == "enableEyeFocus" ) {
//		fc.type = FC_ENABLE_EYE_FOCUS;
//	} else if ( token == "disableEyeFocus" ) {
//		fc.type = FC_DISABLE_EYE_FOCUS;
//	} else if ( token == "disableGravity" ) {
//		fc.type = FC_DISABLE_GRAVITY;
//	} else if ( token == "enableGravity" ) {
//		fc.type = FC_ENABLE_GRAVITY;
//	} else if ( token == "jump" ) {
//		fc.type = FC_JUMP;
//	} else if ( token == "enableClip" ) {
//		fc.type = FC_ENABLE_CLIP;
//	} else if ( token == "disableClip" ) {
//		fc.type = FC_DISABLE_CLIP;
//	} else if ( token == "enableWalkIK" ) {
//		fc.type = FC_ENABLE_WALK_IK;
//	} else if ( token == "disableWalkIK" ) {
//		fc.type = FC_DISABLE_WALK_IK;
//	} else if ( token == "enableLegIK" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_ENABLE_LEG_IK;
//		fc.index = atoi( token );
//	} else if ( token == "disableLegIK" ) {
//		if( !src.ReadTokenOnLine( &token ) ) {
//			return "Unexpected end of line";
//		}
//		fc.type = FC_DISABLE_LEG_IK;
//		fc.index = atoi( token );
//	} else if ( token == "recordDemo" ) {
//		fc.type = FC_RECORDDEMO;
//		if( src.ReadTokenOnLine( &token ) ) {
//			fc.string = new idStr( token );
//		}
//	} else if ( token == "aviGame" ) {
//		fc.type = FC_AVIGAME;
//		if( src.ReadTokenOnLine( &token ) ) {
//			fc.string = new idStr( token );
//		}
//	} else {
//		return va( "Unknown command '%s'", token.c_str() );
//	}
//
//	// check if we've initialized the frame loopup table
//	if ( !frameLookup.Num() ) {
//		// we haven't, so allocate the table and initialize it
//		frameLookup.SetGranularity( 1 );
//		frameLookup.SetNum( anims[ 0 ]->NumFrames() );
//		for( i = 0; i < frameLookup.Num(); i++ ) {
//			frameLookup[ i ].num = 0;
//			frameLookup[ i ].firstCommand = 0;
//		}
//	}
//
//	// allocate space for a new command
//	frameCommands.Alloc();
//
//	// calculate the index of the new command
//	index = frameLookup[ framenum ].firstCommand + frameLookup[ framenum ].num;
//
//	// move all commands from our index onward up one to give us space for our new command
//	for( i = frameCommands.Num() - 1; i > index; i-- ) {
//		frameCommands[ i ] = frameCommands[ i - 1 ];
//	}
//
//	// fix the indices of any later frames to account for the inserted command
//	for( i = framenum + 1; i < frameLookup.Num(); i++ ) {
//		frameLookup[ i ].firstCommand++;
//	}
//
//	// store the new command
//	frameCommands[ index ] = fc;
//
//	// increase the number of commands on this frame
//	frameLookup[ framenum ].num++;
//
//	// return with no error
//	return NULL;
//}

/*
=====================
idAnim::CallFrameCommands
=====================
*/
//void idAnim::CallFrameCommands( idEntity *ent, int from, int to ) const { }
//	int index;
//	int end;
//	int frame;
//	int numframes;
//
//	numframes = anims[ 0 ]->NumFrames();
//
//	frame = from;
//	while( frame != to ) {
//		frame++;
//		if ( frame >= numframes ) {
//			frame = 0;
//		}
//
//		index = frameLookup[ frame ].firstCommand;
//		end = index + frameLookup[ frame ].num;
//		while( index < end ) {
//			const frameCommand_t &command = frameCommands[ index++ ];
//			switch( command.type ) {
//			case FC_SCRIPTFUNCTION: {
//				gameLocal.CallFrameCommand( ent, command.function );
//				break;
//			}
//			case FC_SCRIPTFUNCTIONOBJECT: {
//				gameLocal.CallObjectFrameCommand( ent, command.string->c_str() );
//				break;
//			}
//			case FC_EVENTFUNCTION: {
//				const idEventDef *ev = idEventDef::FindEvent( command.string->c_str() );
//				ent->ProcessEvent( ev );
//				break;
//			}
//			case FC_SOUND: {
//				if ( !command.soundShader ) {
//					if ( !ent->StartSound( command.string->c_str(), SND_CHANNEL_ANY, 0, false, NULL ) ) {
//						gameLocal.Warning( "Framecommand 'sound' on entity '%s', anim '%s', frame %d: Could not find sound '%s'",
//							ent->name.c_str(), FullName(), frame + 1, command.string->c_str() );
//					}
//				} else {
//					ent->StartSoundShader( command.soundShader, SND_CHANNEL_ANY, 0, false, NULL );
//				}
//				break;
//			}
//			case FC_SOUND_VOICE: {
//				if ( !command.soundShader ) {
//					if ( !ent->StartSound( command.string->c_str(), SND_CHANNEL_VOICE, 0, false, NULL ) ) {
//						gameLocal.Warning( "Framecommand 'sound_voice' on entity '%s', anim '%s', frame %d: Could not find sound '%s'",
//							ent->name.c_str(), FullName(), frame + 1, command.string->c_str() );
//					}
//				} else {
//					ent->StartSoundShader( command.soundShader, SND_CHANNEL_VOICE, 0, false, NULL );
//				}
//				break;
//			}
//			case FC_SOUND_VOICE2: {
//				if ( !command.soundShader ) {
//					if ( !ent->StartSound( command.string->c_str(), SND_CHANNEL_VOICE2, 0, false, NULL ) ) {
//						gameLocal.Warning( "Framecommand 'sound_voice2' on entity '%s', anim '%s', frame %d: Could not find sound '%s'",
//							ent->name.c_str(), FullName(), frame + 1, command.string->c_str() );
//					}
//				} else {
//					ent->StartSoundShader( command.soundShader, SND_CHANNEL_VOICE2, 0, false, NULL );
//				}
//				break;
//			}
//			case FC_SOUND_BODY: {
//				if ( !command.soundShader ) {
//					if ( !ent->StartSound( command.string->c_str(), SND_CHANNEL_BODY, 0, false, NULL ) ) {
//						gameLocal.Warning( "Framecommand 'sound_body' on entity '%s', anim '%s', frame %d: Could not find sound '%s'",
//							ent->name.c_str(), FullName(), frame + 1, command.string->c_str() );
//					}
//				} else {
//					ent->StartSoundShader( command.soundShader, SND_CHANNEL_BODY, 0, false, NULL );
//				}
//				break;
//			}
//			case FC_SOUND_BODY2: {
//				if ( !command.soundShader ) {
//					if ( !ent->StartSound( command.string->c_str(), SND_CHANNEL_BODY2, 0, false, NULL ) ) {
//						gameLocal.Warning( "Framecommand 'sound_body2' on entity '%s', anim '%s', frame %d: Could not find sound '%s'",
//							ent->name.c_str(), FullName(), frame + 1, command.string->c_str() );
//					}
//				} else {
//					ent->StartSoundShader( command.soundShader, SND_CHANNEL_BODY2, 0, false, NULL );
//				}
//				break;
//			}
//			case FC_SOUND_BODY3: {
//				if ( !command.soundShader ) {
//					if ( !ent->StartSound( command.string->c_str(), SND_CHANNEL_BODY3, 0, false, NULL ) ) {
//						gameLocal.Warning( "Framecommand 'sound_body3' on entity '%s', anim '%s', frame %d: Could not find sound '%s'",
//							ent->name.c_str(), FullName(), frame + 1, command.string->c_str() );
//					}
//				} else {
//					ent->StartSoundShader( command.soundShader, SND_CHANNEL_BODY3, 0, false, NULL );
//				}
//				break;
//			}
//			case FC_SOUND_WEAPON: {
//				if ( !command.soundShader ) {
//					if ( !ent->StartSound( command.string->c_str(), SND_CHANNEL_WEAPON, 0, false, NULL ) ) {
//						gameLocal.Warning( "Framecommand 'sound_weapon' on entity '%s', anim '%s', frame %d: Could not find sound '%s'",
//							ent->name.c_str(), FullName(), frame + 1, command.string->c_str() );
//					}
//				} else {
//					ent->StartSoundShader( command.soundShader, SND_CHANNEL_WEAPON, 0, false, NULL );
//				}
//				break;
//			}
//			case FC_SOUND_GLOBAL: {
//				if ( !command.soundShader ) {
//					if ( !ent->StartSound( command.string->c_str(), SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL ) ) {
//						gameLocal.Warning( "Framecommand 'sound_global' on entity '%s', anim '%s', frame %d: Could not find sound '%s'",
//							ent->name.c_str(), FullName(), frame + 1, command.string->c_str() );
//					}
//				} else {
//					ent->StartSoundShader( command.soundShader, SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL );
//				}
//				break;
//			}
//			case FC_SOUND_ITEM: {
//				if ( !command.soundShader ) {
//					if ( !ent->StartSound( command.string->c_str(), SND_CHANNEL_ITEM, 0, false, NULL ) ) {
//						gameLocal.Warning( "Framecommand 'sound_item' on entity '%s', anim '%s', frame %d: Could not find sound '%s'",
//							ent->name.c_str(), FullName(), frame + 1, command.string->c_str() );
//					}
//				} else {
//					ent->StartSoundShader( command.soundShader, SND_CHANNEL_ITEM, 0, false, NULL );
//				}
//				break;
//			}
//			case FC_SOUND_CHATTER: {
//				if ( ent->CanPlayChatterSounds() ) {
//					if ( !command.soundShader ) {
//						if ( !ent->StartSound( command.string->c_str(), SND_CHANNEL_VOICE, 0, false, NULL ) ) {
//							gameLocal.Warning( "Framecommand 'sound_chatter' on entity '%s', anim '%s', frame %d: Could not find sound '%s'",
//								ent->name.c_str(), FullName(), frame + 1, command.string->c_str() );
//						}
//					} else {
//						ent->StartSoundShader( command.soundShader, SND_CHANNEL_VOICE, 0, false, NULL );
//					}
//				}
//				break;
//			}
//			case FC_FX: {
//				idEntityFx::StartFx( command.string->c_str(), NULL, NULL, ent, true );
//				break;
//			}
//			case FC_SKIN: {
//				ent->SetSkin( command.skin );
//				break;
//			}
//			case FC_TRIGGER: {
//				idEntity *target;
//
//				target = gameLocal.FindEntity( command.string->c_str() );
//				if ( target ) {
//#ifdef _D3XP
//					SetTimeState ts(target->timeGroup);
//#endif
//					target->Signal( SIG_TRIGGER );
//					target->ProcessEvent( &EV_Activate, ent );
//					target->TriggerGuis();
//				} else {
//					gameLocal.Warning( "Framecommand 'trigger' on entity '%s', anim '%s', frame %d: Could not find entity '%s'",
//						ent->name.c_str(), FullName(), frame + 1, command.string->c_str() );
//				}
//				break;
//			}
//			case FC_TRIGGER_SMOKE_PARTICLE: {
//				ent->ProcessEvent( &AI_TriggerParticles, command.string->c_str() );
//				break;
//			}
//			case FC_MELEE: {
//				ent->ProcessEvent( &AI_AttackMelee, command.string->c_str() );
//				break;
//			}
//			case FC_DIRECTDAMAGE: {
//				ent->ProcessEvent( &AI_DirectDamage, command.string->c_str() );
//				break;
//			}
//			case FC_BEGINATTACK: {
//				ent->ProcessEvent( &AI_BeginAttack, command.string->c_str() );
//				break;
//			}
//			case FC_ENDATTACK: {
//				ent->ProcessEvent( &AI_EndAttack );
//				break;
//			}
//			case FC_MUZZLEFLASH: {
//				ent->ProcessEvent( &AI_MuzzleFlash, command.string->c_str() );
//				break;
//			}
//			case FC_CREATEMISSILE: {
//				ent->ProcessEvent( &AI_CreateMissile, command.string->c_str() );
//				break;
//			}
//			case FC_LAUNCHMISSILE: {
//				ent->ProcessEvent( &AI_AttackMissile, command.string->c_str() );
//				break;
//			}
//			case FC_FIREMISSILEATTARGET: {
//				ent->ProcessEvent( &AI_FireMissileAtTarget, modelDef->GetJointName( command.index ), command.string->c_str() );
//				break;
//			}
//#ifdef _D3XP
//			case FC_LAUNCH_PROJECTILE: {
//				ent->ProcessEvent( &AI_LaunchProjectile, command.string->c_str() );
//				break;
//			}
//			case FC_TRIGGER_FX: {
//				ent->ProcessEvent( &AI_TriggerFX, modelDef->GetJointName( command.index ), command.string->c_str() );
//				break;
//			}
//			case FC_START_EMITTER: {
//				int index = command.string->Find(" ");
//				if(index >= 0) {
//					idStr name = command.string->Left(index);
//					idStr particle = command.string->Right(command.string->Length() - index - 1);
//					ent->ProcessEvent( &AI_StartEmitter, name.c_str(), modelDef->GetJointName( command.index ), particle.c_str() );
//				}
//			}
//
//			case FC_STOP_EMITTER: {
//				ent->ProcessEvent( &AI_StopEmitter, command.string->c_str() );
//			}
//#endif
//			case FC_FOOTSTEP : {
//				ent->ProcessEvent( &EV_Footstep );
//				break;
//			}
//			case FC_LEFTFOOT: {
//				ent->ProcessEvent( &EV_FootstepLeft );
//				break;
//			}
//			case FC_RIGHTFOOT: {
//				ent->ProcessEvent( &EV_FootstepRight );
//				break;
//			}
//			case FC_ENABLE_EYE_FOCUS: {
//				ent->ProcessEvent( &AI_EnableEyeFocus );
//				break;
//			}
//			case FC_DISABLE_EYE_FOCUS: {
//				ent->ProcessEvent( &AI_DisableEyeFocus );
//				break;
//			}
//			case FC_DISABLE_GRAVITY: {
//				ent->ProcessEvent( &AI_DisableGravity );
//				break;
//			}
//			case FC_ENABLE_GRAVITY: {
//				ent->ProcessEvent( &AI_EnableGravity );
//				break;
//			}
//			case FC_JUMP: {
//				ent->ProcessEvent( &AI_JumpFrame );
//				break;
//			}
//			case FC_ENABLE_CLIP: {
//				ent->ProcessEvent( &AI_EnableClip );
//				break;
//			}
//			case FC_DISABLE_CLIP: {
//				ent->ProcessEvent( &AI_DisableClip );
//				break;
//			}
//			case FC_ENABLE_WALK_IK: {
//				ent->ProcessEvent( &EV_EnableWalkIK );
//				break;
//			}
//			case FC_DISABLE_WALK_IK: {
//				ent->ProcessEvent( &EV_DisableWalkIK );
//				break;
//			}
//			case FC_ENABLE_LEG_IK: {
//				ent->ProcessEvent( &EV_EnableLegIK, command.index );
//				break;
//			}
//			case FC_DISABLE_LEG_IK: {
//				ent->ProcessEvent( &EV_DisableLegIK, command.index );
//				break;
//			}
//			case FC_RECORDDEMO: {
//				if ( command.string ) {
//					cmdSystem->BufferCommandText( CMD_EXEC_NOW, va( "recordDemo %s", command.string->c_str() ) );
//				} else {
//					cmdSystem->BufferCommandText( CMD_EXEC_NOW, "stoprecording" );
//				}
//				break;
//			}
//			case FC_AVIGAME: {
//				if ( command.string ) {
//					cmdSystem->BufferCommandText( CMD_EXEC_NOW, va( "aviGame %s", command.string->c_str() ) );
//				} else {
//					cmdSystem->BufferCommandText( CMD_EXEC_NOW, "aviGame" );
//				}
//				break;
//			}
//			}
//		}
//	}
//}

/*
=====================
idAnim::FindFrameForFrameCommand
=====================
*/
int	idAnim::FindFrameForFrameCommand( frameCommandType_t framecommand, const frameCommand_t **command ) const {return -1;}
//	int frame;
//	int index;
//	int numframes;
//	int end;
//
//	if ( !frameCommands.Num() ) {
//		return -1;
//	}
//
//	numframes = anims[ 0 ]->NumFrames();
//	for( frame = 0; frame < numframes; frame++ ) {
//		end = frameLookup[ frame ].firstCommand + frameLookup[ frame ].num;
//		for( index = frameLookup[ frame ].firstCommand; index < end; index++ ) {
//			if ( frameCommands[ index ].type == framecommand ) {
//				if ( command ) {
//					*command = &frameCommands[ index ];
//				}
//				return frame;
//			}
//		}
//	}
//
//	if ( command ) {
//		*command = NULL;
//	}
//
//	return -1;
//}

/*
=====================
idAnim::HasFrameCommands
=====================
*/
bool idAnim::HasFrameCommands( void ) const {
	if ( !frameCommands.Num() ) {
		return false;
	}
	return true;
}

/*
=====================
idAnim::SetAnimFlags
=====================
*/
void idAnim::SetAnimFlags( const animFlags_t &animflags ) {
	flags = animflags;
}

/*
=====================
idAnim::GetAnimFlags
=====================
*/
const animFlags_t &idAnim::GetAnimFlags( void ) const {
	return flags;
}


/***********************************************************************

idAnimBlend

***********************************************************************/

/*
=====================
idAnimBlend::idAnimBlend
=====================
*/
idAnimBlend::idAnimBlend( void ) {
	Reset( NULL );
}

/*
=====================
idAnimBlend::Reset
=====================
*/
void idAnimBlend::Reset( const gltfData *_modelDef ) {
	modelDef	= _modelDef;
	cycle		= 1;
	starttime	= 0;
	endtime		= 0;
	timeOffset	= 0;
	rate		= 1.0f;
	frame		= 0;
	allowMove	= true;
	allowFrameCommands = true;
	animNum		= 0;

	memset( animWeights, 0, sizeof( animWeights ) );

	blendStartValue = 0.0f;
	blendEndValue	= 0.0f;
	blendStartTime	= 0;
	blendDuration	= 0;
}

/*
=====================
idAnimBlend::FullName
=====================
*/
const char *idAnimBlend::AnimFullName( void ) const {
	const idAnim *anim = Anim();
	if ( !anim ) {
		return "";
	}

	return anim->FullName();
}

/*
=====================
idAnimBlend::AnimName
=====================
*/
const char *idAnimBlend::AnimName( void ) const {
	const idAnim *anim = Anim();
	if ( !anim ) {
		return "";
	}

	return anim->Name();
}

/*
=====================
idAnimBlend::NumFrames
=====================
*/
int idAnimBlend::NumFrames( void ) const {
	const idAnim *anim = Anim();
	if ( !anim ) {
		return 0;
	}

	return anim->NumFrames();
}

/*
=====================
idAnimBlend::Length
=====================
*/
int	idAnimBlend::Length( void ) const {
	const idAnim *anim = Anim();
	if ( !anim ) {
		return 0;
	}

	return anim->Length();
}

/*
=====================
idAnimBlend::GetWeight
=====================
*/
float idAnimBlend::GetWeight( int currentTime ) const {
	int		timeDelta;
	float	frac;
	float	w;

	timeDelta = currentTime - blendStartTime;
	if ( timeDelta <= 0 ) {
		w = blendStartValue;
	} else if ( timeDelta >= blendDuration ) {
		w = blendEndValue;
	} else {
		frac = ( float )timeDelta / ( float )blendDuration;
		w = blendStartValue + ( blendEndValue - blendStartValue ) * frac;
	}

	return w;
}

/*
=====================
idAnimBlend::GetFinalWeight
=====================
*/
float idAnimBlend::GetFinalWeight( void ) const {
	return blendEndValue;
}

/*
=====================
idAnimBlend::SetWeight
=====================
*/
void idAnimBlend::SetWeight( float newweight, int currentTime, int blendTime ) {
	blendStartValue = GetWeight( currentTime );
	blendEndValue = newweight;
	blendStartTime = currentTime - 1;
	blendDuration = blendTime;

	if ( !newweight ) {
		endtime = currentTime + blendTime;
	}
}

/*
=====================
idAnimBlend::NumSyncedAnims
=====================
*/
int idAnimBlend::NumSyncedAnims( void ) const {
	const idAnim *anim = Anim();
	if ( !anim ) {
		return 0;
	}

	return anim->NumAnims();
}

/*
=====================
idAnimBlend::SetSyncedAnimWeight
=====================
*/
bool idAnimBlend::SetSyncedAnimWeight( int num, float weight ) {
	const idAnim *anim = Anim();
	if ( !anim ) {
		return false;
	}

	if ( ( num < 0 ) || ( num > anim->NumAnims() ) ) {
		return false;
	}

	animWeights[ num ] = weight;
	return true;
}

/*
=====================
idAnimBlend::SetFrame
=====================
*/
void idAnimBlend::SetFrame( const gltfData *modelDef, int _animNum, int _frame, int currentTime, int blendTime ) {
	Reset( modelDef );
	if ( !modelDef ) {
		return;
	}

	const idAnim *_anim = nullptr;///idAnim(modelDef->animations[ _animNum ];
	if ( !_anim ) {
		return;
	}

	//const gltfAnimation *md5anim = _anim->gltfAnim( 0 );
	//if ( modelDef-->Joints().Num() != md5anim->NumJoints() ) {
	//	gameLocal.Warning( "Model '%s' has different # of joints than anim '%s'", modelDef->GetModelName(), md5anim->Name() );
	//	return;
	//}

	animNum				= _animNum;
	starttime			= currentTime;
	endtime				= -1;
	cycle				= -1;
	animWeights[ 0 ]	= 1.0f;
	frame				= _frame;

	// a frame of 0 means it's not a single frame blend, so we set it to frame + 1
	if ( frame <= 0 ) {
		frame = 1;
	} else if ( frame > _anim->NumFrames() ) {
		frame = _anim->NumFrames();
	}

	// set up blend
	blendEndValue		= 1.0f;
	blendStartTime		= currentTime - 1;
	blendDuration		= blendTime;
	blendStartValue		= 0.0f;
}

/*
=====================
idAnimBlend::CycleAnim
=====================
*/
void idAnimBlend::CycleAnim( const gltfData *modelDef, int _animNum, int currentTime, int blendTime ) {
	Reset( modelDef );
	if ( !modelDef ) {
		return;
	}
	idRandom rnd(0);

	const idAnim *_anim = nullptr;//modelDef->GetAnim( _animNum );
	if ( !_anim ) {
		return;
	}

	const gltfAnimation *md5anim = _anim->gltfAnim( 0 );
	//if ( modelDef->Joints().Num() != md5anim->NumJoints() ) {
	//	gameLocal.Warning( "Model '%s' has different # of joints than anim '%s'", modelDef->GetModelName(), md5anim->Name() );
	//	return;
	//}

	animNum				= _animNum;
	animWeights[ 0 ]	= 1.0f;
	endtime				= -1;
	cycle				= -1;
	if ( _anim->GetAnimFlags().random_cycle_start ) {
		// start the animation at a random time so that characters don't walk in sync
		starttime = currentTime - rnd.RandomFloat() * _anim->Length();
	} else {
		starttime = currentTime;
	}

	// set up blend
	blendEndValue		= 1.0f;
	blendStartTime		= currentTime - 1;
	blendDuration		= blendTime;
	blendStartValue		= 0.0f;
}

/*
=====================
idAnimBlend::PlayAnim
=====================
*/
void idAnimBlend::PlayAnim( const gltfData *modelDef, int _animNum, int currentTime, int blendTime ) {
	Reset( modelDef );
	if ( !modelDef ) {
		return;
	}

	const idAnim *_anim = nullptr;//modelDef->GetAnim( _animNum );
	if ( !_anim ) {
		return;
	}

	const gltfAnimation *md5anim = _anim->gltfAnim( 0 );
	//if ( modelDef->Joints().Num() != md5anim->NumJoints() ) {
	//	gameLocal.Warning( "Model '%s' has different # of joints than anim '%s'", modelDef->GetModelName(), md5anim->Name() );
	//	return;
	//}

	animNum				= _animNum;
	starttime			= currentTime;
	endtime				= starttime + _anim->Length();
	cycle				= 1;
	animWeights[ 0 ]	= 1.0f;

	// set up blend
	blendEndValue		= 1.0f;
	blendStartTime		= currentTime - 1;
	blendDuration		= blendTime;
	blendStartValue		= 0.0f;
}

/*
=====================
idAnimBlend::Clear
=====================
*/
void idAnimBlend::Clear( int currentTime, int clearTime ) {
	if ( !clearTime ) {
		Reset( modelDef );
	} else {
		SetWeight( 0.0f, currentTime, clearTime );
	}
}

/*
=====================
idAnimBlend::IsDone
=====================
*/
bool idAnimBlend::IsDone( int currentTime ) const {
	if ( !frame && ( endtime > 0 ) && ( currentTime >= endtime ) ) {
		return true;
	}

	if ( ( blendEndValue <= 0.0f ) && ( currentTime >= ( blendStartTime + blendDuration ) ) ) {
		return true;
	}

	return false;
}

/*
=====================
idAnimBlend::FrameHasChanged
=====================
*/
bool idAnimBlend::FrameHasChanged( int currentTime ) const {
	// if we don't have an anim, no change
	if ( !animNum ) {
		return false;
	}

	// if anim is done playing, no change
	if ( ( endtime > 0 ) && ( currentTime > endtime ) ) {
		return false;
	}

	// if our blend weight changes, we need to update
	if ( ( currentTime < ( blendStartTime + blendDuration ) && ( blendStartValue != blendEndValue ) ) ) {
		return true;
	}

	// if we're a single frame anim and this isn't the frame we started on, we don't need to update
	if ( ( frame || ( NumFrames() == 1 ) ) && ( currentTime != starttime ) ) {
		return false;
	}

	return true;
}

/*
=====================
idAnimBlend::GetCycleCount
=====================
*/
int idAnimBlend::GetCycleCount( void ) const {
	return cycle;
}

/*
=====================
idAnimBlend::SetCycleCount
=====================
*/
void idAnimBlend::SetCycleCount( int count ) {
	const idAnim *anim = Anim();

	if ( !anim ) {
		cycle = -1;
		endtime = 0;
	} else {
		cycle = count;
		if ( cycle < 0 ) {
			cycle = -1;
			endtime	= -1;
		} else if ( cycle == 0 ) {
			cycle = 1;

			// most of the time we're running at the original frame rate, so avoid the int-to-float-to-int conversion
			if ( rate == 1.0f ) {
				endtime	= starttime - timeOffset + anim->Length();
			} else if ( rate != 0.0f ) {
				endtime	= starttime - timeOffset + anim->Length() / rate;
			} else {
				endtime = -1;
			}
		} else {
			// most of the time we're running at the original frame rate, so avoid the int-to-float-to-int conversion
			if ( rate == 1.0f ) {
				endtime	= starttime - timeOffset + anim->Length() * cycle;
			} else if ( rate != 0.0f ) {
				endtime	= starttime - timeOffset + ( anim->Length() * cycle ) / rate;
			} else {
				endtime = -1;
			}
		}
	}
}

/*
=====================
idAnimBlend::SetPlaybackRate
=====================
*/
void idAnimBlend::SetPlaybackRate( int currentTime, float newRate ) {
	int animTime;

	if ( rate == newRate ) {
		return;
	}

	animTime = AnimTime( currentTime );
	if ( newRate == 1.0f ) {
		timeOffset = animTime - ( currentTime - starttime );
	} else {
		timeOffset = animTime - ( currentTime - starttime ) * newRate;
	}

	rate = newRate;

	// update the anim endtime
	SetCycleCount( cycle );
}

/*
=====================
idAnimBlend::GetPlaybackRate
=====================
*/
float idAnimBlend::GetPlaybackRate( void ) const {
	return rate;
}

/*
=====================
idAnimBlend::SetStartTime
=====================
*/
void idAnimBlend::SetStartTime( int _startTime ) {
	starttime = _startTime;

	// update the anim endtime
	SetCycleCount( cycle );
}

/*
=====================
idAnimBlend::GetStartTime
=====================
*/
int idAnimBlend::GetStartTime( void ) const {
	if ( !animNum ) {
		return 0;
	}

	return starttime;
}

/*
=====================
idAnimBlend::GetEndTime
=====================
*/
int idAnimBlend::GetEndTime( void ) const {
	if ( !animNum ) {
		return 0;
	}

	return endtime;
}

/*
=====================
idAnimBlend::PlayLength
=====================
*/
int idAnimBlend::PlayLength( void ) const {
	if ( !animNum ) {
		return 0;
	}

	if ( endtime < 0 ) {
		return -1;
	}

	return endtime - starttime + timeOffset;
}

/*
=====================
idAnimBlend::AllowMovement
=====================
*/
void idAnimBlend::AllowMovement( bool allow ) {
	allowMove = allow;
}

/*
=====================
idAnimBlend::AllowFrameCommands
=====================
*/
void idAnimBlend::AllowFrameCommands( bool allow ) {
	allowFrameCommands = allow;
}


/*
=====================
idAnimBlend::Anim
=====================
*/
const idAnim *idAnimBlend::Anim( void ) const {
	if ( !modelDef ) {
		return NULL;
	}

	const idAnim *anim = nullptr;// modelDef->GetAnim( animNum );
	return anim;
}

/*
=====================
idAnimBlend::AnimNum
=====================
*/
int idAnimBlend::AnimNum( void ) const {
	return animNum;
}

/*
=====================
idAnimBlend::AnimTime
=====================
*/
int idAnimBlend::AnimTime( int currentTime ) const {
	int time;
	int length;
	const idAnim *anim = Anim();

	if ( anim ) {
		if ( frame ) {
			return FRAME2MS( frame - 1 );
		}

		// most of the time we're running at the original frame rate, so avoid the int-to-float-to-int conversion
		if ( rate == 1.0f ) {
			time = currentTime - starttime + timeOffset;
		} else {
			time = static_cast<int>( ( currentTime - starttime ) * rate ) + timeOffset;
		}

		// given enough time, we can easily wrap time around in our frame calculations, so
		// keep cycling animations' time within the length of the anim.
		length = anim->Length();
		if ( ( cycle < 0 ) && ( length > 0 ) ) {
			time %= length;

			// time will wrap after 24 days (oh no!), resulting in negative results for the %.
			// adding the length gives us the proper result.
			if ( time < 0 ) {
				time += length;
			}
		}
		return time;
	} else {
		return 0;
	}
}

/*
=====================
idAnimBlend::GetFrameNumber
=====================
*/
int idAnimBlend::GetFrameNumber( int currentTime ) const {
	const gltfAnimation	*md5anim;
	frameBlend_t	frameinfo;
	int				animTime;

	const idAnim *anim = Anim();
	if ( !anim ) {
		return 1;
	}

	if ( frame ) {
		return frame;
	}

	md5anim = anim->gltfAnim( 0 );
	animTime = AnimTime( currentTime );
	//md5anim->ConvertTimeToFrame( animTime, cycle, frameinfo );

	return frameinfo.frame1 + 1;
}

/*
=====================
idAnimBlend::BlendAnim
=====================
*/
bool idAnimBlend::BlendAnim( int currentTime, int channel, int numJoints, idJointQuat *blendFrame, float &blendWeight, bool removeOriginOffset, bool overrideBlend, bool printInfo ) const {
	int					i;
	float				lerp;
	float				mixWeight;
	const gltfAnimation	*md5anim;
	idJointQuat *ptr;
	frameBlend_t		frametime;
	idJointQuat *jointFrame;
	idJointQuat *mixFrame;
	int					numAnims;
	int					time;

	const idAnim *anim = Anim();
	if ( !anim ) {
		return false;
	}

	float weight = GetWeight( currentTime );
	if ( blendWeight > 0.0f ) {
		if ( ( endtime >= 0 ) && ( currentTime >= endtime ) ) {
			return false;
		}
		if ( !weight ) {
			return false;
		}
		if ( overrideBlend ) {
			blendWeight = 1.0f - weight;
		}
	}

	if ( ( channel == ANIMCHANNEL_ALL ) && !blendWeight ) {
		// we don't need a temporary buffer, so just store it directly in the blend frame
		jointFrame = blendFrame;
	} else {
		// allocate a temporary buffer to copy the joints from
		jointFrame = ( idJointQuat * )_alloca16( numJoints * sizeof( *jointFrame ) );
	}

	time = AnimTime( currentTime );

	numAnims = anim->NumAnims();
	if ( numAnims == 1 ) {
		//md5anim = anim->MD5Anim( 0 );
		//if ( frame ) {
		//	md5anim->GetSingleFrame( frame - 1, jointFrame, modelDef->GetChannelJoints( channel ), modelDef->NumJointsOnChannel( channel ) );
		//} else {
		//	md5anim->ConvertTimeToFrame( time, cycle, frametime );
		//	md5anim->GetInterpolatedFrame( frametime, jointFrame, modelDef->GetChannelJoints( channel ), modelDef->NumJointsOnChannel( channel ) );
		//}
	} else {
		//
		// need to mix the multipoint anim together first
		//
		// allocate a temporary buffer to copy the joints to
		mixFrame = ( idJointQuat * )_alloca16( numJoints * sizeof( *jointFrame ) );

		if ( !frame ) {
			common->DWarning("missing time convert" );
			//anim->gltfAnim( 0 )->ConvertTimeToFrame( time, cycle, frametime );
		}

		ptr = jointFrame;
		mixWeight = 0.0f;
		for( i = 0; i < numAnims; i++ ) {
			if ( animWeights[ i ] > 0.0f ) {
				//mixWeight += animWeights[ i ];
				//lerp = animWeights[ i ] / mixWeight;
				//md5anim = anim->MD5Anim( i );
				//if ( frame ) {
				//	md5anim->GetSingleFrame( frame - 1, ptr, modelDef->GetChannelJoints( channel ), modelDef->NumJointsOnChannel( channel ) );
				//} else {
				//	md5anim->GetInterpolatedFrame( frametime, ptr, modelDef->GetChannelJoints( channel ), modelDef->NumJointsOnChannel( channel ) );
				//}
				//
				//// only blend after the first anim is mixed in
				//if ( ptr != jointFrame ) {
				//	SIMDProcessor->BlendJoints( jointFrame, ptr, lerp, modelDef->GetChannelJoints( channel ), modelDef->NumJointsOnChannel( channel ) );
				//}

				ptr = mixFrame;
			}
		}

		if ( !mixWeight ) {
			return false;
		}
	}

	if ( removeOriginOffset ) {
		if ( allowMove ) {
#ifdef VELOCITY_MOVE
			jointFrame[ 0 ].t.x = 0.0f;
#else
			jointFrame[ 0 ].t.Zero();
#endif
		}

		if ( anim->GetAnimFlags().anim_turn ) {
			jointFrame[ 0 ].q.Set( -0.70710677f, 0.0f, 0.0f, 0.70710677f );
		}
	}

	if ( !blendWeight ) {
		blendWeight = weight;
		if ( channel != ANIMCHANNEL_ALL ) {
			const int *index = nullptr;//modelDef->GetChannelJoints( channel );
			const int num = 0;//modelDef->NumJointsOnChannel( channel );
			for( i = 0; i < num; i++ ) {
				int j = index[i];
				blendFrame[j].t = jointFrame[j].t;
				blendFrame[j].q = jointFrame[j].q;
			}
		}
	} else {
		blendWeight += weight;
		lerp = weight / blendWeight;
		//SIMDProcessor->BlendJoints( blendFrame, jointFrame, lerp, modelDef->GetChannelJoints( channel ), modelDef->NumJointsOnChannel( channel ) );
	}

	if ( printInfo ) {
		if ( frame ) {
			//common->Printf( "  %s: '%s', %d, %.2f%%\n", channelNames[ channel ], anim->FullName(), frame, weight * 100.0f );
		} else {
			//common->Printf( "  %s: '%s', %.3f, %.2f%%\n", channelNames[ channel ], anim->FullName(), ( float )frametime.frame1 + frametime.backlerp, weight * 100.0f );
		}
	}

	return true;
}

/*
=====================
idAnimBlend::BlendOrigin
=====================
*/
void idAnimBlend::BlendOrigin( int currentTime, idVec3 &blendPos, float &blendWeight, bool removeOriginOffset ) const {
	float	lerp;
	idVec3	animpos;
	idVec3	pos;
	int		time;
	int		num;
	int		i;

	if ( frame || ( ( endtime > 0 ) && ( currentTime > endtime ) ) ) {
		return;
	}

	const idAnim *anim = Anim();
	if ( !anim ) {
		return;
	}

	if ( allowMove && removeOriginOffset ) {
		return;
	}

	float weight = GetWeight( currentTime );
	if ( !weight ) {
		return;
	}

	time = AnimTime( currentTime );

	pos.Zero();
	num = anim->NumAnims();
	for( i = 0; i < num; i++ ) {
		anim->GetOrigin( animpos, i, time, cycle );
		pos += animpos * animWeights[ i ];
	}

	if ( !blendWeight ) {
		blendPos = pos;
		blendWeight = weight;
	} else {
		lerp = weight / ( blendWeight + weight );
		blendPos += lerp * ( pos - blendPos );
		blendWeight += weight;
	}
}

/*
=====================
idAnimBlend::BlendDelta
=====================
*/
void idAnimBlend::BlendDelta( int fromtime, int totime, idVec3 &blendDelta, float &blendWeight ) const {
	idVec3	pos1;
	idVec3	pos2;
	idVec3	animpos;
	idVec3	delta;
	int		time1;
	int		time2;
	float	lerp;
	int		num;
	int		i;

	if ( frame || !allowMove || ( ( endtime > 0 ) && ( fromtime > endtime ) ) ) {
		return;
	}

	const idAnim *anim = Anim();
	if ( !anim ) {
		return;
	}

	float weight = GetWeight( totime );
	if ( !weight ) {
		return;
	}

	time1 = AnimTime( fromtime );
	time2 = AnimTime( totime );
	if ( time2 < time1 ) {
		time2 += anim->Length();
	}

	num = anim->NumAnims();

	pos1.Zero();
	pos2.Zero();
	for( i = 0; i < num; i++ ) {
		anim->GetOrigin( animpos, i, time1, cycle );
		pos1 += animpos * animWeights[ i ];

		anim->GetOrigin( animpos, i, time2, cycle );
		pos2 += animpos * animWeights[ i ];
	}

	delta = pos2 - pos1;
	if ( !blendWeight ) {
		blendDelta = delta;
		blendWeight = weight;
	} else {
		lerp = weight / ( blendWeight + weight );
		blendDelta += lerp * ( delta - blendDelta );
		blendWeight += weight;
	}
}

/*
=====================
idAnimBlend::BlendDeltaRotation
=====================
*/
void idAnimBlend::BlendDeltaRotation( int fromtime, int totime, idQuat &blendDelta, float &blendWeight ) const {
	idQuat	q1;
	idQuat	q2;
	idQuat	q3;
	int		time1;
	int		time2;
	float	lerp;
	float	mixWeight;
	int		num;
	int		i;

	if ( frame || !allowMove || ( ( endtime > 0 ) && ( fromtime > endtime ) ) ) {
		return;
	}

	const idAnim *anim = Anim();
	if ( !anim || !anim->GetAnimFlags().anim_turn ) {
		return;
	}

	float weight = GetWeight( totime );
	if ( !weight ) {
		return;
	}

	time1 = AnimTime( fromtime );
	time2 = AnimTime( totime );
	if ( time2 < time1 ) {
		time2 += anim->Length();
	}

	q1.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	q2.Set( 0.0f, 0.0f, 0.0f, 1.0f );

	mixWeight = 0.0f;
	num = anim->NumAnims();
	for( i = 0; i < num; i++ ) {
		if ( animWeights[ i ] > 0.0f ) {
			mixWeight += animWeights[ i ];
			if ( animWeights[ i ] == mixWeight ) {
				anim->GetOriginRotation( q1, i, time1, cycle );
				anim->GetOriginRotation( q2, i, time2, cycle );
			} else {
				lerp = animWeights[ i ] / mixWeight;
				anim->GetOriginRotation( q3, i, time1, cycle );
				q1.Slerp( q1, q3, lerp );

				anim->GetOriginRotation( q3, i, time2, cycle );
				q2.Slerp( q1, q3, lerp );
			}
		}
	}

	q3 = q1.Inverse() * q2;
	if ( !blendWeight ) {
		blendDelta = q3;
		blendWeight = weight;
	} else {
		lerp = weight / ( blendWeight + weight );
		blendDelta.Slerp( blendDelta, q3, lerp );
		blendWeight += weight;
	}
}

/*
=====================
idAnimBlend::AddBounds
=====================
*/
bool idAnimBlend::AddBounds( int currentTime, idBounds &bounds, bool removeOriginOffset ) const {
	int			i;
	int			num;
	idBounds	b;
	int			time;
	idVec3		pos;
	bool		addorigin;

	if ( ( endtime > 0 ) && ( currentTime > endtime ) ) {
		return false;
	}

	const idAnim *anim = Anim();
	if ( !anim ) {
		return false;
	}

	float weight = GetWeight( currentTime );
	if ( !weight ) {
		return false;
	}

	time = AnimTime( currentTime );
	num = anim->NumAnims();

	addorigin = !allowMove || !removeOriginOffset;
	for( i = 0; i < num; i++ ) {
		if ( anim->GetBounds( b, i, time, cycle ) ) {
			if ( addorigin ) {
				anim->GetOrigin( pos, i, time, cycle );
				b.TranslateSelf( pos );
			}
			bounds.AddBounds( b );
		}
	}

	return true;
}
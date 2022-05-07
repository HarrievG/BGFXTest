#include "gltfAnimation.h"
#include "../idFramework/Common.h"
#include "../idFramework/idlib/containers/HashTable.h"

gltfAnimEditor * animEditor = nullptr;

idCVar gltfAnim_timescale( "gltfAnim_timescale", "1", CVAR_FLOAT, "timescale for swf files" );

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

	gltfAnimation * targetAnim = !anim ? *AnimationList().begin().p : anim ;
	auto animIt =  AnimationList().begin();

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

					if (state->currentFrame + 1 >= timeStamps.Num()  && deltaTime > frameDelta )
					{
						state->currentFrame = 0;
						state->lastAnimTime = currentTime;
						state->startTime = currentTime;
					}else if ( deltaTime >= frameDelta ) {
							state->currentFrame += 1;
							state->lastAnimTime = currentTime;
						}

					switch ( channel->target.TRS ) {
					case gltfAnimation_Channel_Target::rotation:
					{
						idList<idQuat*> & values = GetAccessorView<idQuat>(output);
						switch ( sampler.intType ) {
						case gltfAnimation_Sampler::linear:
							target->rotation = target->rotation.Slerp( *values[state->currentFrame], *values[nextFrame], ( 1.0f / frameDelta ) * deltaTime );
							break;
						case gltfAnimation_Sampler::step:
							target->rotation = *values[state->currentFrame];
							break;
						case gltfAnimation_Sampler::cubicSpline:
							common->Warning(" cubicSpline not implemented for TRS rotation" );
							break;
						default:
						case gltfAnimation_Sampler::count:
							common->Warning(" Unrecognized interpolation type" );
							break;
						}
					}	break;

					case  gltfAnimation_Channel_Target::translation:
					{
						idList<idVec3*> & values = GetAccessorView<idVec3>(output);
						switch ( sampler.intType ) {
						case gltfAnimation_Sampler::linear:
							target->translation.Lerp(*values[state->currentFrame], *values[nextFrame], ( 1.0f / frameDelta ) * deltaTime );
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
					}	break;
					case  gltfAnimation_Channel_Target::scale:
					{
						idList<idVec3*> & values = GetAccessorView<idVec3>(output);
						switch ( sampler.intType ) {

							break;
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

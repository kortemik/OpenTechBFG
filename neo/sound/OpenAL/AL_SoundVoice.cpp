/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Robert Beckebans
Copyright (C) 2014 Vincent Simonetti

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#pragma hdrstop
#include "../../idlib/precompiled.h"
#include "../snd_local.h"

idCVar s_skipHardwareSets( "s_skipHardwareSets", "0", CVAR_BOOL, "Do all calculation, but skip OpenAL calls" );
idCVar s_debugHardware( "s_debugHardware", "0", CVAR_BOOL, "Print a message any time a hardware voice changes" );

// The whole system runs at this sample rate
//static int SYSTEM_SAMPLE_RATE = 44100;
//static float ONE_OVER_SYSTEM_SAMPLE_RATE = 1.0f / SYSTEM_SAMPLE_RATE;

/*
========================
idSoundVoice_OpenAL::idSoundVoice_OpenAL
========================
*/
idSoundVoice_OpenAL::idSoundVoice_OpenAL()
:	triggered( false ),
	openalSource( 0 ),
	leadinSample( NULL ),
	loopingSample( NULL ),
	formatTag( 0 ),
	numChannels( 0 ),
	sampleRate( 0 ),
	paused( true ) {

	memset( &openalStreamingBuffer[0], 0, MAX_QUEUED_BUFFERS * sizeof( ALuint ) );
	memset( &lastOpenalStreamingBuffer[0], 0, MAX_QUEUED_BUFFERS * sizeof( ALuint ) );
}

/*
========================
idSoundVoice_OpenAL::~idSoundVoice_OpenAL
========================
*/
idSoundVoice_OpenAL::~idSoundVoice_OpenAL() {
	DestroyInternal();
}

/*
========================
idSoundVoice_OpenAL::CompatibleFormat
========================
*/
bool idSoundVoice_OpenAL::CompatibleFormat( idSoundSample_OpenAL * s ) {
	if( alIsSource( openalSource ) ) {
		// If this voice has never been allocated, then it's compatible with everything
		return true;
	}
	return false;
}

/*
========================
idSoundVoice_OpenAL::Create
========================
*/
void idSoundVoice_OpenAL::Create( const idSoundSample * leadinSample_, const idSoundSample * loopingSample_ ) {
	if ( IsPlaying() ) {
		// This should never hit
		Stop();
		return;
	}

	triggered = true;

	leadinSample = (idSoundSample_OpenAL *)leadinSample_;
	loopingSample = (idSoundSample_OpenAL *)loopingSample_;

	if ( alIsSource( openalSource ) && CompatibleFormat( leadinSample ) ) {
		sampleRate = leadinSample->format.basic.samplesPerSec;
	} else {
		DestroyInternal();
		formatTag = leadinSample->format.basic.formatTag;
		numChannels = leadinSample->format.basic.numChannels;
		sampleRate = leadinSample->format.basic.samplesPerSec;

		CheckALErrors();

		alGenSources( 1, &openalSource );
		if( CheckALErrors() != AL_NO_ERROR ) {
			// If this hits, then we are most likely passing an invalid sample format, which should have been caught by the loader (and the sample defaulted)
			return;
		}

		alSourcei( openalSource, AL_BUFFER, 0 );

		if( leadinSample->openalBuffer == 0 ) {
			alDeleteBuffers( MAX_QUEUED_BUFFERS, &lastOpenalStreamingBuffer[0] );

			memcpy( &lastOpenalStreamingBuffer[0], &openalStreamingBuffer[0], MAX_QUEUED_BUFFERS * sizeof( ALuint ) );

			alGenBuffers( MAX_QUEUED_BUFFERS, &openalStreamingBuffer[0] );
		}

		if ( s_debugHardware.GetBool() ) {
			if ( loopingSample == NULL || loopingSample == leadinSample ) {
				idLib::Printf( "%dms: %d created for %s\n", Sys_Milliseconds(), openalSource, leadinSample ? leadinSample->GetName() : "<null>" );
			} else {
				idLib::Printf( "%dms: %d created for %s and %s\n", Sys_Milliseconds(), openalSource, leadinSample ? leadinSample->GetName() : "<null>", loopingSample ? loopingSample->GetName() : "<null>" );
			}
		}
	}
	sourceVoiceRate = sampleRate;
	alSource3f( openalSource, AL_POSITION, -position.x, -position.y, -position.z );
	alSourcef( openalSource, AL_GAIN, 0.0f );
	alGetSourcef( openalSource, AL_MIN_GAIN, &minGain );
	alGetSourcef( openalSource, AL_MAX_GAIN, &maxGain );
}

/*
========================
idSoundVoice_OpenAL::DestroyInternal
========================
*/
void idSoundVoice_OpenAL::DestroyInternal() {
	if ( alIsSource( openalSource ) ) {
		if ( s_debugHardware.GetBool() ) {
			idLib::Printf( "%dms: %d destroyed\n", Sys_Milliseconds(), openalSource );
		}
		alDeleteSources( 1, &openalSource );
		openalSource = 0;

		if( openalStreamingBuffer[0] != 0 )
		{
			CheckALErrors();

			alDeleteBuffers( MAX_QUEUED_BUFFERS, &openalStreamingBuffer[0] );
			if( CheckALErrors() == AL_NO_ERROR )
			{
				memset( &openalStreamingBuffer[0], 0, MAX_QUEUED_BUFFERS * sizeof( ALuint ) );
			}
		}

		if( lastOpenalStreamingBuffer[0] != 0 )
		{
			CheckALErrors();

			alDeleteBuffers( MAX_QUEUED_BUFFERS, &lastOpenalStreamingBuffer[0] );
			if( CheckALErrors() == AL_NO_ERROR )
			{
				memset( &lastOpenalStreamingBuffer[0], 0, MAX_QUEUED_BUFFERS * sizeof( ALuint ) );
			}
		}
	}
}

/*
========================
idSoundVoice_OpenAL::Start
========================
*/
void idSoundVoice_OpenAL::Start( int offsetMS, int ssFlags ) {

	if ( s_debugHardware.GetBool() ) {
		idLib::Printf( "%dms: %d starting %s @ %dms\n", Sys_Milliseconds(), openalSource, leadinSample ? leadinSample->GetName() : "<null>", offsetMS );
	}

	if ( !leadinSample ) {
		return;
	}
	if ( !alIsSource( openalSource ) ) {
		return;
	}

	if ( leadinSample->IsDefault() ) {
		idLib::Warning( "Starting defaulted sound sample %s", leadinSample->GetName() );
	}

	assert( offsetMS >= 0 );
	int offsetSamples = MsecToSamples( offsetMS, leadinSample->SampleRate() );
	if ( loopingSample == NULL && offsetSamples >= leadinSample->playLength ) {
		return;
	}

	RestartAt( offsetSamples );
	Update();
	UnPause();
}

/*
========================
idSoundVoice_OpenAL::RestartAt
========================
*/
int idSoundVoice_OpenAL::RestartAt( int offsetSamples ) {
	offsetSamples &= ~127;

	idSoundSample_OpenAL * sample = leadinSample;
	if ( offsetSamples >= leadinSample->playLength ) {
		if ( loopingSample != NULL ) {
			offsetSamples %= loopingSample->playLength;
			sample = loopingSample;
		} else {
			return 0;
		}
	}

	int previousNumSamples = 0;
	for ( int i = 0; i < sample->buffers.Num(); i++ ) {
		if ( sample->buffers[i].numSamples > sample->playBegin + offsetSamples ) {
			return SubmitBuffer( sample, i, sample->playBegin + offsetSamples - previousNumSamples );
		}
		previousNumSamples = sample->buffers[i].numSamples;
	}

	return 0;
}

/*
========================
idSoundVoice_OpenAL::SubmitBuffer
========================
*/
int idSoundVoice_OpenAL::SubmitBuffer( idSoundSample_OpenAL * sample, int bufferNumber, int offset ) {

	if ( sample == NULL || ( bufferNumber < 0 ) || ( bufferNumber >= sample->buffers.Num() ) ) {
		return 0;
	}

	if( sample->openalBuffer != 0 ) {
		alSourcei( openalSource, AL_BUFFER, sample->openalBuffer );
		alSourcei( openalSource, AL_LOOPING, ( sample == loopingSample && loopingSample != NULL ? AL_TRUE : AL_FALSE ) );

		return sample->totalBufferSize;
	} else {
		ALint finishedbuffers;

		if( !triggered ) {
			alGetSourcei( openalSource, AL_BUFFERS_PROCESSED, &finishedbuffers );
			alSourceUnqueueBuffers( openalSource, finishedbuffers, &openalStreamingBuffer[0] );
			if( finishedbuffers == MAX_QUEUED_BUFFERS ) {
				triggered = true;
			}
		} else {
			finishedbuffers = MAX_QUEUED_BUFFERS;
		}

		ALenum format;

		if( sample->format.basic.formatTag == idWaveFile::FORMAT_PCM ) {
			format = sample->NumChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
		} else if( sample->format.basic.formatTag == idWaveFile::FORMAT_ADPCM ) {
			format = sample->NumChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
		} else if( sample->format.basic.formatTag == idWaveFile::FORMAT_XMA2 ) {
			format = sample->NumChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
		} else {
			format = sample->NumChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
		}

		int rate = sample->SampleRate(); /*44100*/

		for( int j = 0; j < finishedbuffers && j < 1; j++ ) {
			alBufferData( openalStreamingBuffer[j], format, sample->buffers[bufferNumber].buffer, sample->buffers[bufferNumber].bufferSize, rate );
		}

		if( finishedbuffers > 0 ) {
			alSourceQueueBuffers( openalSource, 1, &openalStreamingBuffer[0] );

			if( bufferNumber == 0 ) {
				triggered = false;
			}

			return sample->buffers[bufferNumber].bufferSize;
		}
	}

	// should never happen
	return 0;
}

/*
========================
idSoundVoice_OpenAL::Update
========================
*/
bool idSoundVoice_OpenAL::Update() {
	if ( !alIsSource( openalSource ) || leadinSample == NULL ) {
		return false;
	}

	if ( s_skipHardwareSets.GetBool() ) {
		return true;
	}

	assert( gain >= minGain && gain <= maxGain );
	alSourcef( openalSource, AL_GAIN, gain );
	alSourcef( openalSource, AL_PITCH, pitch );
	alSource3f( openalSource, AL_POSITION, -position.x, -position.y, -position.z );

	SetSampleRate( sampleRate, OPERATION_SET );

	// we don't do this any longer because we pause and unpause explicitly when the soundworld is paused or unpaused
	// UnPause();
	return true;
}

/*
========================
idSoundVoice_OpenAL::IsPlaying
========================
*/
bool idSoundVoice_OpenAL::IsPlaying() {
	if ( !alIsSource( openalSource ) ) {
		return false;
	}
	ALint queued;
	alGetSourceiv( openalSource, AL_BUFFERS_QUEUED, &queued );
	return ( queued != 0 );
}

/*
========================
idSoundVoice_OpenAL::FlushSourceBuffers
========================
*/
void idSoundVoice_OpenAL::FlushSourceBuffers() {
	if ( !alIsSource( openalSource ) ) {
		return;
	}

	ALint processed = 0;
	ALint tmp;
	ALuint* buffers;

	alGetSourcei( openalSource, AL_SOURCE_TYPE, &tmp );
	if( tmp != AL_STREAMING ) {
		alSourcei( openalSource, AL_BUFFER, 0 ); // Removes the buffer from reference
		return;
	}
	alGetSourcei( openalSource, AL_LOOPING, &tmp );
	if( tmp == AL_TRUE ) {
		return;
	}
	alGetSourcei( openalSource, AL_SOURCE_STATE, &tmp );
	if ( tmp != AL_STOPPED ) {
		// This will move all buffers to processed, allowing them to be freed
		alSourceStop( openalSource );
	}

	//Get processed buffers
	alGetSourceiv( openalSource, AL_BUFFERS_PROCESSED, &processed );

	if( processed > 0 )
	{
		//Allocate buffer storage
		buffers = ( ALuint* )malloc( processed * sizeof( ALuint ) );
		if( buffers == NULL )
		{
			//Just don't try
			if ( tmp == AL_PLAYING ) {
				alSourcePlay( openalSource );
			}
			return;
		}

		//Get buffers
		alSourceUnqueueBuffers( openalSource, processed, buffers );

		//Free buffers and cleanup
		alDeleteBuffers( processed, buffers );
		free( ( void* )buffers );
	}

	if ( tmp == AL_PLAYING ) {
		// Since this is based of IXAudio2SourceVoice::FlushSourceBuffers which removes buffers to be played without changing state, return to the original state
		alSourcePlay( openalSource );
	}
}

/*
========================
idSoundVoice_OpenAL::Pause
========================
*/
void idSoundVoice_OpenAL::Pause() {
	if ( !alIsSource( openalSource ) || paused ) {
		return;
	}
	if ( s_debugHardware.GetBool() ) {
		idLib::Printf( "%dms: %d pausing %s\n", Sys_Milliseconds(), openalSource, leadinSample ? leadinSample->GetName() : "<null>" );
	}
	alSourcePause( openalSource );
	paused = true;
}

/*
========================
idSoundVoice_OpenAL::UnPause
========================
*/
void idSoundVoice_OpenAL::UnPause() {
	if ( !alIsSource( openalSource ) || !paused ) {
		return;
	}
	if ( s_debugHardware.GetBool() ) {
		idLib::Printf( "%dms: %d unpausing %s\n", Sys_Milliseconds(), openalSource, leadinSample ? leadinSample->GetName() : "<null>" );
	}
	alSourcePlay( openalSource );
	paused = false;
}

/*
========================
idSoundVoice_OpenAL::Stop
========================
*/
void idSoundVoice_OpenAL::Stop() {
	if ( !alIsSource( openalSource ) ) {
		return;
	}
	if ( !paused ) {
		if ( s_debugHardware.GetBool() ) {
			idLib::Printf( "%dms: %d stopping %s\n", Sys_Milliseconds(), openalSource, leadinSample ? leadinSample->GetName() : "<null>" );
		}
		alSourceStop( openalSource );
		FlushSourceBuffers();
		paused = true;
	}
}

/*
========================
idSoundVoice_OpenAL::GetAmplitude
========================
*/
float idSoundVoice_OpenAL::GetAmplitude() {
	//XXX
	return 1.0f;
}

/*
========================
idSoundVoice_OpenAL::ResetSampleRate
========================
*/
void idSoundVoice_OpenAL::SetSampleRate( uint32 newSampleRate, uint32 operationSet ){
	//XXX
}

/*
========================
idSoundVoice_OpenAL::OnBufferStart
========================
*/
void idSoundVoice_OpenAL::OnBufferStart( idSoundSample_OpenAL * sample, int bufferNumber ) {
	SetSampleRate( sample->SampleRate(), 0/*XAUDIO2_COMMIT_NOW*/ );

	idSoundSample_OpenAL * nextSample = sample;
	int nextBuffer = bufferNumber + 1;
	if ( nextBuffer == sample->buffers.Num() ) {
		if ( sample == leadinSample ) {
			if ( loopingSample == NULL ) {
				return;
			}
			nextSample = loopingSample;
		}
		nextBuffer = 0;
	}

	SubmitBuffer( nextSample, nextBuffer, 0 );
}

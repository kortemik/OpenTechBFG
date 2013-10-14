/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

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
#include "../../idlib/precompiled.h"
#include "../snd_local.h"
#include "../../../doomclassic/doom/i_sound.h"

#include <sys/asoundlib.h>

idCVar s_device( "s_device", "-1", CVAR_INTEGER|CVAR_ARCHIVE, "Which audio device to use (listDevices to list, -1 for default)" );
extern idCVar s_volume_dB;

/*
========================
idSoundHardware_OpenAL::idSoundHardware_OpenAL
========================
*/
idSoundHardware_OpenAL::idSoundHardware_OpenAL() {
	openalContext = NULL;
	openalDevice = NULL;

	voices.SetNum( 0 );
	zombieVoices.SetNum( 0 );
	freeVoices.SetNum( 0 );

	lastResetTime = 0;
}

void listDevices_f( const idCmdArgs & args ) {

	ALCcontext * context = soundSystemLocal.hardware.GetContext();

	if ( context == NULL ) {
		idLib::Warning( "No OpenAL context object" );
		return;
	}

	//TODO
	/*
	UINT32 deviceCount = 0;
	if ( pXAudio2->GetDeviceCount( &deviceCount ) != S_OK || deviceCount == 0 ) {
		idLib::Warning( "No audio devices found" );
		return;
	}

	for ( unsigned int i = 0; i < deviceCount; i++ ) {
		XAUDIO2_DEVICE_DETAILS deviceDetails;
		if ( pXAudio2->GetDeviceDetails( i, &deviceDetails ) != S_OK ) {
			continue;
		}
		idStaticList< const char *, 5 > roles;
		if ( deviceDetails.Role & DefaultConsoleDevice ) {
			roles.Append( "Console Device" );
		}
		if ( deviceDetails.Role & DefaultMultimediaDevice ) {
			roles.Append( "Multimedia Device" );
		}
		if ( deviceDetails.Role & DefaultCommunicationsDevice ) {
			roles.Append( "Communications Device" );
		}
		if ( deviceDetails.Role & DefaultGameDevice ) {
			roles.Append( "Game Device" );
		}
		idStaticList< const char *, 11 > channelNames;
		if ( deviceDetails.OutputFormat.dwChannelMask & SPEAKER_FRONT_LEFT ) {
			channelNames.Append( "Front Left" );
		}
		if ( deviceDetails.OutputFormat.dwChannelMask & SPEAKER_FRONT_RIGHT ) {
			channelNames.Append( "Front Right" );
		}
		if ( deviceDetails.OutputFormat.dwChannelMask & SPEAKER_FRONT_CENTER ) {
			channelNames.Append( "Front Center" );
		}
		if ( deviceDetails.OutputFormat.dwChannelMask & SPEAKER_LOW_FREQUENCY ) {
			channelNames.Append( "Low Frequency" );
		}
		if ( deviceDetails.OutputFormat.dwChannelMask & SPEAKER_BACK_LEFT ) {
			channelNames.Append( "Back Left" );
		}
		if ( deviceDetails.OutputFormat.dwChannelMask & SPEAKER_BACK_RIGHT ) {
			channelNames.Append( "Back Right" );
		}
		if ( deviceDetails.OutputFormat.dwChannelMask & SPEAKER_FRONT_LEFT_OF_CENTER ) {
			channelNames.Append( "Front Left of Center" );
		}
		if ( deviceDetails.OutputFormat.dwChannelMask & SPEAKER_FRONT_RIGHT_OF_CENTER ) {
			channelNames.Append( "Front Right of Center" );
		}
		if ( deviceDetails.OutputFormat.dwChannelMask & SPEAKER_BACK_CENTER ) {
			channelNames.Append( "Back Center" );
		}
		if ( deviceDetails.OutputFormat.dwChannelMask & SPEAKER_SIDE_LEFT ) {
			channelNames.Append( "Side Left" );
		}
		if ( deviceDetails.OutputFormat.dwChannelMask & SPEAKER_SIDE_RIGHT ) {
			channelNames.Append( "Side Right" );
		}
		char mbcsDisplayName[ 256 ];
		wcstombs( mbcsDisplayName, deviceDetails.DisplayName, sizeof( mbcsDisplayName ) );
		idLib::Printf( "%3d: %s\n", i, mbcsDisplayName );
		idLib::Printf( "     %d channels, %d Hz\n", deviceDetails.OutputFormat.Format.nChannels, deviceDetails.OutputFormat.Format.nSamplesPerSec );
		if ( channelNames.Num() != deviceDetails.OutputFormat.Format.nChannels ) {
			idLib::Printf( S_COLOR_YELLOW "WARNING: " S_COLOR_RED "Mismatch between # of channels and channel mask\n" );
		}
		if ( channelNames.Num() == 1 ) {
			idLib::Printf( "     %s\n", channelNames[0] );
		} else if ( channelNames.Num() == 2 ) {
			idLib::Printf( "     %s and %s\n", channelNames[0], channelNames[1] );
		} else if ( channelNames.Num() > 2 ) {
			idLib::Printf( "     %s", channelNames[0] );
			for ( int i = 1; i < channelNames.Num() - 1; i++ ) {
				idLib::Printf( ", %s", channelNames[i] );
			}
			idLib::Printf( ", and %s\n", channelNames[channelNames.Num() - 1] );
		}
		if ( roles.Num() == 1 ) {
			idLib::Printf( "     Default %s\n", roles[0] );
		} else if ( roles.Num() == 2 ) {
			idLib::Printf( "     Default %s and %s\n", roles[0], roles[1] );
		} else if ( roles.Num() > 2 ) {
			idLib::Printf( "     Default %s", roles[0] );
			for ( int i = 1; i < roles.Num() - 1; i++ ) {
				idLib::Printf( ", %s", roles[i] );
			}
			idLib::Printf( ", and %s\n", roles[roles.Num() - 1] );
		}
	}
	*/
}

/*
========================
idSoundHardware_OpenAL::Init
========================
*/
void idSoundHardware_OpenAL::Init() {

	//XXX cmdSystem->AddCommand( "listDevices", listDevices_f, 0, "Lists the connected sound devices", NULL );

	// Get device
	openalDevice = alcOpenDevice( NULL );
	if ( openalDevice == NULL ) {
		idLib::FatalError( "Failed to open default OpenAL device." );
		return;
	}

	// Create context
	ALCint atts[] = {
		ALC_FREQUENCY, 44100, //Taken from XA2_SoundHardware::Init()'s outputSampleRate field
		0
	};
	openalContext = alcCreateContext( openalDevice, atts );
	ALCenum err = alcGetError( openalDevice );
	if ( !openalContext || err != ALC_NO_ERROR ) {
		alcCloseDevice( openalDevice );
		idLib::FatalError( "Failed to create OpenAL context. Error: %d.", err );
		return;
	}

	// Set current context
	alcMakeContextCurrent( openalContext );
	err = alcGetError( openalDevice );
	if ( err != ALC_NO_ERROR ) {
		alcDestroyContext( openalContext );
		alcCloseDevice( openalDevice );
		idLib::FatalError( "Failed to set current OpenAL context. Error: %d.", err );
		return;
	}

	alListenerf( AL_GAIN, DBtoLinear( s_volume_dB.GetFloat() ) );

	// ---------------------
	// Try to get information about the sound device
	// ---------------------
	snd_pcm_t * handle;
	snd_mixer_t * mixHandle;
	snd_pcm_info_t pcmInfo;
	snd_pcm_channel_info_t channelInfo;
	snd_mixer_info_t mixInfo;
	snd_mixer_groups_t mixGroups;
	snd_mixer_group_t mixGroup;
	ALCint mono, stereo, sourcesMax = -1;
	int outputChannels = 0;
	int channelMask = 0;

	const ALCchar* deviceName = alcGetString( openalDevice, ALC_DEVICE_SPECIFIER );

	if ( snd_pcm_open_name( &handle, deviceName, SND_PCM_OPEN_PLAYBACK ) >= 0 ) {

		channelInfo.channel = SND_PCM_CHANNEL_PLAYBACK;
		if ( snd_pcm_info( handle, &pcmInfo ) >= 0 && snd_pcm_channel_info( handle, &channelInfo ) >= 0 ) {

			//sourcesMax = channelInfo.max_voices; //Tests have shown this to be quite low, which isn't good for a game like Doom 3. Also, OpenAL takes care of this for us

			if ( snd_mixer_open( &mixHandle, pcmInfo.card, channelInfo.mixer_device ) >= 0 ) {

				if ( snd_mixer_info( mixHandle, &mixInfo ) >= 0 ) {

					mixGroups.groups_size = mixInfo.groups;
					mixGroups.pgroups = new (TAG_AUDIO) snd_mixer_gid_t[ mixInfo.groups ];

					if ( snd_mixer_groups( mixHandle, &mixGroups ) >= 0 && ( mixGroup.caps & SND_MIXER_GRPCAP_PLAY_GRP ) != 0 ) {
						//TODO: try to get the mixer group, from which we can get channels (and possibly channel mask)
					}

					delete [] mixGroups.pgroups;
				}

				snd_mixer_close( mixHandle );
			}
		}

		snd_pcm_close( handle );
	}
	if ( outputChannels == 0 ) {
		// Couldn't get audio info, use "defaults"
		outputChannels = 2;
		channelMask = idWaveFile::CHANNEL_MASK_FRONT_LEFT | idWaveFile::CHANNEL_MASK_FRONT_RIGHT; // Think: headphones
	}

	idSoundVoice::InitSurround( outputChannels, channelMask );

	// ---------------------
	// Initialize the Doom classic sound system.
	// ---------------------
	I_InitSoundHardware( outputChannels, channelMask );

	// Set the maximum number of "voices", AKA sources in OpenAL terms. Maximum number of sources is mono sources + stereo sources
	if ( sourcesMax < 1 ) {
		alcGetIntegerv( openalDevice, ALC_MONO_SOURCES, 1, &mono );
		alcGetIntegerv( openalDevice, ALC_STEREO_SOURCES, 1, &stereo );
		sourcesMax = mono + stereo;
	}

	voices.SetNum( Min( voices.Max(), sourcesMax ) );
	freeVoices.SetNum( Min( voices.Max(), sourcesMax ) );
	zombieVoices.SetNum( 0 );
	for ( int i = 0; i < voices.Num(); i++ ) {
		freeVoices[i] = &voices[i];
	}
}

/*
========================
idSoundHardware_OpenAL::Shutdown
========================
*/
void idSoundHardware_OpenAL::Shutdown() {
	for ( int i = 0; i < voices.Num(); i++ ) {
		voices[ i ].DestroyInternal();
	}
	voices.Clear();
	freeVoices.Clear();
	zombieVoices.Clear();

	// ---------------------
	// Shutdown the Doom classic sound system.
	// ---------------------
	I_ShutdownSoundHardware();

	alcMakeContextCurrent( NULL );
	if ( openalContext != NULL ) {
		alcDestroyContext( openalContext );
		openalContext = NULL;
	}
	if ( openalDevice != NULL ) {
		alcCloseDevice( openalDevice );
		openalDevice = NULL;
	}
}

/*
========================
idSoundHardware_OpenAL::AllocateVoice
========================
*/
idSoundVoice * idSoundHardware_OpenAL::AllocateVoice( const idSoundSample * leadinSample, const idSoundSample * loopingSample ) {
	if ( leadinSample == NULL ) {
		return NULL;
	}
	if ( loopingSample != NULL ) {
		if ( ( leadinSample->format.basic.formatTag != loopingSample->format.basic.formatTag ) || ( leadinSample->format.basic.numChannels != loopingSample->format.basic.numChannels ) ) {
			idLib::Warning( "Leadin/looping format mismatch: %s & %s", leadinSample->GetName(), loopingSample->GetName() );
			loopingSample = NULL;
		}
	}

	// Try to find a free voice that matches the format
	// But fallback to the last free voice if none match the format
	idSoundVoice * voice = NULL;
	for ( int i = 0; i < freeVoices.Num(); i++ ) {
		if ( freeVoices[i]->IsPlaying() ) {
			continue;
		}
		voice = (idSoundVoice *)freeVoices[i];
		if ( voice->CompatibleFormat( (idSoundSample_OpenAL*)leadinSample ) ) {
			break;
		}
	}
	if ( voice != NULL ) {
		voice->Create( leadinSample, loopingSample );
		freeVoices.Remove( voice );
		return voice;
	}

	return NULL;
}

/*
========================
idSoundHardware_OpenAL::FreeVoice
========================
*/
void idSoundHardware_OpenAL::FreeVoice( idSoundVoice * voice ) {
	voice->Stop();

	// In case we are still playing, wait a bit before setting the
	// voice as free.
	zombieVoices.Append( voice );
}

/*
========================
idSoundHardware_OpenAL::Update
========================
*/
void idSoundHardware_OpenAL::Update() {
	if ( openalContext == NULL ) {
		int nowTime = Sys_Milliseconds();
		if ( lastResetTime + 1000 < nowTime ) {
			lastResetTime = nowTime;
			Init();
		}
		return;
	}

	if ( soundSystem->IsMuted() ) {
		alListenerf( AL_GAIN, 0.0f );
	} else {
		alListenerf( AL_GAIN, DBtoLinear( s_volume_dB.GetFloat() ) );
	}

	for ( int i = 0; i < zombieVoices.Num(); i++ ) {
		zombieVoices[i]->FlushSourceBuffers();
		if ( !zombieVoices[i]->IsPlaying() ) {
			freeVoices.Append( zombieVoices[i] );
			zombieVoices.RemoveIndexFast( i );
			i--;
		} else {
			static int playingZombies;
			playingZombies++;
		}
	}
}

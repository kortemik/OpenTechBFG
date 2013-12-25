/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Robert Beckebans
Copyright (C) 2013 Vincent Simonetti

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
#include "../../../doomclassic/doom/i_sound.h"

#include <sys/asoundlib.h>

idCVar s_device( "s_device", "-1", CVAR_INTEGER|CVAR_ARCHIVE, "Which audio device to use (listDevices to list, -1 for default)" );
idCVar s_supportedMixers( "s_supportedMixers", "BT A2DP Out;HDMI Out;Headphone;Master;", CVAR_SOUND|CVAR_INIT, "Which audio mixers are supported for channel counting. Mixers must be separated by ';' without gaps between items, including the last element." );
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

/*
========================
idSoundHardware_OpenAL::PrintDeviceList

Taken from RBDOOM-3-BFG
========================
*/
void idSoundHardware_OpenAL::PrintDeviceList( const char* list )
{
	if( !list || *list == '\0' ) {
		idLib::Printf( "    !!! none !!!\n" );
	} else {
		do {
			idLib::Printf( "    %s\n", list );
			list += strlen( list ) + 1;
		}
		while( *list != '\0' );
	}
}

/*
========================
idSoundHardware_OpenAL::PrintALCInfo

Taken from RBDOOM-3-BFG
========================
*/
void idSoundHardware_OpenAL::PrintALCInfo( ALCdevice* device )
{
	ALCint major, minor;

	if( device ) {
		const ALCchar* devname = NULL;
		idLib::Printf( "\n" );
		if( alcIsExtensionPresent( device, "ALC_ENUMERATE_ALL_EXT" ) != AL_FALSE ) {
			devname = alcGetString( device, ALC_ALL_DEVICES_SPECIFIER );
		}

		if( alcGetError( device ) != ALC_NO_ERROR || !devname ) {
			devname = alcGetString( device, ALC_DEVICE_SPECIFIER );
		}

		idLib::Printf( "** Info for device \"%s\" **\n", devname );
	}
	alcGetIntegerv( device, ALC_MAJOR_VERSION, 1, &major );
	alcGetIntegerv( device, ALC_MINOR_VERSION, 1, &minor );

	if( alcGetError( device ) == ALC_NO_ERROR ) {
		idLib::Printf( "ALC version: %d.%d\n", major, minor );
	}

	if( device ) {
		idLib::Printf( "OpenAL extensions: %s", alGetString( AL_EXTENSIONS ) );

		//idLib::Printf("ALC extensions:");
		//printList(alcGetString(device, ALC_EXTENSIONS), ' ');
		alcGetError( device );
	}
}

/*
========================
listDevices_f

Taken from RBDOOM-3-BFG
========================
*/
void listDevices_f( const idCmdArgs& args ) {

	idLib::Printf( "Available playback devices:\n" );
	if( alcIsExtensionPresent( NULL, "ALC_ENUMERATE_ALL_EXT" ) != AL_FALSE ) {
		idSoundHardware_OpenAL::PrintDeviceList( alcGetString( NULL, ALC_ALL_DEVICES_SPECIFIER ) );
	} else {
		idSoundHardware_OpenAL::PrintDeviceList( alcGetString( NULL, ALC_DEVICE_SPECIFIER ) );
	}

	//idLib::Printf("Available capture devices:\n");
	//printDeviceList(alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER));

	if( alcIsExtensionPresent( NULL, "ALC_ENUMERATE_ALL_EXT" ) != AL_FALSE ) {
		idLib::Printf( "Default playback device: %s\n", alcGetString( NULL, ALC_DEFAULT_ALL_DEVICES_SPECIFIER ) );
	} else {
		idLib::Printf( "Default playback device: %s\n",  alcGetString( NULL, ALC_DEFAULT_DEVICE_SPECIFIER ) );
	}

	//idLib::Printf("Default capture device: %s\n", alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER));

	idSoundHardware_OpenAL::PrintALCInfo( NULL );

	idSoundHardware_OpenAL::PrintALCInfo( ( ALCdevice* )soundSystemLocal.hardware.GetOpenALDevice() );
}

/*
========================
idSoundHardware_OpenAL::Init
========================
*/
void idSoundHardware_OpenAL::Init() {

	cmdSystem->AddCommand( "listDevices", listDevices_f, 0, "Lists the connected sound devices", NULL );

	common->Printf( "Setup OpenAL device and context... " );

	// Get device
	openalDevice = alcOpenDevice( NULL );
	if ( openalDevice == NULL ) {
		idLib::FatalError( "Failed to open default OpenAL device." );
		return;
	}

	// Create context
	ALCint atts[] = {
		ALC_FREQUENCY, 44100, //Taken from XA2_SoundHardware::Init()'s outputSampleRate field. Also specified in XA2_SoundSource.cpp with SYSTEM_SAMPLE_RATE.
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

	common->Printf( "Done setting up OpenAL.\n" );

	const char *extensions = alGetString( AL_EXTENSIONS );

	common->Printf( "OpenAL vendor: %s\n", alGetString( AL_VENDOR ) );
	common->Printf( "OpenAL renderer: %s\n", alGetString( AL_RENDERER ) );
	common->Printf( "OpenAL version: %s\n", alGetString( AL_VERSION ) );
	common->Printf( "OpenAL extensions: %s\n", extensions );

#ifdef AL_EXT_IMA4
	hasIMA4Support = strstr( extensions, "AL_EXT_IMA4" ) != NULL;
#else
	hasIMA4Support = false;
#endif

	// ---------------------
	// Try to get information about the sound device
	//
	// Not really the prettiest, but it's the safest... makes a nice curve /s
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

					if ( snd_mixer_groups( mixHandle, &mixGroups ) >= 0 ) {

						unsigned int lcdChannelCount = 0xFFFFFFFF;
						unsigned int lcdChannelMask = 0;
						idStr groupName;

						for ( int m = 0; m < mixGroups.groups; m++) {

							//Create the name of the group to search for, then check if it exists
							groupName = mixGroups.pgroups[m].name;
							groupName.Append( ';' );

							if ( idStr::FindText( s_supportedMixers.GetString(), groupName, false ) ) {

								memcpy( &mixGroup.gid, &mixGroups.pgroups[m], sizeof( snd_mixer_gid_t ) );

								if ( snd_mixer_group_read( mixHandle, &mixGroup ) >= 0 && ( mixGroup.caps & SND_MIXER_GRPCAP_PLAY_GRP ) != 0 ) {

									unsigned int tChannelCount = 0;
									unsigned int tChannelMask = 0;

									//Max of 8 channels (7.1)
									for ( int channels = 0; channels < 8; channels++ ) {
										if ( ( mixGroup.channels & (1 << channels ) ) != 0 ) {
											tChannelCount++;
											tChannelMask |= (1 << channels ); //SND_MIXER_CHN_MASK_*
										}
									}

									if ( tChannelCount < lcdChannelCount ) {
										lcdChannelCount = tChannelCount;
										lcdChannelMask = tChannelMask;
									}
								}
								break;
							}
						}

						if ( lcdChannelMask != 0 ) {
							outputChannels = lcdChannelCount;

							//Do manually because bits between idWaveFile and SND_MIXER_CHN_MASK_* don't match
							if ( ( lcdChannelMask & SND_MIXER_CHN_MASK_FRONT_LEFT ) != 0) {
								channelMask |= idWaveFile::CHANNEL_MASK_FRONT_LEFT;
							}
							if ( ( lcdChannelMask & SND_MIXER_CHN_MASK_FRONT_RIGHT ) != 0) {
								channelMask |= idWaveFile::CHANNEL_MASK_FRONT_RIGHT;
							}
							if ( ( lcdChannelMask & SND_MIXER_CHN_MASK_FRONT_CENTER ) != 0) {
								channelMask |= idWaveFile::CHANNEL_MASK_FRONT_CENTER;
							}
							if ( ( lcdChannelMask & SND_MIXER_CHN_MASK_REAR_LEFT ) != 0) {
								channelMask |= idWaveFile::CHANNEL_MASK_BACK_LEFT;
							}
							if ( ( lcdChannelMask & SND_MIXER_CHN_MASK_REAR_RIGHT ) != 0) {
								channelMask |= idWaveFile::CHANNEL_MASK_BACK_RIGHT;
							}
							if ( ( lcdChannelMask & SND_MIXER_CHN_MASK_WOOFER ) != 0) {
								channelMask |= idWaveFile::CHANNEL_MASK_LOW_FREQUENCY;
							}
							if ( ( lcdChannelMask & SND_MIXER_CHN_MASK_SURR_LEFT ) != 0) {
								channelMask |= idWaveFile::CHANNEL_MASK_SIDE_LEFT;
							}
							if ( ( lcdChannelMask & SND_MIXER_CHN_MASK_SURR_RIGHT ) != 0) {
								channelMask |= idWaveFile::CHANNEL_MASK_SIDE_RIGHT;
							}
						}
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

	// Set the maximum number of "voices", AKA sources in OpenAL terms. Maximum number of sources is mono sources + stereo sources.
	// Technically doesn't matter, but there is "hearsay" limit of 256 (look at the original Doom 3 source with it's OpenAL implementation)
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

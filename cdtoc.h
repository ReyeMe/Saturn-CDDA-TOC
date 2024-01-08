#pragma once

#include <SEGA_CDC.H>

// -------------------------------------
// Constants
// -------------------------------------

/** @brief maximal number of tracks on disk
 */
#define CD_TRACK_COUNT (99)

// -------------------------------------
// Macros
// -------------------------------------

/** @brief Get track type flags
 *  @param track Track data
 */
#define CDTrackGetTypeFlags(track) (((track)->Control))

/** @brief Check whether track is audio
 *  @param track Track data
 */
#define CDTrackIsAudio(track) ((CDTrackGetTypeFlags(track) & 0x04) == 0x00)

/** @brief Check whether track is 4 channel audio
 *  @param track Track data
 */
#define CDTrackIsAudio4Channel(track) ((CDTrackGetTypeFlags(track) & 0x0C) == 0x08)

/** @brief Check whether audio track has pre-emphasis
 *  @param track Track data
 */
#define CDTrackIsAudioWithPreEmphasis(track) ((CDTrackGetTypeFlags(track) & 0x05) == 0x01)

/** @brief Check whether track is data, recorded uninterrupted
 *  @param track Track data
 */
#define CDTrackIsData(track) ((CDTrackGetTypeFlags(track) & 0x0C) == 0x04)

/** @brief Check whether track is data, recorded incrementally
 *  @param track Track data
 */
#define CDTrackIsDataIncremental(track) ((CDTrackGetTypeFlags(track) & 0x0D) == 0x05)

/** @brief Check whether digital copy of this track is permitted
 *  @param track Track data
 */
#define CDTrackIsCopyPermitted(track) ((CDTrackGetTypeFlags(track) & 0x02) == 0x02)

/** @brief Check whether track is empty
 *  @param track Track data
 */
#define CDTrackIsEmpty(track) (CDTrackGetTypeFlags(track) == 0x0F)

// -------------------------------------
// Types
// -------------------------------------

/** @brief Track location data
 */
typedef struct
{
    unsigned int Control:4;
    unsigned int Number:4;
    unsigned int fad:24;
} CDTrackLocation;

/** @brief Track information data
 */
typedef struct
{
    unsigned char Control:4;
    unsigned char Address:4;
    unsigned char Number;
	union {
		short point;
		struct {
			char psec;
			char pframe;
		} pData;
		
	}pBody;
	
} CDTrackInformation;

/** @brief Session data
 */
typedef struct
{
    unsigned int Control:4;
    unsigned int Address:4;
    unsigned int fad:24;
} CDSession;

/** @brief Table of contents
 */
typedef struct
{
    CDTrackLocation Tracks[CD_TRACK_COUNT];
    CDTrackInformation FirstTrack;
    CDTrackInformation LastTrack;
    CDSession Session;
} CDTableOfContents;

// -------------------------------------
// Functions
// -------------------------------------

/** @brief Gets table of contents
 * @param toc Table of contents data struct
 */
void CDGetTableOfContents(CDTableOfContents * toc)	
{
    CDC_TgetToc((Uint32*)toc);
}

/** @brief Play audio
 * @param fromTrack Start track
 * @param toTrack End track
 * @param loop Loop playback
 * @param startAddress Start address of the playback
 */
void CDDAPlay(int fromTrack, int toTrack, bool loop, int startAddress)
{
	CdcPly ply;

	// Get TOC
	CDTableOfContents toc;
	CDGetTableOfContents(&toc);

	// Start of the playback address
    CDC_PLY_STYPE(&ply) = CDC_PTYPE_FAD; // track number

	if (startAddress == 0)
	{

		CDC_PLY_SFAD(&ply) = toc.Tracks[fromTrack].fad;
	}
	else
	{
		CDC_PLY_SFAD(&ply) = startAddress;
	}

	if (toTrack + 1 < CD_TRACK_COUNT)
	{
		// End of the playback address is start of next track
    	CDC_PLY_ETYPE(&ply) = CDC_PTYPE_FAD;
		CDC_PLY_EFAS(&ply) = toc.Tracks[toTrack + 1].fad - toc.Tracks[fromTrack].fad;
	}
	else
	{
		// End of the playback is end of the disk
    	CDC_PLY_ETYPE(&ply) = CDC_PTYPE_DFL;
	}

	// Playback mode
    CDC_PLY_PMODE(&ply) = CDC_PM_DFL | (loop ? 0xf : 0); // 0xf = infinite repetitions

	// Start playback
    CDC_CdPlay(&ply);
}

/** @brief Stop audio playback
 * @return Position of where playback stopped
 */
int CDDAStop(void)
{
	// Get current address
	CdcStat stat;
	CDC_GetCurStat(&stat);

	// Restore address
    CdcPos poswk;
    CDC_POS_PTYPE(&poswk) = CDC_PTYPE_DFL;
    CDC_CdSeek(&poswk);

	// Last playback address
	return stat.report.fad;
}
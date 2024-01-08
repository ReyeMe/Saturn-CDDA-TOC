#include <jo/jo.h>

/* Include new CDTOC features */
#include "cdtoc.h"

/* Table of contents */
CDTableOfContents CdContent;

/* Current track selection arrow */
int TrackNumber = 0;

/* Indicates whether music is playing right now */
bool isPlaying;

/* last playback position */
int lastPosition;

/** @brief Main demo loop
 */
void DemoLogic()
{
    // This is just here to delay button input
    static int buttonDelay = 0;

    if (buttonDelay > 0)
    {
        buttonDelay--;
    }

    // Read inputs
    if (jo_is_pad1_available())
    {
        if (buttonDelay <= 0)
        {
            // Move track selection
            if (jo_is_pad1_key_pressed(JO_KEY_UP) && TrackNumber > 0)
            {
                TrackNumber--;
                buttonDelay = 10;
            }
            else if (jo_is_pad1_key_pressed(JO_KEY_DOWN) && TrackNumber < CD_TRACK_COUNT - 1)
            {
                TrackNumber++;
                buttonDelay = 10;
            }
        }
        
        // Play selected audio track when pressing START button
        if (jo_is_pad1_key_down(JO_KEY_START) &&
            CDTrackIsAudio(&CdContent.Tracks[TrackNumber]))
        {
			isPlaying = true;
            CDDAPlay(TrackNumber, TrackNumber, false, 0);
        }
		
        // Pause or resume selected audio track when pressing START button
        if (jo_is_pad1_key_down(JO_KEY_Z) &&
            CDTrackIsAudio(&CdContent.Tracks[TrackNumber]))
        {
			if (isPlaying)
			{
				lastPosition = CDDAStop();
			}
			else
			{
				CDDAPlay(TrackNumber, TrackNumber, false, lastPosition);
			}

			isPlaying = !isPlaying;
        }
    }
}

/** @brief Redering loop
 */
void DemoDraw()
{
    // Where table content starts drawing at
    static int RowOffsetStart = 0;
	
    // Show at most 20 tracks on screen at once starting on sixth row
    static int rowStart = 7;
    static int rowCount = 20;

    // Show info about what track is first and what is last
    jo_printf(1,1, "First track: %d", CdContent.FirstTrack.Number);
    jo_printf(1,2, "Last track: %d", CdContent.LastTrack.Number);

    // Draw track table header
    jo_printf(1, 5, "   ID | Type");
    for (int col = 0; col < 40; col++) jo_printf(col, 6, "-");

    // List all tracks
    for (int row = 0; row < rowCount; row++)
    {
        // Get track index and what row the track is on
        int index = row + RowOffsetStart;
        int rowPosition = row + rowStart;

        // Clear row before drawing
        jo_clear_screen_line(rowPosition);

        // Draw row
        if (index < CD_TRACK_COUNT && index >= 0)
        {
            // print location and arrow
            char * selectedArrow = (index == TrackNumber ? "->" : "  ");

            // Check what type track is (Audio, data, empty)
            if (CDTrackIsAudio(&CdContent.Tracks[index]))
            {
                jo_printf(1, rowPosition, "%s %02d | Audio", selectedArrow, index + 1);
            }
            else if (CDTrackIsData(&CdContent.Tracks[index]))
            {
                jo_printf(1, rowPosition, "%s %02d | Data", selectedArrow, index + 1);
            }
            else if (CDTrackIsEmpty(&CdContent.Tracks[index]))
            {
                jo_printf(1, rowPosition, "%s %02d | -", selectedArrow, index + 1);
            }
            else
            {
                jo_printf(1, rowPosition, "%s %02d | Invalid (%d)", selectedArrow, index + 1, CdContent.Tracks[index].Control);
            }
        }
    }
    
    // Scroll
    if (RowOffsetStart + rowCount <= TrackNumber && RowOffsetStart < CD_TRACK_COUNT)
	{
        RowOffsetStart++;
    }
    else if (RowOffsetStart > TrackNumber && RowOffsetStart > 0)
	{
        RowOffsetStart--;
    }
    
    // Show play message
    if (CDTrackIsAudio(&CdContent.Tracks[TrackNumber]))
    {
        jo_printf(2,rowCount + rowStart + 1, "[START] to Play, [X] to pause/resume");
    }
    else
    {
        jo_clear_screen_line(rowCount + rowStart + 1);
    }
}

/** @brief Application entry point
 */
void jo_main(void)
{
    // Initialize Jo
    jo_core_init(JO_COLOR_Black);

    // Bind START+ABC to go to cd player
    jo_core_set_restart_game_callback(jo_goto_boot_menu);

    // Load table of content
    CDGetTableOfContents(&CdContent);

    // Register callbacks
    jo_core_add_callback(DemoDraw);
    jo_core_add_callback(DemoLogic);

    // Start demo
    jo_core_run();
}

/// \file uimessages.h Application Interface Input/Output definitions for VSOS
/// \version 0.07 2015-01-15

#ifndef UIMESSAGES_H
#define UIMESSAGES_H

typedef int (*UICallback)(s_int16 index,u_int16 message,u_int32 value);

int SystemUiMessage(s_int16 index, u_int16 message, u_int32 value);

/// Function for receiving messages
/// \param index Index to application's list of accepted messages, -1 if unknown
/// \param message Type of message to sent, see list of definitions
/// \param value New value of the message
int UiCallbackFunction(s_int16 index, u_int16 message, u_int32 value);

/// An application declares a list of messages it understands.
/// It is specified as a UIMSG_NONE-terminated array of uiMessageListItems.
typedef struct UiMessageListItemStruct {
	u_int16 message_type; //Message type, see list below
	const char *caption; //Pointer to a decorative caption (for automatic UI generator)
} UiMessageListItem;

typedef struct UiMessageStruct {
	u_int16 index;
	u_int16 message;
	u_int32 value;
} UiMessage;

//UI Message definitions

//CATEGORY:VSOS RESERVED 0x0000-0x00FF
#define UIMSG_VSOS_END_OF_LIST 0x0000 //No action / End of list
#define UIMSG_VSOS_GET_MSG_LIST 0x0001 //Ask for a list of uiMessageListItems
#define UIMSG_VSOS_SET_CALLBACK_FUNCTION 0x0002 //Pointer to a function which receives ui messages
#define UIMSG_VSOS_MSG_LIST 0x0003 //Pointer to a list of uiMessageListItems
#define UIMSG_VSOS_SET_CURRENT_PLAYLIST 0x000A // Pointer to a PLAYLIST object (do NOT use after this)
#define UIMSG_VSOS_NO_MORE_FILES 0x000B // sent after a playlist has finished playing
#define UIMSG_VSOS_QUERY_PLAYLIST_ITEM 0x000C // query items on a playlist
#define UIMSG_VSOS_END_OF_REPORT 0x000D
#define UIMSG_VSOS_SET_SPECTRUM_ANALYZER_CALLBACK 0x000E //Ask model to set spectrum analyzer
#define UIMSG_VSOS_AUDIO_DECODER_LOADED 0x000F //Model tells that a decoder has been loaded or dropped
#define UIMSG_DECODE_FILE 0x0010 //Instruction to decode a file pointer
//SUBCATEGORY: HINTS 
//These provide extra information which is useful for rendering the user interface
#define UIMSG_HINT 0x0004 //Auxiliry information for user interface generators
#define UIMSG_INPUTS 0x0005 //Start of a category of input (control) messages
#define UIMSG_OUTPUTS 0x0006 //Start of a category of output (status) messages
#define UIMSG_USE_SAME_POSITION 0x0007 //Use the same position on screen to display
#define UIMSG_USE_SAME_LINE 0x0008 //Use the same line on screen to display

//CATEGORY:BOOL 0x0100-0x01FF
#define UIMSG_BOOLEAN 0x0100 //Uncategorized boolean value

//SUBCATEGORY BOOLEAN:BUTTON 0x0101-0x017F
//Buttons are booleans which are resting at value 0 and the transition from 0 to 1 triggers a functionality
#define UIMSG_IS_BUTTON(x) (((x)&0xFF80U) == 0x0100)
#define UIMSG_BUTTON 0x0100 // Any kind of button
#define UIMSG_BUT 0x0101 // Unspecified Button
#define UIMSG_BUT_PLAY 0x0102 // Start Playing
#define UIMSG_BUT_STOP 0x0103 // Stop Playing
#define UIMSG_BUT_PAUSE 0x0104 // Pause Button, 1=Go to Pause state, 0=Release Pause State
#define UIMSG_BUT_PAUSE_TOGGLE 0x0105 // Pause Button, Toggle pause state
#define UIMSG_BUT_RESTART 0x0106 // Restart the currently playing file
#define UIMSG_BUT_PREVIOUS 0x0107 // Switch to the previous track
#define UIMSG_BUT_NEXT 0x0108 // Switch to the next track
#define UIMSG_BUT_GENERIC_TOUCH 0x109 // Touchpad is touched
#define UIMSG_BUT_CLOSE_DECODER 0x10a // Close the decoder application
#define UIMSG_BUT_DECODER_CLOSED 0x10b // Decoder application is closing
#define UIMSG_BUT_FIRST 0x010c // Switch to the first track
#define UIMSG_BUT_LAST 0x010d // Switch to the last track
#define UIMSG_BUT_END_OF_SONG 0x010e // Report that end of song has been reached
#define UIMSG_BUT_SAVE_POS 0x010f // Save current audio file playback position
#define UIMSG_BUT_RESTORE_POS 0x0110 // Restore previously saved position
#define UIMSG_BUT_VOLUME_UP 0x0111 // Volume up by halfDb
#define UIMSG_BUT_VOLUME_DOWN 0x0112 // Volume down by halfDb
#define UIMSG_BUT_PREVPAGE 0x0120
#define UIMSG_BUT_NEXTPAGE 0x0121

//Subcategory BOOLEAN:BOOL 0x0180-01FF
//Switches are booleans which can rest at state 0 or 1 and the state, 0 or 1 carries a meaning.
#define UIMSG_BOOL 0x0180 //Unspecified Switch
#define UIMSG_BOOL_FAST_FORWARD 0x0181 //Cue forwards
#define UIMSG_BOOL_FAST_BACKWARD 0x0182 //Cue backwards
#define UIMSG_BOOL_SHUFFLE 0x0183 //Shuffle on(1)/off(0)
#define UIMSG_BOOL_FORCE_MONO 0x0184 //Mix audio to mono(1) or don't mix(0)
#define UIMSG_BOOL_DIFFERENTIAL_OUTPUT 0x0185 //Set differential(1) or normal(0) output
#define UIMSG_DISPLAY_ON 0x0186 //Display or user interface on(1) or off(0)

//CATEGORY:NUMERIC 0x0200-0x02FF

//Subcategory NUMERIC:S16/U16 16-bit values 0x0200-0x02FF
#define UIMSG_S16 0x0200 //Unspecified signed 16-bit integer value
#define UIMSG_S16_PERCENTAGE 0x0202 //Unspecified value from 0 to 100
#define UIMSG_S16_LOOP_STATE 0x0203 // See below for values
#define UIMSG_S16_PLAYING 0x0204 // See below for values
#define UIMSG_S16_SONG_NUMBER 0x0205 // Set Base-1 song # in folder/playlist, <0 = fail
#define UIMSG_S16_SET_VOLUME 0x0206 //set volume attenuation in halfdb e.g. 20 = -10dB
#define UIMSG_S16_SET_BALANCE 0x0207 //set volume attenuation in halfdb e.g. 20 = -10dB
#define UIMSG_S16_NEXT_SONG_NUMBER 0x0208 //set number of next track to play after the current one finishes
#define UIMSG_S16_MAX_VU 0x0209 //Max VU meter value since last report
#define UIMSG_S16_SET_SPEED 0x020A //16384 = 1.0x speed
#define UIMSG_S16_SET_PITCH 0x020B //16384 = 1.0x pitch
#define UIMSG_S16_FILTER_1_GAIN 0x0210 //Set gain for ftequ equalizer filter 1
#define UIMSG_S16_FILTER_2_GAIN 0x0211 //Set gain for ftequ equalizer filter 2
#define UIMSG_S16_FILTER_3_GAIN 0x0212 //Set gain for ftequ equalizer filter 3
#define UIMSG_S16_FILTER_4_GAIN 0x0213 //Set gain for ftequ equalizer filter 4
#define UIMSG_S16_FILTER_5_GAIN 0x0214 //Set gain for ftequ equalizer filter 5
#define UIMSG_S16_FILTER_6_GAIN 0x0215 //Set gain for ftequ equalizer filter 6
#define UIMSG_S16_FILTER_7_GAIN 0x0216 //Set gain for ftequ equalizer filter 7
#define UIMSG_S16_FILTER_8_GAIN 0x0217 //Set gain for ftequ equalizer filter 8
#define UIMSG_S16_FILTER_9_GAIN 0x0218 //Set gain for ftequ equalizer filter 9 
#define UIMSG_S16_FILTER_10_GAIN 0x0219 //Set gain for ftequ equalizer filter 10
#define UIMSG_S16_FILTER_11_GAIN 0x021a //Set gain for ftequ equalizer filter 11
#define UIMSG_S16_FILTER_12_GAIN 0x021b //Set gain for ftequ equalizer filter 12
#define UIMSG_S16_FILTER_13_GAIN 0x021c //Set gain for ftequ equalizer filter 13
#define UIMSG_S16_FILTER_14_GAIN 0x021d //Set gain for ftequ equalizer filter 14
#define UIMSG_S16_FILTER_15_GAIN 0x021e //Set gain for ftequ equalizer filter 15
#define UIMSG_S16_FILTER_16_GAIN 0x021f //Set gain for ftequ equalizer filter 16

#define UIMSG_U16 0x0280 //Unspecified unsigned 16-bit integer value


#define UIMSG_IS_INT(x) (((x)> 0x01ff) && ((x)< 0x0400))

#define UIM_LOOP_NONE 0
#define UIM_LOOP_FILE 1
#define UIM_LOOP_FOLDER 2

#define UIM_PLAYING_STOPPED 0
#define UIM_PLAYING_PAUSED 1
#define UIM_PLAYING_PLAYING 2

//Subcategory NUMERIC:LONG 32-bit values 0x0300-0x03FF
#define UIMSG_LONG 0x0300 //Unspecified signed 32-bit integer value
#define UIMSG_UNSIGNED_LONG 0x0301 //Unspecified unsigned 32-bit integer value
#define UIMSG_SAMPLE_RATE 0x0302 //Sample Rate for unspecified purpose, unit: millihertz.
#define UIMSG_DAC_SAMPLE_RATE 0x0303 //Sample Rate for DAC
#define UIMSG_ADC_SAMPLE_RATE 0x0304 //Sample Rate for ADC
#define UIMSG_COORDINATE 0x0305 //Generic coordinate, high word is Y, low word is X, left-top is 0,0.
#define UIMSG_POINTER_LOCATION 0x0306 //Location of a mouse pointer
#define UIMSG_CLICK_LOCATION 0x0307 //Location of a mouse click or touchscreen touch
#define UIMSG_SELECT_TRACK 0x0308 
#define UIMSG_LENGTH_OF_BINARY_DATA 0x0309 //Total length in bytes of a (multipart) binary data block which follows
#define UIMSG_U32_PLAY_TIME_SECONDS 0x030a //Play time in seconds
#define UIMSG_U32_PLAY_FILE_PERCENT 0x030b //Percentage of file played
#define UIMSG_U32_BITS_PER_SECOND   0x030c //Average bitrate
#define UIMSG_U32_FILE_POSITION     0x030d //Position in file in bytes
#define UIMSG_U32_FILE_SIZE         0x030e //File size in bytes
#define UIMSG_U32_TOTAL_PLAY_TIME_SECONDS 0x030f //Play time in seconds



//CATEGORY:TIME 0x0400-0x04FF
//Time is 32-bit signed value, which signifies milliseconds.
#define UIMSG_TIME 0x0400 //Unspecified time, signed 32-bit integer in milliseconds with positive values to the future.
#define UIMSG_TIME_SEEK 0x0401 //Set playing position of song in milliseconds

#define UIMSG_IS_TEXT(x) (((x)&0xff00) == UIMSG_TEXT)

//CATEGORY:TEXTUAL 0x0500-0x05FF
//Textual messages value is a __x char* to a string. 
//The string can be thrown away when uiCallbackFunction call returns.
#define UIMSG_TEXT 0x0500 //Unspecified text, pointer to zero-terminated 16-bit string
#define UIMSG_TEXT_OPEN_FILE 0x0501 //Instruction to open a file based on its name
#define UIMSG_TEXT_CLOSE_FILE 0x0502 //Instruction to close the current file. A closed file cannot be reopened without a new "open file" type operation.
#define UIMSG_TEXT_SONG_NAME 0x0503 //Name of song, null-terminated
#define UIMSG_TEXT_ALBUM_NAME 0x0504 //Name of album, null-terminated
#define UIMSG_TEXT_ARTIST_NAME 0x0505 //Name of artist, null-terminated
#define UIMSG_TEXT_YEAR 0x0506 // Year of performance, null-terminated
#define UIMSG_TEXT_LYRICS 0x0507 //Lyrics update
#define UIMSG_TEXT_NAME_OF_BINARY_DATA 0x0508 //Name of binary block which follows
#define UIMSG_TEXT_SHORT_FILE_NAME 0x050a // Short file name, if available
#define UIMSG_TEXT_LONG_FILE_NAME 0x050b // Long file name, if available.
/* Note: If both short and long file names are available, the short file
   name always comes first. If any other textual song metadata is available,
   like artist and song name, it always comes after file name information. */
#define UIMSG_TEXT_ERROR_MSG 0x050c // Error message, NULL = ok
#define UIMSG_TEXT_TRACK_NUMBER 0x050d //ID3 track number string

//CATEGORY:BINARY 0x0600-0x06FF
//Value's high word is __x pointer to a binary data block
//Value's low word is length of binary data block in bytes
#define UIMSG_BINARY_DATA 0x0600 //Piece of generic binary data

#endif


#include <vo_stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <sysmemory.h>

#include "ID3v2.h"

#define min(a,b) ((a>b)?(b):(a))

#define NUM_TAGS 6
#define MAX_TAG_SIZE 31

#pragma msg 30 off

// ENTRY_1
DLLENTRY(ReturnID3)
void ReturnID3(SongInfo **metadata);

void ReturnID3(SongInfo **metadata)
{
    //SongInfo metadata;
    
	const char *tid[7] = {"TIT2", "TALB", "TPE1", "TRCK", "TYER", "TLEN", "TIT3"};
	const char *tid2[7] = {"TT2", "TAL", "TP1", "TRK", "TYE", "TKE", "TT3"};

    unsigned char output_str[31];
	unsigned char tag_info[31];
	unsigned char common_header[10];
	u_int16 bytesRead = 0;
	u_int16 tagname_size;
	u_int16 i;
	u_int32 startingPosition;
	u_int16 tag_loop;
	u_int16 frame_size;
	u_int16 l_to_read;
	char start_utf[4] = {0xFF,0xFE};
	u_int16 frame_header_size;
    
	
    // Reset pointer within file to beginning of datastream.
    fseek((*metadata)->fp, 0, SEEK_SET);

    // Read the first 10 bytes of the file, which is known as the "common header" for ID3v2 metadata tags
    for (i = 0; i < sizeof(common_header); i++)
    {
        if ((common_header[i] = fgetc((*metadata)->fp)) != EOF)
        {
            bytesRead++;
        }
    }

    // If the header of the file begins with "ID3", we know it's ID3v2
    // Code only supports ID3v2.3 and ID3v2.4, so check 4th byte for version as well
    if ((common_header[0] == 'I') && (common_header[1] == 'D') && (common_header[2] == '3') && (common_header[3] <= 0x04)) 
    {
        // Tag size is last 4 bytes of header
        u_int16 tag_size = (common_header[9]) | (common_header[8] << 7) | (common_header[7] << 14) | (common_header[6] << 28);

        // The size of the data frames depends on the version variant. 0x03/0x04 = 10 bytes, 0x02 = 6 bytes.
        frame_header_size = (common_header[3] == 0x03 || common_header[3] == 0x04) ? 10 : 6;
        tagname_size      = (common_header[3] == 0x03 || common_header[3] == 0x04) ?  4 : 3;

        
        startingPosition = ftell((*metadata)->fp);

        for (tag_loop = 0; tag_loop < NUM_TAGS; tag_loop++)
        {
        	 const char *compare_tagname = ((common_header[3] == 0x03)||(common_header[3] == 0x04)) ? (tid[tag_loop]) : (tid2[tag_loop]);
            fseek((*metadata)->fp, startingPosition, SEEK_SET);
            // This is the tag name we're going to be looking for. Dependent on whether the common header shows the version as being
            // ID3v2_2 or ID3v2_1
    
            // Look for the proper tag ID until we get to the end of the metadata
            while (tag_size) 
            {
                unsigned char frame_header[10];
                u_int16 fr_hdr_bytes = 0;
                
                for (i = 0; i < frame_header_size; i++)
                {
                    if ((frame_header[i] = fgetc((*metadata)->fp)) != EOF)
                    {
                        fr_hdr_bytes++;
                    }
                }
                
                tag_size -= fr_hdr_bytes;
                
                // Check the name of each tag. If the name has a character that isn't in any possible tags,
                // we've somehow reached a part of the file past the metadata and so there's nothing else to look for
                // so we should just exit now instead of searching for metadata within the music
                for (i = 0 ; i < tagname_size; i++) 
                {
                    if ((frame_header[i]>'Z')||(frame_header[i]<'0')) 
                    {
                        // We want to update the appropriate metadata that the information is unknown
                        // and move on to the next metadata parameter to check
                        switch (tag_loop) 
                        {
                            case TITLE_ID3 :
                                strcpy((*metadata)->title, "UNKNOWN");
                                goto contLoop;
                            case ARTIST_ID3 :
                                strcpy((*metadata)->artist, "UNKNOWN");
                                goto contLoop;
                             case ALBUM_ID3 :
                                strcpy((*metadata)->album, "UNKNOWN");
                                goto contLoop;
                            default :
                                goto contLoop;
                        }
                    }
                }
                
                if ((common_header[3] == 0x03)||(common_header[3] == 0x04))
                    frame_size = (frame_header[7]) | (frame_header[6] << 8) | (frame_header[5] << 16) | ((frame_header[4] << 16) << 16);
                else //if (common_header[3] == 0x02)
                    frame_size = (frame_header[5]) | (frame_header[4] << 8) | (frame_header[3] << 16);

                // check to see if we've got the right tag as was asked for
                if (0 == strncmp( (char*) frame_header, compare_tagname, tagname_size))
                {
                    fseek((*metadata)->fp, 1, SEEK_CUR); // Skip past empty byte
                    
                    // UNICODE to ASCII conversion
                    l_to_read = min(frame_size,MAX_TAG_SIZE);

                    if (l_to_read >= 2)
                    {                        
                        for (i = 0; i < 2; i++)
                        {
                            output_str[i] = fgetc((*metadata)->fp);
                        }
                        
                        // unicode tags start with 01 FF FE
                        if (0 == strncmp(start_utf, output_str, 2))
                        {
                    
                            // Unicode decode string:
                            l_to_read = min((frame_size-3)/2,MAX_TAG_SIZE-1);

                            for (i = 0; i <= l_to_read; i++) 
                            {
                                output_str[i] = fgetc((*metadata)->fp);
                                fseek((*metadata)->fp, 1, SEEK_CUR);
                                if ((output_str[i] > '~' ) || (output_str[i] < ' ')) 
                                {
                                    output_str[i] = '_';
                                }
                            }
                            
                            // make sure there's a null terminator
                            output_str[l_to_read] = 0; 
                            
                            switch (tag_loop) 
                            {
                                case TITLE_ID3 :
                                    strcpy((*metadata)->title, output_str);
                                    goto contLoop;
                                case ARTIST_ID3 :
                                    strcpy((*metadata)->artist, output_str);
                                    goto contLoop;
                                 case ALBUM_ID3 :
                                    strcpy((*metadata)->album, output_str);
                                    goto contLoop;
                                default :
                                    goto contLoop;
                            }
                        }
                        
                        // not unicode, go back to the beginning.
                        for (i = 0; i <= l_to_read-2; i++)
                        {
                            output_str[i+2] = fgetc((*metadata)->fp);
                        }
                        
                        // make sure there's a null terminator
                        output_str[l_to_read-1] = 0;
                        
                        switch (tag_loop) 
                        {
                            case TITLE_ID3 :
                                strcpy((*metadata)->title, output_str);
                                goto contLoop;
                            case ARTIST_ID3 :
                                strcpy((*metadata)->artist, output_str);
                                goto contLoop;
                             case ALBUM_ID3 :
                                strcpy((*metadata)->album, output_str);
                                goto contLoop;
                            default :
                                goto contLoop;
                        }
                    }
                }
                else 
                {
                    fseek((*metadata)->fp, frame_size, SEEK_CUR);
                    tag_size -= frame_size;
                }
            } // END : while (tag_size) 
          
            // Only gets here if the entire tag size was checked and the metadata that we 
            // were searching for wasn't found
            // We want to update the appropriate metadata that the information is unknown
            switch (tag_loop) 
            {
                case TITLE_ID3 :
                    strncpy((*metadata)->title, "UNKNOWN", 8);
                    goto contLoop;
                case ARTIST_ID3 :
                    strncpy((*metadata)->artist, "UNKNOWN", 8);
                    goto contLoop;
                 case ALBUM_ID3 :
                    strncpy((*metadata)->album, "UNKNOWN", 8);
                    goto contLoop;
                default :
                    goto contLoop;
            }
            
            contLoop : continue;
        } // END : for (tag_loop = 0; tag_loop < NUM_TAGS; tag_loop++)        
    }
    // It wasn't v2, so it must be v1, which means its at the end of the file instead of the start
    else 
    {
        char raw[128];
        // Last 128 bytes are the tag, so point to -128
        fseek((*metadata)->fp, -128L, SEEK_END);
        // We know it's all data, so read it all in as raw data for processing
        fread(raw, 1, 128, (*metadata)->fp);
 
        // Double check that the first bytes imply it's a TAG, otherwise it's not and we can't do anything
        if (raw[0] != 'T' || raw[1] != 'A' || raw[2] != 'G') 
        {
            printf("Couldn't identify ID3v1 tag");
            return;
        } 
        else 
        {
                // pointer to raw data + 3 bytes since we know the first three bytes are "TAG" and want to skip them        			
                char *b = raw + 3;
                
                memcpy((*metadata)->title, b, 30);
                b += 30;
                memcpy((*metadata)->artist, b, 30);
                b += 30;
                memcpy((*metadata)->album, b, 30);
        }
    }
}

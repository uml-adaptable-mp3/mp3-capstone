#ifndef READ_ID3_H_
#define READ_ID3_H_

#define TITLE_ID3 0
#define ALBUM_ID3 1
#define ARTIST_ID3 2
#define TRACK_NUM_ID3 3
#define YEAR_ID3 4
#define LENGTH_ID3 5
#define SUBTITLE_ID3 6

typedef struct {
    FILE *fp;
	unsigned char title[31];
	unsigned char artist[31];
	unsigned char album[31];
	unsigned char year[5];
	unsigned char comment[31];
	unsigned char genre[2];
} SongInfo;

#endif
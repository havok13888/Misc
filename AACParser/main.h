#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#define ADTS_HEADER_SIZE 	7 //Size of header file
#define FRO_TRUNCATE_SIZE 	4 //number of bytes to be reduce from fro file

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

// struct to handle ADTS data
typedef struct
{
    BYTE ID : 1;
    BYTE channel_config : 2;
    WORD frequency_index : 4;
    WORD length : 13;
    BYTE profile : 2;
} AACFrameInfo;

DWORD read_ADTS_header(unsigned char* pBuffer, AACFrameInfo& frameInfo);
int check_count_frames();
int change_file_size();
int get_file_size(FILE* file);

FILE *aac_file; 		//file handler for aac file
FILE *fro_file;			//file handler for fro file
int  w_file = -1;
char *aac_file_name;
char *fro_file_name;
long aac_file_size;
long fro_file_size;

// Sampling rates
static DWORD sample_rates[] = {96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000};

#endif // MAIN_H_INCLUDED

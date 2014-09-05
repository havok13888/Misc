#include <iostream>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include "main.h"

using namespace std;

int main(int argc, char* argv[])
{

    if ( argc != 2 )
        cout<<"usage: "<< argv[0] <<" <input file>\n";
    else
    {
        //argv[1] = inputfile
        aac_file_name = argv[1];

        //check and fix frames if necessary
        check_count_frames();
    }
    return 0;
}

int check_count_frames()
{
    unsigned char rBuf[ADTS_HEADER_SIZE]; //buffer to hold incoming stream

    long frame_count = 0;
    size_t bytes_read;
    AACFrameInfo info;
    bool loop = true;

    string temp_str;		//temp string used for conversion from string to char*
    string fro_str ="fro";	// use to replace .aac with .fro at the end of the file name


    temp_str = aac_file_name;
    //conversion of filename.aac to filename.fro
    temp_str = temp_str.replace((strlen(aac_file_name)-3),3, fro_str);

    //creating char* with new filename.fro data
    fro_file_name = new char[temp_str.length() + 1];
    strcpy(fro_file_name, temp_str.c_str());

    //open .aac file for reading
    aac_file = fopen(aac_file_name,"rb");
    if (aac_file == NULL)
    { fprintf(stderr, "Unable to open aac file\n"); return -1;} //exit if open file fails

    //open .fro file for reading, needed to get file size
    fro_file = fopen(fro_file_name,"rb");
    if (fro_file == NULL)
    { fprintf(stderr, "Unable to open fro file\n"); return -1;} //exit if open file fails


    while(loop == true)
    {
        bytes_read = fread(rBuf, 1, ADTS_HEADER_SIZE, aac_file);

        //If number of bytes read is lesser than a header size
        //we may have reached the end of the file or encountered an error.
        if(bytes_read != ADTS_HEADER_SIZE)
        {
            if(feof(aac_file))
            {
                cout<<"End of File\n"<<"FrameCount: "<<frame_count<<endl;;
                loop = false;
                aac_file_size = get_file_size(aac_file);
                fro_file_size = get_file_size(fro_file);

            	cout<<"AAC SIZE "<<aac_file_size<<endl;
            	cout<<"FRO SIZE "<<fro_file_size<<endl;

            	aac_file_size -= info.length;
                fro_file_size -= FRO_TRUNCATE_SIZE ;

            }
            else
            {
                fprintf(stderr, "I/O Error.\n");
                return -1;
            }
        }
        else
        {

        	int frame_len = read_ADTS_header(rBuf, info);
            if(frame_len == ADTS_HEADER_SIZE)
            {
                //standard 7 byte ADTS frameheader without CRC protection
                //for now just continue
            }

            // on first read verify that the file is indeed 44kHz
            if(frame_count == 0 && sample_rates[info.frequency_index] != 44100)
            {
                fprintf(stderr, "Not a 44100 kHz AAC file.\n");
                return -1;
            }

            // Frame size has to  be a minimum of the size of the ADTS header
            if(info.length< ADTS_HEADER_SIZE)
            {
                fprintf(stderr, "Bad frame.\n");
                return -1;
            }

            if(info.length == ADTS_HEADER_SIZE)
            {
                //we don't want to handle empty frames yet
                //so just continue
            }

            // after checking everything increment frame count.
            frame_count++;

            // seek rest of frame
            // info.length is frame size which is (header size + frame size)
            fseek(aac_file, info.length - ADTS_HEADER_SIZE, SEEK_CUR);
        }
    }
    if(frame_count%2 != 0)
    {
        cout<<"Fixing Size."<<endl;
        if(change_file_size() == 0)
        {
        	cout<<".aac & .fro file sizes fixed."<<endl;
        }
        else
        {
        	fprintf(stderr, "fix failed.");
        	return -1;
        }
    }
    else
    {
        // even frames just continue
        cout<<"No fixes needed."<<endl;
    }
    // clean up
    fclose(aac_file);
    fclose(fro_file);
    delete [] aac_file_name;
    delete [] fro_file_name;
    return 0;
}

// used for cutting frames and reducing the file size
int change_file_size()
{
	//cut out the last frame from the aac file
	if( (w_file = _open( aac_file_name, O_WRONLY, _S_IWRITE ))  != -1 )
	{
		if( ( (_chsize( w_file, aac_file_size ) ) != 0) )
		{
			fprintf(stderr, "AAC Size could not be changed.");
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "AAC could not be opened for size change.");
		return - 1;
	}

    _close( w_file );

    //cut out the last 4 bytes from the fro file
	if( (w_file = _open( fro_file_name, O_WRONLY, _S_IWRITE ))  != -1 )
	{
		if( ( (_chsize( w_file, fro_file_size ) ) != 0) )
		{
			fprintf(stderr, "FRO Size could not be changed.");
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "FRO could not be opened for size change.");
		return - 1;
	}


	cout << "Post Fix AAC Size "<< get_file_size(aac_file) << endl;
	cout << "Post Fix FRO Size "<< get_file_size(fro_file) << endl;


    _close( w_file );

    return 0;
}

int get_file_size(FILE* file)
{
	int size = 0;
	fseek(file, 0L, SEEK_SET);	//set the pointer to the beggining of the file as it may already be open
	fseek(file, 0L, SEEK_END);	//set the pointer to the end of the file
	size = ftell(file);			//get size of file
	fseek(file, 0L, SEEK_SET);	//set pointer back to start of the file
	return size;
}

DWORD read_ADTS_header(unsigned char* pBuffer, AACFrameInfo& frameInfo)
{
    /* check syncword */
    if (!((pBuffer[0] == 0xFF)&&((pBuffer[1] & 0xF0) == 0xF0)))
    return -1;

    // parse ADTS header info. 7 or 9 bytes
    // ADTS header info - http://wiki.multimedia.cx/index.php?title=ADTS
    frameInfo.ID = (pBuffer[1] >> 3) & 0x01;
    BYTE layer = (pBuffer[1] >> 1) & 0x03;
    BYTE protection_absent = pBuffer[1] & 0x01;
    frameInfo.profile = (pBuffer[2] >> 6) & 0x03;
    frameInfo.frequency_index = (pBuffer[2] >> 2) & 0xF;
    BYTE private_bit = (pBuffer[2] >> 1) & 0x01;
    frameInfo.channel_config = ((pBuffer[2] & 0x01) << 2) + ((pBuffer[3] >> 6) & 0x03);
    BYTE original_copy = (pBuffer[3] >> 5) & 0x01;
    BYTE home = (pBuffer[3] >> 4) & 0x01;
    BYTE copyright_identification_bit = (pBuffer[3] >> 3) & 0x01;
    BYTE copyright_identification_start = (pBuffer[3] >> 2) & 0x01;
    frameInfo.length = ((((unsigned int)pBuffer[3] & 0x3)) << 11)
    | (((unsigned int)pBuffer[4]) << 3) | (pBuffer[5] >> 5);
    WORD adts_buffer_fullness = (((WORD)pBuffer[5] & 0x1F) << 6) | (WORD)((pBuffer[6] >> 2) & 0x3F);
    BYTE no_raw_data_blocks_in_frame = pBuffer[6] & 0x3;
    BYTE frame_len = 7;
    if(protection_absent == 0)
    frame_len += 2;
    return frame_len;

}

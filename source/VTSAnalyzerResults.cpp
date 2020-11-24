#include "VTSAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "VTSAnalyzer.h"
#include "VTSAnalyzerSettings.h"
#include <iostream>
#include <fstream>

VTSAnalyzerResults::VTSAnalyzerResults( VTSAnalyzer* analyzer, VTSAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

VTSAnalyzerResults::~VTSAnalyzerResults()
{
}

void VTSAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
	Frame frame = GetFrame( frame_index );

	char number_str[128];

	if (frame.mData2 == DATA2_TYPE_MOSI_COMMAND && channel == mSettings->mMosiChannel)
	{
		U8 addr, cmd;
		addr = frame.mData1 >> 3;
		cmd = frame.mData1 & 0x3;
		snprintf(number_str, 128, "ADDR: 0x%02x CMD: 0x%x", addr, cmd);
		AddResultString( number_str );
	} else if ((frame.mData2 == DATA2_TYPE_MOSI_DATA && channel == mSettings->mMosiChannel) ||
				(frame.mData2 == DATA2_TYPE_MISO_DATA && channel == mSettings->mMisoChannel))
	{
		char *fmt = "0x%02x";
		if(frame.mFlags == (FLAG_START | FLAG_END))
		{
			fmt = "[0x%02x]";
		} else if(frame.mFlags == FLAG_START)
		{
			fmt = "[0x%02x";
		} else if(frame.mFlags == FLAG_END)
		{
			fmt = "0x%02x]";
		}
		snprintf(number_str, 128, fmt, frame.mData1);
		AddResultString( number_str );
	} else if (frame.mData2 == DATA2_TYPE_ERROR_NO_ACK)
	{
		snprintf(number_str, 128, "ERROR: NO ACK TO %02x", frame.mData1 >> 3);
		AddResultString(number_str);
	}
}

void VTSAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	file_stream << "Time [s],Type,Value" << std::endl;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
		
		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		char *type_str = "ERROR";
		if(frame.mData2 == DATA2_TYPE_MOSI_DATA){
			type_str = "MOSI";
		} else if(frame.mData2 == DATA2_TYPE_MOSI_COMMAND){
			type_str = "MOSI_COMMAND";
		} else if(frame.mData2 == DATA2_TYPE_MISO_DATA){
			type_str = "MISO";
		} else if(frame.mData2 == DATA2_TYPE_ERROR_NO_ACK){
			type_str = "ERROR_NO_ACK";
		}

		char number_str[128];
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );

		file_stream << time_str << "," << type_str << "," << number_str << std::endl;

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

	file_stream.close();
}

void VTSAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
#ifdef SUPPORTS_PROTOCOL_SEARCH
	Frame frame = GetFrame( frame_index );
	ClearTabularText();
	char number_str[128];

	if (frame.mData2 == DATA2_TYPE_MOSI_COMMAND)
	{
		U8 addr, cmd;
		addr = frame.mData1 >> 3;
		cmd = frame.mData2 & 0x3;
		snprintf(number_str, 128, "ADDR: 0x%02x CMD: 0x%x", addr, cmd);
		AddTabularText( number_str );
	} else if ((frame.mData2 == DATA2_TYPE_MOSI_DATA) || (frame.mData2 == DATA2_TYPE_MISO_DATA))
	{
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
		AddTabularText( number_str );
	} else if (frame.mData2 == DATA2_TYPE_ERROR_NO_ACK)
	{
		snprintf(number_str, 128, "ERROR: NO ACK TO 0x%02x", frame.mData1 >> 3);
		AddTabularText(number_str);
	}
#endif
}

void VTSAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	//not supported

}

void VTSAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	//not supported
}
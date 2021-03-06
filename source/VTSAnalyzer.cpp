#include "VTSAnalyzer.h"
#include "VTSAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

VTSAnalyzer::VTSAnalyzer()
:	Analyzer2(),
	mSettings( new VTSAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

VTSAnalyzer::~VTSAnalyzer()
{
	KillThread();
}

void VTSAnalyzer::SetupResults()
{
	mResults.reset( new VTSAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	// not sure if we can concatenate everything in one results object
	if(mSettings->mMisoChannel != UNDEFINED_CHANNEL)
	{
		mResults->AddChannelBubblesWillAppearOn( mSettings->mMisoChannel );
	}
	if(mSettings->mMosiChannel != UNDEFINED_CHANNEL)
	{
		mResults->AddChannelBubblesWillAppearOn( mSettings->mMosiChannel );
	}
	//if(mSettings->mSyncChannel != UNDEFINED_CHANNEL){
	//	mResults->AddChannelBubblesWillAppearOn( mSettings->mSyncChannel );
	//}
}

void VTSAnalyzer::WorkerThread()
{
	enum {
		PHASE_COMMAND,
		PHASE_COMPLEMENT,
		PHASE_ACK,
		PHASE_MISO_DATA,
		PHASE_MOSI_DATA,
	} phase = PHASE_COMMAND;
	mSampleRateHz = GetSampleRate();

	mMosiSerial = GetAnalyzerChannelData( mSettings->mMosiChannel );
	mMisoSerial = GetAnalyzerChannelData( mSettings->mMisoChannel );
	mSync = GetAnalyzerChannelData( mSettings->mSyncChannel );

	U32 samples_per_bit = mSampleRateHz / mSettings->mBitRate;
	U32 samples_to_first_center_of_first_data_bit = U32( 1.5 * double( mSampleRateHz ) / double( mSettings->mBitRate ) );

	U64 frame_start_sample;
	U64 frame_end_sample;

	for( ; ; )
	{
		mSync->AdvanceToNextEdge();
		mResults->AddMarker( mSync->GetSampleNumber(), AnalyzerResults::DownArrow, mSettings->mSyncChannel );
		
		SyncSerials();
		mSync->AdvanceToNextEdge();
		mResults->AddMarker( mSync->GetSampleNumber(), AnalyzerResults::UpArrow, mSettings->mSyncChannel );
		phase = PHASE_COMMAND;
		bool first_data = true;

		U8 cmd;
		Frame frame;
		while(NextChannelEdge() != mSync)
		{
			U64 starting_sample;
			U8 data;
			if(phase == PHASE_COMMAND){
				frame.mStartingSampleInclusive = mMosiSerial->GetSampleNumber();
				data = ReadByte(mMosiSerial, mSettings->mMosiChannel, samples_per_bit, samples_to_first_center_of_first_data_bit, &starting_sample);
				cmd = data;
				phase = PHASE_COMPLEMENT;
				continue;
			} else if(phase == PHASE_COMPLEMENT){
				data = ReadByte(mMosiSerial, mSettings->mMosiChannel,samples_per_bit, samples_to_first_center_of_first_data_bit, &starting_sample);
				frame.mEndingSampleInclusive = mMosiSerial->GetSampleNumber();
				// TODO: check complement
				frame.mData1 = cmd;
				frame.mData2 = DATA2_TYPE_MOSI_COMMAND;
				frame.mFlags = 0;
				AddFrame( frame );
				AnalyzerChannelData* next_edge = NextChannelEdge();
				phase = PHASE_MISO_DATA;
				if(next_edge == mSync){
					frame.mData1 = cmd;
					frame.mData2 = DATA2_TYPE_ERROR_NO_ACK;
					frame.mFlags = 0;
					frame.mStartingSampleInclusive = frame.mEndingSampleInclusive;
					frame.mEndingSampleInclusive = mSync->GetSampleOfNextEdge();
					AddFrame( frame );
				}
				continue;
			} else if(phase == PHASE_MISO_DATA) {
				data = ReadByte(mMisoSerial, mSettings->mMisoChannel,samples_per_bit, samples_to_first_center_of_first_data_bit, &starting_sample);
				frame.mStartingSampleInclusive = starting_sample;
				frame.mEndingSampleInclusive = mMisoSerial->GetSampleNumber();
				frame.mData1 = data;
				frame.mData2 = DATA2_TYPE_MISO_DATA;
				frame.mFlags = first_data ? FLAG_START : 0;
				first_data = false;
				AnalyzerChannelData* next_edge = NextChannelEdge();
				if(next_edge == mSync){
					frame.mFlags |= FLAG_END;
					phase = PHASE_COMMAND;
					first_data = true;
				} else if(next_edge == mMosiSerial){
					frame.mFlags |= FLAG_END;
					phase = PHASE_MOSI_DATA;
					first_data = true;
				}
				
				AddFrame( frame );
			} else if(phase == PHASE_MOSI_DATA) {
				data = ReadByte(mMosiSerial,mSettings->mMosiChannel, samples_per_bit, samples_to_first_center_of_first_data_bit, &starting_sample);
				frame.mStartingSampleInclusive = starting_sample;
				frame.mEndingSampleInclusive = mMosiSerial->GetSampleNumber();
				frame.mData1 = data;
				frame.mData2 = DATA2_TYPE_MOSI_DATA;
				frame.mFlags = first_data ? FLAG_START : 0;
				first_data = false;
				AnalyzerChannelData* next_edge = NextChannelEdge();
				if(next_edge == mSync){
					frame.mFlags |= FLAG_END;
					phase = PHASE_COMMAND;
					first_data = true;
				} else if(next_edge == mMisoSerial){
					frame.mFlags |= FLAG_END;
					phase = PHASE_MISO_DATA;
					first_data = true;
				}
				AddFrame( frame );
			}

		}



	}
}

void VTSAnalyzer::AddFrame(const Frame &f)
{
	mResults->AddFrame( f );
	mResults->CommitResults();
	ReportProgress( f.mEndingSampleInclusive );
}

void VTSAnalyzer::SyncSerials()
{
	U64 sync_sample_number = mSync->GetSampleNumber();
	mMosiSerial->AdvanceToAbsPosition(sync_sample_number);
	mMisoSerial->AdvanceToAbsPosition(sync_sample_number);
}

#define MIN2(a,b) (((a) < (b)) ? (a):(b))
#define MIN3(a,b,c) (((a) < (b)) ? (MIN2((a),(c))):(MIN2((b),(c))))
AnalyzerChannelData* VTSAnalyzer::NextChannelEdge()
{
	// NOTE: this assumes that SYNC, MOSI and MISO don't "edge" together
	U64 next_miso_edge = mMisoSerial->GetSampleOfNextEdge();
	U64 next_mosi_edge = mMosiSerial->GetSampleOfNextEdge();
	U64 next_sync_edge = mSync->GetSampleOfNextEdge();
	U64 first_edge = MIN3(next_miso_edge, next_mosi_edge, next_sync_edge);
	if(first_edge == next_sync_edge){
		return mSync;
	}else if(first_edge == next_miso_edge){
		return mMisoSerial;
	} else {
		return mMosiSerial;
	}
}

U8 VTSAnalyzer::ReadByte(AnalyzerChannelData *serial, Channel& channel, U32 samples_per_bit, U32 samples_to_first_center_of_first_data_bit, U64 *starting_sample)
{
	U8 data = 0;
	U8 mask = 1;

	serial->AdvanceToNextEdge();

	mResults->AddMarker( serial->GetSampleNumber(), AnalyzerResults::Start, channel);
	if(starting_sample != NULL)
	{
		*starting_sample = serial->GetSampleNumber();
	}

	serial->Advance( samples_to_first_center_of_first_data_bit );

	for( U32 i=0; i<8; i++ )
	{
		//let's put a dot exactly where we sample this bit:
		mResults->AddMarker( serial->GetSampleNumber(), AnalyzerResults::Dot, channel );

		if( serial->GetBitState() == BIT_HIGH )
			data |= mask;

		serial->Advance( samples_per_bit );

		mask = mask << 1;
	}
	mResults->AddMarker( serial->GetSampleNumber(), AnalyzerResults::Stop, channel );
	serial->Advance( samples_per_bit );

	return data;
}

bool VTSAnalyzer::NeedsRerun()
{
	return false;
}

U32 VTSAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 VTSAnalyzer::GetMinimumSampleRateHz()
{
	return mSettings->mBitRate * 4;
}

const char* VTSAnalyzer::GetAnalyzerName() const
{
	return "VTS";
}

const char* GetAnalyzerName()
{
	return "VTS";
}

Analyzer* CreateAnalyzer()
{
	return new VTSAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}
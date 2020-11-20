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
	mResults->AddChannelBubblesWillAppearOn( mSettings->mMosiChannel );
}

void VTSAnalyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();

	mMosiSerial = GetAnalyzerChannelData( mSettings->mMosiChannel );

	if( mMosiSerial->GetBitState() == BIT_LOW )
		mMosiSerial->AdvanceToNextEdge();

	U32 samples_per_bit = mSampleRateHz / mSettings->mBitRate;
	U32 samples_to_first_center_of_first_data_bit = U32( 1.5 * double( mSampleRateHz ) / double( mSettings->mBitRate ) );

	for( ; ; )
	{
		mMosiSerial->AdvanceToNextEdge(); //falling edge -- beginning of the start bit
		U64 starting_sample = mMosiSerial->GetSampleNumber();
		U8 data = ReadSerial(mMosiSerial, samples_per_bit, samples_to_first_center_of_first_data_bit);


		//we have a byte to save. 
		Frame frame;
		frame.mData1 = data;
		frame.mFlags = 0;
		frame.mStartingSampleInclusive = starting_sample;
		frame.mEndingSampleInclusive = mMosiSerial->GetSampleNumber();

		mResults->AddFrame( frame );
		mResults->CommitResults();
		ReportProgress( frame.mEndingSampleInclusive );
	}
}

U8 VTSAnalyzer::ReadSerial(AnalyzerChannelData *serial, U32 samples_per_bit, U32 samples_to_first_center_of_first_data_bit)
{
	U8 data = 0;
	U8 mask = 1;
	


	serial->Advance( samples_to_first_center_of_first_data_bit );

	for( U32 i=0; i<8; i++ )
	{
		//let's put a dot exactly where we sample this bit:
		mResults->AddMarker( serial->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mMosiChannel );

		if( serial->GetBitState() == BIT_HIGH )
			data |= mask;

		serial->Advance( samples_per_bit );

		mask = mask << 1;
	}
	mResults->AddMarker( serial->GetSampleNumber(), AnalyzerResults::Square, mSettings->mMosiChannel );
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
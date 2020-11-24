#ifndef VTS_ANALYZER_H
#define VTS_ANALYZER_H

#include <Analyzer.h>
#include "VTSAnalyzerResults.h"
#include "VTSSimulationDataGenerator.h"
#include <iostream>
#include <fstream>

//#define DATA2_TYPE_SYNC 0
#define DATA2_TYPE_MOSI_COMMAND 1
#define DATA2_TYPE_MOSI_DATA 2
#define DATA2_TYPE_MISO_DATA 3
#define DATA2_TYPE_ERROR_NO_ACK 4

#define FLAG_START	(1)
#define FLAG_END 	(1 << 1 )


class VTSAnalyzerSettings;
class ANALYZER_EXPORT VTSAnalyzer : public Analyzer2
{
public:
	VTSAnalyzer();
	virtual ~VTSAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

private:
	virtual U8 ReadByte(AnalyzerChannelData *serial, Channel& channel, U32 samples_per_bit, U32 samples_to_first_center_of_first_data_bit, U64 *starting_sample);
	virtual void SyncSerials();
	virtual AnalyzerChannelData* NextChannelEdge();
	virtual void AddFrame(const Frame &f);

protected: //vars
	std::auto_ptr< VTSAnalyzerSettings > mSettings;
	std::auto_ptr< VTSAnalyzerResults > mResults;
	AnalyzerChannelData* mMosiSerial;
	AnalyzerChannelData* mMisoSerial;
	AnalyzerChannelData* mSync;

	VTSSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	std::vector<U32> mSampleOffsets;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
	BitState mBitLow;
	BitState mBitHigh;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //VTS_ANALYZER_H

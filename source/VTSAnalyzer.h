#ifndef VTS_ANALYZER_H
#define VTS_ANALYZER_H

#include <Analyzer.h>
#include "VTSAnalyzerResults.h"
#include "VTSSimulationDataGenerator.h"

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

protected: //vars
	std::auto_ptr< VTSAnalyzerSettings > mSettings;
	std::auto_ptr< VTSAnalyzerResults > mResults;
	AnalyzerChannelData* mSerial;

	VTSSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //VTS_ANALYZER_H

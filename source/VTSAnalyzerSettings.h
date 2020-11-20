#ifndef VTS_ANALYZER_SETTINGS
#define VTS_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class VTSAnalyzerSettings : public AnalyzerSettings
{
public:
	VTSAnalyzerSettings();
	virtual ~VTSAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mMosiChannel;
	Channel mMisoChannel;
	Channel mSyncChannel;
	U32 mBitRate;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputMOSIChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputMISOChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputSYNCChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
};

#endif //VTS_ANALYZER_SETTINGS

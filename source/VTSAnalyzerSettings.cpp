#include "VTSAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


VTSAnalyzerSettings::VTSAnalyzerSettings()
:	mMosiChannel( UNDEFINED_CHANNEL ),
	mMisoChannel( UNDEFINED_CHANNEL ),
	mSyncChannel( UNDEFINED_CHANNEL ),
	mBitRate( 4800 )
{
	mInputSYNCChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputSYNCChannelInterface->SetTitleAndTooltip( "VTS-SYNC", "Standard VTS SYNC" );
	mInputSYNCChannelInterface->SetChannel( mSyncChannel );
	mInputMOSIChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputMOSIChannelInterface->SetTitleAndTooltip( "VTS-MOSI", "Standard VTS MOSI" );
	mInputMOSIChannelInterface->SetChannel( mMosiChannel );
	mInputMISOChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputMISOChannelInterface->SetTitleAndTooltip( "VTS-MISO", "Standard VTS MISO" );
	mInputMISOChannelInterface->SetChannel( mMisoChannel );

	mBitRateInterface.reset( new AnalyzerSettingInterfaceInteger() );
	mBitRateInterface->SetTitleAndTooltip( "Bit Rate (Bits/S)",  "Specify the bit rate in bits per second." );
	mBitRateInterface->SetMax( 6000000 );
	mBitRateInterface->SetMin( 1 );
	mBitRateInterface->SetInteger( mBitRate );

	AddInterface( mInputMOSIChannelInterface.get() );
	AddInterface( mInputMISOChannelInterface.get() );
	AddInterface( mInputSYNCChannelInterface.get() );
	// Commented out because we're always 4800 baud anyway
	// AddInterface( mBitRateInterface.get() );

	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mMosiChannel, "VTS-MOSI", false );
	AddChannel( mMisoChannel, "VTS-MISO", false );
	AddChannel( mSyncChannel, "VTS-SYNC", false );
}

VTSAnalyzerSettings::~VTSAnalyzerSettings()
{
}

bool VTSAnalyzerSettings::SetSettingsFromInterfaces()
{
	mMosiChannel = mInputMOSIChannelInterface->GetChannel();
	mMisoChannel = mInputMISOChannelInterface->GetChannel();
	mSyncChannel = mInputSYNCChannelInterface->GetChannel();
	mBitRate = mBitRateInterface->GetInteger();

	ClearChannels();
	AddChannel( mMosiChannel, "VTS-MOSI", true );
	AddChannel( mMisoChannel, "VTS-MISO", true );
	AddChannel( mSyncChannel, "VTS-SYNC", true );

	return true;
}

void VTSAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputMOSIChannelInterface->SetChannel( mMosiChannel );
	mInputMISOChannelInterface->SetChannel( mMisoChannel );
	mInputSYNCChannelInterface->SetChannel( mSyncChannel );
	mBitRateInterface->SetInteger( mBitRate );
}

void VTSAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mMosiChannel;
	text_archive >> mMisoChannel;
	text_archive >> mSyncChannel;

	ClearChannels();
	AddChannel( mMosiChannel, "VTS-MOSI", true );
	AddChannel( mMisoChannel, "VTS-MISO", true );
	AddChannel( mSyncChannel, "VTS-SYNC", true );

	UpdateInterfacesFromSettings();
}

const char* VTSAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mMosiChannel;
	text_archive << mMisoChannel;
	text_archive << mSyncChannel;

	return SetReturnString( text_archive.GetString() );
}

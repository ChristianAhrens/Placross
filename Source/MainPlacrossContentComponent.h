/*
  ==============================================================================

    MainPlacrossContentComponent.h
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "AudioPlayer/AudioPlayerComponent.h"
#include "Routing/RoutingComponent.h"
#include "ChannelStrip/ChannelStripComponent.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainPlacrossContentComponent   : public AudioAppComponent
{
public:
    MainPlacrossContentComponent();
    ~MainPlacrossContentComponent() override;

    //==========================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    //==========================================================================
    void resized() override;

private:
    //==========================================================================
    std::unique_ptr<AudioPlayerComponent>                   m_playerComponent;
    std::unique_ptr<RoutingComponent>                       m_routingComponent;
    std::map<int, std::unique_ptr<ChannelStripComponent>>   m_stripComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainPlacrossContentComponent)
};

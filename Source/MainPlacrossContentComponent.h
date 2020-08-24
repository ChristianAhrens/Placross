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
#include "Analyser/AnalyserComponent.h"

#include "../submodules/JUCE-AppBasics/Source/OverlayToggleComponentBase.h"

class CircleComponent;

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainPlacrossContentComponent   :  public AudioAppComponent,
                                        public AudioPlayerComponent::Listener,
                                        public JUCEAppBasics::OverlayToggleComponentBase::OverlayParent
{
public:
    MainPlacrossContentComponent();
    ~MainPlacrossContentComponent() override;

    void setChannelSetup(int numInputChannels, int numOutputChannels, const XmlElement* const storedSettings = nullptr);

    std::pair<int, int> getCurrentDeviceChannelCount();

    //==========================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    //==========================================================================
    void paint(Graphics&) override;
    void resized() override;

    //==========================================================================
    void onNewAudiofileLoaded() override;

private:
    //==========================================================================
    std::unique_ptr<AudioPlayerComponent>                   m_playerComponent;
    std::vector<std::unique_ptr<CircleComponent>>           m_playerConCircles;
    std::unique_ptr<RoutingComponent>                       m_routingComponent;
    std::vector<std::unique_ptr<CircleComponent>>           m_routingConCircles;
    std::map<int, std::unique_ptr<ChannelStripComponent>>   m_stripComponents;
    std::vector<std::unique_ptr<CircleComponent>>           m_stripConCircles;
    std::unique_ptr<AnalyserComponent>                      m_analyserComponent;

    std::vector<Colour> m_channelColours;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainPlacrossContentComponent)
};

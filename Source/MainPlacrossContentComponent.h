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

#include "../submodules/JUCE-AppBasics/Source/OverlayToggleComponentBase.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainPlacrossContentComponent   :  public AudioAppComponent,
                                        public AudioPlayerComponent::Listener,
                                        public OverlayEditorComponentBase::OverlayParent,
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

    //==========================================================================
    void setOverlayEditor(OverlayEditorComponentBase* editor) override;
    bool isEditorActive() override;

private:
    //==========================================================================
    std::unique_ptr<AudioPlayerComponent>                   m_playerComponent;
    std::unique_ptr<RoutingComponent>                       m_routingComponent;
    std::map<int, std::unique_ptr<ChannelStripComponent>>   m_stripComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainPlacrossContentComponent)
};

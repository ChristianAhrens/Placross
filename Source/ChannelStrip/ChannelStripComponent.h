/*
  ==============================================================================

    ChannelStripComponent.h
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "ChannelStripProcessorPlayer.h"
#include "ChannelStripProcessor.h"

#include "../submodules/JUCE-AppBasics/Source/OverlayToggleComponentBase.h"

//==============================================================================
class ChannelStripComponent  :  public JUCEAppBasics::OverlayToggleComponentBase,
                                public AudioIODeviceCallback
{
public:
    //==============================================================================
    using AudioGraphIOProcessor = AudioProcessorGraph::AudioGraphIOProcessor;
    using Node = AudioProcessorGraph::Node;

    //==============================================================================
    ChannelStripComponent();
    ~ChannelStripComponent() override;

    //==============================================================================
    void resized() override;

    //==============================================================================
    void audioDeviceIOCallback(const float** inputChannelData,
        int numInputChannels,
        float** outputChannelData,
        int numOutputChannels,
        int numSamples) override;
    void audioDeviceAboutToStart(AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String &errorMessage) override;

private:
    //==============================================================================
    void initialiseGraph();

    void createAudioNodes();
    void connectAudioNodes();
    void destroyAudioNodes();

    //==============================================================================
    std::unique_ptr<AudioProcessorGraph>                m_mainProcessor;

    ReferenceCountedArray<Node>                         m_processorNodes;

    Node::Ptr                                           m_audioInputNode;
    Node::Ptr                                           m_audioOutputNode;

    ChannelStripProcessorPlayer                         m_player;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelStripComponent)
};

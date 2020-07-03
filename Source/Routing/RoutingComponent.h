/*
  ==============================================================================

    RoutingComponent.h
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "RoutingEditorComponent.h"

//==============================================================================
class RoutingComponent  :   public Component,
                            public DrawableButton::Listener,
                            public AudioIODeviceCallback,
                            public RoutingEditorComponent::RoutingListener,
                            public OverlayEditorComponentBase::OverlayListener
{
public:
    //==============================================================================
    RoutingComponent();
    ~RoutingComponent() override;

    void setIOCount(int inputChannelCount, int outputChannelCount);

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

    //==============================================================================
    void buttonClicked(Button* button) override;

    //==============================================================================
    void audioDeviceIOCallback(const float** inputChannelData,
        int numInputChannels,
        float** outputChannelData,
        int numOutputChannels,
        int numSamples) override;
    void audioDeviceAboutToStart(AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String &errorMessage) override;

    //==============================================================================
    void onRoutingEditingFinished(std::multimap<int, int> const& newRouting) override;
    void toggleEditor() override;

private:
    //==============================================================================
    void initialiseRouting();

    //==============================================================================
    std::unique_ptr<DrawableButton> m_sumButton;

    //==============================================================================
    std::unique_ptr<RoutingEditorComponent> m_editor;

    //==============================================================================
    int m_inputChannelCount{ 0 };
    int m_outputChannelCount{ 0 };

    std::multimap<int, int> m_routingMap{};
    CriticalSection         m_routingLock{};
    AudioSampleBuffer       m_routingOutputBuffer{};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoutingComponent)
};

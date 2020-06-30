/*
  ==============================================================================

    RoutingComponent.h
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class RoutingComponent  :   public Component,
                            public DrawableButton::Listener,
                            public AudioIODeviceCallback
{
public:
    //==============================================================================
    RoutingComponent();
    ~RoutingComponent() override;

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

private:
    //==============================================================================
    void initialiseRouting();

    //==============================================================================
    std::unique_ptr<DrawableButton> m_sumButton;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoutingComponent)
};

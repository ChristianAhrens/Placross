/*
  ==============================================================================

    RoutingComponent.h
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "../submodules/JUCE-AppBasics/Source/OverlayToggleComponentBase.h"

//==============================================================================
class RoutingComponent  :   public JUCEAppBasics::OverlayToggleComponentBase,
                            public AudioIODeviceCallback,
                            public DrawableButton::Listener
{
public:
    //==============================================================================
    RoutingComponent();
    ~RoutingComponent() override;

    void setIOCount(int inputChannelCount, int outputChannelCount);

    //==============================================================================
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


protected:
    void changeOverlayState() override;

private:
    //==============================================================================
    void initialiseRouting();
    void clearRouting();
    void setRouting(std::multimap<int, int> const& routingMap);

    //==============================================================================
    void onRoutingEditingFinished(std::multimap<int, int> const& newRouting);

    void toggleMinimizedMaximizedElementVisibility(bool maximized);

    //==============================================================================
    std::unique_ptr<DrawableButton> m_sumButton;

    //==============================================================================
    std::vector<std::unique_ptr<Label>>                          m_inputLabels;
    std::vector<std::unique_ptr<Label>>                          m_outputLabels;
    std::vector<std::vector<std::unique_ptr<DrawableButton>>>    m_nodeButtons;

    //==============================================================================
    int                     m_inputChannelCount{ 0 };
    int                     m_outputChannelCount{ 0 };
    std::multimap<int, int> m_routingMap{};
    CriticalSection         m_routingLock{};
    AudioSampleBuffer       m_routingOutputBuffer{};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoutingComponent)
};

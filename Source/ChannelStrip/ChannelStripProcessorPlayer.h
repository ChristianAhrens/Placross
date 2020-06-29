/*
  ==============================================================================

    ChannelStripProcessorPlayer.h
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class ChannelStripProcessorPlayer : public AudioProcessorPlayer
{
public:
    //==============================================================================
    ChannelStripProcessorPlayer();
    ~ChannelStripProcessorPlayer() override;

    void audioDeviceIOCallback(const float**, int, float**, int, int) override;
    void audioDeviceAboutToStart(AudioIODevice*) override;
    void audioDeviceStopped() override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStripProcessorPlayer)
};
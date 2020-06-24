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



private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStripProcessorPlayer)
};
/*
  ==============================================================================

    ChannelStripProcessorPlayer.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "ChannelStripProcessorPlayer.h"

ChannelStripProcessorPlayer::ChannelStripProcessorPlayer()
{

}

ChannelStripProcessorPlayer::~ChannelStripProcessorPlayer()
{

}

void ChannelStripProcessorPlayer::audioDeviceIOCallback(const float** const inputChannelData,
    const int numInputChannels,
    float** const outputChannelData,
    const int numOutputChannels,
    const int numSamples)
{
    AudioProcessorPlayer::audioDeviceIOCallback(inputChannelData, numInputChannels, outputChannelData, numOutputChannels, numSamples);
}

void ChannelStripProcessorPlayer::audioDeviceAboutToStart(AudioIODevice* const device)
{
    AudioProcessorPlayer::audioDeviceAboutToStart(device);
}

void ChannelStripProcessorPlayer::audioDeviceStopped()
{
    AudioProcessorPlayer::audioDeviceStopped();
}
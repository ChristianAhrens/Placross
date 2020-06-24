/*
  ==============================================================================

    MainPlacrossContentComponent.h
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "ChannelStrip/ChannelStripComponent.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainPlacrossContentComponent   : public AudioAppComponent,
                               public ChangeListener,
                               public Timer
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
    
    //==========================================================================
    void changeListenerCallback (ChangeBroadcaster* source) override;
    
    //==========================================================================
    void timerCallback() override;
    
    void updateLoopState (bool shouldLoop);

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void changeState (TransportState newState);

    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();
    void loopButtonChanged();

    //==========================================================================
    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;
    ToggleButton loopingToggle;
    Label currentPositionLabel;

    //==========================================================================
    ChannelStripComponent graphComponentL;
    ChannelStripComponent graphComponentR;

    AudioFormatManager formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainPlacrossContentComponent)
};

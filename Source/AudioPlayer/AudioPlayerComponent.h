/*
  ==============================================================================

    AudioPlayerComponent.h
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*
*/
class AudioPlayerComponent   :  public Component,
                                public AudioSource,
                                public ChangeListener,
                                public Timer
{
public:
    class Listener
    {
    public:
        virtual void onNewAudiofileLoaded() = 0;
    };

public:
    AudioPlayerComponent();
    ~AudioPlayerComponent() override;

    int getCurrentChannelCount();

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

    //==========================================================================
    void addListener(Listener* l);
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
    Listener* m_listener{ nullptr };

    //==========================================================================
    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;
    ToggleButton loopingToggle;
    Label currentPositionLabel;

    AudioFormatManager formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlayerComponent)
};

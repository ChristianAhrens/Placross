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
    void paint(Graphics& g) override;
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
    TransportState currentState();

    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();
    void playPauseButtonClicked();
    void nextButtonClicked();
    void prevButtonClicked();
    void loopButtonChanged();

    //==========================================================================
    Listener* m_listener{ nullptr };

    //==========================================================================
    std::unique_ptr<TextButton> m_openButton;
    //std::unique_ptr<TextButton> m_playButton;
    //std::unique_ptr<TextButton> m_stopButton;
    std::unique_ptr<DrawableButton> m_playPauseButton;
    std::unique_ptr<DrawableButton> m_nextButton;
    std::unique_ptr<DrawableButton> m_prevButton;
    std::unique_ptr<ToggleButton> m_loopingToggle;
    std::unique_ptr<Label> m_currentPositionLabel;

    AudioFormatManager m_formatManager;
    std::unique_ptr<AudioFormatReaderSource> m_readerSource;
    AudioTransportSource m_transportSource;
    TransportState m_state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlayerComponent)
};

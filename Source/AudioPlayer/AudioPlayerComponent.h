/*
  ==============================================================================

    AudioPlayerComponent.h
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "../submodules/JUCE-AppBasics/Source/OverlayToggleComponentBase.h"

//==============================================================================
/*
*
*/
class AudioPlayerComponent   :  public JUCEAppBasics::OverlayToggleComponentBase,
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
    virtual ~AudioPlayerComponent();

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

protected:
    void changeOverlayState() override;

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void changeTransportState (TransportState newState);
    TransportState getCurrentTransportState();

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
    std::unique_ptr<DrawableButton> m_playPauseButton;
    std::unique_ptr<DrawableButton> m_nextButton;
    std::unique_ptr<DrawableButton> m_prevButton;
    std::unique_ptr<ToggleButton> m_loopingToggle;
    std::unique_ptr<Label> m_currentPositionLabel;

    AudioFormatManager m_formatManager;
    std::unique_ptr<AudioFormatReaderSource> m_readerSource;
    AudioTransportSource m_transportSource;
    TransportState m_transportState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlayerComponent)
};

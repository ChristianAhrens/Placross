/*
  ==============================================================================

    AudioPlayerComponent.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "AudioPlayerComponent.h"

//==============================================================================
    AudioPlayerComponent::AudioPlayerComponent()
        :   state (Stopped)
    {
        addAndMakeVisible (&openButton);
        openButton.setButtonText ("Open...");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible (&playButton);
        playButton.setButtonText ("Play");
        playButton.onClick = [this] { playButtonClicked(); };
        playButton.setColour (TextButton::buttonColourId, Colours::green);
        playButton.setEnabled (false);

        addAndMakeVisible (&stopButton);
        stopButton.setButtonText ("Stop");
        stopButton.onClick = [this] { stopButtonClicked(); };
        stopButton.setColour (TextButton::buttonColourId, Colours::red);
        stopButton.setEnabled (false);

        addAndMakeVisible (&loopingToggle);
        loopingToggle.setButtonText ("Loop");
        loopingToggle.onClick = [this] { loopButtonChanged(); };

        addAndMakeVisible (&currentPositionLabel);
        currentPositionLabel.setText ("Stopped", dontSendNotification);

        setSize (300, 300);

        formatManager.registerBasicFormats();
        transportSource.addChangeListener (this);

        startTimer (20);
    }

    AudioPlayerComponent::~AudioPlayerComponent()
    {
    }

    int AudioPlayerComponent::getCurrentChannelCount()
    {
        if (readerSource && readerSource->getAudioFormatReader())
            return readerSource->getAudioFormatReader()->numChannels;
        else
            return 2;
    }

    void AudioPlayerComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
    {
        // This function will be called when the audio device is started, or when
        // its settings (i.e. sample rate, block size, etc) are changed.

        // You can use this function to initialise any resources you might need,
        // but be careful - it will be called on the audio thread, not the GUI thread.

        // For more details, see the help for AudioProcessor::prepareToPlay()
        transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void AudioPlayerComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
    {
        // Your audio-processing code goes here!

        // For more details, see the help for AudioProcessor::getNextAudioBlock()

        // Right now we are not producing any data, in which case we need to clear the buffer
        // (to prevent the output of random noise)
        if (readerSource.get() == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        transportSource.getNextAudioBlock (bufferToFill);
    }
    
    void AudioPlayerComponent::releaseResources()
    {
        // This will be called when the audio device stops, or when it is being
        // restarted due to a setting change.

        // For more details, see the help for AudioProcessor::releaseResources()
        transportSource.releaseResources();
    }

    void AudioPlayerComponent::resized()
    {
        FlexBox fb;
        fb.flexDirection = FlexBox::Direction::column;
        fb.justifyContent = FlexBox::JustifyContent::center;

        FlexBox nestedFb;
        nestedFb.flexDirection = FlexBox::Direction::row;
        nestedFb.justifyContent = FlexBox::JustifyContent::center;
        nestedFb.items.addArray({
                FlexItem(loopingToggle).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0)),
                FlexItem(currentPositionLabel).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0))
            });

        fb.items.addArray({
            FlexItem(openButton).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0)),
            FlexItem(playButton).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0)),
            FlexItem(stopButton).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0)),
            FlexItem(nestedFb)  .withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0))
            });
        
        fb.performLayout(getLocalBounds().toFloat());
    }

    void AudioPlayerComponent::changeListenerCallback (ChangeBroadcaster* source)
    {
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
                changeState (Playing);
            else
                changeState (Stopped);
        }
    }

    void AudioPlayerComponent::timerCallback()
    {
        if (transportSource.isPlaying())
        {
            RelativeTime position (transportSource.getCurrentPosition());

            auto minutes = ((int) position.inMinutes()) % 60;
            auto seconds = ((int) position.inSeconds()) % 60;
            auto millis  = ((int) position.inMilliseconds()) % 1000;

            auto positionString = String::formatted ("%02d:%02d:%03d", minutes, seconds, millis);

            currentPositionLabel.setText (positionString, dontSendNotification);
        }
        else
        {
            currentPositionLabel.setText ("Stopped", dontSendNotification);
        }
    }

    void AudioPlayerComponent::addListener(Listener* l)
    {
        m_listener = l;
    }

    void AudioPlayerComponent::updateLoopState (bool shouldLoop)
    {
        if (readerSource.get() != nullptr)
            readerSource->setLooping (shouldLoop);
    }

    void AudioPlayerComponent::changeState (TransportState newState)
    {
        if (state != newState)
        {
            state = newState;

            switch (state)
            {
                case Stopped:
                    stopButton.setEnabled (false);
                    playButton.setEnabled (true);
                    transportSource.setPosition (0.0);
                    break;

                case Starting:
                    playButton.setEnabled (false);
                    transportSource.start();
                    break;

                case Playing:
                    stopButton.setEnabled (true);
                    break;

                case Stopping:
                    transportSource.stop();
                    break;
            }
        }
    }

    void AudioPlayerComponent::openButtonClicked()
    {
        FileChooser chooser ("Select an audio file to play...",
                             {},
            formatManager.getWildcardForAllFormats());

        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            auto* reader = formatManager.createReaderFor (file);

            if (reader != nullptr)
            {
                std::unique_ptr<AudioFormatReaderSource> newSource (new AudioFormatReaderSource (reader, true));                
                transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate, reader->numChannels);
                playButton.setEnabled (true);
                readerSource.reset (newSource.release());

                if (m_listener)
                    m_listener->onNewAudiofileLoaded();
            }
        }
    }

    void AudioPlayerComponent::playButtonClicked()
    {
        updateLoopState (loopingToggle.getToggleState());
        changeState (Starting);
    }

    void AudioPlayerComponent::stopButtonClicked()
    {
        changeState (Stopping);
    }

    void AudioPlayerComponent::loopButtonChanged()
    {
        updateLoopState (loopingToggle.getToggleState());
    }


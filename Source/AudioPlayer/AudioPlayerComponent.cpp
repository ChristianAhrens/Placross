/*
  ==============================================================================

    AudioPlayerComponent.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "AudioPlayerComponent.h"

#include "../submodules/JUCE-AppBasics/Source/Image_utils.hpp"

//==============================================================================
AudioPlayerComponent::AudioPlayerComponent()
    : m_transportState(Stopped)
{
    std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;

    m_openButton = std::make_unique<TextButton>();
    addAndMakeVisible(m_openButton.get());
    m_openButton->setButtonText ("Open...");
    m_openButton->onClick = [this] { openButtonClicked(); };

    m_playPauseButton = std::make_unique<DrawableButton>(String(), DrawableButton::ButtonStyle::ImageFitted);
    addAndMakeVisible(m_playPauseButton.get());
    m_playPauseButton->onClick = [this] { playPauseButtonClicked(); };
    m_playPauseButton->setEnabled(false);
    JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::pause24px_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage);
    JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::play_arrow24px_svg, NormalImage, OverImage, DownImage, DisabledImage);
    m_playPauseButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
    m_playPauseButton->setClickingTogglesState(true);

    m_nextButton = std::make_unique<DrawableButton>(String(), DrawableButton::ButtonStyle::ImageFitted);
    addAndMakeVisible(m_nextButton.get());
    m_nextButton->onClick = [this] { nextButtonClicked(); };
    m_nextButton->setEnabled(false);
    JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::fast_forward24px_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage);
    m_nextButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());

    m_prevButton = std::make_unique<DrawableButton>(String(), DrawableButton::ButtonStyle::ImageFitted);
    addAndMakeVisible(m_prevButton.get());
    m_prevButton->onClick = [this] { prevButtonClicked(); };
    m_prevButton->setEnabled(false);
    JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::fast_rewind24px_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage);
    m_prevButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());

    m_loopingToggle = std::make_unique<ToggleButton>();
    addAndMakeVisible(m_loopingToggle.get());
    m_loopingToggle->setButtonText ("Loop");
    m_loopingToggle->onClick = [this] { loopButtonChanged(); };

    m_currentPositionLabel = std::make_unique<Label>();
    addAndMakeVisible(m_currentPositionLabel.get());
    m_currentPositionLabel->setText ("Stopped", dontSendNotification);

    setSize (300, 300);

    m_formatManager.registerBasicFormats();
    m_transportSource.addChangeListener (this);

    startTimer (20);
}

AudioPlayerComponent::~AudioPlayerComponent()
{
}

int AudioPlayerComponent::getCurrentChannelCount()
{
    if (m_readerSource && m_readerSource->getAudioFormatReader())
        return m_readerSource->getAudioFormatReader()->numChannels;
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
    m_transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void AudioPlayerComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    if (m_readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    m_transportSource.getNextAudioBlock (bufferToFill);
}
    
void AudioPlayerComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    m_transportSource.releaseResources();
}

void AudioPlayerComponent::resized()
{
    OverlayToggleComponentBase::resized();

    FlexBox fb;
    fb.flexDirection = FlexBox::Direction::column;
    fb.justifyContent = FlexBox::JustifyContent::flexStart;

    // we are using the prev/play/next buttons section in both maximized and minimized mode
    FlexBox playCtlFb;
    playCtlFb.flexDirection = FlexBox::Direction::row;
    playCtlFb.justifyContent = FlexBox::JustifyContent::center;
    playCtlFb.items.addArray({
            FlexItem(*m_prevButton).withFlex(1).withMargin(FlexItem::Margin(15, 5, 15, 5)),
            FlexItem(*m_playPauseButton).withFlex(1).withMargin(FlexItem::Margin(0, 0, 0, 0)),
            FlexItem(*m_nextButton).withFlex(1).withMargin(FlexItem::Margin(15, 5, 15, 5))
        });

    // layout all elements for maximized mode
    if(getCurrentOverlayState() == maximized)
    {
        FlexBox loopCtlFb;
        loopCtlFb.flexDirection = FlexBox::Direction::row;
        loopCtlFb.justifyContent = FlexBox::JustifyContent::center;
        loopCtlFb.items.addArray({
                FlexItem(*m_loopingToggle).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0)),
                FlexItem(*m_currentPositionLabel).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0))
            });

        fb.items.addArray({
            FlexItem(*m_openButton).withFlex(1).withMargin(FlexItem::Margin(30, 10, 5, 10)).withMaxHeight(30),
            FlexItem(playCtlFb).withFlex(2).withMargin(FlexItem::Margin(5, 10, 5, 10)).withMaxHeight(80),
            FlexItem(loopCtlFb).withFlex(1).withMargin(FlexItem::Margin(5, 10, 5, 10)).withMaxHeight(30)
            });
    }
    // and only the player section in minimized mode
    else if (getCurrentOverlayState() == minimized)
    {
        fb.items.add(FlexItem(playCtlFb).withFlex(1).withMargin(FlexItem::Margin(5, 10, 5, 10)).withMaxHeight(80));
    }
        
    fb.performLayout(getOverlayBounds().toFloat());
}

void AudioPlayerComponent::changeOverlayState()
{
    OverlayToggleComponentBase::changeOverlayState();

    if (getCurrentOverlayState() == maximized)
    {
        m_openButton->setVisible(true);
        m_loopingToggle->setVisible(true);
        m_currentPositionLabel->setVisible(true);
    }
    else if (getCurrentOverlayState() == minimized)
    {
        m_openButton->setVisible(false);
        m_loopingToggle->setVisible(false);
        m_currentPositionLabel->setVisible(false);
    }
}

void AudioPlayerComponent::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == &m_transportSource)
    {
        if (m_transportSource.isPlaying())
            changeTransportState(Playing);
        else
            changeTransportState(Stopped);
    }
}

void AudioPlayerComponent::timerCallback()
{
    if (m_transportSource.isPlaying())
    {
        RelativeTime position (m_transportSource.getCurrentPosition());

        auto minutes = ((int) position.inMinutes()) % 60;
        auto seconds = ((int) position.inSeconds()) % 60;
        auto millis  = ((int) position.inMilliseconds()) % 1000;

        auto positionString = String::formatted ("%02d:%02d:%03d", minutes, seconds, millis);

        m_currentPositionLabel->setText (positionString, dontSendNotification);
    }
    else
    {
        m_currentPositionLabel->setText ("Stopped", dontSendNotification);
        m_playPauseButton->setToggleState(false, dontSendNotification);
    }
}

void AudioPlayerComponent::addListener(Listener* l)
{
    m_listener = l;
}

void AudioPlayerComponent::updateLoopState (bool shouldLoop)
{
    if (m_readerSource.get() != nullptr)
        m_readerSource->setLooping (shouldLoop);
}

void AudioPlayerComponent::changeTransportState(TransportState newState)
{
    if (m_transportState != newState)
    {
        m_transportState = newState;

        switch (m_transportState)
        {
            case Stopped:
                m_transportSource.setPosition (0.0);
                break;

            case Starting:
                m_playPauseButton->setEnabled(false);
                m_prevButton->setEnabled(false);
                m_nextButton->setEnabled(false);
                m_transportSource.start();
                break;

            case Playing:
                m_playPauseButton->setEnabled(true);
                m_prevButton->setEnabled(true);
                m_nextButton->setEnabled(true);
                break;

            case Stopping:
                m_transportSource.stop();
                break;
        }
    }
}

AudioPlayerComponent::TransportState AudioPlayerComponent::getCurrentTransportState()
{
    return m_transportState;
}

void AudioPlayerComponent::openButtonClicked()
{
    FileChooser chooser ("Select an audio file to play...",
                            {},
        m_formatManager.getWildcardForAllFormats());

    if (chooser.browseForFileToOpen())
    {
        auto file = chooser.getResult();
        auto* reader = m_formatManager.createReaderFor (file);

        if (reader != nullptr)
        {
            std::unique_ptr<AudioFormatReaderSource> newSource (new AudioFormatReaderSource (reader, true));                
            m_transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate, reader->numChannels);
            m_playPauseButton->setEnabled(true);
            m_prevButton->setEnabled(true);
            m_nextButton->setEnabled(true);
            m_readerSource.reset (newSource.release());

            if (m_listener)
                m_listener->onNewAudiofileLoaded();
        }
    }
}

void AudioPlayerComponent::playButtonClicked()
{
    updateLoopState (m_loopingToggle->getToggleState());
    changeTransportState(Starting);
}

void AudioPlayerComponent::stopButtonClicked()
{
    changeTransportState(Stopping);
}

void AudioPlayerComponent::playPauseButtonClicked()
{
    if (getCurrentTransportState() == Playing)
    {
        changeTransportState(Stopping);
    }
    else
    {
        updateLoopState(m_loopingToggle->getToggleState());
        changeTransportState(Starting);
    }
}

void AudioPlayerComponent::nextButtonClicked()
{

}

void AudioPlayerComponent::prevButtonClicked()
{

}

void AudioPlayerComponent::loopButtonChanged()
{
    updateLoopState (m_loopingToggle->getToggleState());
}


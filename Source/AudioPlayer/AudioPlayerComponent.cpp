/*
  ==============================================================================

    AudioPlayerComponent.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "AudioPlayerComponent.h"

#include <Image_utils.h>

//==============================================================================
AudioPlayerComponent::AudioPlayerComponent()
    : m_transportState(TS_Stopped)
{
    std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;

    m_openButton = std::make_unique<DrawableButton>(String(), DrawableButton::ButtonStyle::ImageOnButtonBackground);
    addAndMakeVisible(m_openButton.get());
    m_openButton->onClick = [this] { openButtonClicked(); };
    JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::folder_open24px_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage);
    m_openButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());

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

    m_tableModel = std::make_unique<AudioPlayerTitleTableModel>();
    m_tableModel->setCellColours(
        getLookAndFeel().findColour(TableHeaderComponent::ColourIds::backgroundColourId),
        getLookAndFeel().findColour(TableHeaderComponent::ColourIds::highlightColourId),
        getLookAndFeel().findColour(TableHeaderComponent::ColourIds::outlineColourId),
        getLookAndFeel().findColour(TableHeaderComponent::ColourIds::textColourId));
    m_tableModel->titleSelected = [this](String titleName) { onAudioTitleSelected(titleName); };
    m_tableListBox = std::make_unique<AudioPlayerTitleTableListBox>();
    m_tableListBox->setMultipleSelectionEnabled(false);
    m_tableListBox->setModel(m_tableModel.get());
    // Add columns to the table header
    m_tableListBox->getHeader().addColumn("Title", THC_Title, 120);
    m_tableListBox->getHeader().addColumn("", THC_Length, 30);
    m_tableListBox->getHeader().setSortColumnId(THC_Title, true); // sort forwards by the Input number column
    m_tableListBox->getHeader().setStretchToFitActive(true);
    addAndMakeVisible(m_tableListBox.get());

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

void AudioPlayerComponent::onAudioTitleSelected(String titleName)
{
    File audioFile(titleName);
    loadAudioFile(audioFile);
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

    auto isPortrait = getOverlayBounds().getHeight() > getOverlayBounds().getWidth();

    FlexBox fb;
    fb.justifyContent = FlexBox::JustifyContent::flexStart;

    // we are using the prev/play/next buttons section in both maximized and minimized mode
    FlexBox playCtlFb;
    playCtlFb.justifyContent = FlexBox::JustifyContent::center;
    playCtlFb.items.addArray({
            FlexItem(*m_prevButton).withFlex(1).withMargin(FlexItem::Margin(15, 5, 15, 5)),
            FlexItem(*m_playPauseButton).withFlex(1).withMargin(FlexItem::Margin(0, 0, 0, 0)),
            FlexItem(*m_nextButton).withFlex(1).withMargin(FlexItem::Margin(15, 5, 15, 5))
        });

    // The loop control row is oonly used in maximized mode but we still need to create it in the methods general scope
    FlexBox loopCtlFb;
    loopCtlFb.flexDirection = FlexBox::Direction::row;
    loopCtlFb.justifyContent = FlexBox::JustifyContent::center;
    loopCtlFb.items.addArray({
            FlexItem(*m_loopingToggle).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0)),
            FlexItem(*m_currentPositionLabel).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0))
        });

    // The table and loop control shall always be in a column
    FlexBox tableLoopFb;
    tableLoopFb.flexDirection = FlexBox::Direction::column;
    tableLoopFb.justifyContent = FlexBox::JustifyContent::center;
    tableLoopFb.items.addArray({
                FlexItem(*m_tableListBox).withFlex(4).withMargin(FlexItem::Margin(5, 5, 5, 5)),
                FlexItem(loopCtlFb).withFlex(1).withMargin(FlexItem::Margin(5, 10, 5, 10)).withMaxHeight(30)
        });

    // layout all elements for maximized mode
    if(getCurrentOverlayState() == maximized)
    {
        if (isPortrait)
        {
            playCtlFb.flexDirection = FlexBox::Direction::row;
            fb.flexDirection = FlexBox::Direction::column;
            fb.items.addArray({
                FlexItem(*m_openButton).withFlex(1).withMargin(FlexItem::Margin(30, 10, 5, 10)).withMaxHeight(30),
                FlexItem(playCtlFb).withFlex(2).withMargin(FlexItem::Margin(5, 10, 5, 10)).withMaxHeight(80),
                FlexItem(tableLoopFb).withFlex(5).withMargin(FlexItem::Margin(5, 5, 5, 5))
                });
        }
        else
        {
            playCtlFb.flexDirection = FlexBox::Direction::column;
            fb.flexDirection = FlexBox::Direction::row;
            fb.items.addArray({
                FlexItem(*m_openButton).withFlex(1).withMargin(FlexItem::Margin(30, 10, 5, 10)).withMaxWidth(30),
                FlexItem(playCtlFb).withFlex(2).withMargin(FlexItem::Margin(5, 10, 5, 10)).withMaxWidth(80),
                FlexItem(tableLoopFb).withFlex(5).withMargin(FlexItem::Margin(5, 5, 5, 5))
                });
        }
    }
    // and only the player section in minimized mode
    else if (getCurrentOverlayState() == minimized)
    {
        fb.flexDirection = FlexBox::Direction::column;
        if (isPortrait)
        {
            playCtlFb.flexDirection = FlexBox::Direction::column;
            fb.items.add(FlexItem(playCtlFb).withFlex(1).withMargin(FlexItem::Margin(5, 10, 5, 10)).withMaxWidth(80));
        }
        else
        {
            playCtlFb.flexDirection = FlexBox::Direction::row;
            fb.items.add(FlexItem(playCtlFb).withFlex(1).withMargin(FlexItem::Margin(5, 10, 5, 10)).withMaxHeight(80));
        }
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
        m_tableListBox->setVisible(true);
    }
    else if (getCurrentOverlayState() == minimized)
    {
        m_openButton->setVisible(false);
        m_loopingToggle->setVisible(false);
        m_currentPositionLabel->setVisible(false);
        m_tableListBox->setVisible(false);
    }
}

void AudioPlayerComponent::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == &m_transportSource)
    {
        if (m_transportSource.hasStreamFinished() && (m_transportState == TS_Playing))
            playNextAudioFile();
        else if (m_transportSource.isPlaying())
            changeTransportState(TS_Playing);
        else
            changeTransportState(TS_Stopped);
    }
}

void AudioPlayerComponent::timerCallback()
{
    if (m_transportSource.isPlaying())
    {
        RelativeTime position (m_transportSource.getCurrentPosition());

        auto hours = ((int)position.inHours()) % 60;
        auto minutes = ((int)position.inMinutes()) % 60;
        auto seconds = ((int)position.inSeconds()) % 60;
        auto millis = ((int)position.inMilliseconds()) % 1000;

        auto positionString = String::formatted("%02d:%02d:%02d:%03d", hours, minutes, seconds, millis);

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

void AudioPlayerComponent::loadAudioFile(const File& file)
{
    auto* reader = m_formatManager.createReaderFor (file);
    if (reader != nullptr)
    {
        auto lengthInMilliSec = (reader->lengthInSamples / (reader->sampleRate * 0.001f));
        auto rowNumber = m_tableModel->addTitle(std::make_pair(file.getFullPathName().toStdString(), lengthInMilliSec));
        m_tableListBox->updateContent();
        m_tableListBox->selectRow(rowNumber);

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

void AudioPlayerComponent::playNextAudioFile()
{
    auto currentSelectedRows = m_tableListBox->getSelectedRows();
    if (m_tableModel->selectNextTitle(currentSelectedRows))
        changeTransportState(TS_Starting);
    else
        changeTransportState(TS_Stopped);
}

void AudioPlayerComponent::playPrevAudioFile()
{
    auto currentSelectedRows = m_tableListBox->getSelectedRows();
    if (m_tableModel->selectPrevTitle(currentSelectedRows))
        changeTransportState(TS_Starting);
    else
        changeTransportState(TS_Stopped);
}

void AudioPlayerComponent::changeTransportState(TransportState newState)
{
    if (m_transportState != newState)
    {
        m_transportState = newState;

        switch (m_transportState)
        {
            case TS_Stopped:
                m_transportSource.setPosition (0.0);
                break;

            case TS_Starting:
                m_playPauseButton->setEnabled(false);
                m_prevButton->setEnabled(false);
                m_nextButton->setEnabled(false);
                m_transportSource.start();
                break;

            case TS_Playing:
                m_playPauseButton->setEnabled(true);
                m_prevButton->setEnabled(true);
                m_nextButton->setEnabled(true);
                break;

            case TS_Stopping:
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
        loadAudioFile(file);
    }
}

void AudioPlayerComponent::playButtonClicked()
{
    updateLoopState (m_loopingToggle->getToggleState());
    changeTransportState(TS_Starting);
}

void AudioPlayerComponent::stopButtonClicked()
{
    changeTransportState(TS_Stopping);
}

void AudioPlayerComponent::playPauseButtonClicked()
{
    if (getCurrentTransportState() == TS_Playing)
    {
        changeTransportState(TS_Stopping);
    }
    else
    {
        updateLoopState(m_loopingToggle->getToggleState());
        changeTransportState(TS_Starting);
    }
}

void AudioPlayerComponent::nextButtonClicked()
{
    playNextAudioFile();
}

void AudioPlayerComponent::prevButtonClicked()
{
    playPrevAudioFile();
}

void AudioPlayerComponent::loopButtonChanged()
{
    updateLoopState (m_loopingToggle->getToggleState());
}


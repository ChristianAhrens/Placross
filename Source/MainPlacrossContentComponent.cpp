/*
  ==============================================================================

    MainPlacrossContentComponent.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "MainPlacrossContentComponent.h"

//==============================================================================
    MainPlacrossContentComponent::MainPlacrossContentComponent()
    {
        m_playerComponent = std::make_unique<AudioPlayerComponent>();
        m_playerComponent->addListener(this);
        addAndMakeVisible(m_playerComponent.get());

        m_routingComponent = std::make_unique<RoutingComponent>();
        addAndMakeVisible(m_routingComponent.get());

        // Specify the number of output channels that we want to open
        setChannelSetup(m_playerComponent->getCurrentChannelCount(), getCurrentDeviceChannelCount().second);

        setSize(300, 550);
    }

    MainPlacrossContentComponent::~MainPlacrossContentComponent()
    {
        // This shuts down the audio device and clears the audio source.
        shutdownAudio();
    }

    void MainPlacrossContentComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
    {
        m_playerComponent->prepareToPlay (samplesPerBlockExpected, sampleRate);

        m_routingComponent->audioDeviceAboutToStart(deviceManager.getCurrentAudioDevice());

        for (auto& stripComponentKV : m_stripComponents)
            stripComponentKV.second->audioDeviceAboutToStart(deviceManager.getCurrentAudioDevice());
    }

    void MainPlacrossContentComponent::getNextAudioBlock (const AudioSourceChannelInfo& info)
    {
        // get the next chunk of audio from player ...
        m_playerComponent->getNextAudioBlock (info);

        // ... run it through routing
        m_routingComponent->audioDeviceIOCallback(info.buffer->getArrayOfReadPointers(), info.buffer->getNumChannels(), info.buffer->getArrayOfWritePointers(), info.buffer->getNumChannels(), info.numSamples); // @TODO: info.startSample is not regarded here!!

        // ... and run it through the processorGraphs for each channel
        for (auto i = 0; i < info.buffer->getNumChannels(); ++i)
        {
            if (m_stripComponents.count(i) != 0 && m_stripComponents.at(i))
            {
                auto ReadPointer = info.buffer->getReadPointer(i) + info.startSample;
                auto WritePointer = info.buffer->getWritePointer(i) + info.startSample;
                auto strip = m_stripComponents.at(i).get();
                strip->audioDeviceIOCallback(&ReadPointer, 1, &WritePointer, 1, info.numSamples);
            }
        }
    }

    void MainPlacrossContentComponent::releaseResources()
    {
        m_playerComponent->releaseResources();

        for (auto& stripComponentKV : m_stripComponents)
            stripComponentKV.second->audioDeviceStopped();
    }

    void MainPlacrossContentComponent::paint(Graphics& g)
    {
        
    }

    void MainPlacrossContentComponent::resized()
    {
        FlexBox fb;
        fb.flexDirection = FlexBox::Direction::column;
        fb.justifyContent = FlexBox::JustifyContent::center;

        FlexBox nestedFb;
        nestedFb.flexDirection = FlexBox::Direction::row;
        nestedFb.justifyContent = FlexBox::JustifyContent::center;
        for (auto& stripComponentKV : m_stripComponents)
        {
            nestedFb.items.add(FlexItem(*stripComponentKV.second.get()).withFlex(1).withMargin(FlexItem::Margin(5, 5, 5, 5)));
        }

        fb.items.addArray({
            FlexItem(*m_playerComponent.get()).withMinHeight(160).withMargin(FlexItem::Margin(10,10,0,10)),
            FlexItem(*m_routingComponent.get()).withMinHeight(30).withMargin(FlexItem::Margin(10,10,0,10)),
            FlexItem(nestedFb).withFlex(1).withMinHeight(150).withMargin(FlexItem::Margin(5,5,5,5))
            });
        fb.performLayout(getLocalBounds().toFloat());

        if (isEditorActive())
        {
            m_overlayEditor->setBounds(getBounds().reduced(10));
        }
    }

    void MainPlacrossContentComponent::onNewAudiofileLoaded()
    {
        setChannelSetup(m_playerComponent->getCurrentChannelCount(), getCurrentDeviceChannelCount().second);
    }

    void MainPlacrossContentComponent::setOverlayEditor(OverlayEditorComponentBase* editor)
    {
        if (isEditorActive() && editor == nullptr)
        {
            m_overlayEditor->setVisible(false);
            removeChildComponent(m_overlayEditor);
            m_overlayEditor = nullptr;
        }
        else if (!isEditorActive() && editor != nullptr)
        {
            m_overlayEditor = editor;
            addAndMakeVisible(m_overlayEditor);
            resized();
        }
    }

    bool MainPlacrossContentComponent::isEditorActive()
    {
        return m_overlayEditor != nullptr;
    }

    void MainPlacrossContentComponent::setChannelSetup(int numInputChannels, int numOutputChannels, const XmlElement* const storedSettings)
    {
        for (auto i = 0; i < numOutputChannels; ++i)
        {
            if (m_stripComponents.count(i) == 0)
            {
                m_stripComponents[i] = std::make_unique<ChannelStripComponent>();
                addAndMakeVisible(m_stripComponents.at(i).get());
            }
        }

        m_routingComponent->setIOCount(numInputChannels, numOutputChannels);

        // for our baseclass, the audio device in/out count is relevant. Since we only use playback, inputs are always 0 here!
        setAudioChannels(0, numOutputChannels, storedSettings);
    }

    std::pair<int, int> MainPlacrossContentComponent::getCurrentDeviceChannelCount()
    {
        if(deviceManager.getCurrentAudioDevice())
        {
            auto activeInputChannels = deviceManager.getCurrentAudioDevice()->getActiveInputChannels();
            auto activeOutputChannels = deviceManager.getCurrentAudioDevice()->getActiveOutputChannels();
            auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
            auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

            return std::make_pair(maxInputChannels, maxOutputChannels);
        }
        else
        {
            return std::make_pair(2, 2);
        }
    }

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
        addAndMakeVisible(m_playerComponent.get());

        m_routingComponent = std::make_unique<RoutingComponent>();
        addAndMakeVisible(m_routingComponent.get());

        for (auto i = 0; i < m_playerComponent->getCurrentChannelCount(); ++i)
        {
            m_stripComponents[i] = std::make_unique<ChannelStripComponent>();
            addAndMakeVisible(m_stripComponents.at(i).get());
        }

        setSize (300, 550);

        // Specify the number of output channels that we want to open
        setAudioChannels (0, m_playerComponent->getCurrentChannelCount());
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
    }


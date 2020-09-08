/*
  ==============================================================================

    MainPlacrossContentComponent.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "MainPlacrossContentComponent.h"

#include "../submodules/JUCE-AppBasics/Source/iOS_utils.hpp"


static constexpr int MAX_SUPPORTED_OUTPUTS = 10;

//==============================================================================
class CircleComponent : public Component
{
public:
    CircleComponent()
    {
    };
    CircleComponent(const Colour& colour)
    {
        setCircleColour(colour);
    };
    ~CircleComponent() override
    {
    };

    //==========================================================================
    void paint(Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());

        auto h = static_cast<float>(getHeight());
        auto w = static_cast<float>(getWidth());
        Rectangle<float> smallestSquareRect;
        if (w < h)
        {
            smallestSquareRect = Rectangle<float>(0, 0.5f * (h - w), w, w);
        }
        else
        {
            smallestSquareRect = Rectangle<float>(0.5f * (w - h), 0, h, h);
        }

        g.setColour(m_circleColour);
        g.fillEllipse(smallestSquareRect);
    };

    void setCircleColour(const Colour& colour)
    {
        m_circleColour = colour;
    };

private:
    //==========================================================================
    Colour m_circleColour{ Colours::black };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CircleComponent)
};


//==============================================================================
MainPlacrossContentComponent::MainPlacrossContentComponent()
{
    deviceManager.initialiseWithDefaultDevices(0, MAX_SUPPORTED_OUTPUTS);
    
    m_playerComponent = std::make_unique<AudioPlayerComponent>();
    m_playerComponent->addListener(this);
    m_playerComponent->addOverlayParent(this);
    m_playerComponent->parentResize = [this] { resized(); };
    addAndMakeVisible(m_playerComponent.get());

    m_routingComponent = std::make_unique<RoutingComponent>();
    m_routingComponent->addOverlayParent(this);
    m_routingComponent->parentResize = [this] { resized(); };
    addAndMakeVisible(m_routingComponent.get());

    m_analyserComponent = std::make_unique<AnalyserComponent>();
    m_analyserComponent->addOverlayParent(this);
    m_analyserComponent->parentResize = [this] { resized(); };
    addAndMakeVisible(m_analyserComponent.get());

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

    m_analyserComponent->audioDeviceAboutToStart(deviceManager.getCurrentAudioDevice());
}

void MainPlacrossContentComponent::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    // get the next chunk of audio from player ...
    m_playerComponent->getNextAudioBlock (info);

    // ... run it through routing
    m_routingComponent->audioDeviceIOCallback(info.buffer->getArrayOfReadPointers(), info.buffer->getNumChannels(), info.buffer->getArrayOfWritePointers(), info.buffer->getNumChannels(), info.numSamples); // @TODO: info.startSample is not regarded here!!

    // ... run it through the processorGraphs for each channel
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

    // ... and run it through routing
    m_analyserComponent->audioDeviceIOCallback(info.buffer->getArrayOfReadPointers(), info.buffer->getNumChannels(), info.buffer->getArrayOfWritePointers(), info.buffer->getNumChannels(), info.numSamples); // @TODO: info.startSample is not regarded here!!
}

void MainPlacrossContentComponent::releaseResources()
{
    m_playerComponent->releaseResources();

    for (auto& stripComponentKV : m_stripComponents)
        stripComponentKV.second->audioDeviceStopped();
}

void MainPlacrossContentComponent::paint(Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
}

void MainPlacrossContentComponent::resized()
{
    auto isPortrait = getLocalBounds().getHeight() > getLocalBounds().getWidth();
    auto safety = JUCEAppBasics::iOS_utils::getDeviceSafetyMargins();
    auto safeBounds = getLocalBounds();
    safeBounds.removeFromTop(safety._top);
    safeBounds.removeFromBottom(safety._bottom);
    safeBounds.removeFromLeft(safety._left);
    safeBounds.removeFromRight(safety._right);

    if (isOverlayActive())
    {
        m_activeOverlay->setBounds(safeBounds);
    }
    else
    {
        if (isPortrait)
        {
            FlexBox playerCirclesFb;
            playerCirclesFb.flexDirection = FlexBox::Direction::row;
            playerCirclesFb.items.add(FlexItem().withFlex(3));
            for (auto& playerConCircleKV : m_playerConCircles)
            {
                playerCirclesFb.items.add(FlexItem(*playerConCircleKV.get()).withFlex(1));
            }
            playerCirclesFb.items.add(FlexItem().withFlex(3));

            FlexBox routingCirclesFb;
            routingCirclesFb.flexDirection = FlexBox::Direction::row;
            for (auto& routingConCircleKV : m_routingConCircles)
            {
                routingCirclesFb.items.add(FlexItem().withFlex(2));
                routingCirclesFb.items.add(FlexItem(*routingConCircleKV.get()).withFlex(1));
                routingCirclesFb.items.add(FlexItem().withFlex(2));
            }

            FlexBox stripComponentsFb;
            stripComponentsFb.flexDirection = FlexBox::Direction::row;
            for (auto& stripComponentKV : m_stripComponents)
            {
                stripComponentsFb.items.add(FlexItem(*stripComponentKV.second.get()).withFlex(1).withMargin(FlexItem::Margin(0, 5, 0, 5)));
            }

            FlexBox stripCirclesFb;
            stripCirclesFb.flexDirection = FlexBox::Direction::row;
            for (auto& stripConCircleKV : m_stripConCircles)
            {
                stripCirclesFb.items.add(FlexItem().withFlex(2));
                stripCirclesFb.items.add(FlexItem(*stripConCircleKV.get()).withFlex(1));
                stripCirclesFb.items.add(FlexItem().withFlex(2));
            }


            FlexBox fb;
            fb.flexDirection = FlexBox::Direction::column;
            fb.items.addArray({
                FlexItem(*m_playerComponent.get()).withMinHeight(90).withMargin(FlexItem::Margin(10,10,0,10)),
                FlexItem(playerCirclesFb).withMinHeight(10),
                FlexItem(*m_routingComponent.get()).withMinHeight(45).withMargin(FlexItem::Margin(0,10,0,10)),
                FlexItem(routingCirclesFb).withMinHeight(10),
                FlexItem(stripComponentsFb).withFlex(1).withMinHeight(150).withMargin(FlexItem::Margin(0,5,0,5)),
                FlexItem(stripCirclesFb).withMinHeight(10),
                FlexItem(*m_analyserComponent.get()).withMinHeight(150).withMargin(FlexItem::Margin(0,10,10,10))
                });
            fb.performLayout(safeBounds.toFloat());
        }
        else
        {
            FlexBox playerCirclesFb;
            playerCirclesFb.flexDirection = FlexBox::Direction::column;
            playerCirclesFb.items.add(FlexItem().withFlex(3));
            for (auto& playerConCircleKV : m_playerConCircles)
            {
                playerCirclesFb.items.add(FlexItem(*playerConCircleKV.get()).withFlex(1));
            }
            playerCirclesFb.items.add(FlexItem().withFlex(3));

            FlexBox routingCirclesFb;
            routingCirclesFb.flexDirection = FlexBox::Direction::column;
            for (auto& routingConCircleKV : m_routingConCircles)
            {
                routingCirclesFb.items.add(FlexItem().withFlex(2));
                routingCirclesFb.items.add(FlexItem(*routingConCircleKV.get()).withFlex(1));
                routingCirclesFb.items.add(FlexItem().withFlex(2));
            }

            FlexBox stripComponentsFb;
            stripComponentsFb.flexDirection = FlexBox::Direction::column;
            stripComponentsFb.justifyContent = FlexBox::JustifyContent::center;
            for (auto& stripComponentKV : m_stripComponents)
            {
                stripComponentsFb.items.add(FlexItem(*stripComponentKV.second.get()).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0)));
            }

            FlexBox stripCirclesFb;
            stripCirclesFb.flexDirection = FlexBox::Direction::column;
            for (auto& stripConCircleKV : m_stripConCircles)
            {
                stripCirclesFb.items.add(FlexItem().withFlex(2));
                stripCirclesFb.items.add(FlexItem(*stripConCircleKV.get()).withFlex(1));
                stripCirclesFb.items.add(FlexItem().withFlex(2));
            }


            FlexBox fb;
            fb.flexDirection = FlexBox::Direction::row;
            fb.items.addArray({
                FlexItem(*m_playerComponent.get()).withMinWidth(90).withMargin(FlexItem::Margin(10,0,10,10)),
                FlexItem(playerCirclesFb).withMinWidth(10),
                FlexItem(*m_routingComponent.get()).withMinWidth(45).withMargin(FlexItem::Margin(10,0,10,0)),
                FlexItem(routingCirclesFb).withMinWidth(10),
                FlexItem(stripComponentsFb).withFlex(1).withMinWidth(150).withMargin(FlexItem::Margin(5,0,5,0)),
                FlexItem(stripCirclesFb).withMinWidth(10),
                FlexItem(*m_analyserComponent.get()).withMinWidth(150).withMargin(FlexItem::Margin(10,10,10,0))
                });
            fb.performLayout(safeBounds.toFloat());
        }
    }
}

void MainPlacrossContentComponent::onNewAudiofileLoaded()
{
    setChannelSetup(m_playerComponent->getCurrentChannelCount(), getCurrentDeviceChannelCount().second);
}

void MainPlacrossContentComponent::setChannelSetup(int numInputChannels, int numOutputChannels, const XmlElement* const storedSettings)
{
    // add or remove connection indication circle components
    // for player block depending on new input channel count
    while (m_playerConCircles.size() > numInputChannels)
    {
        removeChildComponent(m_playerConCircles.back().get());
        m_playerConCircles.pop_back();
    }
    while (m_playerConCircles.size() < numInputChannels)
    {
        auto circle = std::make_unique<CircleComponent>();
        addAndMakeVisible(circle.get());
        m_playerConCircles.push_back(std::move(circle));
    }
    
    // add or remove channel strip components
    // for player block depending on new output channel count
    std::vector<int> stripsToRemove;
    for(auto const& stripComponentKV : m_stripComponents)
    {
        auto stripChNum = stripComponentKV.first + 1;
        if(stripChNum > numOutputChannels)
            stripsToRemove.push_back(stripComponentKV.first);
    }
    for(auto ch : stripsToRemove)
    {
        removeChildComponent(m_stripComponents.at(ch).get());
        m_stripComponents.erase(ch);
    }
    for (auto i = 0; i < numOutputChannels; ++i)
    {
        if (m_stripComponents.count(i) == 0)
        {
            m_stripComponents[i] = std::make_unique<ChannelStripComponent>();
            m_stripComponents.at(i)->addOverlayParent(this);
            m_stripComponents.at(i)->parentResize = [this] { resized(); };
            addAndMakeVisible(m_stripComponents.at(i).get());
        }
    }
    
    // add or remove connection indication circle components
    // for channelstrip block depending on new output channel count
    while (m_stripConCircles.size() > numOutputChannels)
    {
        removeChildComponent(m_stripConCircles.back().get());
        m_stripConCircles.pop_back();
    }
    while (m_stripConCircles.size() < numOutputChannels)
    {
        auto circle = std::make_unique<CircleComponent>();
        addAndMakeVisible(circle.get());
        m_stripConCircles.push_back(std::move(circle));
    }

    // update the rounting components io count accordingly
    m_routingComponent->setIOCount(numInputChannels, numOutputChannels);

    // add or remove connection indication circle components
    // for routing block depending on new output channel count
    while (m_routingConCircles.size() > numOutputChannels)
    {
        removeChildComponent(m_routingConCircles.back().get());
        m_routingConCircles.pop_back();
    }
    while (m_routingConCircles.size() < numOutputChannels)
    {
        auto circle = std::make_unique<CircleComponent>();
        addAndMakeVisible(circle.get());
        m_routingConCircles.push_back(std::move(circle));
    }

    // for our baseclass, the audio device in/out count is relevant. Since we only use playback, inputs are always 0 here!
    setAudioChannels(0, numOutputChannels, storedSettings);

    // handle channel colouring
    while (m_channelColours.size() < numInputChannels || m_channelColours.size() < numOutputChannels)
    {
        // dont use full 8-bits per channel, but only values from 40-216, to avoid sharp colours
        auto r = uint8(50 + Random::getSystemRandom().nextInt(206));
        auto g = uint8(50 + Random::getSystemRandom().nextInt(206));
        auto b = uint8(50 + Random::getSystemRandom().nextInt(206));

        m_channelColours.push_back(Colour(r, g, b));
    }

    for (int i = 0; i < numInputChannels; i++)
    {
        m_playerConCircles.at(i)->setCircleColour(m_channelColours.at(i));
    }
    for (int i = 0; i < numOutputChannels; i++)
    {
        m_routingConCircles.at(i)->setCircleColour(m_channelColours.at(i));
        m_stripConCircles.at(i)->setCircleColour(m_channelColours.at(i));

        if (m_stripComponents.count(i) > 0)
            m_stripComponents.at(i)->setChannelColour(m_channelColours.at(i));
    }
    m_analyserComponent->setChannelColours(m_channelColours);
    
    // trigger one resize to update the UI arrangement if neccessary
    resized();
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

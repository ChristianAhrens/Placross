/*
  ==============================================================================

    ChannelStripComponent.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "ChannelStripComponent.h"
#include "ChannelStripProcessor.h"
#include "ChannelStripProcessorEditor.h"

ChannelStripComponent::ChannelStripComponent()
	: m_mainProcessor(new AudioProcessorGraph())
{
	initialiseGraph();

	setSize(600, 460);
}

ChannelStripComponent::~ChannelStripComponent()
{
	auto device = MidiInput::getDefaultDevice();

	destroyAudioNodes();
}

void ChannelStripComponent::setChannelColour(const Colour& colour)
{
	if (m_mainProcessor)
	{
		for (auto const& node : m_mainProcessor->getNodes())
		{
			if (node)
			{
				auto processor = node->getProcessor();
				if (processor)
				{
					auto editor = dynamic_cast<ChannelStripProcessorEditor*>(node->getProcessor()->getActiveEditor());
					if (editor)
					{
						editor->setChannelColour(colour);
					}
				}
			}
		}

	}
}

void ChannelStripComponent::resized()
{
	OverlayToggleComponentBase::resized();

	auto isPortrait = getLocalBounds().getHeight() > getLocalBounds().getWidth();

	FlexBox fb;
	fb.flexDirection = isPortrait ? FlexBox::Direction::column : FlexBox::Direction::row;
	fb.justifyContent = FlexBox::JustifyContent::center;

	for (auto& node : m_mainProcessor->getNodes())
	{
		AudioProcessorEditor* editor = node->getProcessor()->getActiveEditor();
		if (editor)
		{
			fb.items.add(FlexItem(*editor).withFlex(1));
		}
	}

	fb.performLayout(getOverlayBounds().reduced(10).toFloat());
}

void ChannelStripComponent::initialiseGraph()
{
	m_mainProcessor->clear();
	m_mainProcessor->enableAllBuses();
	m_player.setProcessor(m_mainProcessor.get());
	m_mainProcessor->setPlayConfigDetails(1, 1, m_mainProcessor->getSampleRate(), m_mainProcessor->getBlockSize());

	createAudioNodes();
	connectAudioNodes();
}

void ChannelStripComponent::createAudioNodes()
{
	m_audioInputNode = m_mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioInputNode));

	Node::Ptr LPFnode = m_mainProcessor->addNode(std::make_unique<LPFilterProcessor>());
	if (LPFnode != nullptr)
	{
		addAndMakeVisible(LPFnode->getProcessor()->createEditorIfNeeded());
		LPFnode->getProcessor()->setPlayConfigDetails(
			m_mainProcessor->getNumInputChannels(),
			m_mainProcessor->getNumOutputChannels(),
			m_mainProcessor->getSampleRate(),
			m_mainProcessor->getBlockSize());
		LPFnode->getProcessor()->enableAllBuses();
	}
    
    Node::Ptr HPFnode = m_mainProcessor->addNode(std::make_unique<HPFilterProcessor>());
    if (HPFnode != nullptr)
    {
        addAndMakeVisible(HPFnode->getProcessor()->createEditorIfNeeded());
        HPFnode->getProcessor()->setPlayConfigDetails(
            m_mainProcessor->getNumInputChannels(),
            m_mainProcessor->getNumOutputChannels(),
            m_mainProcessor->getSampleRate(),
            m_mainProcessor->getBlockSize());
        HPFnode->getProcessor()->enableAllBuses();
    }

	Node::Ptr Gainnode = m_mainProcessor->addNode(std::make_unique<GainProcessor>());
	if (Gainnode != nullptr)
	{
		addAndMakeVisible(Gainnode->getProcessor()->createEditorIfNeeded());
		Gainnode->getProcessor()->setPlayConfigDetails(
			m_mainProcessor->getNumInputChannels(),
			m_mainProcessor->getNumOutputChannels(),
			m_mainProcessor->getSampleRate(),
			m_mainProcessor->getBlockSize());
		Gainnode->getProcessor()->enableAllBuses();
	}

	m_audioOutputNode = m_mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioOutputNode));
}

void ChannelStripComponent::connectAudioNodes()
{
	ReferenceCountedArray<Node> activeNodes = m_mainProcessor->getNodes();

	jassert(activeNodes.getFirst() == m_audioInputNode);	// We require an input
	jassert(activeNodes.getLast() == m_audioOutputNode);	// as well as an output
	jassert(activeNodes.size() == 5);						// and rely on having three nodes inbetween (HP, LP, Gain)
	if (activeNodes.size() == 5)
	{
		auto Inputnode = activeNodes.getUnchecked(0);
		auto HPFnode = activeNodes.getUnchecked(1);
		auto LPFnode = activeNodes.getUnchecked(2);
		auto Gainnode = activeNodes.getUnchecked(3);
		auto Outputnode = activeNodes.getUnchecked(4);

		for (int channel = 0; channel < m_mainProcessor->getMainBusNumInputChannels(); ++channel)
		{
			// feed the input to both hp and lp
			m_mainProcessor->addConnection({	{ Inputnode->nodeID,    channel },
												{ HPFnode->nodeID,		channel } });
			m_mainProcessor->addConnection({	{ Inputnode->nodeID,    channel },
												{ LPFnode->nodeID,		channel } });

			// feed both hp and lp to gain (sum up)
			m_mainProcessor->addConnection({	{ HPFnode->nodeID,		channel },
												{ Gainnode->nodeID,		channel } });
			m_mainProcessor->addConnection({	{ LPFnode->nodeID,		channel },
												{ Gainnode->nodeID,		channel } });

			// feed gain to output
			m_mainProcessor->addConnection({	{ Gainnode->nodeID,		channel },
												{ Outputnode->nodeID,	channel } });
		}
	}
	else
	{
		// if we do not have the expected 5 nodes, setup a dummy chain of nodes to at least have something
		for (int i = 0; i < activeNodes.size() - 1; ++i)
		{
			for (int channel = 0; channel < m_mainProcessor->getMainBusNumInputChannels(); ++channel)
				m_mainProcessor->addConnection({ { activeNodes.getUnchecked(i)->nodeID,      channel },
												{ activeNodes.getUnchecked(i + 1)->nodeID,  channel } });
		}
	}
}

void ChannelStripComponent::destroyAudioNodes()
{
	// rip up all node connections
	for (auto connection : m_mainProcessor->getConnections())
		m_mainProcessor->removeConnection(connection);

	// delete all editors
	for (auto& node : m_mainProcessor->getNodes())
	{
		auto editor = std::unique_ptr<AudioProcessorEditor>(node->getProcessor()->getActiveEditor());
		node->getProcessor()->editorBeingDeleted(editor.get());
	}
}

void ChannelStripComponent::audioDeviceIOCallback(const float** inputChannelData,
	int numInputChannels,
	float** outputChannelData,
	int numOutputChannels,
	int numSamples)
{
	m_player.audioDeviceIOCallback(inputChannelData, numInputChannels, outputChannelData, numOutputChannels, numSamples);
}

void ChannelStripComponent::audioDeviceAboutToStart(AudioIODevice* device)
{
	m_player.audioDeviceAboutToStart(device);
}

void ChannelStripComponent::audioDeviceStopped()
{
	m_player.audioDeviceStopped();
}

void ChannelStripComponent::audioDeviceError(const juce::String &errorMessage)
{
	m_player.audioDeviceError(errorMessage);
}

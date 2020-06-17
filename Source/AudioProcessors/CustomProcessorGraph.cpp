/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "CustomProcessorGraph.h"

ProcessorBase::ProcessorBase()
{
}

void ProcessorBase::prepareToPlay(double, int)
{
}

void ProcessorBase::releaseResources() 
{
}

void ProcessorBase::processBlock(AudioSampleBuffer&, MidiBuffer&) 
{
}

AudioProcessorEditor* ProcessorBase::createEditor() 
{
	return nullptr; 
}

bool ProcessorBase::hasEditor() const 
{
	return false; 
}

const String ProcessorBase::getName() const 
{
	return {}; 
}

bool ProcessorBase::acceptsMidi() const 
{
	return false; 
}

bool ProcessorBase::producesMidi() const 
{
	return false; 
}

double ProcessorBase::getTailLengthSeconds() const 
{
	return 0; 
}

int ProcessorBase::getNumPrograms() 
{ 
	return 0; 
}

int ProcessorBase::getCurrentProgram() 
{ 
	return 0; 
}

void ProcessorBase::setCurrentProgram(int) 
{
}

const String ProcessorBase::getProgramName(int) 
{ 
	return {}; 
}

void ProcessorBase::changeProgramName(int, const String&) 
{
}

void ProcessorBase::getStateInformation(MemoryBlock&) 
{
}

void ProcessorBase::setStateInformation(const void*, int)
{
}


OscillatorProcessor30Hz::OscillatorProcessor30Hz()
{
	oscillator.setFrequency(30.0f);
	oscillator.initialise([](float x) { return std::sin(x); });
}

void OscillatorProcessor30Hz::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock) };
	oscillator.prepare(spec);
}

void OscillatorProcessor30Hz::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	oscillator.process(context);
}

void OscillatorProcessor30Hz::reset()
{
	oscillator.reset();
}

const String OscillatorProcessor30Hz::getName() const 
{ 
	return "30HzOscillator"; 
}


OscillatorProcessor440Hz::OscillatorProcessor440Hz()
{
	oscillator.setFrequency(440.0f);
	oscillator.initialise([](float x) { return std::sin(x); });
}

void OscillatorProcessor440Hz::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock) };
	oscillator.prepare(spec);
}

void OscillatorProcessor440Hz::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	oscillator.process(context);
}

void OscillatorProcessor440Hz::reset()
{
	oscillator.reset();
}

const String OscillatorProcessor440Hz::getName() const 
{ 
	return "440HzOscillator"; 
}


OscillatorProcessor2kHz::OscillatorProcessor2kHz()
{
	oscillator.setFrequency(2000.0f);
	oscillator.initialise([](float x) { return std::sin(x); });
}

void OscillatorProcessor2kHz::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock) };
	oscillator.prepare(spec);
}

void OscillatorProcessor2kHz::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	oscillator.process(context);
}

void OscillatorProcessor2kHz::reset()
{
	oscillator.reset();
}

const String OscillatorProcessor2kHz::getName() const 
{ 
	return "2kHzOscillator"; 
}


GainProcessor::GainProcessor()
{
	gain.setGainDecibels(-6.0f);
}

void GainProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
	gain.prepare(spec);
}

void GainProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	gain.process(context);
}

void GainProcessor::reset()
{
	gain.reset();
}

const String GainProcessor::getName() const { return "Gain"; }


HPFilterProcessor::HPFilterProcessor() {}

void HPFilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	*filter.state = *dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 1000.0f);

	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
	filter.prepare(spec);
}

void HPFilterProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	filter.process(context);
}

void HPFilterProcessor::reset()
{
	filter.reset();
}

const String HPFilterProcessor::getName() const
{ 
	return "HPFilter"; 
}


LPFilterProcessor::LPFilterProcessor()
{
}

void LPFilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	*filter.state = *dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 1000.0f);

	dsp::ProcessSpec spec{ sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
	filter.prepare(spec);
}

void LPFilterProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	filter.process(context);
}

void LPFilterProcessor::reset()
{
	filter.reset();
}

const String LPFilterProcessor::getName() const 
{ 
	return "LPFilter"; 
}


CustomProcessorGraph::CustomProcessorGraph()
	: mainProcessor(new AudioProcessorGraph())
{
	addAndMakeVisible(processorSlot1);
	addAndMakeVisible(processorSlot2);
	addAndMakeVisible(processorSlot3);
	addAndMakeVisible(processorSlot4);

	processorSlot1.addItemList(processorChoices, 1);
	processorSlot2.addItemList(processorChoices, 1);
	processorSlot3.addItemList(processorChoices, 1);
	processorSlot4.addItemList(processorChoices, 1);

	addAndMakeVisible(labelSlot1);
	addAndMakeVisible(labelSlot2);
	addAndMakeVisible(labelSlot3);
	addAndMakeVisible(labelSlot4);

	labelSlot1.attachToComponent(&processorSlot1, true);
	labelSlot2.attachToComponent(&processorSlot2, true);
	labelSlot3.attachToComponent(&processorSlot3, true);
	labelSlot4.attachToComponent(&processorSlot4, true);

	addAndMakeVisible(bypassSlot1);
	addAndMakeVisible(bypassSlot2);
	addAndMakeVisible(bypassSlot3);
	addAndMakeVisible(bypassSlot4);

	auto inputDevice = MidiInput::getDefaultDevice();
	auto outputDevice = MidiOutput::getDefaultDevice();

	mainProcessor->enableAllBuses();

	deviceManager.initialiseWithDefaultDevices(0, 2);
	deviceManager.addAudioCallback(&player);
	deviceManager.setMidiInputDeviceEnabled(inputDevice.identifier, true);
	deviceManager.addMidiInputDeviceCallback(inputDevice.identifier, &player);
	deviceManager.setDefaultMidiOutputDevice(outputDevice.identifier);

	initialiseGraph();

	player.setProcessor(mainProcessor.get());

	setSize(600, 460);
	startTimer(100);
}

CustomProcessorGraph::~CustomProcessorGraph()
{
	auto device = MidiInput::getDefaultDevice();

	deviceManager.removeAudioCallback(&player);
	deviceManager.setMidiInputDeviceEnabled(device.identifier, false);
	deviceManager.removeMidiInputDeviceCallback(device.identifier, &player);
}

void CustomProcessorGraph::paint(Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void CustomProcessorGraph::resized()
{
	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.justifyContent = FlexBox::JustifyContent::center;
	fb.alignContent = FlexBox::AlignContent::center;

	auto width = getWidth() / 2.0f;
	auto height = getHeight() / 8.0f;

	FlexItem slot1(width, height, processorSlot1);
	FlexItem slot2(width, height, processorSlot2);
	FlexItem slot3(width, height, processorSlot3);
	FlexItem slot4(width, height, processorSlot4);

	FlexItem bypass1(width, height, bypassSlot1);
	FlexItem bypass2(width, height, bypassSlot2);
	FlexItem bypass3(width, height, bypassSlot3);
	FlexItem bypass4(width, height, bypassSlot4);

	fb.items.addArray({ slot1, bypass1, slot2, bypass2, slot3, bypass3, slot4, bypass4 });
	fb.performLayout(getLocalBounds().toFloat());
}

void CustomProcessorGraph::initialiseGraph()
{
	mainProcessor->clear();

	audioInputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioInputNode));
	audioOutputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioOutputNode));
	midiInputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiInputNode));
	midiOutputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiOutputNode));

	connectAudioNodes();
	connectMidiNodes();
}

void CustomProcessorGraph::timerCallback() { updateGraph(); }

void CustomProcessorGraph::updateGraph()
{
	bool hasChanged = false;

	Array<ComboBox*> choices{ &processorSlot1,
							   &processorSlot2,
							   &processorSlot3,
							   &processorSlot4 };

	Array<ToggleButton*> bypasses{ &bypassSlot1,
									&bypassSlot2,
									&bypassSlot3,
									&bypassSlot4 };

	ReferenceCountedArray<Node> slots;
	slots.add(slot1Node);
	slots.add(slot2Node);
	slots.add(slot3Node);
	slots.add(slot4Node);

	for (int i = 0; i < 4; ++i)
	{
		auto& choice = choices.getReference(i);
		auto  slot = slots.getUnchecked(i);

		if (choice->getSelectedId() == 0)
		{
			if (slot != nullptr)
			{
				mainProcessor->removeNode(slot.get());
				slots.set(i, nullptr);
				hasChanged = true;
			}
		}
		else if (choice->getSelectedId() == 1)
		{
			if (slot != nullptr)
			{
				if (slot->getProcessor()->getName() == "30HzOscillator")
					continue;

				mainProcessor->removeNode(slot.get());
			}

			slots.set(i, mainProcessor->addNode(std::make_unique<OscillatorProcessor30Hz>()));
			hasChanged = true;
		}
		else if (choice->getSelectedId() == 2)
		{
			if (slot != nullptr)
			{
				if (slot->getProcessor()->getName() == "440HzOscillator")
					continue;

				mainProcessor->removeNode(slot.get());
			}

			slots.set(i, mainProcessor->addNode(std::make_unique<OscillatorProcessor440Hz>()));
			hasChanged = true;
		}
		else if (choice->getSelectedId() == 3)
		{
			if (slot != nullptr)
			{
				if (slot->getProcessor()->getName() == "2kHzOscillator")
					continue;

				mainProcessor->removeNode(slot.get());
			}

			slots.set(i, mainProcessor->addNode(std::make_unique<OscillatorProcessor2kHz>()));
			hasChanged = true;
		}
		else if (choice->getSelectedId() == 4)
		{
			if (slot != nullptr)
			{
				if (slot->getProcessor()->getName() == "Gain")
					continue;

				mainProcessor->removeNode(slot.get());
			}

			slots.set(i, mainProcessor->addNode(std::make_unique<GainProcessor>()));
			hasChanged = true;
		}
		else if (choice->getSelectedId() == 5)
		{
			if (slot != nullptr)
			{
				if (slot->getProcessor()->getName() == "HPFilter")
					continue;

				mainProcessor->removeNode(slot.get());
			}

			slots.set(i, mainProcessor->addNode(std::make_unique<HPFilterProcessor>()));
			hasChanged = true;
		}
		else if (choice->getSelectedId() == 6)
		{
			if (slot != nullptr)
			{
				if (slot->getProcessor()->getName() == "LPFilter")
					continue;

				mainProcessor->removeNode(slot.get());
			}

			slots.set(i, mainProcessor->addNode(std::make_unique<LPFilterProcessor>()));
			hasChanged = true;
		}
	}

	if (hasChanged)
	{
		for (auto connection : mainProcessor->getConnections())
			mainProcessor->removeConnection(connection);

		ReferenceCountedArray<Node> activeSlots;

		for (auto slot : slots)
		{
			if (slot != nullptr)
			{
				activeSlots.add(slot);

				slot->getProcessor()->setPlayConfigDetails(mainProcessor->getMainBusNumInputChannels(),
					mainProcessor->getMainBusNumOutputChannels(),
					mainProcessor->getSampleRate(),
					mainProcessor->getBlockSize());
			}
		}

		if (activeSlots.isEmpty())
		{
			connectAudioNodes();
		}
		else
		{
			for (int i = 0; i < activeSlots.size() - 1; ++i)
			{
				for (int channel = 0; channel < 2; ++channel)
					mainProcessor->addConnection({ { activeSlots.getUnchecked(i)->nodeID,      channel },
													{ activeSlots.getUnchecked(i + 1)->nodeID,  channel } });
			}

			for (int channel = 0; channel < 2; ++channel)
			{
				mainProcessor->addConnection({ { audioInputNode->nodeID,         channel },
												{ activeSlots.getFirst()->nodeID, channel } });
				mainProcessor->addConnection({ { activeSlots.getLast()->nodeID,  channel },
												{ audioOutputNode->nodeID,        channel } });
			}
		}

		connectMidiNodes();

		for (auto node : mainProcessor->getNodes())
			node->getProcessor()->enableAllBuses();
	}

	for (int i = 0; i < 4; ++i)
	{
		auto  slot = slots.getUnchecked(i);
		auto& bypass = bypasses.getReference(i);

		if (slot != nullptr)
			slot->setBypassed(bypass->getToggleState());
	}

	slot1Node = slots.getUnchecked(0);
	slot2Node = slots.getUnchecked(1);
	slot3Node = slots.getUnchecked(2);
	slot4Node = slots.getUnchecked(3);
}

void CustomProcessorGraph::connectAudioNodes()
{
	for (int channel = 0; channel < 2; ++channel)
		mainProcessor->addConnection({ { audioInputNode->nodeID,  channel },
										{ audioOutputNode->nodeID, channel } });
}

void CustomProcessorGraph::connectMidiNodes()
{
	mainProcessor->addConnection({ { midiInputNode->nodeID,  AudioProcessorGraph::midiChannelIndex },
									{ midiOutputNode->nodeID, AudioProcessorGraph::midiChannelIndex } });
}


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

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             AudioProcessorGraphTutorial
 version:          2.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Explores the audio processor graph.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_dsp, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make

 type:             Component
 mainClass:        MainComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once

//==============================================================================
class ProcessorBase  : public AudioProcessor
{
public:
    //==============================================================================
    ProcessorBase()  {}

    //==============================================================================
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processBlock (AudioSampleBuffer&, MidiBuffer&) override {}

    //==============================================================================
    AudioProcessorEditor* createEditor() override          { return nullptr; }
    bool hasEditor() const override                        { return false; }

    //==============================================================================
    const String getName() const override                  { return {}; }
    bool acceptsMidi() const override                      { return false; }
    bool producesMidi() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0; }

    //==============================================================================
    int getNumPrograms() override                          { return 0; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const String getProgramName (int) override             { return {}; }
    void changeProgramName (int, const String&) override   {}

    //==============================================================================
    void getStateInformation (MemoryBlock&) override       {}
    void setStateInformation (const void*, int) override   {}

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorBase)
};

//==============================================================================
class OscillatorProcessor  : public ProcessorBase
{
public:
    OscillatorProcessor()
    {
        oscillator.setFrequency (440.0f);
        oscillator.initialise ([] (float x) { return std::sin (x); });
    }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        dsp::ProcessSpec spec { sampleRate, static_cast<uint32> (samplesPerBlock) };
        oscillator.prepare (spec);
    }

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override
    {
        dsp::AudioBlock<float> block (buffer);
        dsp::ProcessContextReplacing<float> context (block);
        oscillator.process (context);
    }

    void reset() override
    {
        oscillator.reset();
    }

    const String getName() const override { return "Oscillator"; }

private:
    dsp::Oscillator<float> oscillator;
};

//==============================================================================
class GainProcessor  : public ProcessorBase
{
public:
    GainProcessor()
    {
        gain.setGainDecibels (-6.0f);
    }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        dsp::ProcessSpec spec { sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
        gain.prepare (spec);
    }

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override
    {
        dsp::AudioBlock<float> block (buffer);
        dsp::ProcessContextReplacing<float> context (block);
        gain.process (context);
    }

    void reset() override
    {
        gain.reset();
    }

    const String getName() const override { return "Gain"; }

private:
    dsp::Gain<float> gain;
};

//==============================================================================
class FilterProcessor  : public ProcessorBase
{
public:
    FilterProcessor() {}

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        *filter.state = *dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 1000.0f);

        dsp::ProcessSpec spec { sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
        filter.prepare (spec);
    }

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override
    {
        dsp::AudioBlock<float> block (buffer);
        dsp::ProcessContextReplacing<float> context (block);
        filter.process (context);
    }

    void reset() override
    {
        filter.reset();
    }

    const String getName() const override { return "Filter"; }

private:
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> filter;
};

//==============================================================================
class MainComponent  : public Component,
                       private Timer
{
public:
    //==============================================================================
    using AudioGraphIOProcessor = AudioProcessorGraph::AudioGraphIOProcessor;
    using Node = AudioProcessorGraph::Node;

    //==============================================================================
    MainComponent()
        : mainProcessor (new AudioProcessorGraph())
    {
        addAndMakeVisible (muteInput);
        muteInput.setToggleState (true, dontSendNotification);

        addAndMakeVisible (processorSlot1);
        addAndMakeVisible (processorSlot2);
        addAndMakeVisible (processorSlot3);

        processorSlot1.addItemList (processorChoices, 1);
        processorSlot2.addItemList (processorChoices, 1);
        processorSlot3.addItemList (processorChoices, 1);

        addAndMakeVisible (labelSlot1);
        addAndMakeVisible (labelSlot2);
        addAndMakeVisible (labelSlot3);

        labelSlot1.attachToComponent (&processorSlot1, true);
        labelSlot2.attachToComponent (&processorSlot2, true);
        labelSlot3.attachToComponent (&processorSlot3, true);

        addAndMakeVisible (bypassSlot1);
        addAndMakeVisible (bypassSlot2);
        addAndMakeVisible (bypassSlot3);

        auto inputDevice  = MidiInput::getDefaultDevice();
        auto outputDevice = MidiOutput::getDefaultDevice();

        mainProcessor->enableAllBuses();

        deviceManager.initialiseWithDefaultDevices (2, 2);
        deviceManager.addAudioCallback (&player);
        deviceManager.setMidiInputDeviceEnabled (inputDevice.identifier, true);
        deviceManager.addMidiInputDeviceCallback (inputDevice.identifier, &player);
        deviceManager.setDefaultMidiOutputDevice (outputDevice.identifier);

        initialiseGraph();

        player.setProcessor (mainProcessor.get());

        setSize (600, 400);
        startTimer (100);
    }

    ~MainComponent() override
    {
        auto device = MidiInput::getDefaultDevice();

        deviceManager.removeAudioCallback (&player);
        deviceManager.setMidiInputDeviceEnabled (device.identifier, false);
        deviceManager.removeMidiInputDeviceCallback (device.identifier, &player);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        FlexBox fb;
        fb.flexDirection  = FlexBox::Direction::column;
        fb.justifyContent = FlexBox::JustifyContent::center;
        fb.alignContent   = FlexBox::AlignContent::center;

        auto width  = getWidth()  / 2.0f;
        auto height = getHeight() / 7.0f;

        FlexItem mute (width, height, muteInput);

        FlexItem slot1 (width, height, processorSlot1);
        FlexItem slot2 (width, height, processorSlot2);
        FlexItem slot3 (width, height, processorSlot3);

        FlexItem bypass1 (width, height, bypassSlot1);
        FlexItem bypass2 (width, height, bypassSlot2);
        FlexItem bypass3 (width, height, bypassSlot3);

        fb.items.addArray ({ mute, slot1, slot2, slot3, bypass1, bypass2, bypass3 });
        fb.performLayout (getLocalBounds().toFloat());
    }

private:
    //==============================================================================
    void initialiseGraph()
    {
        mainProcessor->clear();

        audioInputNode  = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::audioInputNode));
        audioOutputNode = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::audioOutputNode));
        midiInputNode   = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::midiInputNode));
        midiOutputNode  = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::midiOutputNode));

        connectAudioNodes();
        connectMidiNodes();
    }

    void timerCallback() override { updateGraph(); }

    void updateGraph()
    {
        bool hasChanged = false;

        Array<ComboBox*> choices { &processorSlot1,
                                   &processorSlot2,
                                   &processorSlot3 };

        Array<ToggleButton*> bypasses { &bypassSlot1,
                                        &bypassSlot2,
                                        &bypassSlot3 };

        ReferenceCountedArray<Node> slots;
        slots.add (slot1Node);
        slots.add (slot2Node);
        slots.add (slot3Node);

        for (int i = 0; i < 3; ++i)
        {
            auto& choice = choices.getReference (i);
            auto  slot   = slots  .getUnchecked (i);

            if (choice->getSelectedId() == 0)
            {
                if (slot != nullptr)
                {
                    mainProcessor->removeNode (slot.get());
                    slots.set (i, nullptr);
                    hasChanged = true;
                }
            }
            else if (choice->getSelectedId() == 1)
            {
                if (slot != nullptr)
                {
                    if (slot->getProcessor()->getName() == "Oscillator")
                        continue;

                    mainProcessor->removeNode (slot.get());
                }

                slots.set (i, mainProcessor->addNode (std::make_unique<OscillatorProcessor>()));
                hasChanged = true;
            }
            else if (choice->getSelectedId() == 2)
            {
                if (slot != nullptr)
                {
                    if (slot->getProcessor()->getName() == "Gain")
                        continue;

                    mainProcessor->removeNode (slot.get());
                }

                slots.set (i, mainProcessor->addNode (std::make_unique<GainProcessor>()));
                hasChanged = true;
            }
            else if (choice->getSelectedId() == 3)
            {
                if (slot != nullptr)
                {
                    if (slot->getProcessor()->getName() == "Filter")
                        continue;

                    mainProcessor->removeNode (slot.get());
                }

                slots.set (i, mainProcessor->addNode (std::make_unique<FilterProcessor>()));
                hasChanged = true;
            }
        }

        if (hasChanged)
        {
            for (auto connection : mainProcessor->getConnections())
                mainProcessor->removeConnection (connection);

            ReferenceCountedArray<Node> activeSlots;

            for (auto slot : slots)
            {
                if (slot != nullptr)
                {
                    activeSlots.add (slot);

                    slot->getProcessor()->setPlayConfigDetails (mainProcessor->getMainBusNumInputChannels(),
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
                        mainProcessor->addConnection ({ { activeSlots.getUnchecked (i)->nodeID,      channel },
                                                        { activeSlots.getUnchecked (i + 1)->nodeID,  channel } });
                }

                for (int channel = 0; channel < 2; ++channel)
                {
                    mainProcessor->addConnection ({ { audioInputNode->nodeID,         channel },
                                                    { activeSlots.getFirst()->nodeID, channel } });
                    mainProcessor->addConnection ({ { activeSlots.getLast()->nodeID,  channel },
                                                    { audioOutputNode->nodeID,        channel } });
                }
            }

            connectMidiNodes();

            for (auto node : mainProcessor->getNodes())
                node->getProcessor()->enableAllBuses();
        }

        for (int i = 0; i < 3; ++i)
        {
            auto  slot   = slots   .getUnchecked (i);
            auto& bypass = bypasses.getReference (i);

            if (slot != nullptr)
                slot->setBypassed (bypass->getToggleState());
        }

        audioInputNode->setBypassed (muteInput.getToggleState());

        slot1Node = slots.getUnchecked (0);
        slot2Node = slots.getUnchecked (1);
        slot3Node = slots.getUnchecked (2);
    }

    void connectAudioNodes()
    {
        for (int channel = 0; channel < 2; ++channel)
            mainProcessor->addConnection ({ { audioInputNode->nodeID,  channel },
                                            { audioOutputNode->nodeID, channel } });
    }

    void connectMidiNodes()
    {
        mainProcessor->addConnection ({ { midiInputNode->nodeID,  AudioProcessorGraph::midiChannelIndex },
                                        { midiOutputNode->nodeID, AudioProcessorGraph::midiChannelIndex } });
    }

    //==============================================================================
    StringArray processorChoices { "Oscillator", "Gain", "Filter" };

    ToggleButton muteInput { "Mute Input" };

    Label labelSlot1 { {}, { "Slot 1" } };
    Label labelSlot2 { {}, { "Slot 2" } };
    Label labelSlot3 { {}, { "Slot 3" } };

    ComboBox processorSlot1;
    ComboBox processorSlot2;
    ComboBox processorSlot3;

    ToggleButton bypassSlot1 { "Bypass 1" };
    ToggleButton bypassSlot2 { "Bypass 2" };
    ToggleButton bypassSlot3 { "Bypass 3" };

    std::unique_ptr<AudioProcessorGraph> mainProcessor;

    Node::Ptr audioInputNode;
    Node::Ptr audioOutputNode;
    Node::Ptr midiInputNode;
    Node::Ptr midiOutputNode;

    Node::Ptr slot1Node;
    Node::Ptr slot2Node;
    Node::Ptr slot3Node;

    AudioDeviceManager deviceManager;
    AudioProcessorPlayer player;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

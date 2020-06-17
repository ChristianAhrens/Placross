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

#include <JuceHeader.h>


//==============================================================================
class ProcessorBase  : public AudioProcessor
{
public:
    //==============================================================================
    ProcessorBase();

    //==============================================================================
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    void processBlock(AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const String getProgramName(int) override;
    void changeProgramName(int, const String&) override;

    //==============================================================================
    void getStateInformation(MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorBase)
};

//==============================================================================
class OscillatorProcessor30Hz  : public ProcessorBase
{
public:
    OscillatorProcessor30Hz();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override;

    void reset() override;

    const String getName() const override;

private:
    dsp::Oscillator<float> oscillator;
};

//==============================================================================
class OscillatorProcessor440Hz : public ProcessorBase
{
public:
    OscillatorProcessor440Hz();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void processBlock(AudioSampleBuffer& buffer, MidiBuffer&) override;

    void reset() override;

    const String getName() const override;

private:
    dsp::Oscillator<float> oscillator;
};

//==============================================================================
class OscillatorProcessor2kHz : public ProcessorBase
{
public:
    OscillatorProcessor2kHz();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void processBlock(AudioSampleBuffer& buffer, MidiBuffer&) override;

    void reset() override;

    const String getName() const override;

private:
    dsp::Oscillator<float> oscillator;
};

//==============================================================================
class GainProcessor  : public ProcessorBase
{
public:
    GainProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override;

    void reset() override;

    const String getName() const override;

private:
    dsp::Gain<float> gain;
};

//==============================================================================
class HPFilterProcessor  : public ProcessorBase
{
public:
    HPFilterProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override;

    void reset() override;

    const String getName() const override;

private:
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> filter;
};

//==============================================================================
class LPFilterProcessor : public ProcessorBase
{
public:
    LPFilterProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void processBlock(AudioSampleBuffer& buffer, MidiBuffer&) override;

    void reset() override;

    const String getName() const override;

private:
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> filter;
};

//==============================================================================
class CustomProcessorGraph  : public Component,
                       private Timer
{
public:
    //==============================================================================
    using AudioGraphIOProcessor = AudioProcessorGraph::AudioGraphIOProcessor;
    using Node = AudioProcessorGraph::Node;

    //==============================================================================
    CustomProcessorGraph();

    ~CustomProcessorGraph() override;

    //==============================================================================
    void paint (Graphics& g) override;

    void resized() override;

private:
    //==============================================================================
    void initialiseGraph();

    void timerCallback() override;

    void updateGraph();

    void connectAudioNodes();

    void connectMidiNodes();

    //==============================================================================
    StringArray processorChoices { "30HzOscillator", "440HzOscillator", "2kHzOscillator", "Gain", "HPFilter", "LPFilter" };

    Label labelSlot1 { {}, { "Slot 1" } };
    Label labelSlot2 { {}, { "Slot 2" } };
    Label labelSlot3 { {}, { "Slot 3" } };
    Label labelSlot4 { {}, { "Slot 4" } };

    ComboBox processorSlot1;
    ComboBox processorSlot2;
    ComboBox processorSlot3;
    ComboBox processorSlot4;

    ToggleButton bypassSlot1 { "Bypass 1" };
    ToggleButton bypassSlot2 { "Bypass 2" };
    ToggleButton bypassSlot3 { "Bypass 3" };
    ToggleButton bypassSlot4 { "Bypass 4" };

    std::unique_ptr<AudioProcessorGraph> mainProcessor;

    Node::Ptr audioInputNode;
    Node::Ptr audioOutputNode;
    Node::Ptr midiInputNode;
    Node::Ptr midiOutputNode;

    Node::Ptr slot1Node;
    Node::Ptr slot2Node;
    Node::Ptr slot3Node;
    Node::Ptr slot4Node;

    AudioDeviceManager deviceManager;
    AudioProcessorPlayer player;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomProcessorGraph)
};

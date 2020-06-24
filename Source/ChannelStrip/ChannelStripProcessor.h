/*
  ==============================================================================

    ChannelStripProcessor.h
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

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

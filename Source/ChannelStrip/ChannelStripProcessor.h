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

    //==============================================================================
    juce::Range<double> getParameterRange(int parameterIndex);
    double getParameterStepWidth(int parameterIndex);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorBase)
};

//==============================================================================
class GainProcessor  : public ProcessorBase
{
public:
    GainProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override;
    void reset() override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

private:
    AudioParameterFloat *m_gainValue;
    dsp::Gain<float> m_gain;
};

//==============================================================================
class HPFilterProcessor  : public ProcessorBase
{
public:
    HPFilterProcessor();

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(AudioSampleBuffer& buffer, MidiBuffer&) override;
    void reset() override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

private:
    AudioParameterFloat* m_filterValue;
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> m_filter;
};

//==============================================================================
class LPFilterProcessor : public ProcessorBase
{
public:
    LPFilterProcessor();

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(AudioSampleBuffer& buffer, MidiBuffer&) override;
    void reset() override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

private:
    AudioParameterFloat *m_filterValue;
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> m_filter;
};

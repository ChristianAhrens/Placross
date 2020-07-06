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
class ProcessorBase  : public AudioProcessor, public AudioProcessorParameter::Listener
{
public:
    struct ProcessorParam
    {
        ProcessorParam(String i, String n, float min, float max, float interval, float skew, float def)
        {
            id = i;
            name = n;
            minV = min;
            maxV = max;
            intervalV = interval;
            skewV = skew;
            defaultV = def;
        }

        String id;      // parameter ID
        String name;    // parameter name
        float minV;     // minimum value
        float maxV;     // maximum value
        float intervalV;// value interval
        float skewV;    // value skew
        float defaultV; // default value
    };
    virtual std::vector<ProcessorBase::ProcessorParam> getProcessorParams() = 0;

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
    void initParameters();
    virtual void updateParameterValues() = 0;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    virtual void parameterValueChanged(int parameterIndex, float newValue) override = 0;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    static float getMappedValue(AudioProcessorParameter* param);
    static float getNormalizedValue(AudioProcessorParameter* param);

protected:
    std::map<String, int> m_IdToIdxMap;

    double m_sampleRate{ 48000.0f };
    int m_samplesPerBlock{ 480 };

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
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override;
    void reset() override;

    //==============================================================================
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void updateParameterValues() override;

    const String getName() const override;

    std::vector<ProcessorBase::ProcessorParam> getProcessorParams() override;

private:
    dsp::Gain<float> m_gain;
};

//==============================================================================
class HPFilterProcessor  : public ProcessorBase
{
public:
    HPFilterProcessor();

    //==============================================================================
    void processBlock(AudioSampleBuffer& buffer, MidiBuffer&) override;
    void reset() override;

    //==============================================================================
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void updateParameterValues() override;

    const String getName() const override;

    std::vector<ProcessorBase::ProcessorParam> getProcessorParams() override;

private:
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> m_filter;
};

//==============================================================================
class LPFilterProcessor : public ProcessorBase
{
public:
    LPFilterProcessor();

    //==============================================================================
    void processBlock(AudioSampleBuffer& buffer, MidiBuffer&) override;
    void reset() override;

    //==============================================================================
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void updateParameterValues() override;

    const String getName() const override;

    std::vector<ProcessorBase::ProcessorParam> getProcessorParams() override;

private:
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> m_filter;
};

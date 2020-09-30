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
class ChannelStripProcessorBase  : public AudioProcessor, public AudioProcessorParameter::Listener
{
public:
    enum ChannelStripProcessorType
    {
        CSPT_LowPass,
        CSPT_HighPass,
        CSPT_Gain,
        CSPT_Invalid
    };

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
    virtual std::vector<ChannelStripProcessorBase::ProcessorParam> getProcessorParams() = 0;

public:
    //==============================================================================
    ChannelStripProcessorBase();

    void initParameters();

    //==============================================================================
    virtual ChannelStripProcessorType getType() = 0;
    virtual void updateParameterValues() = 0;
    virtual float getFilterFequency() = 0;
    virtual float getFilterGain() = 0;
    virtual float getMagnitudeResponse(float freq) = 0;

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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelStripProcessorBase)
};

//==============================================================================
class GainProcessor  : public ChannelStripProcessorBase
{
public:
    GainProcessor();
    
    ChannelStripProcessorType getType() override;
    float getMagnitudeResponse(float freq) override;
    float getFilterFequency() override;
    float getFilterGain() override;

    //==============================================================================
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override;
    void reset() override;

    //==============================================================================
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void updateParameterValues() override;

    const String getName() const override;

    std::vector<ChannelStripProcessorBase::ProcessorParam> getProcessorParams() override;

private:
    dsp::Gain<float> m_gain;
};

//==============================================================================
class HPFilterProcessor  : public ChannelStripProcessorBase
{
public:
    HPFilterProcessor();

    ChannelStripProcessorType getType() override;
    float getMagnitudeResponse(float freq) override;
    float getFilterFequency() override;
    float getFilterGain() override;

    //==============================================================================
    void processBlock(AudioSampleBuffer& buffer, MidiBuffer&) override;
    void reset() override;

    //==============================================================================
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void updateParameterValues() override;

    const String getName() const override;

    std::vector<ChannelStripProcessorBase::ProcessorParam> getProcessorParams() override;

private:
    dsp::StateVariableTPTFilter<float> m_filter;
    dsp::Gain<float> m_gain;
};

//==============================================================================
class LPFilterProcessor : public ChannelStripProcessorBase
{
public:
    LPFilterProcessor();

    ChannelStripProcessorType getType() override;
    float getMagnitudeResponse(float freq) override;
    float getFilterFequency() override;
    float getFilterGain() override;

    //==============================================================================
    void processBlock(AudioSampleBuffer& buffer, MidiBuffer&) override;
    void reset() override;

    //==============================================================================
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void updateParameterValues() override;

    const String getName() const override;

    std::vector<ChannelStripProcessorBase::ProcessorParam> getProcessorParams() override;

private:
    dsp::StateVariableTPTFilter<float> m_filter;
    dsp::Gain<float> m_gain;
};

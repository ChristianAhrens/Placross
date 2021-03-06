/*
  ==============================================================================

    ChannelStripProcessor.cpp
    Created: 24 Jun 2020 11:52:17am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "ChannelStripProcessor.h"
#include "ChannelStripProcessorEditor.h"

static void DBG_IF_DEBUG(String s)
{
#ifdef DEBUG
	DBG(s);
#endif
}

ChannelStripProcessorBase::ChannelStripProcessorBase()
{
}

void ChannelStripProcessorBase::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	m_sampleRate = sampleRate;
	m_samplesPerBlock = samplesPerBlock;

	updateParameterValues();
}

void ChannelStripProcessorBase::releaseResources() 
{
}

void ChannelStripProcessorBase::processBlock(AudioSampleBuffer&, MidiBuffer&) 
{
}

const String ChannelStripProcessorBase::getName() const 
{
	return {}; 
}

bool ChannelStripProcessorBase::acceptsMidi() const 
{
	return false; 
}

bool ChannelStripProcessorBase::producesMidi() const 
{
	return false; 
}

double ChannelStripProcessorBase::getTailLengthSeconds() const 
{
	return 0; 
}

int ChannelStripProcessorBase::getNumPrograms() 
{ 
	return 0; 
}

int ChannelStripProcessorBase::getCurrentProgram() 
{ 
	return 0; 
}

void ChannelStripProcessorBase::setCurrentProgram(int) 
{
}

const String ChannelStripProcessorBase::getProgramName(int) 
{ 
	return {}; 
}

void ChannelStripProcessorBase::changeProgramName(int, const String&) 
{
}

void ChannelStripProcessorBase::getStateInformation(MemoryBlock&) 
{
}

void ChannelStripProcessorBase::setStateInformation(const void*, int)
{
}

void ChannelStripProcessorBase::initParameters()
{
	for (auto param : getProcessorParams())
	{
		auto newAPF = new AudioParameterFloat(
			param.id,
			param.name,
			juce::NormalisableRange<float>(param.minV, param.maxV, param.intervalV, param.skewV),
			param.defaultV);
		newAPF->setValueNotifyingHost(getNormalizedValue(newAPF));
		newAPF->addListener(this);
		addParameter(newAPF);
		m_IdToIdxMap.insert(std::make_pair(param.id, newAPF->getParameterIndex()));
	}
}

void ChannelStripProcessorBase::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
	ignoreUnused(parameterIndex);
	ignoreUnused(gestureIsStarting);
}

AudioProcessorEditor* ChannelStripProcessorBase::createEditor()
{
	auto editor = std::make_unique<ChannelStripProcessorEditor>(*this);
	editor->setSize(50, 60);

	return editor.release();
}

bool ChannelStripProcessorBase::hasEditor() const
{
	return true;
}

float ChannelStripProcessorBase::getMappedValue(AudioProcessorParameter* param)
{
	auto floatParam = dynamic_cast<AudioParameterFloat*>(param);
	if (floatParam)
	{
		// We expect the current value to be a normalized one that shall be mapped 
		// to a range of real values (e.g. given normalized 0...1 mapped to real 20Hz...20kHz)
		float val = *floatParam;
		auto minVal = floatParam->getNormalisableRange().getRange().getStart();
		auto maxVal = floatParam->getNormalisableRange().getRange().getEnd();

		return jmap(jlimit(0.0f, 1.0f, val), minVal, maxVal);
	}
	else
		return 0.0f;
}

float ChannelStripProcessorBase::getNormalizedValue(AudioProcessorParameter* param)
{
	auto floatParam = dynamic_cast<AudioParameterFloat*>(param);
	if (floatParam)
	{
		// We expect the current value to be within a range of real-life values that shall be mapped 
		// to normlized range 0..1 (e.g. given real 20Hz...20kHz to normalized 0...1)
		float val = *floatParam;
		auto minVal = floatParam->getNormalisableRange().getRange().getStart();
		auto maxVal = floatParam->getNormalisableRange().getRange().getEnd();

		return jmap(jlimit(minVal, maxVal, val), minVal, maxVal, 0.0f, 1.0f);
	}
	else
		return 0.0f;
}


GainProcessor::GainProcessor()
	: ChannelStripProcessorBase()
{
	initParameters();
}

ChannelStripProcessorBase::ChannelStripProcessorType GainProcessor::getType()
{
	return ChannelStripProcessorBase::CSPT_Gain;
}

float GainProcessor::getMagnitudeResponse(float freq)
{
	return 1.0f;
}

float GainProcessor::getFilterFequency()
{
	return -1.0f;
}

float GainProcessor::getFilterGain()
{
	return 1.0f;
}

std::vector<ChannelStripProcessorBase::ProcessorParam> GainProcessor::getProcessorParams()
{
	return std::vector<ChannelStripProcessorBase::ProcessorParam>{ { "gain", "Gain", 0.0f, 1.0f, 0.01f, 1.0f, 1.0f } };
}

void GainProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	m_gain.process(context);
}

void GainProcessor::reset()
{
	m_gain.reset();
}

void GainProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
	if (parameterIndex == m_IdToIdxMap.at("gain"))
	{
		m_gain.setGainLinear(newValue);

		DBG_IF_DEBUG("GP new gain value:" + String(newValue));

		dsp::ProcessSpec spec{ m_sampleRate, static_cast<uint32> (m_samplesPerBlock), 1 };
		m_gain.prepare(spec);
	}
}

void GainProcessor::updateParameterValues()
{
	auto idx = m_IdToIdxMap.at("gain");
	parameterValueChanged(idx, getNormalizedValue(getParameters().getUnchecked(idx)));
}

const String GainProcessor::getName() const 
{ 
	return "Gain"; 
}


HPFilterProcessor::HPFilterProcessor()
	: ChannelStripProcessorBase()
{
	initParameters();

	m_filter.setType(dsp::StateVariableTPTFilterType::highpass);
}

ChannelStripProcessorBase::ChannelStripProcessorType HPFilterProcessor::getType()
{
	return ChannelStripProcessorBase::CSPT_HighPass;
}

float HPFilterProcessor::getMagnitudeResponse(float freq)
{
	float magnitude = 0.0;
	float T = 1 / m_sampleRate;
	
	float wdCutoff = 2 * MathConstants<float>::pi * m_filter.getCutoffFrequency();

	//Calculating pre-warped/analogue cutoff frequency to use in virtual analogue frequeny response calculations
	float cutOff = (2 / T) * tan(wdCutoff * T / 2);

	//Digital frequency to evaluate
	float wdEval = 2 * MathConstants<float>::pi * freq;
	float sValue = (2 / T) * tan(wdEval * T / 2);

	/* This is the digital transfer function which is equal to the analogue transfer function
	 evaluated at H(s) where s = (2/T) * tan(wd*T/2) hence why the cutoff used is the pre warped analogue equivalent.
	 See Art Of VA Filter Design 3.8 Bilinear Transform Section */
	magnitude = sValue / (sValue + cutOff);

	magnitude = magnitude * m_gain.getGainLinear();

	//Convert to db for log db response display
	magnitude = Decibels::gainToDecibels(magnitude, getMinDecibels());
	return  magnitude;
}

float HPFilterProcessor::getFilterFequency()
{
	return m_filter.getCutoffFrequency();
}

float HPFilterProcessor::getFilterGain()
{
	return m_gain.getGainLinear();
}

std::vector<ChannelStripProcessorBase::ProcessorParam> HPFilterProcessor::getProcessorParams()
{
	return std::vector<ChannelStripProcessorBase::ProcessorParam>{ { "hpff", "Highpass freq.", 20.0f, 20000.0f, 1.0f, 1.0f, 20.0f }, { "hpfg", "Highpass gain", 0.0f, 1.0f, 0.01f, 1.0f, 1.0f } };
}

void HPFilterProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	m_filter.process(context);
	m_gain.process(context);
}

void HPFilterProcessor::reset()
{
	m_filter.reset();
	m_gain.reset();
}

void HPFilterProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
	auto param = getParameters().getUnchecked(parameterIndex);
	auto fParam = dynamic_cast<AudioParameterFloat*>(param);
	auto min = fParam->getNormalisableRange().getRange().getStart();
	auto max = fParam->getNormalisableRange().getRange().getEnd();
	auto newRangedValue = jmap(jlimit(0.0f, 1.0f, newValue), min, max);

	if (parameterIndex == m_IdToIdxMap.at("hpff"))
	{
		m_filter.setCutoffFrequency(newRangedValue);

		DBG_IF_DEBUG("HPFP new hpff value:" + String(newRangedValue));

		dsp::ProcessSpec spec{ m_sampleRate, static_cast<uint32> (m_samplesPerBlock), 1 };
		m_filter.prepare(spec);
	}
	else if (parameterIndex == m_IdToIdxMap.at("hpfg"))
	{
		m_gain.setGainLinear(newRangedValue);

		DBG_IF_DEBUG("HPFP new hpfg value:" + String(newRangedValue));

		dsp::ProcessSpec spec{ m_sampleRate, static_cast<uint32> (m_samplesPerBlock), 1 };
		m_gain.prepare(spec);
	}
}

void HPFilterProcessor::updateParameterValues()
{
	auto idx = m_IdToIdxMap.at("hpff");
	parameterValueChanged(idx, getNormalizedValue(getParameters().getUnchecked(idx)));
	idx = m_IdToIdxMap.at("hpfg");
	parameterValueChanged(idx, getNormalizedValue(getParameters().getUnchecked(idx)));
}

const String HPFilterProcessor::getName() const
{ 
	return "HighPass"; 
}


LPFilterProcessor::LPFilterProcessor()
	: ChannelStripProcessorBase()
{
	initParameters();

	m_filter.setType(dsp::StateVariableTPTFilterType::lowpass);
}

ChannelStripProcessorBase::ChannelStripProcessorType LPFilterProcessor::getType()
{
	return ChannelStripProcessorBase::CSPT_LowPass;
}

float LPFilterProcessor::getMagnitudeResponse(float freq)
{
	float magnitude = 0.0;
	float T = 1 / m_sampleRate;

	float wdCutoff = 2 * MathConstants<float>::pi * m_filter.getCutoffFrequency();

	//Calculating pre-warped/analogue cutoff frequency to use in virtual analogue frequeny response calculations
	float cutOff = (2 / T) * tan(wdCutoff * T / 2);

	//Digital frequency to evaluate
	float wdEval = 2 * MathConstants<float>::pi * freq;
	float sValue = (2 / T) * tan(wdEval * T / 2);


	/* This is the digital transfer function which is equal to the analogue transfer function
	 evaluated at H(s) where s = (2/T) * tan(wd*T/2) hence why the cutoff used is the pre warped analogue equivalent.
	 See Art Of VA Filter Design 3.8 Bilinear Transform Section */
	magnitude = cutOff / (sValue + cutOff);

	magnitude = magnitude * m_gain.getGainLinear();

	//Convert to db for log db response display
	magnitude = Decibels::gainToDecibels(magnitude, getMinDecibels());
	return  magnitude;
}

float LPFilterProcessor::getFilterFequency()
{
	return m_filter.getCutoffFrequency();
}

float LPFilterProcessor::getFilterGain()
{
	return m_gain.getGainLinear();
}

std::vector<ChannelStripProcessorBase::ProcessorParam> LPFilterProcessor::getProcessorParams()
{
	return std::vector<ChannelStripProcessorBase::ProcessorParam>{ { "lpff", "Lowpass freq.", 20.0f, 20000.0f, 1.0f, 1.0f, 20000.0f }, { "lpfg", "Lowpass gain", 0.0f, 1.0f, 0.01f, 1.0f, 1.0f } };
}

void LPFilterProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	m_filter.process(context);
	m_gain.process(context);
}

void LPFilterProcessor::reset()
{
	m_filter.reset();
	m_gain.reset();
}

void LPFilterProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
	auto param = getParameters().getUnchecked(parameterIndex);
	auto fParam = dynamic_cast<AudioParameterFloat*>(param);
	auto min = fParam->getNormalisableRange().getRange().getStart();
	auto max = fParam->getNormalisableRange().getRange().getEnd();
	auto newRangedValue = jmap(jlimit(0.0f, 1.0f, newValue), min, max);

	if (parameterIndex == m_IdToIdxMap.at("lpff"))
	{
		m_filter.setCutoffFrequency(newRangedValue);

		DBG_IF_DEBUG("LPFP new lpff value:" + String(newRangedValue));

		dsp::ProcessSpec spec{ m_sampleRate, static_cast<uint32> (m_samplesPerBlock), 1 };
		m_filter.prepare(spec);
	}
	else if (parameterIndex == m_IdToIdxMap.at("lpfg"))
	{
		m_gain.setGainLinear(newRangedValue);

		DBG_IF_DEBUG("LPFP new lpfg value:" + String(newRangedValue));

		dsp::ProcessSpec spec{ m_sampleRate, static_cast<uint32> (m_samplesPerBlock), 1 };
		m_gain.prepare(spec);
	}
}

void LPFilterProcessor::updateParameterValues()
{
	auto idx = m_IdToIdxMap.at("lpff");
	parameterValueChanged(idx, getNormalizedValue(getParameters().getUnchecked(idx)));
	idx = m_IdToIdxMap.at("lpfg");
	parameterValueChanged(idx, getNormalizedValue(getParameters().getUnchecked(idx)));
}

const String LPFilterProcessor::getName() const 
{ 
	return "LowPass"; 
}

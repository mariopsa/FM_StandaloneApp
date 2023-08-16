#pragma once

#include <JuceHeader.h>


struct FMWaveSound : public juce::SynthesiserSound

{
    FMWaveSound(juce::Slider& fmodSlider, juce::Slider& modIndexSlider): fmodSlider(fmodSlider), modIndexSlider(modIndexSlider){}

    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
    
private:
    juce::Slider& fmodSlider;
    juce::Slider& modIndexSlider;
};

struct FMWaveVoice : public juce::SynthesiserVoice
{
    FMWaveVoice
    (juce::Slider& fmodSlider, juce::Slider& modIndexSlider):
        fmodSlider(fmodSlider), modIndexSlider(modIndexSlider) {}


    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<FMWaveSound*>(sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
    
    void stopNote(float /*velocity*/, bool allowTailOff) override;

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;

private:
    void updateAngleDelta(double cyclesPerSecond);
    void updateAngleDeltaMod();
    void setSliderParams(juce::Slider& slider, float range_min, float range_max);
    
    //slider address
    juce::Slider& fmodSlider;
    juce::Slider& modIndexSlider;
    
    //initial value
    double currentSampleRate = 0.0, currentAngle = 0.0, currentAngleMod = 0.0, angleDelta = 0.0, angleDeltaMod = 0.0, level = 0.0, tailOff = 0.0;
};

//======================================================================================================================
class SynthAudioSource : public juce::AudioSource

{
    //==================================================================================================================
public:
    SynthAudioSource
    (juce::MidiKeyboardState& keyState, juce::Slider& fmodSlider, juce::Slider& modIndexSlider)
        : keyboardState(keyState), fmodSlider(fmodSlider), modIndexSlider(modIndexSlider)
    {

        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new FMWaveVoice(fmodSlider, modIndexSlider));

        synth.addSound(new FMWaveSound(fmodSlider, modIndexSlider));
    }

    void setUsingSineWaveSound();

    void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        synth.setCurrentPlaybackSampleRate(sampleRate);
    }

    void releaseResources() override {}

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

private:
    //keyboard definition
    juce::MidiKeyboardState& keyboardState;
    
    //Slider address
    juce::Slider& fmodSlider;
    juce::Slider& modIndexSlider;
    
    juce::Synthesiser synth;
};

//======================================================================================================================
class MainContentComponent : public juce::AudioAppComponent,
                             private juce::Timer
{
public:
    //==================================================================================================================
    
    MainContentComponent();

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void resized() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        synthAudioSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        synthAudioSource.getNextAudioBlock(bufferToFill);
    }

    void releaseResources() override
    {
        synthAudioSource.releaseResources();
    }
    
    //====================================================================================================================
private:
    void timerCallback() override;
    
    //SLIDER DEFINITION
    juce::Slider ampSlider;
    juce::Slider fmodSlider;
    juce::Slider modIndexSlider;
    
    //KEYBOARD DEFINITION
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;
    
    //AUDIO SOURCE DEFINITION
    SynthAudioSource synthAudioSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};

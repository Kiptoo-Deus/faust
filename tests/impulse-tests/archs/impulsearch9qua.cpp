#ifndef FAUSTFLOAT
#define FAUSTFLOAT double
#endif

#include "controlTools.h"

//----------------------------------------------------------------------------
//FAUST generated code
//----------------------------------------------------------------------------

<<includeIntrinsic>>

<<includeclass>>

// Wrapping C++ class for the C object

class Cdsp : public dsp {
    
    private:
        
        mydsp* fDSP;
        int* iControl;
        double* fControl;
        int* iZone;
        double* fZone;
        
    public:
        
        Cdsp(int* icontrol, double* fcontrol, int* izone, double* fzone)
        {
            fDSP = newmydsp(icontrol, fcontrol, izone, fzone);
            iControl = icontrol;
            fControl = fcontrol;
            iZone = izone;
            fZone = fzone;
        }
        
        virtual ~Cdsp()
        {
            deletemydsp(fDSP);
        }
        
        virtual int getNumInputs() { return getNumInputsmydsp(fDSP); }
        
        virtual int getNumOutputs() { return getNumOutputsmydsp(fDSP); }
        
        virtual void buildUserInterface(UI* ui_interface)
        {
            UIGlue glue;
            buildUIGlue(&glue, ui_interface, true);
            buildUserInterfacemydsp(fDSP, &glue);
        }
        
        virtual int getSampleRate()
        {
            return getSampleRatemydsp(fDSP);
        }
        
        virtual void init(int sample_rate)
        {
            initmydsp(fDSP, sample_rate);
        }
        
        static void classInit(int sample_rate)
        {
            classInitmydsp(sample_rate);
        }
        
        virtual void instanceInit(int sample_rate)
        {
            instanceInitmydsp(fDSP, sample_rate);
        }
        
        virtual void instanceConstants(int sample_rate)
        {
            instanceConstantsmydsp(fDSP, sample_rate);
        }
        
        virtual void instanceResetUserInterface()
        {
            instanceResetUserInterfacemydsp(fDSP);
        }
        
        virtual void instanceClear()
        {
            instanceClearmydsp(fDSP);
        }
        
        virtual dsp* clone()
        {
            return new Cdsp(iControl, fControl, iZone, fZone);
        }
        
        virtual void metadata(Meta* m)
        {
            MetaGlue glue;
            buildMetaGlue(&glue, m);
            metadatamydsp(&glue);
        }
    
        virtual void control()
        {
            // controlmydsp(fDSP);
        }
    
        virtual void frame(FAUSTFLOAT* RESTRICT inputs, FAUSTFLOAT* RESTRICT outputs)
        {
            // framemydsp(fDSP, inputs, outputs);
        }
        
        virtual void compute(int count, FAUSTFLOAT** input, FAUSTFLOAT** output)
        {
            computemydsp(fDSP, count, input, output);
        }
        
        virtual void compute(double /*date_usec*/, int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs)
        {
            compute(count, inputs, outputs);
        }
        
};


struct OneSample : public decorator_dsp {
    
    FAUSTFLOAT* fInputs;
    FAUSTFLOAT* fOutputs;
    
    OneSample(dsp* dsp):decorator_dsp(dsp),fInputs(nullptr),fOutputs(nullptr)
    {}
    
    virtual ~OneSample()
    {
        delete [] fInputs;
        delete [] fOutputs;
    }
    
    // This is mandatory
    virtual OneSample* clone()
    {
        return new OneSample(fDSP->clone());
    }
    
    // The standard 'compute' expressed using the control/compute (one sample) model
    virtual void compute(int count, FAUSTFLOAT** inputs_aux, FAUSTFLOAT** outputs_aux)
    {
        // TODO : not RT safe
        if (!fInputs) {
            fInputs = new FAUSTFLOAT[getNumInputs() * 4096];
            fOutputs = new FAUSTFLOAT[getNumOutputs() * 4096];
        }
    
        // Control
        fDSP->control();
        
        // Compute
        int num_inputs = getNumInputs();
        int num_outputs = getNumOutputs();
        
        FAUSTFLOAT* inputs_ptr = &fInputs[0];
        FAUSTFLOAT* outputs_ptr = &fOutputs[0];
        
        for (int frame = 0; frame < count; frame++) {
            for (int chan = 0; chan < num_inputs; chan++) {
                inputs_ptr[chan] = inputs_aux[chan][frame];
            }
            inputs_ptr += num_inputs;
        }
    
        inputs_ptr = &fInputs[0];
        for (int frame = 0; frame < count; frame++) {
            // One sample compute
            fDSP->frame(inputs_ptr, outputs_ptr);
            inputs_ptr += num_inputs;
            outputs_ptr += num_outputs;
        }
        
        outputs_ptr = &fOutputs[0];
        for (int frame = 0; frame < count; frame++) {
            for (int chan = 0; chan < num_outputs; chan++) {
                outputs_aux[chan][frame] = outputs_ptr[chan];
            }
            outputs_ptr += num_outputs;
        }
    }
    
    virtual void compute(double date_usec, int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs)
    {
        compute(count, inputs, outputs);
    }
    
};

// To be used in static context with -mem
static void runDSP2(dsp* DSP, const string& file, int& linenum, int nbsamples, bool inpl = false, bool random = false)
{
    char rcfilename[256];
    string filename = file;
    filename = filename.substr(0, filename.find ('.'));
    snprintf(rcfilename, 255, "%src", filename.c_str());
    dsp* oldDSP = DSP;
    
    FUI finterface;
    DSP->buildUserInterface(&finterface);
    
    // Soundfile
    TestMemoryReader* memory_reader = new TestMemoryReader();
    SoundUI sound_ui("", -1, memory_reader, (sizeof(FAUSTFLOAT) == sizeof(double)));
    DSP->buildUserInterface(&sound_ui);
    
    // Get control and then 'initRandom'
    CheckControlUI controlui;
    DSP->buildUserInterface(&controlui);
    controlui.initRandom();
    
    // MIDI control
    midi_handler handler;
    MidiUI midi_ui(&handler);
    DSP->buildUserInterface(&midi_ui);
    
    // Init signal processor and the user interface values
    DSP->instanceInit(44100);
    
    // Check getSampleRate
    if (DSP->getSampleRate() != 44100) {
        cerr << "ERROR runDSP in getSampleRate : " << DSP->getSampleRate() << std::endl;
    }
    
    // Check default after 'init'
    if (!controlui.checkDefaults()) {
        cerr << "ERROR runDSP in checkDefaults after 'init'" << std::endl;
    }
    
    // Check default after 'instanceResetUserInterface'
    controlui.initRandom();
    DSP->instanceResetUserInterface();
    if (!controlui.checkDefaults()) {
        cerr << "ERROR runDSP in checkDefaults after 'instanceResetUserInterface'" << std::endl;
    }
    
    // Check default after 'instanceInit'
    controlui.initRandom();
    DSP->instanceInit(44100);
    if (!controlui.checkDefaults()) {
        cerr << "ERROR runDSP in checkDefaults after 'instanceInit'" << std::endl;
    }
    
    // To test that instanceInit properly init a cloned DSP
    DSP = DSP->clone();
    DSP->instanceInit(44100);
    
    // Init UIs on cloned DSP
    DSP->buildUserInterface(&finterface);
    DSP->buildUserInterface(&sound_ui);
    DSP->buildUserInterface(&midi_ui);
    
    int nins = DSP->getNumInputs();
    int nouts = DSP->getNumOutputs();
    
    channels* ichan = new channels(kFrames, ((inpl) ? std::max(nins, nouts) : nins));
    channels* ochan = (inpl) ? ichan : new channels(kFrames, nouts);
    
    int run = 0;
    
    // recall saved state
    finterface.recallState(rcfilename);
    
    // Test MIDI control
    for (int i = 0; i < 127; i++) {
        handler.handleData2(0, midi::MidiStatus::MIDI_CONTROL_CHANGE, 0, i, 100);
        handler.handleData2(0, midi::MidiStatus::MIDI_POLY_AFTERTOUCH, 0, i, 75);
        handler.handleData2(0, midi::MidiStatus::MIDI_NOTE_ON, 0, i, 75);
        handler.handleData2(0, midi::MidiStatus::MIDI_NOTE_OFF, 0, i, 75);
        handler.handleData2(0, midi::MidiStatus::MIDI_PITCH_BEND, 0, i, 4000);
    }
    handler.handleData1(0, midi::MidiStatus::MIDI_PROGRAM_CHANGE, 0, 10);
    handler.handleData1(0, midi::MidiStatus::MIDI_AFTERTOUCH, 0, 10);
    
    GUI::updateAllGuis();
    
    // print audio frames
    int i = 0;
    try {
        while (nbsamples > 0) {
            if (run == 0) {
                ichan->impulse();
                finterface.setButtons(true);
            }
            if (run >= 1) {
                ichan->zero();
                finterface.setButtons(false);
            }
            int nFrames = min(kFrames, nbsamples);
            
            if (random) {
                int randval = rand();
                int n1 = randval % nFrames;
                int n2 = nFrames - n1;
                DSP->compute(n1, ichan->buffers(), ochan->buffers());
                DSP->compute(n2, ichan->buffers(n1), ochan->buffers(n1));
            } else {
                DSP->compute(nFrames, ichan->buffers(), ochan->buffers());
            }
            
            run++;
            // Print samples
            for (i = 0; i < nFrames; i++) {
                printf("%6d : ", linenum++);
                for (int c = 0; c < nouts; c++) {
                    FAUSTFLOAT f = normalize(ochan->buffers()[c][i]);
                    printf(" %8.6f", f);
                }
                printf("\n");
            }
            nbsamples -= nFrames;
        }
    } catch (...) {
        cerr << "ERROR in '" << file << "' at line : " << i << std::endl;
    }
    
    delete ichan;
    if (ochan != ichan) delete ochan;
    delete oldDSP;
    delete DSP;
}

int iControl[FAUST_INT_CONTROLS];
double fControl[FAUST_REAL_CONTROLS];

int iZone[FAUST_INT_ZONE];
double fZone[FAUST_FLOAT_ZONE];


int main(int argc, char* argv[])
{
    int linenum = 0;
    int nbsamples = 60000;
  
    // print general informations
    printHeader(new Cdsp(iControl, fControl, iZone, fZone), nbsamples);
    
    // linenum is incremented in runDSP
    runDSP2(new Cdsp(iControl, fControl, iZone, fZone), argv[0], linenum, nbsamples/4);
    runDSP2(new Cdsp(iControl, fControl, iZone, fZone), argv[0], linenum, nbsamples/4, false, true);
     
    return 0;
}
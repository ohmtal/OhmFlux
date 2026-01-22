# OPL2/OPL3 Class 

Modular OPL2/OPL3 Class using ymfm and SDL3 

[X] Port OPL Controller to new Dataformat

    [X] define Datastructure
    [X] port basic structure to OPL3Contoller 
    [X] 1 Custom instrument tested in TestBed :)

[ ] unsorted next steps

    [X] change tones to midi tones: Midi 0 = C(-1)
    
         Midi Notes start at oktave -1 and 127 is G-9
         I have the special number 0 = "..." 255 = "==="
         But 0 should be C on oktave -1 
         defined:   
         const uint8_t STOP_NOTE = 128;
         const uint8_t NONE_NOTE= 255;
         [X] NoteToValue
         [X] ValueToNote
         [X] playNote
        
        [X] Notes seams to be still out of sync!
            C-3 sounds like A-4
            => new f_numbers
  
    [~] fixe finetune -128 and is much to thin high  << fixed with op2 import
    [X] fixed op2 import 
    [X] create instrument editor 
    [X] wopl import 
    [X] fms import 
    [X] fms3 load/save

[ ] DSP

   [X] Add DSP Effects 
    - Warmth
    - Bit Crusher "Lo-Fi" Filter
    - Chorus 
    - Reverb
    - Limiter
    - 9 Band Equilizer
   
    
[ ] GUI: 

    [X] Prepare a new sequencer gui 
    [ ] split FluxEditor in 2 new projects SFX and TomsOldFMComposerReloaded

[ ] Create an Instrument Bank and Instrument editor with 2 Banks for OPL3

    [X] Gui for Soundbank
    [X] FMI Importer 
    [X] SBI Importer / Exporter
    [X] OP2 Importer 
    [X] WOPL Importer 
    
    Soundbanks: 
    https://github.com/Wohlstand/OPL3BankEditor/tree/master/Bank_Examples
    https://github.com/sneakernets/DMXOPL/releases
        

    
[ ] Create an Sequencer Gui 
    

[ ] Exporter 

    [X] Wav export with DSP Effects :)

[ ] Finally OPL can be removed from OhmFlux and a I put it into TomsOldFMComposerReloaded 

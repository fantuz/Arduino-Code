/*
 * https://macduino.blogspot.ch/2014/12/tone-music-encoding-howto.html
 * Adapted by Enrico Formenti from an original sketch by Tom Igoe
*/

#include "pitches.h"
#include "durations.h"
#include "Tempos.h"
#include "instructions.h"
#include "Stack.h"
#include "Stack.cpp"
#include "SilentNight.h"

#define INTERNOTE_PAUSE 1.3
#define TONEPIN 8

typedef struct {
  struct {
  uint16_t N;  // position in melody array
  uint16_t D;  // position in durations array
  } I;         // individual addressing
  uint32_t A;  // addressing all together
} POS;

void playMusic( uint16_t *instr, uint16_t *durs, uint16_t len) {

  POS coda;      // address of coda symbol
  POS segno;     // address of segno symbol
  POS invRepeat; // position of inv repeat sign
  POS nskip;     // posistion of the first note to skip
  POS fine;      // end of music symbol position
  POS askip;     // position of the first note to skip in alternate ending
  POS curPos; 

  // reset all internal variables
  coda.A = 0;
  segno.A = 0;
  invRepeat.A = 0;
  nskip.A = 0;
  fine.A = 0;
  askip.A = 0;
  curPos.A = 0;
  
  // start looping
  while(curPos.I.N < len) {
    Stack<POS> reps; // make the stack local so it is emptied when the loop 
                     // is over

    switch(instr[curPos.I.N]) {
      case NOTE_PAUSE:
        // noTone(TONEPIN);
        delay(durs[curPos.I.D]);
        curPos.A += 0x11;
        break;
      case NOTE_CODA:
        if(nskip.A) {
          curPos.A = nskip.A;
          nskip.A = 0;
          coda.A = 0;
          break;
        }
        coda.A = curPos.A+0x10; // start from next position
        (curPos.I.N)++;
        break;
      case NOTE_DACAPO:
        if(reps.isEmpty() || (reps.topElement().A!=curPos.A)) { // if this event has not already occurred
          reps.push(curPos); // add this event
          curPos.A=0;  // repeat from the beginning
          break;
        }
        // otherwise we simply skip and update the instruction pointer
        reps.pop();
        (curPos.I.N)++;
        break;
      case NOTE_SEGNO:
        segno.A=curPos.A+0x10; // start from next position in note array
        (curPos.I.N)++;        
        break;
      case NOTE_REPEAT:
        if(reps.isEmpty() || (reps.topElement().A != curPos.A)) {
          if(invRepeat.A) { // repeat from last inv repeat sign if any
            curPos.A=invRepeat.A+0x11; // add +1 to both N and D pointers
            invRepeat.A=0;
            reps.push(curPos);// push the event on the stack
            break;
          }
          // otherwise repeat from the beginning
          reps.push(curPos);// push the event on the stack
          curPos.A=0;
          break;
        }
        // if this is the sign that issued the event
        reps.pop();
        (curPos.I.N)++;
        break;
      case NOTE_INVREPEAT:
        invRepeat.A=curPos.A+0x10; // start from the next position in notes array
        (curPos.I.N)++;
        break;
      case NOTE_DALSEGNO:
        if(reps.isEmpty() || (reps.topElement().A!=curPos.A)) { // if this has not generated the event
          reps.push(curPos);
          curPos.A=segno.A;
          segno.A=0;
          break;
        }
        // otherwise skip and update N only
        reps.pop();
        segno.A=0;
        (curPos.I.N)++;
        break;
      case NOTE_DALSEGNOCODA:
        if(reps.isEmpty() || (reps.topElement().A!=curPos.A)) {
          reps.push(curPos);
          curPos.A = segno.A;
          segno.A = 0;
          nskip.A = coda.A+0x10;
          break;
        }
        // otherwise skip and update N only
        reps.pop();
        (curPos.I.N)++;
        break;
        case NOTE_DACAPOCODA:
        if(reps.isEmpty() || (reps.topElement().A!=curPos.A)) {
          reps.push(curPos);
          curPos.A = 0;
          segno.A = 0;
          nskip.A = coda.A+0x10;
          break;
        }
        // otherwise skip and update N only
        reps.pop();
        (curPos.I.N)++;
        break;
      case NOTE_FINE:
        if(fine.A) { // we already passed through the music end symbol
          curPos.I.N = len; // then stop playing
          break;
        }
        fine.A = curPos.A;
        (curPos.I.N)++;
        break;
      case NOTE_DACAPOFINE:
        curPos.A = 0;
        break;
      case NOTE_DALSEGNOFINE:
        curPos.A = segno.A;
        break;
      case NOTE_ALTBEGIN:
        if(askip.A) { // we are repeating the sequence, hence skip
          curPos.A = askip.A;
          askip.A = 0;
          break;
        }
        // not repeating so just keep playing
        (curPos.I.N)++;
        break;
      case NOTE_ALTEND:
        askip.A = curPos.A+0x10; // set the skip to the next instruction
                                 // so that it will be skipped during the
                                 // repetition
        break;   
      default: // it is a note instruction
        tone(TONEPIN, instr[curPos.I.N], durs[curPos.I.D]);
        // to distinguish the notes, set a minimum time between them.
        // the note's duration + 30% seems to work well:
        delay(durs[curPos.I.D] * 1.3);
        // stop the tone playing:
        noTone(TONEPIN);
        curPos.A += 0x11; // update both N and D pointers
        break;
    }
  }
}

void setTempo(uint16_t durs[], uint16_t tempo, uint16_t istart, uint16_t iend) {
  register uint16_t i;
  uint16_t base;
  
  base = (uint16_t)(1000.0/((tempo/60.0)*(1+INTERNOTE_PAUSE)));
  
  for(i=istart; i<iend; i++) {
    switch(durs[i]) {
      case DUR_SEMIBREVE: 
        durs[i] = base << 2;
        break;
      case DUR_MINIM: 
        durs[i] = base << 1;
        break;
      case DUR_CROTCHET: 
        durs[i] = base;
        break;
      case DUR_QUAVER:
        durs[i] = base >> 1;
        break;
      case DUR_SEMIQUAVER:
        durs[i] = base >> 2;
        break;
      case DUR_DEMISEMIQUAVER:
        durs[i] = base >> 3;
        break;
      case DUR_HEMIDEMISEMIQUAVER:
        durs[i] = base >> 4;
        break;
      default:
        durs[i] = (uint16_t)(durs[i]/(DUR_CROTCHET*1.0)*base);
        break;      
    }
  }
}

/*
The code below contain just one function. Call it with your own tempo value or choose one constant among those provided in the next section.
The formula to compute the transformation takes into account the pause that we put between a note and the following which was useful to distinguish the notes (this was set to 130% of the note duration in our previous programs).
Whenever the duration is not standard (in the case of an augmentation for example), a simple approximation formula is used. 

Abstract 
In our past programs for playing tone music on Arduino (see the basic program here and its advanced version here) are based on a fixed Tempo which is about 90 crotchets per minute.
In this article we give a simple routine which allows to change the Tempo. We also provide some basic conversion constants for the main Tempos.

Hey all! While transcribing some music sheet (game music) I've seen that the tempo was somewhat strange and it wasn't due only to the deformation of my poor loudspeaker.
Indeed, our past sketches for playing tone music on Arduino (see the basic program here and its advanced version here) are based on a fixed Tempo which is approx 90 crotchets per minute.
This is the same for all other sketches that I could find on the web. Therefore, here is a simple sketch function which changes the Tempo in your melody.

The sketch 
The code below contain just one function. Call it with your own tempo value or choose one constant among those provided in the next section.
The formula to compute the transformation takes into account the pause that we put between a note and the following which was useful to distinguish the notes (this was set to 130% of the note duration in our previous programs).
Whenever the duration is not standard (in the case of an augmentation for example), a simple approximation formula is used. 

Warning: some older version of melodies that you can find over the net use const int noteDurations[] as data type. Of course, you should change this into uint16_t noteDurations[] to be able to use the setTempo() routine.

The tempos
In most of the music sheets, the tempo is indicated a phrase or a word (usually in Italian) and the interpretation of such words/phrases is often a matter of taste or of long discussions.
Along with the previous sketch, below you can find Tempos.h: my personal interpretation of most known tempos.

Which music sheets to use? 
It is not clear which sheets translate better into tone music. I prefer to use easy piano music sheets, or simply start from scratch by trials and errors.

Basic encoding 
Encoding a music sheet essentially consists in defining the following three objects: 
uint16_t instr_len:  this is the length of the array melody. Remark that the size of a melody is limited by the size of int. This is not a true limitation since this is far beyond the memory capabilities of Arduino Uno.
uint16_t instr[]: array containing the instructions for playing the melody
uint16_t durs[]:  array containing the instructions for the duration of the current note or of the current pause.

Encoding pauses 
Pauses are encoded similarly to notes. Indeed, one has to add NOTE_PAUSE instruction in the melody array and adding the corresponding duration in the noteDurations array.

Encoding augmentations 
An augmentation is graphically represented by a dot which follows a note and it is meant to augment the duration of the note by one half. 
Therefore, to encode a note with an augmentation it is enough to encode the value of the pitch as usual; in the durs array we will add the note duration plus the duration of its half.
For example, considering the situation in Figure 1, one would add NOTE_RE4 to the instr array and DUR_MINIM+DUR_CROTCHET to the durs array. 

Encoding ties 
A tie is a symbol which connects two notes or three notes of the same pitch. Even if the two (or three) notes have different durations, their tie should be played as if it was a unique note with
a duration which is the sum of the durations of the composing notes. For example, the tie in Figure 2 is encoded by adding NOTE_SOL3 in the melody array and DUR_MINIM+DUR_SEMIBREVE in the noteDurations array. 
Encoding repetitions 
Repetitions are a bit complicated and consists in some symbols and some Italian phrases. If you are not familiar with them you can learn more in this excellent tutorial. We implemented all repetition types and symbols.
In order to add a repetition one just needs to add its corresponding encoding to the instr array and nothing to the durs array. The coding for the coda symbol (NOTE_CODA) has
to be inserted right before to the note that it refers to. Here is a table of the repetitions sign/phrases and their corresponding encoding.

Symbol/Phrase     Encoding
Repeat                      NOTE_REPEAT
Inverse repeat              NOTE_INVREPEAT
Coda                        NOTE_CODA
Da capo (DC)                NOTE_DACAPO
Da capo a coda              NOTE_DACAPOCODA
Segno                       NOTE_SEGNO
Dal segno (DS)              NOTE_DALSEGNO
Dal segno a coda            NOTE_DALSEGNOCODA
Fine                        NOTE_FINE
Da capo a fine (DC fine)    NOTE_DACAPOFINE
Dal segno a fine (DS fine)  NOTE_DALSEGNOFINE

Warning: 
In order to get a code as short as possible no effort is made to check if the sequence of repetition symbols/phrases is correct (for example, if a NOTE_DACAPO is preceded by a NOTE_CAPO).
In the case of incorrect encoding the behavior is unpredictable.

Alternate endings 
The program allows to encode alternate ending by putting the instruction NOTE_ALTBEGIN at the beginning of the alternate ending and NOTE_ALTEND at the end. You may add as many alternate ending as needed.
The instruction NOTE_ALTEND has to be omitted for the very last one. For example, considering the situation in Figure 3, one should have encoded 
      ..., NOTE_ALTBEGIN, ..., NOTE_ALTEND, NOTE_ALTBEGIN, ...
      
Remark that in order to keep the code as simple (and compact) as possible, the technique described here allows to describe only "linear" ending sequences.
Indeed, in complex music sheets you may find several endings by groups, for example one should play the first ending, the second; then the first one again, followed by the second one, etc.
In such complicated case, you should "unroll" the music yourself by duplicating some parts.

Encoding chords 
Another difficulty is to encode chords. Indeed, due to the limited capabilities of the speaker, the best and quickest way to encode chords is by encoding just the fundamental note of the chord 
and forget about the others (recall that the fundamental note of a chord is normally the one that is at the bottom of the chord). Figure 4 illustrates this idea.

In this post we are going to present a more sophisticated program for playing melodies on your Arduino. It allows you to encode repetitions and alternate endings, compacting your longer melodies.
Keywords: tone music; pauses; augmentations; ties; repetitions.

The new playing routine
The new play function implements more sophisticated management of melodies allowing pauses, ties, augmentations (like the past routine) and it also introduces the possibility
to use repetitions and alternate endings. This post explains how to encode all this. 

The sketch with the new routine is given below. Remark that it makes use of a support class that implements a simple stack.
Source files can found here: Stack.h, Stack.cpp. In order to be able to compile the sketch you should open this two files in other tabs of your Arduino IDE.
You also need instructions.h which contains the encoding for the new instructions used to encode the melodies.

All the songs encoded for the previous version of the playing routine still work for this new version without any modification. Indeed, the new melody encoding is an extension of the older one.
*/

void setup() {
  //Serial.begin(9600);
  
  playMusic((uint16_t *)melody,(uint16_t *)noteDurations,(uint16_t)melody_len);
  
  for (int curNote = 0; curNote < melody_len; curNote++) {
   
    if(melody[curNote])
      tone(TONEPIN, melody[curNote],noteDurations[curNote]);
    else {
      noTone(TONEPIN);
      delay(noteDurations[curNote]);
    }

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    delay(noteDurations[curNote] * 1.3);
    // stop the tone playing:
    noTone(TONEPIN);
  }
 
}

void loop() {
}


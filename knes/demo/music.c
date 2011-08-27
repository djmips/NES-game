#include "knes.h"
#include "music.h"

#define SET_INSTR 0x81
#define END_OF_SEQ 0xFF
#define ENV_MAX 16
#define ENV_END 0
#define TEMPO 9000

#define NUM_CHANNELS   4

struct Channels {
    byte instr[NUM_CHANNELS];
    word seq_idx[NUM_CHANNELS];
    byte seq_delta[NUM_CHANNELS];
    byte vol_env_ptr[NUM_CHANNELS];
    byte note_env_ptr[NUM_CHANNELS];
    byte duty_env_ptr[NUM_CHANNELS];
    byte note[NUM_CHANNELS];
    byte last_period[NUM_CHANNELS];
    byte env_vol[NUM_CHANNELS];
    byte env_note[NUM_CHANNELS];
    byte env_duty[NUM_CHANNELS];
};

static struct Channels channels;

struct Envelope {
    byte env[ENV_MAX];
    byte loop;
};

struct Instrument {
    struct Envelope vol;
    struct Envelope note;
    struct Envelope duty;
};

#include "song.h"

static const byte *sequences[NUM_CHANNELS] = {
    sequence0,
    sequence1,
    sequence2,
    sequence3,
};

static const word note_to_period[] = {
    3177, 2999, 2831, 2672, 2522, 2380, 2247, 2120, 2001, 1889,
    1783, 1683, 1588, 1499, 1415, 1336, 1261, 1190, 1123, 1060,
    1000, 944, 891, 841, 794, 749, 707, 668, 630, 595,
    561, 530, 500, 472, 445, 420, 397, 374, 353, 334,
    315, 297, 280, 265, 250, 236, 222, 210, 198, 187,
    176, 167, 157, 148, 140, 132, 125, 118, 111, 105,
    99, 93, 88, 83, 78, 74, 70, 66, 62, 59,
    55, 52, 49, 46, 44, 41, 39, 37, 35, 33,
    31, 29, 27, 26, 24, 23, 22, 20, 19, 18,
    17, 16, 15, 14, 13, 13
};

static byte cur_chn;
static const byte *seq_ptr;
static word tempo_count;

#define READ_ENVELOPE(d,e,p) { \
    (d) = (e).env[(p)++]; \
    if((e).env[(p)] == ENV_END) { \
      (p) = (e).loop; \
    } \
}

static void write_registers(void);
static void update_channel_envelopes(void);
static void execute_command(void);
static void update_channel_notes(void);

void init_music(void)
{
    CPU.apu.sq1.vol = CPU.apu.sq2.vol = CPU.apu.tri.linear =
        CPU.apu.noise.vol = CPU.apu.dmc.freq = 0;

    CPU.apu.sq1.sweep = CPU.apu.sq2.sweep = 0xF;
    
    CPU.apu_chn = _B(1111);
    
    channels.last_period[0] = channels.last_period[1] =
        channels.instr[0] = channels.instr[1] = channels.instr[2] =
        channels.instr[3] = 0xFF;
}

void play_music(void)
{
    word old_tempo_count = tempo_count;
    
    tempo_count += TEMPO;
    if(tempo_count < old_tempo_count) { // there was an overflow
        for(cur_chn = 0; cur_chn < NUM_CHANNELS; ++cur_chn) {
            seq_ptr = sequences[cur_chn];
            update_channel_notes();
        }
    }

    for(cur_chn = 0; cur_chn < NUM_CHANNELS; ++cur_chn) {
        update_channel_envelopes();
    }
    
    write_registers();
}

void update_channel_envelopes(void)
{
    // update envelopes
    const struct Instrument *instr;
    
    if(channels.instr[cur_chn] == 0xFF) return;
    instr = instr_ptrs[channels.instr[cur_chn]];
    
    READ_ENVELOPE(channels.env_vol[cur_chn], instr->vol, channels.vol_env_ptr[cur_chn]);
    READ_ENVELOPE(channels.env_note[cur_chn], instr->note, channels.note_env_ptr[cur_chn]);
    READ_ENVELOPE(channels.env_duty[cur_chn], instr->duty, channels.duty_env_ptr[cur_chn]);
}

void execute_command(void)
{
    byte b;
    
    b = seq_ptr[channels.seq_idx[cur_chn]++];
    if(b < 128) { // trigger a note (or note off if b == 0)
        channels.note[cur_chn] = b;
        channels.vol_env_ptr[cur_chn] = channels.note_env_ptr[cur_chn] = channels.duty_env_ptr[cur_chn] = 0;
    } else if(b >= SET_INSTR && b < SET_INSTR + 15) {
        channels.instr[cur_chn] = b - SET_INSTR;
    } else if(b == END_OF_SEQ) {
        channels.seq_idx[cur_chn] = 0;
    }
    
}

void update_channel_notes(void)
{
    byte delta;
    
    if(channels.seq_delta[cur_chn]) { // waiting for delta
        if(--channels.seq_delta[cur_chn]) return;
        // delta is now 0, command can be performed
        execute_command();
    }
    
    // look for first non-zero delta
    for(;;) {
        delta = seq_ptr[channels.seq_idx[cur_chn]++];
        if(delta) {
            channels.seq_delta[cur_chn] = delta;
            return;
        }
        // new delta was 0, execute immediately
        execute_command();
    }
}

void write_registers(void)
{
    byte vol;
    word period;
    byte note;
    
    // ---- PULSE 1 ----
    
    vol = channels.note[0] ? channels.env_vol[0] : 0;

    // volume & duty
    // CC65 optimizer is being a bit silly, if the constants are not grouped
    // with parenthesis it'll not optimize them to a single ORA..
    CPU.apu.sq1.vol = channels.env_duty[0] << 6 | (_B(100000) | _B(10000)) | vol;
    
    note = (channels.env_note[0] & 0x80) ? channels.env_note[0] & 0x7F :
        channels.note[0] + channels.env_note[0] - 64;
    period = note_to_period[note];
    
    // period
    CPU.apu.sq1.lo = LOBYTE(period);
    if(HIBYTE(period) != channels.last_period[0]) {
        CPU.apu.sq1.hi = channels.last_period[0] = HIBYTE(period);
    }
    
    // ---- PULSE 2 ----
    
    vol = channels.note[1] ? channels.env_vol[1] : 0;

    // volume & duty
    CPU.apu.sq2.vol = channels.env_duty[1] << 6 | (_B(100000) | _B(10000)) | vol;
    
    note = (channels.env_note[1] & 0x80) ? channels.env_note[1] & 0x7F :
        channels.note[1] + channels.env_note[1] - 64;
    period = note_to_period[note];
    
    // period
    CPU.apu.sq2.lo = LOBYTE(period);
    if(HIBYTE(period) != channels.last_period[1]) {
        CPU.apu.sq2.hi = channels.last_period[1] = HIBYTE(period);
    }
    
    // ---- TRIANGLE ----
    
    vol = channels.note[2] ? channels.env_vol[2] : 0;
    CPU.apu.tri.linear = vol ? 0xFF : 0;
    
    note = (channels.env_note[2] & 0x80) ? channels.env_note[2] & 0x7F :
        channels.note[2] + channels.env_note[2] - 64;
    period = note_to_period[note];
    
    // period
    CPU.apu.tri.lo = LOBYTE(period);
    CPU.apu.tri.hi = HIBYTE(period);
    
    // ---- NOISE ----
    
    vol = channels.note[3] ? channels.env_vol[3] : 0;
    CPU.apu.noise.vol = (_B(100000) | _B(10000)) | vol;
    
    note = (channels.env_note[3] & 0x80) ? channels.env_note[3] & 0x7F :
        channels.note[3] + channels.env_note[3] - 64;
    
    CPU.apu.noise.lo = (channels.env_duty[3] << 7) | (15 - (note & 0xF));
    CPU.apu.noise.hi = 0;
    
}

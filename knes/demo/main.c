/*

    SEAMEN CHRONICLES
    by thefox//aspekt 2010

    This is a platformer example for KNES library.
    Level graphics shamelessly ripped from Super Mario Bros.
    Music by ruuvari.
    
    Yeah, the name doesn't have anything to do with anything.

*/

#include "knes.h"
#include "graphics.h"
#include "map.h"
#include "music.h"

void copy_gfx(void);
void set_game_palette(void);
void set_logo_palette(void);
void set_nt(void);
void update_frame(void);
void update_ego_sprites(void);
void update_ego_physics(void);
void scroll(byte amount);
void upload_ppu_buffers(void);
void check_ego_x_collisions(void);
void check_ego_y_collisions(void);
void play_dpcm_sample(void);
void show_logo(void);
void upload_dynamic_palette(void);

#define EGO_INIT_X            (40 << 8)
#define EGO_INIT_Y            (150 << 8)
#define X_SCROLLPOS           (128 - 16/2)
#define EGO_MAXSPEED_NORMAL   (2 << 8)
#define EGO_MAXSPEED_FAST     (4 << 8)
#define MAX_EGO_JUMP_FRAMES   20

#define ANIMFRAME_RUN         0 // 0 - 2
#define ANIMFRAME_TURN        3
#define ANIMFRAME_JUMP        4
#define ANIMFRAME_STAND       5
#define ANIMFRAME_SQUAT       6
#define NUM_EGO_ANIMFRAMES    7

#define SCROLL_X_LEFT_EDGE    3136 // pixels
#define JUMP_Y_VELOCITY       -800

#pragma bss-name(push, "ZEROPAGE")
#pragma data-name(push, "ZEROPAGE")

static byte joy;
static long ego_x       = EGO_INIT_X; // ego_x/y are 24.8 fixed point
static word old_ego_x   = EGO_INIT_X >> 8;
static long ego_y       = EGO_INIT_Y;
static word old_ego_y   = EGO_INIT_Y >> 8;
static signed ego_xv, ego_yv; // velocity
static signed ego_xa; // acceleration
#define EGO_YA 60     // y-acceleration stays constant
static byte ego_xdir; // 0 = right, 1 = left
static byte ego_grounded;
static byte ego_animframe;
static byte ego_animspeed;
static byte ego_squatting;
static byte ego_throttle;
static byte ego_alive;
static byte ego_jump_frames;
static signed ego_jump_start_vel;

static byte framecount;

static signed scroll_x_left = 0;
static word valid_tilecol = 31;
static word valid_attrcol = 7;

static byte nt_buffer[28];
static byte nt_buffer_full;
static byte attr_buffer[8];
static byte attr_buffer_full;

static byte coin_color = 0x27; // color index for palette entry 3F0D

static byte i, j; // couple of generic loop counters

// byte indicating whether metatile is solid
static const byte solid_tiles[256] = {
    0, 0, 0, 0,    1, 0, 0, 0,    0, 1, 1, 1,    1, 1, 1, 0,
    1, 1, 0, 0,    0, 0, 1, 1,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 1,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    // ...
};

int main(void)
{
    init_music();
    OAM_DMA();
    
    // keep NMI on at all times
    PPU.ctrl = BASE_NT0|ADDRINC_1|SPR_CHR0|BG_CHR1|SPR_8X8|NMI_ON;

    show_logo();
    
    set_game_palette();
    set_nt();
    
    ego_alive = 1;
    
    for(;;) {
        // non-critical code, can safely take more than a single frame (but FPS will drop)
        update_frame();
        if(!ego_alive) {
            CPU.apu_chn = 0;
            wait_vblank();
            PPU.mask = GRAYSCL_OFF|BGCLIP_OFF|SPRCLIP_OFF|BGREND_OFF|SPRREND_OFF|
                TINTRED_OFF|TINTGRN_OFF|TINTBLU_OFF;
            break;
        }
        play_music();
        
        wait_vblank();
        // critical PPU updates
        OAM_DMA();
        upload_ppu_buffers();
        upload_dynamic_palette();
        // set the MSB of scroll in PPU.ctrl
        PPU.ctrl = ADDRINC_1|SPR_CHR0|BG_CHR1|SPR_8X8|NMI_ON|
            (scroll_x_left & 0x100) >> 8;
        PPU.mask = GRAYSCL_OFF|BGCLIP_OFF|SPRCLIP_OFF|BGREND_ON|SPRREND_ON|
            TINTRED_OFF|TINTGRN_OFF|TINTBLU_OFF;
        PPU_SCROLL(scroll_x_left, 0);
    }
    
    return 0;
}

void upload_dynamic_palette(void)
{
    PPU_ADDR(0x3F0D);
    PPU.data = coin_color;
}

void set_game_palette(void)
{
    #define BGCOLOR 0x22
    static const byte palette[] = {
        // background palette
        BGCOLOR, 0x29, 0x1A, 0xF,
        BGCOLOR, 0x36, 0x17, 0xF,
        BGCOLOR, 0x30, 0x21, 0xF,
        BGCOLOR, 0x27, 0x17, 0xF,
        // sprite palette
        BGCOLOR, 0x16, 0x27, 0x18,
    };
    #undef BGCOLOR

    // avoid artifacts by only writing palette in vblank
    wait_vblank();

    PPU_ADDR(0x3F00);
    for(i = 0; i < sizeof palette; ++i) {
        PPU.data = palette[i];
    }
}

void set_logo_palette(void)
{
    #define BGCOLOR 0x0F
    static const byte palette[] = {
        // background palette
        BGCOLOR, 0x01, 0x11, 0x21,
        BGCOLOR, 0x0F, 0x0F, 0x0F,
        BGCOLOR, 0x01, 0x11, 0x21,
        BGCOLOR, 0x01, 0x11, 0x21,
        // sprite palette
        BGCOLOR, 0x01, 0x0F, 0x0F,
    };
    #undef BGCOLOR

    // avoid artifacts by only writing palette in vblank
    wait_vblank();

    PPU_ADDR(0x3F00);
    for(i = 0; i < sizeof palette; ++i) {
        PPU.data = palette[i];
    }
}

void set_nt(void)
{
    const byte *p = MAP_DATA;
    const byte *p2 = MAP_DATA + MAP_W;
    byte metatile;
    byte attrbyte;
    
    PPU_ADDR(0x2000);
    
    // read 16x14 metatiles from map
    for(j = 0; j < 14; ++j) {
        for(i = 0; i < 16; ++i) {
            metatile = *p;
            PPU.data = MET0[metatile];
            PPU.data = MET1[metatile];
            ++p;
        }
        p -= 16;
        for(i = 0; i < 16; ++i) {
            metatile = *p;
            PPU.data = MET2[metatile];
            PPU.data = MET3[metatile];
            ++p;
        }
        p += MAP_W - 16;
    }
    
    // write attributes
    p = MAP_DATA;
    PPU_ADDR(0x23C0);
    for(j = 0; j < 7; ++j) {
        for(i = 0; i < 8; ++i) {
            attrbyte = map_exported_attrib[*p++];
            attrbyte |= (map_exported_attrib[*p++] << 2);
            attrbyte |= (map_exported_attrib[*p2++] << 4);
            attrbyte |= (map_exported_attrib[*p2++] << 6);
            PPU.data = attrbyte;
        }
        p += MAP_W + MAP_W - 16;
        p2 += MAP_W + MAP_W - 16;
    }
}

void update_frame(void)
{
    static const byte coin_palette_lut[] = {
        0x27, 0x27, 0x27, 0x17, 0x07, 0x17
    };
    static byte coin_palette_index;

    byte prev_joy = joy;
    joy = read_joy(0);
    if(!ego_squatting || !ego_grounded) {
        if(joy & JOY_LEFT) {
            ego_xa = -22;
            if(ego_grounded) ego_xdir = 1;
        } else if(joy & JOY_RIGHT) {
            ego_xa = 22;
            if(ego_grounded) ego_xdir = 0;
        } else {
            ego_xa = 0;
        }
    } else {
        ego_xa = 0;
    }
    
    if(joy & JOY_A) {
        if(!(prev_joy & JOY_A)) { // A is down for the first time
            if(ego_grounded) {
                ego_yv = JUMP_Y_VELOCITY; // force y velocity
                ego_jump_frames = 1;
                ego_jump_start_vel = 0;
            }
        } else { // player is holding A down
            if(ego_jump_frames) {
                if(ego_jump_frames < MAX_EGO_JUMP_FRAMES) {
                    ego_yv = JUMP_Y_VELOCITY; // force y velocity
                    ++ego_jump_frames;
                }
            }
        }
    } else {
        ego_jump_frames = 0;
    }
    
    if(ego_grounded) {
        if(joy & JOY_DOWN) {
            ego_squatting = 1;
        } else {
            ego_squatting = 0;
        }
    }
    ego_throttle = joy & JOY_B;

    old_ego_x = ego_x >> 8;
    old_ego_y = ego_y >> 8;

    update_ego_sprites();
    update_ego_physics();
    
    if(ego_y >> 8 > 240) {
      ego_alive = 0;
    }

    // only check collisions if character isn't over the top of the screen (check the sign of ego_y)
    // of course this will also cause collision checks to be skipped for the top two tile rows...
    if(!_P(&ego_y)[3]) {
        check_ego_x_collisions();
        check_ego_y_collisions();
    }

    // catch the scrolling up to ego x position
    // the edge check isn't perfect, if ego is moving fast garbage might be displayed
    if(ego_x >> 8 > scroll_x_left + X_SCROLLPOS && scroll_x_left < SCROLL_X_LEFT_EDGE) {
        scroll((ego_x >> 8) - (scroll_x_left + X_SCROLLPOS));
    }
    
    // animspeed based on ego_xv
    if(ego_xv == 0) {
        ego_animspeed = 0xFF;
    } else if(ego_xv >= -1 << 8 && ego_xv <= 1 << 8) {
        ego_animspeed = 7;
    } else {
        ego_animspeed = 3;
    }
    
    if(ego_animspeed != 0xFF && (framecount & ego_animspeed) == 0) {
        if(++ego_animframe == 3) ego_animframe = 0;
    }
    
    if((framecount & 7) == 0) {
        coin_color = coin_palette_lut[coin_palette_index];
        ++coin_palette_index;
        if(coin_palette_index == sizeof coin_palette_lut) {
            coin_palette_index = 0;
        }
    }
    
    ++framecount;
}

void upload_ppu_buffers(void)
{
    word attraddr;

    if(nt_buffer_full) {
        // calculate nametable address based on valid_tilecol
        // for first NT it is $2000+valid_tilecol
        // after valid_tilecol%64 is >=32 address is $2400+valid_tilecol
        // also need to switch on inc32 mode in PPU.ctrl
        
        PPU.ctrl = BASE_NT0|ADDRINC_32|SPR_CHR0|BG_CHR1|SPR_8X8|NMI_ON;
        
        if((valid_tilecol & 63) < 32) {
            // upload to 0x2000+valid_tilecol
            PPU_ADDR(0x2000 + (valid_tilecol & 31));
        } else {
            // upload to 0x2400+valid_tilecol
            PPU_ADDR(0x2400 + (valid_tilecol & 31));
        }
        for(i = 0; i < 28; ++i) {
            PPU.data = nt_buffer[i];
        }
        nt_buffer_full = 0;
    }
    
    if(attr_buffer_full) {
        PPU.ctrl = BASE_NT0|ADDRINC_1|SPR_CHR0|BG_CHR1|SPR_8X8|NMI_ON;
        
        if((valid_attrcol & 15) < 8) {
            // upload to 0x23C0+valid_tilecol
            attraddr = 0x23C0 + (valid_attrcol & 7);
        } else {
            // upload to 0x27C0+valid_tilecol
            attraddr = 0x27C0 + (valid_attrcol & 7);
        }
        for(i = 0; i < 8; ++i) {
            PPU_ADDR(attraddr);
            PPU.data = attr_buffer[i];
            attraddr += 8;
        }
        attr_buffer_full = 0;
    }
    
}

void scroll(byte amount)
{
    word new_valid_tilecol;
    byte new_valid_attrcol;
    byte metatile;
    const byte *p;
    const byte *p2;
    byte *ntbuf;
    byte *attrbuf;
    byte attrbyte;
    
    if(!amount) return;
    
    p = MAP_DATA;
    ntbuf = nt_buffer;
    
    scroll_x_left += amount;
    new_valid_tilecol = (scroll_x_left + 255) / 8;
    if(new_valid_tilecol != valid_tilecol) {
        valid_tilecol = new_valid_tilecol;
        p += valid_tilecol / 2;
        // copy the new tile column to nt_buffer and set a flag to
        // indicate it needs to be uploaded
        if((valid_tilecol & 1) == 0) {
            for(i = 0; i < 14; ++i) {
                metatile = *p;
                *ntbuf++ = MET0[metatile];
                *ntbuf++ = MET2[metatile];
                p += MAP_W;
            }
        } else {
            for(i = 0; i < 14; ++i) {
                metatile = *p;
                *ntbuf++ = MET1[metatile];
                *ntbuf++ = MET3[metatile];
                p += MAP_W;
            }
        }
        nt_buffer_full = 1;
    }
    
    p = MAP_DATA;
    p2 = MAP_DATA + MAP_W;
    attrbuf = attr_buffer;
    
    new_valid_attrcol = (scroll_x_left + 255) / 32;
    if(new_valid_attrcol != valid_attrcol) {
        valid_attrcol = new_valid_attrcol;
        p += valid_attrcol * 2;
        p2 += valid_attrcol * 2;
        // copy the new attribute column to attr_buffer and set a flag to
        // indicate it needs to be uploaded
        for(i = 0; i < 8; ++i) {
            attrbyte = map_exported_attrib[*p++];
            attrbyte |= (map_exported_attrib[*p] << 2);
            attrbyte |= (map_exported_attrib[*p2++] << 4);
            attrbyte |= (map_exported_attrib[*p2] << 6);
            *attrbuf++ = attrbyte;
            p += MAP_W + MAP_W - 1;
            p2 += MAP_W + MAP_W - 1;
        }
        attr_buffer_full = 1;
    }
}

void update_ego_sprites(void)
{
    // 7 frames, 8 tiles per frame
    // the top bit is set if sprite needs to be horizontally flipped
    // the exception is 0xFC, which is an empty tile
    static const byte animtiles[NUM_EGO_ANIMFRAMES][8] = {
      { 0, 1, 2, 3, 4, 5, 6, 7 },                             // run 0
      { 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF },                 // run 1
      { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 },     // run 2
      { 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F },     // turn
      { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 },     // jump
      { 0, 1, 0x4C, 0x4D, 0x4A, 0x4A|0x80, 0x4B, 0x4B|0x80 }, // stand
      { 0xFC, 0xFC, 8, 9, 0x58, 0x59, 0x5A, 0x5A|0x80 }       // squat
    };

    struct ObjAttr *spr = OAM;
    word y = ego_y >> 8;
    byte attrib = SPR_PAL0|SPR_INFRNTBG|SPRFLIPH_OFF|SPRFLIPV_OFF;
    byte curframe = ego_animframe;
    const byte *tiles;
    
    if(ego_grounded)
    {
        if(ego_xv) {
            // if ego is facing different direction than where he's moving...
            if((HIBYTE(ego_xv) >> 7 != ego_xdir)) {
                curframe = ANIMFRAME_TURN;
            }
        } else {
            // only make ego stand if not accelerating
            if(ego_xa == 0) {
                curframe = ANIMFRAME_STAND;
            }
        }
    } else {
        curframe = ANIMFRAME_JUMP;
    }
    
    if(ego_squatting) {
        curframe = ANIMFRAME_SQUAT;
    }
    
    tiles = animtiles[curframe];
    
    if(ego_xdir == 1) {
      attrib |= SPRFLIPH_ON;
    }
    for(j = 0; j < 4; ++j) {
        byte x = (ego_x >> 8) - scroll_x_left;
        if(ego_xdir == 1) x += 8;
        for(i = 0; i < 2; ++i) {
            spr->x = x;
            spr->y = y;
            if(HIBYTE(y)) {
              spr->y = 0xFF;
            }
            // getting pretty dirty, 0xFC is a special case here (empty tile)
            spr->tile = *tiles != 0xFC ? *tiles & 0x7F : *tiles;
            spr->attrib = (*tiles & 0x80) ? attrib ^ SPRFLIPH_ON : attrib;
            
            ++spr;
            ++tiles;
            x += ego_xdir == 1 ? -8 : 8;
        }
        y += 8;
    }
}

void update_ego_physics(void)
{
    ego_x += ego_xv;
    if(ego_x >> 8 < scroll_x_left) {
        ego_x = ((long)scroll_x_left) << 8;
        ego_xv = 0;
    }
    ego_y += ego_yv;
    ego_xv += ego_xa;
    ego_yv += EGO_YA;
    
    if(ego_throttle) {
        if(ego_xv > EGO_MAXSPEED_FAST) {
            ego_xv = EGO_MAXSPEED_FAST;
        } else if(ego_xv < -EGO_MAXSPEED_FAST) {
            ego_xv = -EGO_MAXSPEED_FAST;
        }
    } else {
        if(ego_xv > EGO_MAXSPEED_NORMAL) {
            ego_xv = EGO_MAXSPEED_NORMAL;
        } else if(ego_xv < -EGO_MAXSPEED_NORMAL) {
            ego_xv = -EGO_MAXSPEED_NORMAL;
        }
    }

    // friction (opposite to direction of movement)
    if(ego_xv < 0) {
        ego_xv += 7;
        if(ego_xv > 0) ego_xv = 0;
    } else if(ego_xv > 0) {
        ego_xv -= 7;
        if(ego_xv < 0) ego_xv = 0;
    }
}

void check_ego_x_collisions(void)
{
    byte first_coll_blockx;
    byte first_coll_blocky;
    byte num_yblocks;
    word ego_x_shifted = ego_x >> 8;
    const byte *p;
    byte collide_x;
    byte dir;
    byte ego_collpt_y = ego_squatting ? 16 : 0;
    
    if(ego_x_shifted == old_ego_x) return;
    
    if(ego_x_shifted > old_ego_x) { // moving right
        first_coll_blockx = (ego_x_shifted + 15) / 16;
        dir = 0;
    } else { // moving left
        first_coll_blockx = ego_x_shifted / 16;
        dir = 1;
    }

    first_coll_blocky = (old_ego_y + ego_collpt_y) / 16;
    
    // TODO: we should check if first_coll_blocky is less than 0, if so, fix to 0
    //   also, use first_coll_blocky in the calculation below
    //   and do some adjustments to y collision check also
    
    num_yblocks = ((old_ego_y + 31) / 16 - (old_ego_y + ego_collpt_y) / 16) + 1;
    
    // calculate a pointer to the first block
    p = MAP_DATA;
    p += first_coll_blockx;
    for(i = 0; i < first_coll_blocky; ++i) {
        p += MAP_W;
    }
    
    collide_x = 0;
    
    for(i = 0; i < num_yblocks; ++i) {
        if(solid_tiles[*p]) {
            collide_x = 1;
            break;
        }
        p += MAP_W;
    }
    
    if(collide_x) {
        if(dir == 0) {
            ego_x = ((long)((first_coll_blockx - 1) * 16)) << 8;
        } else {
            ego_x = ((long)((first_coll_blockx + 1) * 16)) << 8;
        }
        ego_xv = 0;
        // hackety hack: y-collision routine will use this...
        old_ego_x = ego_x >> 8;
    }
}

void check_ego_y_collisions(void)
{
    byte first_coll_blockx;
    byte first_coll_blocky;
    byte num_xblocks;
    word ego_y_shifted = ego_y >> 8;
    const byte *p;
    byte collide_y;
    byte dir;
    byte ego_collpt_y = ego_squatting ? 16 : 0;
    
    if(ego_y_shifted == old_ego_y) return;
    
    first_coll_blockx = old_ego_x / 16;
    
    if(ego_y_shifted > old_ego_y) { // moving down
        first_coll_blocky = (ego_y_shifted + 31) / 16;
        dir = 0;
    } else { // moving up
        first_coll_blocky = (ego_y_shifted + ego_collpt_y) / 16;
        dir = 1;
    }
    
    num_xblocks = ((old_ego_x + 15) / 16 - old_ego_x / 16) + 1;
    
    // calculate a pointer to the first block
    p = MAP_DATA;
    p += first_coll_blockx;
    for(i = 0; i < first_coll_blocky; ++i) {
        p += MAP_W;
    }
    
    collide_y = 0;
    
    for(i = 0; i < num_xblocks; ++i) {
        if(solid_tiles[*p]) {
            collide_y = 1;
            break;
        }
        ++p;
    }
    
    ego_grounded = collide_y && dir == 0;
     
    if(collide_y) {
        if(dir == 0) {
            ego_y = ((long)((first_coll_blocky - 1) * 16 + 16 - 32)) << 8;
        } else {
            ego_y = ((long)((first_coll_blocky + 1) * 16 - ego_collpt_y)) << 8;
        }
        ego_yv = 0;
        ego_jump_frames = 0;
    }
    
}

void play_dpcm_sample(void)
{
    CPU.apu.dmc.freq  = 0xE;  // frequency
    CPU.apu.dmc.raw   = 63;   // starting value for the counter
    CPU.apu.dmc.start = 0xC0; // sample at 0xF000
    CPU.apu.dmc.len   = 0xFE; // sample length is 4065 bytes
    CPU.apu_chn       = 0xF;  // stop old sample
    CPU.apu_chn       = 0x1F; // start sample
}

void show_logo(void)
{
    word i; // NOTE: overrides the global variable "byte i"
    signed logo_scroll_x;
    const char *text = "PRESS START";
    byte text_flash_timer = 0;
    
    set_logo_palette();
    
    // clear both nametables (vertical mirroring)
    // 0x40 is empty in both CHR banks
    PPU_ADDR(0x2000);
    for(i = 0; i < 2048; ++i) {
        PPU.data = 0x40;
    }

    // copy logo nametable to ppu memory
    PPU_ADDR(0x20E0);
    for(i = 0; i < 256; ++i) {
        PPU.data = gfx_index_logo[i] + 0x40; // indices in file are off by 0x40
    }
    // set the attribute table for NT1
    PPU_ADDR(0x23C0);
    for(j = 0; j < 40; ++j) {
        PPU.data = 0;
    }
    // rest of the attribute table to use the 2nd palette set (for text)
    for(j = 0; j < 24; ++j) {
        PPU.data = _B(01010101);
    }
    
    PPU_ADDR(0x22AA);
    while(j = *text++) {
        PPU.data = j - ' ' + 0xC0;
    }
    
    // main logo loop
    for(logo_scroll_x = 256; logo_scroll_x <= 512; logo_scroll_x += 8) {
        wait_vblank();
        // critical PPU updates
        // ...
        // set the MSB of scroll in PPU.ctrl
        PPU.ctrl = ADDRINC_1|SPR_CHR0|BG_CHR1|SPR_8X8|NMI_ON|
            (logo_scroll_x & 0x100) >> 8;
        PPU.mask = GRAYSCL_OFF|BGCLIP_OFF|SPRCLIP_OFF|BGREND_ON|SPRREND_ON|
            TINTRED_OFF|TINTGRN_OFF|TINTBLU_OFF;
        PPU_SCROLL(logo_scroll_x, 0);
    }
    
    // setup sprite #0 hit
    wait_vblank();
    OAM[0].x      = 211;
    OAM[0].y      = 119 - 1;
    OAM[0].tile   = 0xFB;
    OAM[0].attrib = 0;
    OAM_DMA();

    play_dpcm_sample();
    
    // wait for the sample to end
    while((CPU.apu_chn & (byte)_B(00010000)));
    
    for(;;) {
        ++text_flash_timer;
        
        // wait for player to push start...
        if(read_joy(0) & JOY_START) {
            break;
        }
    
        wait_vblank();
        // critical PPU updates
        // flash the text palette
        PPU_ADDR(0x3F05);
        if(text_flash_timer & 0x20) {
            PPU.data = PPU.data = PPU.data = 0x0F;
        } else {
            PPU.data = 0x07;
            PPU.data = 0x17;
            PPU.data = 0x27;
        }
        
        PPU.ctrl = BASE_NT0|ADDRINC_1|SPR_CHR0|BG_CHR1|SPR_8X8|NMI_ON;
        PPU_SCROLL(0, 0);
        
        // use sprite 0 hit to change charset after the logo
        
        // since we're still in vblank, wait for sprite 0 hit flag to be cleared
        //   it happens at the end of the vblank
        while(PPU.status & (byte)_B(01000000));
        // now wait for it to be set
        while(!(PPU.status & (byte)_B(01000000)));
        
        PPU.ctrl = BASE_NT0|ADDRINC_1|SPR_CHR0|BG_CHR0|SPR_8X8|NMI_ON;
    }
    
    wait_vblank();
    PPU.mask = GRAYSCL_OFF|BGCLIP_OFF|SPRCLIP_OFF|BGREND_OFF|SPRREND_OFF|
        TINTRED_OFF|TINTGRN_OFF|TINTBLU_OFF;   
}

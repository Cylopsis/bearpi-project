/*
 * Audio driver: WM8988 codec + SAI2 on STM32MP157 / LiteOS-A HDF.
 *
 * Logging: use dprintf() (kernel UART console) since:
 *   - printf() is not wired to the serial console in kernel space
 *   - HDF_LOG* macros are no-ops in LITEOS kernel builds (plat_log.h) or produce
 *     wrong tags because hdf_log.h #undef's HDF_LOG_TAG before we can set it
 * dprintf is declared in los_printf.h and writes directly to the UART.
 */

#include "los_printf.h"         /* dprintf() → kernel UART console */
#include "hdf_device_desc.h"
#include "hdf_sbuf.h"
#include "i2c_if.h"
#include "osal_io.h"
#include "osal_time.h"

#include "stm32mp1xx_hal.h"
#include "stm32mp1xx_hal_gpio.h"

/* User-space command codes */
#define AUDIO_CMD_BEEP  1   /* payload: uint32 freq_hz, uint32 duration_ms */
#define AUDIO_CMD_VOL   2   /* payload: uint32 volume (0-127)              */
#define AUDIO_CMD_STAT  3   /* no payload; reply: uint32 CR1, SR, PLL4CR, SAI2CKSELR */

/*
 * SAI2 Block A (clock master) and Block B (audio TX to WM8988 DACDAT).
 *
 * Board wiring (from schematic):
 *   SAI2_SD_A (PI6) → WM8988 ADCDAT (ADC output)  — Block A receives
 *   SAI2_SD_B (PA0) → WM8988 DACDAT (DAC input)   — Block B transmits ← audio goes here
 *
 * Block A: Master TX (generates MCLK/SCK/FS). SD_A drives WM8988 ADCDAT;
 *          safe because WM8988 ADC is powered off → ADCDAT is Hi-Z.
 * Block B: Synchronous slave TX (SYNCEN=01). Uses Block A's clocks internally.
 *          Writes on SD_B (PA0/AF8) → WM8988 DACDAT → DAC → LOUT2 → NS4890B → speaker.
 */
#define SAI2A_PHYS      0x4400B004UL   /* SAI2_BASE + 0x004, Block A */
#define SAI2B_PHYS      0x4400B024UL   /* SAI2_BASE + 0x024, Block B */
#define SAI2_SIZE       0x20UL

#define SAI_CR1_OFF     0x00U
#define SAI_CR2_OFF     0x04U
#define SAI_FRCR_OFF    0x08U
#define SAI_SLOTR_OFF   0x0CU
#define SAI_SR_OFF      0x14U
#define SAI_DR_OFF      0x1CU

/*
 * CR1 for Block A (Master TX, clock generator):
 *   MODE=00 | DS=011(16-bit, bits[7:5]=0b011=0x60) | CKSTR=1(bit9=0x200)
 *   OUTDRIV=1(bit13=0x2000) | MCKDIV=1(bit20=0x100000) | MCKEN=1(bit27=0x8000000)
 *
 * SAI_xCR1 DS field (DS_Pos=5, 3 bits): DS=0b011=3 → 16-bit word.
 * PLL4_Q ≈ 24.75 MHz. MCKDIV=1 → MCLK=12.375 MHz ≈ 12.288 MHz (256×48kHz)
 * Fs = 24.75 MHz / 512 = 48,340 Hz  (0.7% error, acceptable)
 */
#define SAI_CR1A_CFG    0x08102260UL   /* Block A master TX, DS=16-bit */

/*
 * CR1 for Block B (Synchronous slave TX):
 *   MODE=10(Slave TX)=0x02 | DS=011(16-bit)=0x60 | CKSTR=1=0x200
 *   SYNCEN=01(bit10=0x400) | OUTDRIV=1=0x2000
 *   No MCKDIV, no MCKEN (Block A handles clocks).
 *
 *   MODE must be 0b10 (Slave TX), not 0b00 (Master TX). Even with SYNCEN=01,
 *   MODE controls whether the TX engine expects to self-generate clocks (00)
 *   or use the partner block's clocks (10). Master TX mode stalls the engine.
 */
#define SAI_CR1B_CFG    0x00002662UL   /* Block B sync slave TX, DS=16-bit, MODE=10 */

/* CR2: FIFO threshold = 1/4 full */
#define SAI_CR2_CFG     0x00000001UL
/* FRCR: FRL=31(32 SCK/frame), FSALL=15(16 SCK active), FSDEF=1, FSOFF=1 → I2S */
#define SAI_FRCR_CFG    0x00050F1FUL
/* SLOTR: NBSLOT[3:0]=1(2 slots), SLOTEN[1:0]=0b11(both active), SLOTSZ=0(=DS) */
#define SAI_SLOTR_CFG   0x00030100UL

#define SAI_SR_FLVL_MSK  0x00070000UL
#define SAI_SR_FLVL_FULL 0x00050000UL  /* FLVL=5: FIFO full */

#define SAI_CR1_SAIEN   (1U << 16)

#define SAI_REG(off)  (*((volatile uint32_t *)(g_saiABase + (off))))
#define SAI_BREG(off) (*((volatile uint32_t *)(g_saiBBase + (off))))

/* WM8988 stereo codec on I2C bus 5, 7-bit address 0x1A */
#define WM8988_I2C_BUS  5
#define WM8988_ADDR     0x1A

/*
 * GPIO pins (from schematic):
 *   PA0  → SAI2_SD_B    (AF8)   WM8988 DACDAT  ← audio TX (Block B)
 *   PA3  → SAI2_MCLK_A  (AF10)  WM8988 MCLK
 *   PI5  → SAI2_SCK_A   (AF10)  WM8988 BCLK
 *   PI6  → SAI2_SD_A    (AF10)  WM8988 ADCDAT  (ADC off, pin is Hi-Z)
 *   PI7  → SAI2_FS_A    (AF10)  WM8988 LRC/FS
 *   GPIOA phys 0x50002000, GPIOI phys 0x5000A000
 */
#define GPIOA_PHYS  0x50002000UL
#define GPIOI_PHYS  0x5000A000UL
#define GPIO_SIZE   0x400UL

/*
 * RCC physical base = 0x50000000.
 * Offsets from struct RCC_TypeDef in stm32mp157axx_ca7.h:
 *   PLL3CR       = 0x880  PLL3CFGR1 = 0x884  PLL3CFGR2 = 0x888
 *   PLL4CR       = 0x894  PLL4CFGR1 = 0x898  PLL4CFGR2 = 0x89C
 *   SAI2CKSELR   = 0x8CC  (SAI2SRC[2:0]: 0=PLL4_Q, 1=PLL3_Q, 5=PLL3_R)
 */
#define RCC_PHYS             0x50000000UL
#define RCC_SIZE             0x1000UL
#define RCC_PLL3CR_OFF       0x880UL
#define RCC_PLL3CFGR1_OFF    0x884UL
#define RCC_PLL3CFGR2_OFF    0x888UL
#define RCC_PLL4CR_OFF       0x894UL
#define RCC_PLL4CFGR1_OFF    0x898UL
#define RCC_PLL4CFGR2_OFF    0x89CUL
#define RCC_SAI2CKSELR_OFF   0x8CCUL

/* 1 kHz sine wave, 48 samples at 48 kHz sample rate, Q15 amplitude */
static const int16_t g_sine48[48] = {
     0,  4276,  8480, 12539, 16383, 19947, 23169, 25995,
 28377, 30272, 31650, 32487, 32767, 32487, 31650, 30272,
 28377, 25995, 23169, 19947, 16383, 12539,  8480,  4276,
     0, -4276, -8480,-12539,-16383,-19947,-23169,-25995,
-28377,-30272,-31650,-32487,-32767,-32487,-31650,-30272,
-28377,-25995,-23169,-19947,-16383,-12539, -8480, -4276
};

static uintptr_t g_saiABase = 0;
static uintptr_t g_saiBBase = 0;
static DevHandle g_i2c     = NULL;
static uint8_t   g_volume  = 100;   /* 0..127 */
static uint32_t  g_pll4cr  = 0;     /* cached at init for STAT cmd */
static uint32_t  g_sai2sel = 0;     /* cached at init for STAT cmd */

/* WM8988 wire protocol: [reg7bit | data_bit8, data_bits7:0] */
static int32_t Wm8988Write(uint8_t reg, uint16_t val)
{
    uint8_t buf[2] = {
        (uint8_t)((reg << 1) | ((val >> 8) & 1u)),
        (uint8_t)(val & 0xFFu)
    };
    struct I2cMsg msg = { .addr = WM8988_ADDR, .flags = 0, .len = 2, .buf = buf };
    /*
     * Stm32mp1I2cTransfer returns the number of messages transferred on
     * success (1 for us), not HDF_SUCCESS (0). Check ret == 1.
     */
    int32_t ret = I2cTransfer(g_i2c, &msg, 1);
    if (ret != 1) {
        dprintf("[audio] WM8988 I2C reg%u=0x%x FAILED ret=%d\r\n", reg, val, ret);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

/*
 * WM8988 register map (from Linux driver wm8988.h / wm8988.c):
 *   R7  (0x07) = IFACE   — audio interface format (not R6 which is reserved)
 *   R8  (0x08) = SRATE   — sample rate
 *   R25 (0x19) = PWR1    — power 1 (VMID, VREF)
 *   R26 (0x1A) = PWR2    — power 2 (DACs, output drivers)
 *   R34 (0x22) = LOUTM1  — Left Output 1 mixer  (bit8 = Left DAC enable)
 *   R35 (0x23) = LOUTM2  — Left Output 2 mixer  (bit8 = Left DAC enable) ← speaker
 *   R36 (0x24) = ROUTM1  — Right Output 1 mixer (bit8 = Right DAC enable)
 *   R37 (0x25) = ROUTM2  — Right Output 2 mixer (bit8 = Right DAC enable) ← speaker
 *   R40 (0x28) = LOUT2V  — Left Output 2 volume
 *   R41 (0x29) = ROUT2V  — Right Output 2 volume
 *
 * NOTE: There is NO "Power3" register. R27 = ADCTL3 (ADC control).
 * Output mixer default = 0x0050: DAC NOT connected. Must set bit8 to connect DAC.
 */
static void Wm8988Init(void)
{
    dprintf("[audio] WM8988 init (I2C%d addr=0x%02x)\r\n",
            WM8988_I2C_BUS, WM8988_ADDR);

    int32_t ret = Wm8988Write(15, 0x000);  /* R15: software reset */
    if (ret != HDF_SUCCESS) {
        dprintf("[audio] WM8988 reset FAILED - codec not responding\r\n");
        return;
    }
    dprintf("[audio] WM8988 reset OK\r\n");
    OsalMSleep(50);

    /* R25 (PWR1): VMID=5kΩ (bits[8:7]=11 fast startup) + VREF=1 (bit6) */
    Wm8988Write(25, 0x1C0);
    OsalMSleep(50);

    /* R26 (PWR2): DACL(8)+DACR(7)+LOUT1(6)+ROUT1(5)+LOUT2(4)+ROUT2(3) all on */
    Wm8988Write(26, 0x1F8);

    /*
     * Audio interface format: I2S (FORMAT=10, bits[1:0]), 16-bit (WL=00, bits[3:2]), slave.
     * Different WM8988 silicon revisions place this register at R6 or R7.
     * Linux driver defines WM8988_IFACE=0x07 (R7, default=0x000A=24-bit I2S).
     * Some datasheets define R6=IFACE and R7=SRATE.
     * Write 0x002 to both to cover both variants without risk.
     */
    Wm8988Write(6,  0x002);  /* R6: IFACE on some WM8988 variants */
    Wm8988Write(7,  0x002);  /* R7: IFACE per Linux driver (WM8988_IFACE=0x07) */
    Wm8988Write(8,  0x000);  /* R8: SRATE normal, 48 kHz (= default, explicit) */

    /*
     * Connect Left DAC → Left Output 2 mixer → LOUT2 → SPK_LP → NS4890B → speaker.
     * (ROUT2 goes to bypass cap to GND on this board — only LOUT2 path matters.)
     *
     * Mixer register layout: bit8=LD2LO (DAC enable), bits[6:4]=LIN2LOVOL (default=5).
     * Write 0x150 = bit8=1 (DAC on) + bits[6:4]=0b101=5 (preserve default volume bits).
     * Writing 0x100 would clear bits[6:4] to 0 which may attenuate the mixer output.
     *
     * Linux driver places these at R34-R37; some WM8988 datasheets use R21-R24.
     * Write to all candidates to cover both register map variants.
     */
    /* Linux driver addresses (authoritative): */
    Wm8988Write(34, 0x150);  /* R34 LOUTM1: Left DAC → Left Output 1 */
    Wm8988Write(35, 0x150);  /* R35 LOUTM2: Left DAC → Left Output 2 (speaker!) */
    Wm8988Write(36, 0x150);  /* R36 ROUTM1: Right DAC → Right Output 1 */
    Wm8988Write(37, 0x150);  /* R37 ROUTM2: Right DAC → Right Output 2 */
    /* Alternative addresses seen in some WM8988 datasheets: */
    Wm8988Write(21, 0x150);  /* R21: Left Output Mixer in some variants */
    Wm8988Write(22, 0x150);  /* R22: Right Output Mixer in some variants */

    /* DAC digital volume: max + simultaneous-update bit on right channel */
    Wm8988Write(10, 0x0FF);  /* R10 LDAC: left vol */
    Wm8988Write(11, 0x1FF);  /* R11 RDAC: right vol + UPDATE bit → applies both */

    /* Output 1 (headphone): max volume + update */
    Wm8988Write(2,  0x07F);  /* R2 LOUT1V: max */
    Wm8988Write(3,  0x17F);  /* R3 ROUT1V: max + update */

    /* Output 2 (speaker, NS4890B via LOUT2→SPK_LP→IN-): max volume + update */
    /* Linux addresses: R40=LOUT2V, R41=ROUT2V */
    Wm8988Write(40, 0x07F);  /* R40 LOUT2V: max */
    Wm8988Write(41, 0x17F);  /* R41 ROUT2V: max + update */
    /* Alternative addresses in some WM8988 datasheets: */
    Wm8988Write(23, 0x17F);  /* R23: LOUT2V in some variants */
    Wm8988Write(24, 0x17F);  /* R24: ROUT2V in some variants */

    /* R5 (ADCDAC): DACMU=0 → unmute DAC */
    Wm8988Write(5,  0x000);

    dprintf("[audio] WM8988 init done\r\n");
}

static void SaiGpioInit(void)
{
    GPIO_TypeDef *gpioA = (GPIO_TypeDef *)OsalIoRemap(GPIOA_PHYS, GPIO_SIZE);
    GPIO_TypeDef *gpioI = (GPIO_TypeDef *)OsalIoRemap(GPIOI_PHYS, GPIO_SIZE);
    if (gpioA == NULL || gpioI == NULL) {
        dprintf("[audio] GPIO OsalIoRemap FAILED\r\n");
        if (gpioA) OsalIoUnmap(gpioA);
        if (gpioI) OsalIoUnmap(gpioI);
        return;
    }

    GPIO_InitTypeDef gi = {
        .Mode      = GPIO_MODE_AF_PP,
        .Pull      = GPIO_NOPULL,
        .Speed     = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = GPIO_AF10_SAI2,
    };

    /* Block A signals: PA3=MCLK_A, PI5=SCK_A, PI6=SD_A, PI7=FS_A (all AF10) */
    gi.Pin = GPIO_PIN_3;
    HAL_GPIO_Init(gpioA, &gi);
    gi.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    HAL_GPIO_Init(gpioI, &gi);

    /* Block B SD_B: PA0=SAI2_SD_B (AF8) → WM8988 DACDAT (audio TX path) */
    gi.Pin       = GPIO_PIN_0;
    gi.Alternate = GPIO_AF8_SAI2;
    HAL_GPIO_Init(gpioA, &gi);

    OsalIoUnmap(gpioA);
    OsalIoUnmap(gpioI);
    dprintf("[audio] SAI2 GPIO init done (PA0/AF8=SD_B, PA3/AF10=MCLK_A, PI5/6/7/AF10)\r\n");
}

static void SaiClockInit(void)
{
    volatile uint8_t *rcc = (volatile uint8_t *)OsalIoRemap(RCC_PHYS, RCC_SIZE);
    if (rcc == NULL) {
        dprintf("[audio] RCC OsalIoRemap FAILED\r\n");
        return;
    }

#define RCCREG(off) (*((volatile uint32_t *)((rcc) + (off))))

    uint32_t pll3cr    = RCCREG(RCC_PLL3CR_OFF);
    uint32_t pll3cfg1  = RCCREG(RCC_PLL3CFGR1_OFF);
    uint32_t pll3cfg2  = RCCREG(RCC_PLL3CFGR2_OFF);
    uint32_t pll4cr    = RCCREG(RCC_PLL4CR_OFF);
    uint32_t pll4cfg1  = RCCREG(RCC_PLL4CFGR1_OFF);
    uint32_t pll4cfg2  = RCCREG(RCC_PLL4CFGR2_OFF);
    uint32_t sai2sel   = RCCREG(RCC_SAI2CKSELR_OFF);

    dprintf("[audio] PLL3CR=0x%08x CFGR1=0x%08x CFGR2=0x%08x\r\n",
            pll3cr, pll3cfg1, pll3cfg2);
    dprintf("[audio] PLL4CR=0x%08x CFGR1=0x%08x CFGR2=0x%08x\r\n",
            pll4cr, pll4cfg1, pll4cfg2);
    dprintf("[audio] SAI2CKSELR=0x%08x (src=%u before change)\r\n",
            sai2sel, sai2sel & 7u);

    /* Cache for STAT command */
    g_pll4cr  = pll4cr;
    g_sai2sel = sai2sel;

    /*
     * Select SAI2 kernel clock source.
     * Try PLL4_Q (src=0) if PLL4 is running (bit0=PLLON, bit1=PLL4RDY).
     * Otherwise fall back to PLL3_Q (src=1).
     */
    uint32_t src = 0;  /* default: PLL4_Q */
    if (!(pll4cr & 0x1u)) {
        dprintf("[audio] PLL4 not enabled, trying PLL3_Q\r\n");
        src = 1;  /* PLL3_Q */
    }

    RCCREG(RCC_SAI2CKSELR_OFF) = (sai2sel & ~7u) | src;
    dprintf("[audio] SAI2CKSELR set src=%u, now=0x%08x\r\n",
            src, RCCREG(RCC_SAI2CKSELR_OFF));

    g_sai2sel = RCCREG(RCC_SAI2CKSELR_OFF);

#undef RCCREG

    OsalIoUnmap((void *)rcc);
}

static void SaiInit(void)
{
    __HAL_RCC_SAI2_CLK_ENABLE();  /* APB2 peripheral bus clock */
    SaiClockInit();                /* audio kernel clock */
    SaiGpioInit();

    g_saiABase = (uintptr_t)OsalIoRemap(SAI2A_PHYS, SAI2_SIZE);
    g_saiBBase = (uintptr_t)OsalIoRemap(SAI2B_PHYS, SAI2_SIZE);
    if (g_saiABase == 0 || g_saiBBase == 0) {
        dprintf("[audio] SAI2 OsalIoRemap FAILED (A=%lu B=%lu)\r\n",
                (unsigned long)g_saiABase, (unsigned long)g_saiBBase);
        if (g_saiABase) { OsalIoUnmap((void *)g_saiABase); g_saiABase = 0; }
        if (g_saiBBase) { OsalIoUnmap((void *)g_saiBBase); g_saiBBase = 0; }
        return;
    }

    /* Configure both blocks (not yet enabled). */
    SAI_REG(SAI_CR1_OFF)   = SAI_CR1A_CFG & ~SAI_CR1_SAIEN;
    SAI_REG(SAI_CR2_OFF)   = SAI_CR2_CFG;
    SAI_REG(SAI_FRCR_OFF)  = SAI_FRCR_CFG;
    SAI_REG(SAI_SLOTR_OFF) = SAI_SLOTR_CFG;

    SAI_BREG(SAI_CR1_OFF)   = SAI_CR1B_CFG & ~SAI_CR1_SAIEN;
    SAI_BREG(SAI_CR2_OFF)   = SAI_CR2_CFG;
    SAI_BREG(SAI_FRCR_OFF)  = SAI_FRCR_CFG;   /* same as Block A; inherited in sync mode */
    SAI_BREG(SAI_SLOTR_OFF) = SAI_SLOTR_CFG;

    /*
     * STM32MP1 RM §SAI sync mode: enable slave FIRST, then master.
     * Block B (slave, SYNCEN=01) enters "waiting for FS" state.
     * Block A (master) then starts → generates first FS → Block B locks and runs.
     */
    SAI_BREG(SAI_CR1_OFF) = SAI_CR1B_CFG | SAI_CR1_SAIEN;   /* slave Block B first */
    SAI_REG(SAI_CR1_OFF)  = SAI_CR1A_CFG | SAI_CR1_SAIEN;   /* master Block A second */

    /*
     * Prime Block A FIFO with zeros. SAI master TX may not generate continuous
     * SCK/FS until it has data — without data it can pause after the first FS
     * pulse, stalling Block B's sync slave TX engine.
     */
    for (int i = 0; i < 8; i++) {
        SAI_REG(SAI_DR_OFF) = 0;
    }

    dprintf("[audio] SAI2A CR1=0x%08x SR=0x%08x FRCR=0x%08x SLOTR=0x%08x\r\n",
            SAI_REG(SAI_CR1_OFF), SAI_REG(SAI_SR_OFF),
            SAI_REG(SAI_FRCR_OFF), SAI_REG(SAI_SLOTR_OFF));
    dprintf("[audio] SAI2B CR1=0x%08x SR=0x%08x FRCR=0x%08x SLOTR=0x%08x\r\n",
            SAI_BREG(SAI_CR1_OFF), SAI_BREG(SAI_SR_OFF),
            SAI_BREG(SAI_FRCR_OFF), SAI_BREG(SAI_SLOTR_OFF));
}

/*
 * DDS playback: step through a 48-sample sine table.
 * Phase Q16: step = freq * 65536 / 1000.
 * Returns 0 if FIFO drained OK, -1 if FIFO stuck (clock not running).
 */
static int32_t PlayBeep(uint32_t freq, uint32_t dur_ms)
{
    if (g_saiBBase == 0 || g_i2c == NULL) {
        dprintf("[audio] PlayBeep: driver not ready\r\n");
        return -1;
    }
    if (freq == 0 || freq > 22000 || dur_ms == 0) return 0;

    dprintf("[audio] PlayBeep %u Hz %u ms  SAI2B CR1=0x%08x SR=0x%08x\r\n",
            freq, dur_ms, SAI_BREG(SAI_CR1_OFF), SAI_BREG(SAI_SR_OFF));

    uint32_t step   = (uint32_t)(((uint64_t)freq << 16) / 1000u);
    uint32_t wrap   = 48u << 16;
    uint32_t phase  = 0;
    uint32_t frames = 48000u * dur_ms / 1000u;
    uint8_t  vol    = g_volume;

    for (uint32_t i = 0; i < frames; i++) {
        int16_t s = (int16_t)((int32_t)g_sine48[phase >> 16] * vol / 127);

        /*
         * Keep Block A FIFO fed with zeros so the master TX engine keeps
         * generating SCK/FS for Block B's sync slave to consume.
         * Non-blocking: skip if Block A FIFO is already full.
         */
        if ((SAI_REG(SAI_SR_OFF) & SAI_SR_FLVL_MSK) < SAI_SR_FLVL_FULL) {
            SAI_REG(SAI_DR_OFF) = 0;
            SAI_REG(SAI_DR_OFF) = 0;
        }

        uint32_t timeout = 500000u;
        while ((SAI_BREG(SAI_SR_OFF) & SAI_SR_FLVL_MSK) >= SAI_SR_FLVL_FULL) {
            if (--timeout == 0) {
                dprintf("[audio] FIFO STUCK at frame %u: SAI2A SR=0x%08x SAI2B CR1=0x%08x SR=0x%08x\r\n",
                        i, SAI_REG(SAI_SR_OFF), SAI_BREG(SAI_CR1_OFF), SAI_BREG(SAI_SR_OFF));
                return -1;
            }
        }

        /* Write L and R channel samples to Block B's DR → PA0/SD_B → WM8988 DACDAT */
        SAI_BREG(SAI_DR_OFF) = (uint32_t)(uint16_t)s;
        SAI_BREG(SAI_DR_OFF) = (uint32_t)(uint16_t)s;

        phase += step;
        if (phase >= wrap) phase -= wrap;
    }

    dprintf("[audio] PlayBeep done\r\n");
    return 0;
}

int32_t AudioDispatch(struct HdfDeviceIoClient *client, int cmdCode,
                      struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;

    if (cmdCode == AUDIO_CMD_STAT) {
        /* Return SAI2 Block A (master) and Block B (TX) SR for diagnosis */
        uint32_t cr1b = (g_saiBBase != 0) ? SAI_BREG(SAI_CR1_OFF) : 0xDEAD0001u;
        uint32_t srb  = (g_saiBBase != 0) ? SAI_BREG(SAI_SR_OFF)  : 0xDEAD0002u;
        uint32_t sra  = (g_saiABase != 0) ? SAI_REG(SAI_SR_OFF)   : 0xDEAD0003u;
        HdfSbufWriteUint32(reply, cr1b);
        HdfSbufWriteUint32(reply, srb);
        HdfSbufWriteUint32(reply, g_pll4cr);
        HdfSbufWriteUint32(reply, g_sai2sel);
        HdfSbufWriteUint32(reply, sra);   /* Block A SR */
        dprintf("[audio] STAT: SAI2A SR=0x%08x  SAI2B CR1=0x%08x SR=0x%08x  PLL4CR=0x%08x SEL=0x%08x\r\n",
                sra, cr1b, srb, g_pll4cr, g_sai2sel);
        return HDF_SUCCESS;
    }

    if (g_saiBBase == 0 || g_i2c == NULL) {
        dprintf("[audio] Dispatch: driver not ready (saiBBase=%lu i2c=%p)\r\n",
                (unsigned long)g_saiBBase, g_i2c);
        return HDF_ERR_INVALID_OBJECT;
    }

    switch (cmdCode) {
    case AUDIO_CMD_BEEP: {
        uint32_t freq = 0, dur = 0;
        if (!HdfSbufReadUint32(data, &freq) || !HdfSbufReadUint32(data, &dur))
            return HDF_ERR_INVALID_PARAM;
        int32_t rc = PlayBeep(freq, dur);
        uint32_t sr = (g_saiBBase != 0) ? SAI_BREG(SAI_SR_OFF) : 0;
        HdfSbufWriteUint32(reply, (uint32_t)(rc == 0 ? 0 : 1));  /* 0=OK, 1=stuck */
        HdfSbufWriteUint32(reply, sr);
        return HDF_SUCCESS;
    }
    case AUDIO_CMD_VOL: {
        uint32_t vol = 0;
        if (!HdfSbufReadUint32(data, &vol))
            return HDF_ERR_INVALID_PARAM;
        if (vol > 127) vol = 127;
        g_volume = (uint8_t)vol;
        /* R40 LOUT2V / R41 ROUT2V — speaker (NS4890B) output volume.
         * bit8 = update bit, bits[6:0] = volume (0x79 = 0dB, 0x00 = mute) */
        uint16_t rv = (uint16_t)(0x079u * vol / 127u);  /* scale 0..0x79 */
        Wm8988Write(40, rv);
        Wm8988Write(41, (uint16_t)(rv | 0x100u));  /* update both with bit8 */
        dprintf("[audio] volume set to %u (reg=0x%03x)\r\n", vol, rv);
        return HDF_SUCCESS;
    }
    default:
        return HDF_ERR_NOT_SUPPORT;
    }
}

int32_t HdfAudioBind(struct HdfDeviceObject *device)
{
    if (device == NULL) return HDF_ERR_INVALID_OBJECT;
    static struct IDeviceIoService svc = { .Dispatch = AudioDispatch };
    device->service = (struct IDeviceIoService *)&svc;
    return HDF_SUCCESS;
}

int32_t HdfAudioInit(struct HdfDeviceObject *device)
{
    (void)device;
    dprintf("[audio] HdfAudioInit start\r\n");

    g_i2c = I2cOpen(WM8988_I2C_BUS);
    if (g_i2c == NULL) {
        dprintf("[audio] I2cOpen(%d) FAILED\r\n", WM8988_I2C_BUS);
        return HDF_FAILURE;
    }
    dprintf("[audio] I2cOpen(%d) OK handle=%p\r\n", WM8988_I2C_BUS, g_i2c);

    SaiInit();
    if (g_saiBBase == 0) {
        I2cClose(g_i2c);
        g_i2c = NULL;
        return HDF_FAILURE;
    }

    Wm8988Init();
    dprintf("[audio] driver init OK\r\n");
    return HDF_SUCCESS;
}

void HdfAudioRelease(struct HdfDeviceObject *device)
{
    (void)device;
    /* Disable master first, then slave (reverse of enable order). */
    if (g_saiABase != 0) {
        SAI_REG(SAI_CR1_OFF) &= ~SAI_CR1_SAIEN;
    }
    if (g_saiBBase != 0) {
        SAI_BREG(SAI_CR1_OFF) &= ~SAI_CR1_SAIEN;
        OsalIoUnmap((void *)g_saiBBase);
        g_saiBBase = 0;
    }
    if (g_saiABase != 0) {
        OsalIoUnmap((void *)g_saiABase);
        g_saiABase = 0;
    }
    if (g_i2c != NULL) {
        I2cClose(g_i2c);
        g_i2c = NULL;
    }
    dprintf("[audio] driver released\r\n");
}

struct HdfDriverEntry g_audioDriverEntry = {
    .moduleVersion = 1,
    .moduleName    = "HDF_AUDIO_WM8988",
    .Bind          = HdfAudioBind,
    .Init          = HdfAudioInit,
    .Release       = HdfAudioRelease,
};

HDF_INIT(g_audioDriverEntry);

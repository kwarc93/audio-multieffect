/*
 * rcc.hpp
 *
 *  Created on: 25 paź 2020
 *      Author: kwarc
 */

#ifndef STM32F7_RCC_HPP_
#define STM32F7_RCC_HPP_

#include <cstdint>

//--------------------------------------------------------------------------------

/** @brief   Macro for initializing  @ref struct rcc::periph_bus.
 *  @example RCC_PERIPH_BUS(AHB1, GPIOA)
 */
#define RCC_PERIPH_BUS(_bus, _periph)  { drivers::rcc::bus::_bus, RCC_##_bus##ENR_##_periph##EN }

namespace drivers
{

//--------------------------------------------------------------------------------

class rcc final
{
public:
    rcc() = delete;

    enum class bus
    {
        AHB1,
        AHB2,
        AHB3,
        AHB4,
        APB1L,
        APB1H,
        APB2,
        APB3,
        APB4,
    };

    enum class reset_source
    {
        bor = 1 << 0,     /**< Brown-out reset. */
        pin = 1 << 1,     /**< Pin reset. */
        por = 1 << 2,     /**< Power on reset. */
        sw = 1 << 3,      /**< Software reset. */
        iwdg = 1 << 4,    /**< Independent watchdog reset. */
        wwdg = 1 << 5,    /**< Window watchdog reset. */
        lpwr = 1 << 6,    /**< Low power reset. */
    };

    /** @brief RCC peripheral definition. Initialize this object with @ref RCC_PERIPH_BUS. */
    struct periph_bus
    {
        rcc::bus bus;
        uint32_t periph;
    };

    /** @brief  Structure for describing main PLL parameters. */
    struct pll_cfg
    {
        uint32_t m;         /**< M divider, allowed range: 0 - 63 */
        uint32_t n;         /**< N multiplier, allowed range: 4 - 512 */
        uint32_t p;         /**< P divider, allowed (even!) values: 2 - 128 */
        uint32_t q;         /**< Q divider, allowed range: 1 - 128 */
        uint32_t r;         /**< R divider, allowed range: 1 - 128 */
    };

    /** @brief  Struct for describing bus prescalers */
    struct bus_presc
    {
        uint32_t sys;       /**< SYSCLK prescaler, use RCC_D1CFGR_D1CPRE_DIVx from CMSIS headers */
        uint32_t ahb;       /**< AHB prescaler, use RCC_D1CFGR_HPRE_DIVx from CMSIS headers */
        uint32_t apb1;      /**< APB1 prescaler, use RCC_D2CFGR_D2PPRE1_DIVx from CMSIS headers */
        uint32_t apb2;      /**< APB2 prescaler, use RCC_D2CFGR_D2PPRE2_DIVx from CMSIS headers */
        uint32_t apb4;      /**< APB4 prescaler, use RCC_D3CFGR_D3PPRE_DIVx from CMSIS headers */
    };

    /** @brief  Structure for describing SAI/I2S PLL parameters. */
    struct sai_i2s_pll
    {
        uint32_t n;         /**< N multiplier, allowed range: 50 - 432 */
        uint32_t p;         /**< P divider, allowed values: 2, 4, 6, 8 */
        uint32_t q;         /**< Q divider, allowed range: 2 - 15 */
        uint32_t r;         /**< R divider, allowed range: 2 - 7 */
        uint32_t div_q;     /**< After-Q divider, allowed values: 1 - 32  */
        uint32_t div_r;     /**< After-R divider (only for SAI), allowed values: 2, 4, 8, 16 */
    };

//--------------------------------------------------------------------------------

    /** @brief  Resets the RCC clock configuration to the default reset state. */
    static void reset(void);

    /** @brief  Resets all RCC peripherals configuration to the default reset state. */
    static void reset_all_periph(void);

    /** @brief  Resets all the RCC peripheral clocks configuration to the default reset state. */
    static void disable_all_periph_clocks(void);

    /**
     * @brief   Enables or disables selected peripheral clock.
     * @param   obj - pointer to structure created by @ref RCC_PERIPH_BUS
     * @param   en - true for enable, false for disable clock
     */
    static void enable_periph_clock(const periph_bus &pbus, bool en);

    /**
     * @brief   Sets oscillators frequencies as a reference for clock calculations.
     * @param   hsi - high speed internal oscillator value (Hz)
     * @param   hse - high speed external oscillator value (Hz)
     */
    static void set_oscillators_values(uint32_t hsi, uint32_t hse);

    /**
     * @brief   Configures the System clock source, PLL Multiplier and Divider factors,
     *          AHB/APBx prescalers and Flash settings.
     * @note    This function should be called only once the RCC clock configuration
     *          is reset to the default reset state.
     * @param   src - clock source, use RCC_PLLCKSELR_PLLSRC_HSx from CMSIS headers
     * @param   pll - pointer to main PLL configuration
     * @param   presc - pointer to bus prescalers configuration
     */
    static void set_main_pll(uint32_t src, const pll_cfg &pll, const bus_presc &presc);

    /**
     * @brief   Configures the second PLL.
     * @note    This function can be called only after set_main_pll() function call.
     * @param   pll - pointer to PLL configuration
     */
    static void set_2nd_pll(const pll_cfg &pll);

    /**
     * @brief   Configures the third PLL.
     * @note    This function can be called only after set_main_pll() function call.
     * @param   pll - pointer to PLL configuration
     */
    static void set_3rd_pll(const pll_cfg &pll);

    /** @brief  Enables/disables high speed internal clock.
     *  @param  state - true enables, false disables clock.
     *  @retval None
     */
    static void toggle_hsi(bool state);

    /** @brief  Enables/disables high speed external clock.
     *  @param  state - true enables, false disables clock.
     *  @retval None
     */
    static void toggle_hse(bool state);

    /** @brief  Enables/disables low speed external clock.
     *  @param  state - true enables, false disables clock.
     *  @retval None
     */
    static void toggle_lse(bool state);

    /** @brief  Enables/disables low speed internal clock.
     *  @param  state - true enables, false disables clock.
     *  @retval None
     */
    static void toggle_lsi(bool state);

    /** @brief  Gets actual system clock frequency.
     *  @param  None
     *  @retval System clock frequency in Hz
     */
    static uint32_t get_sysclk_freq(void);

    /**
     * @brief   Gets actual frequency for given bus.
     * @param   bus - @ref bus
     * @return  0 if bus is not valid
     */
    static uint32_t get_bus_freq(bus bus);

    /**
     * @brief   Gets actual prescaler for given bus.
     * @param   bus - @ref bus
     * @return  -1 if bus is not valid
     */
    static int16_t get_bus_presc(bus bus);

    /**
     * @brief   Gets reset source flags.
     * @return  reset source flags @ref enum reset_source
     */
    static reset_source get_reset_source(void);

    /** @brief  Clears reset source flags. */
    static void clear_reset_source(void);

private:
    static uint32_t get_sysclk_from_pll_source(void);

    static inline uint32_t hsi_clock = 64000000;
    static inline uint32_t hse_clock = 25000000;
};

//--------------------------------------------------------------------------------

}

#endif /* STM32F7_RCC_HPP_ */

#ifndef VO_GPIO_H
#define VO_GPIO_H

/// Saves configuration of pin, sets pin as GPIO input, reads pin state, restores configuration and returns the state.
/// \param pin pin[5:4]=port number (0, 1, 2), pin[3:0]=pin number (0 to 15)
#if 0
auto u_int16 GpioReadPin(register u_int16 pin);
#else
#define GpioReadPin(pin) (GpioReadPinDelay((pin),15,0))
#endif
/// Configures the pin as GPIO output and sets the the pin state (0 or 1).
/// \param pin pin[5:4]=port number (0, 1, 2), pin[3:0]=pin number (0 to 15)
auto void GpioSetPin(register u_int16 pin, register u_int16 state);
/// Configures the pin as GPIO input.
/// \param pin pin[5:4]=port number (0, 1, 2), pin[3:0]=pin number (0 to 15)
auto void GpioSetAsInput(register u_int16 pin);
/// Configures the pin as peripheral pin (not GPIO).
/// \param pin pin[5:4]=port number (0, 1, 2), pin[3:0]=pin number (0 to 15)
auto void GpioSetAsPeripheral(register u_int16 pin);
/// Saves configuration of pin, sets pin as GPIO input, reads pin state, restores configuration and returns the state.
/// \param pin pin[5:4]=port number (0, 1, 2), pin[3:0]=pin number (0 to 15).
/// \param microSeconds Time to wait for pin to settle.
/// \param waitAlways Wait even if pin was already input.
auto u_int16 GpioReadPinDelay(register u_int16 pin, register u_int32 microSeconds, register u_int16 waitAlways);


#endif

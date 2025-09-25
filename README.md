<h1 align="center">MarlinBio: Printess Bioprinter Firmware</h1>

This is an altered version of Marlin for academic and research related bioprinting. It is only intended to work with the open source Printess 2.0 system: https://printess.org **(Note: it will not work with the Printess 1.0 that uses a RUMBA control board)**. Many original Marlin features have been removed by the alterations, as the work to support functionality we don't currently need would be substantial. This is fairly new work that would benefit greatly from testing and feedback; bugs are likely to be encountered, even while working with supported features/settings; if any problematic or undesirable behavior is noticed, please be quick to open an [issue](https://github.com/twrankin/MarlinBio/issues). If you would like to request supporting old features or settings, adding new features or settings, or changing the current behavior, open a [discussion](https://github.com/twrankin/MarlinBio/discussions).

See here for the original README for Marlin: https://github.com/MarlinFirmware/Marlin/blob/bugfix-2.1.x/README.md

## Getting started

Documentation for setting up the hardware is not yet prepared. For instructions, [contact us](https://printess.org/about); much of the setup is similar to Printess 1.0: https://printess.org/build. 

Setting up MarlinBio is largely unchanged from Marlin. It needs to be compiled and linked into a binary file and flashed onto the Printess' main control board, which is typically a [BigTreeTech Octopus Max EZ V1.0](https://biqu.equipment/products/bigtreetech-btt-octopusmax-ez-for-3d-printer).

Building the firmware yourself allows you to read and adjust many useful configuration settings (many of these can also be changed live using G-code commands); this requires some setup, but is well worth the effort if you are an active Printess user. Alternatively, you can [contact us](https://printess.org/about) and ask for a pre-built binary.

### Building from source
For instructions on building the firmware from source, follow along with Marlin's installation documentation: https://marlinfw.org/docs/basics/install_platformio.html. No settings *need* to be changed, the default configuration should work for the Printess, but it can be very useful to read through the options and tailor the functionality to your preference. Check out the available settings in [Marlin/Configuration.h](https://github.com/twrankin/MarlinBio/blob/main/Marlin/Configuration.h). If anything here is changed, the firmware needs to be rebuilt and reinstalled before it will take effect.

### Installing the firmware
All roads should lead to a firmware.bin file; to install it, simply copy the file onto a microSD card, insert the card into the slot on the control board, and power on or reset the board. On start up, the board's bootloader will check the SD card for a "firmware.bin" file, copy it to the board's flash storage, rename it "FIRMWARE.CUR", and immediately run it. If an SD card or "firmware.bin" file is not present, the bootloader will load and run the existing firmware.

## New and changed features

To make Marlin more suitable for the needs of bioprinting, we added several new features and changed some of the existing functionality. If you find something surprising that isn't documented here, please open a [discussion](https://github.com/twrankin/MarlinBio/discussions) and let us know.

The original documentation for G-code commands from Marlin is still a good reference: https://marlinfw.org/meta/gcode. However, many commands have been added, changed, or are not currently supported. The most important changes should be documented here.

### Tool change
To enable working with multiple materials, the Printess allows for up to four extruder systems, each with their own Z axis. Previously, to enable firmware control of different Z axes and extruders, non-standard axis labels would be used, such as A/B/C/I/J/K/U/V/W. For example, moving and extruding the first system might look like `G0 Z1 E1`, while for the second system it might look like `G0 A1 B1`. Assumedly, you could also move the first Z axis with the second extruder... `G0 Z1 B1`, or any such combination.

To avoid confusion and work closer to the standard (especially since many of the axis labels are treated specially by Marlin according to their original intended use), we instead select the current system using the tool change command, `T`. The format of the tool change command is simply `Tx`, where `x` is the tool number to switch to.

Now, only the `X`, `Y`, `Z`, and `E` labels should be used: `G1 X1 Y1 Z1 E1`. On start up, the first tool should be selected. When needing to work with a different tool, say the fourth extruder system, a tool change must be issued: `T4`. Now, `G1 X1 Y1 Z1 E1` will move and extrude the fourth system. For controlling multiple systems in one command, see the **Mixing extruders** section below.

#### Nozzle offsets
X, Y, and Z offsets for the nozzle of each tool can be specified in the configuration as `NOZZLE_OFFSET_X`, `NOZZLE_OFFSET_Y`, and `NOZZLE_OFFSET_Z`. They can also be changed using the `M218` G-code command. For example, `M218 T1 X10` will set the X offset for tool 1 to 10mm. During a tool change from tool 0 to 1, the platform will move 10mm in the positive X direction. Offsets are always relative to tool 0, which should have an offset of 0. Note: If these are set, the associated axis must be homed before issuing a tool change (see the **Sensorless homing** section below).

#### Z raise
When switching tools, the current tool usually needs to be moved out of the way, so that it doesn't interfere with the rest of the print, and the new tool needs to be moved in to place. This can be done manually using `G0` commands before and after calling `T`, or it can be specified as a setting that will automatically raise the current tool to a specified position and lower the new one down to the old position, as a convenience. This is specified as `TOOLCHANGE_ZRAISE` in the configuration, or set using the `M217 Z` command. Moving the new tool to the old position can be disabled by defining `DISABLE_TOOLCHANGE_Z_RETURN`. Note: If this is set, the Z axes must be homed before issuing a tool change (see the **Sensorless homing** section below).

### Mixing extruders

Individual extruder systems can be linked into one tool to facilitate mixing their materials. To better support this, Marlin's original mixing system has been refactored, with all of the associated settings, commands, and functionality changed.

Declaring multiple extruder systems as linked will cause them to be considered as one tool; as such, whenever Z or E movement is specified for that tool, all of the associated Z axis and extruder motors will respond. Any combination of extruders can be linked, to make two linked with two independent, two sets of two, one set of three with one independent, or all four linked. But linked extruders must be sequential, so something like the first extruder linked to only the fourth extruder is not allowed.

Currently, changing the mixing configuration is only possible by defining `MIXING_EXTRUDER` and updating the `MIXING_CONFIGURATION` setting, requiring a firmware rebuild. Changing this live via G-code may be supported in the future. Please see the associated settings for more details on their format.

#### Mix ratios
When extruders are linked, `E` moves will cause all of them to extrude, but they can do so at different rates, as specified in `MIX_RATIOS`. For example, if extruders 0 and 1 are linked with ratios of 1 and 0.5, an extrusion of 10mm for tool 0 would move extruder 0 by 10mm and extruder 1 by 5mm. A ratio can also be 0, allowing for independent movement of extruders. Linked Z axes will always move in step.
`M163` can be used to update the mixing ratios, specifying the extruder with E and the ratio with R: `M163 E2 R0.33`. The other original mixing related commands are not needed or supported.

### Constant extrusion

Constant extrusion is a simple feature intended to partly alleviate some of the complexties of printing with non-newtonian gel based inks inside of compression syringe extrusion systems, as the fluidics are more dynamic than in traditional thermoplastic printing. It will simply set the extruder motor at a constant speed during print moves, regardless of other considerations. The speed is calculated from a simple conservation of mass equation; after solving:
$$ Speed = f \cdot K \cdot \frac{D_n}{D_s} $$
Where $ f $ is the feedrate specified for the move (after adjustment for limits), $ K $ is the correction factor, $ D_n $ is the inner diameter of the needle, and $ D_s $ is the inner diameter of the syringe. If the extruder is part of a mixing system, the speed would also be scaled by its mix ratio.

When enabled, constant extrusion will run during any print move, such as `G1`, `G2`, and `G3`; the `E` label will be ignored for such moves. `G0` commands will work as normal, allowing manual extruder positioning. Extrusion is paused when no movement is queued.

As a convenience, the syringe can also be pressurized before printing, and depressurized after printing. If a pressurization amount is set, the extruder(s) will quickly advance just before a print move, remain pressurized for subsequent print moves, and quickly retract before the next non print move.

#### Parameters
Constant extrusion is typically enabled by default via a setting in the configuration; otherwise, it is enabled whenever any `M789` command is issued without `D`, and disabled whenever `D` is present; enablement/disablement is for all extruders. The parameters for syringe inner diameter, nozzle inner diameter, correction factor, and pressurization can also be set, per extruder, in the configuration or via `M789`. E.g. `M789 E1 S8.66 N0.603 K1.33 P0.25`. 

This is a new feature, so please be sensitive to any seeming issues or undesirable behavior and provide us feedback!

### Sensorless homing

The stepper motor driver that the Printess is intended to use (TMC2209) has a feature that can be used to detect stalls based on current deviations from the expected induction of a moving rotor. This can be used to home the motor in a direction until the endstop is encountered and significant resistance causes a stall to (hopefully) be detected, allowing the position to be inferred.

#### Sensitivity
The sensitivity is the most important parameter, and can be set per motor in the configuration and using the `M914` command, which has not been changed. If it is too high, it will detect slight resistance as a stall, ending homing too early and setting an incorrect position; if it is too low, it will never detect a stall and the motor will torque against the endstop, causing a buzzing noise, until homing fails and the device is killed. This may need to be tweaked often, as many factors such as load, oil, dust, motion settings, breathing too loudly, etc. can cause enough difference to change the ideal value.

#### Homing feedrate and current
Less important, but still useful, parameters that can be tweaked are the homing feedrate and current. Setting them high may result in better detection, but harder crashes into the endstop. They can be adjusted in the configuration and via the `M210` and `M906` commands respectively.

#### Practical use
Homing is performed using the `G28` command; the parameters have not been changed. Only the X, Y, and Z axes can be homed. Sensorless homing is finicky, due to the noise involved in the detection method. It is safer to leave `G28` out of scripts, and run it manually until all of the axes have succeeded before starting a print. If an axis stalls or gets stuck too often, keep tweaking the sensitivity values, homing feedrates, and homing current. Ensuring the motors are properly oiled can also make a big difference (as well as with general printing); they do not often arrive this way.

If a motor torques into the endstop too long (when the sensitivity is too low), it can cause the lead screw to tighten (like screwing on the top of a bottle really tight), which can cause the next movement for that axis to stick. Use a quick `G0` command to check, and disable the power and twist it out of position manually if needed.

### Temperature control

Typical 3D printers only use Joule heating to control temperature. For bioprinting, it can be useful to keep materials at temperatures higher or lower than ambient. To accomplish this, we developed our own temperature controlled syringe holders and print bed that use thermoelectric modules to heat or cool (depending on current polarity).

The temperature control system has been significantly refactored from the original Marlin implementation. If your Printess does not have our temperature control modules, this section is irrelevant. Using other temperature modules is not recommended, do so at your own risk. Documentation for the hardware is beyond the scope of this guide, and will be provided at a later date.

#### Setting targets
Interacting with the temperature system primarily takes place via the `M104` command, which has been completely redone, with a few additional settings in the configuration file. A module begins in a disabled state; it can be set to heat or cool from the disabled state, and subsequently set back to being disabled. To set a target: `M104 I0 T4 C`, where `I` is the module number, `T` is the temperature, `C` is to enable cooling, `H` would be to enable heating, and `D` would be to disable the module (or all modules if `I` was not present). `C` or `H` only need to be specified once to enable cooling or heating from the disabled state. If the temperature is above the cooling target, or below the heating target, the module will be powered in an attempt to control the temperature to the target.

From our testing, sub-zero values are possible, and even stable, in the right setup with high fan speeds. This can be situational, and something like a higher ambient temperature or manufacturing/construction differences can throw everything off. It is recommended to do stability testing as close as possible to the same setup that you will use for an experiment.

Note that a module must be wired in the correct polarity for heating or cooling, and the polarity must be reversed to change this. **Always rewire with the power completely disconnected.**

Module 4 is connected to the output labeled **BED OUT**, which is typically connected to the bed.

#### Safety
**Software safety checks are never sufficient, always be physically present with the device when temperature control is running in case of fire.**

The firmware has several independent thermal safety monitors in order to reduce the risk of fires. The first monitor kicks in when the target is initially set; it ensures the module is making progress towards its target. If the temperature does not move a set amount towards the target in a specified number of seconds, the modules are disabled and the device is killed. It is disabled once the module is close to the target. The second monitor kicks in once the target is reached; it ensures the module stays close to the target, if it drifts too far off for too long, the modules are disabled and the device is killed. The last monitor simply ensures the temperature stays above and below certain minimum and maximum values; it also ensures the temperature sensor is returning sensible values, with at least some tiny variation (as the sensor returning the exact same value without even any noise for several seconds is likely to be a hardware issue).

The monitoring periods and thresholds can be changed in the configuration; **ensure that the change is necessary, and not a workaround to silence annoying errors.**

#### Fans
When a cooling target is set, the heat that is pumped out of the module must be dissipated. This is accomplished by attaching a heatsink to the hot side of the thermoelectric module, to transfer heat into a larger surface area via conduction, and then using a fan to blow air over the heatsink to displace heat into the environment via convection. So each module comes with a fan, that only needs to run when cooling.

#### Automatic fans
The fan speed for a module is automatically determined by default. While the module is disabled or heating, it will not be powered. When a cooling target is set, the fan speed is determined very simply based on a linear profile where it is at full speed when the target is -10C, and off at 90C. Pull requests for a more complex profile are welcome (see `manage_extruder_fans` in `temperature.h`).

The fans can be loud in order to dissipate enough heat to get to a low target. Once the target is reached, the fan speed can be lowered while the module just needs to be maintained. See `AUTO_FAN_DAMPEN` in the configuration.

#### Manual fans
Many will dislike automatic fan control. It may be faster (louder) than necessary, or it may be too slow, as the fastest and most stable cooling is achieved with maximum fan speed at all times. For those who wish to alter the behavior, the simplest way is to manually control the fans via the `M106` G-code command, which has been redone for our uses. Once an `M106` command is issued, automatic fan control is disabled for that module, but can be reenabled by issuing `M106 I0 A`, where `I` is the module index. A command such as `M106 I1 P255` will set the first module's fan to run at maximum speed (`P` ranges from 0-255).

`M106` is now technically PWM control (the method used to control the fan speed), as the PWM ports can be used to drive a number of peripherals, such as LEDs.

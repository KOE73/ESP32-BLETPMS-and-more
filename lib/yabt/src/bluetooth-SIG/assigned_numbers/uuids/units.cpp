#include "units.hpp"

#include <stddef.h>
#include <sstream>

typedef struct {
    uint16_t value;
    const char* name;
} KeyValue;

static const KeyValue data[] = {
    { 0x2700, "unitless" },                                 // ID: org.bluetooth.unit.unitless
    { 0x2701, "length (metre)" },                           // ID: org.bluetooth.unit.length.metre
    { 0x2702, "mass (kilogram)" },                          // ID: org.bluetooth.unit.mass.kilogram
    { 0x2703, "time (second)" },                            // ID: org.bluetooth.unit.time.second
    { 0x2704, "electric current (ampere)" },                // ID: org.bluetooth.unit.electric_current.ampere
    { 0x2705, "thermodynamic temperature (kelvin)" },       // ID: org.bluetooth.unit.thermodynamic_temperature.kelvin
    { 0x2706, "amount of substance (mole)" },               // ID: org.bluetooth.unit.amount_of_substance.mole
    { 0x2707, "luminous intensity (candela)" },             // ID: org.bluetooth.unit.luminous_intensity.candela
    { 0x2710, "area (square metres)" },                     // ID: org.bluetooth.unit.area.square_metres
    { 0x2711, "volume (cubic metres)" },                    // ID: org.bluetooth.unit.volume.cubic_metres
    { 0x2712, "velocity (metres per second)" },             // ID: org.bluetooth.unit.velocity.metres_per_second
    { 0x2713, "acceleration (metres per second squared)" }, // ID: org.bluetooth.unit.acceleration.metres_per_second_squared
    { 0x2714, "wavenumber (reciprocal metre)" },            // ID: org.bluetooth.unit.wavenumber.reciprocal_metre
    { 0x2715, "density (kilogram per cubic metre)" },       // ID: org.bluetooth.unit.density.kilogram_per_cubic_metre
    { 0x2716, "surface density (kilogram per square metre)" },// ID: org.bluetooth.unit.surface_density.kilogram_per_square_metre
    { 0x2717, "specific volume (cubic metre per kilogram)" },// ID: org.bluetooth.unit.specific_volume.cubic_metre_per_kilogram
    { 0x2718, "current density (ampere per square metre)" },// ID: org.bluetooth.unit.current_density.ampere_per_square_metre
    { 0x2719, "magnetic field strength (ampere per metre)" },// ID: org.bluetooth.unit.magnetic_field_strength.ampere_per_metre
    { 0x271A, "amount concentration (mole per cubic metre)" },// ID: org.bluetooth.unit.amount_concentration.mole_per_cubic_metre
    { 0x271B, "mass concentration (kilogram per cubic metre)" },// ID: org.bluetooth.unit.mass_concentration.kilogram_per_cubic_metre
    { 0x271C, "luminance (candela per square metre)" },     // ID: org.bluetooth.unit.luminance.candela_per_square_metre
    { 0x271D, "refractive index" },                         // ID: org.bluetooth.unit.refractive_index
    { 0x271E, "relative permeability" },                    // ID: org.bluetooth.unit.relative_permeability
    { 0x2720, "plane angle (radian)" },                     // ID: org.bluetooth.unit.plane_angle.radian
    { 0x2721, "solid angle (steradian)" },                  // ID: org.bluetooth.unit.solid_angle.steradian
    { 0x2722, "frequency (hertz)" },                        // ID: org.bluetooth.unit.frequency.hertz
    { 0x2723, "force (newton)" },                           // ID: org.bluetooth.unit.force.newton
    { 0x2724, "pressure (pascal)" },                        // ID: org.bluetooth.unit.pressure.pascal
    { 0x2725, "energy (joule)" },                           // ID: org.bluetooth.unit.energy.joule
    { 0x2726, "power (watt)" },                             // ID: org.bluetooth.unit.power.watt
    { 0x2727, "electric charge (coulomb)" },                // ID: org.bluetooth.unit.electric_charge.coulomb
    { 0x2728, "electric potential difference (volt)" },     // ID: org.bluetooth.unit.electric_potential_difference.volt
    { 0x2729, "capacitance (farad)" },                      // ID: org.bluetooth.unit.capacitance.farad
    { 0x272A, "electric resistance (ohm)" },                // ID: org.bluetooth.unit.electric_resistance.ohm
    { 0x272B, "electric conductance (siemens)" },           // ID: org.bluetooth.unit.electric_conductance.siemens
    { 0x272C, "magnetic flux (weber)" },                    // ID: org.bluetooth.unit.magnetic_flux.weber
    { 0x272D, "magnetic flux density (tesla)" },            // ID: org.bluetooth.unit.magnetic_flux_density.tesla
    { 0x272E, "inductance (henry)" },                       // ID: org.bluetooth.unit.inductance.henry
    { 0x272F, "Celsius temperature (degree Celsius)" },     // ID: org.bluetooth.unit.thermodynamic_temperature.degree_celsius
    { 0x2730, "luminous flux (lumen)" },                    // ID: org.bluetooth.unit.luminous_flux.lumen
    { 0x2731, "illuminance (lux)" },                        // ID: org.bluetooth.unit.illuminance.lux
    { 0x2732, "activity referred to a radionuclide (becquerel)" },// ID: org.bluetooth.unit.activity_referred_to_a_radionuclide.becquerel
    { 0x2733, "absorbed dose (gray)" },                     // ID: org.bluetooth.unit.absorbed_dose.gray
    { 0x2734, "dose equivalent (sievert)" },                // ID: org.bluetooth.unit.dose_equivalent.sievert
    { 0x2735, "catalytic activity (katal)" },               // ID: org.bluetooth.unit.catalytic_activity.katal
    { 0x2740, "dynamic viscosity (pascal second)" },        // ID: org.bluetooth.unit.dynamic_viscosity.pascal_second
    { 0x2741, "moment of force (newton metre)" },           // ID: org.bluetooth.unit.moment_of_force.newton_metre
    { 0x2742, "surface tension (newton per metre)" },       // ID: org.bluetooth.unit.surface_tension.newton_per_metre
    { 0x2743, "angular velocity (radian per second)" },     // ID: org.bluetooth.unit.angular_velocity.radian_per_second
    { 0x2744, "angular acceleration (radian per second squared)" },// ID: org.bluetooth.unit.angular_acceleration.radian_per_second_squared
    { 0x2745, "heat flux density (watt per square metre)" },// ID: org.bluetooth.unit.heat_flux_density.watt_per_square_metre
    { 0x2746, "heat capacity (joule per kelvin)" },         // ID: org.bluetooth.unit.heat_capacity.joule_per_kelvin
    { 0x2747, "specific heat capacity (joule per kilogram kelvin)" },// ID: org.bluetooth.unit.specific_heat_capacity.joule_per_kilogram_kelvin
    { 0x2748, "specific energy (joule per kilogram)" },     // ID: org.bluetooth.unit.specific_energy.joule_per_kilogram
    { 0x2749, "thermal conductivity (watt per metre kelvin)" },// ID: org.bluetooth.unit.thermal_conductivity.watt_per_metre_kelvin
    { 0x274A, "energy density (joule per cubic metre)" },   // ID: org.bluetooth.unit.energy_density.joule_per_cubic_metre
    { 0x274B, "electric field strength (volt per metre)" }, // ID: org.bluetooth.unit.electric_field_strength.volt_per_metre
    { 0x274C, "electric charge density (coulomb per cubic metre)" },// ID: org.bluetooth.unit.electric_charge_density.coulomb_per_cubic_metre
    { 0x274D, "surface charge density (coulomb per square metre)" },// ID: org.bluetooth.unit.surface_charge_density.coulomb_per_square_metre
    { 0x274E, "electric flux density (coulomb per square metre)" },// ID: org.bluetooth.unit.electric_flux_density.coulomb_per_square_metre
    { 0x274F, "permittivity (farad per metre)" },           // ID: org.bluetooth.unit.permittivity.farad_per_metre
    { 0x2750, "permeability (henry per metre)" },           // ID: org.bluetooth.unit.permeability.henry_per_metre
    { 0x2751, "molar energy (joule per mole)" },            // ID: org.bluetooth.unit.molar_energy.joule_per_mole
    { 0x2752, "molar entropy (joule per mole kelvin)" },    // ID: org.bluetooth.unit.molar_entropy.joule_per_mole_kelvin
    { 0x2753, "exposure (coulomb per kilogram)" },          // ID: org.bluetooth.unit.exposure.coulomb_per_kilogram
    { 0x2754, "absorbed dose rate (gray per second)" },     // ID: org.bluetooth.unit.absorbed_dose_rate.gray_per_second
    { 0x2755, "radiant intensity (watt per steradian)" },   // ID: org.bluetooth.unit.radiant_intensity.watt_per_steradian
    { 0x2756, "radiance (watt per square metre steradian)" },// ID: org.bluetooth.unit.radiance.watt_per_square_metre_steradian
    { 0x2757, "catalytic activity concentration (katal per cubic metre)" },// ID: org.bluetooth.unit.catalytic_activity_concentration.katal_per_cubic_metre
    { 0x2760, "time (minute)" },                            // ID: org.bluetooth.unit.time.minute
    { 0x2761, "time (hour)" },                              // ID: org.bluetooth.unit.time.hour
    { 0x2762, "time (day)" },                               // ID: org.bluetooth.unit.time.day
    { 0x2763, "plane angle (degree)" },                     // ID: org.bluetooth.unit.plane_angle.degree
    { 0x2764, "plane angle (minute)" },                     // ID: org.bluetooth.unit.plane_angle.minute
    { 0x2765, "plane angle (second)" },                     // ID: org.bluetooth.unit.plane_angle.second
    { 0x2766, "area (hectare)" },                           // ID: org.bluetooth.unit.area.hectare
    { 0x2767, "volume (litre)" },                           // ID: org.bluetooth.unit.volume.litre
    { 0x2768, "mass (tonne)" },                             // ID: org.bluetooth.unit.mass.tonne
    { 0x2780, "pressure (bar)" },                           // ID: org.bluetooth.unit.pressure.bar
    { 0x2781, "pressure (millimetre of mercury)" },         // ID: org.bluetooth.unit.pressure.millimetre_of_mercury
    { 0x2782, "length (ångström)" },                        // ID: org.bluetooth.unit.length.ångström
    { 0x2783, "length (nautical mile)" },                   // ID: org.bluetooth.unit.length.nautical_mile
    { 0x2784, "area (barn)" },                              // ID: org.bluetooth.unit.area.barn
    { 0x2785, "velocity (knot)" },                          // ID: org.bluetooth.unit.velocity.knot
    { 0x2786, "logarithmic radio quantity (neper)" },       // ID: org.bluetooth.unit.logarithmic_radio_quantity.neper
    { 0x2787, "logarithmic radio quantity (bel)" },         // ID: org.bluetooth.unit.logarithmic_radio_quantity.bel
    { 0x27A0, "length (yard)" },                            // ID: org.bluetooth.unit.length.yard
    { 0x27A1, "length (parsec)" },                          // ID: org.bluetooth.unit.length.parsec
    { 0x27A2, "length (inch)" },                            // ID: org.bluetooth.unit.length.inch
    { 0x27A3, "length (foot)" },                            // ID: org.bluetooth.unit.length.foot
    { 0x27A4, "length (mile)" },                            // ID: org.bluetooth.unit.length.mile
    { 0x27A5, "pressure (pound-force per square inch)" },   // ID: org.bluetooth.unit.pressure.pound_force_per_square_inch
    { 0x27A6, "velocity (kilometre per hour)" },            // ID: org.bluetooth.unit.velocity.kilometre_per_hour
    { 0x27A7, "velocity (mile per hour)" },                 // ID: org.bluetooth.unit.velocity.mile_per_hour
    { 0x27A8, "angular velocity (revolution per minute)" }, // ID: org.bluetooth.unit.angular_velocity.revolution_per_minute
    { 0x27A9, "energy (gram calorie)" },                    // ID: org.bluetooth.unit.energy.gram_calorie
    { 0x27AA, "energy (kilogram calorie)" },                // ID: org.bluetooth.unit.energy.kilogram_calorie
    { 0x27AB, "energy (kilowatt hour)" },                   // ID: org.bluetooth.unit.energy.kilowatt_hour
    { 0x27AC, "thermodynamic temperature (degree Fahrenheit)" },// ID: org.bluetooth.unit.thermodynamic_temperature.degree_fahrenheit
    { 0x27AD, "percentage" },                               // ID: org.bluetooth.unit.percentage
    { 0x27AE, "per mille" },                                // ID: org.bluetooth.unit.per_mille
    { 0x27AF, "period (beats per minute)" },                // ID: org.bluetooth.unit.period.beats_per_minute
    { 0x27B0, "electric charge (ampere hours)" },           // ID: org.bluetooth.unit.electric_charge.ampere_hours
    { 0x27B1, "mass density (milligram per decilitre)" },   // ID: org.bluetooth.unit.mass_density.milligram_per_decilitre
    { 0x27B2, "mass density (millimole per litre)" },       // ID: org.bluetooth.unit.mass_density.millimole_per_litre
    { 0x27B3, "time (year)" },                              // ID: org.bluetooth.unit.time.year
    { 0x27B4, "time (month)" },                             // ID: org.bluetooth.unit.time.month
    { 0x27B5, "concentration (count per cubic metre)" },    // ID: org.bluetooth.unit.concentration.count_per_cubic_metre
    { 0x27B6, "irradiance (watt per square metre)" },       // ID: org.bluetooth.unit.irradiance.watt_per_square_metre
    { 0x27B7, "milliliter (per kilogram per minute)" },     // ID: org.bluetooth.unit.transfer_rate.milliliter_per_kilogram_per_minute
    { 0x27B8, "mass (pound)" },                             // ID: org.bluetooth.unit.mass.pound
    { 0x27B9, "metabolic equivalent" },                     // ID: org.bluetooth.unit.metabolic_equivalent
    { 0x27BA, "step (per minute)" },                        // ID: org.bluetooth.unit.step_per_minute
    { 0x27BC, "stroke (per minute)" },                      // ID: org.bluetooth.unit.stroke_per_minute
    { 0x27BD, "pace (kilometre per minute)" },              // ID: org.bluetooth.unit.velocity.kilometer_per_minute
    { 0x27BE, "luminous efficacy (lumen per watt)" },       // ID: org.bluetooth.unit.luminous_efficacy.lumen_per_watt
    { 0x27BF, "luminous energy (lumen hour)" },             // ID: org.bluetooth.unit.luminous_energy.lumen_hour
    { 0x27C0, "luminous exposure (lux hour)" },             // ID: org.bluetooth.unit.luminous_exposure.lux_hour
    { 0x27C1, "mass flow (gram per second)" },              // ID: org.bluetooth.unit.mass_flow.gram_per_second
    { 0x27C2, "volume flow (litre per second)" },           // ID: org.bluetooth.unit.volume_flow.litre_per_second
    { 0x27C3, "sound pressure (decibel)" },                 // ID: org.bluetooth.unit.sound_pressure.decibel_spl
    { 0x27C4, "parts per million" },                        // ID: org.bluetooth.unit.ppm
    { 0x27C5, "parts per billion" },                        // ID: org.bluetooth.unit.ppb
    { 0x27C6, "mass density rate ((milligram per decilitre) per minute)" },// ID: org.bluetooth.unit.mass_density_rate.milligram_per_decilitre_per_minute
    { 0x27C7, "Electrical Apparent Energy (kilovolt ampere hour)" },// ID: org.bluetooth.unit.energy.kilovolt_ampere_hour
    { 0x27C8, "Electrical Apparent Power (volt ampere)" },  // ID: org.bluetooth.unit.power.volt_ampere
};

std::string get_units_name(uint16_t code)
{
    for (size_t i = 0; data[i].name != nullptr; i++)
    {
        if (data[i].value == code)
        {
            return data[i].name;
        }
    }
    std::ostringstream oss;
    oss << "Unknown [" << static_cast<int>(code) << "]";
    return oss.str();
}
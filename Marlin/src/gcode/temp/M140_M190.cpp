#include "../../inc/MarlinConfig.h"

#if HAS_HEATED_BED

#include "../gcode.h"
#include "../../module/temperature.h"
#include "../../lcd/marlinui.h"

/**
 * M140 / M190 – Set Bed Temperature
 *
 * M140: Define a temperatura da cama e retorna imediatamente.
 * M190: Define a temperatura da cama e aguarda até atingir o valor alvo.
 *
 * Parâmetros:
 *  I<index>  : Índice de predefinição de material (se aplicável)
 *  S<temp>   : Temperatura alvo em °C. Usado com M140 (sem espera) e com M190 (aguarda apenas aquecimento)
 *  R<temp>   : Temperatura alvo em °C. Usado com M190 (aguarda aquecimento ou resfriamento)
 *  P<bed>    : Índice da cama aquecida (0 a 3). Opcional; padrão é cama 0. Ignorado se MULTI_BED desativado.
 *
 * Exemplos:
 *  M140 S60         → Em multi‐bed, aplica 60 °C a todas as camas. Em single‐bed, aplica 60 °C à cama 0.
 *  M190 R40         → Aguarda a(s) cama(s) atingir(em) 40 °C
 *  M140 P2 S70      → Em multi‐bed, aplica apenas cama 2 = 70 °C. Em single‐bed, ignora P e aplica cama 0 = 70 °C.
 *  M190 P1 R90      → Em multi‐bed, aguarda cama 1 atingir 90 °C. Em single‐bed, ignora P e aguarda cama 0 = 90 °C.
 *
 * Observações:
 *  - Com PRINTJOB_TIMER_AUTOSTART ativado, M140 pode parar e M190 pode iniciar o cronômetro de impressão.
 *  - Se ENABLE_MULTI_HEATED_BEDS não estiver ativado, qualquer P<bed> é ignorado e atua sobre cama única (índice 0).
 */



void GcodeSuite::M140_M190(const bool isM190) {
  #if ENABLED(ENABLE_MULTI_HEATED_BEDS)

    // ————— Multi-Bed Version —————
    if (DEBUGGING(DRYRUN)) return;

    bool got_temp = false;
    celsius_t temp = 0;
    bool has_bed = parser.seenval('B');
    uint8_t bed  = has_bed ? parser.value_byte() : 0;

    #if HAS_PREHEAT
      got_temp = parser.seenval('I');
      if (got_temp) {
        const uint8_t idx = parser.value_byte();
        temp = ui.material_preset[_MIN(idx, PREHEAT_COUNT - 1)].bed_temp;
      }
    #endif

    bool no_wait_for_cooling = false;
    if (!got_temp) {
      no_wait_for_cooling = parser.seenval('S');
      got_temp = no_wait_for_cooling || (isM190 && parser.seenval('R'));
      if (got_temp) temp = parser.value_celsius();
    }
    if (!got_temp) return;

    if (has_bed) {
      thermalManager.setTargetBed(bed, temp);
      thermalManager.isHeatingBed(bed)
        ? LCD_MESSAGE(MSG_BED_HEATING)
        : LCD_MESSAGE(MSG_BED_COOLING);
    } else {
      thermalManager.setTargetBed(temp);
      thermalManager.isHeatingBed()
        ? LCD_MESSAGE(MSG_BED_HEATING)
        : LCD_MESSAGE(MSG_BED_COOLING);
    }

    TERN_(PRINTJOB_TIMER_AUTOSTART,
      thermalManager.auto_job_check_timer(isM190, !isM190)
    );

    if (isM190) {
      if (has_bed) thermalManager.wait_for_bed(bed, no_wait_for_cooling);
      else          thermalManager.wait_for_bed(no_wait_for_cooling);
    } else {
      if (has_bed) {
        ui.set_status_reset_fn([=]() {
          const celsius_t c = thermalManager.degTargetBed(bed);
          return c < 30 || thermalManager.degBedNear(bed, c);
        });
      } else {
        ui.set_status_reset_fn([]() {
          const celsius_t c = thermalManager.degTargetBed();
          return c < 30 || thermalManager.degBedNear(c);
        });
      }
    }

  #else //fallback SingleBed

    if (DEBUGGING(DRYRUN)) return;

    bool got_temp = false;
    celsius_t temp = 0;

    // Accept 'I' if temperature presets are defined
    #if HAS_PREHEAT
      got_temp = parser.seenval('I');
      if (got_temp) {
        const uint8_t index = parser.value_byte();
        temp = ui.material_preset[_MIN(index, PREHEAT_COUNT - 1)].bed_temp;
      }
    #endif

    // Get the temperature from 'S' or 'R'
    bool no_wait_for_cooling = false;
    if (!got_temp) {
      no_wait_for_cooling = parser.seenval('S');
      got_temp = no_wait_for_cooling || (isM190 && parser.seenval('R'));
      if (got_temp) temp = parser.value_celsius();
    }

    if (!got_temp) return;

    thermalManager.setTargetBed(temp);
    thermalManager.isHeatingBed() ? LCD_MESSAGE(MSG_BED_HEATING) : LCD_MESSAGE(MSG_BED_COOLING);

    // With PRINTJOB_TIMER_AUTOSTART, M190 can start the timer, and M140 can stop it
    TERN_(PRINTJOB_TIMER_AUTOSTART, thermalManager.auto_job_check_timer(isM190, !isM190));

    if (isM190)
      thermalManager.wait_for_bed(no_wait_for_cooling);
    else
      ui.set_status_reset_fn([]{
        const celsius_t c = thermalManager.degTargetBed();
        return c < 30 || thermalManager.degBedNear(c);
      });
  }
  #endif //ENABLE_MULTI_HEATED_BED
#endif // HAS_HEATED_BED

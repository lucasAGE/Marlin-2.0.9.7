/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

/**
 * gcode/temp/M140_M190.cpp
 *
 * Bed target temperature control
 */

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
 *  M140 S60         → Define a temperatura da cama 0 para 60 °C e retorna imediatamente
 *  M190 R40         → Aguarda a cama 0 atingir 40 °C
 *  M140 P2 S70      → Define a cama 2 para 70 °C e retorna
 *  M190 P1 R90      → Aguarda a cama 1 atingir 90 °C
 *
 * Observações:
 *  - Com PRINTJOB_TIMER_AUTOSTART ativado, M140 pode parar e M190 pode iniciar o cronômetro de impressão.
 *  - A cama selecionada pelo parâmetro P só tem efeito se ENABLE_MULTI_HEATED_BEDS estiver ativado.
 *    Caso contrário, todas as chamadas afetam a cama única padrão (bed 0).
 */


void GcodeSuite::M140_M190(const bool isM190) {

  if (DEBUGGING(DRYRUN)) return;

  bool got_temp = false;
  celsius_t temp = 0;

  #if ENABLED(ENABLE_MULTI_HEATED_BEDS)
    const uint8_t bed = parser.seen('P') ? parser.value_byte() : 0;
    if (bed >= MULTI_BED_COUNT) return;
  #else
    constexpr uint8_t bed = 0;
  #endif

  #if HAS_PREHEAT
    got_temp = parser.seenval('I');
    if (got_temp) {
      const uint8_t index = parser.value_byte();
      temp = ui.material_preset[_MIN(index, PREHEAT_COUNT - 1)].bed_temp;
    }
  #endif

  bool no_wait_for_cooling = false;
  if (!got_temp) {
    no_wait_for_cooling = parser.seenval('S');
    got_temp = no_wait_for_cooling || (isM190 && parser.seenval('R'));
    if (got_temp) temp = parser.value_celsius();
  }

  if (!got_temp) return;

  thermalManager.setTargetBed(bed, temp);
  thermalManager.isHeatingBed(bed) ? LCD_MESSAGE(MSG_BED_HEATING) : LCD_MESSAGE(MSG_BED_COOLING);

  TERN_(PRINTJOB_TIMER_AUTOSTART, thermalManager.auto_job_check_timer(isM190, !isM190));

  if (isM190) {
    thermalManager.wait_for_bed(bed, no_wait_for_cooling);
  }
  else {
    switch (bed) {
  case 0: ui.set_status_reset_fn([]{ const celsius_t c = thermalManager.degTargetBed(0); return c < 30 || thermalManager.degBedNear(0, c); }); break;
  case 1: ui.set_status_reset_fn([]{ const celsius_t c = thermalManager.degTargetBed(1); return c < 30 || thermalManager.degBedNear(1, c); }); break;
  case 2: ui.set_status_reset_fn([]{ const celsius_t c = thermalManager.degTargetBed(2); return c < 30 || thermalManager.degBedNear(2, c); }); break;
  case 3: ui.set_status_reset_fn([]{ const celsius_t c = thermalManager.degTargetBed(3); return c < 30 || thermalManager.degBedNear(3, c); }); break;
  default: ui.set_status_reset_fn(nullptr); break;
}

  }
}

#endif // HAS_HEATED_BED
